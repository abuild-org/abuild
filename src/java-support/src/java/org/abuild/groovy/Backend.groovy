package org.abuild.groovy

import org.abuild.groovy.Util
import org.abuild.groovy.ParameterBuilder
import org.abuild.javabuilder.GroovyBackend
import org.codehaus.groovy.control.CompilationFailedException
import org.abuild.javabuilder.BuildArgs
import org.apache.tools.ant.Project
import org.abuild.QTC

class Backend implements GroovyBackend
{
    File buildDirectory
    BuildArgs buildArgs
    def targets

    def loader = new GroovyClassLoader()
    def binding = new Binding()

    def buildState

    boolean run(File buildDirectory, BuildArgs buildArgs, Project antProject,
		List<String> targets, Map<String, String> defines)
    {
        this.buildDirectory = buildDirectory
        this.buildArgs = buildArgs
        this.targets = targets

        def ant = new AntBuilder(antProject)
        this.buildState = new BuildState(
            ant, buildDirectory, buildArgs, defines)

        def parameters = new ParameterBuilder(parameters : buildState.params)

        binding.abuild = buildState
        binding.ant = ant

        // For some reason, when putting "parameters" into the
        // binding, groovy fails to recognize parameters() as
        // parameters.call() but instead tries to resolve it as a
        // method call in the script.  We use an expicit closure
        // instead.
        binding.parameters = { parameters(it) }

        boolean status = false
        try
        {
            status = build()
        }
        catch (Exception e)
        {
            QTC.TC("abuild", "groovy ERR exception during build")
            String message = e.message
            if ((System.getenv("IN_TESTSUITE") != null) &&
                    (e instanceof CompilationFailedException))
            {
                message = "--COMPILATION ERRORS SUPPRESSED--\n"
            }
            System.err.print "Exception caught during build: " + message
            Util.printStackTrace(e)
        }

        status
    }

    boolean build()
    {
        // The logic here strongly parallels abuild's "make" logic.

        def dynamicFile = new File(buildDirectory.path + "/.ab-dynamic.groovy")
        def sourceDirectory = buildDirectory.parentFile
        def buildFile = new File(sourceDirectory.path + "/Abuild.groovy")

        loadScript(dynamicFile)
        loadScript(buildFile)

        def groovyTop = buildState.abuildTop + "/groovy"
        loadScript(groovyTop + "/QTestSupport.groovy")
        def targetType = buildState.interfaceVars['ABUILD_TARGET_TYPE']

        def ruleSearchPath = [new File("${groovyTop}/rules/${targetType}")]

        // Load plugin code and populate ruleSearchPath
        buildState.pluginPaths.each {
            File f = new File("${it}/Plugin.groovy")
            if (f.isFile())
            {
                loadScript(f)
            }
            File rulesDir = new File("${it}/rules/${targetType}")
            if (rulesDir.isDirectory())
            {
                ruleSearchPath << rulesDir
            }
        }

        if (! (buildState.params['abuild.rules'] ||
               buildState.params['abuild.localRules'] ||
               buildState.ruleItems))
        {
            QTC.TC("abuild", "groovy ERR no rules")
            buildState.error(
                "no build rules are defined; one of abuild.rules or" +
                " abuild.localRules must be defined, or at least one" +
                " dependency must have been specified with the" +
                " -with-rules option")
            return false
        }

        // Load any rules specified in params['abuild.rules'].  First
        // search the internal location, and then search in each
        // plugin directory, returning the first item found.
        buildState.resolveAsList('abuild.rules')?.each {
            rule ->
            def found = false
            for (dir in ruleSearchPath)
            {
                File f = new File("${dir.path}/${rule}.groovy")
                if (f.isFile())
                {
                    loadScript(f)
                    found = true
                    break
                }
            }
            if (! found)
            {
                buildState.error("unable to find rule \"${rule}\"")
            }
        }

        // Load build item rules
        buildState.ruleItems.each {
            item ->
            loadScript(new File(buildState.itemPaths[item] + '/Rules.groovy'))
        }

        // Load any local rules files, resolving the path relative to
        // the source directory
        buildState.resolveAsList('abuild.localRules')?.each {
            loadScript(new File("${sourceDirectory.path}/${it}.groovy"))
        }

        if (! buildState.checkGraph())
        {
            return false
        }

        boolean status = this.buildState.runTargets(this.targets)
        if (buildState.anyFailures)
        {
            status = false
        }
        status
    }

    private loadScript(String filename)
    {
        loadScript(new File(filename))
    }

    private loadScript(File file)
    {
        if (buildArgs.verbose)
        {
            println "--> loading ${file.path}"
        }
        try
        {
            Class groovyClass = loader.parseClass(file)
            if (groovyClass)
            {
                GroovyObject groovyObject = groovyClass.newInstance()
                buildState.curFile = file
                runScript(file, groovyObject)
                buildState.curFile = null
            }
        }
        catch (CompilationFailedException e)
        {
            QTC.TC("abuild", "groovy ERR script compilation errors")
            buildState.error("file ${file.path} had compilation errors")
            throw e
        }
    }

    private runScript(file, object)
    {
        object.setBinding(binding)
        try
        {
            object.run()
        }
        catch (Exception e)
        {
            QTC.TC("abuild", "groovy ERR script run exception")
            buildState.error("file ${file} threw exception: " + e.message)
            throw e
        }
    }
}