package org.abuild.support;

import java.io.File;
import java.util.Vector;
import org.apache.tools.ant.MagicNames;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.ProjectHelper;
import org.apache.tools.ant.DefaultLogger;
import org.apache.tools.ant.BuildException;
import org.abuild.ant.AbuildLogger;

public class AntRunner
{
    // Abuild invocation of ant:
    //   buildfile is either Abuilt-ant.xml or TOP/ant/abuild.xml
    //   basedir is abuild-java
    //   logger is org.abuild.ant.AbuildLogger (unless turned off)
    //   targets

    // May have other args (-k, -v, etc.) -- see ant_args in
    // Abuild.cc and processArgs(String[] args) in
    // org/apache/tools/ant/Main.java

    // See getAntVersion in Main.java as well.

    public boolean runAnt(String buildFileName, String dirName,
			  Vector<String> targets)
    {
	System.out.println("Buildfile: " + buildFileName);
	File buildFile = new File(buildFileName);
	File dir = new File(dirName);
	Project p = new Project();
	p.setUserProperty(MagicNames.ANT_FILE, buildFile.getAbsolutePath());
	p.setUserProperty(MagicNames.PROJECT_BASEDIR, dir.getAbsolutePath());
	DefaultLogger consoleLogger = new AbuildLogger();
	consoleLogger.setErrorPrintStream(System.err);
	consoleLogger.setOutputPrintStream(System.out);
	consoleLogger.setMessageOutputLevel(Project.MSG_INFO);
	p.addBuildListener(consoleLogger);
	BuildException error = null;
	try
	{
	    p.fireBuildStarted();
	    p.init();
	    ProjectHelper helper = ProjectHelper.getProjectHelper();
	    p.addReference("ant.projectHelper", helper);
	    helper.parse(p, buildFile);
	    p.executeTargets(targets);
	}
	catch (BuildException e)
	{
	    error = e;
	}
	p.fireBuildFinished(error);
	return (error == null);
    }
}
