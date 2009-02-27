
#include <InterfaceParser.hh>

#include <iostream>
#include <list>
#include <Logger.hh>
#include <Error.hh>
#include <Util.hh>
#include <FlagData.hh>

static Logger* logger = 0;

static std::string
unparse_type(bool recursive, Interface::type_e type,
	     Interface::list_e list_type)
{
    std::string result;
    if (! recursive)
    {
	result += "non-recursive ";
    }
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

static void dump_interface(std::string const& name, Interface const& _interface,
			   std::vector<std::string> const& after_builds)
{
    FlagData flag_data;
    flag_data.addFlag("p0", "test-interface-parser");
    logger->logInfo("dumping interface " + name);
    std::set<std::string> names = _interface.getVariableNames();
    for (std::set<std::string>::iterator iter = names.begin();
	 iter != names.end(); ++iter)
    {
	Interface::VariableInfo info;
	bool status = _interface.getVariable(*iter, flag_data, info);
	assert(status);
	std::string msg = "  " + *iter + ": " +
	    unparse_type(info.recursive, info.type, info.list_type) +
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

    if (after_builds.empty())
    {
	logger->logInfo("  no after-build files");
    }
    else
    {
	logger->logInfo("  after-build files:");
	for (std::vector<std::string>::const_iterator iter =
		 after_builds.begin();
	     iter != after_builds.end(); ++iter)
	{
	    logger->logInfo("    " + *iter);
	}
    }

    logger->logInfo("end of interface " + name);
}

static void usage()
{
    std::cerr << "Usage: test_interface_parser [ -allow-flags flag ... ]"
	      << " -method { 0 | 1 } file ..."
	      << std::endl;
    exit(2);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
	usage();
    }

    int method_arg = 2;
    int first_file_arg = 3;
    std::set<std::string> supported_flags;
    if (strcmp(argv[1], "-allow-flags") == 0)
    {
	int i = 0;
	for (i = 2; i < argc; ++i)
	{
	    if (argv[i][0] != '-')
	    {
		supported_flags.insert(argv[i]);
	    }
	    else
	    {
		break;
	    }
	}
	if (! ((strcmp(argv[i], "-method") == 0) &&
	       (argc >= i + 2)))
	{
	    usage();
	}
	method_arg = i + 1;
	first_file_arg = i + 2;
    }

    int method = atoi(argv[method_arg]);

    logger = Logger::getInstance();
    std::list<boost::shared_ptr<Interface> > interfaces;
    try
    {
	// In method 0, use the same interface parser to read the
	// interfaces in sequence.  In method 1, read interfaces in
	// sequence and import each previously read interface.  The
	// results should be identical except when resetVariable is
	// called.  In that case, importing in sequence will cause
	// reset statements in subsequent files to clear only
	// assignments made in those files.

	// Additionally, the interface flag "test-interface-parser" is
	// set for method 0 and not for method 1.

	// Reused InterfaceParser object for method 0
	InterfaceParser p0("p0", "indep", Util::getCurrentDirectory());
	p0.setSupportedFlags(supported_flags);
	bool debug = false;
	if (Util::getEnv("DEBUG_INTERFACE_PARSER"))
	{
	    debug = true;
	}

	for (int i = first_file_arg; i < argc; ++i)
	{
	    char const* filename = argv[i];
	    std::string dir = Util::dirname(Util::canonicalizePath(filename));

	    // Per-file InterfaceParser for method 1
	    std::string name = "p1-" + Util::basename(filename);
	    InterfaceParser p1(name, "indep", dir);
	    p1.setSupportedFlags(supported_flags);
	    if (debug)
	    {
		p0.setDebugParser(true);
		p1.setDebugParser(true);
	    }

	    if (method == 1)
	    {
		for (std::list<boost::shared_ptr<Interface> >::iterator iter =
			 interfaces.begin();
		     iter != interfaces.end(); ++iter)
		{
		    p1.importInterface(**iter);
		}
	    }
	    InterfaceParser& parser = (method == 0 ? p0 : p1);
	    bool status = parser.parse(filename, true);
	    logger->logInfo(std::string("parse(") + filename + "): " +
			    (status ? "success" : "failure"));
	    boost::shared_ptr<Interface> _interface = parser.getInterface();
	    dump_interface(filename, *_interface, parser.getAfterBuilds());
	    if (method == 1)
	    {
		interfaces.push_back(_interface);
	    }
	}
    }
    catch (std::exception& e)
    {
	std::cerr << e.what() << std::endl;
	exit(2);
    }

    Logger::stopLogger();
    return 0;
}
