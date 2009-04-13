#include "FlagData.hh"
#include <assert.h>

void
FlagData::addFlag(std::string const& name, std::string const& flag)
{
    this->flag_data[name].insert(flag);
}

bool
FlagData::isSet(std::string const& name, std::string const& flag) const
{
    bool result = false;
    if (this->flag_data.count(name) != 0)
    {
	result = ((*(this->flag_data.find(name))).second.count(flag) != 0);
    }
    return result;
}

std::set<std::string>
FlagData::getNames() const
{
    std::set<std::string> result;
    for (std::map<std::string, std::set<std::string> >::const_iterator iter =
	     this->flag_data.begin();
	 iter != this->flag_data.end(); ++iter)
    {
	result.insert((*iter).first);
    }
    return result;
}

bool
FlagData::hasFlags(std::string const& name) const
{
    return (this->flag_data.count(name) != 0);
}

std::set<std::string> const&
FlagData::getFlags(std::string const& name) const
{
    assert(hasFlags(name));
    return (*(this->flag_data.find(name))).second;
}

void
FlagData::removeItem(std::string const& name)
{
    this->flag_data.erase(name);
}
