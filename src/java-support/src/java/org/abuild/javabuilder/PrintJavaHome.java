package org.abuild.javabuilder;

import java.io.File;

public class PrintJavaHome
{
    public static void main(String[] args)
    {
	File javaHome = new File(System.getProperty("java.home"));
	if (javaHome.isDirectory())
	{
	    if (javaHome.getName().equals("jre") &&
		(new File(javaHome.getParent())).isDirectory())
	    {
		File candidate = new File(javaHome.getParent());
		File toolsJar =
		    new File(candidate.getPath() + "/lib/tools.jar");
		File javaBin =
		    new File(candidate.getPath() + "/bin/java");
		if (toolsJar.isFile() && javaBin.isFile())
		{
		    javaHome = candidate;
		}
	    }
	}
	System.out.println(javaHome.getPath());
    }
}
