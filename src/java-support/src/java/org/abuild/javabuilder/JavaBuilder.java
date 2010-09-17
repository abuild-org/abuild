package org.abuild.javabuilder;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.io.PrintStream;
import java.io.File;
import java.net.Socket;
import java.net.SocketException;
import javax.net.SocketFactory;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ArrayBlockingQueue;
import org.abuild.ant.AbuildLogger;
import org.abuild.ant.Deprecate;
import org.apache.tools.ant.MagicNames;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.ProjectHelper;
import org.apache.tools.ant.DefaultLogger;
import org.abuild.QTC;

class JavaBuilder
{
    static private final Pattern response_re =
	Pattern.compile("(\\d+) (.*)");
    static private final Pattern defines_re =
	Pattern.compile("([^-][^=]*)=(.*)");
    private Socket socket;
    private ExecutorService threadPool =
	Executors.newCachedThreadPool(new AbuildThreadFactory());
    private AntRunner antRunner = null;
    private GroovyRunner groovyRunner = null;
    private Map<String, String> defines;
    private BuildArgs buildArgs;
    private Responder responder;

    JavaBuilder(String abuildTop, int port,
		BuildArgs buildArgs,
		Map<String, String> defines)
	throws IOException
    {
	this.buildArgs = buildArgs;
	this.defines = defines;
	SocketFactory factory = SocketFactory.getDefault();
	this.socket = factory.createSocket("127.0.0.1", port);
	this.antRunner = new AntRunner();
	this.groovyRunner = new GroovyRunner();
    }

