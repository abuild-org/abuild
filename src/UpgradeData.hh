#ifndef __UPGRADEDATA_HH__
#define __UPGRADEDATA_HH__

// This class holds onto upgrade data.  It is tightly coupled with
// code in Abuild-upgrade.cc, which directly accesses its data
// members.

#include <string>
#include <list>
#include <set>
#include <map>

class Error;
class ItemConfig;

class UpgradeData
{
  public:
    static std::string const FILE_UPGRADE_DATA;
    static std::string const PLACEHOLDER;

    UpgradeData(Error& error);
    void writeUpgradeData() const;

    // Data stored in configuration file.  All paths in the input file
    // are relative.  Internally, ignored_directories and
    // do_not_upgrade are lists of absolute paths.  Keys in tree_names
    // are relative paths.

    std::set<std::string> ignored_directories;
    std::map<std::string, std::string> tree_names;

    // work data for abuild --upgrade-trees
    std::map<std::string, ItemConfig*> items;
    bool upgrade_required;
    bool missing_treenames;
    std::map<std::string, std::list<std::string> > externals;
    std::map<std::string, std::list<std::string> > tree_deps;
    std::map<std::string, std::list<std::string> > backing_areas;
    std::map<std::string, std::set<std::string> > deleted_trees;
    std::map<std::string, std::set<std::string> > deleted_items;
    std::map<std::string, std::string> item_tree_roots;
    std::map<std::string, std::string> tree_forest_roots;
    std::map<std::string, std::list<std::string> > forest_contents;

  private:
    void readUpgradeData();

    Error& error;
};

#endif // __UPGRADEDATA_HH__
