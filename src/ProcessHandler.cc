#include <ProcessHandler.hh>

#ifdef _WIN32
# include <ProcessHandler_windows.hh>
#else
# include <ProcessHandler_unix.hh>
#endif

#include <QEXC.hh>

ProcessHandler* ProcessHandler::the_instance = 0;

ProcessHandler::ProcessHandler(char* env[]) :
    env(env)
{
}

ProcessHandler::~ProcessHandler()
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
#ifdef _WIN32
    the_instance = new ProcessHandler_windows(env);
#else
    the_instance = new ProcessHandler_unix(env);
#endif
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
