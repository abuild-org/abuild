#include <BuildItem.hh>

#include <Util.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <assert.h>
#include <boost/regex.hpp>

BuildItem::BuildItem(std::string const& item_name,
		     std::string const& tree_name,
		     ItemConfig const* config) :
    item_name(item_name),
    config(config),
    deps(config->getDeps()),
    flag_data(config->getFlagData()),
    trait_data(config->getTraitData()),
    tree_name(tree_name),
    backing_depth(0),
    target_type(TargetType::tt_unknown)
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

std::list<std::string> const&
BuildItem::getChildren() const
{
    return this->config->getChildren();
}

std::list<ItemConfig::BuildAlso> const&
BuildItem::getBuildAlso() const
{
    return this->config->getBuildAlso();
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
    return this->config->getAbsolutePath();
}

std::set<std::string> const&
BuildItem::getOptionalDeps() const
{
    return this->config->getOptionalDeps();
}

bool
BuildItem::isSerial() const
{
    return this->config->isSerial();
}

std::list<std::string> const&
BuildItem::getDeps() const
{
    return this->deps;
}

FlagData const&
BuildItem::getFlagData() const
{
    return this->flag_data;
}

TraitData const&
BuildItem::getTraitData() const
{
    return this->trait_data;
}

std::string const&
BuildItem::getTreeName() const
{
    return this->tree_name;
}

void
BuildItem::setForestRoot(std::string const& root)
{
    this->forest_root = root;
}

