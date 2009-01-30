#ifndef __BUILDTREE_HH__
#define __BUILDTREE_HH__

#include <string>
#include <list>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include <BuildItem.hh>
#include <PlatformData.hh>

class BuildTree
{
  public:
    BuildTree(std::string const& backing_area,
	      std::map<std::string, ExternalData> const& externals,
	      std::map<std::string, ExternalData> const& backed_externals,
	      std::set<std::string> const& declared_traits,
	      std::set<std::string> const& deleted_items,
	      std::list<std::string> const& plugins,
	      PlatformData const& platform_data);

    typedef boost::shared_ptr<BuildItem> BuildItem_ptr;
    typedef std::map<std::string, BuildItem_ptr> BuildItem_map;

    void setBuildItems(BuildItem_map const& items);
    void setSortedItemNames(std::list<std::string> const& sorted_items);
    void addTraits(std::set<std::string> const& traits);
    // Return writable reference to platform data so it can be modified
    PlatformData& getPlatformData();

    std::string const& getBackingArea() const;
    std::map<std::string, ExternalData> const& getExternals() const;
    std::map<std::string, ExternalData> const& getBackedExternals() const;
    BuildItem_map& getBuildItems();
    std::list<std::string> const& getSortedItemNames() const;
    std::set<std::string> const& getSupportedTraits() const;
    std::set<std::string> const& getDeletedItems() const;
    std::list<std::string> const& getPlugins() const;

  private:
    // absolute path of backing area root
    std::string backing_area;
    // relative external path -> absolute path to external root
    std::map<std::string, ExternalData> externals;
    // externals resolved through a backing areas
    std::map<std::string, ExternalData> backed_externals;
    // build item name -> BuildItem
    BuildItem_map build_items;
    // sorted list of build item names
    std::list<std::string> sorted_item_names;
    // traits supported by this tree and its externals
    std::set<std::string> supported_traits;
    std::set<std::string> deleted_items;
    std::list<std::string> plugins;
    PlatformData platform_data;
};

#endif // __BUILDTREE_HH__
