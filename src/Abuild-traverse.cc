// Traversal and static validation of the abuild forest

#include <Abuild.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <FileLocation.hh>
#include <ItemConfig.hh>
#include <BackingConfig.hh>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

bool
Abuild::readConfigs()
{
    // This routine returns true to indicate that we should exit
    // without actually doing any builds.

    if (this->this_platform.empty())
    {
	this->this_config_dir = this->current_directory;
    }
    else
    {
	this->this_config_dir = Util::dirname(this->current_directory);
    }

    if (! Util::isFile(this->this_config_dir + "/" + ItemConfig::FILE_CONF))
    {
	QTC::TC("abuild", "Abuild-traverse ERR no abuild.conf");
	fatal(ItemConfig::FILE_CONF + " not found");
    }
    this->this_config = readConfig(this->this_config_dir, "");

    std::string local_top = findTop(
	this->this_config_dir, "local forest");
    if (local_top.empty())
    {
	QTC::TC("abuild", "Abuild-traverse ERR can't find local top");
	fatal("unable to find root of local forest");
    }

    // We explicitly make forests local to readConfigs.  By not
    // holding onto it after readConfigs exits, we ensure that we
    // don't have any way to get any information about items not in
    // the build set.  This policy has helped to prevent a number of
    // potential logic errors in which we might try, for one reason or
    // another, to access something we shouldn't be accessing.  Before
    // deciding to change this policy, keep in mind that the build set
    // is closed with respect to dependency relationships, and
    // dependency relationships are subject to the integrity
    // guarantee.  See comments in BuildItem::getReferences for
    // additional discussion.
    BuildForest_map forests;
    traverse(forests, local_top);

    // Report any platform selectors that were not used in any tree.
    // This check can't be done until after all forests are traversed
    // since each tree can potentially have different platforms and
    // platform types.
    if (! this->unused_platform_selectors.empty())
    {
	QTC::TC("abuild", "Abuild-traverse unused platform selector");
	error("the following platform selectors were never used for"
	      " platform selection and may refer to unknown"
	      " platform types, compilers, or options:");
	for (std::map<std::string, std::string>::const_iterator iter =
		 this->unused_platform_selectors.begin();
	     iter != this->unused_platform_selectors.end(); ++iter)
	{
	    error("  " + (*iter).second);
	}
    }

    // Compute the list of all known traits.  This routine also
    // validates to make sure that any traits specified on the command
    // line are valid.
    computeValidTraits(forests);

    if (this->monitored || this->dump_data)
    {
	monitorOutput("begin-dump-data");
	dumpData(forests);
	monitorOutput("end-dump-data");
    }

    exitIfErrors();

    if (this->dump_data)
    {
	return true;
    }

    if (this->list_traits)
    {
	listTraits();
	return true;
    }

    if (this->list_platforms)
    {
	listPlatforms(forests);
	return true;
    }

    BuildForest& local_forest = *(forests[local_top]);
    BuildTree_map& buildtrees = local_forest.getBuildTrees();
    BuildItem_map& builditems = local_forest.getBuildItems();

    if (! this->to_find.empty())
    {
	if (this->to_find.find("tree:") == 0)
	{
	    std::string tree = this->to_find.substr(5);
	    if (buildtrees.count(tree))
	    {
		std::cout << "tree " << tree << ": "
			  << buildtrees[tree]->getRootPath() << std::endl;
	    }
	    else
	    {
		std::cout << "tree " << tree << " is unknown" << std::endl;
	    }
	}
	else
	{
	    std::string const& item = this->to_find;
	    if (builditems.count(item))
	    {
		std::cout << "item " << item
			  << " (in tree " << builditems[item]->getTreeName()
			  << "): " << builditems[item]->getAbsolutePath()
			  << std::endl;
	    }
	    else
	    {
		std::cout << "item " << item << " is unknown" << std::endl;
	    }
	}
	return true;
    }

    if (! this->rules_help_topic.empty())
    {
	rulesHelp(local_forest);
	return true;
    }

    computeBuildset(buildtrees, builditems);

    if (! this->full_integrity)
    {
	// Note: we do these checks even when cleaning.  Otherwise,
	// shadowed items won't get cleaned in their backing areas on
	// --clean=all, which is not the expected behavior.
	reportIntegrityErrors(forests, this->buildset, local_top);
	if (Error::anyErrors())
	{
	    QTC::TC("abuild", "Abuild-traverse integrity errors in buildset");
	}
	exitIfErrors();
    }
    // else all integrity checks have already been made

    exitIfErrors();

    if ((! (this->buildset_name.empty() && this->cleanset_name.empty())) &&
	this->buildset.empty())
    {
	QTC::TC("abuild", "Abuild-traverse empty build set");
	notice("build set contains no items");
	return true;
    }

    // Construct a list of build item names in reverse dependency
    // order (leaves of the dependency tree last).  This list is used
    // during construction of the build graph.
    std::list<std::string> const& sorted_items =
	local_forest.getSortedItemNames();
    for (std::list<std::string>::const_reverse_iterator iter =
	     sorted_items.rbegin();
	 iter != sorted_items.rend(); ++iter)
    {
	std::string const& item_name = *iter;
	if (this->buildset.count(item_name))
	{
	    this->buildset_reverse_order.push_back(item_name);
	}
    }

    computeTreePrefixes(local_forest.getSortedTreeNames());

    // A false return means that we should continue processing.
    return false;
}

ItemConfig*
Abuild::readConfig(std::string const& dir, std::string const& parent_dir)
{
    ItemConfig* result = 0;
    try
    {
	result = ItemConfig::readConfig(
	    this->error_handler, this->compat_level, dir, parent_dir);
    }
    catch (QEXC::General& e)
    {
	fatal(e.what());
    }
    return result;
}

ItemConfig*
Abuild::readExternalConfig(std::string const& start_dir,
			   std::string const& start_external)
{
    if (read_external_config_cache[start_dir].count(start_external))
    {
	return read_external_config_cache[start_dir][start_external];
    }

    verbose("looking for external \"" + start_external + "\" of \"" +
	    start_dir + "\"");
    incrementVerboseIndent();

    ItemConfig* config = 0;

    std::set<std::string> seen;
    std::list<std::pair<std::string, std::string> > candidates;
    candidates.push_back(std::make_pair(start_dir, start_external));

    while ((! candidates.empty()) && (config == 0))
    {
	std::string dir = candidates.front().first;
	std::string ext = candidates.front().second;
	candidates.pop_front();
	std::string fulldir = dir;
	if (! ext.empty())
	{
	    fulldir += "/" + ext;
	}
	if (seen.count(fulldir))
	{
	    // No coverage case -- I believe this is precluded by
	    // BackingConfig.cc disallowing a the case of backing area
	    // that doesn't have an Abuild.conf.  Since this code only
	    // follows the Abuild.backing file when the Abuild.conf
	    // file is not found, we would need a loop formed by
	    // Abuild.backing alone files to get here, and such a loop
	    // will be short-circuited by the check in
	    // BackingConfig.cc.
	    error("loop detected trying to find " + ItemConfig::FILE_CONF +
		  " for \"" + start_external + "\" relative to \"" +
		  start_dir + "\"");
	    break;
	}
	else
	{
	    seen.insert(fulldir);
	}

	verbose("inside tree \"" + dir + "\"");
	verbose("checking directory \"" + fulldir + "\"");
	if ((! ext.empty()) && Util::isDirectory(fulldir))
	{
	    // The external directory exists, so work relative to that
	    // directory rather than our original one.
	    verbose("directory exists; switching context from"
		    " original tree to this tree");
	    dir = fulldir;
	    ext.clear();
	}

	if (Util::isFile(fulldir + "/" + ItemConfig::FILE_CONF))
	{
	    // Simple case: there's an actual Abuild.conf here.
	    config = readConfig(fulldir, "");
	    verbose("found at \"" + config->getAbsolutePath() + "\"");
	}
	else if (Util::isFile(dir + "/" + BackingConfig::FILE_BACKING))
	{
	    // There's an Abuild.backing but no Abuild.conf.  Traverse
	    // the backing chain.  If the external exists, we're
	    // traversing its backing chain.  Otherwise, we're
	    // traversing the original directory's backing chain.
	    QTC::TC("abuild", "Abuild-traverse backing without conf");
	    verbose("checking backing areas of \"" + dir + "\"");
	    std::list<std::string> const& backing_areas =
		readBacking(dir)->getBackingAreas();
	    if (! backing_areas.empty())
	    {
		QTC::TC("abuild", "Abuild-traverse external in backing area",
			(ext.empty() ? 0 : 1));
		for (std::list<std::string>::const_iterator iter =
			 backing_areas.begin();
		     iter != backing_areas.end(); ++iter)
		{
		    candidates.push_back(std::make_pair(*iter, ext));
		}
	    }
	}
    }

    decrementVerboseIndent();
    verbose("done looking for external \"" +
	    start_external + "\" of \"" + start_dir + "\"");

    read_external_config_cache[start_dir][start_external] = config;
    return config;		// might be 0
}

std::string
Abuild::findTop(std::string const& start_dir,
		std::string const& description)
{
    // Find the directory that contains the root of the forest
    // containing the item with the given start directory.

    if (find_top_cache.count(start_dir))
    {
	return find_top_cache[start_dir];
    }

    std::string top;
    std::string dir = start_dir;

    verbose("looking for top of build forest \"" + description + "\"");

    incrementVerboseIndent();
    while (true)
    {
	verbose("top-search: checking " + dir);
	ItemConfig* config = readConfig(dir, "");
	if (config->isForestRoot())
	{
	    top = dir;
	    break;
	}
	std::string parent_dir = config->getParentDir();
	if (parent_dir.empty())
	{
	    verbose("top-search: " + dir + " has no parent; ending search");
	    break;
	}
	else
	{
	    dir = parent_dir;
	}
    }
    decrementVerboseIndent();

    if (top.empty())
    {
	QTC::TC("abuild", "Abuild-traverse ERR can't find top");
	error("unable to find top of build forest containing \"" +
	      start_dir + "\" (" + description + ");"
	      " run with --verbose for details");
    }
    else
    {
	verbose("top-search: " + top + " is forest root");
    }

    find_top_cache[start_dir] = top;
    return top;
}

void
Abuild::traverse(BuildForest_map& forests, std::string const& top_path)
{
    std::set<std::string> visiting;
    DependencyGraph external_graph; // 1.0-compatibility only

    traverseForests(forests, external_graph,
		    top_path, visiting, "root of local forest");
    if (this->compat_level.allow_1_0())
    {
	// If we have A -ext-> B <-ext- C, neither a traversal
	// starting from A nor one starting from C will catch all
	// three build trees.  If, as a result of backing areas, we
	// end up traversing from both A and C, will will end up with
	// too many forests, and we will need to merge some of them.
	// We use items_traversed to prevent the same tree from ever
	// being included in more than one forest.  This is all a
	// result of the forests we are creating in 1.1 actually
	// containing multiple 1.0 trees, each of which looks like the
	// root of a forest.

	if (! external_graph.check())
	{
	    QTC::TC("abuild", "Abuild-traverse ERR external_graph failure");
	    error("1.0 compatibility mode was unable to determine"
		  " proper relationships among externals; some errors"
		  " about unknown tree dependencies may be spurious and"
		  " may disappear after you fix tree dependency cycles");
	    reportDirectoryGraphErrors(external_graph, "external-dir");
	}
	else
	{
	    mergeForests(forests, external_graph);
	}

	removeEmptyTrees(forests);
    }

    DependencyGraph backing_graph;
    computeBackingGraph(forests, backing_graph);
    std::list<std::string> const& all_forests = backing_graph.getSortedGraph();
    for (std::list<std::string>::const_iterator iter = all_forests.begin();
	 iter != all_forests.end(); ++iter)
    {
	validateForest(forests, *iter);
    }
}

