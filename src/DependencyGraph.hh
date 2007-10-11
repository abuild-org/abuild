#ifndef __DEPENDENCYGRAPH_HH__
#define __DEPENDENCYGRAPH_HH__

// This object represents a graph of items and their dependencies to
// each other.  Any non-empty string may be used as an item.  If it is
// desired to create a dependency graph of items other than strings,
// use the strings passed to DependencyGraph as keys to look up the
// objects in some other way.  Once items have been added to the
// graph, the graph can be checked for cycles and unknown elements.
// If there are none, then this object can provide recursively
// expanded and topologically sorted dependency lists for any item or
// for the entire graph.  Once a graph has been checked, it becomes
// immutable.  Dependency graphs may be copied.

#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>

class DependencyGraph
{
  public:
    typedef std::string ItemType;
    typedef std::list<ItemType> ItemList;
    typedef std::map<ItemType, ItemList> ItemMap;

    DependencyGraph();

    // Initialization: first add an item, then add its dependencies.
    // These functions may not be called after check() has been
    // called.  An item may be added exactly one time.
    void addItem(ItemType const& item);
    void addDependency(ItemType const& item, ItemType const& dep);

    // Check the graph for unknown elements and cycles.  Return true
    // iff the graph is error-free.  The check() method only does its
    // work the first time it's called.  On subsequent calls, it just
    // returns a boolean indicating whether or not the check was
    // successful.  If unknowns or cycles are detected, details may be
    // retrieved by calling getErrors().  The const version of check
    // may only be called if the graph has previously been checked and
    // can be used to determine whether a previously-checked graph is
    // error-free.
    bool check();
    bool check() const;

    // Querying functions.  These functions may be called only check()
    // has been called.  It is an error to call any of these functions
    // with an item that is not in the graph.  Note that if check()
    // has returned false, not all the information will be completely
    // reliable, but it will be good enough for applications to still
    // do some work based on the results.

    // Returns an item's direct dependencies in the order in which
    // they were added.
    ItemList const& getDirectDependencies(ItemType const& item) const;

    // Returns the list of items that directly depend upon the given
    // item.
    ItemList const& getReverseDependencies(ItemType const& item) const;

    // Returns the recursively expanded list of dependencies of an
    // item in dependency order (topologically sorted).  The item
    // itself is always the last item on the list.
    ItemList const& getSortedDependencies(ItemType const& item) const;

    // Compares two items based on dependency order, using lexical
    // ordering to break ties.  Return -1 if the first item is less
    // than the second item, 0 if they are equal, or 1 if the first
    // item is greater than the second item.
    int compareItems(ItemType const&, ItemType const&) const;

    // Compare two items based on dependency order, returning true iff
    // the first item is less than the second item.
    bool itemLess(ItemType const&, ItemType const&) const;

    // Returns all the items in the graph topologically sorted in
    // dependency order.
    ItemList const& getSortedGraph() const;

    // Error function.  This function may be called only if check()
    // has been called and has returned false.  Initializes "unknowns"
    // to a map from known items to any dependencies they have to
    // unknown items.  Initializes "cycles" to a list of item lists
    // such that each item list contains a series of items that form a
    // cyclic dependency.  The first item in the cycle is always the
    // one that is lexically earliest.
    void getErrors(ItemMap& unknowns,
		   std::vector<ItemList>& cycles) const;

  private:
    // gs = graph state
    enum GraphState { gs_INIT, gs_ERROR, gs_READY };

    // assert that the check() function has or hasn't been called
    // depending on the value.
    void assertChecked(bool) const;

    // Topological sort worker functions
    void topologicalSort();
    void traverse(
	ItemType const& item,
	std::set<ItemType>& visited,
	std::set<std::string>& unknowns_seen,
	std::set<std::string>& cycles_seen,
	ItemList& result);
    void traverseInternal(
	ItemType const& item,
	std::set<ItemType>& visited,
	std::set<ItemType>& visiting,
	ItemList& path,
	std::set<std::string>& unknowns_seen,
	std::set<std::string>& cycles_seen,
	ItemList& result);

    GraphState graph_state;

    // Data for the normal case
    ItemMap dependencies;
    ItemMap sorted_dependencies;
    ItemMap reverse_dependencies;
    ItemList in_order;
    std::map<ItemType, unsigned int> item_order;

    // Error data -- these are empty if there were no errors.  The
    // formats of these are described in the comments for the
    // getErrors method above.
    ItemMap unknowns;
    std::vector<ItemList> cycles;
};

#endif // __DEPENDENCYGRAPH_HH__
