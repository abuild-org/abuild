#ifndef __JAVABUILDER_HH__
#define __JAVABUILDER_HH__

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <string>
#include "ThreadSafeQueue.hh"

class JavaBuilder
{
  public:
    JavaBuilder();
    void invokeAnt();
    void finish();

  private:
    JavaBuilder(JavaBuilder const&);
    JavaBuilder& operator=(JavaBuilder const&);

    void makeRequest(std::string const& request);
    void start();
    void reader();
    void handler();
    void run_java(unsigned short port);

    typedef boost::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr;
    typedef boost::shared_ptr<boost::thread> thread_ptr;

    boost::mutex mutex;
    boost::condition running_cond;
    bool running;
    bool shutting_down;
    boost::asio::io_service io_service;
    socket_ptr sock;
    thread_ptr reader_thread;
    ThreadSafeQueue<std::string> requests;
};

#endif // __JAVABUILDER_HH__
