#ifdef __STRICT_ANSI__
// required to be unset for mingw to find _popen, _MAX_PATH
# undef __STRICT_ANSI__
#endif

#include "Util.hh"

#ifdef _WIN32
# include <windows.h>
# include <direct.h>
# include <io.h>
#else
# include <unistd.h>
# include <dirent.h>
# include <sys/stat.h>
# include <sys/wait.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <set>
#include <sstream>
#include <fstream>
#include <boost/thread/mutex.hpp>
#include "QTC.hh"
#include "QEXC.hh"

std::string
Util::intToString(int num)
{
    std::ostringstream buf;
    buf << num;
    return buf.str();
}

bool
Util::stdoutIsTty()
{
#ifdef _WIN32
    return (bool)(_isatty(_fileno(stdout)));
#else
    return (isatty(1) == 1);
#endif
}

bool
Util::getEnv(std::string const& var, std::string* value)
{
    // This was basically ripped out of wxWidgets.
#ifdef _WIN32
    // first get the size of the buffer
    DWORD len = ::GetEnvironmentVariable(var.c_str(), NULL, 0);
    if (len == 0)
    {
        // this means that there is no such variable
        return false;
    }

    if (value)
    {
	char* t = new char[len + 1];
        ::GetEnvironmentVariable(var.c_str(), t, len);
	*value = t;
	delete [] t;
    }

    return true;
#else
    char* p = getenv(var.c_str());
    if (p == 0)
    {
        return false;
    }
    if (value)
    {
        *value = p;
    }

    return true;
#endif
}

void
Util::normalizePathSeparators(std::string& filename)
{
    for (std::string::iterator iter = filename.begin();
	 iter != filename.end(); ++iter)
    {
	if ((*iter) == '\\')
	{
	    *iter = '/';
	}
    }
}

int
Util::strCaseCmp(std::string const& str1, std::string const& str2)
{
    std::string::const_iterator i1 = str1.begin();
    std::string::const_iterator i2 = str2.begin();

    while ((i1 != str1.end()) && (i2 != str2.end()))
    {
	char c1 = tolower(*i1);
	char c2 = tolower(*i2);
	if (c1 != c2)
	{
	    return (c1 < c2 ? -1 : 1);
	}
	++i1;
	++i2;
    }

    size_t size1 = str1.size();
    size_t size2= str2.size();
    return ((size1 < size2) ? -1
	    : (size1 > size2) ? 1
	    : 0);
}

std::list<std::string>
Util::split(char sep, std::string input)
{
    std::list<std::string> result;
    size_t p;
    while ((p = input.find(sep)) != std::string::npos)
    {
        result.push_back(input.substr(0, p));
        input = input.substr(p + 1);
    }
    result.push_back(input);
    return result;
}

std::list<std::string>
Util::splitBySpace(std::string const& input)
{
    std::list<std::string> result;
    result.push_back("");
    for (std::string::const_iterator iter = input.begin();
	 iter != input.end(); ++iter)
    {
	if (isspace(*iter))
	{
	    if (! result.back().empty())
	    {
		result.push_back("");
	    }
	}
	else
	{
	    result.back().append(1, *iter);
	}
    }
    if (result.back().empty())
    {
	result.pop_back();
    }
    return result;
}

std::list<std::string>
Util::readLinesFromFile(std::string const& filename)
{
    std::ifstream in(filename.c_str());
    if (! in.is_open())
    {
	throw QEXC::System("unable to open file " + filename, errno);
    }
    std::list<std::string> lines = readLinesFromFile(in);
    in.close();
    return lines;
}

std::list<std::string>
Util::readLinesFromFile(std::istream& in)
{
    // tested in KeyVal's test suite and in other places

    std::string buf;

    buf.reserve(in.rdbuf()->in_avail());
    char c;
    while (in.get(c))
    {
	if (buf.capacity() == buf.size())
	{
	    buf.reserve(buf.capacity() * 2);
	}
	buf.append(1, c);
    }

    // Manually strip any trailing newline so that split won't add an
    // extra blank line at the end of the output.
    if ((! buf.empty()) && ((*(buf.rbegin())) == '\n'))
    {
	buf.erase(buf.length() - 1);
    }
    std::list<std::string> result = split('\n', buf);
    for (std::list<std::string>::iterator iter = result.begin();
	 iter != result.end(); ++iter)
    {
	// Strip any carriage return that preceded the line
	// terminating newline.
	std::string& line = *iter;
	if ((! line.empty()) && ((*(line.rbegin())) == '\r'))
	{
	    line.erase(line.length() - 1);
	}
    }
    return result;
}

