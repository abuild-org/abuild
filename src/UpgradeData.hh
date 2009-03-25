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
    static std::string const PLACEHOLDER;

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
    std::map<std::string, std::list<std::string> > externals;
    std::map<std::string, std::list<std::string> > tree_deps;

    // work data for abuild --upgrade-trees
    std::map<std::string, bool> item_dirs; // item_dir -> is_root
    bool upgrade_required;
    bool missing_treenames;

  private:
    void readUpgradeData();

    Error& error;
};

#endif // __UPGRADEDATA_HH__