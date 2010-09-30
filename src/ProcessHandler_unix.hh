#ifndef __PROCESSHANDLER_UNIX_HH__
#define __PROCESSHANDLER_UNIX_HH__

#include <ProcessHandler.hh>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <boost/thread/mutex.hpp>

class ProcessHandler_unix: public ProcessHandler
{
    friend class ProcessHandler;

  public:
    virtual ~ProcessHandler_unix();

    virtual bool runProgram(
	std::string const& progname,
	std::vector<std::string> const& args,
	std::map<std::string, std::string> const& environment,
	bool preserve_env,
	std::string const& dir,
	ProcessHandler::output_handler_t output_handler = 0);

  private:
    ProcessHandler_unix(char* envp[]);
    void addToReadSet(fd_set& read_set, int& nfds, int fd);
    void handleOutput(fd_set& read_set, int& fd,
		      std::string const& description,
		      ProcessHandler::output_handler_t handler,
		      bool is_error);

    boost::mutex mutex;
};

#endif // __PROCESSHANDLER_UNIX_HH__
