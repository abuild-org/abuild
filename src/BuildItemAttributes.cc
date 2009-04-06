#include <BuildItemAttributes.hh>

BuildItemAttributes::BuildItemAttributes() :
    global_treedep(false),
    global_plugin(false)
{
}

void
BuildItemAttributes::setGlobalTreeDep(bool val)
{
    this->global_treedep = val;
}

bool
BuildItemAttributes::getGlobalTreeDep() const
{
    return this->global_treedep;
}

void
BuildItemAttributes::setGlobalPlugin(bool val)
{
    this->global_plugin = val;
}

bool
BuildItemAttributes::getGlobalPlugin() const
{
    return this->global_plugin;
}
