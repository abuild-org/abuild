#include <ProcessHandler.hh>

#ifdef _WIN32
# include <windows.h>
# include <io.h>
#else
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>
# include <sys/select.h>
# include <sys/file.h>
# include <sys/wait.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <set>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <Util.hh>
#include <QEXC.hh>

ProcessHandler* ProcessHandler::the_instance = 0;

ProcessHandler::ProcessHandler(char* env[]) :
    env(env)
{
}

void
ProcessHandler::createInstance(char* env[])
{
    if (the_instance)
    {
	throw QEXC::Internal(
	    "ProcessHandler::createInstance called more than once");
    }
    the_instance = new ProcessHandler(env);
}

ProcessHandler&
ProcessHandler::getInstance()
{
    if (! the_instance)
    {
	throw QEXC::Internal(
	    "ProcessHandler::getInstance called before createInstance");
    }
    return *the_instance;
}

#ifdef _WIN32

// On Windows, if we run a child process that shares the console with
// this program and if that child process is a batch file, we will run
// into trouble if the user hits CTRL-C.  Our process will exit
// immediately, as will any other processes that don't do anything
// special with CTRL-C.  However, any batch files, which were run with
// cmd /c, will prompt the user with "Terminate batch job? (Y/N)".
// There is no way to turn this behavior off.  If we exit before they
// do, the user will not have any way of answering those prompts, and
// his/her console will be hosed.  To avoid this, we keep track of
// child processes that we are running and wait for all of them to
// exit before exiting ourselves if the user has hit CTRL-C.

// To achieve this, we keep a mutex-protected set of running process
// information pointers.  Whenever a process is created or destroyed,
// it must be done while holding the mutex.
static boost::mutex running_process_mutex;
static std::set<PROCESS_INFORMATION*> running_processes;

static bool
cleanProcess(PROCESS_INFORMATION* pi)
{
    // This function must be called with the running_process_mutex
    // locked.  It actually waits for the process to exit and then
    // cleans up after it.  It returns true if the program exited
    // normally.

    DWORD exit_status;
    GetExitCodeProcess(pi->hProcess, &exit_status);
    CloseHandle(pi->hProcess);
    CloseHandle(pi->hThread);
    return (exit_status == 0);
}

static void
waitForProcessesAndExit()
{
    // Wait for all processes to exit, and then force the process to
    // exit.
    boost::mutex::scoped_lock lock(running_process_mutex);
    while (! running_processes.empty())
    {
	PROCESS_INFORMATION* pi = *(running_processes.begin());
	running_processes.erase(pi);
	WaitForSingleObject(pi->hProcess, INFINITE);
	cleanProcess(pi);
    }
    ExitProcess(2);
}

static BOOL
ctrlHandler(DWORD ctrl_type)
{
    // Trap windows CTRL-C event

    switch (ctrl_type)
    {
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_BREAK_EVENT:
	  waitForProcessesAndExit();
	  return TRUE;

        default:
	  return FALSE;
    }
}

static void read_pipe(ProcessHandler::output_handler_t output_handler,
		      bool is_error, HANDLE pipe)
{
    char buf[1024];
    DWORD len;
    while (pipe)
    {
	if (! ReadFile(pipe, buf, sizeof(buf), &len, NULL))
	{
	    if (GetLastError() == ERROR_BROKEN_PIPE)
	    {
		len = 0;
	    }
	    else
	    {
		throw QEXC::General("failure reading from pipe: " +
				    Util::windowsErrorString());
	    }
	}
	{ // private scope
	    boost::mutex::scoped_lock lock(mutex);
	    output_handler(is_error, buf, (int) len);
	}
	if (len == 0)
	{
	    CloseHandle(pipe);
	    pipe = NULL;
	}
    }
}

#else // _WIN32

static void add_to_read_set(fd_set& read_set, int& nfds, int fd)
{
    if (fd != -1)
    {
	FD_SET(fd, &read_set);
	nfds = std::max(nfds, 1 + fd);
    }
}

static void handle_output(fd_set& read_set, int& fd,
			  std::string const& description,
			  ProcessHandler::output_handler_t handler,
			  bool is_error)
{
    if (fd == -1)
    {
	return;
    }
    char buf[1024];
    int len = 0;
    if (FD_ISSET(fd, &read_set))
    {
	len = QEXC::errno_wrapper("read " + description,
				  read(fd, buf, sizeof(buf)));
	handler(is_error, buf, len);
	if (len == 0)
	{
	    close(fd);
	    fd = -1;
	}
    }
}

#endif // _WIN32

