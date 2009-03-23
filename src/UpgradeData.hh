#ifndef __UPGRADEDATA_HH__
#define __UPGRADEDATA_HH__

#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>

class Error;

class UpgradeData
{
  public:
    static std::string const FILE_UPGRADE_DATA;

    UpgradeData(Error& error);
    void scan();
    void writeUpgradeData(
	std::vector<std::list<std::string> > const& forests) const;

    // item_dir -> is_root
    std::map<std::string, bool> const& getItemDirs() const;

    bool upgradeRequired() const;

  private:
    void readUpgradeData();

    Error& error;

    std::set<std::string> ignored_directories;
    std::set<std::string> do_not_upgrade;
    std::map<std::string, std::string> tree_names;

    std::map<std::string, bool> item_dirs;
    bool upgrade_required;
};

#endif // __UPGRADEDATA_HH__
