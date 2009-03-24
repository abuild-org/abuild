// Methods in Abuild class concerned with --upgrade-trees

#include <Abuild.hh>

#include <assert.h>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <ItemConfig.hh>
#include <UpgradeData.hh>

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

    // XXX remember to validate tree names and to make sure they are
    // unique within a forest

    // XXX check all upgrade conditions

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

	if (Util::isFile(dir + "/" + BackingFile::FILE_BACKING))
	{
	    std::string backing_area;
	    readBacking(dir, backing_area);
	    std::string rel_backing = Util::absToRel(backing_area);
	    if (ud.item_dirs.count(rel_backing))
	    {
		QTC::TC("abuild", "Abuild-upgrade ERR local backing");
		error(FileLocation(dir + "/" + BackingFile::FILE_BACKING, 0, 0),
		      "This item's backing area falls within the area"
		      " being upgraded.  You must either rerun " +
		      this->whoami + " from a lower directory or exclude" +
		      " the backing area.");
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
		dep_tree_name = readConfig(dir, "")->getTreeName();
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