void
Abuild::reportDirectoryGraphErrors(DependencyGraph& g,
				   std::string const& description)
{
    DependencyGraph::ItemMap unknowns;
    std::vector<DependencyGraph::ItemList> cycles;
    g.getErrors(unknowns, cycles);

    for (DependencyGraph::ItemMap::iterator i1 = unknowns.begin();
	 i1 != unknowns.end(); ++i1)
    {
	std::string const& dir = (*i1).first;
	std::list<std::string> const& deps = (*i1).second;
	for (std::list<std::string>::const_iterator i2 = deps.begin();
	     i2 != deps.end(); ++i2)
	{
	    error("directory \"" + dir + "\" references unknown " +
		  description + " \"" + *i2 + "\"");
	}
    }

    for (std::vector<DependencyGraph::ItemList>::iterator i1 =
	     cycles.begin();
	 i1 != cycles.end(); ++i1)
    {
	DependencyGraph::ItemList const& cycle = *i1;
	error("the following trees are involved in"
	      " a cycle (" + description + "):");
	for (DependencyGraph::ItemList::const_iterator i2 = cycle.begin();
	     i2 != cycle.end(); ++i2)
	{
	    error("  " + *i2);
	}
    }
}

void
Abuild::computeBackingGraph(BuildForest_map& forests,
			    DependencyGraph& g)
{
    for (BuildForest_map::iterator forest_iter = forests.begin();
	 forest_iter != forests.end(); ++forest_iter)
    {
	std::string const& path = (*forest_iter).first;
	g.addItem(path);
	BuildForest& forest = *((*forest_iter).second);
	std::list<std::string> const& backing_areas = forest.getBackingAreas();
	for (std::list<std::string>::const_iterator biter =
		 backing_areas.begin();
	     biter != backing_areas.end(); ++biter)
	{
	    g.addDependency(path, *biter);
	}
    }
    // Even if there were errors, the build tree graph must always be
    // consistent.  Abuild doesn't store invalid backing areas in the
    // build tree structures.
    if (! g.check())
    {
	reportDirectoryGraphErrors(g, "backing area");
	fatal("unable to continue after backing area error");
    }
}

void
Abuild::traverseForests(BuildForest_map& forests,
			DependencyGraph& external_graph,
			std::string const& top_path,
			std::set<std::string>& visiting,
			std::string const& description)
{
    if (visiting.count(top_path))
    {
        QTC::TC("abuild", "Abuild-traverse ERR backing cycle");
        fatal("backing area cycle detected for " + top_path +
	      " (" + description + ")");
    }

    if (forests.count(top_path))
    {
	return;
    }

    verbose("traversing forest from " + top_path + ": \"" + description + "\"");
    ItemConfig* config = readConfig(top_path, "");
    assert(config->isForestRoot());

    // DO NOT RETURN BELOW THIS POINT until after we remove top_path
    // from visiting.
    incrementVerboseIndent();
    visiting.insert(top_path);

    BuildForest_ptr forest(new BuildForest(top_path));
    std::list<std::string> dirs_with_externals;
    std::list<std::string>& backing_areas = forest->getBackingAreas();
    std::set<std::string>& deleted_trees = forest->getDeletedTrees();
    std::set<std::string>& deleted_items = forest->getDeletedItems();
    traverseItems(*forest, external_graph, top_path, dirs_with_externals,
		  backing_areas, deleted_trees, deleted_items);

    if (this->compat_level.allow_1_0())
    {
	verbose("1.0-compatibility: checking for externals in " + top_path);
	incrementVerboseIndent();
	while (! dirs_with_externals.empty())
	{
	    std::string dir = dirs_with_externals.front();
	    verbose("looking at externals of " + dir);
	    dirs_with_externals.pop_front();
	    ItemConfig* dconfig = readConfig(dir, "");
	    std::list<std::string> const& externals = dconfig->getExternals();
	    for (std::list<std::string>::const_iterator eiter =
		     externals.begin();
		 eiter != externals.end(); ++eiter)
	    {
		std::string const& edecl = *eiter;
		verbose("checking external " + edecl);

		std::string const& epath =
		    Util::canonicalizePath(dir + "/" + edecl);
		std::string file_conf =
		    epath + "/" + ItemConfig::FILE_CONF;
		std::string file_backing =
		    epath + "/" + BackingConfig::FILE_BACKING;

		if (! (Util::isFile(file_conf) ||
		       Util::isFile(file_backing)))
		{
		    // No additional traversal required.  We've
		    // already tried to resolve this in a backing area
		    // and reported if we were unable to do so.
		    verbose("skipping non-local external " + edecl);
		    continue;
		}

		// If there's an Abuild.backing file, we only care
		// about it if this is a forest root or if there's no
		// Abuild.conf.  In other instances, ItemConfig has
		// already complained.
		if (Util::isFile(file_backing) &&
		    (! Util::isFile(file_conf)))
		{
		    QTC::TC("abuild", "Abuild-traverse traverse backing without conf");
		    verbose("getting backing data from and then"
			    " skipping backed external " + edecl);
		    appendBackingData(
			epath, backing_areas, deleted_trees, deleted_items);
		    continue;
		}

		assert(Util::isFile(file_conf));

		std::string ext_top =
		    findTop(epath, "external \"" + edecl + "\" of \"" +
			    dir + "\"");
		if (ext_top.empty())
		{
		    QTC::TC("abuild", "Abuild-traverse ERR findTop from external");
		    // An error was already issued by findTop
		    continue;
		}

		// Traverse items in the external forest as if it were
		// part of the forest for which this traverseForests
		// method was called.  This effectively merges the
		// external forest with the referencing forest.  We
		// can't always catch all cases here.  See comments in
		// mergeForests for additional notes.
		verbose("traversing items for external " + ext_top +
			", which is " + edecl + " from " + dir);
		if (epath == ext_top)
		{
		    external_graph.addDependency(top_path, ext_top);
		}
		traverseItems(*forest, external_graph, ext_top,
			      dirs_with_externals, backing_areas,
			      deleted_trees, deleted_items);
		verbose("done traversing items for external " + ext_top);
	    }
	}
	decrementVerboseIndent();
	verbose("done with externals");
    }
    else
    {
	assert(dirs_with_externals.empty());
    }

    verbose("checking for backing areas");
    incrementVerboseIndent();
    // Normalize backing areas to filter out duplicates and resolve to
    // forest roots.  Traverse backing areas.
    { // private scope
	std::set<std::string> seen;
	std::list<std::string>::iterator iter = backing_areas.begin();
	while (iter != backing_areas.end())
	{
	    std::list<std::string>::iterator next = iter;
	    ++next;
	    verbose("checking for backing area " + *iter);
	    std::string btop = findTop(
		*iter, "backing area of \"" + top_path + "\"");
	    // We've already verified that everything appending to
	    // backing_areas has a valid top.
	    assert(! btop.empty());
	    if (*iter != btop)
	    {
		if (! this->suggest_upgrade)
		{
		    QTC::TC("abuild", "Abuild-traverse backing area points below root");
		}
		verbose("this is actually " + btop);
	    }
	    bool keep = false;
	    if (btop == top_path)
	    {
		// This is the most likely location for backing files,
		// though they could have come from 1.0-style
		// Abulid.backing files as well...
		FileLocation l(
		    top_path + "/" + BackingConfig::FILE_BACKING, 0, 0);
		if (*iter == btop)
		{
		    QTC::TC("abuild", "Abuild-traverse ERR backs explicitly to self");
		    error(l, "this forest lists itself as a backing area");
		}
		else
		{
		    QTC::TC("abuild", "Abuild-traverse ERR backs implicitly to self");
		    error(l, "backing area " + *iter +
			  " belongs to this forest");
		}
	    }
	    else if (seen.count(btop))
	    {
		verbose("this is a duplicate backing area; ignoring");
		QTC::TC("abuild", "Abuild-traverse duplicate backing area");
	    }
	    else
	    {
		seen.insert(btop);
		keep = true;
		verbose("traversing backing area");
		traverseForests(forests, external_graph, btop, visiting,
				"backing area of \"" + top_path + "\"");
		verbose("done traversing backing area");
	    }

	    if (keep)
	    {
		*iter = btop;
	    }
	    else
	    {
		backing_areas.erase(iter, next);
	    }
	    iter = next;
	}
    }
    decrementVerboseIndent();
    verbose("done with all backing areas of " + top_path);

    visiting.erase(top_path);
    forests[top_path] = forest;
    decrementVerboseIndent();
    verbose("done traversing forest from " + top_path);
}

void
Abuild::mergeForests(BuildForest_map& forests,
		     DependencyGraph& external_graph)
{
    assert(this->compat_level.allow_1_0());

    // If A backs to independent forests B and C, which in turn both
    // reference common but independent external D, we can't tell that
    // B, C, and D are all supposed to be part of the same forest
    // until something, such as traversal of A, causes us to traverse
    // both B and C.  In 1.0 compatibility mode, we don't know about
    // externals until we are already finished calling traverseItems,
    // so we are stuck merging forests after the fact.  The
    // items_traversed set prevents us from initially adding any build
    // item or tree to more than one forest, so the merging is
    // relatively straightforward once we determine which forests have
    // to be merged.

    std::map<std::string, std::string> forest_renames;
    std::map<std::string, std::list<std::string> > forest_merges;

    // Figure out independent subsets of the graph formed by external
    // relationships.  We only care about forests that we have
    // previously considered to be forest roots.  (Remember that we
    // are already grouping separate forests together as one in the
    // case of externals.)  For any subset that contains more than one
    // apparent forest root, we need to merge the forests in that
    // subset.
    std::vector<DependencyGraph::ItemList> const& sets =
	external_graph.getIndependentSets();
    for (std::vector<DependencyGraph::ItemList>::const_iterator iter =
	     sets.begin();
	 iter != sets.end(); ++iter)
    {
	DependencyGraph::ItemList const& set = *iter;
	std::list<std::string> merge;
	for (DependencyGraph::ItemList::const_iterator iter =
		 set.begin();
	     iter != set.end(); ++iter)
	{
	    if (forests.count(*iter))
	    {
		merge.push_back(*iter);
	    }
	}
	if (merge.size() <= 1)
	{
	    continue;
	}
	QTC::TC("abuild", "Abuild-traverse forest merge required");
	std::string first = merge.back();
	merge.pop_back();
	forest_merges[first] = merge;
	for (DependencyGraph::ItemList::iterator iter = merge.begin();
	     iter != merge.end(); ++iter)
	{
	    std::string const& other = *iter;
	    forest_renames[other] = first;
	}
    }

    // To merge forests, we just merge their trees, items, backing
    // areas, and backing area data.
    for (std::map<std::string, std::list<std::string> >::iterator i1 =
	     forest_merges.begin();
	 i1 != forest_merges.end(); ++i1)
    {
	std::string const& keep_dir = (*i1).first;
	BuildForest& keep = *(forests[keep_dir]);

	std::list<std::string>& backing_areas = keep.getBackingAreas();
	std::set<std::string>& deleted_trees = keep.getDeletedTrees();
	std::set<std::string>& deleted_items = keep.getDeletedItems();
	std::set<std::string>& global_plugins = keep.getGlobalPlugins();

	std::list<std::string> const& to_remove = (*i1).second;
	for (std::list<std::string>::const_iterator i2 = to_remove.begin();
	     i2 != to_remove.end(); ++i2)
	{
	    std::string const& remove_dir = *i2;
	    BuildForest& remove = *(forests[remove_dir]);

	    verbose("merging forest \"" + remove_dir + "\" into \"" +
		    keep_dir);

	    BuildTree_map& o_buildtrees = remove.getBuildTrees();
	    BuildItem_map& o_builditems = remove.getBuildItems();
	    std::list<std::string>& o_backing_areas = remove.getBackingAreas();
	    std::set<std::string>& o_deleted_trees = remove.getDeletedTrees();
	    std::set<std::string>& o_deleted_items = remove.getDeletedItems();
	    std::set<std::string>& o_global_plugins =
		remove.getGlobalPlugins();

	    for (BuildTree_map::iterator i3 = o_buildtrees.begin();
		 i3 != o_buildtrees.end(); ++i3)
	    {
		std::string const& tree_name = (*i3).first;
		BuildTree_ptr tree = (*i3).second;
		addTreeToForest(keep, tree_name, tree);
	    }

	    for (BuildItem_map::iterator i3 = o_builditems.begin();
		 i3 != o_builditems.end(); ++i3)
	    {
		std::string const& item_name = (*i3).first;
		BuildItem_ptr item = (*i3).second;
		addItemToForest(keep, item_name, item);
	    }

	    backing_areas.insert(
		backing_areas.end(),
		o_backing_areas.begin(), o_backing_areas.end());
	    deleted_trees.insert(
		o_deleted_trees.begin(), o_deleted_trees.end());
	    deleted_items.insert(
		o_deleted_items.begin(), o_deleted_items.end());
	    global_plugins.insert(
		o_global_plugins.begin(), o_global_plugins.end());
	    if (remove.hasExternals())
	    {
		keep.setHasExternals();
	    }

	    forests.erase(remove_dir);
	}
    }

    for (BuildForest_map::iterator iter = forests.begin();
	 iter != forests.end(); ++iter)
    {
	std::string const& root = (*iter).first;
	BuildForest& forest = *((*iter).second);
	std::list<std::string>& backing_areas = forest.getBackingAreas();

	// coalesce backing areas
	std::set<std::string> ba_set;
	std::list<std::string>::iterator i2 = backing_areas.begin();
	while (i2 != backing_areas.end())
	{
	    std::list<std::string>::iterator next = i2;
	    ++next;
	    bool keep = false;
	    std::string ba = *i2;
	    if (forest_renames.count(ba))
	    {
		verbose("in forest \"" + root +
			"\", replacing backing area \"" +
			ba + "\" with \"" + forest_renames[ba] + "\"");
		ba = forest_renames[ba];
	    }
	    if (ba_set.count(ba))
	    {
		// skip duplicate
	    }
	    else
	    {
		ba_set.insert(ba);
		keep = true;
	    }
	    if (keep)
	    {
		*i2 = ba;
	    }
	    else
	    {
		backing_areas.erase(i2, next);
	    }
	    i2 = next;
	}
    }
}

