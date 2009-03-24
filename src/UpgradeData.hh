#ifndef __UPGRADEDATA_HH__
#define __UPGRADEDATA_HH__

// This class holds onto upgrade data.  It is tightly coupled with
// code in Abuild-upgrade.cc, which directly accesses its data
// members.

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
    void writeUpgradeData(
	std::vector<std::list<std::string> > const& forests) const;

    // Data stored in configuration file.  All paths in the input file
    // are relative.  Internally, ignored_directories and
    // do_not_upgrade are lists of absolute paths.  Keys in tree_names
    // are relative paths.

    std::set<std::string> ignored_directories;
    std::set<std::string> do_not_upgrade;
    std::map<std::string, std::string> tree_names;

    // work data for abuild --upgrade-trees
    std::map<std::string, bool> item_dirs; // item_dir -> is_root
    bool upgrade_required;

  private:
    void readUpgradeData();

    Error& error;
};

#endif // __UPGRADEDATA_HH__
