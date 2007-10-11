#include <ThreadSafeQueue.hh>

#include <iostream>
#include <boost/function.hpp>
#include <boost/bind.hpp>

typedef ThreadSafeQueue<int> MessageQueue;

class Reader
{
  public:
    Reader(MessageQueue& mq, int timeout) :
	mq(mq),
	timeout(timeout)
    {
    }
    virtual ~Reader() {}
    virtual void apply();

  private:
    MessageQueue& mq;
    int timeout;
};

void
Reader::apply()
{
    try
    {
	while (1)
	{
	    std::cout << this->mq.dequeue(this->timeout) << std::endl;
	}
    }
    catch (MessageQueue::TimeOut)
    {
	std::cout << "timed out reading from queue" << std::endl;
    }
}

class Even
{
  public:
    bool operator()(int const& i)
	{
	    return ((i % 2) == 0);
	}
};

static void msleep(int msec)
{
    boost::xtime delay;
    boost::xtime_get(&delay, boost::TIME_UTC);
    while (msec >= 1000)
    {
	++delay.sec;
	msec -= 1000;
    }
    delay.nsec += msec * 1000000;
    boost::thread::sleep(delay);
}

int main(int argc, char* argv[])
{
    MessageQueue mq;
    Reader r(mq, 1);
    mq.enqueue(1);
    mq.enqueue(6);
    mq.enqueue(0);
    mq.enqueue(5);
    mq.enqueue(9);
    if (mq.head() != 1)
    {
	std::cout << "oops1" << std::endl;
    }
    if (mq.isEmpty())
    {
	std::cout << "oops2" << std::endl;
    }
    Even even;
    mq.removeIf(even);
    std::cout << "starting" << std::endl;
    boost::function<void (void)> r_apply =
	boost::bind(&Reader::apply, boost::ref(r));
    boost::thread th(r_apply);
    mq.enqueue(0);
    mq.enqueue(1);
    mq.enqueue(2);
    mq.waitUntilEmpty();
    if (! mq.isEmpty())
    {
	std::cout << "oops3" << std::endl;
    }
    msleep(250);
    std::cout << "mq is empty" << std::endl;
    msleep(2000);
    th.join();
    Reader r2(mq, 0);
    boost::function<void (void)> r2_apply =
	boost::bind(&Reader::apply, boost::ref(r2));
    boost::thread th2(r2_apply);
    mq.enqueue(1);
    mq.enqueue(4);
    mq.enqueue(1);
    mq.enqueue(5);
    mq.enqueue(9);
    mq.waitUntilEmpty();
    msleep(100);

    std::cout << "exiting" << std::endl;
    return 0;
}
