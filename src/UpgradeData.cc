#include <UpgradeData.hh>

#include <Util.hh>
#include <ItemConfig.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <CompatLevel.hh>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <algorithm>

std::string const UpgradeData::FILE_UPGRADE_DATA = "abuild.upgrade-data";

UpgradeData::UpgradeData(Error& error) :
    error(error),
    upgrade_required(false)
{
    readUpgradeData();
}

void
UpgradeData::readUpgradeData()
{
    if (! Util::isFile(FILE_UPGRADE_DATA))
    {
	return;
    }

    std::list<std::string> lines = Util::readLinesFromFile(FILE_UPGRADE_DATA);

    enum {
	st_top, st_ignored_dirs, st_do_not_upgrade, st_names
    } state = st_top;

    boost::regex trim_re("\\s*(.*?)\\s*");
    boost::regex section_re("\\[(\\S+?)\\]");
    boost::regex treename_re("([^:\\s]+)\\s*:\\s*(\\S+)");
    boost::smatch match;

    int lineno = 0;
    for (std::list<std::string>::iterator iter = lines.begin();
	 iter != lines.end(); ++iter)
    {
	++lineno;
	FileLocation location(FILE_UPGRADE_DATA, lineno, 0);
	assert(boost::regex_match(*iter, match, trim_re));
	std::string line = match.str(1);

	if (line.empty() || (line[0] == '#'))
	{
	    continue;
	}

	if (boost::regex_match(line, match, section_re))
	{
	    std::string section_name = match.str(1);
	    if (section_name == "ignored-directories")
	    {
		state = st_ignored_dirs;
	    }
	    else if (section_name == "do-not-upgrade")
	    {
		state = st_do_not_upgrade;
	    }
	    else if ((section_name == "forest") ||
		     (section_name == "orphan-trees"))
	    {
		state = st_names;
	    }
	    else
	    {
		QTC::TC("abuild", "UpgradeData ERR unknown section");
		this->error.error(location, "unknown section " +
				  section_name);
	    }
	}
	else if (state == st_ignored_dirs)
	{
	    if (Util::isDirectory(line))
	    {
		this->ignored_directories.insert(
		    Util::canonicalizePath(line));
	    }
	    else
	    {
		QTC::TC("abuild", "UpgradeData ERR ignored not directory");
		this->error.error(
		    location, "path \"" + line + "\" is not a directory");
	    }
	}
	else if (state == st_do_not_upgrade)
	{
	    if (Util::isDirectory(line))
	    {
		this->do_not_upgrade.insert(
		    Util::canonicalizePath(line));
	    }
	    else
	    {
		QTC::TC("abuild", "UpgradeData ERR no upgrade not directory");
		this->error.error(
		    location, "path \"" + line + "\" is not a directory");
	    }
	}
	else if (state == st_names)
	{
	    if (boost::regex_match(line, match, treename_re))
	    {
		std::string path = match.str(1);
		std::string name = match.str(2);
		if (name != "***")
		{
		    // Don't check to make sure path is a directory.  When
		    // an external is resolved to a backing area, we
		    // generate an entry to where that external would be
		    // if it were local.
		    if (this->tree_names.count(path))
		    {
			QTC::TC("abuild", "UpgradeData ERR duplicate");
			this->error.error(location, "duplicate name for"
					  " path \"" + path + "\"");
		    }
		    else
		    {
			this->tree_names[path] = name;
		    }
		}
	    }
	    else
	    {
		QTC::TC("abuild", "UpgradeData ERR invalid treename");
	    }
	}
	else
	{
	    QTC::TC("abuild", "UpgradeData ERR expected section");
	    this->error.error(location, "expected section marker");
	}
    }
}

