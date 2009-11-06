
#include <InterfaceParser.hh>

#include <iostream>
#include <sstream>
#include <list>
#include <vector>
#include <Logger.hh>
#include <Error.hh>
#include <Util.hh>
#include <FlagData.hh>

static Logger* logger = 0;

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
			   bool dump,
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
	    Interface::unparse_type(info.scope, info.type, info.list_type) +
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

    if (dump)
    {
	std::ostringstream s;
	_interface.dump(s);
	logger->logInfo(s.str());
    }

    logger->logInfo("end of interface " + name);
}

static void usage()
{
    std::cerr << "Usage: test_interface_parser [ -allow-flags flag ... ]"
	      << " [ -dump-xml ] -method { 0 | 1 } file ..."
	      << std::endl;
    exit(2);
}

int main(int argc, char* argv[])
{
    bool dump = false;
    std::set<std::string> supported_flags;
    std::vector<std::string> files;
    int method = -1;
    for (char** arg = &argv[1]; *arg; ++arg)
    {
	if (strcmp(*arg, "-allow-flags") == 0)
	{
	    while (*(arg+1) && (*(arg+1))[0] != '-')
	    {
		supported_flags.insert(*(++arg));
	    }
	}
	else if (strcmp(*arg, "-dump-xml") == 0)
	{
	    dump = true;
	}
	else if (strcmp(*arg, "-method") == 0)
	{
	    if (! *(arg+1))
	    {
		usage();
	    }
	    method = atoi(*(++arg));
	}
	else
	{
	    files.push_back(*arg);
	}
    }
    if ((method == -1) || files.empty())
    {
	usage();
    }

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

	for (std::vector<std::string>::iterator iter = files.begin();
	     iter != files.end(); ++iter)
	{
	    char const* filename = (*iter).c_str();
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
	    dump_interface(filename, *_interface, dump,
			   parser.getAfterBuilds());
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
