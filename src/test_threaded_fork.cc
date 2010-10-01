#include <ProcessHandler.hh>
#include <Logger.hh>
#include <Util.hh>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

static int const nthreads = 20;
static int const nperthread = 100;
boost::mutex check_in_mutex;
static std::string echo;
static boost::posix_time::ptime last_check_in(
    boost::posix_time::second_clock::local_time());

static void check_check_in()
{
    while (true)
    {
	Util::msleep(1000);
	boost::posix_time::time_duration delay;
	{ // private scope
	    boost::mutex::scoped_lock lock(check_in_mutex);
	    delay = boost::posix_time::second_clock::local_time() -
		last_check_in;
	}
	if (delay.seconds() > 5)
	{
	    std::cerr << "program appears to be hung; exiting" << std::endl;
	    exit(2);
	}
    }
}


static void check_in()
{
    boost::mutex::scoped_lock lock(check_in_mutex);
    last_check_in = boost::posix_time::second_clock::local_time();
}

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
    ph.runProgram(echo, args, env, true, ".",
		  logger.getOutputHandler(h));
    check_in();

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

    echo = "/bin/echo";
    if (argc > 1)
    {
	echo = argv[1];
    }

    std::vector<boost::shared_ptr<boost::thread> > threads(nthreads);
    for (int i = 0; i < nthreads; ++i)
    {
	threads[i].reset(
	    new boost::thread(
		boost::bind(run_multiple, nperthread * i, nperthread)));
    }
    boost::thread watch(check_check_in);
    for (int i = 0; i < nthreads; ++i)
    {
	threads[i]->join();
    }

    Logger::stopLogger();
    ProcessHandler::destroyInstance();
    return 0;
}
