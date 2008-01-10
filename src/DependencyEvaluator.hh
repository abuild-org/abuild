#ifndef __DEPENDENCYEVALUATOR_HH__
#define __DEPENDENCYEVALUATOR_HH__

// This object is initialized with an error-free dependency graph.  It
// keeps track of the state of each item in the graph based on the
// states of the item's dependencies.  An item's state may be
// "waiting", "ready", "running", "completed", "failed", or
// "depfailed".  An item's state is "ready" when all of its
// dependencies have state "completed".  An item has state "failed"
// when its state is explicitly set to "failed".  Any item has state
// "depfailed" when any of its dependencies have state "failed" or
// "depfailed".  Otherwise, an item's state is "waiting".  A "ready"
// item may have its state explicitly set to "running".  A "running"
// item may have its state explicitly set to "completed" or "failed",
// both of which cause re-evaluation of the states of any reverse
// dependencies.  The DependencyEvaluator may also be asked for the
// first "ready" item in dependency order.  If the flag
// propagate_failure is false, then the "depfailed" state is not used.
// In this case, an item's state is set to "ready" when all of its
// dependencies have state "completed" or "failed".

// A dependency evaluator object may be used only once.

#include <DependencyGraph.hh>
#include <boost/function.hpp>

class DependencyEvaluator
{
  public:
    // item states
    enum ItemState
    {
	i_waiting,		// unsatisfied dependencies
	i_ready,		// ready to process
	i_running,		// being processed
	i_completed,		// completed successfully
	i_failed,		// completed unsuccessfully
	i_depfailed		// omitted because of a dependency failure
    };

    typedef DependencyGraph::ItemType ItemType;
    typedef boost::function<void (ItemType const&, ItemState)> ChangeCallback;

    // A DependencyEvaluator must be constructed with an error-free
    // dependency graph.  The DependencyEvaluator keeps a reference to
    // the graph, so the graph must be kept around as long as the
    // evaluator is kept around.
    explicit DependencyEvaluator(DependencyGraph const& graph);

    // Return a const reference to the graph being used by this
    // evaluator.
    DependencyGraph const& getGraph() const;

    // Turn off failure propagation.  When failure propagation is off,
    // an item is not skipped when its dependencies fail.  In this
    // mode, items become ready once all their dependencies have
    // completed whether successfully or otherwise.  Must be called
    // before changing any item states.
    void disableFailurePropagation();

    // Item state setters.  These automatically re-evaluate the states
    // of reverse dependencies.
    void setRunning(ItemType const&);
    void setCompleted(ItemType const&);
    void setFailed(ItemType const&);

    // Return a count of the number of items in the "running" state.
    int numRunning() const;

    // If any items are "ready", initialize item to the next item in
    // dependency order whose state is "ready", and return true.
    // Otherwise, return false.
    bool nextReadyItem(ItemType& item) const;

    ItemState getItemState(ItemType const&) const;

    // Set a callback to be called every time the state of an item
    // changes.  If the call_now parameter is true, the callback will
    // be called immediately for each item with its current state.
    void setChangeCallback(ChangeCallback callback, bool call_now);

    // Return a string representing the item state
    static std::string unparseState(ItemState);

  private:
    // Determine an item's state based on the states of its
    // dependencies.
    void evaluate(ItemType const&);

    // Evaluate the reverse dependencies of an item.
    void evaluateReverseDependencies(ItemType const& item);

    // Ensure the item's state is as specified
    void assertState(ItemType const&, ItemState);

    // Set an item's state and call the change callback if it changed
    void setItemState(std::string const&, ItemState);

    DependencyGraph const& graph;
    std::map<ItemType, ItemState> states;
    bool propagate_failure;
    int num_running;
    ChangeCallback change_callback;
};

#endif // __DEPENDENCYEVALUATOR_HH__
