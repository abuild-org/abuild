#include <Abuild.hh>
#include <Logger.hh>

int main(int argc, char* argv[], char* envp[])
{
    Logger::getInstance();

    int status = 0;
    std::string exception;
    try
    {
	if (! Abuild(argc, argv, envp).run())
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
    return status;
}
