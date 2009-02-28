import org.codehaus.groovy.control.CompilationFailedException
import org.codehaus.groovy.runtime.StackTraceUtils
import org.abuild.javabuilder.DependencyGraph
import org.abuild.javabuilder.BuildArgs
import org.abuild.javabuilder.ParameterBuilder
import org.apache.tools.ant.Project
import org.apache.tools.ant.BuildException
import org.abuild.QTC

class AbuildBuildFailure extends Exception
{
    AbuildBuildFailure(String message)
    {
        super(message)
    }
}

class BuildState
{
    // The word "public" before a field indicates that it is intended
    // as part of the public interface.  Fields not marked either
    // "public" or "private" are public in groovy, but they are
    // intended to be accessed only from the Builder class in this
    // file.  (As of 1.6, groovy still doesn't honor public, private,
    // and protected anywa.)

    // fields supplied by .ab-dynamic.groovy
    public interfaceVars = [:]
    def itemPaths = [:]
    def abuildTop
    def pluginPaths
    def ruleItems

    // supplied by abuild
    public defines
    public buildDirectory
    BuildArgs buildArgs

    // variables set by the user
    public params = [:]

    // other accessible fields
    public File sourceDirectory = null

    // used internally and by Builder
    def anyFailures = false

    // private
    private AntBuilder ant
    private File curFile
    private g = new DependencyGraph()
    private closures = [:]
    private targetDepOrigins = [:]
    private targetsRun = [:]
    private boolean ready = false

    BuildState(AntBuilder ant, File buildDirectory,
               BuildArgs buildArgs, defines)
    {
        this.ant = ant
        this.buildDirectory = buildDirectory
        this.buildArgs = buildArgs
        this.defines = defines

        this.sourceDirectory = buildDirectory.parentFile

        // Create targets that abuild guarantees will exist
        addTarget('all')

        // Make test call test-only after building "all".
        addTarget('test-only')
        configureTarget('test', 'deps':'all') {
            QTC.TC("abuild", "groovy built-in test target")
            runTarget('test-only')
        }

        // Make check an alias for test
        addTargetDependencies('check', 'test')

        addTarget('doc')
    }

    def addTarget(String name)
    {
        configureTarget(null, name, null)
    }

    def addTargetDependencies(String name, deps)
    {
        configureTarget(name, 'deps' : deps, null)
    }

    def addTargetClosure(String name, cl)
    {
        configureTarget(null, name, cl)
    }

    def configureTarget(String name)
    {
        configureTarget(null, name, null)
    }

    def configureTarget(Map parameters, String name)
    {
        configureTarget(parameters, name, null)
    }

    def configureTarget(String name, Closure body)
    {
        configureTarget(null, name, body)
    }

    def configureTarget(Map parameters, String name, Closure body)
    {
        if (this.ready)
        {
            QTC.TC("abuild", "groovy ERR configureTarget after init")
            fail("configureTarget may not be called after initialization")
        }
        if (! g.dependencies.containsKey(name))
        {
            g.addItem(name)
            closures[name] = []
        }
        boolean replaceClosures = false
        for (key in parameters?.keySet()?.sort())
        {
            if (key == 'deps')
            {
                [parameters['deps']].flatten().each {
                    dep ->
                    g.addDependency(name, dep)
                }
                if (curFile)
                {
                    // Store the fact that this file is an origin for
                    // dependencies being defiled for this target so we
                    // can use this information in error messages.
                    if (! targetDepOrigins.containsKey(name))
                    {
                        targetDepOrigins[name] = [:]
                    }
                    targetDepOrigins[name][curFile] = 1
                }
            }
            else if (key == 'replaceClosures')
            {
                replaceClosures = parameters['replaceClosures']
            }
            else
            {
                def message = "configureTarget: unknown parameter ${key}"
                if (curFile)
                {
                    message += " in file ${curFile}"
                }
                error(message)
            }
        }
        if (replaceClosures)
        {
            closures[name] = []
        }
        if (body)
        {
            def origin = curFile ?: 'built-in targets'
            closures[name] << [origin, body]
        }
    }

    def resetParameter(String name, value)
    {
        params[name] = value
    }

    def appendParameter(String name, value)
    {
        if (params.containsKey(name))
        {
            if (! (params[name] instanceof List))
            {
                params[name] = [params[name]]
            }
        }
        else
        {
            params[name] = []
        }
        params[name] << value
    }

    def resolveVariable(String name)
    {
        resolveVariable(name, null)
    }

    def resolveVariable(String name, defaultValue)
    {
        defines[name] ?: params[name] ?: interfaceVars[name] ?: defaultValue
    }

    def resolveVariableAsString(String name)
    {
        resolveVariableAsString(name, null)
    }

    def resolveVariableAsString(String name, defaultValue)
    {
        def result = resolveVariable(name, defaultValue)
        if (result instanceof List)
        {
            result = result.join(' ')
        }
        result.toString()
    }

    def resolveVariableAsList(String name)
    {
        resolveVariableAsList(name, null)
    }

    def resolveVariableAsList(String name, defaultValue)
    {
        def result = resolveVariable(name, defaultValue)
        if ((result != null) && (! (result instanceof List)))
        {
            result = [result]
        }
        result
    }

    def fail(String message)
    {
        throw new AbuildBuildFailure(message)
    }

