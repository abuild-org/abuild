#include "JavaBuilder.hh"

#include <Util.hh>
#include <ProcessHandler.hh>
#include <QEXC.hh>
#include <QTC.hh>
#include <Error.hh>
#include <Logger.hh>
#include <FileLocation.hh>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <set>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <assert.h>

JavaBuilder::JobData::JobData(
    ProcessHandler::output_handler_t output_handler,
    response_queue_ptr response_queue) :
    output_handler(output_handler),
    response_queue(response_queue)
{
}

void
JavaBuilder::JobData::respond(bool status)
{
    handleOutput(false, "");
    handleOutput(true, "");
    this->response_queue->enqueue(status);
}

void
JavaBuilder::JobData::handleOutput(bool is_error, std::string const& message)
{
    if (this->output_handler)
    {
	this->output_handler(is_error, message.c_str(), message.length());
    }
}

JavaBuilder::JavaBuilder(Error& error,
			 bool capture_output,
			 boost::function<void(std::string const&)> verbose,
			 std::string const& abuild_top,
			 std::string const& java,
			 std::string const& java_home,
			 std::string const& ant_home,
			 std::list<std::string> const& java_libs,
			 std::list<std::string> const& jvm_xargs,
			 std::list<std::string> const& build_args,
			 std::map<std::string, std::string> const& defines) :
    error(error),
    logger(*(Logger::getInstance())),
    process_handler(ProcessHandler::getInstance()),
    capture_output(capture_output),
    verbose(verbose),
    abuild_top(abuild_top),
    java(java),
    java_home(java_home),
    ant_home(ant_home),
    java_libs(java_libs),
    jvm_xargs(jvm_xargs),
    build_args(build_args),
    defines(defines),
    last_request(0),
    run_mode(rm_idle),
    io_output_handler(0)
{
}

bool
JavaBuilder::invoke(std::string const& backend,
		    std::string const& build_file,
		    std::string const& dir,
		    std::list<std::string> const& targets,
		    ProcessHandler::output_handler_t output_handler)
{
    return makeRequest(output_handler,
		       backend + "\001" +
		       build_file + "\001" +
		       dir + "\001" +
		       Util::join(" ", targets) + "\001|");
}

bool
JavaBuilder::makeRequest(ProcessHandler::output_handler_t output_handler,
			 std::string const& message)
{
    try
    {
	start();
    }
    catch (StartupFailed)
    {
	return false;
    }

    // Register response queue
    response_queue_ptr response(new ThreadSafeQueue<bool>());
    int request_number = 0;
    std::string text;

    {
	boost::mutex::scoped_lock lock(this->mutex);
	// request_number must never be 0 since 0 is used for data not
	// associated with a job.  We're in trouble if we ever have
	// more than 2^31 builds, but somehow, I'm not too worried
	// about it.
	request_number = ++this->last_request;
	this->job_data[request_number].reset(
	    new JobData(output_handler, response));

	// Send message and wait for response
	text = Util::intToString(request_number) + " " + message + "\n";
	boost::asio::async_write(
	    *this->sock, boost::asio::buffer(text),
	    boost::bind(
		&JavaBuilder::handleWrite, this,
		boost::asio::placeholders::error));
    }

    return response->dequeue();
}

void
JavaBuilder::finish()
{
    {
	boost::mutex::scoped_lock lock(this->mutex);
	if ((this->run_mode == rm_idle) ||
	    (this->run_mode == rm_startup_failed))
	{
	    return;
	}
	if (this->run_mode == rm_running)
	{
	    setRunMode(rm_shutting_down);
	    boost::asio::async_write(
		*this->sock, boost::asio::buffer("shutdown\n"),
		boost::bind(
		    &JavaBuilder::handleWrite, this,
		    boost::asio::placeholders::error));
	}
	waitForShutdown();
    }
    cleanup();
    {
	boost::mutex::scoped_lock lock(this->mutex);
	setRunMode(rm_idle);
    }
}

