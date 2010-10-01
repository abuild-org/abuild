#include <Abuild.hh>
#include <Logger.hh>
#include <ProcessHandler.hh>

int main(int argc, char* argv[], char* envp[])
{
    Logger::getInstance();
    ProcessHandler::createInstance(envp);

    int status = 0;
    try
    {
	if (! Abuild(argc, argv).run())
	{
	    status = 2;
	}
	Logger::stopLogger();
	ProcessHandler::destroyInstance();
    }
    catch (std::exception& e)
    {
	// Don't destroy the logger or process handler as they may be
	// in use by other threads.
	std::cerr << e.what() << std::endl;
	status = 2;
    }

    return status;
}
