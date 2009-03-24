#include <BuildTree.hh>

BuildTree::BuildTree(std::list<std::string> const& tree_deps,
		     std::set<std::string> const& declared_traits,
		     std::list<std::string> const& plugins,
		     PlatformData const& platform_data) :
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