void
Abuild::removeEmptyTrees(BuildForest_map& forests)
{
    assert(this->compat_level.allow_1_0());

    // Top-level child-only build items look like build tree roots
    // with 1.0 compatibility turned on.  If any of our build trees
    // are like this and have no items, remove them from the forest.
    // Otherwise, we end up with assigned tree names on empty trees.
    for (BuildForest_map::iterator iter = forests.begin();
	 iter != forests.end(); ++iter)
    {
	BuildForest& forest = *((*iter).second);
	std::set<std::string> to_delete;
	BuildTree_map& buildtrees = forest.getBuildTrees();
	for (BuildTree_map::iterator iter = buildtrees.begin();
	     iter != buildtrees.end(); ++iter)
	{
	    std::string const& tree_name = (*iter).first;
	    BuildTree& tree = *((*iter).second);
	    ItemConfig* config = readConfig(tree.getRootPath(), "");
	    if (config->isChildOnly())
	    {
		// We'll only delete this if it has no items and no
		// one depends on it.
		to_delete.insert(tree_name);
	    }
	}

	for (BuildTree_map::iterator iter = buildtrees.begin();
	     iter != buildtrees.end(); ++iter)
	{
	    BuildTree& tree = *((*iter).second);
	    std::list<std::string> const& tree_deps = tree.getTreeDeps();
	    for (std::list<std::string>::const_iterator iter =
		     tree_deps.begin();
		 iter != tree_deps.end(); ++iter)
	    {
		to_delete.erase(*iter);
	    }
	}

	BuildItem_map& builditems = forest.getBuildItems();
	for (BuildItem_map::iterator iter = builditems.begin();
	     iter != builditems.end(); ++iter)
	{
	    BuildItem& item = *((*iter).second);
	    to_delete.erase(item.getTreeName());
	}

	if (! to_delete.empty())
	{
	    QTC::TC("abuild", "Abuild-traverse delete unused empty top-level tree");
	    for (std::set<std::string>::iterator iter = to_delete.begin();
		 iter != to_delete.end(); ++iter)
	    {
		verbose("tree name " + *iter + " was assigned to an unused,"
			" child-only item; removing unneeded tree");
		buildtrees.erase(*iter);
	    }
	    if (to_delete.count(this->local_tree))
	    {
		QTC::TC("abuild", "Abuild-traverse clear local tree");
		this->local_tree.clear();
	    }
	}
    }
}

void
Abuild::traverseItems(BuildForest& forest, DependencyGraph& external_graph,
		      std::string const& top_path,
		      std::list<std::string>& dirs_with_externals,
		      std::list<std::string>& backing_areas,
		      std::set<std::string>& deleted_trees,
		      std::set<std::string>& deleted_items)
{
    if (this->compat_level.allow_1_0())
    {
	if (this->items_traversed.count(top_path))
	{
	    QTC::TC("abuild", "Abuild-traverse forest has already been seen");
	    return;
	}
	external_graph.addItem(top_path);
	this->items_traversed.insert(top_path);
    }

    verbose("traversing items for " + top_path);
    incrementVerboseIndent();

    bool has_backing_area =
	Util::isFile(top_path + "/" + BackingConfig::FILE_BACKING);
    if (has_backing_area)
    {
	verbose("reading backing area data");
	appendBackingData(
	    top_path, backing_areas, deleted_trees, deleted_items);
	verbose("done reading backing area data");
    }

    std::list<std::string> dirs;
    std::map<std::string, std::string> parent_dirs;
    std::map<std::string, std::string> dir_trees;
    std::map<std::string, std::string> dir_tree_roots; // 1.0-compat
    std::set<std::string> upgraded_tree_roots;	       // 1.0-compat
    dirs.push_back(top_path);
    while (! dirs.empty())
    {
	std::string dir = dirs.front();
	dirs.pop_front();
	std::string parent_dir;
	if (parent_dirs.count(dir))
	{
	    parent_dir = parent_dirs[dir];
	}
        ItemConfig* config = readConfig(dir, parent_dir);
	FileLocation location = config->getLocation();

	std::string tree_name;
	std::string tree_root;	// 1.0-compat
	if (dir_trees.count(parent_dir))
	{
	    tree_name = dir_trees[parent_dir];
	    if (this->compat_level.allow_1_0())
	    {
		tree_root = dir_tree_roots[parent_dir];
	    }
	}
	if (config->isTreeRoot())
	{
	    tree_name = registerBuildTree(forest, dir,
					  config, dirs_with_externals);
	    if (this->compat_level.allow_1_0())
	    {
		tree_root = dir;
		if (! config->getTreeName().empty())
		{
		    upgraded_tree_roots.insert(dir);
		}
	    }
	}
	dir_trees[dir] = tree_name;
	if (this->compat_level.allow_1_0())
	{
	    dir_tree_roots[dir] = tree_root;
	    if (config->usesDeprecatedFeatures())
	    {
		this->suggest_upgrade = true;
		if (upgraded_tree_roots.count(tree_root))
		{
		    QTC::TC("abuild", "Abuild-traverse basic deprecation warning");
		    deprecate("1.1", config->getLocation(),
			      "this file uses deprecated features, and"
			      " this build item belongs to a tree that"
			      " has already been upgraded");
		}
	    }
	}

	if (dir == this->this_config_dir)
	{
	    this->local_tree = tree_name;
	}

	std::string item_name = config->getName();
        if (! item_name.empty())
        {
	    BuildItem_ptr item(
		new BuildItem(item_name, tree_name, config));
	    addItemToForest(forest, item_name, item);
	}

	std::list<std::string> const& children = config->getChildren();
	for (std::list<std::string>::const_iterator iter = children.begin();
	     iter != children.end(); ++iter)
	{
	    std::string child_dir = Util::canonicalizePath(dir + "/" + *iter);
            if (Util::isDirectory(child_dir))
            {
		std::string child_conf =
		    Util::absToRel(child_dir + "/" + ItemConfig::FILE_CONF);
                if (Util::isFile(child_conf))
                {
		    dirs.push_back(child_dir);
		    parent_dirs[child_dir] = dir;
                }
                else
                {
                    QTC::TC("abuild", "Abuild-traverse ERR no child config");
                    error(location, "child " + child_conf + " is missing");
                }
            }
            else if (has_backing_area)
	    {
		// Allow sparse trees if we have a backing area.
		// No validation is required for the child
		// directory.
		QTC::TC("abuild", "Abuild-traverse sparse tree");
		verbose("ignoring non-existence of child dir " +
			*iter + " from " + Util::absToRel(dir) +
			" in a tree with a backing area");
	    }
	    else if (config->childIsOptional(*iter))
	    {
		QTC::TC("abuild", "Abuild-traverse ignoring missing optional child");
		verbose("ignoring non-existence of optional child dir " +
			*iter + " from " + Util::absToRel(dir));
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-traverse ERR can't resolve child");
		error(location, "unable to find child " + *iter);
	    }
	}
    }

    decrementVerboseIndent();
    verbose("done traversing items for " + top_path);
}

void
Abuild::addItemToForest(BuildForest& forest, std::string const& item_name,
			BuildItem_ptr item)
{
    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();
    if (builditems.count(item_name))
    {
	FileLocation const& loc = item->getLocation();
	FileLocation const& other_loc =
	    builditems[item_name]->getLocation();
	if (loc == other_loc)
	{
	    // This can only happen if the item is encountered from
	    // different parents, which would always be accompanied by
	    // an intervening Abuild.conf.  Don't report this an error
	    // so we avoid reporting the same condition with multiple
	    // error messages.
	    QTC::TC("abuild", "Abuild-traverse item added twice");
	}
	else
	{
	    QTC::TC("abuild", "Abuild-traverse ERR item multiple locations");
	    error(loc, "build item " + item_name +
		  " appears in multiple locations");
	    error(other_loc, "here is another location");
	}
    }
    else if (item->getTreeName().empty())
    {
	QTC::TC("abuild", "Abuild-traverse ERR named item outside of tree");
	error(item->getLocation(),
	      "named build items are not allowed outside of"
	      " named build trees");
    }
    else
    {
	if (this->compat_level.allow_1_0())
	{
	    std::string const& tree_name = item->getTreeName();
	    BuildTree& tree = *(buildtrees[tree_name]);
	    std::string const& root_path = tree.getRootPath();
	    if (Util::isFile(root_path + "/" + ItemConfig::FILE_CONF))
	    {
		ItemConfig* tree_config = readConfig(root_path, "");
		if (tree_config->getTreeName().empty())
		{
		    if (! this->suggest_upgrade)
		    {
			QTC::TC("abuild", "Abuild-traverse item in unnamed root");
			this->suggest_upgrade = true;
		    }
		}
	    }
	}

	// We want to make sure that item->getTreeName will always be
	// a valid tree, so only store the build item if everything
	// checks out.
	item->setForestRoot(forest.getRootPath());
	builditems[item_name] = item;
    }
}

