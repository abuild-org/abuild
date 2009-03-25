#include <BuildTree.hh>
#include <ItemConfig.hh>

BuildTree::BuildTree(std::string const& root_path,
		     std::list<std::string> const& tree_deps,
		     std::set<std::string> const& declared_traits,
		     std::list<std::string> const& plugins,
		     PlatformData const& platform_data) :
    root_path(root_path),
    location(root_path + "/" + ItemConfig::FILE_CONF, 0, 0),
    tree_deps(tree_deps),
    supported_traits(declared_traits),
    plugins(plugins),
    platform_data(platform_data)
{
}

void
BuildTree::addTraits(std::set<std::string> const& traits)
{
    this->supported_traits.insert(traits.begin(), traits.end());
}

PlatformData&
BuildTree::getPlatformData()
{
    return this->platform_data;
}

std::string const&
BuildTree::getRootPath() const
{
    return this->root_path;
}

FileLocation const&
BuildTree::getLocation() const
{
    return this->location;
}

std::list<std::string> const&
BuildTree::getTreeDeps() const
{
    return this->tree_deps;
}

std::set<std::string> const&
BuildTree::getSupportedTraits() const
{
    return this->supported_traits;
}

std::list<std::string> const&
BuildTree::getPlugins() const
{
    return this->plugins;
}
