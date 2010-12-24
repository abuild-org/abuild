// Construction of build set

#include <Abuild.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

void
Abuild::computeBuildset(BuildTree_map& buildtrees, BuildItem_map& builditems)
{
    // Generate the build set.
    std::string const& this_name = this->this_config->getName();
    BuildTree_ptr this_buildtree;
    if (! this->local_tree.empty())
    {
        if (buildtrees.count(this->local_tree) == 0)
        {
            fatal("INTERNAL ERROR: the current build tree is not known");
        }
        this_buildtree = buildtrees[this->local_tree];
    }
    BuildItem_ptr this_builditem;
    if (! this_name.empty())
    {
        if (builditems.count(this_name) == 0)
        {
            // Can't happen without some other error such as
            // parent/child consistency
            fatal("INTERNAL ERROR: the current build item is not known");
        }
        this_builditem = builditems[this_name];
    }

    std::string set_name = this->buildset_name;
    if (set_name.empty())
    {
	set_name = this->cleanset_name;
    }

    bool cleaning = false;
    if (! set_name.empty())
    {
	cleaning = (! this->cleanset_name.empty());

	if (this->local_tree.empty() &&
	    ((set_name == b_DEPTREES) ||
	     (set_name == b_LOCAL) ||
	     (set_name == b_DESCDEPTREES)))
	{
	    QTC::TC("abuild", "Abuild-buildset ERR bad tree-based build set");
	    error("build set \"" + set_name + "\" is invalid when"
		  " the current build item is not part of any tree");
	}
	else if (! this->buildset_named_items.empty())
	{
	    std::set<std::string> named_items =
		this->buildset_named_items;
            QTC::TC("abuild", "Abuild-buildset buildset names",
		    cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isNamed, _1,
					 boost::ref(named_items)));
	    if (! named_items.empty())
	    {
		std::string unknown_items = Util::join(", ", named_items);
		QTC::TC("abuild", "Abuild-buildset ERR unknown items in set");
		error("unable to add unknown build items to build set: " +
		      unknown_items);
	    }
	}
	else if (! this->buildset_pattern.empty())
	{
	    boost::regex pattern(this->buildset_pattern);
	    QTC::TC("abuild", "Abuild-buildset buildset pattern", cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::matchesPattern, _1,
					 pattern));
	}
	else if (set_name == b_ALL)
        {
            QTC::TC("abuild", "Abuild-buildset buildset all",
		    cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isLocal, _1));
        }
	else if (set_name == b_DEPTREES)
        {
            QTC::TC("abuild", "Abuild-buildset buildset deptrees",
		    cleaning ? 1 : 0);
	    std::set<std::string> trees;
	    std::list<std::string> const& deptrees =
		this_buildtree->getExpandedTreeDeps();
	    trees.insert(deptrees.begin(), deptrees.end());
	    trees.insert(this->local_tree);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isInTrees, _1, trees));
        }
	else if (set_name == b_DESCDEPTREES)
        {
            QTC::TC("abuild", "Abuild-buildset buildset descdeptrees",
		    cleaning ? 1 : 0);
	    std::set<std::string> trees;
	    std::list<std::string> const& deptrees =
		this_buildtree->getExpandedTreeDeps();
	    trees.insert(deptrees.begin(), deptrees.end());
	    trees.insert(this->local_tree);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isInTreesAndAtOrBelowPath,
					 _1, trees, this->current_directory));
        }
        else if (set_name == b_LOCAL)
        {
            QTC::TC("abuild", "Abuild-buildset buildset local",
		    cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isInTree, _1,
					 this->local_tree));
        }
        else if (set_name == b_DESC)
        {
            QTC::TC("abuild", "Abuild-buildset buildset desc",
		    cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isAtOrBelowPath,
					 _1, this->current_directory));
        }
        else if (set_name == b_DEPS)
        {
            if (this_builditem.get())
            {
                QTC::TC("abuild", "Abuild-buildset buildset deps",
			cleaning ? 1 : 0);
		std::list<std::string> const& deps =
		    this_builditem->getExpandedDependencies();
		for (std::list<std::string>::const_iterator iter =
			 deps.begin();
		     iter != deps.end(); ++iter)
		{
		    std::string const& dep = *iter;
		    this->buildset[dep] = builditems[dep];
		}
            }
        }
        else if (set_name == b_CURRENT)
        {
            if (this_builditem.get())
            {
                QTC::TC("abuild", "Abuild-buildset buildset current",
			cleaning ? 1 : 0);
		this->buildset[this_name] = this_builditem;
            }
        }
        else
        {
            fatal("INTERNAL ERROR: unknown build/clean set " + set_name);
        }
    }
    else if (this_builditem.get())
    {
	cleaning = (this->special_target == s_CLEAN);
        this->buildset[this_name] = this_builditem;
    }

    if (addBuildAlsoToBuildset(builditems))
    {
	QTC::TC("abuild", "Abuild-buildset non-trivial build-also");
    }

    if ((! this->apply_targets_to_deps) &&
	(this->related_by_traits.empty()))
    {
	// Only build items that were initially selected for
	// membership in the build set get explicit targets.  Add them
	// now before we expand the build set to include dependencies.
	// If we have any related by traits, we'll add only those
	// items later.
	for (BuildItem_map::iterator iter = this->buildset.begin();
	     iter != this->buildset.end(); ++iter)
	{
	    QTC::TC("abuild", "Abuild-buildset add selected to explicit targets");
	    this->explicit_target_items.insert((*iter).first);
	}
    }

    // We always add dependencies of any items initially in the build
    // set to the build set if we are building.  If we are cleaning,
    // we only do this when the --apply-targets-to-deps or
    // --with-rdeps options are specified.
    bool add_dependencies =
	(! cleaning) || this->apply_targets_to_deps || this->with_rdeps;

    // Expand the build set to include dependencies of all items in
    // the build set.  If we are also expanding because of expand
    // traits, then after expanding based on dependencies, expand
    // based on traits.  Then expand one more time based on
    // dependencies so that anything added because of traits gets its
    // dependencies added as well.  Note that we do not repeat the
    // process again, so if anything is related to one of the last
    // group of dependency-based expansions by an expand trait, it
    // doesn't get added to the set.  (If the build set contains A,
    // the expand set contains trait tester, A-tester tests A, and
    // A-testers depends on X, then A, A-tester, and X all get
    // added.  If X is tested by X-tester, X-tester does not get
    // added.)
    bool expanding = true;
    bool first_pass = true;
    while (expanding)
    {
	expanding = false;

	if (add_dependencies)
	{
	    std::set<std::string> to_add;
	    for (BuildItem_map::iterator iter = this->buildset.begin();
		 iter != this->buildset.end(); ++iter)
	    {
		BuildItem& item = *((*iter).second);
		std::list<std::string> const& alldeps =
		    item.getExpandedDependencies();
		for (std::list<std::string>::const_iterator dep_iter =
			 alldeps.begin();
		     dep_iter != alldeps.end(); ++dep_iter)
		{
		    to_add.insert(*dep_iter);
		}
	    }
	    for (std::set<std::string>::iterator iter = to_add.begin();
		 iter != to_add.end(); ++iter)
	    {
		if (this->buildset.count(*iter) == 0)
		{
		    QTC::TC("abuild", "Abuild-buildset non-trivial dep expansion");
		    expanding = true;
		    this->buildset[*iter] = builditems[*iter];
		}
	    }
	}

	if (addBuildAlsoToBuildset(builditems))
	{
	    QTC::TC("abuild", "Abuild-buildset additional build-also expansion");
	    expanding = true;
	}

	if (! (this->repeat_expansion || first_pass))
	{
	    continue;
	}

	if (this->with_rdeps)
	{
	    bool adding_rdeps = true;
	    while (adding_rdeps)
	    {
		adding_rdeps = false;
		QTC::TC("abuild", "Abuild-buildset expand by rdeps");

		// Add to the build set any item that has a dependency on
		// any item already in the build set.
		std::set<std::string> to_add;
		// For each item, ...
		for (BuildItem_map::iterator iter = builditems.begin();
		     iter != builditems.end(); ++iter)
		{
		    std::string const& item_name = (*iter).first;
		    BuildItem& item = *((*iter).second);
		    // get the list of dependencies. ...
		    std::list<std::string> const& deps =
			item.getDeps();
		    // For each dependency, ...
		    for (std::list<std::string>::const_iterator diter =
			     deps.begin();
			 diter != deps.end(); ++diter)
		    {
			// see if the dependency is already in the build
			// set.  If so, add the item.
			if (this->buildset.count(*diter))
			{
			    to_add.insert(item_name);
			}
		    }
		}
		for (std::set<std::string>::iterator iter = to_add.begin();
		     iter != to_add.end(); ++iter)
		{
		    if (this->buildset.count(*iter) == 0)
		    {
			QTC::TC("abuild", "Abuild-buildset non-trivial rdep expansion");
			expanding = true;
			adding_rdeps = true;
			this->buildset[*iter] = builditems[*iter];
		    }
		}
	    }
	}

	if (! this->related_by_traits.empty())
	{
	    QTC::TC("abuild", "Abuild-buildset expand by trait",
		    cleaning ? 1 : 0);

	    // Add to the build set any item that has all the traits
	    // named in the related by traits in reference any item
	    // already in the build set.
	    std::set<std::string> to_add;
	    // For each item, ...
	    for (BuildItem_map::iterator iter = builditems.begin();
		 iter != builditems.end(); ++iter)
	    {
		std::string const& item_name = (*iter).first;
		BuildItem& item = *((*iter).second);
		// get the list of traits. ...
		TraitData::trait_data_t const& trait_data =
		    item.getTraitData().getTraitData();
		// For each related by trait, ...
		bool missing_any_traits = false;
		for (std::list<std::string>::iterator titer =
			 this->related_by_traits.begin();
		     titer != this->related_by_traits.end(); ++titer)
		{
		    // see if the item has the trait.  If so, ...
		    if (trait_data.count(*titer) != 0)
		    {
			bool refers_to_item_in_buildset = false;
			// see if any of the referent items belong to
			// the build set, and add them if they do.
			std::set<std::string> const& referent_items =
			    (*(trait_data.find(*titer))).second;
			for (std::set<std::string>::const_iterator riter =
				 referent_items.begin();
			     riter != referent_items.end(); ++riter)
			{
			    if (this->buildset.count(*riter))
			    {
				refers_to_item_in_buildset = true;
				break;
			    }
			}
			if (! refers_to_item_in_buildset)
			{
			    missing_any_traits = true;
			}
		    }
		    else
		    {
			missing_any_traits = true;
		    }
		    if (missing_any_traits)
		    {
			break;
		    }
		}
		if (! missing_any_traits)
		{
		    to_add.insert(item_name);
		}
	    }
	    for (std::set<std::string>::iterator iter = to_add.begin();
		 iter != to_add.end(); ++iter)
	    {
		// Add the item to the build set, and also add it to
		// the list of items that are built with explicit
		// targets.
		if (this->buildset.count(*iter) == 0)
		{
		    QTC::TC("abuild", "Abuild-buildset non-trivial trait expansion");
		    expanding = true;
		    this->buildset[*iter] = builditems[*iter];
		}
		if (! this->apply_targets_to_deps)
		{
		    this->explicit_target_items.insert(*iter);
		}
	    }
	}

	first_pass = false;
    }

    // Expand the build set to include all plugins of all items in the
    // buildset.
    if (add_dependencies)
    {
	std::set<std::string> to_add;
	for (BuildItem_map::iterator iter = this->buildset.begin();
	     iter != this->buildset.end(); ++iter)
	{
	    BuildItem& item = *((*iter).second);
	    std::list<std::string> const& plugins = item.getPlugins();
	    for (std::list<std::string>::const_iterator p_iter =
		     plugins.begin();
		 p_iter != plugins.end(); ++p_iter)
	    {
		to_add.insert(*p_iter);
	    }
	}
	for (std::set<std::string>::iterator iter = to_add.begin();
	     iter != to_add.end(); ++iter)
	{
	    this->buildset[*iter] = builditems[*iter];
	}
    }

    if (this->buildset.empty())
    {
        QTC::TC("abuild", "Abuild-buildset empty buildset");
    }

    if (this->apply_targets_to_deps)
    {
	// If all items get explicit targets, add all items now that
	// we've completed construction of the build set.
	QTC::TC("abuild", "Abuild-buildset add all to explicit targets",
		cleaning ? 0 : 1);
	for (BuildItem_map::iterator iter = this->buildset.begin();
	     iter != this->buildset.end(); ++iter)
	{
	    this->explicit_target_items.insert((*iter).first);
	}
    }
}

