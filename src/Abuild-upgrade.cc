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
    ud.scan();
    if (! ud.upgradeRequired())
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
Abuild::constructTreeGraph(UpgradeData& ud, DependencyGraph& g)
{
    // XXX This code is broken for build trees with only backing and
    // not conf.

    std::map<std::string, bool> const& item_dirs = ud.getItemDirs();

    // XXX currently handles only 1.0 logic...

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

	bool bad_backing = false;
	std::list<std::string> backing_chain = getBackingChain(dir);
	for (std::list<std::string>::iterator biter = backing_chain.begin();
	     biter != backing_chain.end(); ++biter)
	{
	    std::string rel_backing = Util::absToRel(*biter);
	    if (! ((rel_backing == "..") ||
		   ((rel_backing.length() > 2) &&
		    (rel_backing.substr(0, 3) == "../"))))
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
	    if (! Util::isFile(epath + "/" + ItemConfig::FILE_CONF))
	    {
		// See if we can find the directory in the backing
		// area chain.
		std::string resolved;
		for (std::list<std::string>::iterator biter =
			 backing_chain.begin();
		     biter != backing_chain.end(); ++biter)
		{
		    std::string candidate = *biter + "/" + edecl;
		    if (Util::isFile(candidate + "/" + ItemConfig::FILE_CONF))
		    {
			resolved = candidate;
			break;
		    }
		}
		if (resolved.empty())
		{
		    QTC::TC("abuild", "Abuild-upgrade ERR external not found");
		    error(location,
			  "external " + edecl + " (" + epath +
			  " relative to current directory)"
			  " does not exist or is not a directory and can't"
			  " be found in the backing area chain");
		}
		else
		{
		    /*ItemConfig* econfig =*/ readConfig(resolved, "");
		    // XXX if econfig has a tree name, use it.
		    // Otherwise, give a warning that the external
		    // resolves to a backing area has not been
		    // upgraded yet, and add it to the do-not-upgrade
		    // list.  Create coverage cases for both.
		}
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
		    error(location,
			  "XXX external " + edecl + " (" + epath +
			  " relative to current directory) is unknown");
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
