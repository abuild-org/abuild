#include <BuildTree.hh>
#include <QEXC.hh>

BuildTree::BuildTree(std::string const& backing_area,
		     std::map<std::string, std::string> const& externals,
		     std::map<std::string, std::string> const& backed_externals,
		     std::set<std::string> const& declared_traits,
		     std::set<std::string> const& deleted_items,
		     std::list<std::string> const& plugins,
		     PlatformData const& platform_data) :
    backing_area(backing_area),
    externals(externals),
    backed_externals(backed_externals),
    supported_traits(declared_traits),
    deleted_items(deleted_items),
    plugins(plugins),
    platform_data(platform_data)
{
}

void
BuildTree::setBuildItems(BuildItem_map const& items,
			 std::map<std::string, std::string> const& paths)
{
    this->build_items = items;
    this->paths = paths;
}

void
BuildTree::setSortedItemNames(std::list<std::string> const& sorted_item_names)
{
    this->sorted_item_names = sorted_item_names;
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
BuildTree::getBackingArea() const
{
    return this->backing_area;
}

std::map<std::string, std::string> const&
BuildTree::getExternals() const
{
    return this->externals;
}

std::map<std::string, std::string> const&
BuildTree::getBackedExternals() const
{
    return this->backed_externals;
}

BuildTree::BuildItem_map&
BuildTree::getBuildItems()
{
    return this->build_items;
}

std::map<std::string, std::string> const&
BuildTree::getPaths() const
{
    return this->paths;
}

std::list<std::string> const&
BuildTree::getSortedItemNames() const
{
    return this->sorted_item_names;
}

std::set<std::string> const&
BuildTree::getSupportedTraits() const
{
    return this->supported_traits;
}

std::set<std::string> const&
BuildTree::getDeletedItems() const
{
    return this->deleted_items;
}

std::list<std::string> const&
BuildTree::getPlugins() const
{
    return this->plugins;
}
