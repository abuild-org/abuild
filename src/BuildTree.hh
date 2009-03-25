#ifndef __BUILDTREE_HH__
#define __BUILDTREE_HH__

#include <string>
#include <list>
#include <set>
#include <PlatformData.hh>
#include <FileLocation.hh>

class BuildTree
{
  public:
    BuildTree(std::string const& root_path,
	      std::list<std::string> const& tree_deps,
	      std::set<std::string> const& declared_traits,
	      std::list<std::string> const& plugins,
	      PlatformData const& platform_data);

    void addTraits(std::set<std::string> const& traits);
    // Return writable reference to platform data so it can be modified
    PlatformData& getPlatformData();

    std::string const& getRootPath() const;
    FileLocation const& getLocation() const;
    std::list<std::string> const& getTreeDeps() const;
    std::set<std::string> const& getSupportedTraits() const;
    std::list<std::string> const& getPlugins() const;
    std::list<std::string> const& getExpandedTreeDeps() const;

    void setExpandedTreeDeps(std::list<std::string> const&);

  private:
    std::string root_path;
    FileLocation location;
    std::list<std::string> tree_deps;
    std::list<std::string> expanded_tree_deps;
    std::set<std::string> supported_traits;
    std::list<std::string> plugins;
    PlatformData platform_data;
};

#endif // __BUILDTREE_HH__
