//
// This is a port of DependencyGraph.cc to groovy.  It is tested by
// the same test suite as the C++ version.
//

package org.abuild.groovy

class DependencyGraph
{
    private final int gs_INIT = 0
    private final int gs_ERROR = 1
    private final int gs_READY = 2

    private graphState = gs_INIT

    // Note: as of 1.5.7, groovy still ignores "private".  Use the
    // accessors defined below to access elements of these maps in
    // order to get cause proper assertions to be called.

    // Data for the normal case.  item -> [item, ...]
    private final dependencies = [:]
    private final sortedDependencies = [:]
    private final reverseDependencies = [:]

    // [item, ...]
    final sortedGraph = []
    // item -> order
    final itemOrder = [:]

    // Error data -- these are empty if there were no errors.
    // Otherwise, they are initialized after check() has been called.
    // "unknowns" maps each item to the list of unknown items they
    // depend on.  Each element of "cycles" is a list of elements that
    // form a cyclic dependency.  The first item in the cycle is
    // always the one that is lexically earliest.
    private final unknowns = [:]
    private final cycles = []

    // Initialization: first add an item, then add its dependencies.
    // These functions may not be called after check() has been
    // called.  An item may be added exactly one time.
    void addItem(String item)
    {
        this.graphState = gs_INIT
        assert ! this.dependencies.containsKey(item)
        assert '' != item
        this.dependencies[item] = []
        this.reverseDependencies[item] = []
    }

    void addDependency(String item, String dep)
    {
        this.graphState = gs_INIT
        assert this.dependencies.containsKey(item)
        assert '' != dep
        this.dependencies[item] << dep
    }

    // Check the graph for unknown elements and cycles.  Return true
    // iff the graph is error-free.  The check() method only does its
    // work the first time it's called.  On subsequent calls, it just
    // returns a boolean indicating whether or not the check was
    // successful.  If unknowns or cycles are detected, details may be
    // retrieved by inspecting the unknowns and cycles properties.
    boolean check()
    {
        if (this.graphState != gs_INIT)
        {
            return (this.graphState == gs_READY)
        }

        // Clear things out from possible previous checks
        this.sortedDependencies.keySet().each {
            this.sortedDependencies[it].clear()
        }
        this.reverseDependencies.keySet().each {
            this.reverseDependencies[it].clear()
        }
        this.unknowns.clear()
        this.cycles.clear()
        this.itemOrder.clear()

        // We precluded an item whose name was the empty string during
        // creation, but check again just to be extra safe.
        assert ! this.dependencies.containsKey("")

        // Create a item whose name is the empty string and make it depend
        // upon all the other items.  The result of sorting the graph from
        // there is a topological sort of the entire graph.
        this.dependencies[""] =
            this.dependencies.keySet().sort().grep { it != "" }

        // Perform a sort on the whole graph
        topologicalSort()

        // Our sortedGraph item list is the sort of the graph from the
        // empty string except for the empty string itself.
        this.sortedGraph = this.sortedDependencies[""]
        assert '' == this.sortedGraph[-1]
        this.sortedGraph.pop()

        // Remove the empty string item
        this.dependencies.remove("")
        this.sortedDependencies.remove("")

        // Initialize the itemOrder map so we can easily compare items by
        // their positions in the sorted list.
        this.sortedGraph.eachWithIndex() {
            item, pos ->
            this.itemOrder[item] = pos
        }

        // Set up table of reverse dependencies.
        for (item in this.dependencies.keySet())
        {
            for (dep in this.dependencies[item])
            {
                if (! this.reverseDependencies.containsKey(dep))
                {
                    this.reverseDependencies[dep] = []
                }
                this.reverseDependencies[dep] << item
            }
        }

        if (! (this.unknowns.isEmpty() && this.cycles.isEmpty()))
        {
            this.graphState = gs_ERROR
        }
        else
        {
            this.graphState = gs_READY
        }

        return (this.graphState == gs_READY)
    }

    // Querying functions.  These functions may be called only check()
    // has been called.  It is an error to call any of these functions
    // with an item that is not in the graph.  Note that if check()
    // has returned false, not all the information will be completely
    // reliable, but it will be good enough for applications to still
    // do some work based on the results.

    // Returns an item's direct dependencies in the order in which
    // they were added.
    def getDirectDependencies(String item)
    {
        assertChecked(true)
        assert this.dependencies.containsKey(item)
        this.dependencies[item]
    }

