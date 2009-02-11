package org.abuild;

import java.util.HashSet;
import java.util.Set;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.io.IOException;

public class QTC
{
    private static String filename = getFilename();
    private static String tc_scope = System.getenv("TC_SCOPE");
    private static Set<String> cache = new HashSet<String>();

    private static String getFilename()
    {
	String filename = System.getenv("TC_WIN_FILENAME");
	if (filename == null)
	{
	    filename = System.getenv("TC_FILENAME");
	}
	return filename;
    }

    public static void TC(String scope, String ccase)
    {
	TC(scope, ccase, 0);
    }

    synchronized public static void TC(String scope, String ccase, int n)
    {
	if (! tcActive(scope))
	{
	    return;
	}

	String key = ccase + " " + n;
	if (cache.contains(key))
	{
	    return;
	}
	cache.add(key);

	try
	{
	    PrintWriter tc = new PrintWriter(new FileWriter(filename, true));
	    tc.println(key);
	    tc.close();
	}
	catch (IOException e)
	{
	    // ignore
	}
    }

    static private boolean tcActive(String scope)
    {
	return ((filename != null) &&
		(tc_scope != null) &&
		(tc_scope.equals(scope)));
    }
}
