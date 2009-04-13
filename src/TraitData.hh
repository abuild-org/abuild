#ifndef __TRAITDATA_HH__
#define __TRAITDATA_HH__

#include <string>
#include <list>
#include <set>
#include <map>

class TraitData
{
  public:
    typedef std::map<std::string, std::set<std::string> > trait_data_t;

    void addTrait(std::string const& trait);
    void addTraitItem(std::string const& trait, std::string const& item);
    bool hasTrait(std::string const& trait) const;
    bool hasTraitItem(std::string const& trait, std::string const& item) const;
    trait_data_t const& getTraitData() const;
    void removeItem(std::string const& item);

  private:
    // trait -> [ item, ... ]
    trait_data_t trait_data;
};

#endif // __TRAITDATA_HH__
