#ifndef __PROCESSHANDLER_HH__
#define __PROCESSHANDLER_HH__

#include <string>
#include <vector>
#include <map>
#include <boost/function.hpp>

class ProcessHandler
{
  public:
    // Create the singleton instance of ProcessHandler.  The argument
    // is the original environment of the calling program.  This
    // function must be called exactly one time.
    static void createInstance(char* env[]);

    // Return a reference to the singleton instance of ProcessHandler
    static ProcessHandler& getInstance();

    // Output handler type for runProgram.  Arguments are bool
    // is_error, char const* data, int len.  Usage is described below.
    typedef boost::function<void (bool, char const*, int)> output_handler_t;

    // Run a program given by progname with the given args and
    // environment and with its current directory set to dir.  If
    // preserve_env is true, the original environment will be
    // preserved.  Otherwise, the existing environment will not be
    // preserved.  If an output handler is provided, runProgram will
    // run the child program with stdout and stderr sent through
    // separate pipes, and will call the handler every time output is
    // received from the child program, including calling with len ==
    // 0 on EOF for each pipe.  Additionally, stdin is the null
    // device.  If no output handler is provided, the program's output
    // will not be trapped and will just go to whatever stdout and
    // stderr are inherited.  Returns true iff the program exited
    // normally.
    bool runProgram(std::string const& progname,
		    std::vector<std::string> const& args,
		    std::map<std::string, std::string> const& environment,
		    bool preserve_env,
		    std::string const& dir,
		    output_handler_t output_handler = 0);

  private:
    ProcessHandler(char* env[]);

    static ProcessHandler* the_instance;
    char** env;
};


#endif // __PROCESSHANDLER_HH__
