#include "JavaBuilder.hh"

#include <Util.hh>
#include <QEXC.hh>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <set>
#include <iostream>
#include <sstream>
#include <cstdlib>

JavaBuilder::JavaBuilder(std::string const& java,
			 std::list<std::string> const& libdirs,
			 char** envp) :
    last_request(0),
    java(java),
    libdirs(libdirs),
    envp(envp),
    running(false),
    shutting_down(false)
{
}

bool
JavaBuilder::invokeAnt(std::string const& build_file,
		       std::string const& basedir,
		       std::list<std::string> const& targets)
{
    return makeRequest("ant " + build_file + " " + basedir + " " +
		       Util::join(" ", targets));
}

bool
JavaBuilder::makeRequest(std::string const& message)
{
    {
	boost::mutex::scoped_lock lock(this->mutex);
	if (this->shutting_down)
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
	if (! this->running)
	{
	    return;
	}
    }
    Request_ptr shutdown(new Request());
    this->requests.enqueue(shutdown);
    this->reader_thread->join();
    boost::mutex::scoped_lock lock(this->mutex);
    this->sock.reset();
    this->reader_thread.reset();
    this->shutting_down = false;
    this->running = false;
}

void
JavaBuilder::start()
{
    boost::mutex::scoped_lock lock(this->mutex);

    if (this->running)
    {
	return;
    }

    this->reader_thread.reset(
	new boost::thread(boost::bind(&JavaBuilder::reader, this)));
    while (! this->running)
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
    this->running = true;
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
	    throw boost::system::system_error(error);
	}
	else if (! this->shutting_down)
	{
	    accumulated_response += std::string(data, length);
	    handleResponse(accumulated_response);
	}
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

void
JavaBuilder::handler()
{
    while (true)
    {
	Request_ptr request = this->requests.dequeue();
	if (request->shutdown)
	{
	    boost::asio::write(*sock, boost::asio::buffer("shutdown\n"));
	    boost::mutex::scoped_lock lock(this->mutex);
	    for (std::map<int, Request_ptr>::iterator iter =
		     this->pending_requests.begin();
		 iter != this->pending_requests.end(); ++iter)
	    {
		(*iter).second->response.enqueue(false);
	    }
	    this->pending_requests.clear();
	    this->shutting_down = true;
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
		    " " + request->text + "\n"));
	}
    }
}

void
JavaBuilder::run_java(unsigned short port)
{
    boost::regex jar_re(".*\\.(?i:jar)");
    boost::smatch match;

    // XXX verbose messages including each dir and each jar?
    std::set<std::string, Util::StringCaseLess> jars_found;
    std::list<std::string> jars;
    for (std::list<std::string>::const_iterator liter = this->libdirs.begin();
	 liter != this->libdirs.end(); ++liter)
    {
	std::string const& dir = *liter;
	std::vector<std::string> entries = Util::getDirEntries(dir);
	for (std::vector<std::string>::iterator eiter = entries.begin();
	     eiter != entries.end(); ++eiter)
	{
	    std::string const& base = *eiter;
	    if (boost::regex_match(base, match, jar_re))
	    {
		std::string full = dir + "/" + base;
		jars.push_back(full);
		jars_found.insert(base);
	    }
	}
    }

    if (jars_found.count("abuild-java-support.jar") == 0)
    {
	// XXX give more information or include libdirs in verbose
	throw QEXC::General("abuild-java-support.jar not found");
    }

    std::vector<std::string> args;
    args.push_back("java");
    args.push_back("-classpath");
    args.push_back(Util::join(Util::pathSeparator(), jars));
    args.push_back("org.abuild.support.JavaBuilder");
    args.push_back(Util::intToString(port));

    std::map<std::string, std::string> environment;
    Util::runProgram(this->java, args, environment, this->envp, ".");
}
