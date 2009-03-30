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
std::string const UpgradeData::PLACEHOLDER = "***";

UpgradeData::UpgradeData(Error& error) :
    upgrade_required(false),
    error(error)
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
	st_top, st_ignored_dirs, st_names
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
	else if (state == st_names)
	{
	    if (boost::regex_match(line, match, treename_re))
	    {
		std::string path = match.str(1);
		std::string name = match.str(2);
		// Any trees with place holder names are dispensible
		// because the user has not put any effort into naming
		// them.
		if (name != PLACEHOLDER)
		{
		    if (Util::isDirectory(path))
		    {
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
		    else
		    {
			QTC::TC("abuild", "UpgradeData ERR tree not directory");
			this->error.error(
			    location, "path \"" + path +
			    "\" is not a directory");
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
UpgradeData::writeUpgradeData() const
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

    std::map<std::string, std::string> names = this->tree_names;
    for (std::map<std::string, std::list<std::string> >::const_iterator i1 =
	     this->forest_contents.begin();
	 i1 != this->forest_contents.end(); ++i1)
    {
	of << std::endl;
	of << "[forest]" << std::endl;
	std::string const& root = (*i1).first;
	of << "# root: " << root << std::endl;
	std::list<std::string> const& trees = (*i1).second;
	for (std::list<std::string>::const_iterator i2 = trees.begin();
	     i2 != trees.end(); ++i2)
	{
	    std::string const& path = *i2;
	    if (this->unnamed_trees.count(path))
	    {
		continue;
	    }
	    std::string name = PLACEHOLDER;
	    if (names.count(path))
	    {
		name = names[path];
		names.erase(path);
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
