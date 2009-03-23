#ifndef __BACKINGFILE_HH__
#define __BACKINGFILE_HH__

#include <FileLocation.hh>
#include <boost/shared_ptr.hpp>
#include <string>
#include <list>
#include <map>

class Error;
class CompatLevel;

class BackingFile
{
  public:
    static BackingFile* readBacking(Error& error_handler,
				    CompatLevel const& compat_level,
				    std::string const& dir);
    static std::list<std::string>
    getBackingChain(Error& error_handler,
		    CompatLevel const& compat_level,
		    std::string const& dir);

    static std::string const FILE_BACKING;

    std::string const& getBackingArea() const;

  private:
    BackingFile(BackingFile const&);
    BackingFile& operator=(BackingFile const&);

    typedef boost::shared_ptr<BackingFile> BackingFile_ptr;
    static std::map<std::string, BackingFile_ptr> cache;

    BackingFile(Error&, CompatLevel const&, FileLocation const&,
		std::string const& dir);
    void validate();

    Error& error;
    CompatLevel const& compat_level;
    FileLocation location;
    std::string dir;
    bool deprecated;

    std::string backing_area;
};

#endif // __BACKINGFILE_HH__
