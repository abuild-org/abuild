#include "TraitData.hh"

void
TraitData::addTrait(std::string const& trait)
{
    // Force the item to exist.  Do not clear it.
    (void) this->trait_data[trait].empty();
}

void
TraitData::addTraitItem(std::string const& trait, std::string const& item)
{
    this->trait_data[trait].insert(item);
}

bool
TraitData::hasTrait(std::string const& trait) const
{
    return (this->trait_data.count(trait) != 0);
}

bool
TraitData::hasTraitItem(std::string const& trait, std::string const& item) const
{
    return (hasTrait(trait) &&
	    ((*(this->trait_data.find(trait))).second.count(item) != 0));
}

std::map<std::string, std::set<std::string> > const&
TraitData::getTraitData() const
{
    return this->trait_data;
}

void
TraitData::removeItem(std::string const& item)
{
    for (trait_data_t::iterator iter = this->trait_data.begin();
	 iter != this->trait_data.end(); ++iter)
    {
	(*iter).second.erase(item);
    }
}
