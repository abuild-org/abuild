package org.abuild.javabuilder;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;
import groovy.lang.Binding;
import groovy.lang.Script;
import groovy.lang.GroovyClassLoader;
import org.apache.tools.ant.Project;

public class GroovyRunner implements BuildRunner
{
    Class groovyClass = null;

    public GroovyRunner(String buildFileName)
    {
	try
	{
	    GroovyClassLoader loader = new GroovyClassLoader();
	    this.groovyClass = loader.parseClass(new File(buildFileName));
	}
	catch (IOException e)
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
	    Script script = (Script) groovyClass.newInstance();
	    Binding binding = new Binding();
	    script.setBinding(binding);
	    binding.setVariable("buildDirectory", new File(dirName));
	    binding.setVariable("buildArgs", buildArgs);
	    binding.setVariable("antProject", antProject);
	    binding.setVariable("targets", targets);
	    binding.setVariable("defines", defines);
	    Object[] args = {};
	    Object resultObject = script.invokeMethod("run", args);
	    if (resultObject instanceof Boolean)
	    {
		status = (Boolean)resultObject;
	    }
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
