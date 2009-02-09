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

JavaBuilder::JavaBuilder(Error& error,
			 boost::function<void(std::string const&)> verbose,
			 std::string const& abuild_top,
			 std::string const& java,
			 std::list<std::string> const& java_libs,
			 char** envp) :
    error(error),
    logger(*(Logger::getInstance())),
    verbose(verbose),
    last_request(0),
    abuild_top(abuild_top),
    java(java),
    java_libs(java_libs),
    envp(envp),
    run_mode(rm_idle)
{
}

bool
JavaBuilder::invoke(std::string const& backend,
		    std::string const& build_file,
		    std::string const& dir,
		    std::list<std::string> const& targets,
		    std::list<std::string> const& other_args,
		    std::map<std::string, std::string> const& defines)
{
    return makeRequest(backend + "\001" +
		       build_file + "\001" +
		       dir + "\001" +
		       Util::join(" ", targets) + "\001" +
		       Util::join(" ", other_args) + "\001" +
		       writeDefines(defines) + "\001|");
}

std::string
JavaBuilder::writeDefines(std::map<std::string, std::string> const& defines)
{
    std::ostringstream buf;
    for (std::map<std::string, std::string>::const_iterator iter =
	     defines.begin();
	 iter != defines.end(); ++iter)
    {
	std::string const& key = (*iter).first;
	std::string const& val = (*iter).second;
	buf << key.length() << " " << key << val.length() << " " << val;
    }
    return buf.str();
}

bool
JavaBuilder::makeRequest(std::string const& message)
{
    {
	boost::mutex::scoped_lock lock(this->mutex);
	if (this->run_mode == rm_shutting_down)
	{
	    throw std::logic_error("makeRequest called while shutting down");
	}
    }
    start();

    Request_ptr request(new Request(message));
    this->requests.enqueue(request);
    return request->response.dequeue();
}

void
JavaBuilder::finish()
{
    {
	boost::mutex::scoped_lock lock(this->mutex);
	if (this->run_mode != rm_running)
	{
	    return;
	}
    }
    Request_ptr shutdown(new Request(Request::rt_shutdown));
    this->requests.enqueue(shutdown);
    cleanup();
}

void
JavaBuilder::cleanup()
{
    this->reader_thread->join();
    boost::mutex::scoped_lock lock(this->mutex);
    this->sock.reset();
    this->reader_thread.reset();
    this->run_mode = rm_idle;
}

void
JavaBuilder::start()
{
    bool call_cleanup = false;

    {
	boost::mutex::scoped_lock lock(this->mutex);
	switch (this->run_mode)
	{
	  case rm_running:
	    return;

	  case rm_degraded:
	    call_cleanup = true;
	    break;

	  case rm_shutting_down:
	    throw std::logic_error("makeRequest called while shutting down");

	  default:
	    break;
	}
    }

    if (call_cleanup)
    {
	cleanup();
    }

    boost::mutex::scoped_lock lock(this->mutex);
    this->reader_thread.reset(
	new boost::thread(boost::bind(&JavaBuilder::reader, this)));
    while (this->run_mode != rm_running)
    {
	this->running_cond.wait(this->mutex);
    }
}

