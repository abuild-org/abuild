#include <ItemConfig.hh>

#include <boost/regex.hpp>
#include <QTC.hh>
#include <Error.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <FileLocation.hh>
#include <KeyVal.hh>
#include <CompatLevel.hh>
#include <BackingConfig.hh>

std::string const ItemConfig::FILE_CONF = "Abuild.conf";
std::string const ItemConfig::FILE_MK = "Abuild.mk";
std::string const ItemConfig::FILE_INTERFACE = "Abuild.interface";
std::string const ItemConfig::FILE_ANT = "Abuild-ant.properties";
std::string const ItemConfig::FILE_GROOVY = "Abuild.groovy";
std::string const ItemConfig::FILE_ANT_BUILD = "Abuild-ant.xml";
std::map<std::string, ItemConfig::KeyVal_ptr> ItemConfig::kv_cache;
std::map<std::string, ItemConfig::ItemConfig_ptr> ItemConfig::ic_cache;
std::string const ItemConfig::k_THIS = "this";
std::string const ItemConfig::k_PARENT = "parent-dir";
std::string const ItemConfig::k_EXTERNAL = "external-dirs";
std::string const ItemConfig::k_DELETED = "deleted";
std::string const ItemConfig::k_NAME = "name";
std::string const ItemConfig::k_TREENAME = "tree-name";
std::string const ItemConfig::k_DESCRIPTION = "description";
std::string const ItemConfig::k_CHILDREN = "child-dirs";
std::string const ItemConfig::k_VISIBLE_TO = "visible-to";
std::string const ItemConfig::k_BUILD_ALSO = "build-also";
std::string const ItemConfig::k_DEPS = "deps";
std::string const ItemConfig::k_TREEDEPS = "tree-deps";
std::string const ItemConfig::k_PLATFORM = "platform-types";
std::string const ItemConfig::k_SUPPORTED_FLAGS = "supported-flags";
std::string const ItemConfig::k_SUPPORTED_TRAITS = "supported-traits";
std::string const ItemConfig::k_TRAITS = "traits";
std::string const ItemConfig::k_PLUGINS = "plugins";
std::string const ItemConfig::k_ATTRIBUTES = "attributes";

// The following characters may never appear in build item names:
//
//  * Comma: various places in the code, including ant properties
//    files and name-based build set specification, use
//    comma-separated lists of build item names.
//
//  * Space: make code and Abuild.conf parsing use space-separated
//    lists of build items
//
//  * Exclamation point: a single exclamation point is used to
//    separate the build item from the platform in the build graph
//    nodes
//
// Carefully consider the implications of adding new legal characters
// to the names of build items.  As currently specified, build item
// names are always valid file names and can always be specified on
// the command line without any special quoting.  These are compelling
// features.
std::string const ItemConfig::ITEM_NAME_RE =
    "[a-zA-Z0-9_][a-zA-Z0-9_-]*(?:\\.[a-zA-Z0-9_-]+)*";

std::string const ItemConfig::PARENT_RE = "\\.\\.(?:/\\.\\.)*";

std::map<std::string, std::string> ItemConfig::valid_keys;
bool ItemConfig::statics_initialized = false;

void ItemConfig::initializeStatics(CompatLevel const& compat_level)
{
    if (statics_initialized)
    {
	return;
    }

    // 1.0 keys
    if (compat_level.allow_1_0())
    {
	valid_keys[k_THIS] = "";
	valid_keys[k_PARENT] = "";
	valid_keys[k_EXTERNAL] = "";
	valid_keys[k_DELETED] = "";
    }

    valid_keys[k_NAME] = "";
    valid_keys[k_TREENAME] = "";
    valid_keys[k_DESCRIPTION] = "";
    valid_keys[k_CHILDREN] = "";
    valid_keys[k_BUILD_ALSO] = "";
    valid_keys[k_DEPS] = "";
    valid_keys[k_TREEDEPS] = "";
    valid_keys[k_VISIBLE_TO] = "";
    valid_keys[k_PLATFORM] = "";
    valid_keys[k_SUPPORTED_FLAGS] = "";
    valid_keys[k_SUPPORTED_TRAITS] = "";
    valid_keys[k_TRAITS] = "";
    valid_keys[k_PLUGINS] = "";
    valid_keys[k_ATTRIBUTES] = "";

    statics_initialized = true;
}

