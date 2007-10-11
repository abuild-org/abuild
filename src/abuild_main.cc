#include <Logger.hh>
#include <Abuild.hh>

int main(int argc, char* argv[], char* envp[])
{
    Logger* logger = Logger::getInstance();

    int status = 0;
    try
    {
	if (! Abuild(argc, argv, envp).run())
	{
	    status = 2;
	}
    }
    catch (std::exception& e)
    {
	logger->logError(e.what());
	status = 2;
    }

    Logger::stopLogger();
    return status;
}
