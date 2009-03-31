// Methods in Abuild class concerned with --upgrade-trees

#include <Abuild.hh>

#include <assert.h>
#include <fstream>
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
    initializeForests(ud, forests);
    allowUnnamedForestRoots(ud);

    if (! ud.missing_treenames.empty())
    {
	error("some build trees have not been assigned names");
    }

    ud.writeUpgradeData();
    info("upgrade data has been written to " +
	 UpgradeData::FILE_UPGRADE_DATA);
    if (this->error_handler.anyErrors())
    {
	info("please edit that file and rerun; for details, see"
	     " \"Upgrading Build Trees from 1.0 to 1.1\""
	     " in the user's manual");
    }
    exitIfErrors();
    upgradeForests(ud);
    return true;
}

void
Abuild::findBuildItems(UpgradeData& ud)
{
    CompatLevel cl(CompatLevel::cl_1_0);
    std::map<std::string, std::string> parent_dirs;
    std::map<std::string, std::string> dir_trees;
    std::list<std::string> dirs;
    dirs.push_back(".");
    while (! dirs.empty())
    {
	std::string dir = dirs.front();
	dirs.pop_front();
	std::string parent = parent_dirs[dir];
	std::string tree_root = dir_trees[parent];
	dir_trees[dir] = tree_root; // may be overridden

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
	    if (is_root)
	    {
		dir_trees[dir] = dir;
		std::string treename = config->getTreeName();
		// XXX should try to get name from backing area...see
		// also econfig code with externals to see if we can
		// combine....refactor getAssignedTreeName, which
		// probably has the right logic already.
		if (! treename.empty())
		{
		    if (ud.tree_names.count(dir) &&
			(treename != ud.tree_names[dir]))
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
		    ud.missing_treenames.insert(dir);
		}
	    }

	    ud.items[dir] = config;
	    ud.item_tree_roots[dir] = dir_trees[dir];
	    if (! config->getName().empty())
	    {
		ud.tree_items[dir_trees[dir]].insert(dir);
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
		parent_dirs[fullpath] = dir;
	    }
	}
    }
}

