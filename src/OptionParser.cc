#include <OptionParser.hh>
#include <QTC.hh>
#include <cstdlib>

OptionParser::OptionParser(
    boost::function<void(std::string const&)> error_fn,
    boost::function<void(std::string const&)> pos_callback)
    :
    error_fn(error_fn),
    pos_callback(pos_callback)
{
}

void
OptionParser::registerNoArg(
    std::string const& option,
    boost::function<void()> callback)
{
    assert(this->arguments.count(option) == 0);
    ArgumentSpecification a;
    a.arg_type = at_none;
    a.cb_void = callback;
    this->arguments[option] = a;
}

void
OptionParser::registerOptionalNumericArg(
    std::string const& option,
    unsigned int default_value,
    boost::function<void(unsigned int)> callback)
{
    assert(this->arguments.count(option) == 0);
    ArgumentSpecification a;
    a.arg_type = at_optional_numeric;
    a.default_value = default_value;
    a.cb_int = callback;
    this->arguments[option] = a;
}

void
OptionParser::registerNumericArg(
    std::string const& option,
    boost::function<void(unsigned int)> callback)
{
    assert(this->arguments.count(option) == 0);
    ArgumentSpecification a;
    a.arg_type = at_numeric;
    a.cb_int = callback;
    this->arguments[option] = a;
}

void
OptionParser::registerStringArg(
    std::string const& option,
    boost::function<void(std::string const&)> callback)
{
    assert(this->arguments.count(option) == 0);
    ArgumentSpecification a;
    a.arg_type = at_string;
    a.cb_string = callback;
    this->arguments[option] = a;
}

void
OptionParser::registerRegexArg(
    std::string const& option,
    std::string const& regex,
    boost::function<void(boost::smatch const&)> callback)
{
    assert(this->arguments.count(option) == 0);
    ArgumentSpecification a;
    a.arg_type = at_regex;
    a.regex = boost::regex(regex);
    a.cb_regex = callback;
    this->arguments[option] = a;
}

void
OptionParser::registerListArg(
    std::string const& option,
    std::string const& term_regex,
    bool discard_term_arg,
    boost::function<void(std::vector<std::string> const&)> callback)
{
    assert(this->arguments.count(option) == 0);
    ArgumentSpecification a;
    a.arg_type = at_list;
    a.regex = boost::regex(term_regex);
    a.discard_term_arg = discard_term_arg;
    a.cb_vector = callback;
    this->arguments[option] = a;
}

void
OptionParser::registerSynonym(
    std::string const& option,
    std::string const& existing_option)
{
    assert(this->arguments.count(option) == 0);
    assert(this->arguments.count(existing_option) != 0);
    this->arguments[option] = this->arguments[existing_option];
}

void
OptionParser::parseOptions(char* argv[])
{
    boost::regex short_option_re("-([a-zA-Z])(\\d+)?");
    boost::regex long_option_re("-?-([a-zA-Z][\\w-]+)(?:=(.*))?");
    boost::cmatch m;
    for (char** arg = argv; *arg; ++arg)
    {
	std::string option;
	std::string attached_arg;
	bool has_attached_arg = false;
	if (boost::regex_match(*arg, m, short_option_re) ||
	    boost::regex_match(*arg, m, long_option_re))
	{
	    option = m[1].str();
	    if ((has_attached_arg = m[2].matched))
	    {
		attached_arg = m[2].str();
	    }
	    QTC::TC("abuild", "OptionParser matched",
		    (((option.length() == 1) ? 0 : 1) |
		     (has_attached_arg ? 0 : 2)));
	    std::map<std::string, ArgumentSpecification>::iterator data =
		this->arguments.find(option);
	    if (data == this->arguments.end())
	    {
		QTC::TC("abuild", "OptionParser ERR unknown option");
		this->error_fn("unknown option \"" + option + "\"");
	    }
	    else
	    {
		// This call may increment arg
		handleOption(option, (*data).second,
			     has_attached_arg, attached_arg, arg);
	    }
	}
	else if (**arg == '-')
	{
	    QTC::TC("abuild", "OptionParser ERR unknown argument");
	    this->error_fn(std::string("unknown argument \"") + *arg + "\"");
	}
	else
	{
	    QTC::TC("abuild", "OptionParser positional argument");
	    this->pos_callback(*arg);
	}
    }
}