    def error(String message)
    {
        this.anyFailures = true
        System.err.println "abuild-groovy: ERROR: ${message}"
    }

    boolean checkGraph()
    {
        if (! g.check())
        {
            QTC.TC("abuild", "groovy ERR graph errors")

            def targetsOfInterest = [:]
            for (u in g.unknowns.keySet().sort())
            {
                targetsOfInterest[u] = 1
                g.unknowns[u].each {
                    error("target \"${u}\" depends on unknown target \"${it}\"")
                }
            }
            for (c in g.cycles)
            {
                error("the following targets are involved" +
                      " in a dependency cycle: " +
                      c.collect { "\"${it}\"" }.join(", "))
                c.each { targetsOfInterest[it] = 1 }
            }

            for (t in targetsOfInterest.keySet().sort())
            {
                if (targetDepOrigins.containsKey(t))
                {
                    error("dependencies are defined for target \"${t}\"" +
                          " in the following files:")
                    targetDepOrigins[t].keySet().each {
                        error("  ${it}")
                    }
                }
            }

            return false
        }
        this.ready = true
        true
    }

    boolean runTarget(target)
    {
        if (! this.ready)
        {
            QTC.TC("abuild", "groovy ERR runTarget during init")
            fail("runTarget may not be called during initialization")
        }

        if (anyFailures && (! buildArgs.keepGoing))
        {
            return false
        }

        if (targetsRun.containsKey(target))
        {
            if (targetsRun[target] == null)
            {
                QTC.TC("abuild", "groovy ERR re-entrant target")
                fail("target \"${target}\" called itself most likely as a" +
                     " result of an explicit runTarget on itself or one" +
                     "of its reverse dependencies")
            }
            else
            {
                QTC.TC("abuild", "groovy cached target result")
                return targetsRun[target]
            }
        }

        // DO NOT RETURN BELOW THIS POINT until the end of the
        // function.

        // Cache result initially as null to prevent loops while
        // invoking targets.  We will replace the cache result with
        // an actual boolean after the target run completes.
        boolean status = targetsRun[target] = null

        if (! closures.containsKey(target))
        {
            QTC.TC("abuild", "groovy ERR unknown target")
            error("unknown target ${target}")
        }
        else if (! runTargets(g.getDirectDependencies(target)))
        {
            QTC.TC("abuild", "groovy ERR dep failure")
            error("not building target \"${target}\" because of" +
                  " failure of its dependencies")
        }
        else
        {
            status = true
            for (d in closures[target])
            {
                def origin = d[0]
                def cl = d[1]
                def exc_to_print = null
                try
                {
                    if (buildArgs.verbose)
                    {
                        println "--> running target ${target} from ${origin}"
                    }
                    cl()
                    if (anyFailures && (! buildArgs.keepGoing))
                    {
                        QTC.TC("abuild", "groovy stop after closure error")
                        break
                    }
                }
                catch (AbuildBuildFailure e)
                {
                    QTC.TC("abuild", "groovy ERR AbuildBuildFailure")
                    error("build failure: " + e.message)
                    if (buildArgs.verbose)
                    {
                        exc_to_print = e
                    }
                    status = false
                }
                catch (BuildException e)
                {
                    QTC.TC("abuild", "groovy ERR ant BuildException")
                    error("ant build failure: " + e.message)
                    if (buildArgs.verbose)
                    {
                        exc_to_print = e
                    }
                    status = false
                }
                catch (Exception e)
                {
                    QTC.TC("abuild", "groovy ERR other target exception")
                    error("Caught exception ${e.class.name}" +
                          " while running code for target \"${target}\"" +
                          " from ${origin}: " +
                          e.message)
                    // Print exception details unconditionally if
                    // it was not a standard build failure.
                    exc_to_print = e
                    status = false
                }

                if (exc_to_print)
                {
                    Builder.printStackTrace(exc_to_print)
                }
            }
        }

        if (! status)
        {
            anyFailures = true
        }

        // cache and return
        targetsRun[target] = status
    }

    boolean runTargets(List targets)
    {
        def status = true
        targets.each {
            target ->
            if (! runTarget(target))
            {
                status = false
            }
        }
        status
    }
}

class Builder
{
    File buildDirectory
    BuildArgs buildArgs
    def targets

    def loader = new GroovyClassLoader()
    def binding = new Binding()

    def buildState
    DependencyGraph g

    Builder(File buildDirectory, BuildArgs buildArgs, Project antProject,
            targets, defines)
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
        buildState.resolveVariableAsList('abuild.rules')?.each {
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
        buildState.resolveVariableAsList('abuild.localRules')?.each {
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

    static printStackTrace(Throwable e)
    {
        boolean inTestSuite = (System.getenv("IN_TESTSUITE") != null)
        if (inTestSuite)
        {
            System.err.println("--begin stack trace--")
        }
        StackTraceUtils.deepSanitize(e)
        e.printStackTrace(System.err)
        if (inTestSuite)
        {
            System.err.println("--end stack trace--")
        }
    }
}

boolean status = false
try
{
    // Arguments passed via binding from GroovyRunner.java
    status = new Builder(buildDirectory, buildArgs, antProject,
                         targets, defines).build()
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
    Builder.printStackTrace(e)
}
// overall script returns value returned by build()
status
