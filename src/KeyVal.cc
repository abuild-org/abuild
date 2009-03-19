#include <KeyVal.hh>

#include <assert.h>
#include <iostream>
#include <fstream>
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
    boost::regex comment_line("\\s*#.*");
    boost::regex kv_line("(\\s*)([^\\s:]+)(\\s*:\\s*(.*?)(\\\\?)\\s*)");
    boost::regex content_re("\\s*(.*?)(\\\\?)\\s*");
    boost::smatch match;

    std::list<std::string> lines =
	Util::readLinesFromFile(this->filename, false);
    int lineno = 0;
    this->orig_data.push_back(OrigData());
    std::string* accumulated_text = &(this->orig_data.back().before);
    std::string* value = 0;
    std::string trash;
    for (std::list<std::string>::iterator iter = lines.begin();
	 iter != lines.end(); ++iter)
    {
	FileLocation location(this->filename, ++lineno, 0);
	std::string orig_line = *iter;

	// Ignore comments even if they occur after a continuation
	// line.
	if (boost::regex_match(orig_line, match, comment_line))
	{
	    QTC::TC("abuild", "KeyVal ignore comment");
	    *accumulated_text += orig_line;
	    continue;
	}

	// Extract content
	assert(boost::regex_match(orig_line, match, content_re));
	std::string content = match[1].str();
	bool continuation = (! match[2].str().empty());
	if (continuation)
	{
	    // If line ends with a continuation character, replace
	    // with a space.
	    content += " ";
	}

	if (value)
	{
	    // Still accumulating text from the current line
	    *accumulated_text += orig_line;
	    *value += content;
	}
	else if (content.empty())
	{
	    QTC::TC("abuild", "KeyVal ignore blank line");
	    *accumulated_text += orig_line;
	}
	else if (boost::regex_match(orig_line, match, kv_line))
	{
	    std::string leading = match[1].str();
	    std::string key = match[2].str();
	    std::string trailing = match[3].str();
	    std::string val_str = match[4].str();
	    std::string continuation = match[5].str();
	    if (! (continuation.empty() || val_str.empty()))
	    {
		val_str += " ";
	    }
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
		trash.clear();
		value = &trash;
	    }
	    else
	    {
		this->data[key] = val_str;
		this->explicit_keys.insert(key);
		value = &(this->data[key]);
	    }
	    assert(accumulated_text == &(this->orig_data.back().before));
	    *accumulated_text += leading;
	    this->orig_data.back().key = key;
	    accumulated_text = &(this->orig_data.back().after);
	    *accumulated_text += trailing;
	}
	else
	{
	    QTC::TC("abuild", "KeyVal ERR syntax error");
	    this->error.error(location, "syntax error");
	    *accumulated_text += orig_line;
	}

	if (! continuation)
	{
	    this->orig_data.push_back(OrigData());
	    accumulated_text = &(this->orig_data.back().before);
	    value = 0;
	}
    }

    if (accumulated_text != &(this->orig_data.back().before))
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

void
KeyVal::writeFile(char const* newfile,
		  std::map<std::string, std::string> const& key_changes) const
{
    std::ofstream of(newfile,
		     std::ios_base::out |
		     std::ios_base::trunc |
		     std::ios_base::binary);
    if (! of.is_open())
    {
	throw QEXC::System(std::string("create ") + newfile, errno);
    }

    for (std::vector<OrigData>::const_iterator iter = this->orig_data.begin();
	 iter != this->orig_data.end(); ++iter)
    {
	OrigData const& od = *iter;
	of << od.before;
	if (! od.key.empty())
	{
	    std::map<std::string, std::string>::const_iterator k =
		key_changes.find(od.key);
	    if (k != key_changes.end())
	    {
		of << (*k).second;
	    }
	    else
	    {
		of << od.key;
	    }
	}
	of << od.after;
    }

    of.close();
}

std::set<std::string>
KeyVal::getKeys() const
{
    return this->keys;
}

std::set<std::string>
KeyVal::getExplicitKeys() const
{
    return this->explicit_keys;
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
