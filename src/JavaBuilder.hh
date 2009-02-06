#ifndef __JAVABUILDER_HH__
#define __JAVABUILDER_HH__

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <list>
#include <string>
#include <map>
#include "ThreadSafeQueue.hh"

class JavaBuilder
{
  public:
    JavaBuilder(std::string const& java,
		std::list<std::string> const& libdirs,
		char* envp[]);
    bool invokeAnt(std::string const& build_file, std::string const& basedir,
		   std::list<std::string> const& targets);
    void finish();

  private:
    JavaBuilder(JavaBuilder const&);
    JavaBuilder& operator=(JavaBuilder const&);

    bool makeRequest(std::string const& request);
    void start();
    void reader();
    void handler();
    void run_java(unsigned short port);
    void handleResponse(std::string& responses);

    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
    typedef boost::shared_ptr<boost::thread> thread_ptr;

    class Request
    {
      public:
	Request() :
	    shutdown(true)
	{
	}
	Request(std::string const& text) :
	    shutdown(false),
	    text(text)
	{
	}
	bool shutdown;
	std::string text;
	ThreadSafeQueue<bool> response;
    };
    typedef boost::shared_ptr<Request> Request_ptr;

    boost::mutex mutex;
    boost::condition running_cond;
    int last_request;
    std::string java;
    std::list<std::string> libdirs;
    char** envp;
    bool running;
    bool shutting_down;
    boost::asio::io_service io_service;
    socket_ptr sock;
    thread_ptr reader_thread;
    ThreadSafeQueue<Request_ptr> requests;
    std::map<int, Request_ptr> pending_requests;
};

#endif // __JAVABUILDER_HH__
