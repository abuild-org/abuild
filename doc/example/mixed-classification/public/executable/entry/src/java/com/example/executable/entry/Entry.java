package com.example.executable.entry;

import java.util.Vector;
import java.util.Iterator;
import com.example.handlers.HelperInterface;
import com.example.handlers.Handler;
import com.example.handlers.HandlerTable;
import com.example.handlers.h1.H1;
import com.example.handlers.h2.H2;

public class Entry
{
    static
    {
	new H1().register();
	new H2().register();
    }

    public static void runExecutable(HelperInterface helper, String args[])
    {
	int n = 0;
	if (args.length > 0)
	{
	    try
	    {
		n = Integer.parseInt(args[0]);
	    }
	    catch (NumberFormatException e)
	    {
		System.err.println("bad number " + args[0]);
		System.exit(2);
	    }
	}

	Iterator<Handler> iter = HandlerTable.getHandlers().iterator();
	while (iter.hasNext())
	{
	    Handler h = iter.next();
	    h.handle(helper, n);
	}
    }
};
