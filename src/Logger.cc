#include <Logger.hh>

#include <iostream>
#include <boost/bind.hpp>

Logger* Logger::the_instance = 0;

Logger*
Logger::getInstance()
{
    if (the_instance == 0)
    {
	the_instance = new Logger();
    }
    return the_instance;
}

void
Logger::stopLogger()
{
    if (the_instance)
    {
	the_instance->writeToLogger(m_shutdown, "");
	the_instance->thread->join();
	delete the_instance;
	the_instance = 0;
    }
}

void
Logger::logInfo(std::string const& message)
{
    writeToLogger(m_info, message + "\n");
}

void
Logger::logError(std::string const& message)
{
    writeToLogger(m_error, message + "\n");
}

void
Logger::flushLog()
{
    this->logger_queue.waitUntilEmpty();
}

Logger::Logger()
{
    thread.reset(new boost::thread(boost::bind(&Logger::loggerMain, this)));
}

void
Logger::writeToLogger(message_type_e message_type, std::string const& msg)
{
    this->logger_queue.enqueue(std::make_pair(message_type, msg));
}

void
Logger::loggerMain()
{
    std::pair<message_type_e, std::string> msg;
    bool done = false;
    while (! done)
    {
	msg = this->logger_queue.head();
	// omit default so gcc will warn for missing case tags
	switch (msg.first)
	{
	  case m_shutdown:
	    done = true;
	    break;

	  case m_info:
	    std::cout << msg.second;
	    std::cout.flush();
	    break;

	  case m_error:
	    std::cerr << msg.second;
	    std::cerr.flush();
	    break;
	}
	// Wait until after we've flushed to deque the message.
	this->logger_queue.dequeue();
    }
}
