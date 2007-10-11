#include <DependencyGraph.hh>
#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>

static void report(DependencyGraph& g)
{
    bool okay = g.check();
    DependencyGraph::ItemList const& items = g.getSortedGraph();
    std::cout << "Graph:" << std::endl;
    for (DependencyGraph::ItemList::const_iterator iter = items.begin();
	 iter != items.end(); ++iter)
    {
	std::cout << *iter << ":";
	DependencyGraph::ItemList const& deps =
	    g.getDirectDependencies(*iter);
	for (DependencyGraph::ItemList::const_iterator diter = deps.begin();
	     diter != deps.end(); ++diter)
	{
	    std::cout << " " << *diter;
	}
	std::cout << std::endl;
    }

    if (okay)
    {
	std::cout << "No errors found." << std::endl;

	std::vector<DependencyGraph::ItemType> vec;
	for (DependencyGraph::ItemList::const_iterator iter = items.begin();
	     iter != items.end(); ++iter)
	{
	    vec.push_back(*iter);
	}

	// Sort the items lexically so that they start off not in
	// dependency order.
	std::sort(vec.begin(), vec.end());
	std::cout << "Lexical:";
	for (std::vector<DependencyGraph::ItemType>::iterator iter =
		 vec.begin();
	     iter != vec.end(); ++iter)
	{
	    std::cout << " " << *iter;
	}
	std::cout << std::endl;

	// Create a predicate based on DependencyGraph::itemLess to
	// sort the items in dependency order.
	boost::function<bool (DependencyGraph::ItemType const&,
			      DependencyGraph::ItemType const&)> pred =
	    boost::bind(&DependencyGraph::itemLess, boost::ref(g), _1, _2);
	std::sort(vec.begin(), vec.end(), pred);
	std::cout << "Dependency order:";
	for (std::vector<DependencyGraph::ItemType>::iterator iter =
		 vec.begin();
	     iter != vec.end(); ++iter)
	{
	    std::cout << " " << *iter;
	}
	std::cout << std::endl;
    }
    else
    {
	std::cout << "Errors found:" << std::endl;
	DependencyGraph::ItemMap unknowns;
	std::vector<DependencyGraph::ItemList> cycles;
	g.getErrors(unknowns, cycles);
	if (! unknowns.empty())
	{
	    std::cout << "  Unknowns:" << std::endl;
	    for (DependencyGraph::ItemMap::iterator iter = unknowns.begin();
		 iter != unknowns.end(); ++iter)
	    {
		std::cout << "    " << (*iter).first << " ->";
		DependencyGraph::ItemList const& errors = (*iter).second;
		for (DependencyGraph::ItemList::const_iterator liter =
			 errors.begin();
		     liter != errors.end(); ++liter)
		{
		    std::cout << " " << *liter;
		}
		std::cout << std::endl;
	    }
	}
	if (! cycles.empty())
	{
	    std::cout << "  Cycles:" << std::endl;
	    for (std::vector<DependencyGraph::ItemList>::iterator iter =
		     cycles.begin();
		 iter != cycles.end(); ++iter)
	    {
		std::cout << "    ";
		DependencyGraph::ItemList const& cycle = *iter;
		for (DependencyGraph::ItemList::const_iterator citer =
			 cycle.begin();
		     citer != cycle.end(); ++citer)
		{
		    std::cout << *citer << " -> ";
		}
		std::cout << cycle.front() << std::endl;
	    }
	}
    }

    for (DependencyGraph::ItemList::const_iterator iter = items.begin();
	 iter != items.end(); ++iter)
    {
	std::cout << "sort(" << *iter << ") =";
	DependencyGraph::ItemList const& sdeps =
	    g.getSortedDependencies(*iter);
	for (DependencyGraph::ItemList::const_iterator diter = sdeps.begin();
	     diter != sdeps.end(); ++diter)
	{
	    std::cout << " " << *diter;
	}
	std::cout << std::endl;
    }
    for (DependencyGraph::ItemList::const_iterator iter = items.begin();
	 iter != items.end(); ++iter)
    {
	std::cout << "rdeps(" << *iter << ") =";
	DependencyGraph::ItemList const& rdeps =
	    g.getReverseDependencies(*iter);
	for (DependencyGraph::ItemList::const_iterator diter = rdeps.begin();
	     diter != rdeps.end(); ++diter)
	{
	    std::cout << " " << *diter;
	}
	std::cout << std::endl;
    }
}

int main()
{
    // Create a correct graph.
    DependencyGraph g1;
    g1.addItem("a");
    g1.addDependency("a", "b");
    g1.addDependency("a", "c");
    g1.addItem("b");
    g1.addDependency("b", "d");
    g1.addDependency("b", "e");
    g1.addDependency("b", "f");
    g1.addItem("c");
    g1.addDependency("c", "g");
    g1.addDependency("c", "h");
    g1.addItem("d");
    g1.addItem("e");
    g1.addDependency("e", "p");
    g1.addDependency("e", "q");
    g1.addItem("f");
    g1.addDependency("f", "q");
    g1.addItem("g");
    g1.addItem("h");
    g1.addItem("i");
    g1.addDependency("i", "c");
    g1.addDependency("i", "j");
    g1.addItem("j");
    g1.addDependency("j", "k");
    g1.addDependency("j", "l");
    g1.addDependency("j", "m");
    g1.addItem("k");
    g1.addDependency("k", "n");
    g1.addDependency("k", "o");
    g1.addItem("l");
    g1.addItem("m");
    g1.addItem("n");
    g1.addItem("o");
    g1.addDependency("o", "q");
    g1.addItem("p");
    g1.addItem("q");
    g1.addDependency("q", "r");
    g1.addItem("r");

    // Now create an erroneous graph.  Make cyclic by adding edges
    // pointing from Q to B, Q to J, and F to B, and make erroneous by
    // having a link added from D to (unknown nodes) W and X and from
    // G to unknown node W.
    DependencyGraph g2 = g1;
    g2.addDependency("q", "b");
    g2.addDependency("q", "j");
    g2.addDependency("f", "b");
    g2.addDependency("d", "w");
    g2.addDependency("d", "x");
    g2.addDependency("g", "w");

    report(g1);
    std::cout << std::endl;
    report(g2);

    return 0;
}
