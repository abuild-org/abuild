package org.abuild.javabuilder;

import java.io.File;
import java.util.Vector;
import java.util.Map;
import org.apache.tools.ant.MagicNames;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.ProjectHelper;
import org.apache.tools.ant.BuildException;

public class AntRunner implements BuildRunner
{
    public boolean invokeBackend(
	String buildFileName, String dirName,
	BuildArgs buildArgs, Project antProject,
	Vector<String> targets, Map<String, String> defines)
    {
        if (! buildArgs.quiet)
	{
	    System.out.println("Buildfile: " + buildFileName);
        }

	File buildFile = new File(buildFileName);
	antProject.setUserProperty(
	    MagicNames.ANT_FILE, buildFile.getAbsolutePath());
	BuildException error = null;
	try
	{
	    antProject.fireBuildStarted();
	    ProjectHelper helper =
		(ProjectHelper) antProject.getReference(
		    ProjectHelper.PROJECTHELPER_REFERENCE);
	    helper.parse(antProject, buildFile);
	    antProject.executeTargets(targets);
	}
	catch (BuildException e)
	{
	    error = e;
	}
	antProject.fireBuildFinished(error);
	return (error == null);
    }
}
