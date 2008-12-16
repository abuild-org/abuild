#include <Interface.hh>
#include <Error.hh>
#include <Logger.hh>
#include <FlagData.hh>
#include <string.h>
#include <assert.h>

static Logger* logger = 0;
static char const* whoami = 0;

static std::string
unparse_type(Interface::type_e type, Interface::list_e list_type)
{
    std::string result;
    switch (type)
    {
      case Interface::t_string:
	result += "string";
	break;

      case Interface::t_boolean:
	result += "boolean";
	break;

      case Interface::t_filename:
	result += "filename";
	break;
    }
    switch (list_type)
    {
      case Interface::l_scalar:
	result += " scalar";
	break;

      case Interface::l_append:
	result += " appendlist";
	break;

      case Interface::l_prepend:
	result += " prependlist";
	break;
    }

    return result;
}

static std::string
unparse_values(std::deque<std::string> const& value)
{
    std::string result;
    for (std::deque<std::string>::const_iterator iter = value.begin();
	 iter != value.end(); ++iter)
    {
	result += " " + (*iter);
    }
    return result;
}

static std::string
unparse_assignment_type(Interface::assign_e assignment_type)
{
    std::string result;
    switch (assignment_type)
    {
      case Interface::a_normal:
	result += "normal";
	break;

      case Interface::a_override:
	result += "override";
	break;

      case Interface::a_fallback:
	result += "fallback";
	break;
    }
    result += " ";
    return result;
}


static void
declare(Interface& _interface, FileLocation const& location,
	std::string const& variable_name,
	Interface::type_e type, Interface::list_e list_type)
{
    logger->logInfo("declaring " + variable_name + ": " +
		    unparse_type(type, list_type));
    bool status = _interface.declareVariable(
	location, variable_name, type, list_type);
    logger->logInfo(status ? "success" : "failure");
}

static void
assign(Interface& _interface, FileLocation const& location,
       std::string const& variable_name,
       std::deque<std::string> value,
       Interface::assign_e assignment_type,
       std::string const& flag = "")
{
    std::string flag_info = (flag.empty() ? "" : "flag=" + flag + " ");
    logger->logInfo(unparse_assignment_type(assignment_type) +
		    flag_info + "assignment to " + variable_name + " =" +
		    unparse_values(value));
    bool status = _interface.assignVariable(
	location, variable_name, value, assignment_type, flag);
    logger->logInfo(status ? "success" : "failure");
}

static void
reset(Interface& _interface, FileLocation const& location,
      std::string const& variable_name)
{
    logger->logInfo("resetting " + variable_name);
    bool status = _interface.resetVariable(location, variable_name);
    logger->logInfo(status ? "success" : "failure");
}

static std::deque<std::string>
make_deque(std::string const& value)
{
    std::deque<std::string> result;
    result.push_back(value);
    return result;
}

static std::deque<std::string>
make_deque(std::string const& v1, std::string const& v2)
{
    std::deque<std::string> result;
    result.push_back(v1);
    result.push_back(v2);
    return result;
}

static void dump_interface(std::string const& name, Interface const& _interface,
			   FlagData const& flag_data)
{
    logger->logInfo("dumping interface " + name);
    std::set<std::string> names = _interface.getVariableNames();
    for (std::set<std::string>::iterator iter = names.begin();
	 iter != names.end(); ++iter)
    {
	Interface::VariableInfo info;
	bool status = _interface.getVariable(*iter, flag_data, info);
	assert(status);
	std::string msg = *iter + ": " +
	    unparse_type(info.type, info.list_type) +
	    ", target type " + TargetType::getName(info.target_type);
	if (info.initialized)
	{
	    msg += " =" + unparse_values(info.value);
	}
	else
	{
	    msg += ": uninitialized";
	}
	logger->logInfo(msg);
    }
    logger->logInfo("end of interface " + name);
}

