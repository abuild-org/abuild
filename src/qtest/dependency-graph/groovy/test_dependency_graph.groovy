import org.abuild.groovy.DependencyGraph

def report(DependencyGraph g)
{
    boolean okay = g.check()
    def items = g.sortedGraph
    println "Graph:"
    for (item in items)
    {
        print "${item}:"
        def deps = g.getDirectDependencies(item)
        if (! deps.isEmpty())
        {
            print " "
            print deps.join(" ")
        }
        println ""
    }

    if (okay)
    {
        println "No errors found."

        def vec = [*items]
        vec.sort()

        print "Lexical: "
        println vec.join(" ")

        vec.sort(g.&compareItems)

        print "Dependency order: "
        println vec.join(" ")
    }
    else
    {
        println "Errors found:"
        if (! g.unknowns.isEmpty())
        {
            println "  Unknowns:"
            for (u in g.unknowns.keySet())
            {
                print "    ${u} -> "
                println g.unknowns[u].join(" ")
            }
        }
        if (! g.cycles.isEmpty())
        {
            println "  Cycles:"
            for (c in g.cycles)
            {
                print "    "
                print c.join(" -> ")
                println " -> " + c[0]
            }
        }
    }

    for (item in items)
    {
        print "sort(${item}) ="
        def sdeps = g.getSortedDependencies(item)
        if (! sdeps.isEmpty())
        {
            print " "
            print sdeps.join(" ")
        }
        println ""
    }
    for (item in items)
    {
        print "rdeps(${item}) ="
        def rdeps = g.getReverseDependencies(item)
        if (! rdeps.isEmpty())
        {
            print " "
            print rdeps.join(" ")
        }
        println ""
    }
}

// Create a correct graph.
def g1 = new DependencyGraph()
g1.addItem("a")
g1.addDependency("a", "b")
g1.addDependency("a", "c")
g1.addItem("b")
g1.addDependency("b", "d")
g1.addDependency("b", "e")
g1.addDependency("b", "f")
g1.addItem("c")
g1.addDependency("c", "g")
g1.addDependency("c", "h")
g1.addItem("d")
g1.addItem("e")
g1.addDependency("e", "p")
g1.addDependency("e", "q")
g1.addItem("f")
g1.addDependency("f", "q")
g1.addItem("g")
g1.addItem("h")
g1.addItem("i")
g1.addDependency("i", "c")
g1.addDependency("i", "j")
g1.addItem("j")
g1.addDependency("j", "k")
g1.addDependency("j", "l")
g1.addDependency("j", "m")
g1.addItem("k")
g1.addDependency("k", "n")
g1.addDependency("k", "o")
g1.addItem("l")
g1.addItem("m")
g1.addItem("n")
g1.addItem("o")
g1.addDependency("o", "q")
g1.addItem("p")
g1.addItem("q")
g1.addDependency("q", "r")
g1.addItem("r")

report(g1)
println ""

// Now create an erroneous graph.  Make cyclic by adding edges
// pointing from Q to B, Q to J, and F to B, and make erroneous by
// having a link added from D to (unknown nodes) W and X and from
// G to unknown node W.
DependencyGraph g2 = g1
g2.addDependency("q", "b")
g2.addDependency("q", "j")
g2.addDependency("f", "b")
g2.addDependency("d", "w")
g2.addDependency("d", "x")
g2.addDependency("g", "w")

report(g2)