    public static void main(String[] args)
    {
	int port = -1;
	String abuildTop = null;
	if (args.length >= 2)
	{
	    abuildTop = args[0];
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
	List<String> otherArgs = new ArrayList<String>();
	Map<String, String> defines = new HashMap<String, String>();
	for (int i = 2; i < args.length; ++i)
	{
	    String arg = args[i];
	    Matcher m = defines_re.matcher(arg);
	    if (arg.startsWith("-"))
	    {
		otherArgs.add(arg);
	    }
	    else if (m.matches())
	    {
		String key = m.group(1);
		String val = m.group(2);
		defines.put(key, val);
	    }
	}
	BuildArgs buildArgs = new BuildArgs();

	// Capture original stderr so we can write to it if there is a
	// serious problem that may interfere with our socket
	// communication back to the main abuild process.
	PrintStream stderr = System.err;

	if (! buildArgs.parseArgs(otherArgs))
	{
	    usage();
	}
	try
	{
	    new JavaBuilder(abuildTop, port, buildArgs, defines).run();
	}
	catch (IOException e)
	{
	    e.printStackTrace(stderr);
	    System.exit(2);
	}
	catch (InterruptedException e)
	{
	    e.printStackTrace(stderr);
	    System.exit(2);
	}
    }

    private static void usage()
    {
	System.err.println("Usage: JavaBuilder abuild_top port args defines");
	System.exit(2);
    }

    private boolean handleInput(String line)
	throws IOException, InterruptedException
    {
	boolean result = false;
	if (line.equals("shutdown"))
	{
	    this.threadPool.shutdownNow();
	    this.responder.shutdown();
	}
	else
	{
	    Matcher m = response_re.matcher(line);
	    if (m.matches())
	    {
		String number = m.group(1);
		String command = m.group(2);
		this.threadPool.execute(new InputHandler(number, command));
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
	try
	{
	    this.responder.sendResponse(number, result);
	}
	catch (InterruptedException e)
	{
	    e.printStackTrace(System.err);
	    System.exit(2);
	}
    }

    private void run()
	throws IOException, InterruptedException
    {
	// All testProtocol code is special case code used by abuild's
	// test suite to fully exercise the protocol between the Java
	// and C++ sides of JavaBuilder.

	this.responder = new Responder(this.socket);
	this.responder.testProtocol = this.buildArgs.testProtocol;
	if (this.buildArgs.captureOutput)
	{
	    if (this.buildArgs.testProtocol)
	    {
		System.out.println("rogue output from before redirection");
		System.out.flush();
		Thread.sleep(500);
		System.err.println("rogue error from before redirection");
	    }

	    OutputHandlerStream newOut =
		new OutputHandlerStream(false, responder);
	    OutputHandlerStream newErr =
		new OutputHandlerStream(true, responder);
	    System.setOut(new PrintStream(newOut, true));
	    System.setErr(new PrintStream(newErr, true));

	    if (this.buildArgs.testProtocol)
	    {
		System.out.println("rogue output from after redirection");
		System.out.flush();
		Thread.sleep(500);
		System.err.println("rogue error from after redirection");
	    }
	}
	LineNumberReader r =
	    new LineNumberReader(
		new InputStreamReader(this.socket.getInputStream()));
	String line = null;
	try
	{
	    while ((line = r.readLine()) != null)
	    {
		if (! handleInput(line))
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

    public static Project createAntProject(
	String dirName, BuildArgs buildArgs, Map<String, String> defines)
    {
	int messageOutputLevel = Project.MSG_INFO;
	if (buildArgs.verbose)
	{
	    messageOutputLevel = Project.MSG_VERBOSE;
	}
	else if (buildArgs.quiet)
	{
	    messageOutputLevel = Project.MSG_WARN;
	}

	File dir = new File(dirName);
	Project p = new Project();
	p.setDefaultInputStream(System.in);
	p.setKeepGoingMode(buildArgs.keepGoing);
	p.setUserProperty(MagicNames.PROJECT_BASEDIR, dir.getAbsolutePath());
	for (Map.Entry<String, String> e: defines.entrySet())
	{
	    p.setUserProperty(e.getKey(), e.getValue());
	}
	DefaultLogger logger = new AbuildLogger();
	logger.setErrorPrintStream(System.err);
	logger.setOutputPrintStream(System.out);
	logger.setMessageOutputLevel(messageOutputLevel);
	logger.setEmacsMode(buildArgs.emacsMode);
	if (buildArgs.emacsMode)
	{
	    p.setUserProperty("abuild.private.emacs-mode", "1");
	}
	if (buildArgs.deprecationIsError)
	{
	    p.setUserProperty("abuild.private.deprecate-is-error", "1");
	    Deprecate.setDeprecateIsError(true);
	}
	if (buildArgs.support1_0)
	{
	    p.setUserProperty("abuild.private.support-1_0", "1");
	}
	p.addBuildListener(logger);

	p.addTaskDefinition("deprecate", Deprecate.class);

	p.init();
	ProjectHelper helper = ProjectHelper.getProjectHelper();
	p.addReference(ProjectHelper.PROJECTHELPER_REFERENCE, helper);

	return p;
    }

    private boolean callBackend(
	String backend, String buildFile, String dir,
	List<String> targets)
    {
	if (this.buildArgs.noOp)
	{
	    QTC.TC("abuild", "JavaBuilder.java noOp");
	    System.out.print("JavaBuilder: would build targets:");
	    for (String t: targets)
	    {
		System.out.print(" " + t);
	    }
	    System.out.println("");
	    return true;
	}

	BuildRunner runner = null;
	if ("ant".equals(backend))
	{
	    runner = this.antRunner;
	}
	else if ("groovy".equals(backend))
	{
	    runner = this.groovyRunner;
	}
	else
	{
	    System.err.println(
		"JavaBuilder: unknown command " + backend);
	    return false;
	}

	Project antProject =
	    createAntProject(dir, this.buildArgs, this.defines);
	return runner.invokeBackend(
	    buildFile, dir, this.buildArgs, antProject, targets, this.defines);
    }

    private boolean runCommand(String[] args)
    {
	boolean status = false;
	if (args.length != 5)
	{
	    System.err.println(
		"JavaBuilder protocol error: received invalid command");
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
	else
	{
	    String backend = args[0];
	    String buildFile = args[1];
	    String dir = args[2];
	    String targets_str = args[3];
	    List<String> targets = new ArrayList<String>();
	    for (String t: targets_str.split(" "))
	    {
		targets.add(t);
	    }
	    status = callBackend(backend, buildFile, dir, targets);
	}

	return status;
    }

    class InputHandler implements Runnable
    {
	String number;
	String command;

	public InputHandler(String number, String command)
	{
	    this.number = number;
	    this.command = command;
	}

	public void run()
	{
	    String[] args = this.command.split("\001");
	    boolean status = false;

	    ThreadGroup g = Thread.currentThread().getThreadGroup();
	    if (! (g instanceof AbuildThreadGroup))
	    {
		throw new Error(
		    "InputHandler's ThreadGroup is not AbuildThreadGroup");
	    }
	    AbuildThreadGroup ag = (AbuildThreadGroup) g;

	    try
	    {
		ag.setJob(number);
		status = JavaBuilder.this.runCommand(args);
	    }
	    catch (Throwable e)
	    {
		System.err.println("build thread threw exception");
		e.printStackTrace(System.err);
	    }
	    JavaBuilder.this.sendResponse(number, status);
	    ag.setJob(null);
	}
    }

    class Responder implements Runnable, OutputHandler
    {
	private PrintStream responseStream;
	private BlockingQueue<String> responseQueue;
	private Thread thread;

	boolean testProtocol = false;

	public Responder(Socket s)
	    throws IOException
	{
	    this.responseStream = new PrintStream(s.getOutputStream());
	    this.responseQueue = new ArrayBlockingQueue<String>(1);
	    this.thread = new Thread(this);
	    this.thread.start();
	}

	public void sendResponse(String number, boolean result)
	    throws InterruptedException
	{
	    responseQueue.put(number + " " + (result ? "true" : "false"));
	}

	public void sendOutput(boolean is_error, String data)
	    throws InterruptedException
	{
	    String number = "0";

	    ThreadGroup g = Thread.currentThread().getThreadGroup();
	    if (g instanceof AbuildThreadGroup)
	    {
		String n = ((AbuildThreadGroup) g).getJob();
		if (n != null)
		{
		    number = n;
		}
	    }

	    if (data.length() > 0)
	    {
		responseQueue.put(
		    number + " data:" + (is_error ? "err" : "out") +
		    " " + data.getBytes().length + "\n" + data);
	    }
	}

	public void shutdown()
	    throws InterruptedException
	{
	    responseQueue.put("");
	    this.thread.join();
	}

	public void run()
	{
	    try
	    {
		while (true)
		{
		    String response = this.responseQueue.take();
		    if ("".equals(response))
		    {
			break;
		    }

		    // Special case code for test suite to fully
		    // exercise C++ half of protocol.  Send partial
		    // messages so we can always guarantee that we
		    // test the logic in JavaBuilder of handling
		    // partial messages.
		    if (this.testProtocol && (response.charAt(0) == '1'))
		    {
			this.testProtocol = false;
			responseStream.print(
			    "1 data:out 10\r\n" +
			    "12345");
			responseStream.flush();
			Thread.sleep(500);
			responseStream.print(
			    "6789\n\r\n" +
			    "0 data:out 6\n" +
			    "hello\n\n" +
			    "0 data:err 7\n" +
			    "potato\n\n");
			responseStream.flush();
		    }

		    responseStream.println(response);
		    responseStream.flush();
		}
	    }
	    catch (InterruptedException e)
	    {
		e.printStackTrace(System.err);
		System.exit(2);
	    }
	}
    }
}