int main(int argc, char* argv[])
{
    if ((whoami = strrchr(argv[0], '/')) == NULL)
    {
	whoami = argv[0];
    }
    else
    {
	++whoami;
    }

    if (argc != 2)
    {
	std::cerr << "Usage: " << whoami << " current-directory" << std::endl;
	exit(2);
    }

    std::string current_directory = argv[1];
    logger = Logger::getInstance();
    Error error;
    FlagData empty_flag_data;

    // Ordinarily, the Interface class would be populated mostly by
    // loading things from interface files.  Here we'll fabricate
    // FileLocation objects to simulate this.

    // Simulate base interface.  Predefine a few variables.
    Interface base("base", "base", error, current_directory);
    base.setTargetType(TargetType::tt_all);

    // Simulate some internal variable definitions.
    declare(base, FileLocation("<internal>", 0, 0),
	    "STRING1", Interface::t_string, Interface::l_scalar);
    assign(base, FileLocation("<internal>", 0, 0),
	   "STRING1", make_deque("potato"), Interface::a_normal);
    declare(base, FileLocation("<internal>", 0, 0),
	    "BOOL1", Interface::t_boolean, Interface::l_scalar);
    assign(base, FileLocation("<internal>", 0, 0),
	   "BOOL1", make_deque("true"), Interface::a_normal);

    // Simulate some base definitions loaded from some files.
    base.setTargetType(TargetType::tt_object_code);
    declare(base, FileLocation("base", 1, 1),
	    "INCLUDES", Interface::t_filename, Interface::l_prepend);
    declare(base, FileLocation("base", 2, 1),
	    "LIBS", Interface::t_string, Interface::l_prepend);
    base.setTargetType(TargetType::tt_platform_independent);
    declare(base, FileLocation("base", 4, 1),
	    "THINGS", Interface::t_string, Interface::l_append);

    dump_interface("base", base, empty_flag_data);

    // Simulate item4 -> item3, item2; item3 -> item1; item2 -> item1.

    Interface item1("item1", "indep", error, current_directory + "/item1");
    item1.setTargetType(TargetType::tt_object_code);

    item1.importInterface(base);
    assign(item1, FileLocation("item1", 1, 1),
	   "INCLUDES", make_deque(".", "include"), Interface::a_normal);
    assign(item1, FileLocation("item1", 2, 1),
	   "LIBS", make_deque("l1-1", "l1-2"), Interface::a_normal);
    assign(item1, FileLocation("item1", 3, 1),
	   "THINGS", make_deque("th1-1", "th1-2"), Interface::a_normal);
    declare(item1, FileLocation("item1", 4, 1),
	    "MOO", Interface::t_string, Interface::l_scalar);
    assign(item1, FileLocation("item1", 5, 1),
	   "MOO", make_deque("baaa"), Interface::a_normal);
    assign(item1, FileLocation("item1", 6, 1),
	   "MOO", make_deque("oink"), Interface::a_override);
    assign(item1, FileLocation("item1", 7, 1),
	   "MOO", make_deque("quack"), Interface::a_fallback);
    // Private assignment based on flag
    assign(item1, FileLocation("item1", 8, 1),
	   "THINGS", make_deque("th1-private"), Interface::a_normal,
	   "private");

    // Dump interface base to show that it is unchanged.
    dump_interface("base", base, empty_flag_data);
    dump_interface("item1", item1, empty_flag_data);

    // Create items 2 and 3 that import item1

    Interface item2("item2", "indep", error, current_directory + "/item2");
    item2.setTargetType(TargetType::tt_object_code);

    item2.importInterface(item1);
    assign(item2, FileLocation("item2", 1, 1),
	   "INCLUDES", make_deque(".", "include"), Interface::a_normal);
    assign(item2, FileLocation("item2", 2, 1),
	   "LIBS", make_deque("l2-1", "l2-2"), Interface::a_normal);
    assign(item2, FileLocation("item2", 3, 1),
	   "THINGS", make_deque("th2-1", "th2-2"), Interface::a_normal);

    dump_interface("item2", item2, empty_flag_data);

    Interface item3("item3", "indep", error, current_directory + "/item3");
    item3.setTargetType(TargetType::tt_object_code);

    item3.importInterface(item1);
    assign(item3, FileLocation("item3", 1, 1),
	   "INCLUDES", make_deque("."), Interface::a_normal);
    assign(item3, FileLocation("item3", 2, 1),
	   "LIBS", make_deque("l3-1"), Interface::a_normal);
    assign(item3, FileLocation("item3", 3, 1),
	   "THINGS", make_deque("th3-1"), Interface::a_normal);

    dump_interface("item3", item3, empty_flag_data);

    // Create item 4 that imports items 2 and 3.

    Interface item4("item4", "indep", error, current_directory + "/item4");
    item4.setTargetType(TargetType::tt_object_code);

    item4.importInterface(item2);
    item4.importInterface(item3);
    assign(item4, FileLocation("item4", 1, 1),
	   "INCLUDES", make_deque("."), Interface::a_normal);
    assign(item4, FileLocation("item4", 2, 1),
	   "LIBS", make_deque("l4-1"), Interface::a_normal);
    assign(item4, FileLocation("item4", 3, 1),
	   "THINGS", make_deque("th4-1"), Interface::a_normal);
    assign(item4, FileLocation("item4", 4, 1),
	   "MOO", make_deque("cluq"), Interface::a_override);
    declare(item4, FileLocation("item4", 5, 1),
	    "QUACK", Interface::t_string, Interface::l_scalar);

    dump_interface("item4", item4, empty_flag_data);

    // Create item5 that creates a conflict with item1.  This is okay
    // until something imports both.

    Interface item5("item5", "indep", error, current_directory + "/item5");
    item5.setTargetType(TargetType::tt_object_code);

    item5.importInterface(base);
    declare(item5, FileLocation("item5", 1, 1),
	    "MOO", Interface::t_string, Interface::l_scalar);
    assign(item5, FileLocation("item5", 2, 1),
	   "MOO", make_deque("baaa"), Interface::a_fallback);

    dump_interface("item5", item5, empty_flag_data);

    // Create a bunch of errors including the conflict between item1
    // and item5.
    Interface bad1("bad1", "indep", error, current_directory + "/bad1");

    bad1.importInterface(item1);
    logger->logInfo("importing item5 with conflicts");
    bad1.importInterface(item5);
    logger->logInfo("finished importing item5");

    // Now do a bunch of bad things.

    // Do this bad assignment with the string form of assignVariable
    // so we exercise it in the test suite.
    logger->logInfo("bad assignment to OINK");
    bad1.assignVariable(FileLocation("bad1", 1, 1),
			"OINK", "moo", Interface::a_normal);
    // non-normal assignment to list
    assign(bad1, FileLocation("bad1", 2, 1),
	   "INCLUDES", make_deque("."), Interface::a_override);
    // bad assignment to boolean
    assign(bad1, FileLocation("bad1", 3, 1),
	   "BOOL1", make_deque("potato"), Interface::a_override);
    declare(bad1, FileLocation("bad1", 4, 1),
	    "FILE", Interface::t_filename, Interface::l_scalar);
    assign(bad1, FileLocation("bad1", 5, 1),
	   "FILE", make_deque(""), Interface::a_normal);
    assign(bad1, FileLocation("bad1", 6, 1),
	   "STRING1", make_deque("spackle"), Interface::a_normal);

    // Get an unknown variable value
    logger->logInfo("getting value for unknown variable");
    { // local scope
	Interface::VariableInfo info;
	bool status = bad1.getVariable("FARBAGE", info);
	logger->logInfo(std::string("getVariable(FARBAGE): ") +
			(status ? "success" : "failure"));
    }

    // Create item6 that loads item2 and reset some things that were
    // imported from item1 and some things that were created in item2.
    // It also includes some flag-based assignments.
    Interface item6("item6", "indep", error, current_directory + "/item6");
    item6.setTargetType(TargetType::tt_object_code);

    item6.importInterface(item1);
    item6.importInterface(item2);
    reset(item6, FileLocation("item6", 1, 1), "MOO");
    reset(item6, FileLocation("item6", 2, 1), "THINGS");
    reset(item6, FileLocation("item6", 3, 1), "UNKNOWN_VARIABLE");
    declare(item6, FileLocation("item6", 4, 1),
	    "FLAG1", Interface::t_string, Interface::l_scalar);
    assign(item6, FileLocation("item6", 5, 1),
	   "FLAG1", make_deque("Lesotho"), Interface::a_normal,
	   "funny-looking");
    declare(item6, FileLocation("item6", 6, 1),
	    "FLAG2", Interface::t_string, Interface::l_scalar);
    assign(item6, FileLocation("item6", 7, 1),
	   "FLAG2", make_deque("Japan"), Interface::a_normal);
    assign(item6, FileLocation("item6", 8, 1),
	   "FLAG2", make_deque("Libya"), Interface::a_override,
	   "plain");
    declare(item6, FileLocation("item6", 9, 1),
	    "FLAG3", Interface::t_string, Interface::l_scalar);
    assign(item6, FileLocation("item6", 10, 1),
	   "FLAG3", make_deque("Rwanda"), Interface::a_normal,
	   "letter");
    assign(item6, FileLocation("item6", 11, 1),
	   "FLAG3", make_deque("Canada"), Interface::a_fallback);

    dump_interface("item6", item6, empty_flag_data);

    // Create item7 that loads item1 and item6 to illustrate that we
    // see the things in item1 that were reset in item6 but not the
    // things in item2 that were reset by item6.
    Interface item7("item7", "indep", error, current_directory + "/item7");

    item7.importInterface(item1);
    item7.importInterface(item6);

    dump_interface("item7", item7, empty_flag_data);

    // Set some flags
    FlagData flag_data;
    flag_data.addFlag("item1", "private");
    flag_data.addFlag("item6", "funny-looking");
    dump_interface("item7", item7, flag_data);
    flag_data.addFlag("item6", "plain");
    flag_data.addFlag("item6", "letter");
    dump_interface("item7", item7, flag_data);

    // Exercise fallback/override logic more thoroughly
    Interface item8("item8", "indep", error, current_directory + "/item8");
    declare(item8, FileLocation("item8", 1, 1),
	    "A", Interface::t_string, Interface::l_scalar);
    assign(item8, FileLocation("item8", 2, 1),
	   "A", make_deque("F1"), Interface::a_normal, "F1");
    assign(item8, FileLocation("item8", 3, 1),
	   "A", make_deque("F2"), Interface::a_override, "F2");
    assign(item8, FileLocation("item8", 4, 1),
	   "A", make_deque("F3"), Interface::a_override, "F3");
    assign(item8, FileLocation("item8", 5, 1),
	   "A", make_deque("F4"), Interface::a_fallback, "F4");
    assign(item8, FileLocation("item8", 6, 1),
	   "A", make_deque("F5"), Interface::a_fallback, "F5");

    logger->logInfo("item8, no flags");
    flag_data = FlagData();
    dump_interface("item8", item8, flag_data);

    logger->logInfo("add flag F5");
    flag_data.addFlag("item8", "F5");
    dump_interface("item8", item8, flag_data);

    logger->logInfo("add flag F4");
    flag_data.addFlag("item8", "F4");
    dump_interface("item8", item8, flag_data);

    logger->logInfo("add flag F1");
    flag_data.addFlag("item8", "F1");
    dump_interface("item8", item8, flag_data);

    logger->logInfo("add flag F2");
    flag_data.addFlag("item8", "F2");
    dump_interface("item8", item8, flag_data);

    logger->logInfo("add flag F3");
    flag_data.addFlag("item8", "F3");
    dump_interface("item8", item8, flag_data);

    // Stop logger
    Logger::stopLogger();

    return 0;
}
