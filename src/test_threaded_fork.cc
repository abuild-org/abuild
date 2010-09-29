#include <ProcessHandler.hh>
#include <Logger.hh>
#include <Util.hh>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

static int const nthreads = 20;
static int const nperthread = 100;

static void run_program(std::string const& prefix)
{
    Logger& logger = *(Logger::getInstance());
    ProcessHandler& ph = ProcessHandler::getInstance();

    Logger::job_handle_t h = logger.requestJobHandle(
	true, "[" + prefix + "] ");
    std::vector<std::string> args;
    args.push_back("echo");
    args.push_back(prefix);
    std::map<std::string, std::string> env;
    ph.runProgram("/bin/echo", args, env, true, ".",
		  logger.getOutputHandler(h));

    logger.closeJob(h);
}

static void run_multiple(int start, int count)
{
    for (int i = start; i < start + count; ++i)
    {
	run_program(Util::intToString(i));
    }
}

int main(int argc, char* argv[], char* envp[])
{
    ProcessHandler::createInstance(envp);
    Logger::getInstance();

    std::vector<boost::shared_ptr<boost::thread> > threads(nthreads);
    for (int i = 0; i < nthreads; ++i)
    {
	threads[i].reset(
	    new boost::thread(
		boost::bind(run_multiple, nperthread * i, nperthread)));
    }
    for (int i = 0; i < nthreads; ++i)
    {
	threads[i]->join();
    }

    Logger::stopLogger();
    return 0;
}
