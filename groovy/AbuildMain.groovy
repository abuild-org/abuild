import org.codehaus.groovy.control.CompilationFailedException
import org.codehaus.groovy.runtime.StackTraceUtils
import org.abuild.support.DependencyGraph

class BuildState
{
    // fields supplied by .ab-dynamic.groovy
    def ifc = [:]
    def itemPaths = [:]
    def abuildTop = null
    def pluginPaths = null

    // supplied from abuild
    def defines = null

    File curFile = null

    def g = new DependencyGraph()
    def closures = [:]

    BuildState(defines)
    {
        this.defines = defines

        // Create targets that abuild guarantees will exist
        setTarget('all')

        // XXX People who define test wrappers must ensure that the
        // same code is set as the body of test-only, test, and check.
        setTarget('test-only')
        setTarget('test', 'deps':'all')
        setTarget('check', 'deps':'all')

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

    def runTargets(List targets)
    {
        def status = true

        if (! g.check())
        {
            // XXX clean up error message, include enough information
            // at registration time to be able to explain where these
            // errors were introduced.
            println "unknowns:\n" + g.unknowns
            println "cycles:\n" + g.cycles
            // XXX FAIL
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
                    println "unknown target ${target}"
                    // XXX FAIL
                }
            }
            def to_run = g.sortedGraph.grep { target_set.containsKey(it) }
            to_run.each {
                target ->
                closures[target].each {
                    d ->
                    def file = d[0]
                    def cl = d[1]
                    try
                    {
                        println "--> running target ${target} from ${file}"
                        cl()
                    }
                    catch (Exception e)
                    {
                        // XXX probably want to specifically catch
                        // org.apache.tools.ant.BuildException and our
                        // own custom BuildException as well.

                        print "Caught exception ${e.class.name}" +
                            " while running {$target} code from ${file}: "
                        StackTraceUtils.deepSanitize(e)
                        e.printStackTrace()
                        // XXX FAIL
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
    def groovyArgs
    def defines

    Builder(File buildDirectory, targets, groovyArgs, defines)
    {
        this.buildDirectory = buildDirectory
        this.targets = targets
        this.groovyArgs = groovyArgs
        this.defines = defines

        // XXX groovyArgs can be -n, -k, -e, -v, or -q.

        this.buildState = new BuildState(defines)

        def ant = new AntBuilder()
        ant.project.setBasedir(buildDirectory.absolutePath)

        binding.abuild = buildState
        binding.ant = ant
    }

    boolean build()
    {
        def dynamicFile = new File(buildDirectory.path + "/.ab-dynamic.groovy")
        def buildFile = new File(buildDirectory.parent + "/Abuild.groovy")

        loadScript(dynamicFile)
        loadScript(buildFile)

        // include qtest support
        // plugin
        // rules
        // local

        this.buildState.runTargets(this.targets)
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
            // XXX FAIL
            println "file ${file.path} had compilation errors"
            StackTraceUtils.deepSanitize(e)
            e.printStackTrace()
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
            print "file ${file} threw exception: "
            StackTraceUtils.deepSanitize(e)
            e.printStackTrace()
            // XXX FAIL
        }
    }
}

// overall script returns value returned by build()
new Builder(buildDirectory, targets, groovyArgs, defines).build()
