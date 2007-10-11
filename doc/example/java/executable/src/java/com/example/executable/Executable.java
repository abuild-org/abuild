package com.example.executable;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import com.example.library.Library;

public class Executable
{
    private void showTextFile()
    {
	try
	{
	    InputStream is = getClass().getClassLoader().getResourceAsStream(
		"com/example/file.txt");
	    if (is == null)
	    {
		System.err.println("can't find com/example/file.txt");
		System.exit(2);
	    }
	    BufferedReader r = new BufferedReader(new InputStreamReader(is));
	    String line;
	    while ((line = r.readLine()) != null)
	    {
		System.out.println(line);
	    }
	    r.close();
	}
	catch (IOException e)
	{
	    System.err.println(e.getMessage());
	}
    }

    public static void main(String[] args)
    {
	if (args.length != 1)
	{
	    System.err.println("Executable: one argument is required");
	    System.exit(2);
	}

	int value = 0;
	try
	{
	    Integer i = new Integer(args[0]);
	    value = i.intValue();
	}
	catch (NumberFormatException e)
	{
	    System.err.println("Executable: argument must be a number");
	    System.exit(2);
	}

	Library lib = new Library(value);
	System.out.println("The opposite of " + value +
			   " is " + lib.getOppose());


	new Executable().showTextFile();
    }
}
