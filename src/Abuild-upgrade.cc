// Methods in Abuild class concerned with --upgrade-trees

#include <Abuild.hh>

#include <assert.h>
#include <fstream>
#include <algorithm>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <ItemConfig.hh>

bool
Abuild::upgradeTrees()
{
    info("searching for build items...");
    std::map<std::string, bool> item_dirs;
    findItems(item_dirs);

    info("discovering relationships among trees...");

    DependencyGraph root_graph;
    constructTreeGraph(item_dirs, root_graph);

    // Do identification and output of islands before exiting if
    // errors.  This information can be helpful for people resolving
    // errors, and it will also make it possible to use output from
    // this program as part of manual conversion of error cases in
    // abuild's test suite.

    info("identifying groups of independent build trees...");
    std::vector<DependencyGraph::ItemList> const& sets =
	root_graph.getIndependentSets();
    for (unsigned int i = 0; i < sets.size(); ++i)
    {
	info("set number " + Util::intToString(i));
	for (DependencyGraph::ItemList::const_iterator i2 = sets[i].begin();
	     i2 != sets[i].end(); ++i2)
	{
	    info("  " + *i2);
	}
    }

    exitIfErrors();

    return true;
}

void
Abuild::findItems(std::map<std::string, bool>& item_dirs)
{
    std::list<std::string> dirs;
    dirs.push_back(".");
    while (! dirs.empty())
    {
	std::string dir = dirs.front();
	dirs.pop_front();

	// XXX support pruning

	if (Util::isFile(dir + "/" + ItemConfig::FILE_CONF))
	{
	    // XXX deprecation should be disabled
	    ItemConfig* config = readConfig(dir, "");
	    item_dirs[dir] = config->isTreeRoot();
	}

	std::vector<std::string> entries = Util::getDirEntries(dir);
	std::sort(entries.begin(), entries.end());
	for (std::vector<std::string>::iterator iter = entries.begin();
	     iter != entries.end(); ++iter)
	{
	    std::string const& entry = *iter;
	    if ((entry == ".") || (entry == ".."))
	    {
		continue;
	    }
	    std::string fullpath;
	    if (dir != ".")
	    {
		fullpath += dir + "/";
	    }
	    fullpath += entry;
	    if (Util::isDirectory(fullpath))
	    {
		dirs.push_back(fullpath);
	    }
	}
    }
}

void
Abuild::constructTreeGraph(std::map<std::string, bool> const& item_dirs,
			   DependencyGraph& g)
{
    // XXX currently handles only 1.0 logic...
    // XXX deal with backing areas...
    for (std::map<std::string, bool>::const_iterator iter = item_dirs.begin();
	 iter != item_dirs.end(); ++iter)
    {
	std::string const& dir = (*iter).first;
	bool is_root = (*iter).second;
	if (! is_root)
	{
	    continue;
	}
	g.addItem(dir);
	FileLocation location(dir + "/" + ItemConfig::FILE_CONF, 0, 0);
	ItemConfig* config = readConfig(dir, "");
	std::list<ExternalData> const& externals = config->getExternals();
	for (std::list<ExternalData>::const_iterator eiter = externals.begin();
	     eiter != externals.end(); ++eiter)
	{
	    std::string edecl = (*eiter).getDeclaredPath();
	    std::string epath = Util::absToRel(
		Util::canonicalizePath(dir + "/" + edecl));
	    if (! Util::isDirectory(epath))
	    {
		error(location,
		      "external " + edecl + " (" + epath +
		      " relative to current directory)"
		      " does not exist or is not a directory");
	    }
	    else if (! (item_dirs.count(epath) &&
			(*item_dirs.find(epath)).second))
	    {
		if (item_dirs.count(epath))
		{
		    error(location,
			  "external " + edecl + " (" + epath +
			  " relative to current directory) was not found"
			  " as a root below the current directory");
		}
		else
		{
		    // XXX Here we probably want to preserve this
		    // external as an external-dirs.  This could
		    // happen if the external points up above the top
		    // of the start directory or in a pruned area.
		    // Maybe we want to handle those differently.  See
		    // discussion in TODO.
		    error(location, "XXX unknown external");
		}
	    }
	    else
	    {
		g.addDependency(dir, epath);
	    }
	}
    }

    if (! g.check())
    {
	// XXX report errors
	fatal("errors found checking tree-level dependencies");
    }
}