void
JavaBuilder::cleanup()
{
    // Should not be called with the mutex locked
    assert(this->run_mode == rm_stopped);
    this->io_thread->join();
    this->io_thread.reset();
    this->sock.reset();
    this->io_service.reset();
}

void
JavaBuilder::start()
{
    boost::mutex::scoped_lock lock(this->mutex);

    switch (this->run_mode)
    {
      case rm_running:
	return;

      case rm_starting_up:
	waitForStartup();
	return;

      case rm_startup_failed:
	throw StartupFailed();
	break;

      case rm_shutting_down:
	throw std::logic_error("start() called while shutting down");
	break;

      case rm_stopped:
	cleanup();
	break;

      case rm_idle:
	break;
    }

    setRunMode(rm_starting_up);
    this->io_thread.reset(
	new boost::thread(boost::bind(&JavaBuilder::runIO, this)));

    waitForStartup();
}

void
JavaBuilder::runIO()
{
    thread_ptr java_thread;

    {
	boost::mutex::scoped_lock lock(this->mutex);

	assert(this->run_mode == rm_starting_up);

	// Set up listen socket, and then start the java program to
	// connect on the port.  Accept one connection and close the
	// listen socket.

	this->io_service.reset(new boost::asio::io_service());

	boost::asio::ip::tcp::resolver resolver(*this->io_service);
	boost::asio::ip::tcp::resolver::query query("127.0.0.1", "0");
	boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
	boost::shared_ptr<boost::asio::ip::tcp::acceptor> a(
	    new boost::asio::ip::tcp::acceptor(
		*this->io_service, boost::asio::ip::tcp::endpoint(*iter)));
	unsigned short port = a->local_endpoint().port();
	this->sock.reset(new boost::asio::ip::tcp::socket(*this->io_service));
	java_thread.reset(
	    new boost::thread(
		boost::bind(&JavaBuilder::runJava, this, port)));
	a->async_accept(*(this->sock),
			boost::bind(&JavaBuilder::handleAccept, this,
				    boost::asio::placeholders::error, a));
    }

    this->io_service->run();

    // Java side has shut down.  Join the thread and fail any
    // pending requests.  There won't be any if we exited
    // normally.
    java_thread->join();

    {
	boost::mutex::scoped_lock lock(this->mutex);
	if (this->run_mode == rm_starting_up)
	{
	    setRunMode(rm_startup_failed);
	}
	else
	{
	    for (std::map<int, JobData_ptr>::iterator iter =
		     this->job_data.begin();
		 iter != this->job_data.end(); ++iter)
	    {
		(*iter).second->respond(false);
	    }
	    this->job_data.clear();
	    setRunMode(rm_stopped);
	}
    }
}

void
JavaBuilder::requestRead()
{
    this->sock->async_read_some(
	boost::asio::buffer(this->data, max_data_size),
	boost::bind(
	    &JavaBuilder::handleRead, this,
	    boost::asio::placeholders::error,
	    boost::asio::placeholders::bytes_transferred));
}

void
JavaBuilder::handleAccept(boost::system::error_code const& ec,
			  boost::shared_ptr<boost::asio::ip::tcp::acceptor> ac)
{
    if (ec)
    {
	throw ec;
    }

    // Do not accept any more connections;
    ac.reset();

    // Start accepting requests
    boost::mutex::scoped_lock lock(this->mutex);
    setRunMode(rm_running);
    requestRead();
}

void
JavaBuilder::handleRead(boost::system::error_code const& ec, size_t length)
{
    boost::mutex::scoped_lock lock(this->mutex);
    if (ec)
    {
	bool eof = false;
	if (ec == boost::asio::error::eof)
	{
	    eof = true;
	}
	else
	{
	    this->error.error(
		FileLocation(),
		"error reading from JavaBuilder: " +
		ec.message() + "; will attempt to recover");
	}

	if ((! eof) || (this->run_mode == rm_running))
	{
	    // EOF in running mode means the Java side expected
	    // unexpectedly.
	    QTC::TC("abuild", "JavaBuilder handle abnormal java exit");
	    this->error.error(
		FileLocation(),
		"JavaBuilder exited unexpectedly; will attempt to recover");
	    setRunMode(rm_stopped);
	}
    }
    else if (this->run_mode == rm_running)
    {
	this->accumulated_response += std::string(data, length);
	handleResponse();
	requestRead();
    }
    else
    {
	// Ignore any input we get when not in running mode; we're
	// already failing any pending requests.
    }
}

