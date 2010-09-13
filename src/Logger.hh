#ifndef __LOGGER_HH__
#define __LOGGER_HH__

#include <string>
#include <list>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <ThreadSafeQueue.hh>
#include <ProcessHandler.hh>

class Logger
{
  public:

    // Creates and/or returns the singleton instance of the logger
    // object.  The caller should call stopLogger() before exiting to
    // ensure that all logged messages are actually output.
    static Logger* getInstance();

    // Stops the logger and destroys the singleton instance.  Any
    // references to it will be invalid.  If the argument is
    // non-empty, it will be printed as an error message as the last
    // thing before the logger is stopped.
    static void stopLogger(std::string const& error_message = "");

    void setPrefixes(std::string const& output_prefix,
		     std::string const& error_prefix);

    typedef int job_handle_t;
    static job_handle_t const NO_JOB = 0;

    // Request a job handle, used to associated log messages with a
    // specific job.  Each line of output is prefixed by
    // output_prefix, and each line of error is prefixed by
    // error_prefix.  If buffer_output is true, output is held until
    // closeJob() is called.  Otherwise, each line is output when
    // received.
    job_handle_t requestJobHandle(
	std::string const& job_name, bool buffer_output,
	std::string const& job_prefix);

    // Return an output handler for the given job suitable for passing
    // to ProcessHandler::runProgram.
    ProcessHandler::output_handler_t getOutputHandler(job_handle_t job);

    // Indicate that a given job has completed
    void closeJob(job_handle_t job);

    // Writes a message to stdout
    void logInfo(std::string const& message);

    // Writes a message to stderr
    void logError(std::string const& message);

    // Waits for the logger queue to be empty.  Warning: this could
    // take an arbitrarily long time if lots of threads are writing to
    // the logger.  It should really only be called from contexts in
    // which it is known that no one is logging.
    void flushLog();

  private:
    Logger(Logger const&);
    Logger& operator=(Logger const&);

    enum message_type_e { m_shutdown, m_info, m_error };

    class JobData
    {
      public:
	JobData(
	    Logger&,
	    std::string const& job_name,
	    bool buffer_output,
	    std::string const& job_prefix);
	void handle_output(bool is_error, char const* data, int len);
	void flush();

      private:
	void completeLine(bool is_error, std::string& line);

	Logger& logger;
	std::string job_name;
	bool buffer_output;
	std::string job_prefix;
	std::string output_line;
	std::string error_line;
	std::list<std::pair<Logger::message_type_e, std::string> > buffer;
    };

    Logger();

    void loggerMain();
    void writeToLogger(message_type_e, std::string const&);
    void writeToLogger(
	std::list<std::pair<message_type_e, std::string> > const&);

    static Logger* the_instance;
    std::string output_prefix;
    std::string error_prefix;
    boost::shared_ptr<boost::thread> thread;
    boost::mutex queue_write_mutex;
    ThreadSafeQueue<std::pair<message_type_e, std::string> > logger_queue;
    job_handle_t next_job;
    std::map<job_handle_t, boost::shared_ptr<JobData> > jobs;
};

#endif // __LOGGER_HH__