    // Returns the list of items that directly depend upon the given
    // item.
    def getReverseDependencies(String item)
    {
        assertChecked(true)
        assert this.reverseDependencies.containsKey(item)
        this.reverseDependencies[item]
    }

    // Returns the recursively expanded list of dependencies of an
    // item in dependency order (topologically sorted).  The item
    // itself is always the last item on the list.
    def getSortedDependencies(String item)
    {
        assertChecked(true)
        assert this.sortedDependencies.containsKey(item)
        this.sortedDependencies[item]
    }

    // Compares two items based on dependency order, using lexical
    // ordering to break ties.  Return -1 if the first item is less
    // than the second item, 0 if they are equal, or 1 if the first
    // item is greater than the second item.
    int compareItems(String i1, String i2)
    {
        assertChecked(true)
        assert this.itemOrder.containsKey(i1)
        assert this.itemOrder.containsKey(i2)
        itemOrder[i1] <=> itemOrder[i2]
    }

    // assert that the check() function has or hasn't been called
    // depending on the value.
    private void assertChecked(boolean val)
    {
        assert ((this.graphState != gs_INIT) == val)
    }

    // Topological sort worker functions
    private void topologicalSort()
    {
        // Traverse the graph separately from each node keeping track
        // of unknowns and cycles globally.

        def unknowns_seen = [:]
        def cycles_seen = [:]
        for (item in dependencies.keySet().sort())
        {
            def visited = [:]
            def result = []
            traverse(item, visited, unknowns_seen, cycles_seen, result)
            this.sortedDependencies[item] = result
        }
    }

    private void traverse(String item, visited, unknowns_seen, cycles_seen,
                          result)
    {
        // Traverse the graph from the given node locally keeping
        // track of any nodes we are traversing through.
        def visiting = [:]
        def path = []
        traverseInternal(item, visited, visiting, path,
                         unknowns_seen, cycles_seen, result)
    }

    private void traverseInternal(String item, visited, visiting, path,
                                  unknowns_seen, cycles_seen, result)
    {
        // Detect the case of a node that doesn't exist.
        if (! this.dependencies.containsKey(item))
        {
            // Generate a string to use to keep track of whether we've
            // seen this error before.  The string is constructed such
            // that it is an unambiguous representation of the error
            // condition regardless of the names of the nodes.
            def unknown = "(" + path[-1] + ") -> (" + item + ")"
            if (! unknowns_seen.containsKey(unknown))
            {
                unknowns_seen[unknown] = 1
                if (! this.unknowns.containsKey(path[-1]))
                {
                    this.unknowns[path[-1]] = []
                }
                this.unknowns[path[-1]] << item
            }
            return
        }

        // If we've already visited this item, just return.  It's a normal
        // case for there to be more than one path to an indirect
        // dependency.
        if (visited.containsKey(item))
        {
            return
        }

        // If we're already visiting this node, we've found a cycle.
        if (visiting.containsKey(item))
        {
            // Make a copy of the current path and remove items from
            // the front until we get to the current item.  That will
            // narrow us down to the list of items in the cycle.
            def cycle = [*path]
            while (cycle[0] != item)
            {
                cycle.remove(0)
            }

            // Canonicalize cycle order so that the node that sorts first
            // is listed first.  This enables us to avoid reporting the
            // same cycle more than once.
            def min = cycle.min()
            while (cycle[0] != min)
            {
                cycle << cycle[0]
                cycle.remove(0)
            }

            // Generate a string that describes the cycle and store it so
            // we can avoid reporting a cycle more than once.  The string
            // is constructed to be an unambiguous representation of the
            // error condition regardless of the names of the nodes.
            def cycle_str = ""
            for (citem in cycle)
            {
                cycle_str += "(${citem}) -> "
            }
            if (! cycles_seen.containsKey(cycle_str))
            {
                cycles_seen[cycle_str] = 1
                this.cycles << cycle
            }

            return
        }

        // Mark the current item as in progress and add it to the path.
        visiting[item] = 1
        path << item

        // Traverse to each neighbor of this item.
        for (neighbor in dependencies[item])
        {
            traverseInternal(neighbor, visited, visiting,
                             path, unknowns_seen, cycles_seen, result)
        }

        // Indicate that we have finished with this node.
        visiting.remove(item)
        path.pop()

        visited[item] = 1
        result << item
    }
}
