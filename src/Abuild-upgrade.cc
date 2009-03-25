// Methods in Abuild class concerned with --upgrade-trees

#include <Abuild.hh>

#include <assert.h>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <ItemConfig.hh>
#include <UpgradeData.hh>
#include <BackingConfig.hh>

bool
Abuild::upgradeTrees()
{
    // Creating UpgradeData object reads the upgrade configuration file.
    UpgradeData ud(this->error_handler);
    exitIfErrors();

    info("searching for build items...");
    findBuildItems(ud);
    if (! ud.upgrade_required)
    {
	QTC::TC("abuild", "Abuild-upgrade upgrade not required");
	info("this forest is already up to date; no upgrade is required");
	return true;
    }

    info("analyzing...");
    DependencyGraph root_graph;
    constructTreeGraph(ud, root_graph);
    std::vector<DependencyGraph::ItemList> const& forests =
	root_graph.getIndependentSets();

    validateProposedForests(ud, forests);

    if (ud.missing_treenames)
    {
	error("some build trees have not been assigned names");
    }

    if (this->error_handler.anyErrors())
    {
	ud.writeUpgradeData(forests);
	info("upgrade data has been written to " +
	     UpgradeData::FILE_UPGRADE_DATA);
	info("please edit that file and rerun; for details, see"
	     " \"Upgrading Build Trees from 1.0 to 1.1\""
	     " in the user's manual");
    }
    exitIfErrors();

    // XXX do upgrade

    // XXX When writing, remember to preserve any existing tree deps

    // XXX Get forest root for each forest.  If there is no
    // Abuild.conf in the forest root directory and if there is any
    // tree in the forest for which the next higher Abuild.conf is
    // above the forest root directory, then create a forest root
    // Abuild.conf.  Its children will be updated normally with
    // pointers to tree roots below it, except that it will have to be
    // created instead of rewritten.

    return true;
}

