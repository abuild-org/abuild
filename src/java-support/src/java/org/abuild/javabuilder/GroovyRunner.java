package org.abuild.javabuilder;

import java.io.File;
import java.util.List;
import java.util.Map;
import org.apache.tools.ant.Project;

public class GroovyRunner implements BuildRunner
{
    // We have to use reflection to instantiate the groovy backend to
    // avoid interdependency between the groovy sources and the Java
    // sources.

    Class groovyClass = null;

    public GroovyRunner()
    {
	try
	{
	    this.groovyClass = Class.forName("org.abuild.groovy.Backend");
	}
	catch (ClassNotFoundException e)
	{
	    System.err.println(
		"abuild: unable to load the abuild groovy builder");
	    e.printStackTrace(System.err);
	}
    }

    public boolean invokeBackend(
	String buildFileName, String dirName,
	BuildArgs buildArgs, Project antProject,
	List<String> targets, Map<String, String> defines)
    {
	boolean status = false;
	Exception exc = null;
	try
	{
	    GroovyBackend backend = (GroovyBackend) groovyClass.newInstance();
	    status = backend.run(
		new File(dirName), buildArgs, antProject, targets, defines);
	}
	catch (InstantiationException e)
	{
	    exc = e;
	}
	catch (IllegalAccessException e)
	{
	    exc = e;
	}
	if (exc != null)
	{
	    System.err.println(
		"abuild: unable to invoke the abuild groovy builder");
	    exc.printStackTrace(System.err);
	}
	return status;
    }
}
