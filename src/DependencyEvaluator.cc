#include "DependencyEvaluator.hh"

#include <assert.h>

DependencyEvaluator::DependencyEvaluator(DependencyGraph const& graph) :
    graph(graph),
    propagate_failure(true),
    num_running(0),
    change_callback(0)
{
    // Make sure the graph is error-free.
    assert(graph.check());

    // Initialize item states to ready if they have no dependencies or
    // waiting otherwise.  Don't use setItemState here since the old
    // states are not meaningful.  Anyway, there's no opportunity to
    // set a change callback before now.
    DependencyGraph::ItemList const& in_order = graph.getSortedGraph();
    for (DependencyGraph::ItemList::const_iterator iter = in_order.begin();
	 iter != in_order.end(); ++iter)
    {
	if (graph.getDirectDependencies(*iter).empty())
	{
	    this->states[*iter] = i_ready;
	}
	else
	{
	    this->states[*iter] = i_waiting;
	}
    }
}

std::string
DependencyEvaluator::unparseState(ItemState state)
{
    std::string str;

    // no default see gcc will warn for missing cases
    switch (state)
    {
      case i_waiting:
	str = "waiting";
	break;

      case i_ready:
	str = "ready";
	break;

      case i_running:
	str = "running";
	break;

      case i_completed:
	str = "completed";
	break;

      case i_failed:
	str = "failed";
	break;

      case i_depfailed:
	str = "dependency-failed";
	break;
    }

    return str;
}

DependencyGraph const&
DependencyEvaluator::getGraph() const
{
    return this->graph;
}

void
DependencyEvaluator::disableFailurePropagation()
{
    this->propagate_failure = false;
}

void
DependencyEvaluator::setRunning(ItemType const& item)
{
    assertState(item, i_ready);
    setItemState(item, i_running);
    ++this->num_running;
}

void
DependencyEvaluator::setCompleted(ItemType const& item)
{
    assertState(item, i_running);
    setItemState(item, i_completed);
    --this->num_running;
    evaluateReverseDependencies(item);
}

void
DependencyEvaluator::setFailed(ItemType const& item)
{
    assertState(item, i_running);
    setItemState(item, i_failed);
    --this->num_running;
    evaluateReverseDependencies(item);
}

bool
DependencyEvaluator::nextReadyItem(ItemType& item) const
{
    DependencyGraph::ItemList const& in_order = graph.getSortedGraph();
    for (DependencyGraph::ItemList::const_iterator iter = in_order.begin();
	 iter != in_order.end(); ++iter)
    {
	if ((*(this->states.find(*iter))).second == i_ready)
	{
	    item = *iter;
	    return true;
	}
    }
    return false;
}

int
DependencyEvaluator::numRunning() const
{
    // Note that we can keep num_running as a count rather than
    // counting through item_states.  The only an item can move into
    // or out of the running state is through one of setRunning,
    // setCompleted, or setFailed.  The only other thing that changes
    // states is evaluate.  It never changes a state to running, and
    // evaluate is only called on items that are in the waiting,
    // completed, or failed state.
    return this->num_running;
}

DependencyEvaluator::ItemState
DependencyEvaluator::getItemState(ItemType const& item) const
{
    assert(this->states.count(item) != 0);
    return (*(this->states.find(item))).second;
}

void
DependencyEvaluator::setChangeCallback(ChangeCallback callback, bool call_now)
{
    this->change_callback = callback;
    if (call_now)
    {
	for (std::map<ItemType, ItemState>::const_iterator iter =
		 this->states.begin();
	     iter != this->states.end(); ++iter)
	{
	    this->change_callback((*iter).first, (*iter).second);
	}
    }
}

void
DependencyEvaluator::evaluate(ItemType const& item)
{
    // This routine looks at the given item and sets its state based
    // on the states of its dependencies as described in the comments
    // at the top of DependencyEvaluator.hh.  When an item's state is
    // set to failed, that information is automatically propagated
    // recursively to its reverse dependencies.

    ItemState istate = this->states[item];
    if ((istate == i_running) ||
	(istate == i_completed) ||
	(istate == i_failed) ||
	(istate == i_depfailed))
    {
	// No need to re-evaluate
	return;
    }
    bool failed = false;
    bool blocked = false;
    DependencyGraph::ItemList const& deps =
	this->graph.getDirectDependencies(item);
    for (DependencyGraph::ItemList::const_iterator dep = deps.begin();
	 dep != deps.end(); ++dep)
    {
	ItemState state = this->states[*dep];
	if ((state == i_failed) || (state == i_depfailed))
	{
	    if (this->propagate_failure)
	    {
		failed = true;
	    }
	    break;
	}
	else if (state != i_completed)
	{
	    blocked = true;
	}
    }
    if (failed)
    {
	setItemState(item, i_depfailed);
	evaluateReverseDependencies(item);
    }
    else if (! blocked)
    {
	setItemState(item, i_ready);
    }
}

void
DependencyEvaluator::evaluateReverseDependencies(ItemType const& item)
{
    DependencyGraph::ItemList const& rdeps =
	this->graph.getReverseDependencies(item);
    for (DependencyGraph::ItemList::const_iterator iter = rdeps.begin();
	 iter != rdeps.end(); ++iter)
    {
	evaluate(*iter);
    }
}

void
DependencyEvaluator::assertState(ItemType const& item, ItemState state)
{
    assert(this->states.count(item) != 0);
    assert(this->states[item] == state);
}

void
DependencyEvaluator::setItemState(std::string const& item, ItemState state)
{
    if (this->states[item] != state)
    {
	this->states[item] = state;
	if (this->change_callback)
	{
	    this->change_callback(item, state);
	}
    }
}
