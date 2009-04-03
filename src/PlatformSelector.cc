#include "PlatformSelector.hh"

#include <QEXC.hh>
#include <Util.hh>
#include <QTC.hh>
#include <boost/regex.hpp>
#include <assert.h>

std::string const PlatformSelector::ANY = "*";

PlatformSelector::PlatformSelector() :
    skip(false),
    dfault(false)
{
}

PlatformSelector::Matcher::Matcher(std::string const& first_platform,
				   PlatformSelector const& p) :
    p(p)
{
    // A platform selector that is "default" means that you should not
    // use the platform selector to perform matches but instead use
    // the default.  Creating a matcher with a p that is default is a
    // programming error.
    assert(! p.isDefault());

    std::string ignore;
    // The first platform is used to get the defaults for all
    // non-optional fields.  The option field is optional, so an empty
    // option value in the selector should only match a platform with
    // an empty option field.
    PlatformSelector::split_platform(
	first_platform, default_os, default_cpu, default_toolset,
	default_compiler, ignore);
}

bool
PlatformSelector::Matcher::matches(std::string const& platform) const
{
    std::string os;
    std::string cpu;
    std::string toolset;
    std::string compiler;
    std::string option;
    PlatformSelector::split_platform(
	platform, os, cpu, toolset, compiler, option);
    return (fieldsMatch(default_os, p.os, os) &&
	    fieldsMatch(default_cpu, p.cpu, cpu) &&
	    fieldsMatch(default_toolset, p.toolset, toolset) &&
	    fieldsMatch(default_compiler, p.compiler, compiler) &&
	    fieldsMatch(default_option, p.option, option));
}

bool
PlatformSelector::Matcher::fieldsMatch(std::string const& dfault,
				       std::string const& selector,
				       std::string const& platform)
{
    return ((selector == ANY) ||
	    ((selector == "") && (platform == dfault)) ||
	    (selector == platform));
}

bool
PlatformSelector::initialize(std::string const& str)
{
    static std::string component = "(?:[a-zA-Z0-9_-]+|\\*)"; // \* is ANY
    static std::string component1or2 =
	component + "(?:\\." + component + ")?";
    static std::string component4or5 =
	component + "\\." + component + "\\." + component + "\\." +
	component1or2;
    static std::string option_re = "option=(?:" + component + ")?";
    static std::string compiler_re = "compiler=" + component1or2;
    static std::string platform_re = "platform=" + component4or5;
    boost::regex selector_re(
	"(?:([a-zA-Z0-9_-]+):)?" // platform type
	"(all|skip|default|" +
	option_re + "|" + compiler_re + "|" + platform_re + ")");

    boost::smatch match;

    if (! boost::regex_match(str, match, selector_re))
    {
	return false;
    }

    bool okay = true;
    this->platform_type = ANY;
    if (match[1].matched)
    {
	this->platform_type = match[1].str();
    }
    std::string selector = match[2].str();
    if (selector == "skip")
    {
	if (this->platform_type == ANY)
	{
	    okay = false;
	}
	else
	{
	    QTC::TC("abuild", "PlatformSelector skip");
	    this->skip = true;
	}
    }
    else if (selector == "default")
    {
	if (this->platform_type == ANY)
	{
	    okay = false;
	}
	else
	{
	    QTC::TC("abuild", "PlatformSelector default");
	    this->dfault = true;
	}
    }
    else if (selector == "all")
    {
	QTC::TC("abuild", "PlatformSelector all");
	this->os = ANY;
	this->cpu = ANY;
	this->toolset = ANY;
	this->compiler = ANY;
	this->option = ANY;
    }
    else
    {
	std::list<std::string> fields = Util::split('=', selector);
	assert(fields.size() == 2);
	std::string specifier = fields.front();
	std::string pattern = fields.back();
	if (specifier == "option")
	{
	    this->option = pattern;
	    QTC::TC("abuild", "PlatformSelector option",
		    this->option.empty() ? 0 : 1);
	}
	else if (specifier == "compiler")
	{
	    fields = Util::split('.', pattern);
	    unsigned int nfields = fields.size();
	    assert((nfields == 1) || (nfields == 2));
	    this->compiler = fields.front();
	    if (nfields == 2)
	    {
		QTC::TC("abuild", "PlatformSelector compiler 2");
		this->option = fields.back();
	    }
	    else
	    {
		QTC::TC("abuild", "PlatformSelector compiler 1");
	    }
	}
	else if (specifier == "platform")
	{
	    split_platform(pattern, this->os, this->cpu, this->toolset,
			   this->compiler, this->option);

	    if (this->option.empty())
	    {
		QTC::TC("abuild", "PlatformSelector platform 4");
	    }
	    else
	    {
		QTC::TC("abuild", "PlatformSelector platform 5");
	    }
	}
	else
	{
	    throw QEXC::Internal(
		"unknown selector in PlatformSelector::initialize");
	}
    }

    return okay;
}

bool
PlatformSelector::isSkip() const
{
    return this->skip;
}

bool
PlatformSelector::isDefault() const
{
    return this->dfault;
}

std::string const&
PlatformSelector::getPlatformType() const
{
    return this->platform_type;
}

void
PlatformSelector::split_platform(std::string const& platform,
				 std::string& os,
				 std::string& cpu,
				 std::string& toolset,
				 std::string& compiler,
				 std::string& option)
{
    std::list<std::string> fields = Util::split('.', platform);
    unsigned int nfields = fields.size();
    assert((nfields == 4) || (nfields == 5));
    os = fields.front();
    fields.pop_front();
    cpu = fields.front();
    fields.pop_front();
    toolset = fields.front();
    fields.pop_front();
    compiler = fields.front();
    if (nfields == 5)
    {
	fields.pop_front();
	option = fields.front();
    }
}
