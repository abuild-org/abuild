#ifndef __LOGGER_HH__
#define __LOGGER_HH__

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <ThreadSafeQueue.hh>

class Logger
{
  public:

    // Creates and/or returns the singleton instance of the logger
    // object.  The caller should call stopLogger() before exiting to
    // ensure that all logged messages are actually output.
    static Logger* getInstance();

    // Stops the logger and destroys the singleton instance.  Any
    // references to it will be invalid.
    static void stopLogger();

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

    Logger();

    void loggerMain();
    void writeToLogger(message_type_e, std::string const&);

    static Logger* the_instance;
    boost::shared_ptr<boost::thread> thread;
    ThreadSafeQueue<std::pair<message_type_e, std::string> > logger_queue;
};

#endif // __LOGGER_HH__