void
Abuild::constructTreeGraph(UpgradeData& ud, DependencyGraph& g)
{
    for (std::map<std::string, ItemConfig*>::const_iterator iter =
	     ud.items.begin();
	 iter != ud.items.end(); ++iter)
    {
	std::string const& dir = (*iter).first;
	ItemConfig* config = (*iter).second;
	bool is_root = config->isTreeRoot();
	if (is_root || config->isForestRoot())
	{
	    g.addItem(dir);
	}
	if (! is_root)
	{
	    continue;
	}
	if (! config->isForestRoot())
	{
	    std::string forest_top =
		findTop(Util::canonicalizePath(dir),
			"tree found during upgrade");
	    if (! forest_top.empty())
	    {
		forest_top = Util::absToRel(forest_top);
	    }
	    if ((forest_top != dir) &&
		(ud.items.count(forest_top)))
	    {
		g.addDependency(forest_top, dir);
	    }
	}

	FileLocation location(dir + "/" + ItemConfig::FILE_CONF, 0, 0);
	bool has_backing = false;
	if (Util::isFile(dir + "/" + BackingConfig::FILE_BACKING))
	{
	    has_backing = true;
	    std::list<std::string> const& backing_areas =
		readBacking(dir)->getBackingAreas();
	    for (std::list<std::string>::const_iterator iter =
		     backing_areas.begin();
		 iter != backing_areas.end(); ++iter)
	    {
		std::string rel_backing = Util::absToRel(*iter);
		if (ud.items.count(rel_backing))
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

	    appendBackingData(dir,
			      ud.backing_areas[dir],
			      ud.deleted_trees[dir],
			      ud.deleted_items[dir]);
	}

	std::list<std::string>& externals = ud.externals[dir];
	std::list<std::string>& tree_deps = ud.tree_deps[dir];
	std::list<std::string> const& old_externals = config->getExternals();
	for (std::list<std::string>::const_iterator eiter =
		 old_externals.begin();
	     eiter != old_externals.end(); ++eiter)
	{
	    std::string const& edecl = *eiter;
	    ItemConfig* econfig = readExternalConfig(dir, edecl);
	    std::string dep_tree_name;
	    if (econfig)
	    {
		std::string epath = Util::absToRel(econfig->getAbsolutePath());
		dep_tree_name = econfig->getTreeName();
		if (ud.items.count(epath) && ud.items[epath])
		{
		    // The external points to a known tree root inside
		    // our area of concern.
		    g.addDependency(dir, epath);
		    if (dep_tree_name.empty() && ud.tree_names.count(epath))
		    {
			dep_tree_name = ud.tree_names[epath];
		    }
		}
		else if (econfig->isTreeRoot())
		{
		    // The external is valid but falls outside of our
		    // area of interest.  It could be somewhere not
		    // below the current directory, in a pruned area,
		    // or resolved through a backing area.
		    dep_tree_name = econfig->getTreeName();
		    if (dep_tree_name.empty() &&
			(! Util::isFile(dir + "/" + edecl + "/" +
					ItemConfig::FILE_CONF)))
		    {
			QTC::TC("abuild", "Abuild-upgrade ERR backed external");
			error(location, "this build item resolves external \"" +
			      edecl + "\" using a backing area, and the"
			      " backed external has not been upgraded; you"
			      " must upgrade the backing area first in this"
			      " case");
		    }
		}
		else
		{
		    QTC::TC("abuild", "Abuild-upgrade ERR external not root");
		    error(location,
			  "external " + edecl + " (" + epath +
			  " relative to current directory) is not"
			  " a build tree root");
		}
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

void
Abuild::initializeForests(
    UpgradeData& ud,
    std::vector<DependencyGraph::ItemList> const& forests)
{
    for (std::vector<DependencyGraph::ItemList>::const_iterator iter =
	     forests.begin();
	 iter != forests.end(); ++iter)
    {
	DependencyGraph::ItemList const& forest = *iter;
	std::string root = getForestRoot(forest);
	bool valid_root = true;
	if (Util::isFile(root + "/" + ItemConfig::FILE_CONF))
	{
	    ItemConfig* config = readConfig(root, "");
	    if (! config->isForestRoot())
	    {
		valid_root = false;
	    }
	}
	if (valid_root)
	{
	    ud.forest_contents[root] = forest;
	    for (std::list<std::string>::const_iterator iter = forest.begin();
		 iter != forest.end(); ++iter)
	    {
		ud.tree_forest_roots[*iter] = root;
	    }
	}
	else
	{
	    QTC::TC("abuild", "Abuild-upgrade ERR invalid forest root");
	    error("wanted to use " + root + " as the root of a forest, but"
		  " it already contains an " + ItemConfig::FILE_CONF +
		  " and is not already a forest root");
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

void
Abuild::allowUnnamedForestRoots(UpgradeData& ud)
{
    // High-level child-only items that serve no purpose other than to
    // connect other trees together show up unnamed trees because 1.0
    // compatibility mode recognizes them as roots.  We have to allow
    // them to be unnamed.  This is the same case that is handled by
    // removeEmptyTrees() in Abuild.cc.

    for (std::map<std::string, std::list<std::string> >::const_iterator iter =
	     ud.forest_contents.begin();
	 iter != ud.forest_contents.end(); ++iter)
    {
	std::string const& forest_root = (*iter).first;
	if ((ud.tree_items[forest_root].empty()) &&
	    (ud.tree_names.count(forest_root) == 0) &&
	    (ud.items.count(forest_root)) &&
	    (ud.items[forest_root]->isChildOnly()))
	{
	    QTC::TC("abuild", "Abuild-upgrade ignore useless empty tree");
	    ud.missing_treenames.erase(forest_root);
	    ud.unnamed_trees.insert(forest_root);
	}
    }
}

void
Abuild::upgradeForests(UpgradeData& ud)
{
    // Initialize backing data for each forest.
    std::map<std::string, std::list<std::string> > backing_areas;
    std::map<std::string, std::set<std::string> > deleted_trees;
    std::map<std::string, std::set<std::string> > deleted_items;
    std::set<std::string> obsolete_backing_dirs;
    for (std::map<std::string, std::list<std::string> >::const_iterator iter =
	     ud.forest_contents.begin();
	 iter != ud.forest_contents.end(); ++iter)
    {
	std::string const& forest_root = (*iter).first;
	appendBackingData(forest_root,
			  backing_areas[forest_root],
			  deleted_trees[forest_root],
			  deleted_items[forest_root]);
	std::list<std::string> const& tree_roots = (*iter).second;
	for (std::list<std::string>::const_iterator iter = tree_roots.begin();
	     iter != tree_roots.end(); ++iter)
	{
	    std::string const& dir = *iter;
	    if (dir == forest_root)
	    {
		continue;
	    }
	    if (Util::isFile(dir + "/" + BackingConfig::FILE_BACKING))
	    {
		appendBackingData(dir,
				  backing_areas[forest_root],
				  deleted_trees[forest_root],
				  deleted_items[forest_root]);
		obsolete_backing_dirs.insert(dir);
	    }
	}
    }

    // Filter out duplicate backing areas
    for (std::map<std::string, std::list<std::string> >::iterator iter =
	     backing_areas.begin();
	 iter != backing_areas.end(); ++iter)
    {
	std::list<std::string>& areas = (*iter).second;
	std::set<std::string> seen;
	std::list<std::string>::iterator i2 = areas.begin();
	while (i2 != areas.end())
	{
	    std::list<std::string>::iterator next = i2;
	    ++next;
	    if (seen.count(*i2))
	    {
		areas.erase(i2, next);
	    }
	    else
	    {
		seen.insert(*i2);
	    }
	    i2 = next;
	}
    }

    // Make a list of directories for which we will have to add
    // elements to child-dirs.  If any element of this map is a
    // directory that has no Abuild.conf file, we just create the
    // Abuild.conf file.  This is a common case for new forest roots.
    // For tree roots that are already the roots of their forests, no
    // child-dirs have to be edited.
    std::map<std::string, std::set<std::string> > children_to_add;
    std::map<std::string, std::set<std::string> > children_to_create;

    for (std::map<std::string, std::string>::iterator iter =
	     ud.tree_forest_roots.begin();
	 iter != ud.tree_forest_roots.end(); ++iter)
    {
	std::string const& tree_root = (*iter).first;
	std::string const& forest_root = (*iter).second;
	std::string path = tree_root;
	while (path != forest_root)
	{
	    path = Util::dirname(path);
	    if (ud.items.count(path))
	    {
		// If this item is part of a different forest, we have
		// interleaved forests, which is precluded by checks
		// in ItemConfig.
		assert(ud.tree_forest_roots[
			   ud.item_tree_roots[path]] == forest_root);
		QTC::TC("abuild", "Abuild-upgrade add child");
		children_to_add[path].insert(
		    Util::absToRel(Util::canonicalizePath(tree_root),
				   Util::canonicalizePath(path)));
		break;
	    }
	    else if (path == forest_root)
	    {
		QTC::TC("abuild", "Abuild-upgrade create new root Abuild.conf");
		children_to_create[path].insert(
		    Util::absToRel(Util::canonicalizePath(tree_root),
				   Util::canonicalizePath(path)));
		break;
	    }
	    assert(path != Util::dirname(path));
	}
    }

    std::string const suffix = "-1_1";
    std::set<std::string> new_files;

    // Create any new forest root items.
    for (std::map<std::string, std::set<std::string> >::iterator iter =
	     children_to_create.begin();
	 iter != children_to_create.end(); ++iter)
    {
	std::string const& dir = (*iter).first;
	std::set<std::string> const& children = (*iter).second;

	std::string newfile = dir + "/" + ItemConfig::FILE_CONF;
	new_files.insert(newfile);
	newfile += suffix;
	std::ofstream of(newfile.c_str(),
			 std::ios_base::out |
			 std::ios_base::trunc);
	if (! of.is_open())
	{
	    throw QEXC::System(std::string("create ") + newfile, errno);
	}

	of << "child-dirs: \\" << std::endl;
	for (std::set<std::string>::const_iterator iter = children.begin();
	     iter != children.end(); ++iter)
	{
	    of << "    " << *iter;
	    std::set<std::string>::const_iterator next = iter;
	    ++next;
	    if (next != children.end())
	    {
		of << " \\";
	    }
	    of << std::endl;
	}

	of.close();
    }

    // Rewrite all existing Abuild.conf files as needed.
    for (std::map<std::string, ItemConfig*>::iterator iter = ud.items.begin();
	 iter != ud.items.end(); ++iter)
    {
	std::string const& dir = (*iter).first;
	ItemConfig* config = (*iter).second;

	std::set<std::string> new_children;
	if (children_to_add.count(dir))
	{
	    new_children = children_to_add[dir];
	}

	std::string tree_name;
	std::list<std::string> externals;
	std::list<std::string> tree_deps;

	if (config->isTreeRoot())
	{
	    tree_name = ud.tree_names[dir];
	    externals = ud.externals[dir];
	    tree_deps = ud.tree_deps[dir];
	}

	if (! externals.empty())
	{
	    QTC::TC("abuild", "Abuild-upgrade partial upgrade");
	    info("WARNING: " + dir + " contains some externals that"
		 " could not be converted to tree-deps; it will be"
		 " necessary to resolve these and rerun the upgrade process");
	}

	std::string newfile = dir + "/" + ItemConfig::FILE_CONF + suffix;
	if (config->upgradeConfig(
		newfile, new_children, tree_name, externals, tree_deps))
	{
	    new_files.insert(dir + "/" + ItemConfig::FILE_CONF);
	}
    }

    // Write out new backing files
    for (std::map<std::string, std::list<std::string> >::iterator iter =
	     backing_areas.begin();
	 iter != backing_areas.end(); ++iter)
    {
	std::list<std::string>& areas = (*iter).second;
	if (areas.empty())
	{
	    continue;
	}

	std::string const& dir = (*iter).first;
	std::set<std::string>& dt = deleted_trees[dir];
	std::set<std::string>& di = deleted_items[dir];

	std::string newfile = dir + "/" + BackingConfig::FILE_BACKING;
	new_files.insert(newfile);
	newfile += suffix;
	std::ofstream of(newfile.c_str(),
			 std::ios_base::out |
			 std::ios_base::trunc);
	if (! of.is_open())
	{
	    throw QEXC::System(std::string("create ") + newfile, errno);
	}

	of << "backing-areas: \\" << std::endl;
	for (std::list<std::string>::const_iterator iter = areas.begin();
	     iter != areas.end(); ++iter)
	{
	    of << "    " << *iter;
	    std::list<std::string>::const_iterator next = iter;
	    ++next;
	    if (next != areas.end())
	    {
		of << " \\";
	    }
	    of << std::endl;
	}
	if (! dt.empty())
	{
	    QTC::TC("abuild", "Abuild-upgrade write deleted-trees");
	    of << "deleted-trees: " << Util::join(" ", dt) << std::endl;
	}
	if (! di.empty())
	{
	    QTC::TC("abuild", "Abuild-upgrade write deleted-items");
	    of << "deleted-items: " << Util::join(" ", di) << std::endl;
	}

	of.close();
    }

    // XXX for each file in new_files, rename to old, rename new to
    // it.  rename obsolete backing files

    exitIfErrors();
}