void
Abuild::populateBuildset(BuildItem_map& builditems,
			 boost::function<bool(BuildItem const*)> pred)
{
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem_ptr item_ptr = (*iter).second;
	// We exclude backed build items from initial population of
	// build set, though they will be added, if needed, to satisfy
	// dependencies.  This helps to reduce extraneous integrity
	// errors when not running in full integrity mode.
	if (pred(item_ptr.get()) && item_ptr->isLocal() &&
	    item_ptr->hasTraits(this->only_with_traits))
	{
	    this->buildset[item_name] = item_ptr;
	}
    }
}

bool
Abuild::addBuildAlsoToBuildset(BuildItem_map& builditems)
{
    // For every item in the build set, if that item specifies any
    // other items to build, add them as well.
    bool adding = true;
    bool added_any = false;
    while (adding)
    {
	adding = false;
	std::set<std::string> to_add;
	for (BuildItem_map::iterator iter = this->buildset.begin();
	     iter != this->buildset.end(); ++iter)
	{
	    BuildItem& item = *((*iter).second);
	    std::list<std::string> const& build_also = item.getBuildAlso();
	    for (std::list<std::string>::const_iterator biter =
		     build_also.begin();
		 biter != build_also.end(); ++biter)
	    {
		to_add.insert(*biter);
	    }
	}
	for (std::set<std::string>::iterator iter = to_add.begin();
	     iter != to_add.end(); ++iter)
	{
	    if ((this->buildset.count(*iter) == 0) &&
		builditems[*iter]->hasTraits(this->only_with_traits))
	    {
		adding = true;
		this->buildset[*iter] = builditems[*iter];
	    }
	}
	if (adding)
	{
	    added_any = true;
	}
    }
    return added_any;
}
