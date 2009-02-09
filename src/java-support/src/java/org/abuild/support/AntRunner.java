package org.abuild.support;

import java.io.File;
import java.util.Vector;
import java.util.Map;
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
			  Vector<String> targets,
			  Vector<String> otherArgs,
			  Map<String, String> defines)
    {
	// Parse otherArgs.  These are passed internal from abuild,
	// not from the user.
	boolean keepGoing = false;
	boolean emacsMode = false;
	int messageOutputLevel = Project.MSG_INFO;
	boolean noOp = false;
	for (String arg: otherArgs)
	{
	    if (arg.equals("-e"))
	    {
                emacsMode = true;
            }
	    else if (arg.equals("-k"))
	    {
                keepGoing = true;
	    }
	    else if (arg.equals("-v"))
	    {
		messageOutputLevel = Project.MSG_VERBOSE;
	    }
	    else if (arg.equals("-q"))
	    {
                messageOutputLevel = Project.MSG_WARN;
	    }
	    else if (arg.equals("-n"))
	    {
		noOp = true;
	    }
	    else
	    {
		System.err.println(
		    "abuild: invalid arguments passed to runAnt: " + arg);
		return false;
	    }
	}

        if (messageOutputLevel >= Project.MSG_INFO)
	{
	    System.out.println("Buildfile: " + buildFileName);
        }
	if (noOp)
	{
	    System.out.print("would build targets:");
	    for (String t: targets)
	    {
		System.out.print(" " + t);
	    }
	    System.out.println("");
	    return true;
	}

	File buildFile = new File(buildFileName);
	File dir = new File(dirName);
	Project p = new Project();
	p.setKeepGoingMode(keepGoing);
	p.setUserProperty(MagicNames.ANT_FILE, buildFile.getAbsolutePath());
	p.setUserProperty(MagicNames.PROJECT_BASEDIR, dir.getAbsolutePath());
	for (Map.Entry<String, String> e: defines.entrySet())
	{
	    p.setUserProperty(e.getKey(), e.getValue());
	}
	DefaultLogger logger = new AbuildLogger();
	logger.setErrorPrintStream(System.err);
	logger.setOutputPrintStream(System.out);
	logger.setMessageOutputLevel(messageOutputLevel);
	logger.setEmacsMode(emacsMode);
	p.addBuildListener(logger);
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
