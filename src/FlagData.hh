#ifndef __FLAGDATA_HH__
#define __FLAGDATA_HH__

#include <string>
#include <set>
#include <map>

class FlagData
{
  public:
    void addFlag(std::string const& name, std::string const& flag);
    bool isSet(std::string const& name, std::string const& flag) const;
    std::set<std::string> getNames() const;
    bool hasFlags(std::string const& name) const;
    std::set<std::string> const& getFlags(std::string const& name) const;
    void removeItem(std::string const& name);

  private:
    // name -> [ flag, ... ]
    std::map<std::string, std::set<std::string> > flag_data;
};

#endif // __FLAGDATA_HH__