std::string
Util::getCurrentDirectory()
{
    bool failed = false;
#ifdef _WIN32
    char path[_MAX_PATH];
    if (_getcwd(path, _MAX_PATH) == 0)
    {
	failed = true;
    }
#else
    char path[1024];
    if (getcwd(path, sizeof(path)) == 0)
    {
	failed = true;
    }
#endif
    if (failed)
    {
	throw QEXC::System("unable to obtain current directory", errno);
    }
    std::string result = path;
    normalizePathSeparators(result);
    return result;
}

void
Util::setCurrentDirectory(std::string const& dir)
{
#ifdef _WIN32
    if (! ::SetCurrentDirectory(dir.c_str()))
    {
	throw QEXC::General("unable to change directories to " + dir);
    }
#else
    QEXC::errno_wrapper("change directories to " + dir, chdir(dir.c_str()));
#endif
}

bool
Util::isAbsolutePath(std::string const& path)
{
    bool result = false;
#ifdef _WIN32
    if ((path.length() >= 3) &&
	isalpha(path[0]) &&
	(path[1] == ':') &&
	((path[2] == '/') || (path[2] == '\\')))
    {
	result = true;
    }
#endif

    if (! result)
    {
	if ((path.length() >= 1) &&
	    ((path[0] == '/') || (path[0] == '\\')))
	{
	    result = true;
	}
    }

    return result;
}

std::string
Util::canonicalizePath(std::string path)
{
#ifndef _WIN32
    std::set<std::string> sawlinks;
#endif

    assert(! path.empty());
    normalizePathSeparators(path);
    if (! isAbsolutePath(path))
    {
	path = getCurrentDirectory() + "/" + path;
    }

    std::list<std::string> components = split('/', path);
    std::list<std::string> result;
    for (std::list<std::string>::iterator iter = components.begin();
	 iter != components.end(); ++iter)
    {
	std::string const& component = *iter;
	if ((component.empty()) && (! result.empty()))
	{
	    QTC::TC("abuild", "Util::canonicalizePath ignore empty");
	    // ignore
	}
	else if (component == ".")
	{
	    QTC::TC("abuild", "Util::canonicalizePath ignore .");
	    // ignore
	}
	else if (component == "..")
	{
	    if (result.size() > 1)
	    {
		QTC::TC("abuild", "Util::canonicalizePath handle ..");
		result.pop_back();
	    }
	    else
	    {
		QTC::TC("abuild", "Util::canonicalizePath ignore ..");
	    }
	}
	else
	{
	    result.push_back(component);
	}

	// Note: we have no coverage cases inside the #ifdef below
	// because they can't be seen when _WIN32 is defined.  The
	// test suite when run on UNIX fully exercises this code.
	// Instead, we set these silly variables.

	bool saw_relative = false;
	bool saw_absolute = false;
	bool saw_loop = false;

#ifndef _WIN32
	// Resolve symbolic links.  This code does this somewhat
	// half-heartedly so that it can avoid issuing an error
	// message.  If this is a loop, just pretend it's not a
	// symbolic link and keep going.  If we get any permissions
	// errors or the path is too long, also pretend it's not a
	// symbolic link.  So basically, if there are problems with
	// accessing the path, we'll return something that would be
	// equilvalent to the path even if it isn't really the full
	// canonical path.

	if (result.size() > 1)
	{
	    std::string so_far = join("/", result);
	    if (! sawlinks.count(so_far))
	    {
		struct stat statbuf;
		if ((lstat(so_far.c_str(), &statbuf) == 0) &&
		    (S_ISLNK(statbuf.st_mode)))
		{
		    char linkbuf[2048];
		    memset(linkbuf, 0, sizeof(linkbuf));
		    if (readlink(so_far.c_str(), linkbuf,
				 sizeof(linkbuf) - 1) > 0)
		    {
			sawlinks.insert(so_far);
			std::string newpath = linkbuf;
			std::list<std::string> tmp = split('/', newpath);
			std::list<std::string>::iterator here = iter;
			std::list<std::string>::iterator next = iter;
			++next;
			components.insert(next, tmp.begin(), tmp.end());
			next = here;
			++next;
			if (isAbsolutePath(newpath))
			{
			    saw_absolute = true;
			    // Start over again replacing what we've
			    // seen so far with the result of reading
			    // this link.
			    components.erase(components.begin(), next);
			    iter = components.begin();
			    result.clear();
			    result.push_back("");
			}
			else
			{
			    saw_relative = true;
			    // Pretend we haven't seen this component,
			    // and replace it with the result of
			    // readlink.
			    --iter;
			    components.erase(here, next);
			    result.pop_back();
			}
		    }
		}
	    }
	    else
	    {
		saw_loop = true;
	    }
	}
#else
	// Just set the QTC variables to true so the coverage cases
	// pass in Windows.
	saw_relative = true;
	saw_absolute = true;
	saw_loop = true;
#endif

	if (saw_relative)
	{
	    QTC::TC("abuild", "Util::canoncalizePath absolute link");
	}

	if (saw_absolute)
	{
	    QTC::TC("abuild", "Util::canoncalizePath relative link");
	}

	if (saw_loop)
	{
	    QTC::TC("abuild", "Util::canonicalizePath loop detected");
	}
    }

    std::string canonical = join("/", result);
    if (result.size() == 1)
    {
	canonical += "/";
    }

    return canonical;
}

