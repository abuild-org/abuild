#include <ProcessHandler_unix.hh>

#include <fcntl.h>
#include <sys/file.h>
#include <sys/wait.h>

#include <string.h>
#include <stdlib.h>
#include <set>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <Util.hh>
#include <QEXC.hh>

ProcessHandler_unix::ProcessHandler_unix(char* env[]) :
    ProcessHandler(env)
{
}

ProcessHandler_unix::~ProcessHandler_unix()
{
}


void
ProcessHandler_unix::addToReadSet(fd_set& read_set, int& nfds, int fd)
{
    if (fd != -1)
    {
	FD_SET(fd, &read_set);
	nfds = std::max(nfds, 1 + fd);
    }
}

void
ProcessHandler_unix::handleOutput(fd_set& read_set, int& fd,
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

bool
ProcessHandler_unix::runProgram(
    std::string const& progname,
    std::vector<std::string> const& args,
    std::map<std::string, std::string> const& environment,
    bool preserve_env,
    std::string const& dir,
    output_handler_t output_handler)
{
    bool status = true;

    int output_pipe[2];
    int error_pipe[2];
    output_pipe[0] = -1;
    output_pipe[1] = -1;
    error_pipe[0] = -1;
    error_pipe[1] = -1;

    int pid = -1;

    { // private scope
	// Make sure we set the close on exec flag for all our pipes
	// before we call fork.  Otherwise, we end up with lots of
	// pipes open in child processes, which causes all sorts of
	// problems including fork() hanging and EOF not appearing on
	// pipes when processes exit.
	boost::mutex::scoped_lock lock(this->mutex);
	if (output_handler)
	{
	    QEXC::errno_wrapper("create output pipe", pipe(output_pipe));
	    QEXC::errno_wrapper("create error pipe", pipe(error_pipe));
	    QEXC::errno_wrapper("set close on exec",
				fcntl(output_pipe[0], F_SETFD, FD_CLOEXEC));
	    QEXC::errno_wrapper("set close on exec",
				fcntl(output_pipe[1], F_SETFD, FD_CLOEXEC));
	    QEXC::errno_wrapper("set close on exec",
				fcntl(error_pipe[0], F_SETFD, FD_CLOEXEC));
	    QEXC::errno_wrapper("set close on exec",
				fcntl(error_pipe[1], F_SETFD, FD_CLOEXEC));
	}

	pid = fork();
    }
    if (pid == -1)
    {
	if (output_handler)
	{
	    close(output_pipe[0]);
	    close(output_pipe[1]);
	    close(error_pipe[0]);
	    close(error_pipe[1]);
	}
	return false;
    }
    if (pid == 0)
    {
	// This code must take care not to allocate or free any memory
	// through STL.  In particular, creating and destroying
	// instances of std::string appears to cause lockups on
	// Solaris.  Avoid creating exceptions or using non-const
	// access to STL containers.

	if (chdir(dir.c_str()) == -1)
	{
	    _exit(1);
	}

	int stdin_fd = -1;
	if (output_handler)
	{
	    close(output_pipe[0]);
	    close(error_pipe[0]);
	    stdin_fd = open("/dev/null", O_RDONLY);
	    if (stdin_fd == -1)
	    {
		_exit(1);
	    }
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
	    char* vp = new char[(*iter).first.length() + 1 +
				(*iter).second.length() + 1];
	    strcpy(vp, (*iter).first.c_str());
	    strcat(vp, "=");
	    strcat(vp, (*iter).second.c_str());
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
	    close(stdin_fd);
	    close(output_pipe[1]);
	    close(error_pipe[1]);
	}

	execve(progname.c_str(), argv, env);
	_exit(1);
    }
    else
    {
	// Initialize exit_status to a non-zero value so that we'll
	// see the process as having failed if we are never able to
	// read its exit status.
	int exit_status = !0;
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
		addToReadSet(read_set, nfds, child_out);
		addToReadSet(read_set, nfds, child_err);
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
		    handleOutput(read_set, child_out, "child's stdout",
				 output_handler, false);
		    handleOutput(read_set, child_err, "child's stderr",
				 output_handler, true);
		}
	    }
	}
	if (waitpid(pid, &exit_status, 0) != pid)
	{
	    // If we can't get the exit status, treat the process
	    // as having failed.
	    exit_status = !0;
	}
	status = (exit_status == 0);
    }

    return status;
}