bool
ProcessHandler::runProgram(
    std::string const& progname,
    std::vector<std::string> const& args,
    std::map<std::string, std::string> const& environment,
    bool preserve_env,
    std::string const& dir,
    output_handler_t output_handler)
{
    bool status = true;

#ifdef _WIN32
    static bool installed_ctrl_handler = false;

    { // private scope
	boost::mutex::scoped_lock lock(running_process_mutex);
	if (! installed_ctrl_handler)
	{
	    if (! SetConsoleCtrlHandler((PHANDLER_ROUTINE) ctrlHandler, TRUE))
	    {
		throw QEXC::General("could not set control handler: " +
				    Util::windowsErrorString());
	    }
	    installed_ctrl_handler = true;
	}
    }

    // Pipe handling code is mostly taken from
    // http://msdn.microsoft.com/en-us/library/ms682499%28VS.85%29.aspx

    HANDLE child_in = NULL;
    HANDLE child_out_r = NULL;
    HANDLE child_out_w = NULL;
    HANDLE child_err_r = NULL;
    HANDLE child_err_w = NULL;

    if (output_handler)
    {
	// Create inheritable pipes
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	if (! (CreatePipe(&child_out_r, &child_out_w, &saAttr, 0) &&
	       CreatePipe(&child_err_r, &child_err_w, &saAttr, 0)))
	{
	    throw QEXC::General("CreatePipe failed for child I/O: " +
				Util::windowsErrorString());
	}
	child_in = CreateFile("NUL", GENERIC_READ, FILE_SHARE_WRITE, &saAttr,
			      OPEN_EXISTING, FILE_ATTRIBUTE_READONLY,
			      NULL);
	if (child_in == INVALID_HANDLE_VALUE)
	{
	    throw QEXC::General("unable to open NUL: " +
				Util::windowsErrorString());
	}
    }

    std::string progpath = progname;
    Util::appendExe(progpath);
    std::string suffix = Util::getExtension(progpath);

    std::string cmdline_str;
    bool first = true;
    for (std::vector<std::string>::const_iterator argp = args.begin();
	 argp != args.end(); ++argp)
    {
	if (first)
	{
	    first = false;
	}
	else
	{
	    cmdline_str += " ";
	}
	for (std::string::const_iterator ch = (*argp).begin();
	     ch != (*argp).end(); ++ch)
	{
	    if (*ch == ' ')
	    {
		cmdline_str += "\" \"";
	    }
	    else if (*ch == '"')
	    {
		cmdline_str += "\\\"";
	    }
	    else
	    {
		cmdline_str.append(1, *ch);
	    }
	}
    }

    std::string env_str;
    if ((! environment.empty()) || preserve_env)
    {
	for (std::map<std::string, std::string>::const_iterator iter =
		 environment.begin();
	     iter != environment.end(); ++iter)
	{
	    env_str += (*iter).first + "=" + (*iter).second;
	    env_str.append(1, '\0');
	}

	if (preserve_env)
	{
	    for (char** envp = this->env; *envp; ++envp)
	    {
		env_str += *envp;
		env_str.append(1, '\0');
	    }
	}
    }
    else
    {
	env_str.append(1, '\0');
    }
    env_str.append(1, '\0');

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    if (output_handler)
    {
	si.hStdError = child_err_w;
	si.hStdOutput = child_out_w;
	si.hStdInput = child_in;
	si.dwFlags |= STARTF_USESTDHANDLES;
    }
    ZeroMemory(&pi, sizeof(pi));

    std::string comspec;
    std::string appname = progpath;
    if ((suffix == "bat") && Util::getEnv("COMSPEC", &comspec))
    {
	cmdline_str = comspec + " /c " + cmdline_str;
	appname = comspec;
    }

    char* cmdline = new char[cmdline_str.length() + 1];
    strcpy(cmdline, cmdline_str.c_str());
    char* env = new char[env_str.length()];
    memcpy(env, env_str.c_str(), env_str.length());

    BOOL result = false;

    { // private scope
	boost::mutex::scoped_lock lock(running_process_mutex);

	result =
	    CreateProcess(appname.c_str(), // LPCTSTR lpApplicationName,
			  cmdline,	   // LPTSTR lpCommandLine,
			  NULL,	// LPSECURITY_ATTRIBUTES lpProcessAttributes,
			  NULL,	// LPSECURITY_ATTRIBUTES lpThreadAttributes,
			  TRUE,	// BOOL bInheritHandles,
			  0,	// DWORD dwCreationFlags,
			  env,	// LPVOID lpEnvironment,
			  dir.c_str(), // LPCTSTR lpCurrentDirectory,
			  &si,	// LPSTARTUPINFO lpStartupInfo,
			  &pi);	// LPPROCESS_INFORMATION lpProcessInformation);

	delete [] cmdline;
	delete [] env;

	if (! result)
	{
	    return false;
	}

	running_processes.insert(&pi);
    }

    if (output_handler)
    {
	// Close child side of pipes
	CloseHandle(child_out_w);
	CloseHandle(child_err_w);
	CloseHandle(child_in);

	// Read stdout and stderr in separate threads.  Considerable
	// research suggests that there's no way to do the equivalent
	// of a blocking select on anonymous pipes.
	// WaitForSingleObject and WaitForMultipleObjects show the
	// pipe to be signaled as long the pipe is open.
	boost::thread error_th(
	    boost::bind(read_pipe, output_handler, true, child_err_r));
	read_pipe(output_handler, false, child_out_r);
	error_th.join();
    }
    WaitForSingleObject(pi.hProcess, INFINITE);

    { // private scope
	boost::mutex::scoped_lock lock(running_process_mutex);
	running_processes.erase(&pi);
	status = cleanProcess(&pi);
    }
#else
    int output_pipe[2];
    int error_pipe[2];
    output_pipe[0] = -1;
    output_pipe[1] = -1;
    error_pipe[0] = -1;
    error_pipe[1] = -1;
    if (output_handler)
    {
	QEXC::errno_wrapper("create output pipe", pipe(output_pipe));
	QEXC::errno_wrapper("create error pipe", pipe(error_pipe));
    }

    int pid = fork();
    if (pid == -1)
    {
	return false;
    }
    if (pid == 0)
    {
	if (chdir(dir.c_str()) == -1)
	{
	    _exit(1);
	}

	int stdin_fd = -1;
	if (output_handler)
	{
	    close(output_pipe[0]);
	    close(error_pipe[0]);
	    stdin_fd = QEXC::errno_wrapper("open /dev/null",
					   open("/dev/null", O_RDONLY));
	}

	int nvars = environment.size();
	if (preserve_env)
	{
	    for (char** envp = this->env; *envp; ++envp)
	    {
		++nvars;
	    }
	}

	char** env = new char*[nvars + 1];
	char** envp = env;
	for (std::map<std::string, std::string>::const_iterator iter =
		 environment.begin();
	     iter != environment.end(); ++iter)
	{
	    std::string v = (*iter).first + "=" + (*iter).second;
	    char* vp = new char[v.length() + 1];
	    strcpy(vp, v.c_str());
	    *envp++ = vp;
	}
	if (preserve_env)
	{
	    for (char** oenvp = this->env; *oenvp; ++oenvp)
	    {
		*envp++ = *oenvp;
	    }
	}
	*envp = 0;

	char** argv = new char*[args.size() + 1];
	char** argp = argv;
	for (std::vector<std::string>::const_iterator iter = args.begin();
	     iter != args.end(); ++iter)
	{
	    char* vp = new char[(*iter).length() + 1];
	    strcpy(vp, (*iter).c_str());
	    *argp++ = vp;
	}
	*argp = 0;

	if (output_handler)
	{
	    dup2(stdin_fd, 0);
	    dup2(output_pipe[1], 1);
	    dup2(error_pipe[1], 2);
	}

	execve(progname.c_str(), argv, env);
	_exit(1);
    }
    else
    {
	int exit_status;
	if (output_handler)
	{
	    close(output_pipe[1]);
	    close(error_pipe[1]);
	    int child_out = output_pipe[0];
	    int child_err = error_pipe[0];
	    while (! ((child_out == -1) && (child_err == -1)))
	    {
		fd_set read_set;
		int nfds = 0;
		FD_ZERO(&read_set);
		add_to_read_set(read_set, nfds, child_out);
		add_to_read_set(read_set, nfds, child_err);
		switch(select(nfds, &read_set, 0, 0, 0))
		{
		  case 0:
		    // timeout
		    throw QEXC::Internal("select timed out with timeout = 0");
		    break;

		  case -1:
		    // ignore interrupted system calls; just restart
		    if (errno != EINTR)
		    {
			throw QEXC::System("select failed", errno);
		    }
		    break;

		  default:
		    handle_output(read_set, child_out, "child's stdout",
				  output_handler, false);
		    handle_output(read_set, child_err, "child's stderr",
				  output_handler, true);
		}
	    }
	}
	if (waitpid(pid, &exit_status, 0) != pid)
	{
	    return false;
	}
	status = (exit_status == 0);
    }
#endif

    return status;
}
