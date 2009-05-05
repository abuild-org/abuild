package com.example.codeGenerator;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import org.apache.tools.ant.Task;
import org.apache.tools.ant.BuildException;

public class ExampleTask extends Task
{
    private File sourceDir;
    public void setSourceDir(File dir)
    {
	this.sourceDir = dir;
    }

    private String fullClassName;
    public void setClassName(String name)
    {
	this.fullClassName = name;
    }

    public void execute() throws BuildException
    {
	if (this.sourceDir == null)
	{
	    throw new BuildException("no sourcedir specified");
	}
	if (this.fullClassName == null)
	{
	    throw new BuildException("no fullclassname specified");
	}
	Pattern fullClassName_re = Pattern.compile(
	    "^((?:[a-zA-Z0-9_]+\\.)*)([a-zA-Z0-9_]+)$");

	Matcher m = fullClassName_re.matcher(this.fullClassName);
	if (! m.matches())
	{
	    throw new BuildException("invalid fullclassname");
	}

	String packageName = m.group(1);
	String className = m.group(2);

	if (packageName.length() > 0)
	{
	    packageName = packageName.substring(0, packageName.length() - 1);
	}

	File outputFile = new File(
	    this.sourceDir.getAbsolutePath() + "/" +
	    this.fullClassName.replace('.', '/') + ".java");
	File parentDir = outputFile.getParentFile();

	if (parentDir.isDirectory())
	{
	    // okay
	}
	else if (! parentDir.mkdirs())
	{
	    throw new BuildException("unable to create directory " +
				     parentDir.getAbsolutePath());
	}

	if (! outputFile.isFile())
	{
	    try
	    {
		FileWriter w = new FileWriter(outputFile);
		if (packageName.length() > 0)
		{
		    w.write("package " + packageName + ";\n");
		}
		w.write("public class " + className + "\n{\n");
		w.write("    public int negate(int n)\n    {\n");
		w.write("        return -n;\n    }\n");
		w.write("}\n");
		w.close();
	    }
	    catch (IOException e)
	    {
		throw new BuildException("IOException: " + e.getMessage());
	    }

	    log("created " + outputFile.getAbsolutePath());
	}
    }
}
