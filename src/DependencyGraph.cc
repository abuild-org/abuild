#include <DependencyGraph.hh>

#include <assert.h>
#include <boost/bind.hpp>

DependencyGraph::DependencyGraph() :
    graph_state(gs_INIT)
{
}

void
DependencyGraph::addItem(ItemType const& item)
{
    this->graph_state = gs_INIT;
    assert(this->dependencies.count(item) == 0);
    assert(! item.empty());
    this->dependencies[item] = ItemList();
    this->reverse_dependencies[item] = ItemList();
}

void
DependencyGraph::addDependency(ItemType const& item, ItemType const& dep)
{
    this->graph_state = gs_INIT;
    assert(this->dependencies.count(item) != 0);
    assert(! dep.empty());
    this->dependencies[item].push_back(dep);
}

DependencyGraph::ItemList const&
DependencyGraph::getDirectDependencies(ItemType const& item) const
{
    assertChecked(true);
    assert(this->dependencies.count(item) != 0);
    return (*(dependencies.find(item))).second;
}

DependencyGraph::ItemList const&
DependencyGraph::getSortedDependencies(ItemType const& item) const
{
    assertChecked(true);
    assert(this->sorted_dependencies.count(item) != 0);
    return (*(sorted_dependencies.find(item))).second;
}

DependencyGraph::ItemList const&
DependencyGraph::getReverseDependencies(ItemType const& item) const
{
    assertChecked(true);
    assert(this->reverse_dependencies.count(item) != 0);
    return (*(reverse_dependencies.find(item))).second;
}

int
DependencyGraph::compareItems(ItemType const& i1, ItemType const& i2) const
{
    assertChecked(true);
    assert(this->item_order.count(i1) != 0);
    assert(this->item_order.count(i2) != 0);
    unsigned int i1n = (*(this->item_order.find(i1))).second;
    unsigned int i2n = (*(this->item_order.find(i2))).second;
    if (i1n < i2n)
    {
	return -1;
    }
    else if (i1n > i2n)
    {
	return 1;
    }
    else
    {
	return 0;
    }
}

bool
DependencyGraph::itemLessFunction(ItemType const& i1, ItemType const& i2) const
{
    return compareItems(i1, i2) == -1;
}

boost::function<bool (DependencyGraph::ItemType const&,
		      DependencyGraph::ItemType const&)>
DependencyGraph::itemLess() const
{
    return boost::bind(&DependencyGraph::itemLessFunction, this, _1, _2);
}

DependencyGraph::ItemList const&
DependencyGraph::getSortedGraph() const
{
    assertChecked(true);
    return this->in_order;
}

std::vector<DependencyGraph::ItemList> const&
DependencyGraph::getIndependentSets()
{
    assertChecked(true);
    assert(check());
    if (! this->independent_sets.empty())
    {
	return this->independent_sets;
    }

    std::map<ItemType, int> set_numbers;
    int next_set = 0;

    // Assign each item to an independent set by traversing the graph
    // in both directions from any node that does not already appear
    // in a set.
    for (ItemList::const_iterator iter = in_order.begin();
	 iter != in_order.end(); ++iter)
    {
	ItemType const& node = *iter;
	if (set_numbers.count(node) == 0)
	{
	    int set = next_set++;
	    ItemList nodes;
	    nodes.push_back(node);
	    while (! nodes.empty())
	    {
		ItemType const& node = nodes.front();
		if (! set_numbers.count(node))
		{
		    set_numbers[node] = set;
		    ItemList const& deps = getDirectDependencies(node);
		    nodes.insert(nodes.end(), deps.begin(), deps.end());
		    ItemList const& rdeps = getReverseDependencies(node);
		    nodes.insert(nodes.end(), rdeps.begin(), rdeps.end());
		}
		nodes.pop_front();
	    }
	}
    }

    // Gather nodes in groups based on their set numbers
    std::map<int, std::vector<ItemType> > sets;
    for (std::map<ItemType, int>::iterator iter = set_numbers.begin();
	 iter != set_numbers.end(); ++iter)
    {
	sets[(*iter).second].push_back((*iter).first);
    }

    // Generate final results
    for (std::map<int, std::vector<ItemType> >::iterator iter = sets.begin();
	 iter != sets.end(); ++iter)
    {
	std::vector<ItemType>& nodes = (*iter).second;
	std::sort(nodes.begin(), nodes.end(), itemLess());
	this->independent_sets.push_back(ItemList());
	ItemList& nodelist = this->independent_sets.back();
	nodelist.insert(nodelist.end(), nodes.begin(), nodes.end());
    }

    return this->independent_sets;
}

void
DependencyGraph::assertChecked(bool val) const
{
    assert(val == (this->graph_state != gs_INIT));
}

void
DependencyGraph::getErrors(ItemMap& unknowns,
			   std::vector<ItemList>& cycles) const
{
    assertChecked(true);
    unknowns = this->unknowns;
    cycles = this->cycles;
}

// "Check" and friends are the workhorse methods of this class.

