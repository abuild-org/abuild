#ifndef __PROCESSHANDLER_WINDOWS_HH__
#define __PROCESSHANDLER_WINDOWS_HH__

#include <ProcessHandler.hh>
#include <windows.h>
#include <io.h>
#include <boost/thread/mutex.hpp>
#include <set>

class ProcessHandler_windows: public ProcessHandler
{
    friend class ProcessHandler;

  public:
    virtual ~ProcessHandler_windows();

    virtual bool runProgram(
	std::string const& progname,
	std::vector<std::string> const& args,
	std::map<std::string, std::string> const& environment,
	bool preserve_env,
	std::string const& dir,
	ProcessHandler::output_handler_t output_handler = 0);

  private:
    ProcessHandler_windows(char* envp[]);

    static void waitForProcessesAndExit();
    static BOOL ctrlHandler(DWORD ctrl_type);
    static bool cleanProcess(PROCESS_INFORMATION* pi);

    void readPipe(ProcessHandler::output_handler_t output_handler,
		  bool is_error, HANDLE pipe);

    static boost::mutex running_processes_mutex;
    static std::set<PROCESS_INFORMATION*> running_processes;

    boost::mutex output_mutex;
};

#endif // __PROCESSHANDLER_WINDOWS_HH__
