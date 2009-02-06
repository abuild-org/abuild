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

    JavaBuilder(int port)
	throws IOException
    {
	SocketFactory factory = SocketFactory.getDefault();
	this.socket = (Socket) factory.createSocket("127.0.0.1", port);
	this.responseStream = new PrintStream(this.socket.getOutputStream());
    }

    public static void main(String[] args)
    {
	if (args.length == 0)
	{
	    usage();
	}
	int port = -1;
	if (args.length == 1)
	{
	    try
	    {
		port = Integer.parseInt(args[0]);
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
	    new JavaBuilder(port).run();
	}
	catch (IOException e)
	{
	    e.printStackTrace(System.err);
	    System.exit(2);
	}
    }

    private static void usage()
    {
	System.err.println("Usage: JavaBuilder port");
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
	if (args.length < 4)
	{
	    System.err.println(
		"JavaBuilder received ant command with fewer than 4 arguments");
	}
	else
	{
	    String build_file = args[1];
	    String dir = args[2];
	    Vector<String> targets = new Vector<String>();
	    for (int i = 3; i < args.length; ++i)
	    {
		targets.add(args[i]);
	    }
	    status = new AntRunner().runAnt(build_file, dir, targets);
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
	    // XXX handle command
	    String[] args = this.command.split(" ");
	    boolean status = false;
	    if (args.length == 0)
	    {
		System.err.println(
		    "JavaBuilder protocol error: received empty command");
	    }
	    else if (args[0].equals("ant"))
	    {
		status = JavaBuilder.this.runAnt(args);
	    }
	    else
	    {
		System.err.println(
		    "JavaBuilder: unknown command " + args[0]);
	    }

	    JavaBuilder.this.sendResponse(number, status);
	}
    }
}
