import org.codehaus.groovy.control.CompilationFailedException
import org.codehaus.groovy.runtime.StackTraceUtils
import org.abuild.javabuilder.DependencyGraph
import org.abuild.javabuilder.BuildArgs
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
    // fields supplied by .ab-dynamic.groovy
    def ifc = [:]
    def itemPaths = [:]
    def abuildTop
    def pluginPaths

    // supplied by abuild
    def prop
    File buildDirectory
    BuildArgs buildArgs

    // other accessible fields
    File sourceDirectory = null

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
        this.prop = defines

        this.sourceDirectory = buildDirectory.parentFile

        // Create targets that abuild guarantees will exist
        setTarget('all')

        // Make test call test-only after building "all".
        setTarget('test-only')
        setTarget('test', 'deps':'all') {
            QTC.TC("abuild", "groovy built-in test target")
            runTarget('test-only')
        }

        // Make check an alias for test
        setTarget('check', 'deps':'test')

        setTarget('doc')
    }

    def setTarget(String name)
    {
        setTarget(null, name, null)
    }

    def setTarget(Map parameters, String name)
    {
        setTarget(parameters, name, null)
    }

    def setTarget(String name, Closure body)
    {
        setTarget(null, name, body)
    }

    def setTarget(Map parameters, String name, Closure body)
    {
        if (! g.dependencies.containsKey(name))
        {
            g.addItem(name)
            closures[name] = []
        }
        if (parameters?.containsKey('deps'))
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
        if (body)
        {
            def origin = curFile ?: 'built-in targets'
            closures[name] << [origin, body]
        }
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
            return targetsRun[target]
        }

        // DO NOT RETURN BELOW THIS POINT until the end of the
        // function.

        // Cache result initially as failure to prevent loops while
        // invoking targets.  We will replace the cache result with
        // true if the target run succeeds.
        boolean status = targetsRun[target] = false

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
            closures[target].each {
                d ->
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
    def targets

    def loader = new GroovyClassLoader()
    def binding = new Binding()

    def buildState
    DependencyGraph g

    Builder(File buildDirectory, BuildArgs buildArgs, Project antProject,
            targets, defines)
    {
        this.buildDirectory = buildDirectory
        this.targets = targets

        def ant = new AntBuilder(antProject)
        this.buildState = new BuildState(
            ant, buildDirectory, buildArgs, defines)

        binding.abuild = buildState
        binding.ant = ant
    }

    boolean build()
    {
        // The logic here strongly parallels abuild's "make" logic.

        def dynamicFile = new File(buildDirectory.path + "/.ab-dynamic.groovy")
        def sourceDirectory = buildDirectory.parentFile
        def buildFile = new File(sourceDirectory.path + "/Abuild.groovy")

        loadScript(dynamicFile)
        loadScript(buildFile)

        loadScript(buildState.abuildTop + "/groovy/QTestSupport.groovy")

        // Load plugin code

        // XXX Plugin.groovy for each plugin

        // Load any rules specified in prop['abuild.rules'].  First
        // search the internal location, and then search in each
        // plugin directory, returning the first item found.

        // XXX ${it}.groovy

        // Load build item rules

        // XXX load Rules.groovy from each requested build item

        // Load any local rules files, resolving the path relative to
        // the source directory

        if (buildState.prop.containsKey('abuild.local-rules'))
        {
            buildState.prop['abuild.local-rules'].each {
                loadScript(new File(sourceDirectory.path + "/" + it))
            }
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
