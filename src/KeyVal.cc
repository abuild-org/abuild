#include <KeyVal.hh>

#include <assert.h>
#include <iostream>
#include <string>
#include <boost/regex.hpp>
#include <QEXC.hh>
#include <QTC.hh>
#include <Util.hh>
#include <FileLocation.hh>

KeyVal::KeyVal(char const* filename,
	       std::set<std::string> const& keys,
	       std::map<std::string, std::string> const& defaults) :
    error("KeyVal"),
    filename(filename),
    keys(keys),
    defaults(defaults)
{
    // Make sure that there are no duplicates between keys and
    // defaults, and add all the values in defaults to keys.

    bool duplicates = false;
    for (std::map<std::string, std::string>::const_iterator iter =
	     defaults.begin();
	 iter != defaults.end(); ++iter)
    {
	if (keys.count((*iter).first))
	{
	    duplicates = true;
	}
	else
	{
	    this->keys.insert((*iter).first);
	}
    }
    assert(! duplicates);
}

bool
KeyVal::readFile()
{
    static boost::regex comment_line("\\s*#.*");
    static boost::regex strip_spaces("\\s*(.*?)\\s*");
    static boost::regex kv_line("([^\\s:]+)\\s*:\\s*(.*?)");
    boost::smatch match;

    std::list<std::string> lines = Util::readLinesFromFile(this->filename);
    std::string line;
    int lineno = 0;
    for (std::list<std::string>::iterator iter = lines.begin();
	 iter != lines.end(); ++iter)
    {
	FileLocation location(this->filename, ++lineno, 0);

	// Ignore comments even if they occur after a continuation
	// line.
	if (boost::regex_match(*iter, match, comment_line))
	{
	    QTC::TC("abuild", "KeyVal ignore comment");
	    continue;
	}

	// Strip leading and trailing spaces
	assert(boost::regex_match(*iter, match, strip_spaces));
	line += match[1].str();

	if (line.empty())
	{
	    QTC::TC("abuild", "KeyVal ignore blank line");
	    continue;
	}

	// If line ends with a continuation character, replace with a
	// space and ignore until we get a line that doesn't.
	if (*(line.rbegin()) == '\\')
	{
	    line[line.length() - 1] = ' ';
	    continue;
	}

	if (boost::regex_match(line, match, kv_line))
	{
	    std::string key = match[1].str();
	    std::string value = match[2].str();
	    if (this->data.count(key))
	    {
		QTC::TC("abuild", "KeyVal ERR duplicate key");
		this->error.error(location, "duplicate value for " + key +
				  " (using new value)");
	    }
	    if (! this->keys.count(key))
	    {
		QTC::TC("abuild", "KeyVal ERR invalid key");
		this->error.error(location, "invalid key " + key +
				  " (ignored)");
	    }
	    else
	    {
		this->data[key] = value;
	    }
	}
	else
	{
	    QTC::TC("abuild", "KeyVal ERR syntax error");
	    this->error.error(location, "syntax error");
	}

	line.clear();
    }

    if (! line.empty())
    {
	QTC::TC("abuild", "KeyVal ERR end with continuation line");
	this->error.error(FileLocation(this->filename, 0, 0),
			  "file ended with continuation line");
    }

    std::set<std::string> missing_keys;
    for (std::set<std::string>::iterator iter = this->keys.begin();
	 iter != this->keys.end(); ++iter)
    {
	std::string const& key = *iter;
	if (! this->data.count(key))
	{
	    if (defaults.count(key))
	    {
		QTC::TC("abuild", "KeyVal using default");
		this->data[key] = this->defaults[key];
	    }
	    else
	    {
		missing_keys.insert(key);
	    }
	}
    }

    if (! missing_keys.empty())
    {
	std::string message = "no values for the following keys: " +
	    Util::join(" ", missing_keys);
	QTC::TC("abuild", "KeyVal ERR missing keys");
	this->error.error(FileLocation(this->filename, 0, 0), message);
    }

    return (this->error.numErrors() == 0);
}

std::set<std::string>
KeyVal::getKeys() const
{
    return this->keys;
}

std::string const&
KeyVal::getVal(std::string const& key) const
{
    assert(this->data.count(key));
    return (*(this->data.find(key))).second;
}

std::string const&
KeyVal::getVal(std::string const& key,
	       std::string const& fallback_value) const
{
    if (this->data.count(key))
    {
	return getVal(key);
    }
    else
    {
	return fallback_value;
    }
}
