package org.abuild.support;

import java.io.File;
import java.io.IOException;
import java.util.Vector;
import groovy.lang.Binding;
import groovy.lang.Script;
import groovy.lang.GroovyClassLoader;

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
	    // XXX
	}
    }

    public boolean runGroovy(String dirName,
			     Vector<String> targets,
			     Vector<String> defines)
    {
	boolean result = false;
	try
	{
	    Script script = (Script) groovyClass.newInstance();
	    Binding binding = new Binding();
	    script.setBinding(binding);
	    binding.setVariable("buildDirectory", new File(dirName));
	    binding.setVariable("targets", targets);
	    binding.setVariable("defines", defines);
	    Object[] args = {};
	    Object resultObject = script.invokeMethod("run", args);
	    if (resultObject instanceof Boolean)
	    {
		result = (Boolean)resultObject;
	    }
	}
	catch (InstantiationException e)
	{
	    // XXX
	}
	catch (IllegalAccessException e)
	{
	    // XXX
	}
	return result;
    }
}
