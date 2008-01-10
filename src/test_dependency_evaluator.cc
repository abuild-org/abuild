#include "DependencyEvaluator.hh"

#include <iostream>

static void dump_graph(DependencyGraph const& g)
{
    DependencyGraph::ItemList const& items = g.getSortedGraph();
    std::cout << "Graph:" << std::endl;
    for (DependencyGraph::ItemList::const_iterator iter = items.begin();
	 iter != items.end(); ++iter)
    {
	std::cout << "  " << *iter << ":";
	DependencyGraph::ItemList const& deps =
	    g.getDirectDependencies(*iter);
	for (DependencyGraph::ItemList::const_iterator diter = deps.begin();
	     diter != deps.end(); ++diter)
	{
	    std::cout << " " << *diter;
	}
	std::cout << std::endl;
    }
}

static void dump_states(DependencyEvaluator const& e)
{
    DependencyGraph::ItemList const& items = e.getGraph().getSortedGraph();
    std::cout << "States:" << std::endl;
    for (DependencyGraph::ItemList::const_iterator iter = items.begin();
	 iter != items.end(); ++iter)
    {
	std::cout << "  " << *iter << ": "
		  << e.getItemState(*iter) << std::endl;
    }
}

static void run(DependencyEvaluator& e, bool error)
{
    std::cout << "Running evaluator" << std::endl;
    dump_states(e);
    bool done = false;
    while (! done)
    {
	std::set<DependencyEvaluator::ItemType> running;
	DependencyEvaluator::ItemType item;
	while (e.nextReadyItem(item))
	{
	    std::cout << "item " << item << " is ready; running item"
		      << std::endl;
	    e.setRunning(item);
	    running.insert(item);
	}
	if (e.numRunning() == 0)
	{
	    std::cout << "all items are done" << std::endl;
	    done = true;
	}
	else
	{
	    std::cout << "no more items are ready;"
		      << " marking running items complete"
		      << std::endl;
	    for (std::set<DependencyEvaluator::ItemType>::iterator iter =
		     running.begin();
		 iter != running.end(); ++iter)
	    {
		if (error && (*iter == "o"))
		{
		    std::cout << "marking " << *iter << " as failed"
			      << std::endl;
		    e.setFailed(*iter);
		}
		else
		{
		    e.setCompleted(*iter);
		}
	    }
	    dump_states(e);
	}
    }
}

int main()
{
    // Create a correct graph.
    DependencyGraph g;
    g.addItem("a");
    g.addDependency("a", "b");
    g.addDependency("a", "c");
    g.addItem("b");
    g.addDependency("b", "d");
    g.addDependency("b", "e");
    g.addDependency("b", "f");
    g.addItem("c");
    g.addDependency("c", "g");
    g.addDependency("c", "h");
    g.addItem("d");
    g.addItem("e");
    g.addDependency("e", "p");
    g.addDependency("e", "q");
    g.addItem("f");
    g.addDependency("f", "q");
    g.addItem("g");
    g.addItem("h");
    g.addItem("i");
    g.addDependency("i", "c");
    g.addDependency("i", "j");
    g.addItem("j");
    g.addDependency("j", "k");
    g.addDependency("j", "l");
    g.addDependency("j", "m");
    g.addItem("k");
    g.addDependency("k", "n");
    g.addDependency("k", "o");
    g.addItem("l");
    g.addItem("m");
    g.addItem("n");
    g.addItem("o");
    g.addDependency("o", "q");
    g.addItem("p");
    g.addItem("q");
    g.addDependency("q", "r");
    g.addItem("r");

    if (! g.check())
    {
	std::cerr << "graph check failed" << std::endl;
	exit(2);
    }

    dump_graph(g);

    DependencyEvaluator e1(g);
    run(e1, false);
    std::cout << std::endl;
    DependencyEvaluator e2(g);
    run(e2, true);
    std::cout << std::endl;
    DependencyEvaluator e3(g);
    e3.disableFailurePropagation();
    run(e3, true);

    return 0;
}
