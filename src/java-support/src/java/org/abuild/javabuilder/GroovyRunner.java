package org.abuild.javabuilder;

import java.io.File;
import java.io.IOException;
import java.util.Vector;
import java.util.Map;
import groovy.lang.Binding;
import groovy.lang.Script;
import groovy.lang.GroovyClassLoader;
import org.apache.tools.ant.Project;

public class GroovyRunner
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

    public boolean runGroovy(String dirName,
			     Vector<String> targets,
			     Vector<String> groovyArgs,
			     Map<String, String> defines)
    {
	Project project = AntRunner.createAntProject(
	    dirName, groovyArgs, defines);
	boolean result = false;
	Exception exc = null;
	try
	{
	    Script script = (Script) groovyClass.newInstance();
	    Binding binding = new Binding();
	    script.setBinding(binding);
	    binding.setVariable("buildDirectory", new File(dirName));
	    binding.setVariable("targets", targets);
	    binding.setVariable("groovyArgs", groovyArgs);
	    binding.setVariable("defines", defines);
	    binding.setVariable("antProject", project);
	    Object[] args = {};
	    Object resultObject = script.invokeMethod("run", args);
	    if (resultObject instanceof Boolean)
	    {
		result = (Boolean)resultObject;
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
	return result;
    }
}