std::string
Abuild::registerBuildTree(BuildForest& forest,
			  std::string const& dir,
			  ItemConfig* config,
			  std::list<std::string>& dirs_with_externals)
{
    std::string tree_name = config->getTreeName();
    std::list<std::string> tree_deps = config->getTreeDeps();

    if (this->compat_level.allow_1_0())
    {
	// Assign a name if needed.
	if (tree_name.empty())
	{
	    tree_name = getAssignedTreeName(dir);
	}

	// Map any externals to tree dependencies.
	std::list<std::string> const& externals = config->getExternals();
	if (! externals.empty())
	{
	    forest.setHasExternals();
	    dirs_with_externals.push_back(dir);
	}
	for (std::list<std::string>::const_iterator eiter = externals.begin();
	     eiter != externals.end(); ++eiter)
	{
	    std::string const& edecl = *eiter;
	    ItemConfig* econfig = readExternalConfig(dir, edecl);
	    if (econfig)
	    {
		std::string const& epath = econfig->getAbsolutePath();
		if (econfig->isTreeRoot())
		{
		    std::string ext_tree_name = econfig->getTreeName();
		    if (ext_tree_name.empty())
		    {
			ext_tree_name = getAssignedTreeName(epath);
		    }
		    else
		    {
			this->suggest_upgrade = true;
			deprecate("1.1", config->getLocation(),
				  "external \"" + edecl +
				  "\" of \"" + dir + "\" points to"
				  " a named tree in an upgraded"
				  " build area");
		    }
		    tree_deps.push_back(ext_tree_name);
		}
		else
		{
		    QTC::TC("abuild", "Abuild-traverse ERR external not root");
		    error(FileLocation(epath + "/" +
				       ItemConfig::FILE_CONF, 0, 0),
			  "this build item does not appear to be"
			  " a build tree root, but it is referenced"
			  " as external \"" + edecl + "\" of \"" +
			  dir + "\"");
		    error(config->getLocation(), "here is the referring item");
		}
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-traverse ERR can't resolve external");
		error(config->getLocation(),
		      "unable to locate or resolve external \"" +
		      edecl + "\"");
	    }
	}
    }
    else
    {
	assert(! tree_name.empty());
    }

    BuildTree_ptr tree(
	new BuildTree(tree_name, dir, tree_deps,
		      config->getOptionalTreeDeps(),
		      config->getSupportedTraits(),
		      config->getPlugins(),
		      this->internal_platform_data));
    addTreeToForest(forest, tree_name, tree);
    if (config->hasGlobalPlugins())
    {
	QTC::TC("abuild", "Abuild-traverse add global plugin");
	std::set<std::string>& global_plugins = forest.getGlobalPlugins();
	std::set<std::string> const& t_global_plugins =
	    config->getGlobalPlugins();
	global_plugins.insert(
	    t_global_plugins.begin(), t_global_plugins.end());
    }

    return tree_name;
}

void
Abuild::addTreeToForest(BuildForest& forest, std::string const& tree_name,
			BuildTree_ptr tree)
{
    BuildTree_map& buildtrees = forest.getBuildTrees();
    if (buildtrees.count(tree_name))
    {
	if (tree->getRootPath() == buildtrees[tree_name]->getRootPath())
	{
	    // See comments in addItemToForest for an explanation of
	    // when this can happen.  We ignore this here to avoid
	    // having error messages for the same error.
	}
	else
	{
	    QTC::TC("abuild", "Abuild-traverse ERR duplicate tree");
	    error(tree->getLocation(),
		  "another tree with the name \"" + tree_name + "\" has been"
		  " found in this forest");
	    error(buildtrees[tree_name]->getLocation(),
		  "here is the other tree");
	}
    }
    else
    {
	tree->setForestRoot(forest.getRootPath());
	buildtrees[tree_name] = tree;
    }
}

std::string
Abuild::getAssignedTreeName(std::string const& dir,
			    bool use_backing_name_only)
{
    assert(this->compat_level.allow_1_0());

    if (this->assigned_tree_names.count(dir))
    {
	QTC::TC("abuild", "Abuild-traverse previously assigned tree name");
	return this->assigned_tree_names[dir];
    }

    // Assign a name to this tree.  In order for inheriting externals
    // from backing areas to work properly, we need to take the
    // backing area's tree name if this is a 1.0 root with a backing
    // file.  This means we end up traversing the backing chain at
    // this point.
    std::set<std::string> visiting;
    return getAssignedTreeName(dir, visiting, use_backing_name_only);
}

std::string
Abuild::getAssignedTreeName(std::string const& dir,
			    std::set<std::string>& visiting,
			    bool use_backing_name_only)
{
    assert(this->compat_level.allow_1_0());

    // We check visiting before calling recursively so we can create a
    // better error message.
    assert(! visiting.count(dir));
    visiting.insert(dir);

    std::string tree_name;

    if (Util::isFile(dir + "/" + BackingConfig::FILE_BACKING))
    {
	BackingConfig* backing = readBacking(dir);
	if (backing->isDeprecated() &&
	    (! backing->getBackingAreas().empty()))
	{
	    // This is an old-style backing area.  It must point to a
	    // tree root which is presumably the root of the
	    // corresponding tree in the backing area.

	    std::list<std::string> const& backing_areas =
		backing->getBackingAreas();
	    assert(backing_areas.size() == 1);
	    std::string const& backing_area = backing_areas.front();
	    if (! Util::isFile(backing_area + "/" + ItemConfig::FILE_CONF))
	    {
		// This may be precluded by other code.  Don't bother
		// with a coverage case.
		fatal("1.0-style " + BackingConfig::FILE_BACKING +
		      " file in " + dir + " appears not to point"
		      " to a directory containing " + ItemConfig::FILE_CONF);
	    }
	    ItemConfig* config = readConfig(backing_area, "");
	    if (! config->isTreeRoot())
	    {
		fatal("1.0-style " + BackingConfig::FILE_BACKING +
		      " file in " + dir + " appears not to point"
		      " to a tree root");
	    }
	    tree_name = config->getTreeName();
	    if (! tree_name.empty())
	    {
		QTC::TC("abuild", "Abuild-traverse inherit tree name from backing");
	    }
	    else if (this->assigned_tree_names.count(backing_area))
	    {
		// I don't think this can actually happen because of
		// the order in which we see build trees.
		tree_name = this->assigned_tree_names[backing_area];
	    }
	    else
	    {
		if (visiting.count(backing_area))
		{
		    QTC::TC("abuild", "Abuild-traverse ERR backing area loop");
		    fatal("backing area loop found for " + backing_area +
			  ", a backing area of " + dir);
		}
		else
		{
		    QTC::TC("abuild", "Abuild-traverse trying backing of backing");
		    tree_name = getAssignedTreeName(
			backing_area, visiting, use_backing_name_only);
		}
	    }
	}
    }

    if (tree_name.empty() && (! use_backing_name_only))
    {
	// If we got here, this tree doesn't have a backing area, so
	// we'll have to generate a tree name.  The random number is
	// there to prevent people from ever being able to rely on
	// what the tree is called, which in turn should hopefully
	// encourage people to upgrade.
	tree_name = "tree." +
	    Util::intToString(++this->last_assigned_tree_number) +
	    ".-" + Util::intToString(std::rand() % 9999) + "-";
	this->assigned_tree_names[dir] = tree_name;
	verbose("assigned tree name " + tree_name + " to " + dir);
    }

    visiting.erase(dir);

    return tree_name;
}

void
Abuild::validateForest(BuildForest_map& forests, std::string const& top_path)
{
    verbose("build tree " + top_path + ": validating");

    BuildForest& forest = *(forests[top_path]);

    // Many of these checks have side effects.  The order of these
    // checks is very sensitive as some checks depend upon side
    // effects of other operations having been completed.
    forest.propagateGlobals();
    resolveFromBackingAreas(forests, top_path);
    checkTreeDependencies(forest);
    resolveTraits(forest);
    checkPlatformTypes(forest);
    checkPlugins(forest);
    checkItemNames(forest);
    checkItemDependencies(forest);
    checkBuildAlso(forest);
    checkDepTreeAccess(forest);
    updatePlatformTypes(forest);
    checkDependencyPlatformTypes(forest);
    checkFlags(forest);
    checkTraits(forest);
    checkIntegrity(forests, top_path);
    if (this->full_integrity)
    {
	reportIntegrityErrors(forests, forest.getBuildItems(), top_path);
    }
    computeBuildablePlatforms(forest);

    verbose("build tree " + top_path + ": validation completed");
}

void
Abuild::checkTreeDependencies(BuildForest& forest)
{
    // Make sure that there are no cycles or errors (references to
    // non-existent trees) in the dependency graph of build trees.
    // This code is essentially identical to checkItemDependencies
    // but with enough surface differences to be different code.

    // Create a dependency graph for all build trees in this forest.

    BuildTree_map& buildtrees = forest.getBuildTrees();

    DependencyGraph g;
    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	std::string const& tree_name = (*iter).first;
	BuildTree& tree = *((*iter).second);
	std::set<std::string> const& optional_tree_deps =
	    tree.getOptionalTreeDeps();
	g.addItem(tree_name);
	// dependencies is a copy, not a const reference, to tree
	// dependencies so we don't modify it (through removeTreeDep)
	// while iterating through it.
	std::list<std::string> dependencies = tree.getTreeDeps();
	for (std::list<std::string>::const_iterator i2 = dependencies.begin();
	     i2 != dependencies.end(); ++i2)
        {
	    std::string const& tree_dep = *i2;
	    if (optional_tree_deps.count(tree_dep) &&
		buildtrees.count(tree_dep) == 0)
	    {
		QTC::TC("abuild", "Abuild-traverse skipping optional tree dependency");
		tree.removeTreeDep(tree_dep);
		continue;
	    }
	    g.addDependency(tree_name, tree_dep);
        }
    }

    bool check_okay = g.check();

    // Whether or not we found errors, create a table indicating which
    // trees can see which other trees.
    std::map<std::string, std::set<std::string> >& tree_access_table =
	forest.getTreeAccessTable();

    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	std::string const& tree_name = (*iter).first;
	BuildTree& tree = *((*iter).second);
	DependencyGraph::ItemList const& sdeps =
	    g.getSortedDependencies(tree_name);
	tree.setExpandedTreeDeps(sdeps);
	for (std::list<std::string>::const_iterator i2 = sdeps.begin();
	     i2 != sdeps.end(); ++i2)
	{
	    tree_access_table[tree_name].insert(*i2);
	}
    }

    forest.setSortedTreeNames(g.getSortedGraph());

    if (! check_okay)
    {
	DependencyGraph::ItemMap unknowns;
	std::vector<DependencyGraph::ItemList> cycles;
	g.getErrors(unknowns, cycles);

	for (DependencyGraph::ItemMap::iterator iter = unknowns.begin();
	     iter != unknowns.end(); ++iter)
	{
	    std::string const& node = (*iter).first;
	    DependencyGraph::ItemList const& unknown_items = (*iter).second;
	    for (DependencyGraph::ItemList::const_iterator i2 =
		     unknown_items.begin();
		 i2 != unknown_items.end(); ++i2)
	    {
		std::string const& unknown = *i2;
		QTC::TC("abuild", "Abuild-traverse ERR unknown tree dependency");
		error(buildtrees[node]->getLocation(),
		      "tree " + node + " depends on unknown build tree " +
		      unknown);
	    }
	}

	std::set<std::string> cycle_trees;
	for (std::vector<DependencyGraph::ItemList>::const_iterator iter =
		 cycles.begin();
	     iter != cycles.end(); ++iter)
	{
	    DependencyGraph::ItemList const& data = *iter;
	    QTC::TC("abuild", "Abuild-traverse ERR circular tree dependency");
	    std::string cycle = Util::join(" -> ", data);
	    cycle += " -> " + data.front();
	    error("circular dependency detected among build trees: " + cycle);
	    for (DependencyGraph::ItemList::const_iterator i2 = data.begin();
		 i2 != data.end(); ++i2)
	    {
		cycle_trees.insert(*i2);
	    }
	}

	for (std::set<std::string>::iterator iter = cycle_trees.begin();
	     iter != cycle_trees.end(); ++iter)
        {
            error(buildtrees[*iter]->getLocation(),
		  *iter + " participates in a circular tree dependency");
        }
    }
}

