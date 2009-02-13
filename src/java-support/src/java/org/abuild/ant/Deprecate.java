package org.abuild.ant;

import org.apache.tools.ant.Project;
import org.apache.tools.ant.Task;
import org.apache.tools.ant.BuildException;

public class Deprecate extends Task
{
    private static boolean deprecationIsError = false;

    private String message = "";
    private String version;

    public static void setDeprecateIsError(boolean v)
    {
	deprecationIsError = v;
    }

    public void setVersion(String version)
    {
	this.version = version;
    }

    public void setMessage(String msg)
    {
	this.message = this.message + msg;
    }

    public void addText(String msg)
    {
	this.message = this.message + getProject().replaceProperties(msg);
    }

    public void execute() throws BuildException
    {
	if (this.version == null)
	{
	    throw new BuildException(
		"the deprecate task requires the version attribute",
		getLocation());
	}

	log("(abuild version " + this.version + "): " + message,
	    Project.MSG_WARN);
	// throw BuildException if we're in deprecation error mode
	if (deprecationIsError)
	{
	    throw new BuildException("deprecation error mode; failing",
				     getLocation());
	}
    }
}
