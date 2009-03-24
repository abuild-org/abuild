#ifndef __BUILDFOREST_HH__
#define __BUILDFOREST_HH__

#include <BuildItem.hh>
#include <BuildTree.hh>
#include <boost/shared_ptr.hpp>
#include <string>
#include <list>
#include <set>
#include <map>

class BuildForest
{
  public:
    BuildForest(std::list<std::string> const& backing_areas,
		std::set<std::string> const& deleted_trees,
		std::set<std::string> const& deleted_items);

    typedef boost::shared_ptr<BuildItem> BuildItem_ptr;
    typedef std::map<std::string, BuildItem_ptr> BuildItem_map;
    typedef boost::shared_ptr<BuildTree> BuildTree_ptr;
    typedef std::map<std::string, BuildTree_ptr> BuildTree_map;

    void setBuildItems(BuildItem_map const& items);
    void setSortedItemNames(std::list<std::string> const& sorted_items);
    void setBuildTrees(BuildTree_map const& trees);

    std::list<std::string> const& getBackingAreas() const;
    BuildItem_map& getBuildItems();
    BuildTree_map& getBuildTrees();
    std::list<std::string> const& getSortedItemNames() const;
    std::set<std::string> const& getDeletedTrees() const;
    std::set<std::string> const& getDeletedItems() const;

  private:
    // absolute paths of backing area roots
    std::list<std::string> backing_areas;
    // build item name -> BuildItem
    BuildItem_map build_items;
    // build tree name -> BuildTree
    BuildTree_map build_trees;
    // sorted list of build item names
    std::list<std::string> sorted_item_names;
    std::set<std::string> deleted_trees;
    std::set<std::string> deleted_items;
};

#endif // __BUILDFOREST_HH__
