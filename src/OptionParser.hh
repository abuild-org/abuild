#ifndef __OPTIONPARSER_HH__
#define __OPTIONPARSER_HH__

#include <boost/function.hpp>
#include <boost/regex.hpp>
#include <vector>
#include <map>

// The option parser recognizes options and their arguments and then
// calls a callback based on the options.  Option arguments are
// recognized using perl-compatible regular expression objects with the
// Boost.Regex library.  Callbacks are defined using Boost.Function.
//
// Options are identified as the string that follows the dash or dashes.
//
// Recognition of options:
//
//  * An option may be a single alphabetic character or two or more
//    alphanumeric characters, underscores, or dashes starting with an
//    alphabetic character.
//
//  * Single character options are recognized only when preceded by a
//    single dash.
//
//  * Multi-character options are recognized when preceded by one or two
//    dashes.
//
//  * Anything not starting with a dash that is not an option argument is
//    treated as a positional argument.
//
// Options may have arguments.  There are several cases:
//
//  * Arguments may be specified as optional numeric, numeric, string,
//    matching a particular regular expression, or a list of strings.
//    Numeric options are always required to be positive numbers.
//
//  * Arguments that are lists of strings may optionally have a
//    termination regular expression.  All strings up to but not
//    including the first argument that matches the termination
//    expression are included in the list.  If discard_term_arg is
//    true, the termination argument will be discarded.  Otherwise, it
//    will considered as a normal argument after the end of the list.
//
//
// Recognition of arguments to options:
//
//  * Single character options' arguments must follow the option as the
//    next word (-b all).  Special case exception: numeric arguments to
//    single-character options may be directly concatenated to their
//    arguments (-j4).
//
//  * Multi-character options' arguments may either be separated from the
//    argument with = (--jobs=4) or may follow it (--jobs 4).
//
//  * If an option's argument is optional and is not joined to the option
//    with =, the next command line argument will only be treated as the
//    argument to that option if it does not start with dash.  This is
//    only possible with numeric arguments.
//
// You may register an option as having no argument or any of the
// specified argument types.  You may also register an option as being a
// synonym to another option.

class OptionParser
{
  public:
    OptionParser(
	boost::function<void(std::string const&)> error_fn,
	boost::function<void(std::string const&)> pos_callback);

    void registerNoArg(
	std::string const& option,
	boost::function<void()> callback);
    void registerOptionalNumericArg(
	std::string const& option,
	unsigned int default_value,
	boost::function<void(unsigned int)> callback);
    void registerNumericArg(
	std::string const& option,
	boost::function<void(unsigned int)> callback);
    void registerStringArg(
	std::string const& option,
	boost::function<void(std::string const&)> callback);
    void registerRegexArg(
	std::string const& option,
	std::string const& regex,
	boost::function<void(boost::smatch const&)> callback);
    void registerListArg(
	std::string const& option,
	std::string const& term_regex,
	bool discard_term_arg,
	boost::function<void(std::vector<std::string> const&)> callback);
    void registerSynonym(
	std::string const& option,
	std::string const& existing_option);

    // argv is just the options to parse; typically this would be
    // called with argv + 1 to skip argv[0].
    void parseOptions(char* argv[]);

  private:
    enum arg_type_e
    {
	at_none,
	at_optional_numeric,
	at_numeric,
	at_string,
	at_regex,
	at_list
    };

    class ArgumentSpecification
    {
      public:
	ArgumentSpecification() :
	    arg_type(at_none),
	    default_value(0),
	    discard_term_arg(false)
	{
	}

	arg_type_e arg_type;
	int default_value;	// at_optional_numeric
	bool discard_term_arg;	// at_list
	boost::regex regex;	// at_regex, at_list

	// Only one of these will be defined.  It's up to the code to
	// call the right one.
	boost::function<void()> cb_void;
	boost::function<void(int)> cb_int;
	boost::function<void(std::string const&)> cb_string;
	boost::function<void(boost::smatch const&)> cb_regex;
	boost::function<void(std::vector<std::string> const&)> cb_vector;
    };

    void handleOption(std::string const& option, ArgumentSpecification const&,
		      bool has_attached_arg, std::string const& attached_arg,
		      char **& arg);

    std::map<std::string, ArgumentSpecification> arguments;
    boost::function<void(std::string const&)> error_fn;
    boost::function<void(std::string const&)> pos_callback;
};

#endif // __OPTIONPARSER_HH__
