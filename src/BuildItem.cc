#include <BuildItem.hh>

#include <Util.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <boost/regex.hpp>

BuildItem::BuildItem(std::string const& item_name,
		     ItemConfig const* config,
		     std::string const& absolute_path,
		     std::string const& tree_top) :
    item_name(item_name),
    config(config),
    absolute_path(absolute_path),
    tree_top(tree_top),
    backing_depth(0),
    external_depth(0),
    target_type(TargetType::tt_unknown),
    plugin(false),
    plugin_anywhere(false)
{
    // An item is always writable and always has backing depth 0
    // relative to its local build tree.

    // Initialize platform_types based on what was declared.
    // Additional platform types may be added at runtime, and platform
    // data will be supplied later.
    setPlatformTypes(config->getPlatformTypes());
}

std::string const&
BuildItem::getName() const
{
    return this->config->getName();
}

std::string const&
BuildItem::getDescription() const
{
    return this->config->getDescription();
}

std::string const&
BuildItem::getParent() const
{
    return this->config->getParent();
}

std::list<std::string> const&
BuildItem::getChildren() const
{
    return this->config->getChildren();
}

std::list<std::string> const&
BuildItem::getExternals() const
{
    return this->config->getExternals();
}

std::list<std::string> const&
BuildItem::getDeps() const
{
    return this->config->getDeps();
}

std::string const&
BuildItem::getDepPlatformType(std::string const& dep) const
{
    PlatformSelector const* ps = 0;
    return this->config->getDepPlatformType(dep, ps);
}

std::string const&
BuildItem::getDepPlatformType(std::string const& dep,
			      PlatformSelector const*& ps) const
{
    return this->config->getDepPlatformType(dep, ps);
}

FlagData const&
BuildItem::getFlagData() const
{
    return this->config->getFlagData();
}

TraitData const&
BuildItem::getTraitData() const
{
    return this->config->getTraitData();
}

bool
BuildItem::supportsFlag(std::string const& flag) const
{
    return this->config->supportsFlag(flag);
}

std::set<std::string> const&
BuildItem::getSupportedFlags() const
{
    return this->config->getSupportedFlags();
}

std::string const&
BuildItem::getVisibleTo() const
{
    return this->config->getVisibleTo();
}

bool
BuildItem::hasBuildFile() const
{
    return this->config->hasBuildFile();
}

FileLocation const&
BuildItem::getLocation() const
{
    return this->config->getLocation();
}

ItemConfig::Backend
BuildItem::getBackend() const
{
    return this->config->getBackend();
}

bool
BuildItem::hasAntBuild() const
{
    return this->config->hasAntBuild();
}

std::string const&
BuildItem::getBuildFile() const
{
    return this->config->getBuildFile();
}

std::string const&
BuildItem::getAbsolutePath() const
{
    return this->absolute_path;
}

std::string const&
BuildItem::getTreeTop() const
{
    return this->tree_top;
}

std::list<std::string> const&
BuildItem::getExpandedDependencies() const
{
    return this->expanded_dependencies;
}

unsigned int
BuildItem::getBackingDepth() const
{
    return this->backing_depth;
}

unsigned int
BuildItem::getExternalDepth() const
{
    return this->external_depth;
}

bool
BuildItem::isLocal() const
{
    return ((this->backing_depth == 0) && (this->external_depth == 0));
}

std::map<std::string, std::set<std::string> > const&
BuildItem::getShadowedPlugins() const
{
    return this->shadowed_plugins;
}

std::set<std::string> const&
BuildItem::getShadowedDependencies() const
{
    return this->shadowed_dependencies;
}

TargetType::target_type_e
BuildItem::getTargetType() const
{
    if (this->target_type == TargetType::tt_unknown)
    {
	throw QEXC::Internal(
	    "attempt to retrieve build item target type before it was set");
    }
    return this->target_type;
}

std::set<std::string>
BuildItem::getPlatformTypes() const
{
    std::set<std::string> result;
    for (pt_map::const_iterator iter = this->platform_types.begin();
	 iter != this->platform_types.end(); ++iter)
    {
	result.insert((*iter).first);
    }
    return result;
}

std::string const&
BuildItem::getPlatformType(std::string const& platform) const
{
    if (this->target_type == TargetType::tt_object_code)
    {
	assert(this->platform_to_type.count(platform) != 0);
	return (*(this->platform_to_type.find(platform))).second;
    }
    else
    {
	return platform;
    }
}

std::set<std::string>
BuildItem::getBuildablePlatforms() const
{
    std::set<std::string> result;
    for (pt_map::const_iterator iter = this->platform_types.begin();
	 iter != this->platform_types.end(); ++iter)
    {
	std::vector<std::string> const& platforms = (*iter).second;
	result.insert(platforms.begin(), platforms.end());
    }
    return result;
}

std::string
BuildItem::getBestPlatformForType(std::string platform_type,
				  PlatformSelector const* ps) const
{
    std::string result;
    if (ps)
    {
	platform_type = ps->getPlatformType();
    }
    if (this->platform_types.count(platform_type) != 0)
    {
	std::vector<std::string> const& platforms =
	    (*(this->platform_types.find(platform_type))).second;
	if (! platforms.empty())
	{
	    if (ps)
	    {
		PlatformSelector::Matcher m(platforms[0], *ps);
		for (std::vector<std::string>::const_iterator iter =
			 platforms.begin();
		     iter != platforms.end(); ++iter)
		{
		    if (m.matches(*iter))
		    {
			QTC::TC("abuild", "BuildItem used selector for type");
			result = *iter;
			break;
		    }
		}
	    }
	    if (result.empty())
	    {
		QTC::TC("abuild", "BuildItem used first for type");
		result = platforms[0];
	    }
	}
    }
    return result;
}

