package org.abuild.support;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.io.PrintStream;
import java.net.Socket;
import java.net.SocketException;
import javax.net.SocketFactory;
import java.util.Vector;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;

class JavaBuilder
{
    private Socket socket;
    private Pattern response_re = Pattern.compile("(\\d+) (.*)");
    private PrintStream responseStream;
    private ExecutorService threadPool = Executors.newCachedThreadPool();
    private AntRunner antRunner = null;
    private GroovyRunner groovyRunner = null;

    JavaBuilder(String abuildTop, int port)
	throws IOException
    {
	SocketFactory factory = SocketFactory.getDefault();
	this.socket = factory.createSocket("127.0.0.1", port);
	this.responseStream = new PrintStream(this.socket.getOutputStream());
	this.antRunner = new AntRunner();
	this.groovyRunner = new GroovyRunner(
	    abuildTop + "/groovy/AbuildMain.groovy");
    }

    public static void main(String[] args)
    {
	int port = -1;
	String abuild_top = null;
	if (args.length == 2)
	{
	    abuild_top = args[0];
	    try
	    {
		port = Integer.parseInt(args[1]);
	    }
	    catch (NumberFormatException e)
	    {
		// ignore
	    }
	}
	if (port == -1)
	{
	    usage();
	}
	try
	{
	    new JavaBuilder(abuild_top, port).run();
	}
	catch (IOException e)
	{
	    e.printStackTrace(System.err);
	    System.exit(2);
	}
    }

    private static void usage()
    {
	System.err.println("Usage: JavaBuilder abuild_top port");
	System.exit(2);
    }

    private boolean handle(String line)
	throws IOException
    {
	boolean result = false;
	if (line.equals("shutdown"))
	{
	    this.threadPool.shutdownNow();
	}
	else
	{
	    Matcher m = response_re.matcher(line);
	    if (m.matches())
	    {
		String number = m.group(1);
		String command = m.group(2);
		this.threadPool.execute(new Handler(number, command));
		result = true;
	    }
	    else
	    {
		System.err.println("Protocol error: received " + line);
	    }
	}

	return result;
    }

    private synchronized void sendResponse(String number, boolean result)
    {
	responseStream.println(number + " " + (result ? "true" : "false"));
    }

    private void run()
	throws IOException
    {
	LineNumberReader r =
	    new LineNumberReader(
		new InputStreamReader(this.socket.getInputStream()));
	String line = null;
	try
	{
	    while ((line = r.readLine()) != null)
	    {
		if (! handle(line))
		{
		    break;
		}
	    }
	    this.socket.close();
	}
	catch (SocketException e)
	{
	    // ignore and return
	}
    }

    boolean runAnt(String[] args)
    {
	boolean status = false;
	if (args.length != 6)
	{
	    System.err.println(
		"JavaBuilder received ant command with other than 6 arguments");
	    for (String arg: args)
	    {
		System.err.println("  " + arg);
	    }
	}
	else
	{
	    String build_file = args[1];
	    String dir = args[2];
	    String targets_str = args[3];
	    String other_args_str = args[4];
	    Vector<String> targets = new Vector<String>();
	    Vector<String> other_args = new Vector<String>();
	    for (String t: targets_str.split(" "))
	    {
		targets.add(t);
	    }
	    for (String t: other_args_str.split(" "))
	    {
		other_args.add(t);
	    }
	    status = this.antRunner.runAnt(
		build_file, dir, targets, other_args);
	}

	return status;
    }

    boolean runGroovy(String[] args)
    {
	boolean status = false;
	if (args.length != 5)
	{
	    System.err.println(
		"JavaBuilder received groovy" +
		" command with other than 5 arguments");
	    for (String arg: args)
	    {
		System.err.println("  " + arg);
	    }
	}
	else
	{
	    String dir = args[1];
	    String targets_str = args[2];
	    String defines_str = args[3];
	    Vector<String> targets = new Vector<String>();
	    Vector<String> defines = new Vector<String>();
	    for (String t: targets_str.split(" "))
	    {
		targets.add(t);
	    }
	    for (String t: defines_str.split(" "))
	    {
		defines.add(t);
	    }
	    status = this.groovyRunner.runGroovy(dir, targets, defines);
	}

	return status;
    }

    boolean runCommand(String[] args)
    {
	boolean status = false;
	if (args.length == 0)
	{
	    System.err.println(
		"JavaBuilder protocol error: received empty command");
	}
	else if (! args[args.length - 1].equals("|"))
	{
	    System.err.println(
		"JavaBuilder protocol error: command did not end with |");
	    for (String arg: args)
	    {
		System.err.println("  " + arg);
	    }
	}
	else if (args[0].equals("ant"))
	{
	    status = runAnt(args);
	}
	else if (args[0].equals("groovy"))
	{
	    status = runGroovy(args);
	}
	else
	{
	    System.err.println(
		"JavaBuilder: unknown command " + args[0]);
	}

	return status;
    }

    class Handler implements Runnable
    {
	String number;
	String command;

	public Handler(String number, String command)
	{
	    this.number = number;
	    this.command = command;
	}

	public void run()
	{
	    String[] args = this.command.split("\001");
	    boolean status = false;
	    try
	    {
		status = JavaBuilder.this.runCommand(args);
	    }
	    catch (Throwable e)
	    {
		System.err.println("build thread threw exception");
		e.printStackTrace(System.err);
	    }
	    JavaBuilder.this.sendResponse(number, status);
	}
    }
}
