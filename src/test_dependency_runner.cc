// We have a list of items to process.  Each item may depend upon
// other items.  We have a pool of n threads with which to process
// them.  Process as many items as we can subject to the constraint
// that an item may be processed only when all its dependencies have
// been processed.

#include <iostream>
#include <sstream>
#include <string>

#include <stdio.h>
#include <string.h>

#include <boost/thread.hpp>

#include "Util.hh"
#include "Logger.hh"
#include "ThreadSafeQueue.hh"
#include "DependencyGraph.hh"
#include "DependencyRunner.hh"
#include "WorkerPool.hh"

static char const* whoami = 0;

// Logger
static Logger* logger = 0;

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

static void
change_callback(std::string const& item, DependencyEvaluator::ItemState state)
{
    logger->logInfo("** " + item + " -> " +
		    DependencyEvaluator::unparseState(state));
}

static bool
process_item(std::string item, bool fail11, bool silent)
{
    if (! silent)
    {
	logger->logInfo("processing " + item);
	msleep(500);
    }
    bool status = true;
    if (fail11 && (item == "11"))
    {
	status = false;
    }
    if (! silent)
    {
	logger->logInfo("result: " + item + " -> " +
			std::string(status ? "succeeded" : "failed"));
    }
    return status;
}

static void
init_graph(DependencyGraph& graph)
{
    // Create items 2 through 200 with each dependent upon every
    // other one of its factors.
    for (int i = 2; i < 200; ++i)
    {
	std::string item = Util::intToString(i);
	graph.addItem(item);
	bool toggle = true;
	for (int j = 2; j <= i/2; ++j)
	{
	    if ((j * (i / j)) == i)
	    {
		if (toggle)
		{
		    graph.addDependency(item, Util::intToString(j));
		}
		toggle = ! toggle;
	    }
	}
    }
}

int main(int argc, char* argv[])
{
    if ((whoami = strrchr(argv[0], '/')) == NULL)
    {
	whoami = argv[0];
    }
    else
    {
	++whoami;
    }

    // Unbuffer stdout
    setbuf(stdout, 0);

    // Start logger
    logger = Logger::getInstance();

    DependencyGraph graph;
    init_graph(graph);
    if (! graph.check())
    {
	std::cerr << "graph errors" << std::endl;
	exit(2);
    }

    for (int i = 0; i < 5; ++i)
    {
	// i = 0: fail11, ! silent, ! stop_on_first_error
	// i = 1: fail11, silent, stop_on_first_error
	// i = 2: ! fail11, silent, ! stop_on_first_error
	// i = 3: fail11, silent, ! stop_on_first_error, use_callback
	// i = 4: 3 + disable_failure_propagation
	bool fail11 = (i != 2);
	bool silent = (i != 0);
	bool stop_on_first_error = (i == 1);
	bool disable_failure_propagation = (i == 4);
	bool use_callback = (i >= 3);
	int num_threads = (i >= 3 ? 1 : 50);
	DependencyRunner r(graph, num_threads,
			   boost::bind(process_item, _1, fail11, silent));
	if (use_callback)
	{
	    r.setChangeCallback(change_callback, true);
	}
	bool status = r.run(stop_on_first_error, disable_failure_propagation);
	logger->flushLog();
	if (status)
	{
	    std::cout << "Status: all succeeded" << std::endl;
	}
	else
	{
	    std::cout << "Status: some failed" << std::endl;
	}

	std::cout << "Final States:" << std::endl;
	DependencyEvaluator const& e = r.getEvaluator();
	DependencyGraph::ItemList const& all_items = graph.getSortedGraph();
	if (i == 0)
	{
	    // Print all item states
	    for (DependencyGraph::ItemList::const_iterator iter =
		     all_items.begin();
		 iter != all_items.end(); ++iter)
	    {
		std::cout << *iter << ": " << e.getItemState(*iter) << std::endl;
	    }
	}
	else if (i == 1)
	{
	    // Make sure some are complete, some are failed, and some
	    // are waiting or ready.
	    bool any_completed = false;
	    bool any_failed = false;
	    bool any_waiting_or_ready = false;
	    bool any_other = false;
	    for (DependencyGraph::ItemList::const_iterator iter =
		     all_items.begin();
		 iter != all_items.end(); ++iter)
	    {
		switch (e.getItemState(*iter))
		{
		  case DependencyEvaluator::i_completed:
		    any_completed = true;
		    break;

		  case DependencyEvaluator::i_failed:
		  case DependencyEvaluator::i_depfailed:
		    any_failed = true;
		    break;

		  case DependencyEvaluator::i_ready:
		  case DependencyEvaluator::i_waiting:
		    any_waiting_or_ready = true;
		    break;

		  default:
		    any_other = true;
		    break;
		}
	    }
	    if (any_completed && any_failed && any_waiting_or_ready &&
		(! any_other))
	    {
		std::cout << "states okay" << std::endl;
	    }
	    else
	    {
		std::cout << "states unexpected" << std::endl;
	    }
	}
	else if (i == 2)
	{
	    // Make sure some are items are complete
	    bool okay = true;
	    for (DependencyGraph::ItemList::const_iterator iter =
		     all_items.begin();
		 iter != all_items.end(); ++iter)
	    {
		if (e.getItemState(*iter) != DependencyEvaluator::i_completed)
		{
		    okay = false;
		    break;
		}
	    }
	    if (okay)
	    {
		std::cout << "states okay" << std::endl;
	    }
	    else
	    {
		std::cout << "states unexpected" << std::endl;
	    }
	}
    }

    // Stop logger
    Logger::stopLogger();

    return 0;
}
