#include <ItemConfig.hh>

#include <boost/regex.hpp>
#include <QTC.hh>
#include <Error.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <FileLocation.hh>
#include <KeyVal.hh>
#include <CompatLevel.hh>

std::string const ItemConfig::FILE_CONF = "Abuild.conf";
std::string const ItemConfig::FILE_MK = "Abuild.mk";
std::string const ItemConfig::FILE_INTERFACE = "Abuild.interface";
std::string const ItemConfig::FILE_ANT = "Abuild-ant.properties";
std::string const ItemConfig::FILE_GROOVY = "Abuild.groovy";
std::string const ItemConfig::FILE_ANT_BUILD = "Abuild-ant.xml";
std::map<std::string, ItemConfig::ItemConfig_ptr> ItemConfig::cache;
std::string const ItemConfig::k_THIS = "this";
std::string const ItemConfig::k_PARENT = "parent-dir";
std::string const ItemConfig::k_EXTERNAL = "external-dirs";
std::string const ItemConfig::k_DELETED = "deleted";
std::string const ItemConfig::k_NAME = "name";
std::string const ItemConfig::k_TREENAME = "treename";
std::string const ItemConfig::k_DESCRIPTION = "description";
std::string const ItemConfig::k_CHILDREN = "child-dirs";
std::string const ItemConfig::k_VISIBLE_TO = "visible-to";
std::string const ItemConfig::k_BUILD_ALSO = "build-also";
std::string const ItemConfig::k_DEPS = "deps";
std::string const ItemConfig::k_PLATFORM = "platform-types";
std::string const ItemConfig::k_SUPPORTED_FLAGS = "supported-flags";
std::string const ItemConfig::k_SUPPORTED_TRAITS = "supported-traits";
std::string const ItemConfig::k_TRAITS = "traits";
std::string const ItemConfig::k_PLUGINS = "plugins";

// The following characters may never appear in build item names:
//
//  * Comma: various places in the code, including ant properties
//    files and name-based build set specification, use
//    comma-separated lists of build item names.
//
//  * Space: make code and Abuild.conf parsing use space-separated
//    lists of build items
//
//  * Colon: a single colon is used to separate the build item from
//    the platform in the build graph nodes
//
// Carefully consider the implications of adding new legal characters
// to the names of build items.  As currently specified, build item
// names are always valid file names and can always be specified on
// the command line without any special quoting.  These are compelling
// features.
std::string const ItemConfig::ITEM_NAME_RE =
    "[a-zA-Z0-9_][a-zA-Z0-9_-]*(?:\\.[a-zA-Z0-9_-]+)*";

std::map<std::string, std::string> ItemConfig::valid_keys;

// Initialize this after all other status
bool ItemConfig::statics_initialized = ItemConfig::initializeStatics();

bool ItemConfig::initializeStatics()
{
    valid_keys[k_THIS] = "";
    valid_keys[k_DESCRIPTION] = "";
    valid_keys[k_PARENT] = "";
    valid_keys[k_CHILDREN] = "";
    valid_keys[k_EXTERNAL] = "";
    valid_keys[k_BUILD_ALSO] = "";
    valid_keys[k_DEPS] = "";
    valid_keys[k_VISIBLE_TO] = "";
    valid_keys[k_PLATFORM] = "";
    valid_keys[k_SUPPORTED_FLAGS] = "";
    valid_keys[k_SUPPORTED_TRAITS] = "";
    valid_keys[k_TRAITS] = "";
    valid_keys[k_DELETED] = "";
    valid_keys[k_PLUGINS] = "";

    return true;
}

ItemConfig*
ItemConfig::readConfig(Error& error, CompatLevel const& compat_level,
		       std::string const& dir)
{
    std::string file = dir + "/" + FILE_CONF;
    if (cache.count(dir))
    {
	return cache[dir].get();
    }

    // An exception is thrown if the file does not exist.  It is the
    // caller's responsibility to ensure that it does.
    KeyVal kv(file.c_str(), std::set<std::string>(), valid_keys);

    kv.readFile();

    FileLocation location(file, 0, 0);
    ItemConfig_ptr item;
    item.reset(new ItemConfig(error, compat_level, location, kv, dir));
    item->validate();

    // Cache and return
    cache[dir] = item;
    return item.get();
}