bool
DependencyGraph::check()
{
    if (this->graph_state != gs_INIT)
    {
	return (this->graph_state == gs_READY);
    }

    // We precluded an item whose name was the empty string during
    // creation, but check again just to be extra safe.
    assert(this->dependencies.count("") == 0);

    // Create a item whose name is the empty string and make it depend
    // upon all the other items.  The result of sorting the graph from
    // there is a topological sort of the entire graph.
    this->dependencies[""] = ItemList();
    for (ItemMap::iterator iter = this->dependencies.begin();
	 iter != this->dependencies.end(); ++iter)
    {
	if (! (*iter).first.empty())
	{
	    this->dependencies[""].push_back((*iter).first);
	}
    }

    // Perform a sort on the whole graph
    topologicalSort();

    // Our in-order item list is the sort of the graph from the empty
    // string except for the empty string itself.
    this->in_order = this->sorted_dependencies[""];
    assert(this->in_order.back().empty());
    this->in_order.pop_back();

    // Remove the empty string item
    this->dependencies.erase("");
    this->sorted_dependencies.erase("");

    // Initialize the item_order map so we can easily compare items by
    // their positions in the sorted list.
    { // local scope
	int pos = 0;
	for (ItemList::iterator iter = this->in_order.begin();
	     iter != this->in_order.end(); ++iter)
	{
	    this->item_order[*iter] = ++pos;
	}
    }

    // Set up table of reverse dependencies.  We've already guaranteed
    // in addItem that every item will appear in the table.
    for (ItemMap::iterator item = this->dependencies.begin();
	 item != this->dependencies.end(); ++item)
    {
	for (ItemList::iterator dep = (*item).second.begin();
	     dep != (*item).second.end(); ++dep)
	{
	    this->reverse_dependencies[*dep].push_back((*item).first);
	}
    }

    if (! (this->unknowns.empty() && this->cycles.empty()))
    {
	this->graph_state = gs_ERROR;
    }
    else
    {
	this->graph_state = gs_READY;
    }

    return (this->graph_state == gs_READY);
}

bool
DependencyGraph::check() const
{
    assertChecked(true);
    return (this->graph_state == gs_READY);
}

void
DependencyGraph::topologicalSort()
{
    // Traverse the graph separately from each node keeping track of
    // unknowns and cycles globally.

    std::set<std::string> unknowns_seen;
    std::set<std::string> cycles_seen;
    for (ItemMap::iterator iter = this->dependencies.begin();
	 iter != this->dependencies.end(); ++iter)
    {
	ItemType const& item = (*iter).first;
	std::set<ItemType> visited;
	ItemList result;
	traverse(item, visited, unknowns_seen, cycles_seen, result);
	this->sorted_dependencies[item] = result;
    }
}

void
DependencyGraph::traverse(ItemType const& item,
			  std::set<ItemType>& visited,
			  std::set<std::string>& unknowns_seen,
			  std::set<std::string>& cycles_seen,
			  ItemList& result)
{
    // Traverse the graph from the given node locally keeping track of
    // any nodes we are traversing through.
    std::set<ItemType> visiting;
    ItemList path;
    traverseInternal(item, visited, visiting, path,
		     unknowns_seen, cycles_seen, result);
}

void
DependencyGraph::traverseInternal(
    ItemType const& item,
    std::set<ItemType>& visited,
    std::set<ItemType>& visiting,
    ItemList& path,
    std::set<std::string>& unknowns_seen,
    std::set<std::string>& cycles_seen,
    ItemList& result)
{
    // Detect the case of a node that doesn't exist.
    if (this->dependencies.count(item) == 0)
    {
	// Generate a string to use to keep track of whether we've
	// seen this error before.  The string is constructed such
	// that it is an unambiguous representation of the error
	// condition regardless of the names of the nodes.
	std::string unknown = "(" + path.back() + ") -> (" + item + ")";
	if (unknowns_seen.count(unknown) == 0)
	{
	    unknowns_seen.insert(unknown);
	    this->unknowns[path.back()].push_back(item);
	}
	return;
    }

    // If we've already visited this item, just return.  It's a normal
    // case for there to be more than one path to an indirect
    // dependency.
    if (visited.count(item))
    {
	return;
    }

    // If we're already visiting this node, we've found a cycle.
    if (visiting.count(item))
    {
	// Make a copy of the current path and remove items from the
	// front until we get to the current item.  That will narrow
	// us down to the list of items in the cycle.
	ItemList cycle = path;
	while (cycle.front() != item)
	{
	    cycle.pop_front();
	}

	// Canonicalize cycle order so that the node that sorts first
	// is listed first.  This enables us to avoid reporting the
	// same cycle more than once.  (VC7.1 doesn't have
	// std::min_element, so we have to code it ourselves.)
	std::string min = cycle.front();
	for (ItemList::iterator iter = cycle.begin();
	     iter != cycle.end(); ++iter)
	{
	    if (*iter < min)
	    {
		min = *iter;
	    }
	}
	while (cycle.front() != min)
	{
	    std::string t = cycle.front();
	    cycle.pop_front();
	    cycle.push_back(t);
	}

	// Generate a string that describes the cycle and store it so
	// we can avoid reporting a cycle more than once.  The string
	// is constructed to be an unambiguous representation of the
	// error condition regardless of the names of the nodes.
	std::string cycle_str;
	for (ItemList::iterator citer = cycle.begin();
	     citer != cycle.end(); ++citer)
	{
	    cycle_str += "(" + *citer + ") -> ";
	}
	if (cycles_seen.count(cycle_str) == 0)
	{
	    cycles_seen.insert(cycle_str);
	    this->cycles.push_back(cycle);
	}
	return;
    }

    // Mark the current item as in progress and add it to the path.
    visiting.insert(item);
    path.push_back(item);

    // Traverse to each neighbor of this item.
    ItemList const& neighbors = dependencies[item];
    for (ItemList::const_iterator neighbor = neighbors.begin();
	 neighbor != neighbors.end(); ++neighbor)
    {
	traverseInternal(*neighbor, visited, visiting,
			 path, unknowns_seen, cycles_seen, result);
    }

    // Indicate that we have finished with this node.
    visiting.erase(item);
    path.pop_back();

    visited.insert(item);
    result.push_back(item);
}