void
JavaBuilder::handleWrite(boost::system::error_code const& ec)
{
    if (ec)
    {
	boost::mutex::scoped_lock lock(this->mutex);

	// No other action required -- presumably we will get a read
	// error or EOF as well.
	if (this->run_mode == rm_running)
	{
	    this->error.error(
		FileLocation(),
		"error writing message to JavaBuilder: " +
		ec.message() + "; will attempt to recover");
	}
    }
}

void
JavaBuilder::handleResponse()
{
    boost::regex response_re("(\\d+) (true|false)\r?");
    boost::regex data_re("(\\d+) data:(out|err) (\\d+)\r?");
    boost::smatch match;

    size_t p;
    while ((p = this->accumulated_response.find("\n")) != std::string::npos)
    {
	std::string response = this->accumulated_response.substr(0, p);
	this->accumulated_response = this->accumulated_response.substr(p + 1);
	if (boost::regex_match(response, match, response_re))
	{
	    int request_number = std::atoi(match[1].str().c_str());
	    std::string result_str = match[2].str();
	    bool result = (result_str == "true");
	    JobData_ptr job;
	    if (! this->job_data.count(request_number))
	    {
		throw QEXC::General("protocol error: received response "
				    "for unknown request " +
				    Util::intToString(request_number));
	    }
	    job = this->job_data[request_number];
	    this->job_data.erase(request_number);
	    job->respond(result);
	}
	else if (this->capture_output &&
		 boost::regex_match(response, match, data_re))
	{
	    int request_number = std::atoi(match[1].str().c_str());
	    std::string message_type = match[2].str();
	    size_t message_length = std::atoi(match[3].str().c_str());
	    std::string message;
	    bool have_message = false;
	    bool is_error = (message_type == "err");
	    if (accumulated_response.length() > message_length)
	    {
		std::string data =
		    accumulated_response.substr(0, message_length);
		std::string rest =
		    accumulated_response.substr(message_length);
		if ((rest.length() >= 1) && (rest[0] == '\n'))
		{
		    QTC::TC("abuild", "JavaBuilder response ends with NL");
		    have_message = true;
		    rest = rest.substr(1);
		}
		else if ((rest.length() >= 2) && (rest.substr(0, 2) == "\r\n"))
		{
		    QTC::TC("abuild", "JavaBuilder response ends with CRNL");
		    have_message = true;
		    rest = rest.substr(2);
		}
		if (have_message)
		{
		    message = data;
		    accumulated_response = rest;
		}
	    }
	    if (! have_message)
	    {
		QTC::TC("abuild", "JavaBuilder read partial message");
		accumulated_response = response + "\n" + accumulated_response;
		break;
	    }

	    if (request_number == 0)
	    {
		QTC::TC("abuild", "JavaBuilder data for job 0");
		handleRogueOutput(is_error, message.c_str(), message.length());
	    }
	    else
	    {
		JobData_ptr job;
		if (! this->job_data.count(request_number))
		{
		    throw QEXC::General("protocol error: received data "
					"for unknown request " +
					Util::intToString(request_number));
		}
		job = this->job_data[request_number];
		job->handleOutput(is_error, message);
	    }
	}
	else
	{
	    throw QEXC::General("protocol error: received \"" + response +
				"\" from java JavaBuilder program");
	}
	QTC::TC("abuild", "JavaBuilder pending data",
		this->accumulated_response.empty() ? 1 : 0);
    }
}

void
JavaBuilder::waitForStartup()
{
    // Must be called with mutex locked
    while (! ((this->run_mode == rm_running) ||
	      (this->run_mode == rm_startup_failed)))
    {
	this->running_cond.wait(this->mutex);
    }
    if (this->run_mode == rm_startup_failed)
    {
	throw StartupFailed();
    }
}

