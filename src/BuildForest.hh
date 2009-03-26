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
    BuildForest(std::string const& root_path);

    typedef boost::shared_ptr<BuildItem> BuildItem_ptr;
    typedef std::map<std::string, BuildItem_ptr> BuildItem_map;
    typedef boost::shared_ptr<BuildTree> BuildTree_ptr;
    typedef std::map<std::string, BuildTree_ptr> BuildTree_map;

    std::string const& getRootPath() const;

    BuildItem_map& getBuildItems();
    BuildTree_map& getBuildTrees();
    std::list<std::string>& getBackingAreas();
    std::set<std::string>& getDeletedTrees();
    std::set<std::string>& getDeletedItems();

    std::map<std::string, std::set<std::string> >& getTreeAccessTable();
    void setSortedTreeNames(std::list<std::string> const& sorted_trees);
    std::list<std::string> const& getSortedTreeNames() const;
    void setSortedItemNames(std::list<std::string> const& sorted_items);
    std::list<std::string> const& getSortedItemNames() const;

  private:
    BuildForest(BuildForest const&);
    BuildForest& operator=(BuildForest const&);

    std::string root_path;

    BuildItem_map build_items;
    BuildTree_map build_trees;
    std::list<std::string> backing_areas;
    std::set<std::string> deleted_trees;
    std::set<std::string> deleted_items;

    std::map<std::string, std::set<std::string> > tree_access_table;
    std::list<std::string> sorted_tree_names;
    std::list<std::string> sorted_item_names;
};

#endif // __BUILDFOREST_HH__