void
ItemConfig::validate()
{
    this->name = this->kv.getVal(k_THIS);
    if (this->name.empty())
    {
	checkUnnamed();
    }
    else
    {
	checkName();
    }

    this->parent = this->kv.getVal(k_PARENT);
    if (! this->parent.empty())
    {
	checkParent();
	checkNonRoot();
    }

    checkChildren();
    checkBuildAlso();
    checkDeps();
    checkVisibleTo();
    checkBuildfile();
    checkPlatforms();
    checkExternals();
    checkSupportedFlags();
    checkSupportedTraits();
    checkTraits();
    checkDeleted();
    checkPlugins();

    // no validation required for description
    this->description = this->kv.getVal(k_DESCRIPTION);
}

void
ItemConfig::checkUnnamed()
{
    std::string msg = "is not permitted for unnamed build items";

    if (checkEmptyKey(k_BUILD_ALSO, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR build-also without this");
    }
    if (checkEmptyKey(k_DEPS, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR deps without this");
    }
    if (checkEmptyKey(k_PLATFORM, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR platform-types without this");
    }
    if (checkEmptyKey(k_SUPPORTED_FLAGS, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR supported-flags without this");
    }
    if (checkEmptyKey(k_TRAITS, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR traits without this");
    }
}

void
ItemConfig::checkNonRoot()
{
    std::string msg = "may only appear in a root build item";

    if (checkEmptyKey(k_EXTERNAL, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR external on non-root");
    }
    if (checkEmptyKey(k_SUPPORTED_TRAITS, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR supported-traits on non-root");
    }
    if (checkEmptyKey(k_DELETED, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR deleted on non-root");
    }
    if (checkEmptyKey(k_PLUGINS, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR plugins on non-root");
    }
}

void
ItemConfig::checkName()
{
    if (! validName(this->name, "build item names"))
    {
	QTC::TC("abuild", "ItemConfig ERR build item invalid name");
    }
}

void
ItemConfig::checkParent()
{
    if (Util::isAbsolutePath(this->parent))
    {
	QTC::TC("abuild", "ItemConfig ERR absolute parent");
	this->error.error(this->location,
			  "\"" + k_PARENT + "\" must be a relative path");
    }
}

void
ItemConfig::checkChildren()
{
    this->children = Util::splitBySpace(this->kv.getVal(k_CHILDREN));
    if (checkRelativePaths(this->children, k_CHILDREN + " entries"))
    {
	QTC::TC("abuild", "ItemConfig ERR absolute child");
    }
}

void
ItemConfig::checkBuildAlso()
{
    boost::regex item_name_re(ITEM_NAME_RE);
    boost::smatch match;

    this->build_also = Util::splitBySpace(this->kv.getVal(k_BUILD_ALSO));

    std::list<std::string>::iterator iter = this->build_also.begin();
    while (iter != this->build_also.end())
    {
	std::list<std::string>::iterator next = iter;
	++next;
	if (! boost::regex_match(*iter, match, item_name_re))
	{
	    QTC::TC("abuild", "ItemConfig ERR bad build-also");
	    this->error.error(this->location,
			      "invalid " + k_BUILD_ALSO +
			      " build item name " + *iter);
	    this->build_also.erase(iter, next);
	}
	iter = next;
    }
}

void
ItemConfig::checkDeps()
{
    boost::regex flag_re("-flag=(\\S+)");
    boost::regex platform_re("-platform=(\\S+)");
    boost::regex item_name_re(ITEM_NAME_RE);
    boost::smatch match;

    this->deps = Util::splitBySpace(this->kv.getVal(k_DEPS));

    std::string last_dep;
    std::list<std::string>::iterator iter = this->deps.begin();
    while (iter != this->deps.end())
    {
	std::list<std::string>::iterator next = iter;
	++next;
	if (boost::regex_match(*iter, match, item_name_re))
	{
	    last_dep = *iter;
	}
	else
	{
	    if (boost::regex_match(*iter, match, flag_re))
	    {
		if (last_dep.empty())
		{
		    QTC::TC("abuild", "ItemConfig ERR flag without dep");
		    this->error.error(this->location, "flag " + *iter +
				      " is not preceded by a build item name");
		}
		else
		{
		    this->flag_data.addFlag(last_dep, match.str(1));
		}
	    }
	    else if (boost::regex_match(*iter, match, platform_re))
	    {
		if (last_dep.empty())
		{
		    QTC::TC("abuild", "ItemConfig ERR ptype without dep");
		    this->error.error(this->location,
				      "platform declaration " + *iter +
				      " is not preceded by a build item name");
		}
		else
		{
		    if (this->dep_platform_types.count(last_dep) != 0)
		    {
			QTC::TC("abuild", "ItemConfig ERR duplicate dep ptype");
			this->error.error(this->location,
					  "a platform has already"
					  " been declared for dependency " +
					  last_dep);
		    }
		    std::string dep_platform_type = match.str(1);
		    PlatformSelector p;
		    if (dep_platform_type.find(':') != std::string::npos)
		    {
			if (p.initialize(dep_platform_type) &&
			    (! p.isSkip()) &&
			    (p.getPlatformType() != PlatformSelector::ANY))
			{
			    QTC::TC("abuild", "ItemConfig ptype is selector");
			    this->dep_platform_selectors[last_dep].reset(
				new PlatformSelector(p));
			}
			else
			{
			    QTC::TC("abuild", "ItemConfig bad ptype selector");
			    this->error.error(this->location,
					      "invalid platform selector " +
					      dep_platform_type);
			}
		    }
		    this->dep_platform_types[last_dep] = dep_platform_type;
		}
	    }
	    else
	    {
		QTC::TC("abuild", "ItemConfig ERR bad dep");
		this->error.error(this->location,
				  "invalid dependency " + *iter);
	    }
	    this->deps.erase(iter, next);
	}
	iter = next;
    }
}

void
ItemConfig::checkVisibleTo()
{
    this->visible_to = this->kv.getVal(k_VISIBLE_TO);
    if (this->visible_to.empty())
    {
	return;
    }

    bool okay = true;

    std::list<std::string> name_components = Util::split('.', this->name);
    if (name_components.size() > 1)
    {
	std::list<std::string> visible_to_components =
	    Util::split('.', this->visible_to);
	if (visible_to_components.back() != "*")
	{
	    QTC::TC("abuild", "ItemConfig ERR invalid visible-to");
	    okay = false;
	    this->error.error(this->location,
			      k_VISIBLE_TO + " value must be * or end with .*");
	}
	else
	{
	    visible_to_components.pop_back();
	    bool okay = true;
	    if (visible_to_components.size() > (name_components.size() - 2))
	    {
		QTC::TC("abuild", "ItemConfig ERR visible-to too long");
		okay = false;
	    }
	    else if (! visible_to_components.empty())
	    {
		std::string scope =
		    Util::join(".", visible_to_components) + ".";
		if (scope != this->name.substr(0, scope.length()))
		{
		    QTC::TC("abuild", "ItemConfig ERR visible-to not ancestor");
		    okay = false;
		}
	    }
	    if (! okay)
	    {
		this->error.error(this->location,
				  k_VISIBLE_TO +
				  " value must be * or an ancestor scope "
				  "higher than this item's default scope");
	    }
	}
    }
    else
    {
	QTC::TC("abuild", "ItemConfig ERR visible-to for public item");
	okay = false;
	this->error.error(this->location,
			  k_VISIBLE_TO +
			  " not permitted for public build items");
    }

    // If there are errors, clear visible_to so that later code can
    // assume a valid value.
    if (! okay)
    {
	this->visible_to.clear();
    }
}

void
ItemConfig::checkBuildfile()
{
    int count = 0;
    maybeSetBuildFile(FILE_MK, count);
    maybeSetBuildFile(FILE_ANT, count);
    maybeSetBuildFile(FILE_ANT_BUILD, count);
    maybeSetBuildFile(FILE_GROOVY, count);

    if (count > 1)
    {
	QTC::TC("abuild", "ItemConfig ERR multiple build files");
	this->error.error(this->location,
			  "more than one build file exists");
    }
}

void
ItemConfig::checkPlatforms()
{
    std::list<std::string> o_platform_types =
	Util::splitBySpace(this->kv.getVal(k_PLATFORM));

    bool any_platform_files =
	(Util::fileExists(dir + "/" + FILE_INTERFACE) ||
	 (! this->buildfile.empty()));

    bool platform_types_empty = o_platform_types.empty();

    if (any_platform_files)
    {
	if (this->name.empty())
	{
	    QTC::TC("abuild", "ItemConfig ERR config files without this");
	    this->error.error(this->location,
			      "build and interface files are not "
			      "permitted for unnamed build items");
	}
	else if (platform_types_empty)
	{
	    QTC::TC("abuild", "ItemConfig ERR no platform types");
	    this->error.error(this->location, "\"" + k_PLATFORM +
			      "\" is mandatory if build or interfaces files"
			      " are present");
	}
    }
    else
    {
	if (! platform_types_empty)
	{
	    QTC::TC("abuild", "ItemConfig ERR platforms without build files");
	    this->error.error(this->location, "\"" + k_PLATFORM +
			      "\" may not appear if there are no build or"
			      " interface files");
	}
    }

    if (checkDuplicates(o_platform_types, this->platform_types, "platform type"))
    {
	QTC::TC("abuild", "ItemConfig ERR duplicate platform type");
    }
}

void
ItemConfig::checkExternals()
{
    // For now, don't provide any mechanism to allow spaces in
    // winpath.  Even if we did, it probably wouldn't work right with
    // make.  People can always use the short forms of the paths.
    boost::regex winpath_re("-winpath=(\\S+)");
    boost::smatch match;

    std::list<std::string> words =
	Util::splitBySpace(this->kv.getVal(k_EXTERNAL));
    for (std::list<std::string>::iterator iter = words.begin();
	 iter != words.end(); ++iter)
    {
	std::string const& word = *iter;
	if (word == "-ro")
	{
	    if (this->externals.empty())
	    {
		QTC::TC("abuild", "ItemConfig ERR ro without external");
		this->error.error(this->location, "-ro is not preceded by"
				  " external directory");
	    }
	    else
	    {
		QTC::TC("abuild", "ItemConfig read-only external");
		ExternalData data = this->externals.back();
		this->externals.pop_back();
		this->externals.push_back(
		    ExternalData(data.getDeclaredPath(), true));
	    }
	}
	else if (boost::regex_match(word, match, winpath_re))
	{
	    if (this->externals.empty())
	    {
		QTC::TC("abuild", "ItemConfig ERR winpath without external");
		this->error.error(this->location, "-winpath is not preceded by"
				  " external directory");
	    }
	    else
	    {
		QTC::TC("abuild", "ItemConfig winpath");
#ifdef _WIN32
		ExternalData data = this->externals.back();
		this->externals.pop_back();
		this->externals.push_back(
		    ExternalData(match.str(1), data.isReadOnly()));
#endif
	    }
	}
	else
	{
	    this->externals.push_back(ExternalData(*iter, false));
	}
    }
}

void
ItemConfig::checkSupportedFlags()
{
    boost::smatch match;

    std::list<std::string> o_supported_flags =
	Util::splitBySpace(this->kv.getVal(k_SUPPORTED_FLAGS));
    if (checkDuplicates(o_supported_flags, this->supported_flags, "flag"))
    {
	QTC::TC("abuild", "ItemConfig ERR duplicate flag");
    }
    if (filterInvalidNames(this->supported_flags, "flag names"))
    {
	QTC::TC("abuild", "ItemConfig ERR invalid flag");
    }

    for (std::set<std::string>::iterator iter = this->supported_flags.begin();
	 iter != this->supported_flags.end(); ++iter)
    {
	// Turn on all supported flags locally
	this->flag_data.addFlag(this->name, *iter);
    }
}

void
ItemConfig::checkSupportedTraits()
{
    std::list<std::string> o_supported_traits =
	Util::splitBySpace(this->kv.getVal(k_SUPPORTED_TRAITS));

    if (checkDuplicates(o_supported_traits, this->supported_traits, "trait"))
    {
	QTC::TC("abuild", "ItemConfig ERR duplicate trait");
    }
    if (filterInvalidNames(this->supported_traits, "trait names"))
    {
	QTC::TC("abuild", "ItemConfig ERR invalid trait");
    }
}

void
ItemConfig::checkTraits()
{
    boost::regex item_re("-item=(\\S+)");
    boost::regex item_name_re(ITEM_NAME_RE);
    boost::smatch match;

    std::list<std::string> o_traits =
	Util::splitBySpace(this->kv.getVal(k_TRAITS));

    std::string last_trait;
    for (std::list<std::string>::const_iterator iter = o_traits.begin();
	 iter != o_traits.end(); ++iter)
    {
	if (boost::regex_match(*iter, match, item_name_re))
	{
	    last_trait = *iter;
	    this->trait_data.addTrait(last_trait);
	}
	else if (boost::regex_match(*iter, match, item_re))
	{
	    if (last_trait.empty())
	    {
		QTC::TC("abuild", "ItemConfig ERR item without trait");
		this->error.error(this->location, "item " + *iter +
				  " is not preceded by a trait name");
	    }
	    else
	    {
		this->trait_data.addTraitItem(last_trait, match.str(1));
	    }
	}
	else
	{
	    QTC::TC("abuild", "ItemConfig ERR bad trait");
	    this->error.error(this->location, "invalid trait " + *iter);
	}
    }
}

void
ItemConfig::checkDeleted()
{
    std::list<std::string> o_deleted =
	Util::splitBySpace(this->kv.getVal(k_DELETED));
    if (checkDuplicates(o_deleted, this->deleted, "deleted item"))
    {
	QTC::TC("abuild", "ItemConfig ERR duplicate deleted item");
    }
}

void
ItemConfig::checkPlugins()
{
    this->plugins = Util::splitBySpace(this->kv.getVal(k_PLUGINS));
    std::set<std::string> s_plugins;
    if (checkDuplicates(this->plugins, s_plugins, "plugin"))
    {
	QTC::TC("abuild", "ItemConfig ERR duplicate plugins item");
    }
}

bool
ItemConfig::validName(std::string const& val, std::string const& description)
{
    boost::regex item_name_re(ITEM_NAME_RE);
    boost::smatch match;

    if (! boost::regex_match(val, match, item_name_re))
    {
	this->error.error(this->location,
			  description + " may contain only alphanumeric"
			  " characters, underscores, periods, and dashes");
	return false;
    }
    return true;
}

bool
ItemConfig::filterInvalidNames(std::set<std::string>& names,
			       std::string const& description)
{
    bool error_found = false;
    std::set<std::string> to_remove;
    for (std::set<std::string>::iterator iter = names.begin();
	 iter != names.end(); ++iter)
    {
	if (! validName(*iter, description))
	{
	    error_found = true;
	    to_remove.insert(*iter);
	}
    }
    for (std::set<std::string>::iterator iter = to_remove.begin();
	 iter != to_remove.end(); ++iter)
    {
	names.erase(*iter);
    }
    return error_found;
}

bool
ItemConfig::checkEmptyKey(std::string const& key, std::string const& msg)
{
    if (! this->kv.getVal(key).empty())
    {
	this->error.error(this->location, "\"" + key + "\" " + msg);
	return true;
    }
    return false;
}

bool
ItemConfig::checkDuplicates(std::list<std::string> const& declared,
			    std::set<std::string>& filtered,
			    std::string const& thing)
{
    bool error_found = false;
    for (std::list<std::string>::const_iterator iter = declared.begin();
	 iter != declared.end(); ++iter)
    {
	if (filtered.count(*iter))
	{
	    error_found = true;
	    this->error.error(this->location,
			      thing + " \"" + *iter +
			      "\" listed more than once");
	}
	else
	{
	    filtered.insert(*iter);
	}
    }
    return error_found;
}

bool
ItemConfig::checkRelativePaths(std::list<std::string>& paths,
			       std::string const& description)
{
    bool error_found = false;
    std::list<std::string>::iterator iter = paths.begin();
    while (iter != paths.end())
    {
	std::list<std::string>::iterator next = iter;
	++next;
	if (Util::isAbsolutePath(*iter))
	{
	    error_found = true;
	    this->error.error(this->location,
			      description + " must be relative paths");
	    paths.erase(iter, next);
	}
	iter = next;
    }
    return error_found;
}

void
ItemConfig::maybeSetBuildFile(std::string const& file, int& count)
{
    if (Util::fileExists(dir + "/" + file))
    {
	if (this->buildfile.empty())
	{
	    this->buildfile = file;
	}
	++count;
    }
}

ItemConfig::ItemConfig(
    Error& error,
    CompatLevel const& compat_level,
    FileLocation const& location,
    KeyVal const& kv,
    std::string const& dir)
    :
    error(error),
    compat_level(compat_level),
    location(location),
    kv(kv),
    dir(dir)
{
}

std::string const&
ItemConfig::getName() const
{
    return this->name;
}

std::string const&
ItemConfig::getDescription() const
{
    return this->description;
}

std::string const&
ItemConfig::getParent() const
{
    return this->parent;
}

std::list<std::string> const&
ItemConfig::getChildren() const
{
    return this->children;
}

std::list<ExternalData> const&
ItemConfig::getExternals() const
{
    return this->externals;
}

std::list<std::string> const&
ItemConfig::getBuildAlso() const
{
    return this->build_also;
}

std::list<std::string> const&
ItemConfig::getDeps() const
{
    return this->deps;
}

std::string const&
ItemConfig::getDepPlatformType(std::string const& dep,
			       PlatformSelector const*& selector) const
{
    static std::string none;

    selector = 0;
    if (this->dep_platform_types.count(dep) != 0)
    {
	if (this->dep_platform_selectors.count(dep) != 0)
	{
	    // may be null
	    selector = (*this->dep_platform_selectors.find(dep)).second.get();
	}
	return (*this->dep_platform_types.find(dep)).second;
    }
    return none;
}

FlagData const&
ItemConfig::getFlagData() const
{
    return this->flag_data;
}

TraitData const&
ItemConfig::getTraitData() const
{
    return this->trait_data;
}

std::string const&
ItemConfig::getVisibleTo() const
{
    return this->visible_to;
}

bool
ItemConfig::hasBuildFile() const
{
    return (! this->buildfile.empty());
}

std::set<std::string> const&
ItemConfig::getPlatformTypes() const
{
    return this->platform_types;
}

FileLocation const&
ItemConfig::getLocation() const
{
    return this->location;
}

bool
ItemConfig::supportsFlag(std::string const& flag) const
{
    return (this->supported_flags.count(flag) != 0);
}

std::set<std::string> const&
ItemConfig::getSupportedFlags() const
{
    return this->supported_flags;
}

std::set<std::string> const&
ItemConfig::getSupportedTraits() const
{
    return this->supported_traits;
}

std::set<std::string> const&
ItemConfig::getDeleted() const
{
    return this->deleted;
}

ItemConfig::Backend
ItemConfig::getBackend() const
{
    Backend result = b_none;
    if (! this->buildfile.empty())
    {
	if (this->buildfile == FILE_MK)
	{
	    result = b_make;
	}
	else if (this->buildfile == FILE_GROOVY)
	{
	    result = b_groovy;
	}
	else
	{
	    result = b_ant;
	}
    }
    return result;
}

bool
ItemConfig::hasAntBuild() const
{
    return (this->buildfile == FILE_ANT_BUILD);
}

std::string const&
ItemConfig::getBuildFile() const
{
    return this->buildfile;
}

std::list<std::string> const&
ItemConfig::getPlugins() const
{
    return this->plugins;
}
