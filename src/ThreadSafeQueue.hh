#ifndef __THREADSAFEQUEUE_HH__
#define __THREADSAFEQUEUE_HH__

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

template<class T>
class ThreadSafeQueue
{
  public:
    class TimeOut: public std::exception
    {
      public:
	virtual ~TimeOut() throw ()
	{
	}
	virtual char const* what() const throw ()
	{
	    return "timeout";
	}
    };

    ThreadSafeQueue<T>()
    {
	// nothing needed
    }
    bool isEmpty()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	return this->data.empty();
    }
    void enqueue(T const& item, bool first = false)
    {
	boost::mutex::scoped_lock lock(this->mutex);
	if (first)
	{
	    this->data.push_front(item);
	}
	else
	{
	    this->data.push_back(item);
	}
	this->not_empty.notify_all();
    }
    T dequeue(int timeout = 0)
    {
	boost::mutex::scoped_lock lock(this->mutex);
	while (this->data.empty())
	{
	    wait(lock, this->not_empty, timeout);
	}
	T result = this->data.front();
	this->data.pop_front();
	if (this->data.empty())
	{
	    this->empty.notify_all();
	}
	return result;
    }
    T head(int timeout = 0)
    {
	boost::mutex::scoped_lock lock(this->mutex);
	while (this->data.empty())
	{
	    wait(lock, this->not_empty, timeout);
	}
	return this->data.front();
    }
    void waitUntilEmpty(int timeout = 0)
    {
	boost::mutex::scoped_lock lock(this->mutex);
	while (! this->data.empty())
	{
	    wait(lock, this->empty, timeout);
	}
    }

    template<class Pred>
    void removeIf(Pred& pr)
    {
	boost::mutex::scoped_lock lock(this->mutex);
	this->data.remove_if(pr);
    }

  private:
    // prohibit copying and assignment
    ThreadSafeQueue<T>(ThreadSafeQueue<T> const&);
    ThreadSafeQueue<T>& operator=(ThreadSafeQueue<T> const&);

    void wait(boost::mutex::scoped_lock& lock,
	      boost::condition& cond, int timeout)
    {
	if (timeout)
	{
	    boost::xtime delay;
	    boost::xtime_get(&delay, boost::TIME_UTC);
	    delay.sec += timeout;
	    if (! cond.timed_wait(lock, delay))
	    {
		throw TimeOut();
	    }
	}
	else
	{
	    cond.wait(lock);
	}
    }

    std::list<T> data;
    boost::mutex mutex;
    boost::condition not_empty;
    boost::condition empty;
};

#endif // __THREADSAFEQUEUE_HH__