std::string
Util::dirname(std::string path)
{
    std::string::size_type location = path.find_last_of("/\\");
    std::string result;
    if (location == std::string::npos)
    {
	result = ".";
    }
    else
    {
	result = path.substr(0, location);
	if (result.empty())
	{
	    result = "/";
	}

#ifdef _WIN32
	if ((result.length() == 2) &&
	    isalpha(result[0]) &&
	    (result[1] == ':'))
	{
	    result += "/";
	}
#endif

    }
    return result;
}

std::string
Util::basename(std::string path)
{
    std::string::size_type location = path.find_last_of("/\\");
    std::string result;
    if (location == std::string::npos)
    {
	result = path;
    }
    else
    {
	result = path.substr(location + 1);
    }
    return result;
}

bool
Util::fileExists(std::string const& filename)
{
    bool result = false;

#ifdef _WIN32
    DWORD attributes = GetFileAttributes(filename.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES)
    {
	result = true;
    }
#else
    struct stat statbuf;
    if (stat(filename.c_str(), &statbuf) != -1)
    {
	result = true;
    }
#endif

    return result;
}

bool
Util::isFile(std::string const& filename)
{
    bool result = false;

#ifdef _WIN32
    DWORD attributes = GetFileAttributes(filename.c_str());
    if ((attributes != INVALID_FILE_ATTRIBUTES) &&
	((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0))
    {
	result = true;
    }
#else
    struct stat statbuf;
    if ((stat(filename.c_str(), &statbuf) != -1) &&
	(S_ISREG(statbuf.st_mode)))
    {
	result = true;
    }
#endif

    return result;
}

bool
Util::isDirectory(std::string const& filename)
{
    bool result = false;

#ifdef _WIN32
    DWORD attributes = GetFileAttributes(filename.c_str());
    if ((attributes != INVALID_FILE_ATTRIBUTES) &&
	((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0))
    {
	result = true;
    }
#else
    struct stat statbuf;
    if ((stat(filename.c_str(), &statbuf) != -1) &&
	(S_ISDIR(statbuf.st_mode)))
    {
	result = true;
    }
#endif

    return result;
}

std::string
Util::getExtension(std::string const& path)
{
    std::string extension;
#ifdef _WIN32
    unsigned int len = path.length();
    if (len >= 4)
    {
	std::string last4 = path.substr(len - 4);
	if (last4[0] == '.')
	{
	    for (unsigned int i = 1; i <= 3; ++i)
	    {
		last4[i] = tolower(last4[i]);
	    }
	    extension = last4.substr(1);
	}
    }
#endif
    return extension;
}

void
Util::appendExe(std::string& progname)
{
#ifdef _WIN32
    std::string extension = getExtension(progname);
    if ((extension == "bat") || (extension == "com") || (extension == "exe"))
    {
	// nothing to do
    }
    else
    {
	if (isFile(progname + ".bat"))
	{
	    progname += ".bat";
	}
	else if (isFile(progname + ".com"))
	{
	    progname += ".com";
	}
	else
	{
	    // Do this unconditionally -- if it's not there, it will
	    // fail anyway, and there happens to be a program by this
	    // name without a suffix, we don't want it anyway.
	    progname += ".exe";
	}
    }
#endif
}

std::string
Util::removeExe(std::string const& arg)
{
#ifdef _WIN32
    std::string extension = getExtension(arg);
    if ((extension == "bat") || (extension == "com") || (extension == "exe"))
    {
	return arg.substr(0, arg.length() - 4);
    }
    else
    {
	return arg;
    }
#else
    return arg;
#endif
}

std::list<std::string>
Util::findProgramInPath(std::string progname)
{
    std::list<std::string> result;

    appendExe(progname);
    std::string path;
    if (getEnv("PATH", &path))
    {
	std::list<std::string> pathdirs;
#ifdef _WIN32
	pathdirs = split(';', path);
	// "." is implicitly in the path in Windows.
	pathdirs.push_front(".");
#else
	pathdirs = split(':', path);
#endif
	for (std::list<std::string>::iterator iter = pathdirs.begin();
	     iter != pathdirs.end(); ++iter)
	{
	    std::string const& dir = *iter;
	    std::string file =
		canonicalizePath(dir + "/" + progname);
	    bool exists = false;
	    if (isFile(file))
	    {
		exists = true;
#ifdef _WIN32
		file = removeExe(file);
#else
		if (access(file.c_str(), X_OK) != 0)
		{
		    exists = false;
		}
#endif
	    }

	    if (exists)
	    {
		result.push_back(file);
	    }
	}
    }

    return result;
}

bool
Util::getProgramFullPath(std::string progname, std::string& progpath)
{
    bool found = false;
    if (progname.find_last_of("/\\") != std::string::npos)
    {
	found = true;
	progpath = removeExe(canonicalizePath(progname));
    }
    else
    {
	std::list<std::string> candidates = findProgramInPath(progname);
	if (! candidates.empty())
	{
	    found = true;
	    progpath = candidates.front();
	}
    }

    return found;
}

static bool
pathComponentsMatch(std::string const& s1, std::string const& s2)
{
    // Do a non-case-sensitive comparison on Windows
#ifdef _WIN32
    return (Util::strCaseCmp(s1, s2) == 0);
#else
    return (s1 == s2);
#endif
}

std::string
Util::absToRel(std::string const& path, std::string container)
{
    if (container.empty())
    {
	container = getCurrentDirectory();
    }

    assert(isAbsolutePath(path));
    assert(isAbsolutePath(container));

#ifdef _WIN32
    if ((path.length() >= 2) && (path[1] == ':') &&
	(container.length() >= 2) && (container[1] == ':') &&
	(tolower(path[0]) != tolower(container[0])))
    {
	// No way to construct a relative path to a file on a
	// different drive.
	return path;
    }
#endif

    std::list<std::string> pitems = split('/', path);
    std::list<std::string> litems = split('/', container);
    while ((! pitems.empty()) && (! litems.empty()) &&
	   (pathComponentsMatch(pitems.front(), litems.front())))
    {
	pitems.pop_front();
	litems.pop_front();
    }
    std::string result;
    unsigned int lsize = litems.size();
    for (unsigned int i = 0; i < lsize; ++i)
    {
	result += "../";
    }
    if (! pitems.empty())
    {
	result += join("/", pitems);
    }
    if (*(result.rbegin()) == '/')
    {
	result.erase(result.length() - 1);
    }
    if (result.empty())
    {
	result = ".";
    }

    return result;
}

std::string
Util::getProgramOutput(std::string cmd)
{
    FILE* (*open_function)(char const*, char const*);
    int (*close_function)(FILE*);
#ifdef _WIN32
    open_function = _popen;
    close_function = _pclose;
#else
    open_function = popen;
    close_function = pclose;
    cmd += " 2>/dev/null";
#endif

    FILE* f = QEXC::fopen_wrapper("popen(" + cmd + ")",
				  open_function(cmd.c_str(), "r"));;
    std::string result;
    char buf[128];
    while (! feof(f))
    {
	if (fgets(buf, sizeof(buf), f) != NULL)
	{
	    result += buf;
	}
    }
    int status = close_function(f);
    if (status != 0)
    {
	throw QEXC::General(
	    cmd + " failed with status " + intToString(status));
    }

    if (result.length() && (*(result.rbegin()) == '\n'))
    {
	result.erase(result.length() - 1);
    }
    if (result.length() && (*(result.rbegin()) == '\r'))
    {
	result.erase(result.length() - 1);
    }

    return result;
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

#endif // _WIN32

bool
Util::runProgram(std::string const& progname,
		 std::vector<std::string> const& args,
		 std::map<std::string, std::string> const& environment,
		 char* old_env[],
		 std::string const& dir)
{
    bool status = true;

#ifdef _WIN32
    static bool installed_ctrl_handler = false;

    if (! installed_ctrl_handler)
    {
	if (! SetConsoleCtrlHandler((PHANDLER_ROUTINE) ctrlHandler, TRUE))
	{
	    throw QEXC::General("could not set control handler");
	}
	installed_ctrl_handler = true;
    }

    std::string progpath = progname;
    appendExe(progpath);
    std::string suffix = getExtension(progpath);

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
    if ((! environment.empty()) || old_env)
    {
	for (std::map<std::string, std::string>::const_iterator iter =
		 environment.begin();
	     iter != environment.end(); ++iter)
	{
	    env_str += (*iter).first + "=" + (*iter).second;
	    env_str.append(1, '\0');
	}

	if (old_env)
	{
	    for (char** envp = old_env; *envp; ++envp)
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
    ZeroMemory(&pi, sizeof(pi));

    std::string comspec;
    std::string appname = progpath;
    if ((suffix == "bat") && getEnv("COMSPEC", &comspec))
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

    WaitForSingleObject(pi.hProcess, INFINITE);

    { // private scope
	boost::mutex::scoped_lock lock(running_process_mutex);
	running_processes.erase(&pi);
	status = cleanProcess(&pi);
    }
#else
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

	int nvars = environment.size();
	if (old_env)
	{
	    for (char** envp = old_env; *envp; ++envp)
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
	if (old_env)
	{
	    for (char** oenvp = old_env; *oenvp; ++oenvp)
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

	execve(progname.c_str(), argv, env);
    }
    else
    {
	int exit_status;
	if (waitpid(pid, &exit_status, 0) != pid)
	{
	    return false;
	}
	status = (exit_status == 0);
    }
#endif

    return status;
}

std::list<std::string>
Util::getDirEntries(std::string const& path)
{
    // Get list of names in the directory
    std::list<std::string> entries;

#ifdef _WIN32
    HANDLE fhandle;
    WIN32_FIND_DATA filedata;

    fhandle = FindFirstFile(std::string(path + "\\*").c_str(), &filedata);
    if (fhandle != INVALID_HANDLE_VALUE)
    {
	bool done = false;
	while (! done)
	{
	    std::string name = filedata.cFileName;
	    entries.push_back(name);
	    if (! FindNextFile(fhandle, &filedata))
	    {
		if (GetLastError() == ERROR_NO_MORE_FILES)
		{
		    done = true;
		}
		else
		{
		    throw QEXC::General(
			"FindNextFile failed while reading " + path);
		}
	    }
	}
    }
    FindClose(fhandle);
#else
    static boost::mutex mutex;
    DIR *dir;
    if ((dir = opendir(path.c_str())) == NULL)
    {
	throw QEXC::System("open directory " + path, errno);
    }
    { // local scope to call non-threadsafe readdir
	boost::mutex::scoped_lock lock(mutex);
	struct dirent* dir_entry = 0;
	while ((dir_entry = readdir(dir)) != NULL)
	{
	    std::string name = dir_entry->d_name;
	    entries.push_back(name);
	}
    }
    closedir(dir);
#endif

    return entries;
}

void
Util::removeFileRecursively(std::string const& path)
{
    if (isDirectory(path))
    {
	std::list<std::string> entries = getDirEntries(path);
	for (std::list<std::string>::iterator iter = entries.begin();
	     iter != entries.end(); ++iter)
	{
	    std::string const& name = *iter;
	    if (! ((name == ".") || (name == "..")))
	    {
		std::string fullpath = path + "/" + name;
		removeFileRecursively(fullpath);
	    }
	}

#ifdef _WIN32
# define rmdir _rmdir
#endif
	QEXC::errno_wrapper("remove directory " + path, rmdir(path.c_str()));
#ifdef _WIN32
# undef rmdir
#endif
    }
    else
    {
#ifdef _WIN32
	SetFileAttributes(path.c_str(), FILE_ATTRIBUTE_NORMAL);
# define unlink _unlink
#endif
	QEXC::errno_wrapper("remove " + path, unlink(path.c_str()));
#ifdef _WIN32
# undef unlink
#endif
    }
}

void
Util::makeDirectory(std::string const& path)
{
    if (! isDirectory(path))
    {
	int result = 0;
#ifdef _WIN32
	result = _mkdir(path.c_str());
#else
	result = mkdir(path.c_str(), 0777);
#endif
	QEXC::errno_wrapper("create directory " + path, result);
    }
}

std::string
Util::XMLify(std::string const& str, bool attribute)
{
    std::string result;
    for (std::string::const_iterator iter = str.begin();
	 iter != str.end(); ++iter)
    {
	char ch = *iter;
	switch (ch)
	{
	  case '<':
	    result += "&lt;";
	    break;

	  case '>':
	    result += "&gt;";
	    break;

	  case '&':
	    result += "&amp;";
	    break;

	  case '\'':
	    result += (attribute ? "&apos;" : "'");
	    break;

	  case '"':
	    result += (attribute ? "&quot;" : "\"");
	    break;

	  case '\r':
	    result += (attribute ? "" : "\r");
	    break;

	  case '\n':
	    result += (attribute ? " " : "\n");
	    break;

	  default:
	    result.append(1, ch);
	    break;
	}
    }
    return result;
}
