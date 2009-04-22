#ifndef __WORKERPOOL_HH__
#define __WORKERPOOL_HH__

// This object manages a pool of worker threads for processing a
// number of objects in parallel.  The WorkerPool is initialized with
// the number of workers that should be created and a method to be
// called for each item that processes the item and returns a status
// code.

// The intended mode of operation is that the user of the pool has a
// loop that calls wait().  The wait() method blocks until the pool
// has been requested to stop, a result is ready, or a worker is
// available.  When wait() returns, methods may be called to determine
// which of those conditions have been satisfied, and appropriate
// action may be taken.  If you are in a state where there is nothing
// else to do until there are results, then you should call
// waitForResults() instead of wait().  Please see comments
// accompanying the method declarations for details.

// For a non-trivial example of using a worker pool, please see the
// DependencyRunner class.

#include <vector>
#include <deque>
#include <iostream>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <ThreadSafeQueue.hh>

template <typename ItemType, typename StatusType>
class WorkerPool
{
  public:
    typedef boost::tuple<ItemType, StatusType> ResultType;

    WorkerPool(unsigned int num_workers,
	       boost::function<StatusType (ItemType)> const& method) :
	num_workers(num_workers),
	method(method),
	shutdown_requested(false)
    {
	assert(num_workers >= 1);
	for (unsigned int i = 0; i < num_workers; ++i)
	{
	    // Create an entry point and a message queue for each thread.
	    // Each thread calls worker_main with its index into the array
	    // as its argument.
	    thread_entries.push_back(
		boost::bind(
		    &WorkerPool<ItemType, StatusType>::worker_main, this, i));
	    queues.push_back(item_queue_ptr_t(new item_queue_t()));
	}
	for (unsigned int i = 0; i < num_workers; ++i)
	{
	    // Now that the other vectors are initialized, create the
	    // threads.  The threads may read these vectors concurrently.
	    threads.push_back(
		thread_ptr_t(new boost::thread(thread_entries[i])));
	}
    }

    // Block until the worker pool is ready to do something.  When
    // this method returns, at least one of the status querying
    // functions below will return true.
    void wait()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	while ((! this->shutdown_requested) &&
	       (this->results.empty()) &&
	       (this->available_workers.empty()))
	{
	    this->condition.wait(lock);
	}
    }

    // Block until there are results to report.  This should be called
    // instead of wait() if there is nothing ready to be processed and
    // nothing new can become available without additional results.
    void waitForResults()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	while ((! this->shutdown_requested) &&
	       (this->results.empty()))
	{
	    this->condition.wait(lock);
	}
    }

    // Status querying function.

    // Return true iff a shutdown request has been received.
    bool shutdownRequested()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	return this->shutdown_requested;
    }

    // Return true iff any results are available
    bool resultsAvailable()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	return (! this->results.empty());
    }

    // Return the number of available workers
    int availableWorkers()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	return this->available_workers.size();
    }

    // Return the number of busy workers
    int busyWorkers()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	return this->num_workers - this->available_workers.size();
    }

    // Return true iff all workers are idle and there are no pending
    // results to read
    bool idle()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	return ((this->available_workers.size() == this->num_workers) &&
		(this->results.empty()));
    }

    // Action methods

    // Get a result.  This method will block until a result is ready.
    // It is best to avoid calling this method unless
    // resultsAvailable() has returned true.  This method returns a
    // tuple containing the item that was processed and the status
    // returned by its worker.
    boost::tuple<ItemType, StatusType> getResults()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	while (this->results.empty())
	{
	    this->condition.wait(lock);
	}
	ResultType r = this->results.front();
	this->results.pop_front();
	if (this->results.empty())
	{
	    this->condition.notify_all();
	}
	return r;
    }

    // Submit an item to be processed.  This method will block until a
    // worker is ready.  It is best to call it only after
    // availableWorkers() has returned > 0.  When the worker has
    // finished, the results will be made available to be returned by
    // a subsequent call to getResults().
    void processItem(ItemType item)
    {
	boost::mutex::scoped_lock lock(this->mutex);
	while (this->available_workers.empty())
	{
	    this->condition.wait(lock);
	}
	int worker_id = this->available_workers.front();
	this->available_workers.pop_front();
	// A true as the first item of the tuple means we do want to
	// process this item.  A false would tell the thread to exit.
	this->queues[worker_id]->enqueue(boost::make_tuple(true, item));
    }

    // Request shutdown.  Calling this method from one thread any
    // thread blocked on wait() to return and the shutdownRequested()
    // method to subsequently return true.  It also asks any running
    // worker threads to exit when they finish their current jobs.  As
    // of the initial implementation, it does not have any impact on
    // any jobs that may be running.
    void requestShutdown()
    {
	boost::mutex::scoped_lock lock(this->mutex);
	this->shutdown_requested = true;
	for (unsigned int i = 0; i < this->num_workers; ++i)
	{
	    // Placing false as first element of tuple tells the
	    // thread to exit.
	    this->queues[i]->enqueue(boost::make_tuple(false, ItemType()));
	}
	this->condition.notify_all();
    }

    // Join all worker threads.  Calls requestShutdown().
    void joinThreads()
    {
	requestShutdown();
	for (unsigned int i = 0; i < this->num_workers; ++i)
	{
	    this->threads[i]->join();
	}
    }

  private:

    void worker_main(int worker_id)
    {
	try
	{
	    item_queue_t& queue = *(this->queues[worker_id]);
	    { // local scope
		boost::mutex::scoped_lock lock(this->mutex);
		this->available_workers.push_back(worker_id);
		this->condition.notify_all();
	    }
	    while (true)
	    {
		item_tuple_t tuple = queue.dequeue();
		bool process_item = boost::get<0>(tuple);
		if (! process_item)
		{
		    break;
		}
		ItemType item = boost::get<1>(tuple);
		StatusType status = this->method(item);
		ResultType result = boost::make_tuple(item, status);
		{ // local scope
		    boost::mutex::scoped_lock lock(this->mutex);
		    this->results.push_back(result);
		    this->available_workers.push_back(worker_id);
		    this->condition.notify_all();
		}
	    }
	}
	catch (std::exception& e)
	{
	    std::cerr << "uncaught exception in thread: " << e.what()
		      << std::endl;
	    exit(2);
	}
    }

    boost::mutex mutex;
    boost::condition condition;

    unsigned int num_workers;
    boost::function<StatusType (ItemType)> method;
    bool shutdown_requested;

    // Here are a few typedefs to make the code more readable.
    typedef boost::tuple<bool, ItemType> item_tuple_t;
    typedef ThreadSafeQueue<item_tuple_t> item_queue_t;
    typedef boost::shared_ptr<item_queue_t> item_queue_ptr_t;
    typedef boost::shared_ptr<boost::thread> thread_ptr_t;

    // There is one of these for each worker.
    std::vector<boost::function<void (void)> > thread_entries;
    std::vector<item_queue_ptr_t> queues;
    std::vector<thread_ptr_t> threads;

    // Create results queue and worker queue.  We'll use regular STL
    // containers for these instead of ThreadSafeQueues because we are
    // protecting them with our own mutex.
    std::deque<ResultType> results;

    // This queue contains the ID numbers of all available workers.
    std::deque<int> available_workers;
};

#endif // __WORKERPOOL_HH__