void
Abuild::findBuildItems(UpgradeData& ud)
{
    CompatLevel cl(CompatLevel::cl_1_0);
    std::list<std::string> dirs;
    dirs.push_back(".");
    while (! dirs.empty())
    {
	std::string dir = dirs.front();
	dirs.pop_front();

	if (ud.ignored_directories.count(Util::canonicalizePath(dir)))
	{
	    continue;
	}

	if (Util::isFile(dir + "/" + ItemConfig::FILE_CONF))
	{
	    ItemConfig* config = readConfig(dir, "");
	    if (config->usesDeprecatedFeatures())
	    {
		ud.upgrade_required = true;
	    }
	    bool is_root = config->isTreeRoot();
	    ud.item_dirs[dir] = is_root;
	    if (is_root)
	    {
		std::string treename = config->getTreeName();
		if (! treename.empty())
		{
		    if (ud.tree_names.count(dir))
		    {
			QTC::TC("abuild", "Abuild-upgrade ERR name mismatch");
			error("the name assigned to the tree at \"" +
			      dir + "\" in the upgrade data file (\"" +
			      ud.tree_names[dir] + "\") differs from"
			      " the tree's actual name (\"" + treename +
			      "\"); ignoring information from the upgrade"
			      " data file");
		    }
		    ud.tree_names[dir] = treename;
		}
		else if (! ud.tree_names.count(dir))
		{
		    ud.missing_treenames = true;
		}
	    }
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
Abuild::constructTreeGraph(UpgradeData& ud, DependencyGraph& g)
{
    for (std::map<std::string, bool>::const_iterator iter =
	     ud.item_dirs.begin();
	 iter != ud.item_dirs.end(); ++iter)
    {
	std::string const& dir = (*iter).first;
	bool is_root = (*iter).second;
	if (! is_root)
	{
	    continue;
	}
	g.addItem(dir);
	if (! Util::isFile(dir + "/" + ItemConfig::FILE_CONF))
	{
	    // This is a tree root with Abuild.backing but no
	    // Abuild.conf.
	    QTC::TC("abuild", "Abuild-upgrade skipping root with no conf");
	    continue;
	}
	FileLocation location(dir + "/" + ItemConfig::FILE_CONF, 0, 0);
	ItemConfig* config = readConfig(dir, "");

	if (Util::isFile(dir + "/" + BackingConfig::FILE_BACKING))
	{
	    std::list<std::string> backing_areas =
		readBacking(dir)->getBackingAreas();
	    for (std::list<std::string>::iterator iter = backing_areas.begin();
		 iter != backing_areas.end(); ++iter)
	    {
		std::string rel_backing = Util::absToRel(*iter);
		if (ud.item_dirs.count(rel_backing))
		{
		    QTC::TC("abuild", "Abuild-upgrade ERR local backing");
		    error(FileLocation(dir + "/" +
				       BackingConfig::FILE_BACKING, 0, 0),
			  "backing area \"" + *iter + "\" falls within the"
			  " area being upgraded; rerun " +
			  this->whoami + " from a lower directory or exclude" +
			  " the backing area");
		}
	    }
	}

	std::list<std::string>& externals = ud.externals[dir];
	std::list<std::string>& tree_deps = ud.tree_deps[dir];
	std::list<ExternalData> const& old_externals = config->getExternals();
	for (std::list<ExternalData>::const_iterator eiter =
		 old_externals.begin();
	     eiter != old_externals.end(); ++eiter)
	{
	    std::string edecl = (*eiter).getDeclaredPath();
	    std::string epath = Util::absToRel(
		Util::canonicalizePath(dir + "/" + edecl));
	    ItemConfig* econfig = readExternalConfig(dir, edecl);
	    std::string dep_tree_name;

	    if (ud.item_dirs.count(epath) && ud.item_dirs[epath])
	    {
		// The external points to a known tree root inside our
		// area of concern.

		g.addDependency(dir, epath);
		dep_tree_name = econfig->getTreeName();
	    }
	    else if (econfig && econfig->isTreeRoot())
	    {
		// The external is valid but falls outside of our area
		// of interest.  It could be somewhere not below the
		// current directory, in a pruned area, or resolved
		// through a backing area.
		dep_tree_name = econfig->getTreeName();
	    }
	    else if (econfig)
	    {
		QTC::TC("abuild", "Abuild-upgrade ERR external not root");
		error(location,
		      "external " + edecl + " (" + epath +
		      " relative to current directory) is not"
		      " a build tree root");
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-upgrade ERR unknown external");
		error(location,
		      "external " + edecl + " does not exist and cannot"
		      " be resolved through a backing area");
	    }

	    if (dep_tree_name.empty())
	    {
		externals.push_back(edecl);
	    }
	    else
	    {
		tree_deps.push_back(dep_tree_name);
	    }
	}
    }

    if (! g.check())
    {
	DependencyGraph::ItemMap unknowns;
	std::vector<DependencyGraph::ItemList> cycles;
	g.getErrors(unknowns, cycles);
	// We only add the dependency to known roots, so unknowns
	// could happen only as a result of a programming error.
	assert(unknowns.empty());

	QTC::TC("abuild", "Abuild-upgrade ERR tree-dep cycle");
	for (std::vector<DependencyGraph::ItemList>::iterator i1 =
		 cycles.begin();
	     i1 != cycles.end(); ++i1)
	{
	    DependencyGraph::ItemList const& cycle = *i1;
	    error("the following trees are involved in"
		  " an external-dirs cycle:");
	    for (DependencyGraph::ItemList::const_iterator i2 = cycle.begin();
		 i2 != cycle.end(); ++i2)
	    {
		error("  " + *i2);
	    }
	}

	exitIfErrors();
    }
}

void
Abuild::validateProposedForests(
    UpgradeData& ud, std::vector<DependencyGraph::ItemList> const& forests)
{
    for (std::vector<DependencyGraph::ItemList>::const_iterator iter =
	     forests.begin();
	 iter != forests.end(); ++iter)
    {
	// Make sure each tree in the forest has a unique name.
	std::list<std::string> const& forest = *iter;
	std::map<std::string, std::string> names;
	for (std::list<std::string>::const_iterator iter = forest.begin();
	     iter != forest.end(); ++iter)
	{
	    std::string const& tree = *iter;
	    if (ud.tree_names.count(tree))
	    {
		std::string const& name = ud.tree_names[tree];
		if (names.count(name))
		{
		    QTC::TC("abuild", "Abuild-upgrade ERR duplicate name");
		    error("tree name \"" + name + "\" has been assigned to \"" +
			  tree + "\" and also to \"" + names[name] + "\"");
		}
		else
		{
		    names[name] = tree;
		}
		if (! ItemConfig::isNameValid(name))
		{
		    QTC::TC("abuild", "Abuild-upgrade ERR invalid name");
		    error("tree name \"" + name + "\", assigned to \"" +
			  tree + "\", is not a valid tree name");
		}
	    }
	}
    }
}

std::string
Abuild::getForestRoot(std::list<std::string> const& forest)
{
    std::string result = Util::canonicalizePath(forest.front());
    for (std::list<std::string>::const_iterator iter = forest.begin();
	 iter != forest.end(); ++iter)
    {
	std::string const& dir = *iter;
	while (! Util::isDirUnder(Util::canonicalizePath(dir), result))
	{
	    result = Util::dirname(result);
	}
    }
    assert(Util::isDirUnder(result));
    return Util::absToRel(result);
}
