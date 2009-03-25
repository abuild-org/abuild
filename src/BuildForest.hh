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
    BuildForest();

    typedef boost::shared_ptr<BuildItem> BuildItem_ptr;
    typedef std::map<std::string, BuildItem_ptr> BuildItem_map;
    typedef boost::shared_ptr<BuildTree> BuildTree_ptr;
    typedef std::map<std::string, BuildTree_ptr> BuildTree_map;

    BuildItem_map& getBuildItems();
    BuildTree_map& getBuildTrees();
    std::list<std::string>& getBackingAreas();
    std::set<std::string>& getDeletedTrees();
    std::set<std::string>& getDeletedItems();

    void setSortedItemNames(std::list<std::string> const& sorted_items);
    std::list<std::string> const& getSortedItemNames() const;

  private:
    BuildForest(BuildForest const&);
    BuildForest& operator=(BuildForest const&);

    BuildItem_map build_items;
    BuildTree_map build_trees;
    std::list<std::string> backing_areas;
    std::set<std::string> deleted_trees;
    std::set<std::string> deleted_items;

    std::list<std::string> sorted_item_names;
};

#endif // __BUILDFOREST_HH__