void
Abuild::resolveFromBackingAreas(BuildForest_map& forests,
				std::string const& top_path)
{
    BuildForest& forest = *(forests[top_path]);
    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();
    std::list<std::string>& backing_areas = forest.getBackingAreas();
    std::set<std::string> const& trees_to_delete = forest.getDeletedTrees();
    std::set<std::string> const& items_to_delete = forest.getDeletedItems();
    std::set<std::string> trees_not_deleted = trees_to_delete;
    std::set<std::string> items_not_deleted = items_to_delete;

    // The most likely location for a problem is the forest root
    // Abuild.backing file.  In 1.0 compatibility mode, it might be
    // some Abuild.conf, but we're not going to keep track of the
    // origin of ever single item/tree deletion request.
    FileLocation location(top_path + "/" + BackingConfig::FILE_BACKING, 0, 0);

    // If A backs to B and C and if B backs to C, we want to ignore
    // backing area C of A and let A get C's items through B.  This
    // helps to seeing duplicates needlessly.  We'll make this
    // determination in a manner that allows us to preserve the order
    // in which backing areas were specified to the maximum possible
    // extent.
    DependencyGraph backing_graph;
    computeBackingGraph(forests, backing_graph);
    std::set<std::string> covered;
    for (std::list<std::string>::iterator iter = backing_areas.begin();
	 iter != backing_areas.end(); ++iter)
    {
	std::list<std::string> deps =
	    backing_graph.getSortedDependencies(*iter);
	assert(deps.back() == *iter);
	deps.pop_back();
	covered.insert(deps.begin(), deps.end());
    }

    { // local scope
	std::list<std::string>::iterator iter = backing_areas.begin();
	while (iter != backing_areas.end())
	{
	    std::list<std::string>::iterator next = iter;
	    ++next;
	    if (covered.count(*iter))
	    {
		// It is not necessary to tell the user about this.
		// The user might have done this on purpose to
		// indicate a desire to back from the first backing
		// area's backing area even if the first one is later
		// reconfigured, or it may be a remnant from before
		// one of the backing areas pointed to the other.  As
		// we are able to work around this safely and the
		// situation is not harmful, issuing a warning would
		// just be an annoyance.
		QTC::TC("abuild", "Abuild-traverse skipping covered backing area");
		verbose("backing area " + *iter + " is being removed because"
			" it is covered by another backing area");
		backing_areas.erase(iter, next);
	    }
	    iter = next;
	}
    }
    if (backing_areas.size() > 1)
    {
	QTC::TC("abuild", "Abuild-traverse multiple backing areas");
    }

    // Copy trees and items from the backing areas.  Exclude any
    // deleted trees, deleted items, or items in deleted trees.

    for (std::list<std::string>::const_iterator iter = backing_areas.begin();
	 iter != backing_areas.end(); ++iter)
    {
	BuildTree_map const& backing_trees = forests[*iter]->getBuildTrees();
	for (BuildTree_map::const_iterator tree_iter = backing_trees.begin();
	     tree_iter != backing_trees.end(); ++tree_iter)
	{
	    std::string const& tree_name = (*tree_iter).first;
	    BuildTree const& tree = *((*tree_iter).second);
	    if (trees_to_delete.count(tree_name))
	    {
		QTC::TC("abuild", "Abuild-traverse not copying deleted tree");
		trees_not_deleted.erase(tree_name);
		if (buildtrees.count(tree_name))
		{
		    // This is not an error.  If you want to replace
		    // one tree with another one with the same name,
		    // that's fine as deleting the tree also prevents
		    // copying the old tree's items.  With items, it's
		    // different -- replacing an item is sufficient to
		    // get rid of the old one.  You don't have t do
		    // delete it too.
		    QTC::TC("abuild", "Abuild-traverse deleted tree exists locally");
		}
	    }
	    else if (buildtrees.count(tree_name))
	    {
		if (buildtrees[tree_name]->isLocal())
		{
		    QTC::TC("abuild", "Abuild-traverse override build tree");
		}
		else
		{
		    FileLocation const& loc = tree.getLocation();
		    FileLocation const& other_loc =
			buildtrees[tree_name]->getLocation();
		    // See comment near this same check for build
		    // items for why this assertion pass.
		    assert(! (loc == other_loc));
		    QTC::TC("abuild", "Abuild-traverse ERR tree multiple backing areas");
		    error(loc, "this tree appears in multiple backing areas");
		    error(other_loc, "here is another location for this tree");
		}
	    }
	    else
	    {
		// Copy build tree information from backing area
		buildtrees[tree_name].reset(new BuildTree(tree));
		BuildTree& new_tree = *(buildtrees[tree_name]);
		new_tree.incrementBackingDepth();
	    }
        }

	BuildItem_map const& backing_items = forests[*iter]->getBuildItems();
	for (BuildItem_map::const_iterator item_iter = backing_items.begin();
	     item_iter != backing_items.end(); ++item_iter)
	{
	    std::string const& item_name = (*item_iter).first;
	    BuildItem const& item = *((*item_iter).second);
	    std::string const& tree_name = item.getTreeName();
	    if (trees_to_delete.count(tree_name))
	    {
		QTC::TC("abuild", "Abuild-traverse not copying item from deleted tree");
		if (builditems.count(item_name) &&
		    builditems[item_name]->getTreeName() != tree_name)
		{
		    QTC::TC("abuild", "Abuild-traverse replace item from deleted tree");
		}
	    }
	    else if (items_to_delete.count(item_name))
	    {
		QTC::TC("abuild", "Abuild-traverse not copying deleted item");
		items_not_deleted.erase(item_name);
		if (builditems.count(item_name))
		{
		    QTC::TC("abuild", "Abuild-traverse ERR deleted item exists locally");
		    error(location,
			  "item \"" + item_name + "\" is marked for"
			  " deletion, but it appears locally in this"
			  " forest");
		    error(builditems[item_name]->getLocation(),
			  "here is the location of the item in"
			  " this forest");
		}
	    }
	    else if (builditems.count(item_name))
	    {
		if (builditems[item_name]->isLocal())
		{
		    QTC::TC("abuild", "Abuild-traverse override build item");
		}
		else
		{
		    FileLocation const& loc = item.getLocation();
		    FileLocation const& other_loc =
			builditems[item_name]->getLocation();
		    // It should be impossible for us to see the same
		    // build item from multiple backing areas.  The
		    // only way this should be able to happen would be
		    // if one backing area depends on the other, and
		    // we've already detected and precluded that case
		    // by checking "covered" above.  If this assertion
		    // fails, there is probably a logic error either
		    // there or in merging forests.
		    assert(! (loc == other_loc));
		    QTC::TC("abuild", "Abuild-traverse ERR item multiple backing areas");
		    error(loc, "this item appears in multiple backing areas");
		    error(other_loc, "here is another location for this item");
		}
	    }
	    else
	    {
		// Copy build item information from backing area
		builditems[item_name].reset(new BuildItem(item));
		BuildItem& new_item = *(builditems[item_name]);
		new_item.incrementBackingDepth();
		if (new_item.getBackingDepth() > 1)
		{
		    QTC::TC("abuild", "Abuild-traverse backing depth > 1");
		}
	    }
        }
    }

    for (std::set<std::string>::iterator iter = trees_not_deleted.begin();
	 iter != trees_not_deleted.end(); ++iter)
    {
	QTC::TC("abuild", "Abuild-traverse ERR deleting non-existent tree");
	error(location,
	      "tree \"" + *iter + "\" was marked for deletion"
	      " but was not seen in a backing area");
    }
    for (std::set<std::string>::iterator iter = items_not_deleted.begin();
	 iter != items_not_deleted.end(); ++iter)
    {
	QTC::TC("abuild", "Abuild-traverse ERR deleting non-existent item");
	error(location,
	      "item \"" + *iter + "\" was marked for deletion"
	      " but was not seen in a backing area");
    }
}

void
Abuild::checkDepTreeAccess(BuildForest& forest)
{
    // Check to make sure no item references an item in a tree that
    // its tree does not depend on

    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    // If item is not local, it was checked in its home
	    // forest.  If it depends on items in the local tree,
	    // that's a different error and will be reported as such.
	    continue;
	}
	std::list<std::string> const& alldeps = item.getExpandedDependencies();
	for (std::list<std::string>::const_iterator diter = alldeps.begin();
	     diter != alldeps.end(); ++diter)
	{
	    std::string const& dep_name = (*diter);
	    if (! builditems.count(dep_name))
	    {
		continue;
	    }
	    BuildItem& dep_item = *(builditems[dep_name]);
	    if (! checkAllowedTreeItem(forest, item, dep_item, "depend on"))
	    {
		QTC::TC("abuild", "Abuild-traverse ERR depend on invisible item");
	    }
	}
    }
}

bool
Abuild::checkAllowedTreeItem(BuildForest& forest,
			     BuildItem& referrer,
			     BuildItem& referent,
			     std::string const& action)
{
    bool okay = true;
    std::map<std::string, std::set<std::string> > const& tree_access_table =
	forest.getTreeAccessTable();
    std::string const& referrer_name = referrer.getName();
    std::string const& referrer_tree = referrer.getTreeName();
    std::string const& referent_name = referent.getName();
    std::string const& referent_tree = referent.getTreeName();

    std::set<std::string> const& allowed_trees =
	(*(tree_access_table.find(referrer_tree))).second;
    if (allowed_trees.count(referent_tree) == 0)
    {
	okay = false;
	error(referrer.getLocation(),
	      "build item \"" + referrer_name + "\" may not " +
	      action + " build item \"" + referent_name + "\" because \"" +
	      referrer_name + "\"'s tree (\"" + referrer_tree +
	      "\") does not depend on \"" + referent_name +
	      "\"'s tree (\"" + referent_tree + "\")");
    }
    return okay;
}

bool
Abuild::checkAllowedTreeTree(BuildForest& forest,
			     BuildItem& referrer,
			     std::string const& referent_tree,
			     std::string const& action)
{
    bool okay = true;
    std::map<std::string, std::set<std::string> > const& tree_access_table =
	forest.getTreeAccessTable();
    std::string const& referrer_name = referrer.getName();
    std::string const& referrer_tree = referrer.getTreeName();

    std::set<std::string> const& allowed_trees =
	(*(tree_access_table.find(referrer_tree))).second;
    if (allowed_trees.count(referent_tree) == 0)
    {
	okay = false;
	error(referrer.getLocation(),
	      "build item \"" + referrer_name + "\" may not " +
	      action + " build tree \"" + referent_tree + "\" because \"" +
	      referrer_name + "\"'s tree (\"" + referrer_tree +
	      "\") does not depend on it");
    }
    return okay;
}

void
Abuild::resolveTraits(BuildForest& forest)
{
    // Merge supported traits of each tree's dependencies into the tree.

    BuildTree_map& buildtrees = forest.getBuildTrees();
    std::list<std::string> treenames = forest.getSortedTreeNames();
    for (std::list<std::string>::iterator iter = treenames.begin();
	 iter != treenames.end(); ++iter)
    {
	std::string const& tree_name = *iter;
	BuildTree& tree = *(buildtrees[tree_name]);
	std::list<std::string> const& deps = tree.getExpandedTreeDeps();
	for (std::list<std::string>::const_iterator i2 = deps.begin();
	     i2 != deps.end(); ++i2)
	{
	    if (buildtrees.count(*i2) == 0)
	    {
		// An error would have reported already
		continue;
	    }
	    BuildTree& dtree = *(buildtrees[*i2]);
	    tree.addTraits(dtree.getSupportedTraits());
	}
    }
}