std::string const&
BuildItem::getForestRoot() const
{
    return this->forest_root;
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

bool
BuildItem::isLocal() const
{
    return (this->backing_depth == 0);
}

bool
BuildItem::isInTree(std::string const& tree) const
{
    return (this->tree_name == tree);
}

bool
BuildItem::isInTrees(std::set<std::string> const& trees) const
{
    return (trees.count(this->tree_name) != 0);
}

bool
BuildItem::isInTreesAndAtOrBelowPath(std::set<std::string> const& trees,
				     std::string const& path) const
{
    return (isInTrees(trees) && isAtOrBelowPath(path));
}

std::set<std::string> const&
BuildItem::getShadowedReferences() const
{
    return this->shadowed_references;
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
BuildItem::getBestPlatformForType(
    std::string platform_type,
    PlatformSelector const* ps,
    std::map<std::string, PlatformSelector> const& platform_selectors) const
{
    // Pick the highest priority selected platform of the requested
    // platform type that matches the given platform selector.  If no
    // platform selector was provided, fall back to general ones
    // supplied.  If no matching platform is selected, then pick the
    // first matching platform.  If no matching platforms are
    // available, return the empty string, which will be considered an
    // error.
    std::string result;

    if (ps)
    {
	QTC::TC("abuild", "BuildItem platform selection with selector");
    }
    else
    {
	// If the dependency was declared with a platform type
	// only and no selector, see if the user specified any
	// general selectors.
	if (platform_selectors.count(platform_type))
	{
	    QTC::TC("abuild", "BuildItem specific platform selector");
	    ps = &((*(platform_selectors.find(platform_type))).second);
	}
	else if (platform_selectors.count(PlatformSelector::ANY))
	{
	    QTC::TC("abuild", "BuildItem default platform selector");
	    ps = &((*(platform_selectors.find(PlatformSelector::ANY))).second);
	}
	else
	{
	    QTC::TC("abuild", "BuildItem no platform selector");
	}
	if (ps && ps->isSkip())
	{
	    QTC::TC("abuild", "BuildItem ignoring skip selector");
	    ps = 0;
	}
    }

    if (ps && (ps->getPlatformType() != PlatformSelector::ANY))
    {
	platform_type = ps->getPlatformType();
    }
    if (this->platform_types.count(platform_type) != 0)
    {
	std::vector<std::string> const& platforms =
	    (*(this->platform_types.find(platform_type))).second;
	if (! platforms.empty())
	{
	    if ((this->target_type == TargetType::tt_object_code) &&
		ps && (! ps->isDefault()))
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
		QTC::TC("abuild", "BuildItem used first for type",
			(this->target_type == TargetType::tt_object_code)
			? 1 : 0);
		result = platforms[0];
	    }
	}
    }
    return result;
}

std::string
BuildItem::getBestPlatformForPlatform(
    std::string const& platform,
    std::map<std::string, PlatformSelector> const& platform_selectors) const
{
    // Pick the best choice among our buildable platforms for
    // compatibility with the specified platform.  If we can't find
    // one, return the empty string, which will be considered an
    // error.
    std::string result;
    if ((this->target_type == TargetType::tt_all) ||
	(getBuildablePlatforms().count(platform)))
    {
	// If we are an "all" build item, we can build on any
	// platform.  If we actually build on the exact platform
	// requested, then this is the best match.  (That's actually
	// the normal case.)  In either case, we are able to build on
	// the requsted platform.
	QTC::TC("abuild", "BuildItem best platform for platform is platform",
		(this->target_type == TargetType::tt_all) ? 0 : 1);
	result = platform;
    }
    else
    {
	// If we can't build on the requested platform, see if we can
	// build on any platform that is compatible with the requested
	// platform.  Find out from platform_data what platform type
	// the requested platform belongs to and what other platform
	// types are compatible with it.  Then see if we build on any
	// of those platform types and pick the best matching platform
	// for the best matching platform type.  Note that once we
	// find a matching platform type, we stop looking.  This is
	// true even if the first matching platform type has no
	// matching platforms but subsequent matching types do have
	// matching platforms.  The reason for this is that the list
	// of available platform types is static while the list of
	// available platforms may depend on the environment, and we
	// don't want the shape of the build graph to be influenced by
	// the environment (beyond the influence of platform
	// selectors, which can still only change which platform is
	// selected within a platform type).
	assert(this->platform_data.get());
	std::string const& platform_type =
	    this->platform_data->getPlatformType(platform);
	std::vector<std::string> const& compatible_types =
	    this->platform_data->getCompatiblePlatformTypes(platform_type);
	for (std::vector<std::string>::const_iterator iter =
		 compatible_types.begin();
	     iter != compatible_types.end(); ++iter)
	{
	    if (this->platform_types.count(*iter))
	    {
		result = getBestPlatformForType(*iter, 0, platform_selectors);
		break;
	    }
	}

	if (! result.empty())
	{
	    QTC::TC("abuild", "BuildItem found compatible platform type");
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
BuildItem::isAtOrBelowPath(std::string const& path) const
{
    std::string const& absolute_path = getAbsolutePath();
    return Util::isDirUnder(absolute_path, path);
}

bool
BuildItem::hasShadowedReferences() const
{
    return (! this->shadowed_references.empty());
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

std::map<std::string, bool> const&
BuildItem::getOptionalDependencyPresence() const
{
    return this->optional_dep_presence;
}

void
BuildItem::incrementBackingDepth()
{
    ++this->backing_depth;
}

void
BuildItem::setOptionalDependencyPresence(std::string const& item,
					 bool present)
{
    this->optional_dep_presence[item] = present;
    if (! present)
    {
	// Make this item forget everything it knows about this
	// dependency.  Must be called before setExpandedDependencies.
	assert(this->expanded_dependencies.empty());
	QTC::TC("abuild", "BuildItem remove dependency");
	this->deps.remove(item);
	this->flag_data.removeItem(item);
	this->trait_data.removeItem(item);
    }
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
BuildItem::setPlatformData(boost::shared_ptr<PlatformData> platform_data)
{
    this->platform_data = platform_data;
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
BuildItem::setShadowedReferences(
    std::set<std::string> const& shadowed_references)
{
    this->shadowed_references = shadowed_references;
}

void
BuildItem::setInterface(std::string const& platform,
			boost::shared_ptr<Interface> _interface)
{
    this->interfaces[platform] = _interface;
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
    if (this->backing_depth != 0)
    {
	throw QEXC::Internal(
	    "attempt to perform disallowed operation on non-local build item");
    }
}
