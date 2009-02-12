#ifndef __BUILDITEM_HH__
#define __BUILDITEM_HH__

#include <string>
#include <list>
#include <set>
#include <boost/shared_ptr.hpp>
#include <TargetType.hh>
#include <Interface.hh>
#include <ItemConfig.hh>
#include <boost/regex.hpp>

class TraitData;

class BuildItem
{
  public:
    BuildItem(std::string const& item_name,
	      ItemConfig const* config,
	      std::string const& absolute_path,
	      std::string const& tree_top);

    // Proxy methods to ItemConfig
    std::string const& getName() const;
    std::string const& getDescription() const;
    std::string const& getParent() const;
    std::list<std::string> const& getChildren() const;
    std::list<ExternalData> const& getExternals() const;
    std::list<std::string> const& getBuildAlso() const;
    std::list<std::string> const& getRuleBuildItems() const;
    std::list<std::string> const& getDeps() const;
    std::string const& getDepPlatformType(std::string const&) const;
    std::string const& getDepPlatformType(
	std::string const&, PlatformSelector const*&) const;
    FlagData const& getFlagData() const;
    TraitData const& getTraitData() const;
    bool supportsFlag(std::string const& flag) const;
    std::set<std::string> const& getSupportedFlags() const;
    std::string const& getVisibleTo() const;
    bool hasBuildFile() const;
    FileLocation const& getLocation() const;
    ItemConfig::Backend getBackend() const;
    bool hasAntBuild() const;
    std::string const& getBuildFile() const;

    std::string const& getAbsolutePath() const;
    std::string const& getTreeTop() const;
    // Note: list does not include the item itself
    std::list<std::string> const& getExpandedDependencies() const;
    unsigned int getBackingDepth() const;
    unsigned int getExternalDepth() const;
    bool isLocal() const;
    std::set<std::string> const& getShadowedDependencies() const;
    std::map<std::string, std::set<std::string> > const&
    getShadowedPlugins() const;
    std::set<std::string> getPlatformTypes() const;
    std::string const& getPlatformType(std::string const& platform) const;
    std::set<std::string> getBuildablePlatforms() const;
    std::set<std::string> const& getBuildPlatforms() const;
    std::string getBestPlatformForType(std::string platform_type,
				       PlatformSelector const*) const;
    TargetType::target_type_e getTargetType() const;
    bool isNamed(std::set<std::string>& item_names) const;
    bool matchesPattern(boost::regex& pattern) const;
    bool isWritable() const;
    bool isAtOrBelowPath(std::string const& path) const;
    bool hasShadowedReferences() const;
    Interface const& getInterface(std::string const& platform) const;
    // True iff item has all listed traits.  Returns true if list is empty.
    bool hasTraits(std::list<std::string> const& traits) const;
    bool isPlugin() const;
    std::list<std::string> const& getPlugins() const;
    // Return a list of all items this item references, including itself
    std::set<std::string> getReferences() const;

    void incrementBackingDepth();
    void incrementExternalDepth();
    void setReadOnly();
    void addShadowedPlugin(std::string const& local_tree,
			   std::string const& remote_tree);
    void setPlatformTypes(std::set<std::string> const& platform_types);
    void setTargetType(TargetType::target_type_e target_type);
    void setBuildablePlatforms(
	std::string const& platform_type,
	std::vector<std::string> const& buildable_platforms);
    void setBuildablePlatforms(std::set<std::string> const&);
    void setBuildPlatforms(std::set<std::string> const&);
    void addBuildPlatform(std::string const&);

    // Note: if last item of passed-in list of expanded dependencies
    // is the item itself, it is removed.
    void setExpandedDependencies(std::list<std::string> const&);

    void setShadowedDependencies(std::set<std::string> const&);
    void setInterface(std::string const& platform,
		      boost::shared_ptr<Interface>);
    void setPlugin(bool);
    void setPlugins(std::list<std::string> const&);

  private:
    // allow copying

    // platform type -> [ buildable platform, ... ]
    typedef std::map<std::string, std::vector<std::string> > pt_map;

    void assertLocal() const;

    std::string item_name;
    ItemConfig const* config;         // memory managed by ItemConfig
    std::string absolute_path;	// canonical path to item directory
    std::string tree_top;	// containing tree
    unsigned int backing_depth;	// 0 in local build tree and externals
    unsigned int external_depth;    // 0 in local build tree and backing chain
    bool force_read_only;
    pt_map platform_types;	    // platform types and associated platforms
    std::map<std::string, std::string> platform_to_type;
    std::set<std::string> build_platforms; // platforms we will build on
    std::list<std::string> expanded_dependencies; // recursively expanded
    std::set<std::string> shadowed_dependencies;
    std::map<std::string, std::set<std::string> > shadowed_plugins;
    std::map<std::string, boost::shared_ptr<Interface> > interfaces;
    TargetType::target_type_e target_type;
    bool plugin;		// plugin in the containing tree
    std::list<std::string> plugins;
};

#endif // __BUILDITEM_HH__
