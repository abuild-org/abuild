package com.example.executable.entry;

import com.example.consumers.ProcessorInterface;
import com.example.consumers.Consumer;
import com.example.consumers.ConsumerTable;
import com.example.consumers.c1.C1;
import com.example.consumers.c2.C2;

public class Entry
{
    static
    {
	new C1().register();
	new C2().register();
    }

    public static void runExecutable(ProcessorInterface processor,
				     String args[])
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

	for (Consumer c: ConsumerTable.getConsumers())
	{
	    c.consume(processor, n);
	}
    }
}
