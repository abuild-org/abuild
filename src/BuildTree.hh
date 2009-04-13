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
    BuildTree(std::string const& name,
	      std::string const& root_path,
	      std::list<std::string> const& tree_deps,
	      std::set<std::string> const& optional_tree_deps,
	      std::set<std::string> const& declared_traits,
	      std::list<std::string> const& plugins,
	      PlatformData const& platform_data);

    void addTraits(std::set<std::string> const& traits);
    // Return writable reference to platform data so it can be modified
    PlatformData& getPlatformData();

    std::string const& getName() const;
    std::string const& getRootPath() const;
    FileLocation const& getLocation() const;
    std::list<std::string> const& getTreeDeps() const;
    std::set<std::string> const& getSupportedTraits() const;
    std::list<std::string> const& getPlugins() const;
    std::list<std::string> const& getExpandedTreeDeps() const;
    std::set<std::string> const& getOptionalTreeDeps() const;
    std::set<std::string> const& getOmittedTreeDeps() const;

    void setForestRoot(std::string const&);
    std::string const& getForestRoot() const;
    int getBackingDepth() const;
    bool isLocal() const;
    void incrementBackingDepth();
    void setExpandedTreeDeps(std::list<std::string> const&);
    void removeTreeDep(std::string const& item);

    void addTreeDeps(std::set<std::string> const& extra_tree_deps);
    void addPlugins(std::set<std::string> const& extra_plugins);

  private:
    std::string name;
    std::string root_path;
    std::string forest_root;
    FileLocation location;
    std::list<std::string> tree_deps;
    std::list<std::string> expanded_tree_deps;
    std::set<std::string> optional_tree_deps;
    std::set<std::string> omitted_tree_deps;
    std::set<std::string> supported_traits;
    std::list<std::string> plugins;
    PlatformData platform_data;
    int backing_depth;
};

#endif // __BUILDTREE_HH__
