#include "Logger.hh"
#include <boost/bind.hpp>
#include <string.h>
#include <assert.h>

static void call_with_length(
    boost::function<void(bool, char const*, int)> fn,
    bool bval, char const* str)
{
    fn(bval, str, strlen(str));
}

static boost::function<void(bool, char const*)>
bind_handler(boost::function<void(bool, char const*, int)> fn)
{
    return boost::bind(call_with_length, fn, _1, _2);
}

int main()
{
    Logger* logger = Logger::getInstance();
    logger->setPrefixes("O ", "E ");
    Logger::job_handle_t j1 = logger->requestJobHandle(true, "");
    Logger::job_handle_t j2 = logger->requestJobHandle(false, "[J2] ");
    Logger::job_handle_t j3 = logger->requestJobHandle(true, "");

    logger->logInfo("message 1");
    logger->logError("message 2");

    boost::function<void(bool, char const*)> h1 =
	bind_handler(logger->getOutputHandler(j1));
    boost::function<void(bool, char const*)> h2 =
	bind_handler(logger->getOutputHandler(j2));
    boost::function<void(bool, char const*)> h3 =
	bind_handler(logger->getOutputHandler(j3));

    logger->setJobHeader(j2, "job 2 header");
    logger->setJobHeader(j3, "job 3 header");

    // Do lots of interleaving from a single thread.  This simulates
    // multithreading but allows us to get predictable output.  We
    // have output interleaved with error within and between jobs as
    // well as interleaved jobs.  The regular logInfo interrupts lines
    // of output from job2, which is not interleaved, but the logger
    // keeps the lines together.  We also intersperse logging lines
    // that are associated with jobs and don't come through the output
    // handler.

    h1(false, "job 1 output li");
    logger->logInfo("job 1 additional output", j1);
    h1(true,  "job 1 error line 1\n");
    h2(false, "job 2 output line 1\n");
    h1(false, "ne 1\n");
    h2(true,  "job 2 error line 1\n");
    h3(false, "job 3 output line\n");
    h2(false, "job 2 out");
    logger->logInfo("message 3");
    h1(true,  "job 1 error line 2\n");
    h2(true,  "job 2 er");
    logger->logInfo("job 2 additional output", j2);
    logger->logError("job 2 additional error", j2);
    h1(false, "job 1 output line 2\n");
    h3(true , "job 3 error line\n");
    h2(true,  "ror line 2\n");
    h1(false, "job 1 partial output line");
    h2(false, "put line 2\n");

    logger->closeJob(j3);
    logger->closeJob(j2);
    // don't close job j1 -- let stopLogger deal with it

    // Test harmless operations for NO_JOB
    assert(logger->getOutputHandler(0) == 0);
    logger->closeJob(0);

    Logger::stopLogger();
    return 0;
}
