#include <BuildForest.hh>

BuildForest::BuildForest()
{
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
BuildForest::setSortedItemNames(std::list<std::string> const& sorted_items)
{
    this->sorted_item_names = sorted_items;
}

std::list<std::string> const&
BuildForest::getSortedItemNames() const
{
    return this->sorted_item_names;
}
