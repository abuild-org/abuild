#include <Abuild.hh>
#include <Logger.hh>
#include <ProcessHandler.hh>

int main(int argc, char* argv[], char* envp[])
{
    Logger::getInstance();
    ProcessHandler::createInstance(envp);

    int status = 0;
    std::string exception;
    try
    {
	if (! Abuild(argc, argv).run())
	{
	    status = 2;
	}
    }
    catch (std::exception& e)
    {
	exception = e.what();
	status = 2;
    }

    Logger::stopLogger(exception);
    if (exception.empty())
    {
	ProcessHandler::destroyInstance();
    }
    return status;
}
