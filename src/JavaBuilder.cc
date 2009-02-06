#include "JavaBuilder.hh"

#include <boost/bind.hpp>
#include <iostream>
#include <sstream>

JavaBuilder::JavaBuilder() :
    running(false),
    shutting_down(false)
{
}

void
JavaBuilder::makeRequest(std::string const& request)
{
    {
	boost::mutex::scoped_lock lock(this->mutex);
	if (this->shutting_down)
	{
	    throw std::logic_error("makeRequest called while shutting down");
	}
    }
    start();

    this->requests.enqueue(request);
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
    this->requests.enqueue("shutdown\n");
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

	std::cout << std::string(data, length);
    }

    handler_thread.join();
    java_thread->join();
}

void
JavaBuilder::handler()
{
    while (true)
    {
	std::string request = this->requests.dequeue();
	boost::asio::write(*sock, boost::asio::buffer(request));
	if (request == "shutdown\n")
	{
	    boost::mutex::scoped_lock lock(this->mutex);
	    this->shutting_down = true;
	    break;
	}
    }
}

void
JavaBuilder::run_java(unsigned short port)
{
    // XXX
    std::ostringstream buf;
    buf << "java -classpath"
	<< " ../java/abuild-java/dist/request-handler.jar"
	<< " com.example.handler.JavaBuilder " << port;
    system(buf.str().c_str());
}
