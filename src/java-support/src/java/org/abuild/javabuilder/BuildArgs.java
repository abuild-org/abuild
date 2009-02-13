package org.abuild.javabuilder;

import java.util.Vector;

public class BuildArgs
{
    public boolean verbose = false;
    public boolean quiet = false;
    public boolean keepGoing = false;
    public boolean emacsMode = false;
    public boolean noOp = false;
    public boolean deprecationIsError = false;

    public boolean parseArgs(Vector<String> args)
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