ItemConfig*
ItemConfig::readConfig(Error& error, CompatLevel const& compat_level,
		       std::string const& odir, std::string const& parent_dir)
{
    std::string dir = Util::canonicalizePath(odir);
    if (ic_cache.count(dir))
    {
	return ic_cache[dir].get();
    }

    initializeStatics(compat_level);
    KeyVal const& kv = readKeyVal(error, compat_level, dir);
    FileLocation location(dir + "/" + FILE_CONF, 0, 0);
    ItemConfig_ptr item;
    item.reset(
	new ItemConfig(error, compat_level, location, kv, dir, parent_dir));
    item->validate();

    // Cache and return
    ic_cache[dir] = item;
    return item.get();
}

KeyVal const&
ItemConfig::readKeyVal(Error& error, CompatLevel const& compat_level,
		       std::string const& dir)
{
    if (kv_cache.count(dir))
    {
	return *(kv_cache[dir]);
    }

    initializeStatics(compat_level);
    std::string file = dir + "/" + FILE_CONF;
    KeyVal_ptr kv(
	new KeyVal(file.c_str(), std::set<std::string>(), valid_keys));

    // An exception is thrown if the file does not exist.  It is the
    // caller's responsibility to ensure that it does.
    kv->readFile();
    kv_cache[dir] = kv;
    return *kv;
}

void
ItemConfig::validate()
{
    findParentDir();
    detectRoot();

    checkDeprecated();
    checkName();
    checkTreeName();
    checkRoot();
    checkParent();
    checkChildren();
    checkBuildAlso();
    checkDeps();
    checkTreeDeps();
    checkVisibleTo();
    checkBuildfile();
    checkPlatforms();
    checkExternals();
    checkSupportedFlags();
    checkSupportedTraits();
    checkTraits();
    checkDeleted();
    checkPlugins();
    checkAttributes();

    // no validation required for description
    this->description = this->kv.getVal(k_DESCRIPTION);
}

void
ItemConfig::findParentDir()
{
    if (! this->parent_dir.empty())
    {
	return;
    }

    std::string parent_candidate = this->dir;

    while (! parent_candidate.empty())
    {
	std::string tmp = Util::dirname(parent_candidate);
	if (tmp == parent_candidate)
	{
	    // We reached the root
	    QTC::TC("abuild", "ItemConfig found root searching for parent");
	    parent_candidate.clear();
	    break;
	}
	else
	{
	    parent_candidate = tmp;
	    if (Util::fileExists(
		    parent_candidate + "/" + ItemConfig::FILE_CONF))
	    {
		break;
	    }
	    else
	    {
		QTC::TC("abuild", "ItemConfig skipping dir in parent search");
	    }
	}
    }

    if (! parent_candidate.empty())
    {
	// See if this item has this directory as a child.
	KeyVal const& kv = readKeyVal(this->error, this->compat_level,
				      parent_candidate);
	bool parent_has_child = false;
	// Just read the keys and parse directly rather than calling
	// readConfig().  This prevents repeatedly recursing all the
	// way up to the root, seeing unwanted errors, etc.
	std::list<std::string> const& children =
	    Util::splitBySpace(kv.getVal(k_CHILDREN));
	for (std::list<std::string>::const_iterator iter = children.begin();
	     iter != children.end(); ++iter)
	{
	    if (Util::canonicalizePath(
		    parent_candidate + "/" + *iter) == dir)
	    {
		parent_has_child = true;
		break;
	    }
	}
	if (parent_has_child)
	{
	    this->parent_dir = parent_candidate;
	}
    }
}

void
ItemConfig::detectRoot()
{
    std::set<std::string> const& ek = this->kv.getExplicitKeys();

    if (ek.count(k_TREENAME))
    {
	// If a file has a tree-name key, it is definitely a 1.1 root.
	// In strict 1.1 mode, that's the only way it can be a root.
	this->is_root = true;
    }
    else if (this->compat_level.allow_1_0())
    {
	// In 1.0 compatibility mode...
	if (ek.count(k_PARENT))
	{
	    // If the file has a parent-dir key, it's definitely not a
	    // root.
	    this->is_root = false;
	}
	else if (! this->parent_dir.empty())
	{
	    // If it is a child of the next higher build item and
	    // doesn't have tree-name, it is not a root.
	    this->is_root = false;
	}
	else
	{
	    this->is_root = true;
	}
    }
    else
    {
	this->is_root = false;
    }

    this->is_child_only = ((ek.size() == 1) && ek.count(k_CHILDREN));

    if (this->parent_dir.empty() && (this->is_root || is_child_only))
    {
	this->is_forest_root = true;
    }
}

