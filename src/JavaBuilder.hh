#ifndef __JAVABUILDER_HH__
#define __JAVABUILDER_HH__

#ifdef _WIN32
# define _WIN32_WINNT 0x0501
#endif

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <list>
#include <string>
#include <map>
#include "ThreadSafeQueue.hh"

class Error;
class Logger;

class JavaBuilder
{
  public:
    JavaBuilder(Error& error,
		boost::function<void(std::string const&)> verbose,
		std::string const& abuild_top,
		std::string const& java,
		std::list<std::string> const& java_libs,
		char* envp[],
		std::list<std::string> const& build_args,
		std::map<std::string, std::string> const& defines);
    bool invoke(std::string const& backend,
		std::string const& build_file,
		std::string const& dir,
		std::list<std::string> const& targets);
    void finish();

  private:
    JavaBuilder(JavaBuilder const&);
    JavaBuilder& operator=(JavaBuilder const&);

    class StartupFailed
    {
    };

    enum run_mode_e { rm_idle, rm_running, rm_starting_up, rm_startup_failed,
		      rm_shutting_down, rm_stopped };
    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
    typedef boost::shared_ptr<boost::thread> thread_ptr;
    typedef boost::shared_ptr<ThreadSafeQueue<bool> > response_queue_ptr;

    bool makeRequest(std::string const& request);
    void requestRead();
    void start();
    void cleanup();
    void runIO();
    void handleAccept(boost::system::error_code const&,
		      boost::shared_ptr<boost::asio::ip::tcp::acceptor>);
    void handleRead(boost::system::error_code const&, size_t length);
    void handleWrite(boost::system::error_code const&);
    void runJava(unsigned short port);
    void handleResponse();
    void waitForStartup();
    void waitForShutdown();
    void setRunMode(run_mode_e);

    Error& error;
    Logger& logger;
    boost::function<void(std::string const&)> verbose;
    std::string abuild_top;
    std::string java;
    std::list<std::string> java_libs;
    char** envp;
    std::list<std::string> build_args;
    std::map<std::string, std::string> defines;
    static unsigned int const max_data_size = 1024;
    char data[max_data_size];
    std::string accumulated_response;

    // These fields must be accessed with mutex protection.
    boost::mutex mutex;
    boost::condition running_cond;
    int last_request;
    run_mode_e run_mode;
    boost::shared_ptr<boost::asio::io_service> io_service;
    socket_ptr sock;
    thread_ptr io_thread;
    std::map<int, response_queue_ptr > response_queues;
};

#endif // __JAVABUILDER_HH__