void
UpgradeData::scan()
{
    // XXX This code is broken for build trees with only backing and
    // not conf.

    CompatLevel cl(CompatLevel::cl_1_0);
    std::list<std::string> dirs;
    dirs.push_back(".");
    while (! dirs.empty())
    {
	std::string dir = dirs.front();
	dirs.pop_front();

	if (this->ignored_directories.count(Util::canonicalizePath(dir)))
	{
	    continue;
	}

	if (Util::isFile(dir + "/" + ItemConfig::FILE_CONF))
	{
	    ItemConfig* config = ItemConfig::readConfig(
		this->error, cl, dir, "");
	    this->item_dirs[dir] = config->isTreeRoot();
	    if (config->usesDeprecatedFeatures())
	    {
		this->upgrade_required = true;
	    }
	}
	else if (Util::isFile(dir + "/" + BackingFile::FILE_BACKING))
	{
	    QTC::TC("abuild", "UpgradeData backing without conf");
	    std::list<std::string> backing_chain =
		BackingFile::getBackingChain(this->error, cl, dir);

	    // XXX more or less duplicated with code in
	    // Abuild-upgrade.cc....search for "resolved".  Also
	    // doesn't completely give us what we need, which is the
	    // name of the tree in the backing area, if any.

	    std::string resolved;
	    for (std::list<std::string>::iterator biter =
		     backing_chain.begin();
		 biter != backing_chain.end(); ++biter)
	    {
		std::string candidate = *biter;
		if (Util::isFile(candidate + "/" + ItemConfig::FILE_CONF))
		{
		    resolved = candidate;
		    break;
		}
	    }
	    if (! resolved.empty())
	    {
		// XXX but how are we going to get the config?
		QTC::TC("abuild", "UpgradeData got root from backing");
		this->item_dirs[dir] = true;
	    }
	}

	std::vector<std::string> entries = Util::getDirEntries(dir);
	std::sort(entries.begin(), entries.end());
	for (std::vector<std::string>::iterator iter = entries.begin();
	     iter != entries.end(); ++iter)
	{
	    std::string const& entry = *iter;
	    if ((entry == ".") || (entry == ".."))
	    {
		continue;
	    }
	    std::string fullpath;
	    if (dir != ".")
	    {
		fullpath += dir + "/";
	    }
	    fullpath += entry;
	    if (Util::isDirectory(fullpath))
	    {
		dirs.push_back(fullpath);
	    }
	}
    }
}

void
UpgradeData::writeUpgradeData(
    std::vector<std::list<std::string> > const& forests) const
{
    std::string newfile = FILE_UPGRADE_DATA + ".new";
    std::ofstream of(newfile.c_str(),
		     std::ios_base::out |
		     std::ios_base::trunc);
    if (! of.is_open())
    {
	throw QEXC::System("create " + newfile, errno);
    }

    of << "[ignored-directories]" << std::endl;
    of << "# Place one directory to ignore while scanning on each line."
       << std::endl;
    for (std::set<std::string>::const_iterator iter =
	     this->ignored_directories.begin();
	 iter != this->ignored_directories.end(); ++iter)
    {
	of << Util::absToRel(*iter) << std::endl;
    }

    of << std::endl;

    of << "[do-not-upgrade]" << std::endl;
    of << "# List valid external-dirs that are not being converted here."
       << std::endl
       << "# Each entry here must be in some tree's external-dirs list"
       << " and must not" << std::endl
       << "# be found during the scan because it's either under an "
       << "ignored location" << std::endl
       << "# or not somewhere under the current directory." << std::endl;
    for (std::set<std::string>::const_iterator iter =
	     this->do_not_upgrade.begin();
	 iter != this->do_not_upgrade.end(); ++iter)
    {
	of << Util::absToRel(*iter) << std::endl;
    }

    std::map<std::string, std::string> names = this->tree_names;
    for (std::vector<std::list<std::string> >::const_iterator i1 =
	     forests.begin();
	 i1 != forests.end(); ++i1)
    {
	of << std::endl;
	of << "[forest]" << std::endl;
	std::list<std::string> const& trees = *i1;
	for (std::list<std::string>::const_iterator i2 = trees.begin();
	     i2 != trees.end(); ++i2)
	{
	    std::string const& path = *i2;
	    std::string name = "***";
	    if (names.count(path))
	    {
		name = names[path];
		names.erase(name);
	    }
	    of << path << ": " << name << std::endl;
	}
    }

    if (! names.empty())
    {
	QTC::TC("abuild", "UpgradeData orphan names");
	of << std::endl;
	of << "[orphan-trees]" << std::endl;
	of << "# The following trees were previously assigned names but"
	   << " no longer appear" << std::endl
	   << "# to exist.  If you don't need them anymore, you may"
	   << " remove them" << std::endl
	   << "# from thsi file." << std::endl;
	for (std::map<std::string, std::string>::iterator iter = names.begin();
	     iter != names.end(); ++iter)
	{
	    of << (*iter).first << ": " << (*iter).second << std::endl;
	}
    }
    of.close();
    if (Util::isFile(FILE_UPGRADE_DATA))
    {
	boost::filesystem::remove(FILE_UPGRADE_DATA);
    }
    boost::filesystem::rename(newfile, FILE_UPGRADE_DATA);
}

std::map<std::string, bool> const&
UpgradeData::getItemDirs() const
{
    return this->item_dirs;
}

bool
UpgradeData::upgradeRequired() const
{
    return this->upgrade_required;
}
