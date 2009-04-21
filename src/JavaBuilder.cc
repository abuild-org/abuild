#include "JavaBuilder.hh"

#include <Util.hh>
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

JavaBuilder::JavaBuilder(Error& error,
			 boost::function<void(std::string const&)> verbose,
			 std::string const& abuild_top,
			 std::string const& java,
			 std::list<std::string> const& java_libs,
			 char** envp,
			 std::list<std::string> const& build_args,
			 std::map<std::string, std::string> const& defines) :
    error(error),
    logger(*(Logger::getInstance())),
    verbose(verbose),
    abuild_top(abuild_top),
    java(java),
    java_libs(java_libs),
    envp(envp),
    build_args(build_args),
    defines(defines),
    last_request(0),
    run_mode(rm_idle)
{
}

bool
JavaBuilder::invoke(std::string const& backend,
		    std::string const& build_file,
		    std::string const& dir,
		    std::list<std::string> const& targets)
{
    return makeRequest(backend + "\001" +
		       build_file + "\001" +
		       dir + "\001" +
		       Util::join(" ", targets) + "\001|");
}

bool
JavaBuilder::makeRequest(std::string const& message)
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
	// We're in trouble if we ever have more than 2^31 builds, but
	// somehow, I'm not too worried about it.
	request_number = ++this->last_request;
	this->response_queues[request_number] = response;

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
    cleanup();
}

void
JavaBuilder::cleanup()
{
    // Must be called with the mutex locked
    this->io_thread->join();
    this->io_thread.reset();
    this->sock.reset();
    this->io_service.reset();
    setRunMode(rm_idle);
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
	    for (std::map<int, response_queue_ptr>::iterator iter =
		     this->response_queues.begin();
		 iter != this->response_queues.end(); ++iter)
	    {
		(*iter).second->enqueue(false);
	    }
	    this->response_queues.clear();
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
	    response_queue_ptr response_queue;
	    {
		if (! this->response_queues.count(request_number))
		{
		    throw QEXC::General("protocol error: received response "
					"for unknown request " +
					Util::intToString(request_number));
		}
		response_queue = this->response_queues[request_number];
		this->response_queues.erase(request_number);
	    }
	    response_queue->enqueue(result);
	}
	else
	{
	    throw QEXC::General("protocol error: received \"" + response +
				"\" from java JavaBuilder program");
	}
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
    verbose("invoking java: " + Util::join(" ", args));
    Util::runProgram(this->java, args, environment, this->envp, ".");

    boost::mutex::scoped_lock lock(this->mutex);
    if (this->run_mode == rm_starting_up)
    {
	error.error(FileLocation(), "java builder backend failed to start");
	this->io_service->stop();
    }
}
