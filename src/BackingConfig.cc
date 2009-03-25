#include <BackingConfig.hh>

#include <Error.hh>
#include <CompatLevel.hh>
#include <Util.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <KeyVal.hh>
#include <set>

std::string const BackingConfig::FILE_BACKING = "Abuild.backing";

std::map<std::string, BackingConfig::BackingConfig_ptr> BackingConfig::cache;
std::set<std::string> BackingConfig::required_keys;
std::map<std::string, std::string> BackingConfig::defaulted_keys;

std::string const BackingConfig::k_BACKING_AREAS = "backing-areas";
std::string const BackingConfig::k_DELETED_ITEMS = "deleted-items";
std::string const BackingConfig::k_DELETED_TREES = "deleted-trees";
bool BackingConfig::statics_initialized = false;

void BackingConfig::initializeStatics()
{
    if (statics_initialized)
    {
	return;
    }

    required_keys.insert(k_BACKING_AREAS);
    defaulted_keys[k_DELETED_ITEMS] = "";
    defaulted_keys[k_DELETED_TREES] = "";

    statics_initialized = true;
}

BackingConfig*
BackingConfig::readBacking(Error& error_handler,
			   CompatLevel const& compat_level,
			   std::string const& dir)
{
    if (cache.count(dir))
    {
	return cache[dir].get();
    }

    FileLocation location(dir + "/" + FILE_BACKING, 0, 0);
    BackingConfig_ptr bf;
    bf.reset(
	new BackingConfig(error_handler, compat_level, location, dir));
    bf->validate();

    // Cache and return
    cache[dir] = bf;
    return bf.get();
}

BackingConfig::BackingConfig(
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
BackingConfig::validate()
{
    if (! (this->compat_level.allow_1_0() && readOldFormat()))
    {
	std::string file = dir + "/" + FILE_BACKING;
	KeyVal kv(file.c_str(), required_keys, defaulted_keys);
	if (! kv.readFile())
	{
	    // An error message has already been issued
	    QTC::TC("abuild", "BackingConfig ERR invalid backing file");
	    return;
	}

	this->backing_areas = Util::splitBySpace(kv.getVal(k_BACKING_AREAS));
	std::list<std::string> tmp =
	    Util::splitBySpace(kv.getVal(k_DELETED_ITEMS));
	this->deleted_items.insert(tmp.begin(), tmp.end());
	tmp = Util::splitBySpace(kv.getVal(k_DELETED_TREES));
	this->deleted_trees.insert(tmp.begin(), tmp.end());
    }

    for (std::list<std::string>::iterator iter = this->backing_areas.begin();
	 iter != this->backing_areas.end(); ++iter)
    {
	if (! Util::isAbsolutePath(*iter))
	{
	    *iter = this->dir + "/" + *iter;
	}
	*iter = Util::canonicalizePath(*iter);
    }
}

bool
BackingConfig::readOldFormat()
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
	std::string line = lines.front();
	if (line.find(':') == std::string::npos)
	{
	    backing_areas.push_back(line);
	    this->deprecated = true;
	}
    }

    if (this->deprecated)
    {
	// XXX report somewhere
	return true;
    }

    return false;
}

bool
BackingConfig::isDeprecated() const
{
    return this->deprecated;
}

std::list<std::string> const&
BackingConfig::getBackingAreas() const
{
    return this->backing_areas;
}

std::set<std::string> const&
BackingConfig::getDeletedTrees() const
{
    return this->deleted_trees;
}

std::set<std::string> const&
BackingConfig::getDeletedItems() const
{
    return this->deleted_items;
}

void
BackingConfig::appendBackingData(std::list<std::string>& ba,
				 std::set<std::string>& dt,
				 std::set<std::string>& di)
{
    ba.insert(ba.end(), this->backing_areas.begin(), this->backing_areas.end());
    dt.insert(this->deleted_trees.begin(), this->deleted_trees.end());
    di.insert(this->deleted_items.begin(), this->deleted_items.end());
}