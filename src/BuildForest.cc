#include <BuildForest.hh>

BuildForest::BuildForest(std::string const& root_path) :
    root_path(root_path),
    has_externals(false)
{
}

std::string const&
BuildForest::getRootPath() const
{
    return this->root_path;
}

BuildForest::BuildItem_map&
BuildForest::getBuildItems()
{
    return this->build_items;
}

BuildForest::BuildTree_map&
BuildForest::getBuildTrees()
{
    return this->build_trees;
}

std::list<std::string>&
BuildForest::getBackingAreas()
{
    return this->backing_areas;
}


std::set<std::string>&
BuildForest::getDeletedTrees()
{
    return this->deleted_trees;
}

std::set<std::string>&
BuildForest::getDeletedItems()
{
    return this->deleted_items;
}

void
BuildForest::setHasExternals()
{
    this->has_externals = true;
}

bool
BuildForest::hasExternals() const
{
    return this->has_externals;
}

std::map<std::string, std::set<std::string> >&
BuildForest::getTreeAccessTable()
{
    return this->tree_access_table;
}

void
BuildForest::setSortedTreeNames(std::list<std::string> const& sorted_trees)
{
    this->sorted_tree_names = sorted_trees;
}

std::list<std::string> const&
BuildForest::getSortedTreeNames() const
{
    return this->sorted_tree_names;
}

void
BuildForest::setSortedItemNames(std::list<std::string> const& sorted_items)
{
    this->sorted_item_names = sorted_items;
}

std::list<std::string> const&
BuildForest::getSortedItemNames() const
{
    return this->sorted_item_names;
}

void
BuildForest::addGlobalPlugin(std::string const& item_name)
{
    this->global_plugins.insert(item_name);
}

std::set<std::string>&
BuildForest::getGlobalPlugins()
{
    return this->global_plugins;
}

void
BuildForest::propagateGlobals()
{
    for (BuildTree_map::iterator iter = this->build_trees.begin();
	 iter != this->build_trees.end(); ++iter)
    {
	BuildTree& tree = *((*iter).second);
	tree.addPlugins(this->global_plugins);
    }
}
