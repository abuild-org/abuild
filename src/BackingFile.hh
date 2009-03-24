#ifndef __BACKINGFILE_HH__
#define __BACKINGFILE_HH__

#include <FileLocation.hh>
#include <boost/shared_ptr.hpp>
#include <string>
#include <list>
#include <set>
#include <map>

class Error;
class CompatLevel;

class BackingFile
{
  public:
    static BackingFile* readBacking(Error& error_handler,
				    CompatLevel const& compat_level,
				    std::string const& dir);
    static std::string const FILE_BACKING;

    std::list<std::string> const& getBackingAreas() const;
    std::set<std::string> const& getDeletedTrees() const;
    std::set<std::string> const& getDeletedItems() const;

  private:
    BackingFile(BackingFile const&);
    BackingFile& operator=(BackingFile const&);


    static std::string const k_BACKING_AREAS;
    static std::string const k_DELETED_ITEMS;
    static std::string const k_DELETED_TREES;
    static std::set<std::string> required_keys;
    static std::map<std::string, std::string> defaulted_keys;

    static bool statics_initialized;
    static void initializeStatics();

    typedef boost::shared_ptr<BackingFile> BackingFile_ptr;
    static std::map<std::string, BackingFile_ptr> cache;

    BackingFile(Error&, CompatLevel const&, FileLocation const&,
		std::string const& dir);
    bool readOldFormat();
    void validate();

    Error& error;
    CompatLevel const& compat_level;
    FileLocation location;
    std::string dir;
    bool deprecated;

    std::list<std::string> backing_areas;
    std::set<std::string> deleted_trees;
    std::set<std::string> deleted_items;
};

#endif // __BACKINGFILE_HH__
