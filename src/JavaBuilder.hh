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

class Error;

class JavaBuilder
{
  public:
    JavaBuilder(Error& error, std::string const& abuild_top,
		std::string const& java,
		std::list<std::string> const& libdirs,
		char* envp[]);
    bool invoke(std::string const& backend,
		std::string const& build_file,
		std::string const& dir,
		std::list<std::string> const& targets,
		std::list<std::string> const& other_args,
		std::map<std::string, std::string> const& defines);
    void finish();

  private:
    JavaBuilder(JavaBuilder const&);
    JavaBuilder& operator=(JavaBuilder const&);

    bool makeRequest(std::string const& request);
    void start();
    void cleanup();
    void reader();
    void handler();
    void run_java(unsigned short port);
    void handleResponse(std::string& responses);
    std::string writeDefines(std::map<std::string, std::string> const&);

    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
    typedef boost::shared_ptr<boost::thread> thread_ptr;

    class Request
    {
      public:
	enum request_type_e { rt_shutdown, rt_break, rt_normal };
	Request(request_type_e t) :
	    request_type(t)
	{
	}
	Request(std::string const& text) :
	    request_type(rt_normal),
	    text(text)
	{
	}
	request_type_e request_type;
	std::string text;
	ThreadSafeQueue<bool> response;
    };
    typedef boost::shared_ptr<Request> Request_ptr;

    enum run_mode_e { rm_idle, rm_running, rm_shutting_down, rm_degraded };

    Error& error;
    boost::mutex mutex;
    boost::condition running_cond;
    int last_request;
    std::string abuild_top;
    std::string java;
    std::list<std::string> libdirs;
    char** envp;
    run_mode_e run_mode;
    boost::asio::io_service io_service;
    socket_ptr sock;
    thread_ptr reader_thread;
    ThreadSafeQueue<Request_ptr> requests;
    std::map<int, Request_ptr> pending_requests;
};

#endif // __JAVABUILDER_HH__
