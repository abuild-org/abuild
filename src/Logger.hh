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
    // references to it will be invalid, so this function should not
    // be called unless it is known that no threads are accessing the
    // logger.
    static void stopLogger();

    void setPrefixes(std::string const& output_prefix,
		     std::string const& error_prefix);

    typedef int job_handle_t;
    static job_handle_t const NO_JOB = 0;

    // Request a job handle, used to associated log messages with a
    // specific job.  If buffer_output is true, output is held until
    // closeJob() is called.  Otherwise, each line is output when
    // received.
    job_handle_t requestJobHandle(
	bool buffer_output, std::string const& job_prefix);

    // If a non-empty job_header is set, it will be appended with a
    // newline and logged as output the next line of output or error,
    // if any.  If there is no output after a call to setJobHeader(),
    // the job header will not be output.  Setting the job header to
    // the empty string clears it.
    void setJobHeader(job_handle_t job, std::string const& job_header);

    // Return an output handler for the given job suitable for passing
    // to ProcessHandler::runProgram.
    ProcessHandler::output_handler_t getOutputHandler(job_handle_t job);

    // Indicate that a given job has completed
    void closeJob(job_handle_t job);

    // Writes a message to stdout
    void logInfo(std::string const& message, job_handle_t job = NO_JOB);

    // Writes a message to stderr
    void logError(std::string const& message, job_handle_t job = NO_JOB);

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
	    Logger::job_handle_t job,
	    bool buffer_output,
	    std::string const& job_prefix);
	void setJobHeader(std::string const& job_header);
	void handle_output(bool is_error, char const* data, int len);
	void flush();
	void handleMessage(bool is_error, std::string const& line);
	std::string prefixMessage(std::string const& msg);

      private:
	void completeLine(bool is_error, std::string& line);

	boost::recursive_mutex mutex;
	Logger& logger;
	Logger::job_handle_t job;
	bool buffer_output;
	std::string job_prefix;
	std::string job_header;
	std::string output_line;
	std::string error_line;
	std::list<std::pair<Logger::message_type_e, std::string> > buffer;
    };

    Logger();

    void loggerMain();
    std::string prefixMessage(std::string const& msg, job_handle_t job);
    boost::shared_ptr<JobData> findJob(job_handle_t job);
    void handleMessage(bool is_error, std::string const& msg,
		       job_handle_t job);
    void writeToLogger(message_type_e, std::string const&,
		       job_handle_t job);
    void writeToLogger(
	std::list<std::pair<message_type_e, std::string> > const&,
	job_handle_t job);

    static Logger* the_instance;
    std::string output_prefix;
    std::string error_prefix;
    boost::shared_ptr<boost::thread> thread;
    boost::mutex queue_write_mutex;
    boost::recursive_mutex jobdata_mutex;
    ThreadSafeQueue<std::pair<message_type_e, std::string> > logger_queue;
    job_handle_t next_job;
    std::map<job_handle_t, boost::shared_ptr<JobData> > jobs;
};

#endif // __LOGGER_HH__
