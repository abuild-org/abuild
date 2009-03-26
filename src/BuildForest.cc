#include <BuildForest.hh>

BuildForest::BuildForest(std::string const& root_path) :
    root_path(root_path)
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
