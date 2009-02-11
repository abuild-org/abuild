package org.abuild.ant;

import java.util.Enumeration;
import java.util.Set;
import java.util.HashSet;
import java.util.Map;
import java.util.HashMap;
import java.io.File;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.Target;
import org.apache.tools.ant.ProjectHelper;

/**
 * This code determines whether a particular build file contains a
 * given target.  It does by creating a sub-ant and, rather than
 * calling the target, just checking to see whether it is there.
 */
public class CheckForTarget
{
    private final static Map<String, Set<String> > targetsByFilename =
	new HashMap<String, Set<String>>();

    // This method provides a wrapper around the one unavoidable
    // unchecked cast when working with the Ant APIs.
    @SuppressWarnings("unchecked")
    private static Map<String, Target> getProjectTargets(Project project)
    {
	return project.getTargets();
    }

    public static boolean hasTarget(
	Project project, File antFile, String target)
    {
	boolean result = false;
	synchronized(targetsByFilename)
	{
	    Set<String> targets = targetsByFilename.get(
		antFile.getAbsolutePath());
	    if (targets == null)
	    {
		// Create a new project and copy all properties
		// into it.  Copy user properties first and then
		// all other properties.
		Project newProject = project.createSubProject();
		newProject.setJavaVersionProperty();
		project.copyUserProperties(newProject);

		@SuppressWarnings("unchecked")
		    Enumeration e = project.getProperties().keys();
		while (e.hasMoreElements())
		{
		    String key = e.nextElement().toString();
		    String value =
			project.getProperties().get(key).toString();
		    if (newProject.getProperty(key) == null)
		    {
			newProject.setNewProperty(key, value);
		    }
		}

		// Load the antFile
		ProjectHelper.configureProject(newProject, antFile);

		Map<String, Target> projectTargets =
		    getProjectTargets(newProject);
		targets = new HashSet<String>(projectTargets.size());
		targets.addAll(projectTargets.keySet());
		targetsByFilename.put(
		    antFile.getAbsolutePath(), targets);
	    }

	    if (targets.contains(target))
	    {
		result = true;
	    }
	}
	return result;
    }
}