void
Abuild::checkPlugins(BuildForest& forest)
{
    // Make sure each plugin exists and lives in an accessible tree,
    // and set each tree's plugin list.

    BuildItem_map& builditems = forest.getBuildItems();
    BuildTree_map& buildtrees = forest.getBuildTrees();
    std::set<std::string> const& global_plugins = forest.getGlobalPlugins();

    if (this->compat_level.allow_1_0())
    {
	if (! global_plugins.empty() && forest.hasExternals())
	{
	    QTC::TC("abuild", "Abuild-traverse ERR global plugins with externals");
	    error("at least one build tree in the forest rooted at " +
		  forest.getRootPath() + " uses external-dirs, and "
		  " global plugins are in use; global plugins may not"
		  " be used until all tree dependencies are converted to"
		  " named tree dependencies");
	}
    }

    // tree -> plugins in that tree
    std::map<std::string, std::set<std::string> > plugin_data;

    std::map<std::string, std::set<std::string> > const& access_table =
	forest.getTreeAccessTable();
    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	std::string const& tree_name = (*iter).first;
	plugin_data[tree_name].empty(); // force this entry to exist
	BuildTree& tree = *((*iter).second);
	if (! tree.isLocal())
	{
	    continue;
	}
	std::set<std::string> const& allowed_trees =
	    (*(access_table.find(tree_name))).second;
	std::list<std::string> const& plugins = tree.getPlugins();
	for (std::list<std::string>::const_iterator iter = plugins.begin();
	     iter != plugins.end(); ++iter)
	{
	    std::string const& item_name = *iter;
	    if (builditems.count(item_name) == 0)
	    {
		QTC::TC("abuild", "Abuild-traverse ERR invalid plugin");
		error(tree.getLocation(),
		      "plugin \"" + item_name + "\" does not exist");
		continue;
	    }
	    BuildItem& item = *(builditems[item_name]);
	    std::string const& item_tree = item.getTreeName();
	    if (global_plugins.count(item_name))
	    {
		QTC::TC("abuild", "Abuild-traverse allow access to global plugin");
	    }
	    else if (allowed_trees.count(item_tree) == 0)
	    {
		QTC::TC("abuild", "Abuild-traverse ERR plugin in invisible tree");
		error(tree.getLocation(),
		      "this tree may not declare item \"" +
		      item_name + "\" as a plugin because because its"
		      " tree does not depend on \"" + item_name +
		      "\"'s tree (\"" + item_tree + "\")");
		continue;
	    }

	    this->plugins.insert(item_name);
	    plugin_data[tree_name].insert(item_name);

	    bool item_error = false;
	    // Although whether or not an item has dependencies is not
	    // context-specific, whether or not an item is a plugin
	    // is.
	    if (! item.getDeps().empty())
	    {
		QTC::TC("abuild", "Abuild-traverse ERR plugin with dependencies");
		item_error = true;
		error(tree.getLocation(),
		      "item \"" + item_name + "\" is declared as a plugin,"
		      " but it has dependencies");
	    }
	    if (! item.getBuildAlso().empty())
	    {
		QTC::TC("abuild", "Abuild-traverse ERR plugin with build-also");
		item_error = true;
		error(tree.getLocation(),
		      "item \"" + item_name + "\" is declared as a plugin,"
		      " but it specifies other items to build");
	    }

	    // checkPlatformTypes() must have already been called.
	    // Require all plugins to be build items of target type
	    // "all".  This means that the don't build anything or
	    // have an Abuild.interface file.
	    if (item.getTargetType() != TargetType::tt_all)
	    {
		QTC::TC("abuild", "Abuild-traverse ERR plugin with target type !all");
		item_error = true;
		error(tree.getLocation(),
		      "item \"" + item_name + "\" is declared as a plugin,"
		      " but it has a build or non-plugin interface file");
	    }

	    if (item_error)
	    {
		error(item.getLocation(),
		      "here is \"" + item_name + "\"'s " +
		      ItemConfig::FILE_CONF);
		if (global_plugins.count(item_name))
		{
		    // This is a very unfortunate situation, but it's
		    // probably not worth coding around it.  If
		    // someone botches up with a global plugin by
		    // giving it dependencies, build-also, or other
		    // things that cause errors above, they will see a
		    // whole bunch of errors right away and will
		    // probably understand what caused them.  Trying
		    // to repeat all these checks separately for
		    // global plugins would needlessly complicate the
		    // code for a rather obscure error condition.
		    QTC::TC("abuild", "Abuild-traverse ERR error for global plugin");
		    error(item.getLocation(),
			  "NOTE: this item declares itself as a global plugin,"
			  " so some of the above error messages may be"
			  " repeated for every tree, including those"
			  " that do not explicitly declare the item"
			  " to be a plugin");
		}
	    }
	}
    }

    // Check to make sure no item depends on an item that has been
    // declared as a plugin for its tree.  Also store the list of
    // plugins each build item should see.  Do this only for items
    // local to this forest.

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}

	// Store the list of plugins
	std::string const& tree_name = item.getTreeName();
	BuildTree& tree = *(buildtrees[tree_name]);
	item.setPlugins(tree.getPlugins());

	// Check dependencies
	std::list<std::string> const& deps = item.getDeps();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
	{
	    std::string const& dep_name = (*dep_iter);
	    // If the dependency doesn't exist, we'll be reporting
	    // that later.
	    if (builditems.count(dep_name) != 0)
	    {
		if (plugin_data[tree_name].count(dep_name))
		{
		    QTC::TC("abuild", "Abuild-traverse ERR item depends on plugin");
		    error(item.getLocation(),
			  "this item depends on \"" + dep_name + "\","
			  " which is declared as a plugin");
		    if (global_plugins.count(dep_name))
		    {
			error(builditems[dep_name]->getLocation(),
			      "the dependency is declared as a global plugin");
		    }
		    else
		    {
			error(tree.getLocation(),
			      "the dependency is declared as a plugin here");
		    }
		}
	    }
	}
    }
}

bool
Abuild::isPlugin(std::string const& item)
{
    return (this->plugins.count(item) != 0);
}

void
Abuild::checkPlatformTypes(BuildForest& forest)
{
    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();

    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	BuildTree& tree = *((*iter).second);
	if (! tree.isLocal())
	{
	    continue;
	}
	PlatformData& platform_data = *(tree.getPlatformData());

	// Load any additional platform data from our plugins
	std::list<std::string> const& plugins = tree.getPlugins();
	for (std::list<std::string>::const_iterator iter = plugins.begin();
	     iter != plugins.end(); ++iter)
	{
	    std::string const& plugin_name = *iter;
	    if (builditems.count(plugin_name) != 0)
	    {
		BuildItem& plugin = *(builditems[plugin_name]);
		loadPlatformData(platform_data, plugin.getAbsolutePath());
	    }
	}
    }

    // Validate all local build items to ensure that they have valid
    // platform types.

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);

	// Check only local build items
	if (! item.isLocal())
	{
	    continue;
	}

	BuildTree& tree = *(buildtrees[item.getTreeName()]);
	PlatformData& platform_data = *(tree.getPlatformData());
	FileLocation const& location = item.getLocation();
	std::set<std::string> platform_types = item.getPlatformTypes();

	TargetType::target_type_e target_type = TargetType::tt_all;

	std::map<TargetType::target_type_e, int> target_types;
	for (std::set<std::string>::const_iterator iter =
		 platform_types.begin();
	     iter != platform_types.end(); ++iter)
	{
	    std::string const& platform_type = *iter;
	    if (platform_data.isPlatformTypeValid(platform_type))
	    {
		TargetType::target_type_e target_type =
		    platform_data.getTargetType(platform_type);
		if (target_types.count(target_type) == 0)
		{
		    target_types[target_type] = 1;
		}
		else
		{
		    ++target_types[target_type];
		}
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-traverse ERR unknown platform type");
		error(location,
		      "unknown platform type \"" + *iter + "\"");
	    }
	}
	if (target_types.size() == 1)
	{
	    target_type = (*(target_types.begin())).first;
	    QTC::TC("abuild", "Abuild-traverse multiple target types",
		    (*(target_types.begin())).second == 1 ? 0 : 1);
	}
	else if (target_types.size() > 1)
	{
	    QTC::TC("abuild", "Abuild-traverse ERR incompatible platform types");
	    error(location,
		  "platforms in different target types may not be mixed");
	}
	else
	{
	    // No platform data -- this item doesn't build anything.
	    target_type = TargetType::tt_all;
	}

	item.setTargetType(target_type);
    }
}

void
Abuild::checkItemNames(BuildForest& forest)
{
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem const& item = *((*iter).second);
	std::string const& item_name = (*iter).first;
	FileLocation const& item_location = item.getLocation();
	std::list<std::string> const& dependencies = item.getDeps();
	for (std::list<std::string>::const_iterator diter =
		 dependencies.begin();
	     diter != dependencies.end(); ++diter)
	{
	    std::string const& dep = *diter;
	    if (! accessibleFrom(builditems, item_name, dep))
	    {
		QTC::TC("abuild", "Abuild-traverse ERR inaccessible dep");
		error(item_location,
		      item_name + " may not depend on " + dep +
		      " because it is private to another scope");
	    }
	}
    }
}

void
Abuild::checkBuildAlso(BuildForest& forest)
{
    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}
	std::string const& item_name = (*iter).first;
	FileLocation const& item_location = item.getLocation();

	std::list<ItemConfig::BuildAlso> const& build_also =
	    item.getBuildAlso();
	for (std::list<ItemConfig::BuildAlso>::const_iterator biter =
		 build_also.begin();
	     biter != build_also.end(); ++biter)
	{
	    std::string other_item;
	    bool is_tree;
	    bool desc;
	    bool with_tree_deps;
	    (*biter).getDetails(other_item, is_tree, desc, with_tree_deps);
	    if (is_tree)
	    {
		if (buildtrees.count(other_item) == 0)
		{
		    QTC::TC("abuild", "Abuild-traverse ERR invalid build also tree");
		    error(item_location,
			  item_name +
			  " requests build of unknown build tree \"" +
			  other_item + "\"");
		}
		else if (! checkAllowedTreeTree(
			     forest, item, other_item,
			     "request build of"))
		{
		    QTC::TC("abuild", "Abuild-traverse ERR build-also invisible tree");
		}
	    }
	    else
	    {
		if (builditems.count(other_item) == 0)
		{
		    QTC::TC("abuild", "Abuild-traverse ERR invalid build also");
		    error(item_location,
			  item_name +
			  " requests building of unknown build item \"" +
			  other_item + "\"");
		}
		else if (! checkAllowedTreeItem(
			     forest, item, *(builditems[other_item]),
			     "request build of"))
		{
		    QTC::TC("abuild", "Abuild-traverse ERR build-also invisible item");
		}
	    }
	}
    }
}

bool
Abuild::accessibleFrom(BuildItem_map& builditems,
		       std::string const& accessor_name,
		       std::string const& accessee_name)
{
    boost::regex scope_re("(.*\\.)[^\\.]+");
    boost::smatch match;

    bool accessible = true;
    if (boost::regex_match(accessee_name, match, scope_re))
    {
	// This is not a public build item.  By default, the scope of
	// visibility is the parent of this build item.  We assign
	// "scope" the scope with a trailing period.
	std::string scope = match[1].str();

	std::string visibility;
	if (builditems.count(accessee_name))
	{
	    visibility = builditems[accessee_name]->getVisibleTo();
	}
	if (visibility == "*")
	{
	    // This item is globally visible
	    QTC::TC("abuild", "Abuild-traverse globally visible item");
	    scope.clear();
	}
	else if (! visibility.empty())
	{
	    // Removing the trailing * from the visibility gives us
	    // the actual scope.
	    QTC::TC("abuild", "Abuild-traverse set scope from visibility");
	    assert(*(visibility.rbegin()) == '*');
	    scope = visibility.substr(0, visibility.length() - 1);
	}

	if (accessor_name.substr(0, scope.length()) == scope)
	{
	    QTC::TC("abuild", "Abuild-traverse accessor sees ancestor",
		    (visibility.length() > 1) ? 1 : 0);
	}
	else if (scope == accessor_name + ".")
	{
	    QTC::TC("abuild", "Abuild-traverse accessor sees child");
	}
	else
	{
	    accessible = false;
	}
    }
    else
    {
	// This is a public build item.
    }

    return accessible;
}

