package org.abuild.javabuilder;

import java.io.IOException;
import java.io.InputStreamReader;
import java.io.LineNumberReader;
import java.io.PrintStream;
import java.io.File;
import java.net.Socket;
import java.net.SocketException;
import javax.net.SocketFactory;
import java.util.Vector;
import java.util.Map;
import java.util.HashMap;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;
import org.abuild.ant.AbuildLogger;
import org.apache.tools.ant.MagicNames;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.ProjectHelper;
import org.apache.tools.ant.DefaultLogger;

class JavaBuilder
{
    private Socket socket;
    private Pattern response_re = Pattern.compile("(\\d+) (.*)");
    private Pattern defines_re = Pattern.compile("(\\d+) (.*)");
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
	String abuildTop = null;
	if (args.length == 2)
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
	try
	{
	    new JavaBuilder(abuildTop, port).run();
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

    private boolean handleInput(String line)
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

    private Map<String, String> parseDefines(String defines_str)
    {
	Map<String, String> defines = new HashMap<String, String>();

	String work_str = defines_str;
	Vector<String> work_vec = new Vector<String>();
	while (! "".equals(work_str))
	{
	    Matcher m = defines_re.matcher(work_str);
	    if (m.matches())
	    {
		String len_str = m.group(1);
		String rest = m.group(2);
		int len = 0;
		try
		{
		    len = Integer.parseInt(len_str);
		}
		catch (NumberFormatException e)
		{
		    System.err.println("invalid length in defines string");
		    return null;
		}
		String token = rest.substring(0, len);
		work_str = rest.substring(len);
		work_vec.add(token);
	    }
	    else
	    {
		System.err.println("invalid defines string");
		return null;
	    }
	}

	if (work_vec.size() % 2 != 0)
	{
	    System.err.println("odd number of strings in defines string");
	    return null;
	}

	for (int i = 0; i < work_vec.size(); i += 2)
	{
	    defines.put(work_vec.get(i), work_vec.get(i + 1));
	}

	return defines;
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
	p.addBuildListener(logger);

	p.init();
	ProjectHelper helper = ProjectHelper.getProjectHelper();
	p.addReference(ProjectHelper.PROJECTHELPER_REFERENCE, helper);

	return p;
    }

    private boolean callBackend(
	String backend, String buildFile, String dir,
	Vector<String> targets, Vector<String> otherArgs,
	Map<String, String> defines)
    {
	BuildArgs buildArgs = new BuildArgs();
	if (! buildArgs.parseArgs(otherArgs))
	{
	    return false;
	}

	if (buildArgs.noOp)
	{
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

	Project antProject = createAntProject(dir, buildArgs, defines);
	return runner.invokeBackend(
	    buildFile, dir, buildArgs, antProject, targets, defines);
    }

    private boolean runCommand(String[] args)
    {
	boolean status = false;
	if (args.length != 7)
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
	    String otherArgs_str = args[4];
	    String defines_str = args[5];
	    Vector<String> targets = new Vector<String>();
	    Vector<String> otherArgs = new Vector<String>();
	    for (String t: targets_str.split(" "))
	    {
		targets.add(t);
	    }
	    if (! "".equals(otherArgs_str))
	    {
		for (String t: otherArgs_str.split(" "))
		{
		    otherArgs.add(t);
		}
	    }
	    Map<String, String> defines = parseDefines(defines_str);
	    if (defines == null)
	    {
		// An error message has already been issued
	    }
	    else
	    {
		status = callBackend(
		    backend, buildFile, dir, targets, otherArgs, defines);
	    }
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
