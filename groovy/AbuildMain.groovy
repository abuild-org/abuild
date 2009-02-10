import org.codehaus.groovy.control.CompilationFailedException
import org.codehaus.groovy.runtime.StackTraceUtils
import org.abuild.support.DependencyGraph
import org.apache.tools.ant.Project
import org.apache.tools.ant.BuildException

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
    def abuildTop = null
    def pluginPaths = null

    // supplied from abuild
    def defines = null
    File buildDirectory = null

    // other accessible fields
    File sourceDirectory = null
    boolean verbose = false
    boolean quiet = false
    boolean emacsMode = false
    boolean keepGoing = false

    // private
    private AntBuilder ant = null
    private File curFile = null

    private g = new DependencyGraph()
    private closures = [:]

    BuildState(AntBuilder ant, File buildDirectory,
               defines, boolean verbose,
               boolean quiet, boolean emacsMode, boolean keepGoing)
    {
        this.ant = ant
        this.buildDirectory = buildDirectory
        this.sourceDirectory = buildDirectory.parentFile
        this.defines = defines
        this.verbose = verbose
        this.quiet = quiet
        this.emacsMode = emacsMode
        this.keepGoing = keepGoing

        // Create targets that abuild guarantees will exist
        setTarget('all')

        // XXX People who define test wrappers must ensure that the
        // same code is set as the body of test-only and test.
        setTarget('test-only')
        setTarget('test', 'deps':'all')

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
        }
        if (body)
        {
            closures[name] << [curFile, body]
        }
    }

    def fail(String message)
    {
        throw new AbuildBuildFailure(message)
    }

    private runTargets(List targets)
    {
        def status = true

        if (! g.check())
        {
            // XXX clean up error message, include enough information
            // at registration time to be able to explain where these
            // errors were introduced.
            System.err.println "unknowns:\n" + g.unknowns
            System.err.println "cycles:\n" + g.cycles
            status = false
        }
        else
        {
            def target_set = [:]
            targets.each {
                target ->
                if (closures.containsKey(target))
                {
                    target_set[target] = 1
                    g.getSortedDependencies(target).each {
                        dep ->
                        target_set[dep] = 1
                    }
                }
                else
                {
                    // XXX error message formatting?
                    System.err.println "ERROR: unknown target ${target}"
                    status = false
                }
            }
            // XXX deal with failure propagation and keepGoing mode
            def to_run = g.sortedGraph.grep { target_set.containsKey(it) }
            to_run.each {
                target ->
                closures[target].each {
                    d ->
                    def file = d[0]
                    def cl = d[1]
                    def exc_to_print = null
                    try
                    {
                        if (verbose)
                        {
                            println "--> running target ${target} from ${file}"
                        }
                        cl()
                    }
                    catch (AbuildBuildFailure e)
                    {
                        System.err.println "ERROR: build failure: " + e.message
                        if (verbose)
                        {
                            exc_to_print = e
                        }
                        status = false;
                    }
                    catch (BuildException e)
                    {
                        System.err.println "ERROR: ant build failure: " +
                            e.message
                        if (verbose)
                        {
                            exc_to_print = e
                        }
                        status = false
                    }
                    catch (Exception e)
                    {
                        System.err.print "ERROR: Caught exception" +
                            " ${e.class.name}" +
                            " while running ${target} code from ${file}: "
                        // Print exception details unconditionally if
                        // it was not a standard build failure.
                        exc_to_print = e
                        status = false
                    }

                    if (exc_to_print)
                    {
                        StackTraceUtils.deepSanitize(exc_to_print)
                        exc_to_print.printStackTrace(System.err)
                    }
                }
            }
        }

        status
    }
}


class Builder
{
    def loader = new GroovyClassLoader()
    def binding = new Binding()

    def buildState
    File buildDirectory
    def targets
    def otherArgs
    def defines

    boolean noOp = false

    Builder(File buildDirectory, project, targets, otherArgs, defines)
    {
        this.buildDirectory = buildDirectory
        this.targets = targets
        this.otherArgs = otherArgs
        this.defines = defines

        // otherArgs are passed internally by abuild.  They do not
        // come from the user.  The Java code already handled some,
        // but there are some that we need to know as well.
	boolean emacsMode = false
        boolean verbose = false
        boolean quiet = false
        boolean keepGoing = false
	for (String arg: otherArgs)
	{
	    if (arg.equals("-e"))
	    {
                emacsMode = true
            }
	    else if (arg.equals("-v"))
	    {
                verbose = true
	    }
	    else if (arg.equals("-q"))
	    {
                quiet = true
	    }
	    else if (arg.equals("-n"))
	    {
		noOp = true
	    }
	    else if (arg.equals("-k"))
	    {
		keepGoing = true
	    }
        }

        def ant = new AntBuilder(project)
        this.buildState = new BuildState(
            ant, buildDirectory, defines,
            verbose, quiet, emacsMode, keepGoing)

        binding.abuild = buildState
        binding.ant = ant
    }

    boolean build()
    {
        if (noOp)
        {
            println "would build targets: " + targets.join(" ")
            return true
        }
        def dynamicFile = new File(buildDirectory.path + "/.ab-dynamic.groovy")
        def buildFile = new File(buildDirectory.parent + "/Abuild.groovy")

        loadScript(dynamicFile)
        loadScript(buildFile)

        loadScript(buildState.abuildTop + "/groovy/QTestSupport.groovy")

        // plugin
        // rules
        // local

        // return value returned by runTargets
        this.buildState.runTargets(this.targets)
    }

    def loadScript(String filename)
    {
        loadScript(new File(filename))
    }

    def loadScript(File file)
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
            System.err.println "ERROR: file ${file.path} had compilation errors"
            throw e
        }
    }

    def runScript(file, object)
    {
        object.setBinding(binding)
        try
        {
            object.run()
        }
        catch (Exception e)
        {
            System.err.println "ERROR: file ${file} threw exception"
            throw e
        }
    }
}

// overall script returns value returned by build()
boolean status = false
try
{
    status = new Builder(buildDirectory, antProject,
                         targets, groovyArgs, defines).build()
}
catch (Exception e)
{
    System.err.print "Exception caught during build: " + e
    StackTraceUtils.deepSanitize(e)
    e.printStackTrace()
}
// XXX test various failure types
status