void
Abuild::checkItemDependencies(BuildForest& forest)
{
    // Make sure that there are no cycles or errors (references to
    // non-existent build items) in the dependency graph.

    BuildItem_map& builditems = forest.getBuildItems();
    DependencyGraph g;
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	std::set<std::string> const& optional_deps = item.getOptionalDeps();
	g.addItem(item_name);
	// dependencies is a copy, not a const reference, since our
	// calls to item.setOptionalDependencyPresence will
	// potentially modify the item's dependency list, and we don't
	// want to be interating through it at the time.
	std::list<std::string> dependencies = item.getDeps();
	for (std::list<std::string>::iterator i2 = dependencies.begin();
	     i2 != dependencies.end(); ++i2)
        {
	    std::string const& dep = *i2;
	    if (optional_deps.count(dep))
	    {
		bool present = (builditems.count(dep) != 0);
		item.setOptionalDependencyPresence(dep, present);
		if (! present)
		{
		    QTC::TC("abuild", "Abuild-traverse skipping optional dependency");
		    continue;
		}
	    }
	    g.addDependency(item_name, dep);
        }
    }

    bool check_okay = g.check();

    // Whether or not we found errors, capture the expanded dependency
    // list for each build item.
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	item.setExpandedDependencies(g.getSortedDependencies(item_name));
    }

    forest.setSortedItemNames(g.getSortedGraph());

    if (! check_okay)
    {
	DependencyGraph::ItemMap unknowns;
	std::vector<DependencyGraph::ItemList> cycles;
	g.getErrors(unknowns, cycles);

	for (DependencyGraph::ItemMap::iterator iter = unknowns.begin();
	     iter != unknowns.end(); ++iter)
	{
	    std::string const& node = (*iter).first;
	    DependencyGraph::ItemList const& unknown_items = (*iter).second;
	    for (DependencyGraph::ItemList::const_iterator i2 =
		     unknown_items.begin();
		 i2 != unknown_items.end(); ++i2)
	    {
		std::string const& unknown = *i2;
		QTC::TC("abuild", "Abuild-traverse ERR unknown dependency");
		error(builditems[node]->getLocation(),
		      node + " depends on unknown build item " + unknown);
	    }
	}

	std::set<std::string> cycle_items;
	for (std::vector<DependencyGraph::ItemList>::const_iterator iter =
		 cycles.begin();
	     iter != cycles.end(); ++iter)
	{
	    // Don't make this a warning.  See abuild document for a
	    // discussion of circular dependencies and how to resolve
	    // them.
	    DependencyGraph::ItemList const& data = *iter;
	    QTC::TC("abuild", "Abuild-traverse ERR circular dependency");
	    std::string cycle = Util::join(" -> ", data);
	    cycle += " -> " + data.front();
	    error("circular dependency detected: " + cycle);
	    for (DependencyGraph::ItemList::const_iterator i2 = data.begin();
		 i2 != data.end(); ++i2)
	    {
		cycle_items.insert(*i2);
	    }
	}

	for (std::set<std::string>::iterator iter = cycle_items.begin();
	     iter != cycle_items.end(); ++iter)
        {
            error(builditems[*iter]->getLocation(),
		  *iter + " participates in a circular dependency");
        }
    }
}

void
Abuild::updatePlatformTypes(BuildForest& forest)
{
    // For every build item with target type "all", if all the item's
    // direct dependencies have the same platform types, inherit
    // platform types and target type from them.  We have to perform
    // this check in reverse dependency order/forward build order
    // (from most depended-on to least depended-on) so that any
    // dependencies that were originally target type "all" but didn't
    // have to be have already been updated.

    BuildItem_map& builditems = forest.getBuildItems();
    std::list<std::string> const& sorted_items =
	forest.getSortedItemNames();
    for (std::list<std::string>::const_iterator iter = sorted_items.begin();
	 iter != sorted_items.end(); ++iter)
    {
	std::string const& item_name = *iter;
	BuildItem& item = *(builditems[item_name]);
	if (! item.isLocal())
	{
	    continue;
	}
	TargetType::target_type_e target_type = item.getTargetType();
	if (target_type != TargetType::tt_all)
	{
	    continue;
	}
	bool candidate = true;
	bool first = true;
	std::string dep_platform_types;
	std::list<std::string> const& deps = item.getDeps();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
	{
	    std::string const& dep_name = *dep_iter;
	    if (builditems.count(dep_name) == 0)
	    {
		candidate = false;
		break;
	    }
	    BuildItem& dep = *(builditems[dep_name]);
	    if (dep.getTargetType() == TargetType::tt_all)
	    {
		candidate = false;
		break;
	    }
	    std::string platform_types =
		Util::join(" ", dep.getPlatformTypes());
	    if (first)
	    {
		dep_platform_types = platform_types;
		first = false;
	    }
	    else if (dep_platform_types == platform_types)
	    {
		QTC::TC("abuild", "Abuild-traverse non-trivial update platform types");
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-traverse platform type difference");
		candidate = false;
		break;
	    }
	}
	if (first || (! candidate))
	{
	    continue;
	}

	// All of this item's dependencies have the same list of
	// platform types as each other.  Inherit platform types and
	// target type from this item.
	QTC::TC("abuild", "Abuild-traverse inherit platform types");
	// First build item is known to exist and be valid
	assert(builditems.count(deps.front()));
	BuildItem& first_dep = *(builditems[deps.front()]);
	item.setPlatformTypes(first_dep.getPlatformTypes());
	item.setTargetType(first_dep.getTargetType());
    }
}

void
Abuild::checkDependencyPlatformTypes(BuildForest& forest)
{
    // Verify that each dependency that is declared with a specific
    // platform type is from an item whose target type is not tt_all
    // and to an item that has the given platform type.  Must be
    // called after updatePlatformTypes.

    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem const& item = *((*iter).second);
	FileLocation const& item_location = item.getLocation();
	bool has_any_dep_platform_types = false;
	std::list<std::string> const& deps = item.getDeps();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
	{
	    std::string const& dep_name = *dep_iter;
	    if (builditems.count(dep_name) == 0)
	    {
		// error reported elsewhere
		continue;
	    }
	    PlatformSelector const* ps = 0;
	    std::string dep_platform_type =
		item.getDepPlatformType(dep_name, ps);
	    if (dep_platform_type.empty())
	    {
		// no platform type declared for this dependency
		continue;
	    }
	    if (ps)
	    {
		dep_platform_type = ps->getPlatformType();
		// ItemConfig precludes dep_platform_type being ANY
		assert(dep_platform_type != PlatformSelector::ANY);
	    }
	    has_any_dep_platform_types = true;
	    // Ensure that the dependency has this platform type
	    BuildItem& dep = *(builditems[dep_name]);
	    if (dep_platform_type == PlatformData::pt_INDEP)
	    {
		QTC::TC("abuild", "Abuild-traverse ERR dep ptype indep");
		error(item_location, "dependencies may not be declared"
		      " with platform type \"" + dep_platform_type + "\"");
	    }
	    else if (dep.getPlatformTypes().count(dep_platform_type) == 0)
	    {
		QTC::TC("abuild", "Abuild-traverse ERR dep doesn't have dep ptype");
		error(item_location, "dependency \"" + dep_name + "\" declared"
		      " with platform type \"" + dep_platform_type + "\" "
		      ", which dependency does not have");
	    }
	}
	if (has_any_dep_platform_types &&
	    (item.getTargetType() == TargetType::tt_all))
	{
	    QTC::TC("abuild", "Abuild-traverse ERR all with ptype deps");
	    error(item_location, "this item has no platform types, so it may"
		  " not declare platform types on its dependencies");
	}
    }
}

void
Abuild::checkFlags(BuildForest& forest)
{
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem const& item = *((*iter).second);
	std::string const& item_name = (*iter).first;
	FileLocation const& item_location = item.getLocation();
	FlagData const& flag_data = item.getFlagData();
	std::set<std::string> dependencies = flag_data.getNames();
	for (std::set<std::string>::const_iterator diter =
		 dependencies.begin();
	     diter != dependencies.end(); ++diter)
	{
	    std::string const& dep_name = *diter;
	    if (builditems.count(dep_name) == 0)
	    {
		// error already reported
		QTC::TC("abuild", "Abuild-traverse skipping flag for unknown dep");
		continue;
	    }
	    BuildItem const& dep_item = *(builditems[dep_name]);
	    std::set<std::string> const& flags = flag_data.getFlags(dep_name);
	    for (std::set<std::string>::const_iterator fiter = flags.begin();
		 fiter != flags.end(); ++fiter)
	    {
		if (! dep_item.supportsFlag(*fiter))
		{
		    QTC::TC("abuild", "Abuild-traverse ERR unsupported flag");
		    error(item_location,
			  item_name + " may not specify flag " +
			  *fiter + " for dependency " + dep_name +
			  " because " + dep_name +
			  " does not support that flag");
		}
	    }
	}
    }
}

void
Abuild::checkTraits(BuildForest& forest)
{
    // For each build item, make sure each trait is allowed in the
    // current build tree and that any referent build items are
    // accessible.  Check only items that are local to the current
    // tree.

    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}
	std::set<std::string> const& supported_traits =
	    buildtrees[item.getTreeName()]->getSupportedTraits();
	FileLocation const& location = item.getLocation();
	TraitData::trait_data_t const& trait_data =
	    item.getTraitData().getTraitData();
	for (TraitData::trait_data_t::const_iterator titer = trait_data.begin();
	     titer != trait_data.end(); ++titer)
	{
	    std::string const& trait = (*titer).first;
	    if (supported_traits.count(trait) == 0)
	    {
		QTC::TC("abuild", "Abuild-traverse ERR unsupported trait");
		error(location, "trait " + trait +
		      " is not supported in this item's build tree");
	    }
	    std::set<std::string> const& items = (*titer).second;
	    for (std::set<std::string>::const_iterator iiter = items.begin();
		 iiter != items.end(); ++iiter)
	    {
		std::string const& referent_item = *iiter;
		if (! accessibleFrom(builditems, item_name, referent_item))
		{
		    QTC::TC("abuild", "Abuild-traverse ERR inaccessible trait referent");
		    error(location, "trait " + trait +
			  " refers to item " + referent_item +
			  " which is private to another scope");
		}
		if (builditems.count(referent_item) == 0)
		{
		    QTC::TC("abuild", "Abuild-traverse ERR invalid trait referent");
		    error(location, "trait " + trait +
			  " refers to item " + referent_item +
			  " which does not exist");
		}
		else if (! checkAllowedTreeItem(
			     forest, item, *builditems[referent_item],
			     "refer by trait to"))
		{
		    QTC::TC("abuild", "Abuild-traverse ERR invisible trait referent");
		}
	    }
	}
    }
}

