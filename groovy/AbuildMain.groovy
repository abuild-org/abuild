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

    File curFile = null
/*
    def g = new DependencyGraph()
    def closures = [:]

    BuildState()
    {
        registerTarget('all')
    }

    def registerTarget(String name)
    {
        registerTarget(null, name, null)
    }

    def registerTarget(Map parameters, String name)
    {
        registerTarget(parameters, name, null)
    }

    def registerTarget(String name, Closure body)
    {
        registerTarget(null, name, body)
    }

    def registerTarget(Map parameters, String name, Closure body)
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
    }
*/
}


class Builder
{
    def loader = new GroovyClassLoader()
    def binding = new Binding()
    def buildState = new BuildState()

    Builder()
    {
        binding.abuild = buildState;
        binding.ant = new AntBuilder()
    }

    def loadScripts(files)
    {
        files.each {
            file ->
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

    def runTargets(List targets)
    {
        buildState.runTargets(targets)
    }
}

// overall status
def status = false

// Check values passed in from abuild
//   buildDirectory
//   targets
//   defines

def dynamicFile = new File(buildDirectory.path + "/.ab-dynamic.groovy")
def buildFile = new File(buildDirectory.parent + "/Abuild.groovy")
def b = new Builder()
b.loadScripts([dynamicFile, buildFile])

// XXX do build
status = true

status
