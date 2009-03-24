#ifndef __BUILDTREE_HH__
#define __BUILDTREE_HH__

#include <string>
#include <list>
#include <set>
#include <PlatformData.hh>

class BuildTree
{
  public:
    BuildTree(std::list<std::string> const& tree_deps,
	      std::set<std::string> const& declared_traits,
	      std::list<std::string> const& plugins,
	      PlatformData const& platform_data);

    void addTraits(std::set<std::string> const& traits);
    // Return writable reference to platform data so it can be modified
    PlatformData& getPlatformData();

    std::list<std::string> const& getTreeDeps() const;
    std::set<std::string> const& getSupportedTraits() const;
    std::list<std::string> const& getPlugins() const;

  private:
    std::list<std::string> tree_deps;
    std::set<std::string> supported_traits;
    std::list<std::string> plugins;
    PlatformData platform_data;
};

#endif // __BUILDTREE_HH__
