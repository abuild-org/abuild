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
    private PrintStream responseStream;
    private ExecutorService threadPool = Executors.newCachedThreadPool();
    private AntRunner antRunner = null;
    private GroovyRunner groovyRunner = null;
    private Map<String, String> defines;
    private BuildArgs buildArgs;

    JavaBuilder(String abuildTop, int port,
		BuildArgs buildArgs,
		Map<String, String> defines)
	throws IOException
    {
	this.buildArgs = buildArgs;
	this.defines = defines;
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
	    e.printStackTrace(System.err);
	    System.exit(2);
	}
    }

    private static void usage()
    {
	System.err.println("Usage: JavaBuilder abuild_top port args defines");
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
	// Synchronize writing responses, including a flush, in hopes
	// of avoiding sending interleaved responses.  Even with this,
	// interleaved responses seem to appear once in a blue moon.
	// The best fix would be to have a responder thread that reads
	// responses off of a therad-safe queue and sends them out
	// serially.
	responseStream.println(number + " " + (result ? "true" : "false"));
	responseStream.flush();
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