void
OptionParser::handleOption(
    std::string const& option, ArgumentSpecification const& arg_data,
    bool has_attached_arg, std::string const& attached_arg,
    char **& arg)
{
    boost::regex numeric_re("\\d+");
    boost::smatch m;

    arg_type_e arg_type = arg_data.arg_type;
    bool arg_available = false;
    std::string first_arg;
    bool first_arg_is_next_arg = false;
    if (has_attached_arg)
    {
	arg_available = true;
	first_arg = attached_arg;
    }
    else if (*(arg + 1))
    {
	arg_available = true;
	first_arg = *(arg + 1);
	first_arg_is_next_arg = true;
    }

    unsigned int numeric_arg = 0;
    bool has_numeric_arg = false;
    if ((arg_type == at_numeric) || (arg_type == at_optional_numeric))
    {
	// Check for numeric argument
	if (arg_available && boost::regex_match(first_arg, m, numeric_re))
	{
	    QTC::TC("abuild", "OptionParser found numeric arg",
		    first_arg_is_next_arg ? 0 : 1);
	    has_numeric_arg = true;
	    if (first_arg_is_next_arg)
	    {
		// Consume the next argument
		++arg;
	    }
	    numeric_arg = std::atoi(m[0].str().c_str());
	}
	else if ((arg_type == at_optional_numeric) && (! has_attached_arg))
	{
	    QTC::TC("abuild", "OptionParser using default numeric value");
	    has_numeric_arg = true;
	    numeric_arg = arg_data.default_value;
	}
    }

    switch (arg_type)
    {
	// omit default so gcc will warn for missing case labels
      case at_none:
	if (has_attached_arg)
	{
	    QTC::TC("abuild", "OptionParser ERR unwanted attached arg");
	    this->error_fn(
		"option \"" + option + "\" does not take any arguments");
	}
	else
	{
	    QTC::TC("abuild", "OptionParser call void callback");
	    arg_data.cb_void();
	}
	break;

      case at_optional_numeric:
      case at_numeric:
	if (! has_numeric_arg)
	{
	    QTC::TC("abuild", "OptionParser ERR non-numeric arg",
		    (arg_type == at_optional_numeric ? 0 : 1));
	    this->error_fn(
		"option \"" + option + "\"'s argument must be numeric");
	}
	else
	{
	    QTC::TC("abuild", "OptionParser call int callback");
	    arg_data.cb_int(numeric_arg);
	}
	break;

      case at_string:
      case at_regex:
	if (! arg_available)
	{
	    QTC::TC("abuild", "OptionParser ERR missing required arg",
		    (arg_type == at_string ? 0 : 1));
	    this->error_fn("option \"" + option + "\" requires an argument");
	}
	else
	{
	    if (first_arg_is_next_arg)
	    {
		// Consume argument
		++arg;
	    }
	    if (arg_type == at_string)
	    {
		QTC::TC("abuild", "OptionParser call string callback");
		arg_data.cb_string(first_arg);
	    }
	    else if (boost::regex_match(first_arg, m, arg_data.regex))
	    {
		QTC::TC("abuild", "OptionParser call regex callback");
		arg_data.cb_regex(m);
	    }
	    else
	    {
		QTC::TC("abuild", "OptionParser ERR regex mismatch");
		this->error_fn("invalid argument to option \"" + option +
			       "\": \"" + first_arg + "\"");
	    }
	}
	break;

      case at_list:
	{
	    // Gather up list options.  Consume arguments up to but
	    // not including the first one that matches the
	    // termination regex.
	    std::vector<std::string> args;
	    if (has_attached_arg)
	    {
		args.push_back(attached_arg);
	    }
	    bool done = false;
	    while (! done)
	    {
		if (*(arg + 1) == 0)
		{
		    QTC::TC("abuild", "OptionParser list to end of args",
			    arg_data.discard_term_arg ? 0 : 1);
		    done = true;
		}
		else
		{
		    bool consume = true;
		    bool discard = false;
		    std::string next_arg = *(arg + 1);
		    if (boost::regex_match(next_arg, m, arg_data.regex))
		    {
			QTC::TC("abuild", "OptionParser found term arg",
				arg_data.discard_term_arg ? 0 : 1);
			done = true;
			consume = false;
			discard = arg_data.discard_term_arg;
		    }
		    if (consume)
		    {
			++arg;
			args.push_back(next_arg);
		    }
		    else if (discard)
		    {
			++arg;
		    }
		}
	    }
	    QTC::TC("abuild", "OptionParser call vector callback",
		    has_attached_arg ? 0 :
		    args.empty() ? (arg_data.discard_term_arg ? 1 : 2) :
		    3);
	    arg_data.cb_vector(args);
	}
	break;
    }
}