void
JavaBuilder::reader()
{
    // Set up listen socket, and then start the java program to
    // connect on the port.  Accept one connection and close the
    // listen socket.

    thread_ptr java_thread;

    { // private scope

	boost::asio::ip::tcp::resolver resolver(this->io_service);
	boost::asio::ip::tcp::resolver::query query("127.0.0.1", "0");
	boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
	boost::asio::ip::tcp::acceptor a(
	    this->io_service, boost::asio::ip::tcp::endpoint(*iter));
	unsigned short port = a.local_endpoint().port();
	this->sock.reset(new boost::asio::ip::tcp::socket(this->io_service));
	java_thread.reset(
	    new boost::thread(
		boost::bind(&JavaBuilder::run_java, this, port)));
	a.accept(*(this->sock));
    }

    // Start handler
    boost::thread handler_thread(
	boost::bind(&JavaBuilder::handler, this));

    // Main read loop
    this->run_mode = rm_running;
    this->running_cond.notify_all();
    std::string accumulated_response;
    while (true)
    {
	char data[1024];
	boost::system::error_code error;
	size_t length =
	    this->sock->read_some(boost::asio::buffer(data), error);
	if (error == boost::asio::error::eof)
	{
	    break;
	}
	else if (error)
	{
	    this->error.error(
		FileLocation(),
		"error reading from JavaBuilder: " +
		error.message() + "; will attempt to recover");
	    break;
	}
	else if (this->run_mode == rm_running)
	{
	    accumulated_response += std::string(data, length);
	    handleResponse(accumulated_response);
	}
    }

    bool degraded = false;
    {
	boost::mutex::scoped_lock lock(this->mutex);
	if (this->run_mode == rm_running)
	{
	    // The java side exited unexpectedly
	    degraded = true;
	    QTC::TC("abuild", "JavaBuilder handle abnormal java exit");
	    this->run_mode = rm_degraded;
	}
    }
    if (degraded)
    {
	Request_ptr r(new Request(Request::rt_break));
	this->requests.enqueue(r);
	// Degraded mode recovery seems to sometimes cause
	// segmentation faults within the boost library.  I'm not sure
	// whether it's my code or theirs.
	this->error.error(
	    FileLocation(),
	    "JavaBuilder exited unexpectedly; will try to recover,"
	    " but a crash is likely if this is a parallel build");
    }

    handler_thread.join();
    java_thread->join();
}

void
JavaBuilder::handleResponse(std::string& responses)
{
    boost::regex response_re("(\\d+) (true|false)\r?");
    boost::smatch match;

    size_t p;
    while ((p = responses.find("\n")) != std::string::npos)
    {
	std::string response = responses.substr(0, p);
	responses = responses.substr(p + 1);
	if (boost::regex_match(response, match, response_re))
	{
	    int request_number = std::atoi(match[1].str().c_str());
	    std::string result_str = match[2].str();
	    bool result = (result_str == "true");
	    Request_ptr request;
	    {
		boost::mutex::scoped_lock lock(this->mutex);
		if (! this->pending_requests.count(request_number))
		{
		    throw QEXC::General("protocol error: received response "
					"for unknown request " +
					Util::intToString(request_number));
		}
		request = this->pending_requests[request_number];
		this->pending_requests.erase(request_number);
	    }
	    request->response.enqueue(result);
	}
	else
	{
	    throw QEXC::General("protocol error: received \"" + response +
				"\" from java JavaBuilder program");
	}
    }
}

static bool
asio_completion_condition(boost::system::error_code&, std::size_t)
{
    return true;
}

void
JavaBuilder::handler()
{
    while (true)
    {
	Request_ptr request = this->requests.dequeue();
	boost::system::error_code error;
	if (request->request_type == Request::rt_shutdown)
	{
	    this->run_mode = rm_shutting_down;
	    boost::asio::write(*sock, boost::asio::buffer("shutdown\n"),
			       asio_completion_condition, error);
	    // ignore error condition
	    break;
	}
	else if (request->request_type == Request::rt_break)
	{
	    break;
	}
	else
	{
	    // We're in trouble if we ever have more than 2^31 builds,
	    // but somehow, I'm not too worried about it.
	    int request_number = ++this->last_request;
	    {
		boost::mutex::scoped_lock lock(this->mutex);
		this->pending_requests[request_number] = request;
	    }
	    boost::asio::write(
		*sock, boost::asio::buffer(
		    Util::intToString(request_number) +
		    " " + request->text + "\n"),
		asio_completion_condition, error);
	    if (error)
	    {
		this->error.error(
		    FileLocation(),
		    "error writing message to JavaBuilder: " +
		    error.message() + "; will attempt to recover");
		// Just break....presumably the reader will also get
		// an error or EOF.
		break;
	    }
	}
    }

    boost::mutex::scoped_lock lock(this->mutex);
    for (std::map<int, Request_ptr>::iterator iter =
	     this->pending_requests.begin();
	 iter != this->pending_requests.end(); ++iter)
    {
	(*iter).second->response.enqueue(false);
    }
    this->pending_requests.clear();
}

void
JavaBuilder::run_java(unsigned short port)
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
    args.push_back("java");
    args.push_back("-classpath");
    args.push_back(Util::join(Util::pathSeparator(), jars));
    args.push_back("org.abuild.support.JavaBuilder");
    args.push_back(this->abuild_top);
    args.push_back(Util::intToString(port));

    std::map<std::string, std::string> environment;
    Util::runProgram(this->java, args, environment, this->envp, ".");
}
