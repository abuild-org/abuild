package org.abuild.javabuilder;

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
    public static Project createAntProject(
	String dirName,
	Vector<String> otherArgs,
	Map<String, String> defines)
    {
	// Parse otherArgs.  These are passed internally from abuild,
	// not from the user.
	boolean keepGoing = false;
	boolean emacsMode = false;
	Integer messageOutputLevel = Project.MSG_INFO;
	Boolean noOp = false;
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
		return null;
	    }
	}

	File dir = new File(dirName);
	Project p = new Project();
	p.setKeepGoingMode(keepGoing);
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
	if (emacsMode)
	{
	    p.setUserProperty("abuild.private.emacs-mode", "1");
	}
	p.addBuildListener(logger);
	BuildException error = null;

	p.init();
	ProjectHelper helper = ProjectHelper.getProjectHelper();
	p.addReference(ProjectHelper.PROJECTHELPER_REFERENCE, helper);
	p.addReference("abuild.ant.noOp", noOp);
	p.addReference("abuild.ant.messageOutputLevel", messageOutputLevel);

	return p;
    }

    public boolean runAnt(String buildFileName, String dirName,
			  Vector<String> targets,
			  Vector<String> otherArgs,
			  Map<String, String> defines)
    {
	Project p = createAntProject(dirName, otherArgs, defines);
	if (p == null)
	{
	    return false;
	}

	Integer messageOutputLevel =
	    (Integer) p.getReference("abuild.ant.messageOutputLevel");

        if (messageOutputLevel >= Project.MSG_INFO)
	{
	    System.out.println("Buildfile: " + buildFileName);
        }
	Boolean noOp = (Boolean) p.getReference("abuild.ant.noOp");
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
	p.setUserProperty(MagicNames.ANT_FILE, buildFile.getAbsolutePath());
	BuildException error = null;
	try
	{
	    p.fireBuildStarted();
	    ProjectHelper helper =
		(ProjectHelper) p.getReference(
		    ProjectHelper.PROJECTHELPER_REFERENCE);
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