std::set<std::string> const&
BuildItem::getBuildPlatforms() const
{
    return this->build_platforms;
}

bool
BuildItem::isNamed(std::set<std::string>& item_names) const
{
    if (item_names.count(this->item_name))
    {
	item_names.erase(this->item_name);
	return true;
    }
    return false;
}

bool
BuildItem::matchesPattern(boost::regex& pattern) const
{
    boost::smatch match;
    return boost::regex_match(this->item_name, match, pattern);
}

bool
BuildItem::isWritable() const
{
    return (this->backing_depth == 0);
}

bool
BuildItem::isAtOrBelowPath(std::string const& path) const
{
    if (path == this->absolute_path)
    {
	return true;
    }
    else if (this->absolute_path.substr(0, path.length() + 1) == (path + "/"))
    {
	return true;
    }
    return false;
}

bool
BuildItem::hasShadowedReferences() const
{
    return (! (this->shadowed_plugins.empty() &&
	       this->shadowed_dependencies.empty()));
}

Interface const&
BuildItem::getInterface(std::string const& platform) const
{
    return *((*(this->interfaces.find(platform))).second);
}

bool
BuildItem::hasTraits(std::list<std::string> const& traits) const
{
    TraitData const& trait_data = getTraitData();
    for (std::list<std::string>::const_iterator iter = traits.begin();
	 iter != traits.end(); ++iter)
    {
	if (! trait_data.hasTrait(*iter))
	{
	    return false;
	}
    }
    return true; // return true if list is empty

}

bool
BuildItem::isPlugin() const
{
    return this->plugin;
}

bool
BuildItem::isPluginAnywhere() const
{
    return this->plugin_anywhere;
}

std::list<std::string> const&
BuildItem::getPlugins() const
{
    return this->plugins;
}

std::set<std::string>
BuildItem::getReferences() const
{
    // We only include dependencies and plugins here, not items we
    // refer to with a trait.  If we were to add trait referent items
    // here, we'd also have to add them to the build set so that we
    // could get information about them.  That would have to be done
    // recursively.  Then we might want to extend the integrity
    // guarantee to cover trait referent items, and this could get out
    // of hand.  If we really wanted to be able to include traits here
    // so that we could get the location of any item we referred to
    // through a trait, we could let go of the policy of throwing away
    // all build item data about items not in the build set so that we
    // could still look up the paths to these items.  That would be
    // unfortunate though since a number of potential errors have been
    // caught because of that policy.  Bottom line: if you want the
    // location of something, you have to depend on it.
    std::set<std::string> references;
    references.insert(this->expanded_dependencies.begin(),
		      this->expanded_dependencies.end());
    references.insert(this->plugins.begin(), this->plugins.end());
    references.insert(this->item_name);
    return references;
}

void
BuildItem::incrementBackingDepth()
{
    ++this->backing_depth;
}

void
BuildItem::incrementExternalDepth()
{
    ++this->external_depth;
}

void
BuildItem::addShadowedPlugin(std::string const& local_tree,
			     std::string const& remote_tree)
{
    this->shadowed_plugins[local_tree].insert(remote_tree);
}

void
BuildItem::setPlatformTypes(std::set<std::string> const& platform_types)
{
    assertLocal();
    for (std::set<std::string>::const_iterator iter = platform_types.begin();
	 iter != platform_types.end(); ++iter)
    {
	this->platform_types[*iter].clear();
    }
}

void
BuildItem::setTargetType(TargetType::target_type_e target_type)
{
    assertLocal();
    this->target_type = target_type;
}

void
BuildItem::setBuildablePlatforms(
    std::string const& platform_type,
    std::vector<std::string> const& buildable_platforms)
{
    assertLocal();
    assert(this->platform_types.count(platform_type) != 0);
    this->platform_types[platform_type] = buildable_platforms;
    for (std::vector<std::string>::const_iterator iter =
	     buildable_platforms.begin();
	 iter != buildable_platforms.end(); ++iter)
    {
	this->platform_to_type[*iter] = platform_type;
    }
}

void
BuildItem::setBuildPlatforms(std::set<std::string> const& platforms)
{
    this->build_platforms = platforms;
}

void
BuildItem::addBuildPlatform(std::string const& platform)
{
    this->build_platforms.insert(platform);
}

void
BuildItem::setExpandedDependencies(
    std::list<std::string> const& expanded_dependencies)
{
    // allowed for non-local build items -- may include shadowed
    // dependencies
    this->expanded_dependencies = expanded_dependencies;
    if (this->expanded_dependencies.back() == this->item_name)
    {
	this->expanded_dependencies.pop_back();
    }
}

void
BuildItem::setShadowedDependencies(
    std::set<std::string> const& shadowed_dependencies)
{
    this->shadowed_dependencies = shadowed_dependencies;
}

void
BuildItem::setInterface(std::string const& platform,
			boost::shared_ptr<Interface> interface)
{
    this->interfaces[platform] = interface;
}

void
BuildItem::setPlugin(bool val)
{
    // allowed for non-local build items
    this->plugin = val;
    if (val)
    {
	this->plugin_anywhere = val;
    }
}

void
BuildItem::setPlugins(std::list<std::string> const& plugins)
{
    assertLocal();
    this->plugins = plugins;
}

void
BuildItem::assertLocal() const
{
    if (! ((this->backing_depth == 0) &&
	   (this->external_depth == 0)))
    {
	throw QEXC::Internal(
	    "attempt to perform disallowed operation on non-local build item");
    }
}
