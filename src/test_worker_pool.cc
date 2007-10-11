#include <WorkerPool.hh>
#include <iostream>

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

static int f(std::string str)
{
    msleep(1000);
    return str.length();
}

static bool g(int)
{
    return true;
}

int main()
{
    WorkerPool<std::string, int> pool(4, f);

    std::list<std::string> items;
    items.push_back("1 one");
    items.push_back("1 two");
    items.push_back("1 three");
    items.push_back("1 four");
    items.push_back("2 five");
    items.push_back("2 six");
    items.push_back("2 seven");
    items.push_back("2 eight");
    items.push_back("3 nine");
    items.push_back("3 ten");
    items.push_back("3 elephant");

    bool done = false;
    while (! done)
    {
	pool.wait();
	while (pool.resultsAvailable())
	{
	    boost::tuple<std::string, int> result = pool.getResults();
	    std::cout << "result: "
		      << result.get<0>() << " " << result.get<1>() << "\n";
	}
	while (pool.availableWorkers() && (! items.empty()))
	{
	    pool.processItem(items.front());
	    items.pop_front();
	}

	if (pool.idle() && items.empty())
	{
	    done = true;
	}
    }
    pool.joinThreads();

    WorkerPool<int, bool> pool2(3, g);
    pool2.joinThreads();

    return 0;
}