void
JavaBuilder::waitForShutdown()
{
    // Must be called with mutex locked
    while (this->run_mode != rm_stopped)
    {
	this->running_cond.wait(this->mutex);
    }
}

void
JavaBuilder::setRunMode(run_mode_e mode)
{
    // Must be called with mutex locked
    this->run_mode = mode;
    this->running_cond.notify_all();
}

void
JavaBuilder::runJava(unsigned short port)
{
    boost::regex jar_re(".*\\.(?i:jar)");
    boost::smatch match;

    verbose("setting up classpath for JavaBuilder");
    std::list<std::string> jars;
    for (std::list<std::string>::const_iterator jiter = this->java_libs.begin();
	 jiter != this->java_libs.end(); ++jiter)
    {
	if (Util::isDirectory(*jiter))
	{
	    std::string const& dir = *jiter;
	    verbose("scanning directory " + dir);
	    std::vector<std::string> entries = Util::getDirEntries(dir);
	    for (std::vector<std::string>::iterator eiter = entries.begin();
		 eiter != entries.end(); ++eiter)
	    {
		std::string const& base = *eiter;
		if (boost::regex_match(base, match, jar_re))
		{
		    std::string full = dir + "/" + base;
		    verbose("adding " + full);
		    jars.push_back(full);
		}
	    }
	}
	else
	{
	    verbose("adding non-directory " + *jiter);
	    jars.push_back(*jiter);
	}
    }

    std::vector<std::string> args;
    // The IBM jdk seems to want argv[0] to be its full path if this
    // is not in its path, so set argv[0] to the full path rather than
    // just "java".
    args.push_back(this->java);
    for (std::list<std::string>::const_iterator iter =
	     this->jvm_xargs.begin();
	 iter != this->jvm_xargs.end(); ++iter)
    {
	args.push_back(*iter);
    }
    args.push_back("-classpath");
    args.push_back(Util::join(Util::pathSeparator(), jars));
    args.push_back("org.abuild.javabuilder.JavaBuilder");
    args.push_back(this->abuild_top);
    args.push_back(Util::intToString(port));
    for (std::list<std::string>::const_iterator iter =
	     this->build_args.begin();
	 iter != this->build_args.end(); ++iter)
    {
	args.push_back(*iter);
    }
    for (std::map<std::string, std::string>::const_iterator iter =
	     this->defines.begin();
	 iter != this->defines.end(); ++iter)
    {
	std::string const& key = (*iter).first;
	std::string const& val = (*iter).second;
	args.push_back(key + "=" + val);
    }

    std::map<std::string, std::string> environment;
    environment["JAVA_HOME"] = this->java_home;
    environment["ANT_HOME"] = this->ant_home;
    verbose("JAVA_HOME=" + this->java_home);
    verbose("ANT_HOME=" + this->ant_home);
    verbose("invoking java: " + Util::join(" ", args));

    ProcessHandler::output_handler_t output_handler = 0;
    Logger::job_handle_t logger_job = Logger::NO_JOB;
    if (this->capture_output)
    {
	logger_job = this->logger.requestJobHandle(
	    "JavaBuilder", false, "[JavaBuilder] ", true);
	this->io_output_handler = this->logger.getOutputHandler(logger_job);
	output_handler = boost::bind(
	    &JavaBuilder::handleRogueOutput, this, _1, _2, _3);
    }
    QTC::TC("abuild", "JavaBuilder capture output",
	    this->capture_output ? 1 : 0);
    process_handler.runProgram(this->java, args, environment, true, ".",
			       output_handler);
    this->logger.closeJob(logger_job);
    this->io_output_handler = 0;

    boost::mutex::scoped_lock lock(this->mutex);
    if (this->run_mode == rm_starting_up)
    {
	error.error(FileLocation(), "java builder backend failed to start");
	this->io_service->stop();
    }
}

void
JavaBuilder::handleRogueOutput(bool is_error, char const* data, int len)
{
    this->io_output_handler(is_error, data, len);
}
