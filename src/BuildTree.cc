#include <BuildTree.hh>
#include <ItemConfig.hh>

BuildTree::BuildTree(std::string const& name,
		     std::string const& root_path,
		     std::list<std::string> const& tree_deps,
		     std::set<std::string> const& declared_traits,
		     std::list<std::string> const& plugins,
		     PlatformData const& platform_data) :
    name(name),
    root_path(root_path),
    location(root_path + "/" + ItemConfig::FILE_CONF, 0, 0),
    tree_deps(tree_deps),
    supported_traits(declared_traits),
    plugins(plugins),
    platform_data(platform_data),
    backing_depth(0)
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
BuildTree::getName() const
{
    return this->name;
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

std::list<std::string> const&
BuildTree::getExpandedTreeDeps() const
{
    return this->expanded_tree_deps;
}

void
BuildTree::setForestRoot(std::string const& root)
{
    this->forest_root = root;
}

std::string const&
BuildTree::getForestRoot() const
{
    return this->forest_root;
}

int
BuildTree::getBackingDepth() const
{
    return this->backing_depth;
}

bool
BuildTree::isLocal() const
{
    return (this->backing_depth == 0);
}

void
BuildTree::incrementBackingDepth()
{
    ++this->backing_depth;
}

void
BuildTree::setExpandedTreeDeps(std::list<std::string> const& exp)
{
    this->expanded_tree_deps = exp;
    if (this->expanded_tree_deps.back() == this->name)
    {
	this->expanded_tree_deps.pop_back();
    }
}
