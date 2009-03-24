#include <BuildForest.hh>

BuildForest::BuildForest(std::list<std::string> const& backing_areas,
			 std::set<std::string> const& deleted_trees,
			 std::set<std::string> const& deleted_items) :
    backing_areas(backing_areas),
    deleted_trees(deleted_trees),
    deleted_items(deleted_items)
{
}

void
BuildForest::setBuildItems(BuildItem_map const& items)
{
    this->build_items = items;
}

void
BuildForest::setSortedItemNames(std::list<std::string> const& sorted_items)
{
    this->sorted_item_names = sorted_items;
}

void
BuildForest::setBuildTrees(BuildTree_map const& trees)
{
    this->build_trees = trees;
}

std::list<std::string> const&
BuildForest::getBackingAreas() const
{
    return this->backing_areas;
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

std::list<std::string> const&
BuildForest::getSortedItemNames() const
{
    return this->sorted_item_names;
}

std::set<std::string> const&
BuildForest::getDeletedTrees() const
{
    return this->deleted_trees;
}

std::set<std::string> const&
BuildForest::getDeletedItems() const
{
    return this->deleted_items;
}