void
Abuild::checkIntegrity(BuildForest_map& forests,
		       std::string const& top_path)
{
    // Check abuild's basic integrity guarantee.  Refer to the manual
    // for details.

    // Find items with shadowed references (dependencies or plugins).
    // An item has a shadowed reference if the local forest resolves
    // the item to a different path than the item's native forest.
    // Local items therefore can't have shadowed references.  (They
    // can still have references with shadowed references, so they
    // can still have integrity errors.)
    BuildForest& forest = *(forests[top_path]);
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (item.isLocal())
	{
	    continue;
	}

	std::string const& item_forest = item.getForestRoot();
	BuildItem_map& item_forest_items =
	    forests[item_forest]->getBuildItems();
	std::set<std::string> shadowed_references;

	std::set<std::string> const& refs = item.getReferences();
	for (std::set<std::string>::const_iterator ref_iter = refs.begin();
	     ref_iter != refs.end(); ++ref_iter)
        {
	    std::string const& ref_name = (*ref_iter);
	    if ((builditems.count(ref_name) == 0) ||
		(item_forest_items.count(ref_name) == 0))
	    {
		// unknown item already reported
		continue;
	    }

	    if (builditems[ref_name]->getAbsolutePath() !=
		item_forest_items[ref_name]->getAbsolutePath())
	    {
		// The instance of ref_name that item would see is
		// shadowed.
		shadowed_references.insert(ref_name);
	    }
        }

	item.setShadowedReferences(shadowed_references);
    }
}

void
Abuild::reportIntegrityErrors(BuildForest_map& forests,
			      BuildItem_map& builditems,
			      std::string const& top_path)
{
    std::set<std::string> integrity_error_items;

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	if (! item.getShadowedReferences().empty())
	{
	    integrity_error_items.insert(item_name);
	}
    }

    for (std::set<std::string>::const_iterator iter =
	     integrity_error_items.begin();
	 iter != integrity_error_items.end(); ++iter)
    {
	std::string const& item_name = *iter;
	BuildItem& item = *(builditems[item_name]);
	std::set<std::string> const& shadowed_references =
	    item.getShadowedReferences();
	std::list<std::string> const& plugin_list = item.getPlugins();
	std::set<std::string> plugins;
	plugins.insert(plugin_list.begin(), plugin_list.end());
	for (std::set<std::string>::const_iterator ref_iter =
		 shadowed_references.begin();
	     ref_iter != shadowed_references.end(); ++ref_iter)
	{
	    std::string const& ref_name = *ref_iter;
	    BuildItem& ref = *(builditems[ref_name]);
	    if (plugins.count(ref_name))
	    {
		QTC::TC("abuild", "Abuild-traverse ERR plugin integrity");
		error(item.getLocation(), "build item \"" + item_name +
		      "\" in tree \"" + item.getTreeName() +
		      "\" uses plugin \"" + ref_name +
		      "\", which is shadowed");
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-traverse ERR dep inconsistency");
		error(item.getLocation(), "build item \"" + item_name +
		      "\" depends on \"" + ref_name + "\", which is shadowed");
	    }
	    error(ref.getLocation(),
		  "here is the location of \"" + ref_name + "\"");
	}
    }
}

void
Abuild::computeBuildablePlatforms(BuildForest& forest)
{
    // For each build item, determine the list of platforms that are
    // to be built for that item.  Do this only for items that are
    // native to the local forest.
    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}

	BuildTree& item_tree = *(buildtrees[item.getTreeName()]);
	boost::shared_ptr<PlatformData> platform_data =
	    item_tree.getPlatformData();
	item.setPlatformData(platform_data);
	std::set<std::string> const& platform_types =
	    item.getPlatformTypes();
	std::set<std::string> build_platforms;
	for (std::set<std::string>::const_iterator i2 =
		 platform_types.begin();
	     i2 != platform_types.end(); ++i2)
        {
	    std::string const& platform_type = *i2;
	    if (platform_data->isPlatformTypeValid(platform_type))
	    {
		// Add all platforms for the platform type to the
		// buildable platforms list for the platform type.
		// Then add each one that is to build by default to
		// the build platforms list.  Additional build
		// platforms may be added later.
		PlatformData::selected_platforms_t const& platforms =
		    platform_data->getPlatformsByType(platform_type);
		std::vector<std::string> buildable_platforms;
		for (PlatformData::selected_platforms_t::const_iterator i3 =
			 platforms.begin();
		     i3 != platforms.end(); ++i3)
		{
		    std::string const& platform = (*i3).first;
		    bool selected = (*i3).second;
		    buildable_platforms.push_back(platform);
		    if (selected)
		    {
			build_platforms.insert(platform);
		    }
		}
		item.setBuildablePlatforms(platform_type, buildable_platforms);
	    }
        }
        item.setBuildPlatforms(build_platforms);
    }
}

void
Abuild::appendBackingData(std::string const& dir,
			  std::list<std::string>& backing_areas,
			  std::set<std::string>& deleted_trees,
			  std::set<std::string>& deleted_items)
{
    std::string file_backing = dir + "/" + BackingConfig::FILE_BACKING;
    if (Util::isFile(file_backing))
    {
	BackingConfig* backing = readBacking(dir);
	if (backing->isDeprecated())
	{
	    this->deprecated_backing_files.insert(file_backing);
	}
	std::list<std::string> const& dirs = backing->getBackingAreas();
	for (std::list<std::string>::const_iterator iter = dirs.begin();
	     iter != dirs.end(); ++iter)
	{
	    // The backing area must be a directory that contains an
	    // Abuild.conf (which is guaranteed by BackingConfig), and
	    // we must be able to find the root of the forest from
	    // there.  We detect this here when we have enough
	    // information to create a good error message.
	    std::string const& bdir = *iter;
	    std::string btop = findTop(
		bdir, "backing area \"" + bdir +
		"\" from \"" + file_backing + "\"");
	    if (btop.empty())
	    {
		QTC::TC("abuild", "Abuild-traverse ERR can't find backing top");
		error(backing->getLocation(),
		      "unable to determine top of forest containing"
		      " backing area \"" + bdir + "\"");
	    }
	    else
	    {
		backing_areas.push_back(bdir);
	    }
	}
	backing->appendBackingData(deleted_trees, deleted_items);
    }

    if (this->compat_level.allow_1_0())
    {
	if (Util::isFile(dir + "/" + ItemConfig::FILE_CONF))
	{
	    ItemConfig* config = readConfig(dir, "");
	    std::set<std::string> const& d = config->getDeleted();
	    deleted_items.insert(d.begin(), d.end());
	}
    }
}

BackingConfig*
Abuild::readBacking(std::string const& dir)
{
    BackingConfig* backing = BackingConfig::readBacking(
	this->error_handler, this->compat_level, dir);
    std::list<std::string> const& backing_areas = backing->getBackingAreas();
    if (backing_areas.empty())
    {
        QTC::TC("abuild", "Abuild-traverse ERR invalid backing file");
        error(FileLocation(dir + "/" + BackingConfig::FILE_BACKING, 0, 0),
	      "unable to get backing area data");
    }
    return backing;
}

void
Abuild::computeValidTraits(BuildForest_map& forests)
{
    for (BuildForest_map::iterator i1 = forests.begin();
	 i1 != forests.end(); ++i1)
    {
	BuildTree_map& buildtrees = ((*i1).second)->getBuildTrees();
	for (BuildTree_map::iterator iter = buildtrees.begin();
	     iter != buildtrees.end(); ++iter)
	{
	    BuildTree& tree = *((*iter).second);
	    std::set<std::string> const& traits = tree.getSupportedTraits();
	    this->valid_traits.insert(traits.begin(), traits.end());
	}
    }

    std::list<std::string> all_traits = this->only_with_traits;
    all_traits.insert(all_traits.end(),
		      this->related_by_traits.begin(),
		      this->related_by_traits.end());
    for (std::list<std::string>::iterator iter = all_traits.begin();
	 iter != all_traits.end(); ++iter)
    {
	if (this->valid_traits.count(*iter) == 0)
	{
	    QTC::TC("abuild", "Abuild-traverse ERR unknown trait");
	    error("trait " + *iter + " is unknown");
	}
    }
}

void
Abuild::listTraits()
{
    if (this->valid_traits.empty())
    {
	QTC::TC("abuild", "Abuild-traverse listTraits with no traits");
	std::cout << "No traits are supported." << std::endl;
    }
    else
    {
	QTC::TC("abuild", "Abuild-traverse listTraits");
	std::cout << "The following traits are supported:" << std::endl;
	for (std::set<std::string>::iterator iter = this->valid_traits.begin();
	     iter != this->valid_traits.end(); ++iter)
	{
	    std::cout << "  " << *iter << std::endl;
	}
    }
}

void
Abuild::listPlatforms(BuildForest_map& forests)
{
    for (BuildForest_map::iterator forest_iter = forests.begin();
	 forest_iter != forests.end(); ++forest_iter)
    {
	bool forest_root_output = false;
	std::string const& forest_root = (*forest_iter).first;
	BuildForest& forest = *((*forest_iter).second);
	BuildTree_map& buildtrees = forest.getBuildTrees();
	for (BuildTree_map::iterator tree_iter = buildtrees.begin();
	     tree_iter != buildtrees.end(); ++tree_iter)
	{
	    bool tree_name_output = false;
	    std::string const& tree_name = (*tree_iter).first;
	    BuildTree& tree = *((*tree_iter).second);
	    PlatformData& platform_data = *(tree.getPlatformData());
	    std::set<std::string> const& platform_types =
		platform_data.getPlatformTypes(TargetType::tt_object_code);
	    for (std::set<std::string>::const_iterator pt_iter =
		     platform_types.begin();
		 pt_iter != platform_types.end(); ++pt_iter)
	    {
		bool platform_type_output = false;
		std::string const& platform_type = *pt_iter;
		PlatformData::selected_platforms_t const& platforms =
		    platform_data.getPlatformsByType(platform_type);
		for (PlatformData::selected_platforms_t::const_iterator p_iter =
			 platforms.begin();
		     p_iter != platforms.end(); ++p_iter)
		{
		    if (! forest_root_output)
		    {
			std::cout << "forest " << forest_root << std::endl;
			forest_root_output = true;
		    }
		    if (! tree_name_output)
		    {
			std::cout << "  tree " << tree_name << std::endl;
			tree_name_output = true;
		    }
		    if (! platform_type_output)
		    {
			std::cout << "    platform type " << platform_type;
			std::string const& parent =
			    platform_data.getPlatformTypeParent(platform_type);
			if (! parent.empty())
			{
			    std::cout << "; parent " << parent;
			}
			std::cout << std::endl;
			platform_type_output = true;
		    }
		    std::string const& platform = (*p_iter).first;
		    bool selected = (*p_iter).second;
		    std::cout << "      platform " << platform
			      << "; built by default: "
			      << (selected ? "yes" : "no")
			      << std::endl;
		}
	    }
	}
    }
}

void
Abuild::computeTreePrefixes(std::list<std::string> const& tree_names)
{
    // Assign a prefix for each tree such that prefixes sort lexically
    // in the same order as trees sort topologically.  We use this for
    // creating build graph nodes in a way that improves the build
    // ordering when multiple trees are involved.  It's harmless to
    // have entries in this map for trees that don't actually
    // participate in the build.  Create zero-filled, fixed length,
    // numeric prefixes for each tree.

    unsigned int ndigits = Util::digitsIn(tree_names.size());
    int count = 0;
    for (std::list<std::string>::const_iterator iter = tree_names.begin();
	 iter != tree_names.end(); ++iter)
    {
	std::string const& tree_name = *iter;
	++count;
	this->buildgraph_tree_prefixes[tree_name] =
	    Util::intToString(count, ndigits);
    }
}

void
Abuild::computeItemPrefixes()
{
    // Assign a prefix for each item in the build graph.  These are
    // used as prefixes when capturing but interleaving output of
    // multithreaded builds.  They should be uniform length, but the
    // order isn't important.

    DependencyGraph::ItemList items = this->build_graph.getSortedGraph();
    unsigned int ndigits = Util::digitsIn(items.size());
    int count = 0;
    for (DependencyGraph::ItemList::iterator iter = items.begin();
	 iter != items.end(); ++iter)
    {
	this->buildgraph_item_prefixes[*iter] =
	    "[" + Util::intToString(++count, ndigits) + "]";
    }
}
