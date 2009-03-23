#include <BackingFile.hh>

#include <Error.hh>
#include <CompatLevel.hh>
#include <Util.hh>
#include <QTC.hh>
#include <QEXC.hh>

std::string const BackingFile::FILE_BACKING = "Abuild.backing";

std::map<std::string, BackingFile::BackingFile_ptr> BackingFile::cache;

BackingFile*
BackingFile::readBacking(Error& error_handler,
			 CompatLevel const& compat_level,
			 std::string const& dir)
{
    if (cache.count(dir))
    {
	return cache[dir].get();
    }

    FileLocation location(dir + "/" + FILE_BACKING, 0, 0);
    BackingFile_ptr bf;
    bf.reset(
	new BackingFile(error_handler, compat_level, location, dir));
    bf->validate();

    // Cache and return
    cache[dir] = bf;
    return bf.get();
}

BackingFile::BackingFile(
    Error& error,
    CompatLevel const& compat_level,
    FileLocation const& location,
    std::string const& dir)
    :
    error(error),
    compat_level(compat_level),
    location(location),
    dir(dir),
    deprecated(false)
{
}

void
BackingFile::validate()
{
    std::string file = dir + "/" + FILE_BACKING;
    std::list<std::string> lines = Util::readLinesFromFile(file);

    { // local scope
	// Remove comments and blank lines
	std::list<std::string>::iterator iter = lines.begin();
	while (iter != lines.end())
	{
	    std::list<std::string>::iterator next = iter;
	    ++next;
	    if (((*iter).length() == 0) || ((*iter)[0] == '#'))
	    {
		lines.erase(iter, next);
	    }
	    iter = next;
	}
    }

    if (lines.size() == 1)
    {
	this->backing_area = lines.front();
	if (! Util::isAbsolutePath(backing_area))
	{
	    backing_area = dir + "/" + backing_area;
	}
	backing_area = Util::canonicalizePath(backing_area);
    }
    else
    {
        QTC::TC("abuild", "BackingFile ERR invalid backing file");
        this->error.error(this->location, "invalid syntax");
    }
}

std::string const&
BackingFile::getBackingArea() const
{
    return this->backing_area;
}
