package org.abuild.javabuilder;

import java.util.List;

public class BuildArgs
{
    public boolean verbose = false;
    public boolean quiet = false;
    public boolean keepGoing = false;
    public boolean emacsMode = false;
    public boolean noOp = false;
    public boolean deprecationIsError = false;
    public boolean support1_0 = true;

    public boolean parseArgs(List<String> args)
    {
	boolean status = true;
	for (String arg: args)
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
		verbose = true;
	    }
	    else if (arg.equals("-q"))
	    {
		quiet = true;
	    }
	    else if (arg.equals("-n"))
	    {
		noOp = true;
	    }
	    else if (arg.equals("-de"))
	    {
		deprecationIsError = true;
	    }
	    else if (arg.equals("-cl1_1"))
	    {
		support1_0 = false;
	    }
	    else
	    {
		System.err.println(
		    "abuild: invalid argument passed to JavaBuilder: " + arg);
		status = false;
	    }
	}
	if (verbose && quiet)
	{
	    quiet = false;
	}
	return status;
    }
}
