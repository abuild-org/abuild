package org.abuild.groovy

import org.abuild.groovy.Util
import org.abuild.groovy.DependencyGraph
import org.abuild.groovy.Parameterized
import org.abuild.javabuilder.BuildArgs
import org.apache.tools.ant.BuildException
import org.abuild.QTC

class BuildState implements Parameterized
{
    // The word "public" before a field indicates that it is intended
    // as part of the public interface.  Fields not marked either
    // "public" or "private" are public in groovy, but they are
    // intended to be accessed only from the Backend class in this
    // file.  (As of 1.6, groovy still doesn't honor public, private,
    // and protected anywa.)

    // fields supplied by .ab-dynamic.groovy
    public interfaceVars = [:]
    def itemPaths = [:]
    def abuildTop
    def pluginPaths
    def rulePaths

    // supplied by abuild
    public defines
    public buildDirectory
    BuildArgs buildArgs

    // variables set by the user
    public params = [:]

    // other accessible fields
    public File sourceDirectory = null

    // used internally and by Backend
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

    void setParameter(String name, Object value)
    {
        params[name] = ['list': false, 'value': value]
    }

    void appendParameter(String name, Object value)
    {
        if (params.containsKey(name))
        {
            if (! params[name].list)
            {
                fail("parameter $name has been previously set as non-list")
            }
        }
        else
        {
            params[name] = ['list': true, 'value': []]
        }
        params[name]['value'] << value
    }

    void deleteParameter(String name)
    {
        params.remove(name)
    }

    Object resolve(String name, defaultValue)
    {
        // Note that we return params[name].value even if null.  For
        // defines, we can have only string values, and for
        // interfaceVars, we can have only strings and lists.
        (defines[name] ?:
         (params.containsKey(name) ? params[name].value :
              (interfaceVars[name] ?: defaultValue)))
    }

    Object resolve(String name)
    {
        resolve(name, null)
    }

    // may return null
    String resolveAsString(String name, defaultValue)
    {
        def result = resolve(name, defaultValue)
        if (result instanceof GString)
        {
            result = result as String
        }
        if (! ((result == null) || (result instanceof String)))
        {
            fail("resolveAsString called on non-string parameter" +
                 " ${name}, value ${result}")
        }
        result
    }

    String resolveAsString(String name)
    {
        resolveAsString(name, null)
    }

    // may return null
    List resolveAsList(String name, defaultValue)
    {
        def result = resolve(name, defaultValue)
        if ((result != null) && (! (result instanceof List)))
        {
            result = [result]
        }
        result
    }

    List resolveAsList(String name)
    {
        resolveAsList(name, null)
    }

    def runActions(String parameter, Closure defaultAction)
    {
        runActions(resolveAsList(parameter, [:]), defaultAction)
    }

    def runActions(List actions, Closure defaultAction)
    {
        actions.each {
            action ->
            switch (action)
            {
              case Closure:
                action()
                break;

              case Map:
                defaultAction(action)
                break;

              default:
                fail('expected elements of $parameter to be a Closure or Map')
                break;
            }
        }
    }

    def fail(String message)
    {
        throw new BuildFailure(message)
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
                catch (BuildFailure e)
                {
                    QTC.TC("abuild", "groovy ERR abuild BuildFailure")
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
                    Util.printStackTrace(exc_to_print)
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