void
ItemConfig::checkDeprecated()
{
    // Note: presence of explicit keys doesn't automatically cause
    // deprecated to be set.  Abuild.cc does its own checks for
    // externals that point to upgraded trees.  This helps warning
    // people about deprecated features when there is no possible
    // remedy.

    std::set<std::string> const& ek = this->kv.getExplicitKeys();
    if (this->is_root &&
	(! (this->is_child_only && this->is_forest_root)) &&
	(! ek.count(k_TREENAME)))
    {
	QTC::TC("abuild", "ItemConfig root without tree-name");
	this->deprecated = true;
    }
    if (ek.count(k_THIS))
    {
	QTC::TC("abuild", "ItemConfig has this");
	this->deprecated = true;
    }
    if (ek.count(k_PARENT))
    {
	QTC::TC("abuild", "ItemConfig has parent-dir");
	this->deprecated = true;
    }
    if (ek.count(k_DELETED))
    {
	QTC::TC("abuild", "ItemConfig has deleted");
	this->deprecated = true;
    }

    if (this->deprecated && (! this->compat_level.allow_1_0()))
    {
	throw QEXC::Internal(
	    std::string(this->location) +
	    ": deprecated is true but we're not in 1.0-compatibility mode");
    }
}

void
ItemConfig::checkUnnamed()
{
    std::string msg = "is not permitted for unnamed build items";

    if (checkKeyPresent(k_BUILD_ALSO, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR build-also without name");
    }
    if (checkKeyPresent(k_DEPS, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR deps without name");
    }
    if (checkKeyPresent(k_PLATFORM, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR platform-types without name");
    }
    if (checkKeyPresent(k_SUPPORTED_FLAGS, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR supported-flags without name");
    }
    if (checkKeyPresent(k_TRAITS, msg))
    {
	QTC::TC("abuild", "ItemConfig ERR traits without name");
    }
    if (checkKeyPresent(k_DESCRIPTION, msg))
    {
	// Since we don't store unnamed build items, there's no point
	// in allowing a description.  People can put comments in
	// instead.
	QTC::TC("abuild", "ItemConfig ERR description without name");
    }
}

void
ItemConfig::checkRoot()
{
    bool has_backing_file =
	Util::isFile(this->dir + "/" + BackingConfig::FILE_BACKING);
    if (has_backing_file && (! this->is_forest_root))
    {
	QTC::TC("abuild", "ItemConfig ERR Abuild.backing non-forest root");
	this->error.error(
	    FileLocation(dir + "/" + BackingConfig::FILE_BACKING, 0, 0),
	    BackingConfig::FILE_BACKING +
	    " file ignored for non forest-root build item");
    }
    if (this->compat_level.allow_1_0())
    {
	if (this->is_root && this->is_forest_root &&
	    this->tree_name.empty() && has_backing_file)
	{
	    QTC::TC("abuild", "ItemConfig allowing deprecated deleted");
	}
	else
	{
	    if (checkKeyPresent(k_DELETED,
				"is ignored except in deprecated"
				" 1.0 build tree roots with backing"
				" areas"))
	    {
		QTC::TC("abuild", "ItemConfig ERR deleted on non-1.0-root");
	    }
	}
    }

    if (! this->is_root)
    {
	std::string msg = "may only appear in a root build item";

	if (this->compat_level.allow_1_0())
	{
	    if (checkKeyPresent(k_EXTERNAL, msg))
	    {
		QTC::TC("abuild", "ItemConfig ERR external on non-root");
	    }
	}
	if (checkKeyPresent(k_TREEDEPS, msg))
	{
	    QTC::TC("abuild", "ItemConfig ERR tree-deps on non-root");
	}
	if (checkKeyPresent(k_SUPPORTED_TRAITS, msg))
	{
	    QTC::TC("abuild", "ItemConfig ERR supported-traits on non-root");
	}
	if (checkKeyPresent(k_PLUGINS, msg))
	{
	    QTC::TC("abuild", "ItemConfig ERR plugins on non-root");
	}
    }
}

void
ItemConfig::checkName()
{
    this->name = this->kv.getVal(k_NAME);
    if (this->compat_level.allow_1_0())
    {
	if (this->kv.getExplicitKeys().count(k_THIS))
	{
	    if (this->kv.getExplicitKeys().count(k_NAME))
	    {
		QTC::TC("abuild", "ItemConfig ERR this and name");
		this->error.error(this->location,
				  "\"" + k_THIS + "\" and \"" +
				  k_NAME + "\" may not appear in the same " +
				  FILE_CONF + " file");
	    }
	    this->name = this->kv.getVal(k_THIS);
	}
    }
    if (this->name.empty())
    {
	checkUnnamed();
    }
    else if (! validName(this->name, "build item names"))
    {
	QTC::TC("abuild", "ItemConfig ERR build item invalid name");
    }
}

void
ItemConfig::checkTreeName()
{
    this->tree_name = this->kv.getVal(k_TREENAME);
    if (! this->tree_name.empty())
    {
	if (! validName(this->tree_name, "build tree names"))
	{
	    QTC::TC("abuild", "ItemConfig ERR build tree invalid name");
	}
    }
}

void
ItemConfig::checkParent()
{
    if (! this->compat_level.allow_1_0())
    {
	return;
    }

    std::string parent_key = this->kv.getVal(k_PARENT);
    if (parent_key.empty())
    {
	return;
    }

    Util::stripTrailingSlash(parent_key);
    boost::regex parent_re(PARENT_RE);
    boost::smatch match;
    if (! boost::regex_match(parent_key, match, parent_re))
    {
	QTC::TC("abuild", "ItemConfig ERR invalid parent");
	this->error.error(this->location,
			  "\"" + k_PARENT +
			  "\" must point up in the file system");
    }
    else if (! Util::isFile(this->dir + "/" + parent_key + "/" + FILE_CONF))
    {
	QTC::TC("abuild", "ItemConfig ERR no parent Abuild.conf");
	this->error.error(this->location,
			  "\"" + k_PARENT +
			  "\" points to a directory that does not contain " +
			  FILE_CONF);
    }
    else
    {
	// Make sure there are no intervening Abuild.conf files.  We
	// check parent/child symmetry during traversal.
	std::list<std::string> elements = Util::split('/', parent_key);
	elements.pop_back();
	while (! elements.empty())
	{
	    std::string path = Util::join("/", elements);
	    if (Util::isFile(this->dir + "/" + path + "/" + FILE_CONF))
	    {
		QTC::TC("abuild", "ItemConfig ERR interleaved Abuild.conf");
		this->error.error(
		    this->location,
		    "relative to this item, directory \"" + path + "\","
		    " which is between this item and its parent, contains " +
		    FILE_CONF + "; interleaved " + FILE_CONF + " files are"
		    " not permitted");
	    }
	    elements.pop_back();
	}
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
    std::list<std::string>::iterator iter = this->children.begin();
    while (iter != this->children.end())
    {
	std::list<std::string>::iterator next = iter;
	++next;
	Util::stripTrailingSlash(*iter);
	std::string const& child = *iter;
	std::list<std::string> elements = Util::split('/', child);
	bool error_found = false;
	for (std::list<std::string>::iterator eiter = elements.begin();
	     eiter != elements.end(); ++eiter)
	{
	    if (*eiter == "..")
	    {
		error_found = true;
		break;
	    }
	}
	if (error_found)
	{
	    QTC::TC("abuild", "ItemConfig ERR .. in child");
	    this->error.error(this->location,
			      k_CHILDREN + " entries may not include \"..\"");
	    this->children.erase(iter, next);
	}
	else
	{
	    bool has_symlinks = false;
	    if (hasSymlinks(child))
	    {
		// Coverage check is below
		this->error.error(this->location,
				  "child directory \"" + child + "\" is a"
				  " symbolic link or crosses a symbolic link");
		has_symlinks = true;
	    }
	    if ((! Util::osSupportsSymlinks()) || has_symlinks)
	    {
		QTC::TC("abuild", "ItemConfig ERR child symlink");
	    }

	    // Check for intervening Abuild.conf directories.  No need
	    // to check for existence of child Abuild.conf.  It may
	    // not exist because of backing areas, and we'll check
	    // during traversal.  However, it is never correct for an
	    // intervening file to exist.
	    elements.pop_back();
	    while (! elements.empty())
	    {
		std::string path = Util::join("/", elements);
		if (Util::isFile(this->dir + "/" + path + "/" + FILE_CONF))
		{
		    QTC::TC("abuild", "ItemConfig ERR interleaved child Abuild.conf");
		    this->error.error(
			this->location,
			"relative to this item, directory \"" + path + "\","
			" which is between this item and child \"" + child +
			"\" contains " +
			FILE_CONF + "; interleaved " + FILE_CONF + " files are"
			" not permitted");
		}
		elements.pop_back();
	    }
	}
	iter = next;
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
	    else if (*iter == "-optional")
	    {
		if (last_dep.empty())
		{
		    QTC::TC("abuild", "ItemConfig ERR optional without dep");
		    this->error.error(this->location, "-optional"
				      " is not preceded by a build item name");
		}
		else
		{
		    this->optional_deps.insert(last_dep);
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
ItemConfig::checkTreeDeps()
{
    boost::regex item_name_re(ITEM_NAME_RE);
    boost::smatch match;

    this->tree_deps = Util::splitBySpace(this->kv.getVal(k_TREEDEPS));

    std::string last_dep;
    std::list<std::string>::iterator iter = this->tree_deps.begin();
    while (iter != this->tree_deps.end())
    {
	std::list<std::string>::iterator next = iter;
	++next;
	if (boost::regex_match(*iter, match, item_name_re))
	{
	    last_dep = *iter;
	}
	else
	{
	    if (*iter == "-optional")
	    {
		if (last_dep.empty())
		{
		    QTC::TC("abuild", "ItemConfig ERR optional without treedep");
		    this->error.error(this->location, "-optional"
				      " is not preceded by a build tree name");
		}
		else
		{
		    this->optional_tree_deps.insert(last_dep);
		}
	    }
	    else
	    {
		QTC::TC("abuild", "ItemConfig ERR bad tree dep");
		this->error.error(this->location,
				  "invalid tree dependency " + *iter);
	    }
	    this->tree_deps.erase(iter, next);
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
	    QTC::TC("abuild", "ItemConfig ERR config files without name");
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

    if (checkDuplicates(o_platform_types, this->platform_types,
			"platform type"))
    {
	QTC::TC("abuild", "ItemConfig ERR duplicate platform type");
    }
}

void
ItemConfig::checkExternals()
{
    if (! this->compat_level.allow_1_0())
    {
	return;
    }

    boost::regex winpath_re("-winpath=(\\S+)");
    boost::smatch match;
    bool any_symlinks = false;

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
		this->error.deprecate(
		    "1.1", this->location,
		    "the read-only flag for externals is now ignored;"
		    " use --ro-path and --rw-path from the command-line"
		    " instead");
	    }
	}
	else if (boost::regex_match(word, match, winpath_re))
	{
	    QTC::TC("abuild", "ItemConfig ERR winpath");
	    this->error.error(this->location,
			      "-winpath is no longer supported");
	}
	else if (Util::isAbsolutePath(*iter))
	{
	    QTC::TC("abuild", "ItemConfig ERR absolute external");
	    this->error.error(
		this->location,
		"absolute path externals are no longer supported");
	}
	else if (hasSymlinks(*iter))
	{
	    this->error.error(
		this->location,
		"external directory \"" + *iter + "\" is a symbolic link or"
		" crosses a symbolic link");
	    // Coverage check is below
	    any_symlinks = true;
	}
	else
	{
	    this->externals.push_back(*iter);
	}
    }

    if (any_symlinks)
    {
	this->error.error(
	    this->location,
	    "NOTE: Although external-dirs may no longer traverse symbolic "
	    "links, abuild 1.1 supports use of multiple backing areas, so "
	    "that solution may work for you instead.");
    }

    if ((! Util::osSupportsSymlinks()) || any_symlinks)
    {
	// avoid coverage failure on operating systems without symlinks
	QTC::TC("abuild", "ItemConfig ERR external symlink");
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
    if (! this->compat_level.allow_1_0())
    {
	return;
    }

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

void
ItemConfig::checkAttributes()
{
    std::list<std::string> attrs =
	Util::splitBySpace(this->kv.getVal(k_ATTRIBUTES));
    for (std::list<std::string>::iterator iter = attrs.begin();
	 iter != attrs.end(); ++iter)
    {
	std::string const& attr = *iter;
	if (attr == "global-tree-dep")
	{
	    if (this->tree_name.empty())
	    {
		QTC::TC("abuild", "ItemConfig ERR global-tree-dep non-root");
		this->error.error(this->location,
				  "the \"global-tree-dep\" attribute may only"
				  " be applied to root build items of named"
				  " trees");
	    }
	    else
	    {
		this->attributes.setGlobalTreeDep(true);
	    }
	}
	else if (attr == "global-plugin")
	{
	    if (this->name.empty())
	    {
		QTC::TC("abuild", "ItemConfig ERR global-plugin unnamed");
		this->error.error(this->location,
				  "the \"global-plugin\" attribute may only"
				  " be applied to named build items");
	    }
	    else
	    {
		this->attributes.setGlobalPlugin(true);
	    }
	}
	else
	{
	    QTC::TC("abuild", "ItemConfig ERR invalid attribute");
	    this->error.error(this->location,
			      "unknown attribute \"" + attr + "\"");
	}
    }
}

bool
ItemConfig::isNameValid(std::string const& name)
{
    boost::regex item_name_re(ITEM_NAME_RE);
    boost::smatch match;
    return boost::regex_match(name, match, item_name_re);
}

bool
ItemConfig::validName(std::string const& val, std::string const& description)
{
    if (! isNameValid(val))
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
ItemConfig::checkKeyPresent(std::string const& key, std::string const& msg)
{
    if (this->kv.getExplicitKeys().count(key))
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

bool
ItemConfig::hasSymlinks(std::string const& to_check)
{

    std::string path = this->dir;
    std::list<std::string> elements = Util::split('/', to_check);
    while (! elements.empty())
    {
	path += "/" + elements.front();
	elements.pop_front();
	if (Util::isSymlink(path))
	{
	    return true;
	}
    }

    return false;
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
    std::string const& dir,
    std::string const& parent_dir)
    :
    error(error),
    compat_level(compat_level),
    location(location),
    kv(kv),
    dir(dir),
    parent_dir(parent_dir),
    is_root(false),
    is_forest_root(false),
    is_child_only(false),
    deprecated(false)
{
}

bool
ItemConfig::isTreeRoot() const
{
    return this->is_root;
}

bool
ItemConfig::isForestRoot() const
{
    return this->is_forest_root;
}

bool
ItemConfig::isChildOnly() const
{
    return this->is_child_only;
}

bool
ItemConfig::usesDeprecatedFeatures() const
{
    return this->deprecated;
}

std::string const&
ItemConfig::getAbsolutePath() const
{
    return this->dir;
}

std::string const&
ItemConfig::getParentDir() const
{
    return this->parent_dir;
}

std::string const&
ItemConfig::getName() const
{
    return this->name;
}

std::string const&
ItemConfig::getTreeName() const
{
    return this->tree_name;
}

std::string const&
ItemConfig::getDescription() const
{
    return this->description;
}

std::list<std::string> const&
ItemConfig::getChildren() const
{
    return this->children;
}

std::list<std::string> const&
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

std::list<std::string> const&
ItemConfig::getTreeDeps() const
{
    return this->tree_deps;
}

std::set<std::string> const&
ItemConfig::getOptionalDeps() const
{
    return this->optional_deps;
}

std::set<std::string> const&
ItemConfig::getOptionalTreeDeps() const
{
    return this->optional_tree_deps;
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

bool
ItemConfig::isGlobalTreeDep() const
{
    return this->attributes.getGlobalTreeDep();
}

bool
ItemConfig::isGlobalPlugin() const
{
    return this->attributes.getGlobalPlugin();
}

bool
ItemConfig::upgradeConfig(std::string const& file,
			  std::set<std::string> const& new_children,
			  std::string const& tree_name,
			  std::list<std::string> const& externals,
			  std::list<std::string> const& new_tree_deps)
{
    // 1.0 compatibility only

    std::set<std::string> deletions;
    std::vector<std::string> additions;
    std::map<std::string, std::string> replacements;
    std::map<std::string, std::string> key_changes;

    std::set<std::string> const& ek = this->kv.getExplicitKeys();

    if (ek.count(k_THIS))
    {
	key_changes[k_THIS] = k_NAME;
    }
    if (ek.count(k_PARENT))
    {
	deletions.insert(k_PARENT);
    }
    if (ek.count(k_DELETED))
    {
	deletions.insert(k_DELETED);
    }
    if ((ek.count(k_TREENAME) == 0) && (! tree_name.empty()))
    {
	additions.push_back(k_TREENAME + ": " + tree_name);
    }
    else
    {
	assert(this->tree_name == tree_name);
    }

    std::string sep = " \\" + this->kv.getPreferredEOL() + "    ";

    // add new_tree_deps that aren't already present
    std::set<std::string> seen;
    for (std::list<std::string>::const_iterator iter = this->tree_deps.begin();
	 iter != this->tree_deps.end(); ++iter)
    {
	seen.insert(*iter);
    }
    std::list<std::string> tree_deps_to_add;
    for (std::list<std::string>::const_iterator iter = new_tree_deps.begin();
	 iter != new_tree_deps.end(); ++iter)
    {
	if (! seen.count(*iter))
	{
	    tree_deps_to_add.push_back(*iter);
	}
    }
    if (! tree_deps_to_add.empty())
    {
	std::string tr = k_TREEDEPS + ":" + sep;
	if (! this->tree_deps.empty())
	{
	    tr += Util::join(sep, this->tree_deps) + sep;
	}
	tr += Util::join(sep, tree_deps_to_add);
	if (ek.count(k_TREEDEPS))
	{
	    QTC::TC("abuild", "ItemConfig replace tree_deps");
	    replacements[k_TREEDEPS] = tr;
	}
	else
	{
	    QTC::TC("abuild", "ItemConfig add tree_deps");
	    additions.push_back(tr);
	}
    }

    // add new_children that aren't already present
    seen.clear();
    for (std::list<std::string>::const_iterator iter = this->children.begin();
	 iter != this->children.end(); ++iter)
    {
	seen.insert(Util::canonicalizePath(this->dir + "/" + *iter));
    }
    std::list<std::string> children_to_add;
    for (std::set<std::string>::const_iterator iter = new_children.begin();
	 iter != new_children.end(); ++iter)
    {
	if (! seen.count(Util::canonicalizePath(this->dir + "/" + *iter)))
	{
	    children_to_add.push_back(*iter);
	}
    }
    if (! children_to_add.empty())
    {
	std::string ch = k_CHILDREN + ":" + sep;
	if (! this->children.empty())
	{
	    ch += Util::join(sep, this->children) + sep;
	}
	ch += Util::join(sep, children_to_add);
	if (ek.count(k_CHILDREN))
	{
	    QTC::TC("abuild", "ItemConfig replace children");
	    replacements[k_CHILDREN] = ch;
	}
	else
	{
	    QTC::TC("abuild", "ItemConfig add children");
	    additions.push_back(ch);
	}
    }

    // replace externals
    if (externals.empty())
    {
	if (ek.count(k_EXTERNAL))
	{
	    QTC::TC("abuild", "ItemConfig delete externals");
	    deletions.insert(k_EXTERNAL);
	}
    }
    else
    {
	assert(! this->externals.empty());
	if (new_tree_deps.empty())
	{
	    // We must have preserved all the old externals.
	    QTC::TC("abuild", "ItemConfig preserve externals");
	}
	else
	{
	    QTC::TC("abuild", "ItemConfig replace externals");
	    replacements[k_EXTERNAL] = k_EXTERNAL + ":" + sep +
		Util::join(sep, externals);
	}
    }

    bool must_upgrade =
	(! ((key_changes.empty() &&
	     additions.empty() &&
	     replacements.empty() &&
	     deletions.empty())));

    if (must_upgrade)
    {
	this->kv.writeFile(file.c_str(),
			   key_changes,
			   replacements,
			   deletions,
			   additions);
    }

    return must_upgrade;
}
