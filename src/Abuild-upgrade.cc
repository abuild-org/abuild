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

    // XXX remember to make sure that everything in do_not_upgrade
    // appears in external-dirs but is not a key in item_dirs

    // XXX remember to validate tree names and to make sure they are
    // unique within a forest

    // XXX check all upgrade conditions

    // XXX Only write if not doing an upgrade
    info("writing upgrade data file...");
    ud.writeUpgradeData(forests);

    if (this->error_handler.anyErrors())
    {
	info("upgrade data has been written to " +
	     UpgradeData::FILE_UPGRADE_DATA);
	info("please edit that file and rerun; for details, see"
	     " \"Upgrading Build Trees from 1.0 to 1.1\""
	     " in the user's manual");
    }

    exitIfErrors();

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

	ItemConfig* config = readPossiblyBackedConfig(dir);
	if (config)
	{
	    ud.item_dirs[dir] = config->isTreeRoot();
	    if (config->usesDeprecatedFeatures())
	    {
		ud.upgrade_required = true;
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
    // XXX currently handles only 1.0 logic...

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

	bool bad_backing = false;
	std::list<std::string> backing_chain = getBackingChain(dir);
	for (std::list<std::string>::iterator biter = backing_chain.begin();
	     biter != backing_chain.end(); ++biter)
	{
	    std::string rel_backing = Util::absToRel(*biter);
	    if (Util::isDirUnder(*biter))
	    {
		QTC::TC("abuild", "Abuild-upgrade ERR local backing chain");
		error(FileLocation(dir + "/" + BackingFile::FILE_BACKING, 0, 0),
		      "Part of this item's backing area chain falls under"
		      " the current directory.  You must either rerun " +
		      this->whoami + " from a lower directory or exclude" +
		      " the backing area.  Ignoring the backing chain;"
		      " expect spurious errors");
		bad_backing = true;
		break;
	    }
	}
	if (bad_backing)
	{
	    backing_chain.clear();
	}

	std::list<ExternalData> const& externals = config->getExternals();
	for (std::list<ExternalData>::const_iterator eiter = externals.begin();
	     eiter != externals.end(); ++eiter)
	{
	    std::string edecl = (*eiter).getDeclaredPath();
	    std::string epath = Util::absToRel(
		Util::canonicalizePath(dir + "/" + edecl));

	    // XXX This logic is still wrong for externals that point
	    // into pruned areas.

	    if (ud.item_dirs.count(epath))
	    {
		if (! ud.item_dirs[epath])
		{
		    QTC::TC("abuild", "Abuild-upgrade ERR external not root");
		    error(location,
			  "external " + edecl + " (" + epath +
			  " relative to current directory) is not"
			  " a build tree root");
		}
		else
		{
		    // XXX good...get tree name, if any
		    g.addDependency(dir, epath);
		}
	    }
	    else
	    {
		ItemConfig* econfig = readPossiblyBackedConfig(dir, edecl);
		if (econfig)
		{
		    // XXX We can't really tell whether this points
		    // into a pruned area or came from a backing area.
		    // readPossiblyBackedConfig needs to provide some
		    // information.
		    info("XXX resolved " + dir + "//" + edecl +
			 " through a backing area");
		    if (Util::isDirUnder(Util::canonicalizePath(epath)))
		    {
			QTC::TC("abuild", "Abuild-upgrade resolved local ext");
			info("XXX declared location is under start dir");
			// XXX get tree name.  Right now, this ends up
			// in orphan-trees, which is not where it
			// should be.
			ud.tree_names[epath] = "***"; // XXX hard-coded ***
		    }
		    else
		    {
			QTC::TC("abuild", "Abuild-upgrade non-local ext");
			// XXX Here we probably want to preserve this
			// external as an external-dirs.  This could
			// happen if the external points up above the
			// top of the start directory or is in a
			// pruned area.
		    }
		}
		else
		{
		    error(location,
			  "external " + edecl + " does not exist and cannot"
			  " be resolved through a backing area");
		}
	    }
	}
    }

    if (! g.check())
    {
	// XXX report errors
	fatal("errors found checking tree-level dependencies");
    }
}
