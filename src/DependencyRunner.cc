#include <DependencyRunner.hh>

DependencyRunner::DependencyRunner(
    DependencyGraph const& g,
    int num_workers,
    boost::function<bool (std::string)> const& method) :

    evaluator(g),
    pool(num_workers, method)
{
}

DependencyEvaluator const&
DependencyRunner::getEvaluator() const
{
    return this->evaluator;
}

DependencyGraph const&
DependencyRunner::getGraph() const
{
    return this->evaluator.getGraph();
}

void
DependencyRunner::setChangeCallback(DependencyEvaluator::ChangeCallback callback,
				    bool call_now)
{
    this->evaluator.setChangeCallback(callback, call_now);
}

bool
DependencyRunner::run(bool stop_on_first_error,
		      bool disable_failure_propagation)
{
    bool done = false;
    bool any_errors = false;
    bool stop = false;
    if (disable_failure_propagation)
    {
	evaluator.disableFailurePropagation();
    }
    while (! done)
    {
	std::string item;
	if (evaluator.nextReadyItem(item))
	{
	    pool.wait();
	}
	else if (! pool.idle())
	{
	    pool.waitForResults();
	}
	while (pool.resultsAvailable())
	{
	    boost::tuple<std::string, bool> result = pool.getResults();
	    std::string const& finished_item = result.get<0>();
	    bool status = result.get<1>();
	    if (status)
	    {
		evaluator.setCompleted(finished_item);
	    }
	    else
	    {
		any_errors = true;
		if (stop_on_first_error)
		{
		    stop = true;
		}
		evaluator.setFailed(finished_item);
	    }
	}

	if (! stop)
	{
	    // Don't process any new items if any results are
	    // available.  Get and process the results first since
	    // processing the results could potentially change which
	    // item we would process next.  Failing to do this doesn't
	    // actually cause anything to get processed before it's
	    // ready, but it does potentially cause things to be
	    // processed in a non-determistic order which makes
	    // testing very difficult.
	    while ((! pool.resultsAvailable()) &&
		   pool.availableWorkers() && evaluator.nextReadyItem(item))
	    {
		evaluator.setRunning(item);
		pool.processItem(item);
	    }
	}

	if (pool.idle() && (stop || (! evaluator.nextReadyItem(item))))
	{
	    done = true;
	}
    }
    pool.joinThreads();

    return (! any_errors);
}
