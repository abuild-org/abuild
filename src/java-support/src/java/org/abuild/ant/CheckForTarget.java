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
import org.apache.tools.ant.Task;
import org.apache.tools.ant.BuildException;

/**
 * Implements an Ant Task that determines whether a particular build
 * file contains a given target.  It does by creating a sub-ant and,
 * rather than calling the target, just checking to see whether it is
 * there.
 */
public class CheckForTarget extends Task
{
    private final static Map<String, Set<String> > targetsByFilename =
	new HashMap<String, Set<String>>();

    private File antFile = null;
    public void setAntfile(File antFile)
    {
        this.antFile = antFile;
    }

    private String target = null;
    public void setTarget(String target)
    {
        if ("".equals(target))
	{
            throw new BuildException("target attribute must not be empty");
        }
        this.target = target;
    }

    private String property = null;
    public void setProperty(String property)
    {
        if ("".equals(property))
	{
            throw new BuildException("property attribute must not be empty");
        }
        this.property = property;
    }

    // This method provides a wrapper around the one unavoidable
    // unchecked cast when working with the Ant APIs.
    @SuppressWarnings("unchecked")
    private Map<String, Target> getProjectTargets(Project project)
    {
	return project.getTargets();
    }

    public void execute() throws BuildException
    {
        try
	{
	    synchronized(targetsByFilename)
	    {
		Set<String> targets = targetsByFilename.get(
		    this.antFile.getAbsolutePath());
		if (targets == null)
		{
		    // Create a new project and copy all properties
		    // into it.  Copy user properties first and then
		    // all other properties.
		    Project newProject = getProject().createSubProject();
		    newProject.setJavaVersionProperty();
		    getProject().copyUserProperties(newProject);

		    @SuppressWarnings("unchecked")
		    Enumeration e = getProject().getProperties().keys();
		    while (e.hasMoreElements())
		    {
			String key = e.nextElement().toString();
			String value =
			    getProject().getProperties().get(key).toString();
			if (newProject.getProperty(key) == null)
			{
			    newProject.setNewProperty(key, value);
			}
		    }

		    // Load the antFile
		    ProjectHelper.configureProject(newProject, this.antFile);

		    Map<String, Target> projectTargets =
			getProjectTargets(newProject);
		    targets = new HashSet<String>(projectTargets.size());
		    targets.addAll(projectTargets.keySet());
		    targetsByFilename.put(
			this.antFile.getAbsolutePath(), targets);
		}

		if (targets.contains(this.target))
		{
		    getProject().setNewProperty(this.property, "true");
		}
	    }
	}
	catch (BuildException ex)
	{
	    throw ProjectHelper.addLocationToBuildException(
		ex, getLocation());
        }
    }
}
