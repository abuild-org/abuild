#include <Logger.hh>

#include <iostream>
#include <boost/bind.hpp>
#include <QEXC.hh>

Logger* Logger::the_instance = 0;

Logger::JobData::JobData(
    Logger& logger,
    std::string const& job_name, bool buffer_output,
    std::string const& output_prefix, std::string const& error_prefix) :
    logger(logger),
    job_name(job_name),
    buffer_output(buffer_output),
    output_prefix(output_prefix),
    error_prefix(error_prefix)
{
}

void
Logger::JobData::handle_output(bool is_error, char const* data, int len)
{
    std::string& line = (is_error ? this->error_line : this->output_line);
    if (len == 0)
    {
	if ((! line.empty()) && (*(line.rbegin()) != '\n'))
	{
	    line += "[no newline]\n";
	}
	if (! line.empty())
	{
	    completeLine(is_error, line);
	}
    }
    else
    {
	for (int i = 0; i < len; ++i)
	{
	    line.append(1, data[i]);
	    if (data[i] == '\n')
	    {
		completeLine(is_error, line);
	    }
	}
    }
}

void
Logger::JobData::flush()
{
    // Flush any partial lines
    handle_output(false, "", 0);
    handle_output(true, "", 0);

    if (this->buffer.empty())
    {
	return;
    }

    this->buffer.push_front(
	std::make_pair(Logger::m_info, this->job_name + ": BEGIN OUTPUT\n"));
    this->buffer.push_back(
	std::make_pair(Logger::m_info, this->job_name + ": END OUTPUT\n"));
    this->logger.writeToLogger(this->buffer);
    this->buffer.clear();
}

void
Logger::JobData::completeLine(bool is_error, std::string& line)
{
    Logger::message_type_e message_type = (is_error ? m_error : m_info);
    std::string const& prefix =
	(is_error ? this->error_prefix : this->output_prefix);
    std::string message = prefix + line;
    line.clear();

    if (this->buffer_output)
    {
	this->buffer.push_back(std::make_pair(message_type, message));
    }
    else
    {
	this->logger.writeToLogger(message_type, message);
    }
}

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
Logger::stopLogger(std::string const& error_message)
{
    if (the_instance)
    {
	while (! the_instance->jobs.empty())
	{
	    the_instance->closeJob((*(the_instance->jobs.begin())).first);
	}

	if (! error_message.empty())
	{
	    the_instance->logError(error_message);
	}
	the_instance->writeToLogger(m_shutdown, "");
	the_instance->thread->join();
	delete the_instance;
	the_instance = 0;
    }
}

Logger::job_handle_t
Logger::requestJobHandle(
    std::string const& job_name, bool buffer_output,
    std::string const& output_prefix, std::string const& error_prefix)
{
    job_handle_t job = this->next_job++;
    this->jobs[job].reset(new JobData(*this, job_name, buffer_output,
				      output_prefix, error_prefix));
    return job;
}

boost::function<void (bool, char const*, int)>
Logger::getOutputHandler(job_handle_t job)
{
    if (job == NO_JOB)
    {
	return 0;
    }

    std::map<job_handle_t, boost::shared_ptr<JobData> >::iterator iter =
	this->jobs.find(job);
    if (iter == this->jobs.end())
    {
	throw QEXC::Internal(
	    "Logger::getOutputHandler called for non-existent job");
    }
    boost::shared_ptr<JobData> j = (*iter).second;
    return boost::bind(&JobData::handle_output, j.get(), _1, _2, _3);
}

void
Logger::closeJob(job_handle_t job)
{
    if (job == NO_JOB)
    {
	return;
    }

    std::map<job_handle_t, boost::shared_ptr<JobData> >::iterator iter =
	this->jobs.find(job);
    if (iter == this->jobs.end())
    {
	throw QEXC::Internal("Logger::closeJob called on non-existent job");
    }
    boost::shared_ptr<JobData> j = (*iter).second;
    this->jobs.erase(iter);
    j->flush();
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

Logger::Logger() :
    next_job(1)
{
    thread.reset(new boost::thread(boost::bind(&Logger::loggerMain, this)));
}

void
Logger::writeToLogger(message_type_e message_type, std::string const& msg)
{
    boost::mutex::scoped_lock l(this->queue_write_mutex);
    this->logger_queue.enqueue(std::make_pair(message_type, msg));
}

void
Logger::writeToLogger(
    std::list<std::pair<message_type_e, std::string> > const& messages)
{
    boost::mutex::scoped_lock l(this->queue_write_mutex);
    for (std::list<std::pair<message_type_e,
			     std::string> >::const_iterator iter =
	     messages.begin();
	 iter != messages.end(); ++iter)
    {
	this->logger_queue.enqueue(*iter);
    }
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
