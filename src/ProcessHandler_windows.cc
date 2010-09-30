#include <ProcessHandler_windows.hh>

#include <string.h>
#include <stdlib.h>
#include <set>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <Util.hh>
#include <QEXC.hh>

// If we run a child process that shares the console with this program
// and if that child process is a batch file, we will run into trouble
// if the user hits CTRL-C.  Our process will exit immediately, as
// will any other processes that don't do anything special with
// CTRL-C.  However, any batch files, which were run with cmd /c, will
// prompt the user with "Terminate batch job? (Y/N)".  There is no way
// to turn this behavior off.  If we exit before they do, the user
// will not have any way of answering those prompts, and his/her
// console will be hosed.  To avoid this, we keep track of child
// processes that we are running and wait for all of them to exit
// before exiting ourselves if the user has hit CTRL-C.

boost::mutex ProcessHandler_windows::running_processes_mutex;
std::set<PROCESS_INFORMATION*> ProcessHandler_windows::running_processes;

ProcessHandler_windows::ProcessHandler_windows(char* env[]) :
    ProcessHandler(env)
{
}

ProcessHandler_windows::~ProcessHandler_windows()
{
}

bool
ProcessHandler_windows::cleanProcess(PROCESS_INFORMATION* pi)
{
    // This function must be called with running_processes_mutex
    // locked.  It actually waits for the process to exit and then
    // cleans up after it.  It returns true if the program exited
    // normally.

    DWORD exit_status;
    GetExitCodeProcess(pi->hProcess, &exit_status);
    CloseHandle(pi->hProcess);
    CloseHandle(pi->hThread);
    return (exit_status == 0);
}

void
ProcessHandler_windows::waitForProcessesAndExit()
{
    // Wait for all processes to exit, and then force the process to
    // exit.
    boost::mutex::scoped_lock lock(running_processes_mutex);
    while (! running_processes.empty())
    {
	PROCESS_INFORMATION* pi = *(running_processes.begin());
	running_processes.erase(pi);
	WaitForSingleObject(pi->hProcess, INFINITE);
	cleanProcess(pi);
    }
    ExitProcess(2);
}

BOOL
ProcessHandler_windows::ctrlHandler(DWORD ctrl_type)
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

void
ProcessHandler_windows::readPipe(
    ProcessHandler::output_handler_t output_handler,
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
	    boost::mutex::scoped_lock lock(this->output_mutex);
	    output_handler(is_error, buf, (int) len);
	}
	if (len == 0)
	{
	    CloseHandle(pipe);
	    pipe = NULL;
	}
    }
}


bool
ProcessHandler_windows::runProgram(
    std::string const& progname,
    std::vector<std::string> const& args,
    std::map<std::string, std::string> const& environment,
    bool preserve_env,
    std::string const& dir,
    output_handler_t output_handler)
{
    bool status = true;

    static bool installed_ctrl_handler = false;

    { // private scope
	boost::mutex::scoped_lock lock(running_processes_mutex);
	if (! installed_ctrl_handler)
	{
	    if (! SetConsoleCtrlHandler(
		    (PHANDLER_ROUTINE) ProcessHandler_windows::ctrlHandler,
		    TRUE))
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
	boost::mutex::scoped_lock lock(running_processes_mutex);

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
	    boost::bind(&ProcessHandler_windows::readPipe, this,
			output_handler, true, child_err_r));
	readPipe(output_handler, false, child_out_r);
	error_th.join();
    }
    WaitForSingleObject(pi.hProcess, INFINITE);

    { // private scope
	boost::mutex::scoped_lock lock(running_processes_mutex);
	running_processes.erase(&pi);
	status = cleanProcess(&pi);
    }

    return status;
}
