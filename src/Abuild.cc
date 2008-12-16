#include <Abuild.hh>

#include <assert.h>
#include <fstream>
#include <sstream>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <Logger.hh>
#include <FileLocation.hh>
#include <ItemConfig.hh>
#include <InterfaceParser.hh>
#include <DependencyRunner.hh>

std::string const Abuild::ABUILD_VERSION = "1.0.2";
std::string const Abuild::OUTPUT_DIR_PREFIX = "abuild-";
std::string const Abuild::FILE_BACKING = "Abuild.backing";
std::string const Abuild::FILE_DYNAMIC_MK = ".ab-dynamic.mk";
std::string const Abuild::FILE_DYNAMIC_ANT = ".ab-dynamic-ant.properties";
std::string const Abuild::b_ALL = "all";
std::string const Abuild::b_LOCAL = "local";
std::string const Abuild::b_DESC = "desc";
std::string const Abuild::b_DEPS = "deps";
std::string const Abuild::b_CURRENT = "current";
std::set<std::string> Abuild::valid_buildsets;
std::string const Abuild::s_CLEAN = "clean";
std::string const Abuild::s_NO_OP = "no-op";
// PLUGIN_PLATFORM can't match a real platform name.
std::string const Abuild::PLUGIN_PLATFORM = "[plugin]";
std::string const Abuild::FILE_PLUGIN_INTERFACE = "plugin.interface";
std::string const Abuild::BUILDER_RE = "([^:]+):([^:]+)";
std::set<std::string> Abuild::special_targets;
std::list<std::string> Abuild::default_targets;

// Initialize this after all other status
bool Abuild::statics_initialized = Abuild::initializeStatics();

Abuild::Abuild(int argc, char* argv[], char* envp[]) :
    argc(argc),
    argv(argv),
    envp(envp),
    whoami(Util::removeExe(Util::basename(argv[0]))),
    stdout_is_tty(Util::stdoutIsTty()),
    max_workers(1),
    keep_going(false),
    no_dep_failures(false),
    full_integrity(false),
    list_traits(false),
    list_platforms(false),
    dump_data(false),
    dump_build_graph(false),
    verbose_mode(false),
    silent(false),
    monitored(false),
    use_abuild_logger(true),
    apply_targets_to_deps(false),
    local_build(false),
    error_handler(whoami),
    this_config(0),
#ifdef _WIN32
    have_perl(false),
#endif
    logger(*(Logger::getInstance()))
{
    Error::setErrorCallback(
	boost::bind(&Abuild::monitorErrorCallback, this, _1));
}

Abuild::~Abuild()
{
    Error::clearErrorCallback();
}

bool
Abuild::initializeStatics()
{
    valid_buildsets.insert(b_ALL);
    valid_buildsets.insert(b_LOCAL);
    valid_buildsets.insert(b_DESC);
    valid_buildsets.insert(b_DEPS);
    valid_buildsets.insert(b_CURRENT);

    special_targets.insert(s_CLEAN);
    special_targets.insert(s_NO_OP);

    default_targets.push_back("all");

    return true;
}

bool
Abuild::run()
{
    bool status = false;
    try
    {
	status = runInternal();
    }
    catch (std::exception& e)
    {
	monitorOutput(std::string("fatal-error ") + e.what());
	throw;
    }
    return status;
}

bool
Abuild::runInternal()
{
    // Handle a few arguments that short-circuit normal operation.
    if (this->argc == 2)
    {
	bool exit_now = false;
	std::string first_arg = argv[1];
	if ((first_arg == "-V") || (first_arg == "--version"))
	{
	    boost::function<void(std::string const&)> l =
		boost::bind(&Logger::logInfo, &(this->logger), _1);
	    l(this->whoami + " version " + ABUILD_VERSION);
	    l("");
	    l("Copyright (c) 2007, 2008 Jay Berkenbilt, Argon ST");
	    l("This software may be distributed under the terms of version 2 of");
	    l("the Artistic License which may be found in the source and binary");
	    l("distributions.  It is provided \"as is\" without express or");
	    l("implied warranty.");
	    exit_now = true;
	}
	else if ((first_arg == "-H") || (first_arg == "--help"))
	{
	    help();
	    exit_now = true;
	}

	if (exit_now)
	{
	    return true;
	}
    }

    if (! Util::getProgramFullPath(argv[0], this->program_fullpath))
    {
	fatal("unable to determine full path of program");
    }
    this->abuild_top = Util::dirname(this->program_fullpath);
    while (! Util::isFile(this->abuild_top + "/make/abuild.mk"))
    {
	std::string next = Util::dirname(this->abuild_top);
	if (this->abuild_top == next)
	{
	    fatal("unable to find root of abuild tree above " +
		  this->program_fullpath);
	}
	else
	{
	    this->abuild_top = next;
	}
    }

    if (this->argc == 2)
    {
	if (strcmp(argv[1], "--print-abuild-top") == 0)
	{
	    this->logger.logInfo(this->abuild_top);
	    return true;
	}
    }

    parseArgv();
    exitIfErrors();

    initializePlatforms();
    exitIfErrors();

    if (readConfigs())
    {
	// readConfigs did everything that needs to be done.
	return true;
    }

    exitIfErrors();

    bool okay = true;
    if (this->special_target == s_CLEAN)
    {
        QTC::TC("abuild", "Abuild clean");
        cleanPath("", this->current_directory);
    }
    else if (! this->cleanset_name.empty())
    {
        cleanBuildset();
    }
    else
    {
        okay = buildBuildset();
    }

    return okay;
}

void
Abuild::getThisPlatform()
{
    // Determine whether we are in an abuild output directory

    std::string base_cwd = Util::basename(Util::getCurrentDirectory());
    if ((! Util::isFile(ItemConfig::FILE_CONF.c_str())) &&
	(Util::isFile(std::string("../" + ItemConfig::FILE_CONF).c_str())) &&
	(base_cwd.substr(0, OUTPUT_DIR_PREFIX.length()) == OUTPUT_DIR_PREFIX) &&
	Util::isFile(".abuild"))
    {
	// skip prefix
	this->this_platform = base_cwd.substr(OUTPUT_DIR_PREFIX.length());
    }
}

void
Abuild::parseArgv()
{
    boost::regex jobs_re("(?:-j|--jobs=)(\\d+)");
    boost::regex make_jobs_re("--make-jobs(?:=(\\d+))?");
    boost::regex buildset_re("--build=(\\S+)");
    boost::regex cleanset_re("--clean=(\\S+)");

    boost::smatch match;

    std::list<std::string> platform_selector_strings;
    std::string platform_selector_env;
    if (Util::getEnv("ABUILD_PLATFORM_SELECTORS", &platform_selector_env))
    {
	QTC::TC("abuild", "Abuild ABUILD_PLATFORM_SELECTORS");
	platform_selector_strings = Util::splitBySpace(platform_selector_env);
    }

    bool with_deps = false;
    char** argp = &argv[1];
    while (*argp)
    {
	std::string arg = *argp++;
	if (boost::regex_match(arg, match, jobs_re))
	{
	    this->max_workers = atoi(match[1].str().c_str());
	    if (max_workers == 0)
	    {
		usage("-j's argument must be > 0");
	    }
	}
	else if (boost::regex_match(arg, match, make_jobs_re))
	{
	    std::string make_njobs;
	    if (match[1].matched)
	    {
		make_njobs = match[1].str();
	    }
	    this->make_args.push_back("-j" + make_njobs);
	}
	else if ((arg == "-k") || (arg == "--keep-going"))
	{
	    this->make_args.push_back("-k");
	    this->ant_args.push_back("-k");
	    this->keep_going = true;
	}
	else if (arg == "--no-dep-failures")
	{
	    this->no_dep_failures = true;
	}
	else if ((arg == "-n") ||
		 (arg == "--just-print") ||
		 (arg == "--dry-run") ||
		 (arg == "--recon"))
	{
	    this->make_args.push_back("-n");
	}
	else if ((arg == "-e") || (arg == "--emacs"))
	{
	    this->ant_args.push_back("-e");
	    this->ant_args.push_back("-Dabuild.private.emacs-mode=1");
	}
	else if (arg == "--no-abuild-logger")
	{
	    this->use_abuild_logger = false;
	}
	else if (arg == "-C")
	{
	    if (! *argp)
	    {
		usage("-C requires an argument");
	    }
	    this->start_dir = *argp++;
	}
	else if (arg == "--find-conf")
	{
	    this->start_dir = findConf();
	}
	else if (arg == "--make")
	{
	    while (*argp)
	    {
		this->make_args.push_back(*argp++);
		if (*argp && (strcmp(*argp, "--ant") == 0))
		{
		    --argp;
		    break;
		}
	    }
	}
	else if (arg == "--ant")
	{
	    while (*argp)
	    {
		this->ant_args.push_back(*argp++);
		if (*argp && (strcmp(*argp, "--make") == 0))
		{
		    --argp;
		    break;
		}
	    }
	}
	else if (boost::regex_match(arg, match, buildset_re))
	{
	    this->buildset_name = match[1].str();
	    checkBuildsetName("build", this->buildset_name);
	}
	else if (arg == "-b")
	{
	    if (! *argp)
	    {
		usage("-b requires an argument");
	    }
	    this->buildset_name = *argp++;
	    checkBuildsetName("build", this->buildset_name);
	}
	else if (boost::regex_match(arg, match, cleanset_re))
	{
	    this->cleanset_name = match[1].str();
	    checkBuildsetName("clean", this->cleanset_name);
	}
	else if (arg == "-c")
	{
	    if (! *argp)
	    {
		usage("-c requires an argument");
	    }
	    this->cleanset_name = *argp++;
	    checkBuildsetName("clean", this->cleanset_name);
	}
	else if (arg == "--only-with-traits")
	{
	    if (! *argp)
	    {
		usage("--only-with-traits requires an argument");
	    }
	    this->only_with_traits = Util::split(',', *argp++);
	}
	else if (arg == "--related-by-traits")
	{
	    if (! *argp)
	    {
		usage("--related-by-traits requires an argument");
	    }
	    this->related_by_traits = Util::split(',', *argp++);
	}
	else if ((arg == "--platform-selector") || (arg == "-p"))
	{
	    if (! *argp)
	    {
		usage("--platform-selector requires an argument");
	    }
	    platform_selector_strings.push_back(*argp++);
	}
	else if ((arg == "--with-deps") || (arg == "-d"))
	{
	    with_deps = true;
	}
	else if (arg == "--apply-targets-to-deps")
	{
	    this->apply_targets_to_deps = true;
	}
	else if (arg == "--full-integrity")
	{
	    this->full_integrity = true;
	}
	else if (arg == "--list-traits")
	{
	    this->list_traits = true;
	}
	else if (arg == "--list-platforms")
	{
	    this->list_platforms = true;
	}
	else if (arg == "--dump-data")
	{
	    this->dump_data = true;
	}
	else if (arg == "--dump-build-graph")
	{
	    this->dump_build_graph = true;
	}
	else if (arg == "--verbose")
	{
	    this->make_args.push_back("ABUILD_VERBOSE=1");
	    this->ant_args.push_back("-DABUILD_VERBOSE=1");
	    this->ant_args.push_back("-verbose");
	    this->verbose_mode = true;
	}
	else if (arg == "--silent")
	{
	    this->make_args.push_back("ABUILD_SILENT=1");
	    this->ant_args.push_back("-DABUILD_SILENT=1");
	    this->ant_args.push_back("-quiet");
	    this->silent = true;
	}
	else if (arg == "--monitored")
	{
	    this->monitored = true;
	}
	else if ((! arg.empty()) && (arg[0] == '-'))
	{
	    usage("invalid option " + arg);
	}
	else if (special_targets.count(arg))
	{
	    if (! this->special_target.empty())
	    {
		usage("only one special target may be specified");
	    }
	    this->special_target = arg;
	}
	else
	{
	    this->targets.push_back(arg);
	}
    }

    // If an alternative start directory was specified, chdir to it
    // before we ever access any of the user's files.
    if (! this->start_dir.empty())
    {
	// Throws an exception on failure
	Util::setCurrentDirectory(this->start_dir);
    }
    this->current_directory = Util::getCurrentDirectory();

    getThisPlatform();

    if (with_deps)
    {
	if (! (this->buildset_name.empty() &&
	       this->cleanset_name.empty()))
	{
	    usage("--with-deps may not be used with --build or --clean");
	}
	QTC::TC("abuild", "Abuild with_deps");
	this->buildset_name = b_CURRENT;
    }

    // Perform additional validation of the command-line arguments to
    // make sure we haven't specified --build with --clean or other
    // mutually exclusive options.

    if ((this->dump_data || this->list_traits) &&
	(! (this->cleanset_name.empty() &&
	    this->buildset_name.empty() &&
	    this->targets.empty())))
    {
	usage("--dump-data and --list-traits may not be combined with"
	      " build sets or targets");
    }

    if (! this->cleanset_name.empty())
    {
	if ((! this->buildset_name.empty()) ||
	    (! this->targets.empty()) ||
	    (! this->special_target.empty()))
	{
	    usage("--clean may not be combined with --build, --with-deps,"
		  " or targets");
	}
    }
    else if (this->special_target == s_CLEAN)
    {
	// already known cleanset_name is empty
	if ((! this->buildset_name.empty()) ||
	    (! this->targets.empty()))
	{
	    usage("\"clean\" may not be combined with --build, --with-deps,"
		  " or other targets");
	}
    }
    else if (! this->special_target.empty())
    {
	// already known cleanset_name is undefined
	if (! this->targets.empty())
	{
	    usage("\"" + this->special_target + "\" may not be combined"
		  " with any other targets");
	}
    }

    if (this->buildset_name.empty() && this->cleanset_name.empty())
    {
	this->local_build = true;
	if (! (this->only_with_traits.empty() &&
	       this->related_by_traits.empty()))
	{
	    usage("--only-with-traits and --related-by-traits may"
		  " not be used without a build set or --with-deps");
	}
    }

    if (! (this->only_with_traits.empty() && this->related_by_traits.empty()))
    {
	QTC::TC("abuild", "Abuild category et_traits", with_deps ? 0 : 1);
    }

    if (! this->this_platform.empty())
    {
	if (this->special_target == s_CLEAN)
	{
	    // Special case: allow clean to be passed to backend when
	    // run from an output directory.
	    this->targets.push_back(this->special_target);
	    this->special_target.clear();
	}
	if (! (this->special_target.empty() &&
	       this->buildset_name.empty() &&
	       this->cleanset_name.empty()))
	{
	    fatal("special targets, build sets, and clean sets may not be"
		  " specified when running inside an output directory");
	}
    }

    // validate platform selectors
    for (std::list<std::string>::iterator iter =
	     platform_selector_strings.begin();
	 iter != platform_selector_strings.end(); ++iter)
    {
	std::string ptype;
	PlatformSelector p;
	if (p.initialize(*iter))
	{
	    // Don't validate ptype or any of the fields themselves.
	    // It's not incorrect to reference non-existent platforms
	    // or platform types here.  That way, people can set the
	    // ABUILD_PLATFORM_SELECTORS environment variable based on
	    // some build tree they work in and not worry about it if
	    // they build in a different tree that doesn't have all
	    // the same platforms and platform types.
	    this->platform_selectors[p.getPlatformType()] = p;
	}
	else
	{
	    QTC::TC("abuild", "Abuild ERR invalid platform selector");
	    error("invalid platform selector " + *iter);
	}
    }

    if (this->targets.empty())
    {
	this->targets = default_targets;
    }
}

void
Abuild::checkBuildsetName(std::string const& kind, std::string const& name)
{
    boost::regex name_re("name:(\\S+)");
    boost::regex pattern_re("pattern:(\\S+)");
    boost::smatch match;

    if (boost::regex_match(name, match, name_re))
    {
	std::list<std::string> namelist = Util::split(',', match[1].str());
	this->buildset_named_items.insert(namelist.begin(), namelist.end());
    }
    else if (boost::regex_match(name, match, pattern_re))
    {
	std::string pattern = match[1].str();
	try
	{
	    boost::regex expression(pattern);
	    this->buildset_pattern = pattern;
	}
	catch (boost::bad_expression)
	{
	    QTC::TC("abuild", "Abuild ERR bad buildset pattern");
	    usage("invalid regular expression " + pattern);
	}
    }
    else if (valid_buildsets.count(name) == 0)
    {
	usage("unknown " + kind + " set " + name);
    }
}

std::string
Abuild::findConf()
{
    // Find the first directory at or above the current directory that
    // contains an ItemConfig::FILE_CONF.

    std::string dir = Util::getCurrentDirectory();
    while (true)
    {
	if (Util::fileExists(dir + "/" + ItemConfig::FILE_CONF))
	{
	    break;
	}
	std::string last_dir = dir;
	dir = Util::dirname(dir);
	if (dir == last_dir)
	{
	    fatal("unable to find " + ItemConfig::FILE_CONF +
		  " at or above the current directory");
	}
    }

    return dir;
}

void
Abuild::initializePlatforms()
{
#ifdef _WIN32
    try
    {
	// For now, require cygwin perl.  Although some of the perl
	// scripts try to work with mswin32 perl, abuild doesn't work
	// properly unless the first perl in the path is cygwin perl.
	Util::getProgramOutput(
	    "perl -e \"die unless $^O eq 'cygwin'\" 2>nul:");
	this->have_perl = true;
    }
    catch (QEXC::General& e)
    {
	// We don't have perl
	verbose("cygwin perl not found; not attempting non-Java builds");
    }
#endif

    bool initialize_object_code = true;
    std::string cmd = "\"" + this->abuild_top +
	"/private/bin/get_native_platform_data\"";

#ifdef _WIN32
    if (this->have_perl)
    {
	cmd = "perl " + cmd + " --windows";
    }
    else
    {
	initialize_object_code = false;
    }
#endif

    if (initialize_object_code)
    {
	std::string native_platform_data = Util::getProgramOutput(cmd);
	std::list<std::string> data = Util::splitBySpace(native_platform_data);
	if (data.size() != 3)
	{
	    fatal("unable to parse native platform data (" +
		  native_platform_data +")");
	}
	this->native_os = data.front();
	data.pop_front();
	this->native_cpu = data.front();
	data.pop_front();
	this->native_toolset = data.front();
	data.pop_front();
    }

    loadPlatformData(this->internal_platform_data,
		     this->abuild_top + "/private");
}

void
Abuild::loadPlatformData(PlatformData& platform_data,
			 std::string const& dir)
{
    static std::string component = "[a-zA-Z0-9_-]+";
    static std::string component1or2 =
	component + "(?:\\." + component + ")?";
    static std::string component4or5 =
	component + "\\." + component + "\\." + component + "\\." +
	component1or2;
    boost::regex ignore_re("\\s*(?:#.*)?");
    boost::regex platform_type_re("platform-type (" + component + ")");
    boost::regex platform_re(
	"platform (-lowpri )?(" + component4or5 +
	") -type (" + component + ")");
    boost::regex native_compiler_re(
	"native-compiler (-lowpri )?(" + component1or2 + ")");

    boost::smatch match;

    // Load platform types

    std::string platform_types = dir + "/platform-types";
    if (Util::isFile(platform_types))
    {
	std::list<std::string> lines;
	lines = Util::readLinesFromFile(platform_types);
	int lineno = 0;
	for (std::list<std::string>::iterator iter = lines.begin();
	     iter != lines.end(); ++iter)
	{
	    std::string const& line = *iter;
	    FileLocation location(platform_types, ++lineno, 0);
	    if (boost::regex_match(line, match, ignore_re))
	    {
		// Ignore
	    }
	    else if (boost::regex_match(line, match, platform_type_re))
	    {
		std::string platform_type = match[1].str();
		try
		{
		    platform_data.addPlatformType(
			platform_type, TargetType::tt_object_code);
		}
		catch (QEXC::General& e)
		{
		    QTC::TC("abuild", "Abuild ERR addPlatformType error");
		    error(location, e.what());
		}
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild ERR bad platform type specification");
		error(location, "invalid platform type specification");
	    }
	}
    }

    // Load platforms
    bool try_platforms = true;
#ifdef _WIN32
    if (! this->have_perl)
    {
	try_platforms = false;
    }
#endif

    std::string list_platforms = dir + "/list_platforms";
    if (try_platforms && Util::isFile(list_platforms))
    {
	std::string cmd = list_platforms;
#ifdef _WIN32
	cmd = "perl " + cmd + " --windows";
#endif
	// Perhaps we should pass native_{os,cpu,toolset} to
	// list_platforms somehow either via arguments or environment
	// variables.  Environment variables might be easier, but our
	// getProgramOutput implementation doesn't allow them to be
	// passed at the moment.
	FileLocation location(list_platforms, 0, 0);
	std::string platform_data_output = Util::getProgramOutput(cmd);
	std::istringstream in(platform_data_output);
	std::list<std::string> lines = Util::readLinesFromFile(in);
	for (std::list<std::string>::iterator iter = lines.begin();
	     iter != lines.end(); ++iter)
	{
	    std::string const& line = *iter;
	    if (boost::regex_match(line, match, ignore_re))
	    {
		// Ignore
	    }
	    else if (boost::regex_match(line, match, platform_re))
	    {
		bool lowpri = ! match[1].str().empty();
		std::string platform = match[2].str();
		std::string type = match[3].str();
		try
		{
		    platform_data.addPlatform(platform, type, lowpri);
		    QTC::TC("abuild", "Abuild add platform",
			    lowpri ? 0 : 1);
		}
		catch (QEXC::General& e)
		{
		    QTC::TC("abuild", "Abuild ERR addPlatform error");
		    error(location, e.what());
		}
	    }
	    else if (boost::regex_match(line, match, native_compiler_re))
	    {
		bool lowpri = ! match[1].str().empty();
		std::string compiler = match[2].str();
		std::string platform =
		    this->native_os + "." +
		    this->native_cpu + "." +
		    this->native_toolset + "." +
		    compiler;
		try
		{
		    platform_data.addPlatform(
			platform, PlatformData::pt_NATIVE, lowpri);
		    QTC::TC("abuild", "Abuild add native compiler",
			    lowpri ? 0 : 1);
		}
		catch (QEXC::General& e)
		{
		    QTC::TC("abuild", "Abuild ERR add compiler error");
		    error(location, e.what());
		}
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild ERR bad platform specification");
		error(location, "invalid syntax in list_platforms output");
	    }
	}
    }

    try
    {
	platform_data.check(this->platform_selectors);
    }
    catch (QEXC::General& e)
    {
	fatal("errors detected loading platforms:\n" + e.unparse());
    }
}

bool
Abuild::readConfigs()
{
    // This routine returns true to indicate that we should exit
    // without actually doing any builds.

    if (this->this_platform.empty())
    {
	this->this_config_dir = this->current_directory;
    }
    else
    {
	this->this_config_dir = Util::dirname(this->current_directory);
    }

    if (! Util::isFile(this->this_config_dir + "/" + ItemConfig::FILE_CONF))
    {
	QTC::TC("abuild", "Abuild ERR no abuild.conf");
	fatal(ItemConfig::FILE_CONF + " not found");
    }
    this->this_config = readConfig(this->this_config_dir);

    std::string local_top = findTop();

    // We explicitly make buildtrees local to readConfigs.  By not
    // holding onto it after readConfigs exits, we ensure that we
    // don't have any way to get any information about items not in
    // the build set.  This policy has helped to prevent a number of
    // potential logic errors in which we might try, for one reason or
    // another, to access something we shouldn't be accessing.  Before
    // deciding to change this policy, keep in mind that the build set
    // is closed with respect to dependency relationships, and
    // dependency relationships are subject to the integrity
    // guarantee.  See comments in BuildItem::getReferences for
    // additional discussion.
    BuildTree_map buildtrees;
    std::set<std::string> visiting;
    traverse(buildtrees, local_top, visiting, FileLocation(),
	     "root of local tree");

    // Compute the list of all known traits.  This routine also
    // validates to make sure that any traits specified on the command
    // line are valid.
    computeValidTraits(buildtrees);

    if (this->monitored || this->dump_data)
    {
	monitorOutput("begin-dump-data");
	dumpData(buildtrees);
	monitorOutput("end-dump-data");
    }

    exitIfErrors();

    if (this->dump_data)
    {
	return true;
    }

    if (this->list_traits)
    {
	listTraits();
	return true;
    }

    if (this->list_platforms)
    {
	listPlatforms(buildtrees);
	return true;
    }

    BuildTree& tree_data = *(buildtrees[local_top]);
    BuildItem_map& builditems = tree_data.getBuildItems();
    computeBuildset(builditems);

    if (! this->full_integrity)
    {
	// Note: we do these checks even when cleaning.  Otherwise,
	// shadowed items won't get cleaned in their backing areas on
	// --clean=all, which is not the expected behavior.
	reportIntegrityErrors(buildtrees, this->buildset, local_top);
	if (Error::anyErrors())
	{
	    QTC::TC("abuild", "Abuild integrity errors in buildset");
	}
	exitIfErrors();
    }
    // else all integrity checks have already been made

    exitIfErrors();

    if ((! (this->buildset_name.empty() && this->cleanset_name.empty())) &&
	this->buildset.empty())
    {
	QTC::TC("abuild", "Abuild empty build set");
	info("build set contains no items");
	return true;
    }

    // Construct a list of build item names in reverse dependency
    // order.  This list is used during construction of the build
    // graph.
    std::list<std::string> const& sorted_items =
	tree_data.getSortedItemNames();
    for (std::list<std::string>::const_reverse_iterator iter =
	     sorted_items.rbegin();
	 iter != sorted_items.rend(); ++iter)
    {
	std::string const& item_name = *iter;
	if (this->buildset.count(item_name))
	{
	    this->buildset_reverse_order.push_back(item_name);
	}
    }

    // A false return means that we should continue processing.
    return false;
}

ItemConfig*
Abuild::readConfig(std::string const& dir)
{
    ItemConfig* result = 0;
    try
    {
	result = ItemConfig::readConfig(this->error_handler, dir);
    }
    catch (QEXC::General& e)
    {
	fatal(e.what());
    }
    return result;
}

std::string
Abuild::findTop()
{
    // Find the top directory of the system containing this build item
    // directory.  That is the first directory above us that contains
    // an ItemConfig::FILE_CONF file without a parent-dir key.  Cycles
    // are detected, so this is not an infinite loop.

    std::string dir = this->this_config_dir;

    bool found = false;
    std::set<std::string> seen;
    std::string child_dir;
    FileLocation child_location;
    while (! found)
    {
	if (seen.count(dir))
	{
	    QTC::TC("abuild", "Abuild ERR cycle reading Abuild.conf");
	    fatal("parent-dir loop detected while looking for root of "
		  "build tree");
	}
	seen.insert(dir);

	ItemConfig* config = readConfig(dir);
	FileLocation location = config->getLocation();
	std::string const& parent = config->getParent();

	if (! child_dir.empty())
	{
	    // Make sure the last item's parent has it as a child
	    bool parent_has_child = false;
	    std::list<std::string> const& children = config->getChildren();
	    for (std::list<std::string>::const_iterator iter = children.begin();
		 iter != children.end(); ++iter)
	    {
		if (Util::canonicalizePath(dir + "/" + *iter) == child_dir)
		{
		    parent_has_child = true;
		    break;
		}
	    }
	    if (! parent_has_child)
	    {
		QTC::TC("abuild", "Abuild ERR not child of parent");
		error(child_location, "this directory is not listed as"
		      " a child of its parent");
	    }
	}

	if (parent.empty())
	{
	    found = true;
	}
	else
	{
	    child_dir = dir;
	    child_location = location;
	    dir = Util::canonicalizePath(dir + "/" + parent);
	    if (! Util::fileExists(dir + "/" + ItemConfig::FILE_CONF))
	    {
		QTC::TC("abuild", "Abuild ERR missing parent");
		error(location, "parent " + parent + " has no " +
		      ItemConfig::FILE_CONF);
		fatal("unable to find root of build tree");
	    }
	}
    }

    return dir;
}

void
Abuild::traverse(BuildTree_map& buildtrees, std::string const& top_path,
		 std::set<std::string>& visiting,
		 FileLocation const& referrer,
		 std::string const& description)
{
    if (visiting.count(top_path))
    {
        QTC::TC("abuild", "Abuild ERR backing cycle");
        fatal("backing area/externals cycle detected for " + top_path +
	      " (" + description + ")");
    }

    if (buildtrees.count(top_path))
    {
	return;
    }

    verbose("traversing build tree " + top_path);
    std::string backing_area;
    ItemConfig* config = 0;

    std::string top_conf = top_path + "/" + ItemConfig::FILE_CONF;
    if (Util::isFile(top_conf))
    {
        config = readConfig(top_path);
        if (! config->getParent().empty())
        {
            QTC::TC("abuild", "Abuild ERR build tree root with parent");
	    FileLocation top_location(
		top_path + "/" + ItemConfig::FILE_CONF, 0, 0);
            error(top_location, "this build item sets " + ItemConfig::k_PARENT +
		  " but is referenced as a build tree root (" +
		  description +")");
	    if (! referrer.getFilename().empty())
	    {
		error(referrer, "here is the referring build item");
	    }
            return;
        }
    }

    std::string top_backing = top_path + "/" + FILE_BACKING;
    if (Util::isFile(top_backing))
    {
	readBacking(top_path, backing_area);
    }

    if (config == 0)
    {
        if (backing_area.empty())
        {
            QTC::TC("abuild", "Abuild ERR no build tree top config");
            fatal("no " + ItemConfig::FILE_CONF + " found in " + top_path +
		  " (" + description + ")");
        }
        else
        {
            QTC::TC("abuild", "Abuild Abuild.backing without Abuild.conf");
        }
    }

    // DO NOT RETURN BELOW THIS POINT until after we remove top_path
    // from visiting.
    visiting.insert(top_path);

    // Do a post-order traversal of the backing area/externals tree,
    // traversing each build tree once we have traversed its backing
    // area and externals.

    if (! backing_area.empty())
    {
	verbose("build tree " + top_path + " has backing area " + backing_area);
        traverse(buildtrees, backing_area, visiting,
		 FileLocation(top_backing, 0, 0),
		 "backing area of " + top_path);
	if (buildtrees.count(backing_area) == 0)
	{
	    backing_area.clear();
	}
	verbose("build tree " + top_path +
		": backing area traversal completed");
    }

    std::map<std::string, ExternalData> externals;
    std::map<std::string, ExternalData> backed_externals;
    std::set<std::string> traits;
    std::set<std::string> deleted_items;
    std::list<std::string> plugins;
    if (config)
    {
	std::list<ExternalData> const& config_externals =
	    config->getExternals();
	for (std::list<ExternalData>::const_iterator iter =
		 config_externals.begin();
	     iter != config_externals.end(); ++iter)
        {
	    // We discard external directories that don't point to an
	    // actual build tree here.  This can only happen in an
	    // error condition in which other error messages have been
	    // issued.
	    ExternalData external_data = *iter;
	    std::string const& declared_path = external_data.getDeclaredPath();
	    if (Util::isAbsolutePath(declared_path))
	    {
		QTC::TC("abuild", "Abuild absolute external");
		external_data.setAbsolutePath(
		    Util::canonicalizePath(declared_path));
	    }
	    else
	    {
		external_data.setAbsolutePath(
		    Util::canonicalizePath(top_path + "/" + declared_path));
	    }

            if (Util::isDirectory(external_data.getAbsolutePath()))
            {
		std::string const& absolute_path =
		    external_data.getAbsolutePath();
		verbose("build tree " + top_path + " has external " +
			absolute_path);
                traverse(buildtrees, absolute_path, visiting,
			 FileLocation(top_conf, 0, 0),
			 "external of " + top_path);
		verbose("build tree " + top_path +
			": traversal of external " + absolute_path +
			" completed");
                if (buildtrees.count(absolute_path))
                {
                    externals[declared_path] = external_data;
                }
            }
            else
            {
		if (haveExternal(buildtrees, backing_area, external_data))
                {
		    verbose("build tree " + top_path + " resolved external " +
			    external_data.getAbsolutePath() +
			    " in backing area");
                    QTC::TC("abuild", "Abuild external in backing area");
                    backed_externals[declared_path] = external_data;
		}
		else
		{
                    QTC::TC("abuild", "Abuild ERR can't resolve external");
                    error(FileLocation(top_conf, 0, 0),
			  "unable to find external " + declared_path);
                }
            }
        }

	traits = config->getSupportedTraits();
	deleted_items = config->getDeleted();
	plugins = config->getPlugins();
    }
    else
    {
	// If and only if we don't have a root build item, it's okay
	// to inherit our traits from our backing area.
	assert(buildtrees.count(backing_area) != 0);
	traits = buildtrees[backing_area]->getSupportedTraits();
	if (! traits.empty())
	{
	    QTC::TC("abuild", "Abuild inherit traits from backing area");
	}
    }

    visiting.erase(top_path);
    buildtrees[top_path].reset(
	new BuildTree(backing_area, externals, backed_externals,
		      traits, deleted_items, plugins,
		      this->internal_platform_data));

    // Traverse build items defined locally and then perform a series
    // of analysis and validation.
    if (config)
    {
	verbose("build tree " + top_path + ": processing build items");
	traverseItems(buildtrees, top_path);
    }

    BuildTree& tree_data = *(buildtrees[top_path]);

    resolveItems(buildtrees, top_path);

    BuildItem_map& builditems = tree_data.getBuildItems();

    // Many of these checks have side effects.  The order of these
    // checks is very sensitive as some checks depend upon side
    // effects of other operations having been completed.
    verbose("build tree " + top_path + ": validating");
    resolveTraits(buildtrees, top_path);
    checkPlatformTypes(tree_data, builditems, top_path);
    checkPlugins(tree_data, builditems, top_path);
    checkItemNames(builditems, top_path);
    checkDependencies(tree_data, builditems, top_path);
    updatePlatformTypes(tree_data, builditems, top_path);
    checkDependencyPlatformTypes(builditems);
    checkFlags(builditems);
    checkTraits(tree_data, builditems, top_path);
    checkIntegrity(buildtrees, top_path);
    if (this->full_integrity)
    {
	reportIntegrityErrors(buildtrees, builditems, top_path);
    }
    computeBuildablePlatforms(tree_data, builditems, top_path);
    verbose("build tree " + top_path + ": traversal completed");
}

void
Abuild::traverseItems(BuildTree_map& buildtrees,
		      std::string const& top_path)
{
    BuildItem_map builditems;
    std::map<std::string, std::string> paths;
    BuildTree& tree_data = *(buildtrees[top_path]);
    std::string const& backing_area = tree_data.getBackingArea();

    std::list<std::string> dirs;
    dirs.push_back(top_path);
    while (! dirs.empty())
    {
	std::string dir = dirs.front();
	dirs.pop_front();
	std::string tree_relative = Util::absToRel(dir, top_path);
        ItemConfig* config = readConfig(dir);
	FileLocation location = config->getLocation();

	std::string item_this = config->getName(); // may be empty
        paths[tree_relative] = item_this;
        if (! item_this.empty())
        {
            if (builditems.count(item_this))
            {
		std::string other_dir =
		    builditems[item_this]->getAbsolutePath();
		if (dir == other_dir)
		{
		    QTC::TC("abuild", "Abuild ERR item traversal loop");
		    error(builditems[item_this]->getLocation(),
			  "loop detected while reading " +
			  ItemConfig::FILE_CONF + " files: build item " +
			  item_this + " reached by multiple paths");
		    continue;
		}
		else
		{
		    QTC::TC("abuild", "Abuild ERR item multiple locations");
		    error(FileLocation(dir + "/" + ItemConfig::FILE_CONF, 0, 0),
			  "build item " + item_this +
			  " appears in multiple locations");
		    error(builditems[item_this]->getLocation(),
			  "here is another location");
		}
            }
            else
            {
		builditems[item_this].reset(
		    new BuildItem(item_this, config, dir, top_path));
            }
        }

	std::list<std::string> const& children = config->getChildren();
	for (std::list<std::string>::const_iterator iter = children.begin();
	     iter != children.end(); ++iter)
	{
	    std::string child_dir = Util::canonicalizePath(dir + "/" + *iter);
            if (Util::isDirectory(child_dir))
            {
		std::string child_conf =
		    Util::absToRel(child_dir + "/" + ItemConfig::FILE_CONF);
                if (Util::isFile(child_conf))
                {
                    ItemConfig* child_config = readConfig(child_dir);
		    dirs.push_back(child_dir);
		    std::string ch_parent = child_config->getParent();
		    if (ch_parent.empty())
		    {
			QTC::TC("abuild", "Abuild ERR child no parent");
			error(location,
			      "child " + child_conf + " has no " +
			      ItemConfig::k_PARENT);
		    }
		    else if (Util::canonicalizePath(
				 dir + "/" + *iter + "/" +
				 ch_parent) != dir)
		    {
			QTC::TC("abuild", "Abuild ERR wrong parent");
			error(location, "child " + child_conf + " has " +
			      ItemConfig::k_PARENT +
			      " that doesn't point to this directory");
		    }
                }
                else
                {
                    QTC::TC("abuild", "Abuild ERR no child config");
                    error(location, "child " + child_conf + " is missing");
                }
            }
            else
            {
                // See if this directory is in our backing area
		std::string child_tree_relative =
		    Util::absToRel(child_dir, top_path);
                if (checkBuilditemPath(buildtrees, backing_area,
				       child_tree_relative))
                {
                    QTC::TC("abuild", "Abuild child in backing area");
                    // No need to traverse into children -- traversal
                    // was already done during initial read.
                }
                else
                {
                    QTC::TC("abuild", "Abuild ERR can't resolve child");
                    error(location, "unable to find child " + *iter);
                }
            }
        }
    }

    tree_data.setBuildItems(builditems, paths);
}

void
Abuild::resolveItems(BuildTree_map& buildtrees,
		     std::string const& top_path)
{
    BuildTree& tree_data = *(buildtrees[top_path]);
    std::map<std::string, ExternalData> const& externals =
	tree_data.getExternals();
    BuildItem_map& builditems = tree_data.getBuildItems();
    std::set<std::string> const& deleted_items = tree_data.getDeletedItems();
    std::set<std::string> deleted_in_externals;

    // Resolve any build items visible in our externals.  Anything
    // visible in an external and also visible here is an error.

    for (std::map<std::string, ExternalData>::const_iterator ext_iter =
	     externals.begin();
	 ext_iter != externals.end(); ++ext_iter)
    {
	ExternalData const& external_data = (*ext_iter).second;
	bool read_only_external = external_data.isReadOnly();
	BuildTree& external_tree =
	    *(buildtrees[external_data.getAbsolutePath()]);
	BuildItem_map const& external_items =
	    external_tree.getBuildItems();
	std::set<std::string> const& external_deleted =
	    external_tree.getDeletedItems();
	deleted_in_externals.insert(external_deleted.begin(),
				    external_deleted.end());
	for (BuildItem_map::const_iterator item_iter = external_items.begin();
	     item_iter != external_items.end(); ++item_iter)
	{
	    std::string const& item_name = (*item_iter).first;
	    BuildItem const& item = *((*item_iter).second);
	    if (builditems.count(item_name))
            {
		std::string const& this_path =
		    builditems[item_name]->getAbsolutePath();
		std::string const& other_path = item.getAbsolutePath();
                if (this_path == other_path)
                {
                    // Okay -- perhaps two of our externals share a
                    // common external.
                    QTC::TC("abuild", "Abuild build item multiple paths");
                }
                else
                {
                    QTC::TC("abuild", "Abuild ERR in local and external");
		    error(FileLocation(this_path + "/" + ItemConfig::FILE_CONF,
				       0, 0),
			  "build item " + item_name +
			  " conflicts with an item in an external");
		    error(FileLocation(other_path + "/" + ItemConfig::FILE_CONF,
				       0, 0),
			  "here is another location");
                }
            }
            else
            {
                builditems[item_name].reset(new BuildItem(item));
		builditems[item_name]->incrementExternalDepth();
		if (read_only_external)
		{
		    builditems[item_name]->setReadOnly();
		}
            }
        }
    }

    // Resolve anything visible in our backing area.  In this case,
    // name clashes are normal and just indicate that we are shadowing
    // a backed build item with a local build item.  Avoid copying any
    // items from backing areas that were deleted in externals.

    std::string const& backing_area = tree_data.getBackingArea();
    if (! backing_area.empty())
    {
	BuildItem_map const& backing_items =
	    buildtrees[backing_area]->getBuildItems();
	for (BuildItem_map::const_iterator item_iter = backing_items.begin();
	     item_iter != backing_items.end(); ++item_iter)
	{
	    std::string const& item_name = (*item_iter).first;
	    if (deleted_in_externals.count(item_name))
	    {
		QTC::TC("abuild", "Abuild not copying deleted in external");
	    }
	    else
	    {
		BuildItem const& item = *((*item_iter).second);
		if (builditems.count(item_name))
		{
		    QTC::TC("abuild", "Abuild override build item");
		}
		else
		{
		    // Copy build item information from backing area
		    builditems[item_name].reset(new BuildItem(item));
		    BuildItem& new_item = *(builditems[item_name]);
		    new_item.incrementBackingDepth();
		}
	    }
        }
    }

    // Remove any items that have been marked for deletion.  Any such
    // items must exist and must have external depth = 0 and backing
    // depth > 0.
    FileLocation top_location(top_path + "/" + ItemConfig::FILE_CONF, 0, 0);
    for (std::set<std::string>::const_iterator iter = deleted_items.begin();
	 iter != deleted_items.end(); ++iter)
    {
	std::string const& item_name = *iter;
	if (builditems.count(item_name) == 0)
	{
	    QTC::TC("abuild", "Abuild ERR deleting non-existent item");
	    error(top_location, "unable to delete non-existent build item \"" +
		  item_name + "\"");
	}
	else
	{
	    BuildItem const& item = *(builditems[item_name]);
	    if (! ((item.getBackingDepth() > 0) &&
		   (item.getExternalDepth() == 0)))
	    {
		QTC::TC("abuild", "Abuild ERR deleting unqualified item");
		error(top_location, "unable to delete item \"" + item_name +
		      "\" because it is not in our direct backing chain");
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild deleting item");
		builditems.erase(item_name);
	    }
	}
    }
}

void
Abuild::resolveTraits(BuildTree_map& buildtrees,
		      std::string const& top_path)
{
    // Merge supported traits of this tree's externals into this tree.
    BuildTree& tree_data = *(buildtrees[top_path]);
    std::map<std::string, ExternalData> externals =
	tree_data.getExternals();
    std::map<std::string, ExternalData> const& backed_externals =
	tree_data.getBackedExternals();
    externals.insert(backed_externals.begin(), backed_externals.end());
    for (std::map<std::string, ExternalData>::const_iterator iter =
	     externals.begin();
	 iter != externals.end(); ++iter)
    {
	tree_data.addTraits(
	    buildtrees[(*iter).second.getAbsolutePath()]->getSupportedTraits());
    }
}

void
Abuild::checkPlugins(BuildTree& tree_data,
		     BuildItem_map& builditems,
		     std::string const& top_path)
{
    // Make sure each plugin actually exists, and mark the item as a
    // plugin just for this tree.

    // First, clear the plugin flag for all build items.
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	item.setPlugin(false);
    }

    // Next, set the plugin flag for any build item that is declared
    // as a plugin in this tree.
    FileLocation top_location(top_path + "/" + ItemConfig::FILE_CONF, 0, 0);
    std::list<std::string> const& plugins = tree_data.getPlugins();
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	std::string const& item_name = *iter;
	if (builditems.count(item_name))
	{
	    BuildItem& item = *(builditems[item_name]);
	    item.setPlugin(true);
	    this->plugins_anywhere.insert(item_name);
	    bool item_error = false;
	    // Although whether or not an item has dependencies is not
	    // context-specific, whether or not an item is a plugin
	    // is.
	    if (! item.getDeps().empty())
	    {
		QTC::TC("abuild", "Abuild ERR plugin with dependencies");
		item_error = true;
		error(top_location,
		      "item \"" + item_name + "\" is declared as a plugin,"
		      " but it has dependencies");
	    }

	    // checkPlatformTypes() must have already been called.
	    // Require all plguins to be build items of target type
	    // "all".  This means that the don't build anything or
	    // have an Abuild.interface file.
	    if (item.getTargetType() != TargetType::tt_all)
	    {
		QTC::TC("abuild", "Abuild ERR plugin with target type !all");
		item_error = true;
		error(top_location,
		      "item \"" + item_name + "\" is declared as a plugin,"
		      " but it has a build or non-plugin interface file");
	    }

	    if (item_error)
	    {
		error(item.getLocation(),
		      "here is \"" + item_name + "\"'s " +
		      ItemConfig::FILE_CONF);
	    }
	}
	else
	{
	    QTC::TC("abuild", "Abuild ERR invalid plugin");
	    error(top_location,
		  "plugin \"" + item_name + "\" does not exist");
	}
    }

    // Check to make sure no item depends on a plugin.  Also store the
    // list of plugins this build item should see.  Do this only for
    // items local to this tree.  If an item from another tree depends
    // on a plugin in this tree, that's a shadowed plugin, and it will
    // be reported later.  Also, since the list of plugins is specific
    // to the tree, it would be incorrect to override the plugin list
    // here.
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}
	std::list<std::string> const& deps = item.getDeps();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
	{
	    std::string const& dep_name = (*dep_iter);
	    // If the dependency doesn't exist, we'll be reporting
	    // that later.
	    if (builditems.count(dep_name) != 0)
	    {
		BuildItem& dep = *(builditems[dep_name]);
		if (dep.isPlugin())
		{
		    QTC::TC("abuild", "Abuild ERR item depends on plugin");
		    error(item.getLocation(),
			  "this item depends on \"" + dep_name + "\","
			  " which is declared as a plugin");
		    error(top_location,
			  "the dependency is declared as a plugin here");
		}
	    }
	}

	// Store the list of plugins
	item.setPlugins(plugins);
    }
}

bool
Abuild::isPluginAnywhere(std::string const& item)
{
    return (this->plugins_anywhere.count(item) != 0);
}

void
Abuild::checkPlatformTypes(BuildTree& tree_data,
			   BuildItem_map& builditems,
			   std::string const& top_path)
{
    PlatformData& platform_data = tree_data.getPlatformData();

    // Load any additional platform data from our plugins

    std::list<std::string> const& plugins = tree_data.getPlugins();
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	std::string const& plugin_name = *iter;
	if (builditems.count(plugin_name) != 0)
	{
	    BuildItem& plugin = *(builditems[plugin_name]);
	    loadPlatformData(platform_data, plugin.getAbsolutePath());
	}
    }

    // Validate all native build items to ensure that they have valid
    // platform types.

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);

	// Check only native build items
	if (! item.isLocal())
	{
	    continue;
	}

	FileLocation const& location = item.getLocation();
	std::set<std::string> platform_types = item.getPlatformTypes();

	TargetType::target_type_e target_type = TargetType::tt_all;

	std::map<TargetType::target_type_e, int> target_types;
	for (std::set<std::string>::const_iterator iter =
		 platform_types.begin();
	     iter != platform_types.end(); ++iter)
	{
	    std::string const& platform_type = *iter;
	    if (platform_data.isPlatformTypeValid(platform_type))
	    {
		TargetType::target_type_e target_type =
		    platform_data.getTargetType(platform_type);
		if (target_types.count(target_type) == 0)
		{
		    target_types[target_type] = 1;
		}
		else
		{
		    ++target_types[target_type];
		}
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild ERR unknown platform type");
		error(location,
		      "unknown platform type \"" + *iter + "\"");
	    }
	}
	if (target_types.size() == 1)
	{
	    target_type = (*(target_types.begin())).first;
	    QTC::TC("abuild", "Abuild multiple target types",
		    (*(target_types.begin())).second == 1 ? 0 : 1);
	}
	else if (target_types.size() > 1)
	{
	    QTC::TC("abuild", "Abuild ERR incompatible platform types");
	    error(location,
		  "platforms in different target types may not be mixed");
	}
	else
	{
	    // No platform data -- this item doesn't build anything.
	    target_type = TargetType::tt_all;
	}

	item.setTargetType(target_type);
    }
}

void
Abuild::checkItemNames(BuildItem_map& builditems,
		       std::string const& top_path)
{
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem const& item = *((*iter).second);
	std::string const& item_name = (*iter).first;
	FileLocation const& item_location = item.getLocation();
	std::list<std::string> const& dependencies = item.getDeps();
	for (std::list<std::string>::const_iterator diter =
		 dependencies.begin();
	     diter != dependencies.end(); ++diter)
	{
	    std::string const& dep = *diter;
	    if (! accessibleFrom(builditems, item_name, dep))
	    {
		QTC::TC("abuild", "Abuild ERR inaccessible dep");
		error(item_location,
		      item_name + " may not depend on " + dep +
		      " because it is private to another scope");
	    }
	}
    }
}

bool
Abuild::accessibleFrom(BuildItem_map& builditems,
		       std::string const& accessor_name,
		       std::string const& accessee_name)
{
    boost::regex scope_re("(.*\\.)[^\\.]+");
    boost::smatch match;

    bool accessible = true;
    if (boost::regex_match(accessee_name, match, scope_re))
    {
	// This is not a public build item.  By default, the scope of
	// visibility is the parent of this build item.  We assign
	// "scope" the scope with a trailing period.
	std::string scope = match[1].str();

	std::string visibility;
	if (builditems.count(accessee_name))
	{
	    visibility = builditems[accessee_name]->getVisibleTo();
	}
	if (visibility == "*")
	{
	    // This item is globally visible
	    QTC::TC("abuild", "Abuild globally visible item");
	    scope.clear();
	}
	else if (! visibility.empty())
	{
	    // Removing the trailing * from the visibility gives us
	    // the actual scope.
	    QTC::TC("abuild", "Abuild set scope from visibility");
	    assert(*(visibility.rbegin()) == '*');
	    scope = visibility.substr(0, visibility.length() - 1);
	}

	if (accessor_name.substr(0, scope.length()) == scope)
	{
	    QTC::TC("abuild", "Abuild accessor sees ancestor",
		    (visibility.length() > 1) ? 1 : 0);
	}
	else if (scope == accessor_name + ".")
	{
	    QTC::TC("abuild", "Abuild accessor sees child");
	}
	else
	{
	    accessible = false;
	}
    }
    else
    {
	// This is a public build item.
    }

    return accessible;
}


void
Abuild::checkDependencies(BuildTree& tree_data,
			  BuildItem_map& builditems,
			  std::string const& top_path)
{
    // Make sure that there are no cycles or errors (references to
    // non-existent build items) in the dependency graph.

    // Create a dependency graph for all build items we can see.

    DependencyGraph g;
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	g.addItem(item_name);
	std::list<std::string> const& dependencies = item.getDeps();
	for (std::list<std::string>::const_iterator i2 = dependencies.begin();
	     i2 != dependencies.end(); ++i2)
        {
            g.addDependency(item_name, *i2);
        }
    }

    bool check_okay = g.check();

    // Whether or not we found errors, capture the expanded dependency
    // list for each build item.
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	item.setExpandedDependencies(g.getSortedDependencies(item_name));
    }

    tree_data.setSortedItemNames(g.getSortedGraph());

    if (! check_okay)
    {
	DependencyGraph::ItemMap unknowns;
	std::vector<DependencyGraph::ItemList> cycles;
	g.getErrors(unknowns, cycles);

	for (DependencyGraph::ItemMap::iterator iter = unknowns.begin();
	     iter != unknowns.end(); ++iter)
	{
	    std::string const& node = (*iter).first;
	    DependencyGraph::ItemList const& unknown_items = (*iter).second;
	    for (DependencyGraph::ItemList::const_iterator i2 =
		     unknown_items.begin();
		 i2 != unknown_items.end(); ++i2)
	    {
		std::string const& unknown = *i2;
		QTC::TC("abuild", "Abuild ERR unknown dependency");
		error(builditems[node]->getLocation(),
		      node + " depends on unknown build item " + unknown);
	    }
	}

	std::set<std::string> cycle_items;
	for (std::vector<DependencyGraph::ItemList>::const_iterator iter =
		 cycles.begin();
	     iter != cycles.end(); ++iter)
	{
	    // Don't make this a warning.  See abuild document for a
	    // discussion of circular dependencies and how to resolve
	    // them.
	    DependencyGraph::ItemList const& data = *iter;
	    QTC::TC("abuild", "Abuild ERR circular dependency");
	    std::string cycle = Util::join(" -> ", data);
	    cycle += " -> " + data.front();
	    error("circular dependency detected: " + cycle);
	    for (DependencyGraph::ItemList::const_iterator i2 = data.begin();
		 i2 != data.end(); ++i2)
	    {
		cycle_items.insert(*i2);
	    }
	}

	for (std::set<std::string>::iterator iter = cycle_items.begin();
	     iter != cycle_items.end(); ++iter)
        {
            error(builditems[*iter]->getLocation(),
		  *iter + " participates in a circular dependency");
        }
    }
}

void
Abuild::updatePlatformTypes(BuildTree& tree_data,
			    BuildItem_map& builditems,
			    std::string const& top_path)
{
    // For every build item with target type "all", if all the item's
    // direct dependencies have the same platform types, inherit
    // platform types and target type from them.  We have to perform
    // this check in reverse dependency order so that any dependencies
    // that were originally target type "all" but didn't have to be
    // have already been updated.

    std::list<std::string> const& sorted_items =
	tree_data.getSortedItemNames();
    for (std::list<std::string>::const_iterator iter = sorted_items.begin();
	 iter != sorted_items.end(); ++iter)
    {
	std::string const& item_name = *iter;
	BuildItem& item = *(builditems[item_name]);
	if (! item.isLocal())
	{
	    continue;
	}
	TargetType::target_type_e target_type = item.getTargetType();
	if (target_type != TargetType::tt_all)
	{
	    continue;
	}
	bool candidate = true;
	bool first = true;
	std::string dep_platform_types;
	std::list<std::string> const& deps = item.getDeps();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
	{
	    std::string const& dep_name = *dep_iter;
	    if (builditems.count(dep_name) == 0)
	    {
		candidate = false;
		break;
	    }
	    BuildItem& dep = *(builditems[dep_name]);
	    if (dep.getTargetType() == TargetType::tt_all)
	    {
		candidate = false;
		break;
	    }
	    std::string platform_types =
		Util::join(" ", dep.getPlatformTypes());
	    if (first)
	    {
		dep_platform_types = platform_types;
		first = false;
	    }
	    else if (dep_platform_types == platform_types)
	    {
		QTC::TC("abuild", "Abuild non-trivial update platform types");
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild platform type difference");
		candidate = false;
		break;
	    }
	}
	if (first || (! candidate))
	{
	    continue;
	}

	// All of this item's dependencies have the same list of
	// platform types as each other.  Inherit platform types and
	// target type from this item.
	QTC::TC("abuild", "Abuild inherit platform types");
	// First build item is known to exist and be valid
	assert(builditems.count(deps.front()));
	BuildItem& first_dep = *(builditems[deps.front()]);
	item.setPlatformTypes(first_dep.getPlatformTypes());
	item.setTargetType(first_dep.getTargetType());
    }
}

void
Abuild::checkDependencyPlatformTypes(BuildItem_map& builditems)
{
    // Verify that each dependency that is declared with a specific
    // platform type is from an item whose target type is not tt_all
    // and to an item that has the given platform type.  Must be
    // called after updatePlatformTypes.

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem const& item = *((*iter).second);
	FileLocation const& item_location = item.getLocation();
	bool has_any_dep_platform_types = false;
	std::list<std::string> const& deps = item.getDeps();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
	{
	    std::string const& dep_name = *dep_iter;
	    if (builditems.count(dep_name) == 0)
	    {
		// error reported elsewhere
		continue;
	    }
	    PlatformSelector const* ps = 0;
	    std::string dep_platform_type =
		item.getDepPlatformType(dep_name, ps);
	    if (dep_platform_type.empty())
	    {
		// no platform type declared for this dependency
		continue;
	    }
	    if (ps)
	    {
		dep_platform_type = ps->getPlatformType();
	    }
	    has_any_dep_platform_types = true;
	    // Ensure that the dependency has this platform type
	    BuildItem& dep = *(builditems[dep_name]);
	    if (dep_platform_type == PlatformData::pt_INDEP)
	    {
		QTC::TC("abuild", "Abuild ERR dep ptype indep");
		error(item_location, "dependencies may not be declared"
		      " with platform type \"" + dep_platform_type + "\"");
	    }
	    else if (dep.getPlatformTypes().count(dep_platform_type) == 0)
	    {
		QTC::TC("abuild", "Abuild ERR dep doesn't have dep ptype");
		error(item_location, "dependency \"" + dep_name + "\" declared"
		      " with platform type \"" + dep_platform_type + "\" "
		      ", which dependency does not have");
	    }
	}
	if (has_any_dep_platform_types &&
	    (item.getTargetType() == TargetType::tt_all))
	{
	    QTC::TC("abuild", "Abuild ERR all with ptype deps");
	    error(item_location, "this item has no platform types, so it may"
		  " not declare platform types on its dependencies");
	}
    }
}

void
Abuild::checkFlags(BuildItem_map& builditems)
{
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem const& item = *((*iter).second);
	std::string const& item_name = (*iter).first;
	FileLocation const& item_location = item.getLocation();
	FlagData const& flag_data = item.getFlagData();
	std::set<std::string> dependencies = flag_data.getNames();
	for (std::set<std::string>::const_iterator diter =
		 dependencies.begin();
	     diter != dependencies.end(); ++diter)
	{
	    std::string const& dep_name = *diter;
	    if (builditems.count(dep_name) == 0)
	    {
		// error already reported
		QTC::TC("abuild", "Abuild skipping flag for unknown dep");
		continue;
	    }
	    BuildItem const& dep_item = *(builditems[dep_name]);
	    std::set<std::string> const& flags = flag_data.getFlags(dep_name);
	    for (std::set<std::string>::const_iterator fiter = flags.begin();
		 fiter != flags.end(); ++fiter)
	    {
		if (! dep_item.supportsFlag(*fiter))
		{
		    QTC::TC("abuild", "Abuild ERR unsupported flag");
		    error(item_location,
			  item_name + " may not specify flag " +
			  *fiter + " for dependency " + dep_name +
			  " because " + dep_name +
			  " does not support that flag");
		}
	    }
	}
    }
}

void
Abuild::checkTraits(BuildTree& tree_data,
		    BuildItem_map& builditems,
		    std::string const& top_path)
{
    // For each build item, make sure each trait is allowed in the
    // current build tree and that any referent build items are
    // accessible.  Check only items that are native to the current
    // tree.

    std::set<std::string> const& supported_traits =
	tree_data.getSupportedTraits();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (item.getTreeTop() != top_path)
	{
	    continue;
	}
	std::string const& item_name = (*iter).first;
	FileLocation const& location = item.getLocation();
	TraitData::trait_data_t const& trait_data =
	    item.getTraitData().getTraitData();
	for (TraitData::trait_data_t::const_iterator titer = trait_data.begin();
	     titer != trait_data.end(); ++titer)
	{
	    std::string const& trait = (*titer).first;
	    if (supported_traits.count(trait) == 0)
	    {
		QTC::TC("abuild", "Abuild ERR unsupported trait");
		error(location, "trait " + trait +
		      " is not supported in this item's build tree");
	    }
	    std::set<std::string> const& items = (*titer).second;
	    for (std::set<std::string>::const_iterator iiter = items.begin();
		 iiter != items.end(); ++iiter)
	    {
		std::string const& referent_item = *iiter;
		if (! accessibleFrom(builditems, item_name, referent_item))
		{
		    QTC::TC("abuild", "Abuild ERR inaccessible trait referent");
		    error(location, "trait " + trait +
			  " refers to item " + referent_item +
			  " which is private to another scope");
		}
	    }
	}
    }
}

void
Abuild::checkIntegrity(BuildTree_map& buildtrees,
		       std::string const& top_path)
{
    // Check abuild's basic integrity guarantee.  Refer to the manual
    // for details.

    BuildItem_map& builditems = buildtrees[top_path]->getBuildItems();

    std::set<std::string> plugin_check_trees;

    // Find items with shadowed dependencies.  An item has a shadowed
    // dependency if the local tree resolves the item to a different
    // path than the item's native tree.  Native items therefore can't
    // have shadowed dependencies.  (They can still have dependencies
    // with shadowed dependencies, so they can still have integrity
    // errors.)  Also store a list of all build trees for which we
    // have to perform plugin checks.
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (item.isLocal())
	{
	    continue;
	}

	std::string const& item_tree = item.getTreeTop();
	plugin_check_trees.insert(item_tree);
	BuildItem_map& item_tree_items = buildtrees[item_tree]->getBuildItems();
	std::set<std::string> shadowed_dependencies;

	std::list<std::string> const& deps = item.getExpandedDependencies();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
        {
	    std::string const& dep_name = (*dep_iter);
	    if ((builditems.count(dep_name) == 0) ||
		(item_tree_items.count(dep_name) == 0))
	    {
		// unknown dependency error already reported
		continue;
	    }

	    if (builditems[dep_name]->getAbsolutePath() !=
		item_tree_items[dep_name]->getAbsolutePath())
	    {
		// The instance of dep_name that item would see is
		// shadowed.
		shadowed_dependencies.insert(dep_name);
	    }
        }

	item.setShadowedDependencies(shadowed_dependencies);
    }

    // Create a mapping of name to path for each plugin that is
    // declared by the local tree and also present natively in the
    // local tree.  Any such plugin could potentially be shadowing a
    // plugin in a more distant tree.
    BuildTree& tree_data = *(buildtrees[top_path]);
    std::list<std::string> const& plugins = tree_data.getPlugins();
    std::map<std::string, std::string> plugin_paths;
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	std::string const& plugin_name = *iter;
	if (builditems.count(plugin_name) == 0)
	{
	    // error already reported
	    continue;
	}
	BuildItem& plugin = *(builditems[plugin_name]);
	plugin_paths[plugin_name] = plugin.getAbsolutePath();
    }

    // For each plugin check tree, see if that tree defines any of the
    // same plugins the local tree does and resolves them to a
    // different location.
    for (std::set<std::string>::const_iterator iter =
	     plugin_check_trees.begin();
	 iter != plugin_check_trees.end(); ++iter)
    {
	std::string const& other_top_path = *iter;
	BuildTree& other_tree = *(buildtrees[other_top_path]);
	std::list<std::string> const& other_plugins = other_tree.getPlugins();
	for (std::list<std::string>::const_iterator plugin_iter =
		 other_plugins.begin();
	     plugin_iter != other_plugins.end(); ++plugin_iter)
	{
	    std::string const& plugin = *plugin_iter;
	    if (plugin_paths.count(plugin) == 0)
	    {
		// We don't care about this plugin
		continue;
	    }
	    BuildItem_map& other_items = other_tree.getBuildItems();
	    if (other_items.count(plugin) == 0)
	    {
		// error already reported
		continue;
	    }
	    if (other_items[plugin]->getAbsolutePath() !=
		plugin_paths[plugin])
	    {
		this->shadowed_plugins[top_path][other_top_path].
		    push_back(plugin);
	    }
        }
    }

    // For each non-native build item, if it references shadowed
    // plugins, store this fact.

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (item.isLocal())
	{
	    continue;
	}

	std::string const& item_tree = item.getTreeTop();
	if ((this->shadowed_plugins.count(top_path) != 0) &&
	    (this->shadowed_plugins[top_path].count(item_tree) != 0))
	{
	    item.addShadowedPlugin(top_path, item_tree);
	}
    }
}

void
Abuild::reportIntegrityErrors(BuildTree_map& buildtrees,
			      BuildItem_map& builditems,
			      std::string const& top_path)
{
    std::set<std::string> integrity_error_items;
    std::map<std::string, std::set<std::string> > plugin_error_trees;

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	if (! item.getShadowedDependencies().empty())
	{
	    integrity_error_items.insert(item_name);
	}
	std::map<std::string, std::set<std::string> > const&
	    item_shadowed_plugins = item.getShadowedPlugins();
	for (std::map<std::string, std::set<std::string> >::const_iterator i2 =
		 item_shadowed_plugins.begin();
	     i2 != item_shadowed_plugins.end(); ++i2)
	{
	    std::string const& local_tree = (*i2).first;
	    if (this->full_integrity && (local_tree != top_path))
	    {
		// In full integrity mode, let each tree report its
		// own errors.
		continue;
	    }
	    std::set<std::string> const& remote_trees = (*i2).second;
	    for (std::set<std::string>::const_iterator i3 =
		     remote_trees.begin();
		 i3 != remote_trees.end(); ++i3)
	    {
		std::string const& remote_tree = *i3;
		plugin_error_trees[local_tree].insert(remote_tree);
	    }
	}
    }

    for (std::set<std::string>::const_iterator iter =
	     integrity_error_items.begin();
	 iter != integrity_error_items.end(); ++iter)
    {
	std::string const& item_name = *iter;
	BuildItem& item = *(builditems[item_name]);
	std::set<std::string> const& shadowed_dependencies =
	    item.getShadowedDependencies();
	for (std::set<std::string>::const_iterator dep_iter =
		 shadowed_dependencies.begin();
	     dep_iter != shadowed_dependencies.end(); ++dep_iter)
	{
	    std::string const& dep_name = *dep_iter;
	    BuildItem& dep = *(builditems[dep_name]);
	    QTC::TC("abuild", "Abuild ERR dep inconsistency");
	    error(item.getLocation(), "build item \"" + item_name +
		  "\" depends on \"" + dep_name + "\", which is shadowed");
	    error(dep.getLocation(),
		  "here is the location of \"" + dep_name + "\"");
	}
    }

    for (std::map<std::string, std::set<std::string> >::const_iterator i1 =
	     plugin_error_trees.begin();
	 i1 != plugin_error_trees.end(); ++i1)
    {
	std::string const& local_tree = (*i1).first;
	if (this->full_integrity && (local_tree != top_path))
	{
	    // In full integrity mode, let each tree report its
	    // own errors.
	    continue;
	}
	std::set<std::string> const& remote_trees = (*i1).second;
	for (std::set<std::string>::const_iterator i2 = remote_trees.begin();
	     i2 != remote_trees.end(); ++i2)
	{
	    std::string const& remote_tree = *i2;
	    std::vector<std::string> plugins =
		this->shadowed_plugins[local_tree][remote_tree];
	    FileLocation local_tree_location(
		local_tree + "/" + ItemConfig::FILE_CONF, 0, 0);
	    FileLocation remote_tree_location(
		remote_tree + "/" + ItemConfig::FILE_CONF, 0, 0);
	    BuildItem_map& local_items = buildtrees[local_tree]->getBuildItems();
	    QTC::TC("abuild", "Abuild ERR plugin integrity");
	    error(remote_tree_location,
		  "some plugins declared by this tree are shadowed");
	    error(local_tree_location,
		  "this is the tree that contains the shadowing plugins");
	    for (std::vector<std::string>::const_iterator p_iter =
		     plugins.begin();
		 p_iter != plugins.end(); ++p_iter)
	    {
		std::string const& plugin_name = *p_iter;
		BuildItem& plugin = *(local_items[plugin_name]);
		error(plugin.getLocation(),
		      "here is the shadowing copy of plugin \"" +
		      plugin_name + "\"");
	    }
	}
    }
}

void
Abuild::computeBuildablePlatforms(BuildTree& tree_data,
				  BuildItem_map& builditems,
				  std::string const& top_path)
{
    // For each build item, determine the list of platforms that are
    // to be built for that item.  Do this only for items that are
    // native to the local tree.
    PlatformData& platform_data = tree_data.getPlatformData();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}
	std::set<std::string> const& platform_types =
	    item.getPlatformTypes();
	std::set<std::string> build_platforms;
	for (std::set<std::string>::const_iterator i2 =
		 platform_types.begin();
	     i2 != platform_types.end(); ++i2)
        {
	    std::string const& platform_type = *i2;
	    if (platform_data.isPlatformTypeValid(platform_type))
	    {
		// Add all platforms for the platform type to the
		// buildable platforms list for the platform type.
		// Then add each one that is to build by default to
		// the build platforms list.  Additional build
		// platforms may be added later.
		PlatformData::selected_platforms_t const& platforms =
		    platform_data.getPlatformsByType(platform_type);
		std::vector<std::string> buildable_platforms;
		for (PlatformData::selected_platforms_t::const_iterator i3 =
			 platforms.begin();
		     i3 != platforms.end(); ++i3)
		{
		    std::string const& platform = (*i3).first;
		    bool selected = (*i3).second;
		    buildable_platforms.push_back(platform);
		    if (selected)
		    {
			build_platforms.insert(platform);
		    }
		}
		item.setBuildablePlatforms(platform_type, buildable_platforms);
	    }
        }
        item.setBuildPlatforms(build_platforms);
    }
}

void
Abuild::readBacking(std::string const& dir,
		    std::string& backing_area)
{
    std::string file = dir + "/" + FILE_BACKING;
    std::list<std::string> lines = Util::readLinesFromFile(file);

    { // local scope
	// Remove comments and blank lines
	std::list<std::string>::iterator iter = lines.begin();
	while (iter != lines.end())
	{
	    std::list<std::string>::iterator next = iter;
	    ++next;
	    if (((*iter).length() == 0) || ((*iter)[0] == '#'))
	    {
		lines.erase(iter, next);
	    }
	    iter = next;
	}
    }

    if (lines.size() == 1)
    {
	backing_area = lines.front();
	if (! Util::isAbsolutePath(backing_area))
	{
	    backing_area = dir + "/" + backing_area;
	}
	backing_area = Util::canonicalizePath(backing_area);
    }
    else
    {
        QTC::TC("abuild", "Abuild ERR invalid backing file");
        fatal(file + ": invalid syntax");
    }
}

bool
Abuild::haveExternal(BuildTree_map& buildtrees,
		     std::string const& backing_top,
		     ExternalData& external)
{
    if (backing_top.empty())
    {
	return false;
    }

    std::string const& declared_path = external.getDeclaredPath();

    // Don't attempt to resolve absolute path externals.
    if (Util::isAbsolutePath(declared_path))
    {
	return false;
    }

    if (! buildtrees.count(backing_top))
    {
	fatal("Abuild::getExternal: no build tree data for " + backing_top);
    }
    BuildTree& tree_data = *(buildtrees[backing_top]);

    std::map<std::string, ExternalData> const& externals =
	tree_data.getExternals();
    bool result = false;
    if (externals.count(declared_path))
    {
        result = true;
	external.setAbsolutePath(
	    (*(externals.find(declared_path))).second.getAbsolutePath());
    }
    else
    {
	std::string const& backing_area = tree_data.getBackingArea();
        result = haveExternal(buildtrees, backing_area, external);
    }
    return result;
}

bool
Abuild::checkBuilditemPath(BuildTree_map& buildtrees,
			   std::string const& backing_top,
			   std::string const& tree_relative)
{
    if (backing_top.empty())
    {
	return false;
    }
    BuildTree& tree_data = *(buildtrees[backing_top]);
    bool result = false;
    if (tree_data.getPaths().count(tree_relative))
    {
	result = true;
    }
    else
    {
	std::string const& backing_area = tree_data.getBackingArea();
	result = checkBuilditemPath(buildtrees, backing_area, tree_relative);
    }
    return result;
}

void
Abuild::computeValidTraits(BuildTree_map& buildtrees)
{
    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	BuildTree& tree = *((*iter).second);
	std::set<std::string> const& traits = tree.getSupportedTraits();
	this->valid_traits.insert(traits.begin(), traits.end());
    }

    std::list<std::string> all_traits = this->only_with_traits;
    all_traits.insert(all_traits.end(),
		      this->related_by_traits.begin(),
		      this->related_by_traits.end());
    for (std::list<std::string>::iterator iter = all_traits.begin();
	 iter != all_traits.end(); ++iter)
    {
	if (this->valid_traits.count(*iter) == 0)
	{
	    QTC::TC("abuild", "Abuild ERR unknown trait");
	    error("trait " + *iter + " is unknown");
	}
    }
}

void
Abuild::listTraits()
{
    if (this->valid_traits.empty())
    {
	QTC::TC("abuild", "Abuild listTraits with no traits");
	std::cout << "No traits are supported." << std::endl;
    }
    else
    {
	QTC::TC("abuild", "Abuild listTraits");
	std::cout << "The following traits are supported:" << std::endl;
	for (std::set<std::string>::iterator iter = this->valid_traits.begin();
	     iter != this->valid_traits.end(); ++iter)
	{
	    std::cout << "  " << *iter << std::endl;
	}
    }
}

void
Abuild::listPlatforms(BuildTree_map& buildtrees)
{
    for (BuildTree_map::iterator tree_iter = buildtrees.begin();
	 tree_iter != buildtrees.end(); ++tree_iter)
    {
	bool tree_name_output = false;
	BuildTree& tree_data = *((*tree_iter).second);
	PlatformData& platform_data = tree_data.getPlatformData();
	std::set<std::string> const& platform_types =
	    platform_data.getPlatformTypes(TargetType::tt_object_code);
	for (std::set<std::string>::const_iterator pt_iter =
		 platform_types.begin();
	     pt_iter != platform_types.end(); ++pt_iter)
	{
	    bool platform_type_output = false;
	    std::string const& platform_type = *pt_iter;
	    PlatformData::selected_platforms_t const& platforms =
		platform_data.getPlatformsByType(platform_type);
	    for (PlatformData::selected_platforms_t::const_iterator p_iter =
			 platforms.begin();
		 p_iter != platforms.end(); ++p_iter)
	    {
		if (! tree_name_output)
		{
		    std::cout << "tree " << (*tree_iter).first << std::endl;
		    tree_name_output = true;
		}
		if (! platform_type_output)
		{
		    std::cout << "  platform type " << platform_type
			      << std::endl;
		    platform_type_output = true;
		}
		std::string const& platform = (*p_iter).first;
		bool selected = (*p_iter).second;
		std::cout << "    platform " << platform
			  << "; built by default: "
			  << (selected ? "yes" : "no")
			  << std::endl;
	    }
	}
    }
}

void
Abuild::dumpData(BuildTree_map& buildtrees)
{
    this->logger.flushLog();

    std::ostream& o = std::cout;

    // Create a dependency graph of build trees based on backing areas
    // and externals.  Output build trees in order based on that graph.

    DependencyGraph g;
    for (BuildTree_map::iterator tree_iter = buildtrees.begin();
	 tree_iter != buildtrees.end(); ++tree_iter)
    {
	std::string const& path = (*tree_iter).first;
	g.addItem(path);
	BuildTree& tree = *((*tree_iter).second);
	std::string const& backing_area = tree.getBackingArea();
	if (! backing_area.empty())
	{
	    g.addDependency(path, backing_area);
	}
	std::map<std::string, ExternalData> const& externals =
	    tree.getExternals();
	for (std::map<std::string, ExternalData>::const_iterator iter =
		 externals.begin();
	     iter != externals.end(); ++iter)
	{
	    g.addDependency(path, (*iter).second.getAbsolutePath());
	}
    }
    // Even if there were errors, the build tree graph must always be
    // consistent.  Abuild doesn't store invalid backing areas or
    // externals in the build tree structures.
    assert(g.check());
    std::list<std::string> const& allTrees = g.getSortedGraph();

    o << "<?xml version=\"1.0\"?>" << std::endl
      << "<abuild-data version=\"1\"";
    if (Error::anyErrors())
    {
	o << " errors=\"1\"";
    }
    o << ">" << std::endl;
    dumpPlatformData(this->internal_platform_data, " ");
    if (! this->valid_traits.empty())
    {
	o << " <supported-traits>" << std::endl;
	for (std::set<std::string>::iterator iter = this->valid_traits.begin();
	     iter != this->valid_traits.end(); ++iter)
	{
	    o << "  <supported-trait name=\"" << *iter << "\"/>" << std::endl;
	}
	o << " </supported-traits>" << std::endl;
    }

    o << " <build-trees>" << std::endl;

    std::map<std::string, int> tree_numbers;
    int cur_tree = 0;
    for (std::list<std::string>::const_iterator tree_iter = allTrees.begin();
	 tree_iter != allTrees.end(); ++tree_iter)
    {
	std::string const& tree_path = *tree_iter;
	tree_numbers[tree_path] = ++cur_tree;
	BuildTree& tree_data = *(buildtrees[tree_path]);
	std::string const& backing_area = tree_data.getBackingArea();
	std::map<std::string, ExternalData> const& externals =
	    tree_data.getExternals();
	std::map<std::string, ExternalData> const& backed_externals =
	    tree_data.getBackedExternals();
	BuildItem_map& builditems = tree_data.getBuildItems();
	std::set<std::string> const& traits = tree_data.getSupportedTraits();
	std::set<std::string> const& deleted_items =
	    tree_data.getDeletedItems();
	std::list<std::string> const& plugins = tree_data.getPlugins();

	o << "  <build-tree" << std::endl
	  << "   id=\"bt-" << cur_tree << "\"" << std::endl
	  << "   absolute-path=\"" << tree_path << "\"" << std::endl
	  << "  >" << std::endl;
	dumpPlatformData(tree_data.getPlatformData(), "   ");
	if (! traits.empty())
	{
	    o << "   <supported-traits>" << std::endl;
	    for (std::set<std::string>::const_iterator trait_iter =
		     traits.begin();
		 trait_iter != traits.end(); ++trait_iter)
	    {
		o << "    <supported-trait name=\"" << *trait_iter << "\"/>"
		  << std::endl;
	    }
	    o << "   </supported-traits>" << std::endl;
	}
	if (! backing_area.empty())
	{
	    o << "   <backing-area build-tree=\"bt-"
	      << tree_numbers[backing_area]
	      << "\"/>" << std::endl;
	}
	for (std::map<std::string, ExternalData>::const_iterator iter =
		 externals.begin();
	     iter != externals.end(); ++iter)
	{
	    o << "   <external build-tree=\"bt-"
	      << tree_numbers[(*iter).second.getAbsolutePath()] << "\"";
	    if ((*iter).second.isReadOnly())
	    {
		o << " read-only=\"1\"";
	    }
	    o << "/>" << std::endl;
	}
	for (std::map<std::string, ExternalData>::const_iterator iter =
		 backed_externals.begin();
	     iter != backed_externals.end(); ++iter)
	{
	    o << "   <external build-tree=\"bt-"
	      << tree_numbers[(*iter).second.getAbsolutePath()]
	      << "\" backed=\"1\"";
	    if ((*iter).second.isReadOnly())
	    {
		o << " read-only=\"1\"";
	    }
	    o << "/>" << std::endl;
	}
	if (! deleted_items.empty())
	{
	    o << "   <deleted-items>" << std::endl;
	    for (std::set<std::string>::const_iterator iter =
		     deleted_items.begin();
		 iter != deleted_items.end(); ++iter)
	    {
		o << "    <deleted-item name=\"" + *iter + "\"/>" << std::endl;
	    }
	    o << "   </deleted-items>" << std::endl;
	}
	if (! plugins.empty())
	{
	    o << "   <plugins>" << std::endl;
	    for (std::list<std::string>::const_iterator iter = plugins.begin();
		 iter != plugins.end(); ++iter)
	    {
		o << "    <plugin name=\"" + *iter + "\"/>" << std::endl;
	    }
	    o << "   </plugins>" << std::endl;
	}

	std::list<std::string> const& sorted_items =
	    tree_data.getSortedItemNames();

	for (std::list<std::string>::const_iterator item_iter =
		 sorted_items.begin();
	     item_iter != sorted_items.end(); ++item_iter)
	{
	    std::string const& name = (*item_iter);
	    BuildItem& item = *(builditems[name]);
	    dumpBuildItem(item, name, tree_numbers);
        }
	o << "  </build-tree>" << std::endl;
    }

    o << " </build-trees>" << std::endl
      << "</abuild-data>" << std::endl;
}

void
Abuild::dumpPlatformData(PlatformData const& platform_data,
			 std::string const& indent)
{
    std::ostream& o = std::cout;

    o << indent << "<platform-data>" << std::endl;
    static TargetType::target_type_e target_types[] = {
	TargetType::tt_platform_independent,
	TargetType::tt_object_code,
	TargetType::tt_java
    };
    static int const ntarget_types =
	sizeof(target_types) / sizeof(TargetType::target_type_e);

    for (int i = 0; i < ntarget_types; ++i)
    {
	std::set<std::string> const& platform_types =
	    platform_data.getPlatformTypes(target_types[i]);
	for (std::set<std::string>::const_iterator i1 = platform_types.begin();
	     i1 != platform_types.end(); ++i1)
	{
	    std::string const& platform_type = *i1;
	    o << indent << " <platform-type name=\""
	      << platform_type << "\" target-type=\""
	      << TargetType::getName(target_types[i]) << "\"";
	    PlatformData::selected_platforms_t const& platforms =
		platform_data.getPlatformsByType(platform_type);
	    if (platforms.empty())
	    {
		o << "/>" << std::endl;
	    }
	    else
	    {
		o << ">" << std::endl;
		for (PlatformData::selected_platforms_t::const_iterator i2 =
			 platforms.begin();
		     i2 != platforms.end(); ++i2)
		{
		    std::string const& name = (*i2).first;
		    bool selected = (*i2).second;
		    o << indent << "  <platform name=\"" << name
		      << "\" selected=\"" << (selected ? "1" : "0")
		      << "\"/>"
		      << std::endl;
		}
		o << indent << " </platform-type>" << std::endl;
	    }
	}
    }
    o << indent << "</platform-data>" << std::endl;
}

void
Abuild::dumpBuildItem(BuildItem& item, std::string const& name,
		      std::map<std::string, int>& tree_numbers)
{
    std::ostream& o = std::cout;

    std::string description = Util::XMLify(item.getDescription(), true);
    std::string visible_to = item.getVisibleTo();
    std::list<std::string> const& declared_dependencies =
	item.getDeps();
    std::list<std::string> const& expanded_dependencies =
	item.getExpandedDependencies();
    std::set<std::string> const& platform_types =
	item.getPlatformTypes();
    std::set<std::string> const& buildable_platforms =
	item.getBuildablePlatforms();
    std::set<std::string> const& supported_flags =
	item.getSupportedFlags();
    TraitData::trait_data_t const& traits =
	item.getTraitData().getTraitData();
    bool any_subelements =
	(! (declared_dependencies.empty() &&
	    expanded_dependencies.empty() &&
	    platform_types.empty() &&
	    buildable_platforms.empty() &&
	    supported_flags.empty() &&
	    traits.empty()));

    o << "   <build-item" << std::endl
      << "    name=\"" << name << "\"" << std::endl;
    if (! description.empty())
    {
	o << "    description=\"" << description << "\"" << std::endl;
    }
    o << "    home-tree=\"bt-"
      << tree_numbers[item.getTreeTop()] << "\"" << std::endl
      << "    absolute-path=\"" << item.getAbsolutePath() << "\""
      << std::endl
      << "    backing-depth=\"" << item.getBackingDepth() << "\""
      << std::endl
      << "    external-depth=\"" << item.getExternalDepth() << "\""
      << std::endl;
    if (item.hasShadowedReferences())
    {
	o << "    has-shadowed-references=\"1\"" << std::endl;
    }
    if (! visible_to.empty())
    {
	o << "    visible-to=\"" << visible_to << "\"" << std::endl;
    }
    o << "    target-type=\""
      << TargetType::getName(item.getTargetType()) << "\""
      << std::endl
      << "    is-plugin=\"" << (item.isPlugin() ? "1" : "0") << "\""
      << std::endl
      << "    is-plugin-anywhere=\""
      << (isPluginAnywhere(name) ? "1" : "0") << "\""
      << std::endl;
    if (any_subelements)
    {
	FlagData const& flag_data = item.getFlagData();
	o << "   >" << std::endl;
	if (! declared_dependencies.empty())
	{
	    o << "    <declared-dependencies>" << std::endl;
	    for (std::list<std::string>::const_iterator dep_iter =
		     declared_dependencies.begin();
		 dep_iter != declared_dependencies.end(); ++dep_iter)
	    {
		o << "     <dependency name=\"" << *dep_iter << "\"";
		std::string const& platform_type =
		    item.getDepPlatformType(*dep_iter);
		if (! platform_type.empty())
		{
		    o << " platform-type=\"" << platform_type << "\"";
		}
		if (! flag_data.hasFlags(*dep_iter))
		{
		    o << "/>" << std::endl;
		}
		else
		{
		    std::set<std::string> const& flags =
			flag_data.getFlags(*dep_iter);
		    o << ">" << std::endl;
		    for (std::set<std::string>::const_iterator f_iter =
			     flags.begin();
			 f_iter != flags.end(); ++f_iter)
		    {
			o << "      <flag name=\"" << *f_iter << "\"/>"
			  << std::endl;
		    }
		    o << "     </dependency>" << std::endl;
		}
	    }
	    o << "    </declared-dependencies>" << std::endl;
	}
	if (! expanded_dependencies.empty())
	{
	    o << "    <expanded-dependencies>" << std::endl;
	    for (std::list<std::string>::const_iterator dep_iter =
		     expanded_dependencies.begin();
		 dep_iter != expanded_dependencies.end(); ++dep_iter)
	    {
		o << "     <dependency name=\"" << *dep_iter << "\"/>"
		  << std::endl;
	    }
	    o << "    </expanded-dependencies>" << std::endl;
	}
	if (! platform_types.empty())
	{
	    o << "    <platform-types>" << std::endl;
	    for (std::set<std::string>::const_iterator pt_iter =
		     platform_types.begin();
		 pt_iter != platform_types.end(); ++pt_iter)
	    {
		o << "     <platform-type name=\"" << *pt_iter << "\"/>"
		  << std::endl;
	    }
	    o << "    </platform-types>" << std::endl;
	}
	if (! buildable_platforms.empty())
	{
	    o << "    <buildable-platforms>" << std::endl;
	    for (std::set<std::string>::const_iterator p_iter =
		     buildable_platforms.begin();
		 p_iter != buildable_platforms.end(); ++p_iter)
	    {
		o << "     <platform name=\"" << *p_iter << "\"/>"
		  << std::endl;
	    }
	    o << "    </buildable-platforms>" << std::endl;
	}
	if (! supported_flags.empty())
	{
	    o << "    <supported-flags>" << std::endl;
	    for (std::set<std::string>::const_iterator f_iter =
		     supported_flags.begin();
		 f_iter != supported_flags.end(); ++f_iter)
	    {
		o << "     <supported-flag name=\"" << *f_iter << "\"/>"
		  << std::endl;
	    }
	    o << "    </supported-flags>" << std::endl;
	}
	if (! traits.empty())
	{
	    o << "    <traits>" << std::endl;
	    for (TraitData::trait_data_t::const_iterator t_iter = traits.begin();
		 t_iter != traits.end(); ++t_iter)
	    {
		std::string const& trait = (*t_iter).first;
		std::set<std::string> const& referents = (*t_iter).second;
		o << "     <trait name=\"" << trait << "\"";
		if (referents.empty())
		{
		    o << "/>" << std::endl;
		}
		else
		{
		    o << ">" << std::endl;
		    for (std::set<std::string>::const_iterator r_iter =
			     referents.begin();
			 r_iter != referents.end(); ++r_iter)
		    {
			o << "      <trait-referent name=\"" << *r_iter
			  << "\"/>" << std::endl;
		    }
		    o << "     </trait>" << std::endl;
		}
	    }
	    o << "    </traits>" << std::endl;
	}
	o << "   </build-item>" << std::endl;
    }
    else
    {
	o << "   />" << std::endl;
    }
}

void
Abuild::computeBuildset(BuildItem_map& builditems)
{
    // Generate the build set.
    std::string const& this_name = this->this_config->getName();
    BuildItem_ptr this_builditem;
    if (! this_name.empty())
    {
        if (builditems.count(this_name) == 0)
        {
            // Can't happen without some other error such as
            // parent/child consistency
            fatal("INTERNAL ERROR: the current build item is not known");
        }
        this_builditem = builditems[this_name];
    }

    std::string set_name = this->buildset_name;
    if (set_name.empty())
    {
	set_name = this->cleanset_name;
    }

    bool cleaning = false;
    if (! set_name.empty())
    {
	cleaning = (! this->cleanset_name.empty());
	if (! this->buildset_named_items.empty())
	{
	    std::set<std::string> named_items =
		this->buildset_named_items;
            QTC::TC("abuild", "Abuild buildset names", cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isNamed, _1,
					 boost::ref(named_items)));
	    if (! named_items.empty())
	    {
		std::string unknown_items = Util::join(", ", named_items);
		QTC::TC("abuild", "Abuild ERR unknown items in set");
		error("unable to add unknown build items to build set: " +
		      unknown_items);
	    }
	}
	else if (! this->buildset_pattern.empty())
	{
	    boost::regex pattern(this->buildset_pattern);
	    QTC::TC("abuild", "Abuild buildset pattern", cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::matchesPattern, _1,
					 pattern));
	}
	else if (set_name == b_ALL)
        {
            QTC::TC("abuild", "Abuild buildset all", cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isWritable, _1));
        }
        else if (set_name == b_LOCAL)
        {
            QTC::TC("abuild", "Abuild buildset local", cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isLocal, _1));
        }
        else if (set_name == b_DESC)
        {
            QTC::TC("abuild", "Abuild buildset desc", cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isAtOrBelowPath,
					 _1, this->current_directory));
        }
        else if (set_name == b_DEPS)
        {
            if (this_builditem.get())
            {
                QTC::TC("abuild", "Abuild buildset deps", cleaning ? 1 : 0);
		std::list<std::string> const& deps =
		    this_builditem->getExpandedDependencies();
		for (std::list<std::string>::const_iterator iter =
			 deps.begin();
		     iter != deps.end(); ++iter)
		{
		    std::string const& dep = *iter;
		    this->buildset[dep] = builditems[dep];
		}
            }
        }
        else if (set_name == b_CURRENT)
        {
            if (this_builditem.get())
            {
                QTC::TC("abuild", "Abuild buildset current", cleaning ? 1 : 0);
		this->buildset[this_name] = this_builditem;
            }
        }
        else
        {
            fatal("INTERNAL ERROR: unknown build/clean set " + set_name);
        }
    }
    else if (this_builditem.get())
    {
	cleaning = (this->special_target == s_CLEAN);
        this->buildset[this_name] = this_builditem;
    }

    if ((! this->apply_targets_to_deps) &&
	(this->related_by_traits.empty()))
    {
	// Only build items that were initially selected for
	// membership in the build set get explicit targets.  Add them
	// now before we expand the build set to include dependencies.
	// If we have any related by traits, we'll add only those
	// items later.
	for (BuildItem_map::iterator iter = this->buildset.begin();
	     iter != this->buildset.end(); ++iter)
	{
	    QTC::TC("abuild", "Abuild add selected to explicit targets");
	    this->explicit_target_items.insert((*iter).first);
	}
    }

    // We always add dependencies of any items initiallyh in the build
    // set to the build set if we are building.  If we are cleaning,
    // we only do this when the --apply-targets-to-deps option is
    // specified.
    bool add_dependencies = (! cleaning) || this->apply_targets_to_deps;
    bool expanding = (add_dependencies || (! this->related_by_traits.empty()));
    bool expanded_by_traits = false;

    // Expand the build set to include dependencies of all items in
    // the build set.  If we are also expanding because of expand
    // traits, then after expanding based on dependencies, expand
    // based on traits.  Then expand one more time based on
    // dependencies so that anything added because of traits gets its
    // dependencies added as well.  Note that we do not repeat the
    // process again, so if anything is related to one of the last
    // group of dependency-based expansions by an expand trait, it
    // doesn't get added to the set.  (If the build set contains A,
    // the expand set contains trait tester, A-tester tests A, and
    // A-testers depends on X, then A, A-tester, and X all get
    // added.  If X is tested by X-tester, X-tester does not get
    // added.)
    while (expanding)
    {
	if (add_dependencies)
	{
	    std::set<std::string> to_add;
	    for (BuildItem_map::iterator iter = this->buildset.begin();
		 iter != this->buildset.end(); ++iter)
	    {
		BuildItem& item = *((*iter).second);
		std::list<std::string> const& alldeps =
		    item.getExpandedDependencies();
		for (std::list<std::string>::const_iterator dep_iter =
			 alldeps.begin();
		     dep_iter != alldeps.end(); ++dep_iter)
		{
		    to_add.insert(*dep_iter);
		}
	    }
	    for (std::set<std::string>::iterator iter = to_add.begin();
		 iter != to_add.end(); ++iter)
	    {
		this->buildset[*iter] = builditems[*iter];
	    }
	}

	if (expanded_by_traits || this->related_by_traits.empty())
	{
	    expanding = false;
	}
	else
	{
	    QTC::TC("abuild", "Abuild expand by trait", cleaning ? 1 : 0);

	    // Add to the build set any item that has all the traits
	    // named in the related by traits in reference any item
	    // already in the build set.
	    std::set<std::string> to_add;
	    // For each item, ...
	    for (BuildItem_map::iterator iter = builditems.begin();
		 iter != builditems.end(); ++iter)
	    {
		std::string const& item_name = (*iter).first;
		BuildItem& item = *((*iter).second);
		// get the list of traits. ...
		TraitData::trait_data_t const& trait_data =
		    item.getTraitData().getTraitData();
		// For each related by trait, ...
		bool missing_any_traits = false;
		for (std::list<std::string>::iterator titer =
			 this->related_by_traits.begin();
		     titer != this->related_by_traits.end(); ++titer)
		{
		    // see if the item has the trait.  If so, ...
		    if (trait_data.count(*titer) != 0)
		    {
			bool refers_to_item_in_buildset = false;
			// see if any of the referent items belong to
			// the build set, and add them if they do.
			std::set<std::string> const& referent_items =
			    (*(trait_data.find(*titer))).second;
			for (std::set<std::string>::const_iterator riter =
				 referent_items.begin();
			     riter != referent_items.end(); ++riter)
			{
			    if (this->buildset.count(*riter))
			    {
				refers_to_item_in_buildset = true;
				break;
			    }
			}
			if (! refers_to_item_in_buildset)
			{
			    missing_any_traits = true;
			}
		    }
		    else
		    {
			missing_any_traits = true;
		    }
		    if (missing_any_traits)
		    {
			break;
		    }
		}
		if (! missing_any_traits)
		{
		    to_add.insert(item_name);
		}
	    }
	    for (std::set<std::string>::iterator iter = to_add.begin();
		 iter != to_add.end(); ++iter)
	    {
		// Add the item to the build set, and also add it to
		// the list of items that are built with explicit
		// targets.
		this->buildset[*iter] = builditems[*iter];
		if (! this->apply_targets_to_deps)
		{
		    this->explicit_target_items.insert(*iter);
		}
	    }
	    expanded_by_traits = true;
	}
    }

    // Expand the build set to include all plugins of all items in the
    // buildset.
    std::set<std::string> to_add;
    for (BuildItem_map::iterator iter = this->buildset.begin();
	 iter != this->buildset.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	std::list<std::string> const& plugins = item.getPlugins();
	for (std::list<std::string>::const_iterator p_iter = plugins.begin();
	     p_iter != plugins.end(); ++p_iter)
	{
	    to_add.insert(*p_iter);
	}
    }
    for (std::set<std::string>::iterator iter = to_add.begin();
	 iter != to_add.end(); ++iter)
    {
	this->buildset[*iter] = builditems[*iter];
    }

    if (this->buildset.empty())
    {
        QTC::TC("abuild", "Abuild empty buildset");
    }

    if (this->apply_targets_to_deps)
    {
	// If all items get explicit targets, add all items now that
	// we've completed construction of the build set.
	QTC::TC("abuild", "Abuild add all to explicit targets",
		cleaning ? 0 : 1);
	for (BuildItem_map::iterator iter = this->buildset.begin();
	     iter != this->buildset.end(); ++iter)
	{
	    this->explicit_target_items.insert((*iter).first);
	}
    }
}

void
Abuild::populateBuildset(BuildItem_map& builditems,
			 boost::function<bool(BuildItem const*)> pred)
{
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem_ptr item_ptr = (*iter).second;
	if (pred(item_ptr.get()) && item_ptr->hasTraits(this->only_with_traits))
	{
	    this->buildset[item_name] = item_ptr;
	}
    }
}

bool
Abuild::buildBuildset()
{
    std::string const& this_name = this->this_config->getName();
    if (this->local_build &&
        (this_name.empty() ||
	 (! this_config->hasBuildFile())))
    {
        QTC::TC("abuild", "Abuild no local build");
        info("nothing to build in this directory");
        return true;
    }

    verbose("constructing build graph");

    // Initialize the build_graph dependency graph for items in the
    // build set taking build platforms into consideration.  Also
    // determine which backends we need.  We must call
    // addItemToBuildGraph in reverse dependency order.  See comments
    // there for details.

    bool some_native_items_skipped = false;
    bool need_gmake = false;
    bool need_ant = false;
    for (std::vector<std::string>::const_iterator iter =
	     this->buildset_reverse_order.begin();
	 iter != this->buildset_reverse_order.end(); ++iter)
    {
	std::string const& item_name = *iter;
	BuildItem& item = *(this->buildset[item_name]);
	if (addItemToBuildGraph(item_name, item))
	{
	    // We only check the back end if the item was added to the
	    // build graph.  This makes it possible, for example, to
	    // build Java items in a mixed language build tree when no
	    // C/C++ platform are available or vice versa.

	    // omit default so gcc will warn for missing cases
	    switch (item.getBackend())
	    {
	      case ItemConfig::b_make:
		need_gmake = true;
		break;

	      case ItemConfig::b_ant:
		need_ant = true;
		break;

	      case ItemConfig::b_none:
		// no backend requirements
		break;
	    }
	}
	else
	{
	    QTC::TC("abuild", "Abuild some items skipped");
	    verbose("skipping build item " + item_name +
		    " because there are no platforms for its platform types");
	    std::set<std::string> const& platform_types =
		item.getPlatformTypes();
	    if (platform_types.count("native"))
	    {
		some_native_items_skipped = true;
	    }
	}
    }

    if ((! need_ant) && (! need_gmake) && some_native_items_skipped)
    {
	QTC::TC("abuild", "Abuild some native items skipped");
	info("some native items were skipped because there are"
	     " no valid native platforms");
#ifdef _WIN32
	if (! this->have_perl)
	{
	    info("Note that cygwin perl and a properly configured compiler"
		 " are required to support native builds on Windows.");
	}
#endif
    }

    exitIfErrors();
    if (! this->build_graph.check())
    {
        fatal("INTERNAL ERROR: graph traversal errors while sorting"
              " final platform-aware dependency graph");
    }

    if (this->dump_build_graph)
    {
	boost::function<void(std::string const&)> o =
	    boost::bind(&Logger::logInfo, &(this->logger), _1);

	DependencyGraph::ItemList const& items =
	    this->build_graph.getSortedGraph();
	for (DependencyGraph::ItemList::const_iterator iter =
		 items.begin();
	     iter != items.end(); ++iter)
	{
	    DependencyGraph::ItemList const& deps =
		this->build_graph.getSortedDependencies(*iter);
	    DependencyGraph::ItemList::const_reverse_iterator dep_iter =
		deps.rbegin();
	    std::string msg = *dep_iter + " -> ";
	    ++dep_iter;
	    if (dep_iter == deps.rend())
	    {
		msg += "<none>";
	    }
	    else
	    {
		msg += Util::join(" ", dep_iter, deps.rend());
	    }
	    o(msg);
	}

	return true;
    }

    if (need_gmake)
    {
	findGnuMakeInPath();
    }
    if (need_ant)
    {
	findAnt();
    }

    boost::function<bool(std::string const&, std::string const&)> filter;
    if (this->local_build)
    {
        if (this->this_platform.empty())
        {
            // build current item for all platforms
	    filter = boost::bind(&Abuild::isThisItem, this, _1, _2);
        }
        else
        {
            // build current item for current platform
	    filter = boost::bind(&Abuild::isThisItemThisPlatform, this, _1, _2);
        }
    }
    else
    {
        // build all items in buildset
	filter = boost::bind(&Abuild::isAnyItem, this, _1, _2);
    }

    // Load base interface.  Use a name that can't possibly conflict
    // with any build item name.
    InterfaceParser base_parser(":base", ":base", this->abuild_top);
    if (! base_parser.parse(
	    this->abuild_top + "/private/base.interface", false))
    {
	fatal("errors detected in base interface file");
    }
    this->base_interface = base_parser.getInterface();

    // Load interfaces for each plugin.
    bool plugin_interface_errors = false;
    for (BuildItem_map::iterator iter = this->buildset.begin();
	 iter != this->buildset.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	if (isPluginAnywhere(item_name))
	{
	    verbose("loading interface for plugin " + item_name);
	    if (! createPluginInterface(item_name, item))
	    {
		plugin_interface_errors = true;
	    }
	}
    }
    if (plugin_interface_errors)
    {
	fatal("errors detected in plugin interface files");
    }

    // Set variables whose values are global for all items.
    FileLocation internal("<global-initialization>", 0, 0);
    assert(this->base_interface->assignVariable(
	       internal, "ABUILD_STDOUT_IS_TTY",
	       (this->stdout_is_tty ? "1" : "0"),
	       Interface::a_normal));

    // Build appropriate items in dependency order.
    DependencyRunner r(this->build_graph, this->max_workers,
		       boost::bind(&Abuild::itemBuilder, this, _1, filter));
    if (this->monitored)
    {
	r.setChangeCallback(boost::bind(&Abuild::stateChangeCallback,
					this, _1, _2), true);
    }
    verbose("starting build");
    this->logger.flushLog();
    bool stop_on_error = true;
    bool disable_failure_propagation = false;
    if (this->keep_going)
    {
	stop_on_error = false;
	if (this->no_dep_failures)
	{
	    disable_failure_propagation = true;
	}
    }
    return r.run(stop_on_error, disable_failure_propagation);
}

bool
Abuild::addItemToBuildGraph(std::string const& item_name, BuildItem& item)
{
    TargetType::target_type_e item_type = item.getTargetType();
    std::list<std::string> const& deps = item.getDeps();
    std::set<std::string> const& build_platforms =
	item.getBuildPlatforms();

    if (build_platforms.empty())
    {
	return false;
    }

    // The build graph is the real dependency graph that abuild
    // actually builds from.  Rather than being a simple dependency
    // graph between items, as we constructed and verified eariler,
    // this one is between item/platform pairs.

    // Each item has a a list of "buildable" platforms.  This is the
    // list of all platforms on which the item could be built.  Each
    // item also has a list of "build" platforms.  This is the list of
    // platforms on which we will actually build this item.  It is
    // always a subset (which may contain all members) of the
    // buildable platform list.

    // For purposes of constructing buildable and build platform
    // lists, there are three cases.

    // The first and simplest case is platform-independent items.
    // These are items that explicitly declare platform type "indep".
    // Their buildable platform list is always "indep", and their
    // build platform list is also always "indep".

    // The second case is for items of target type "java" or
    // "object-code".  Although, in fact, there is only one java
    // platform type or platform, nothing in the code knows or cares
    // about this, and for purposes of build/buildable platform
    // creation, java and object-code items are treated in the same
    // way.  For these items, the list of buildable platforms is the
    // list of all platforms that belong to any declared platform
    // type.  The list of build platforms is initially selected from
    // the buildable platforms based on selection criteria (documented
    // elsewhere) and may be expanded to include any platforms that a
    // reverse depenedency needs to build on.  For example, suppose A
    // depends on B, A can be built only with compiler c1, and B can
    // be built with c1 and c2.  If B would be built only with c2
    // based on the selection criteria, A's requirement of c1 would
    // cause B to also be built with c1.  We refer to this as
    // "opportunistic platform selection."  In order for this to work
    // properly, we must be able to have analysis of a particular item
    // for build graph insertion be able to modify that item's
    // dependencies.  This is the reason that we have to add items to
    // the build graph in reverse dependency order.

    // The third case is for items of target type "all".  These items,
    // by definition, have no platform types declared.  They also have
    // no Abuild.interface files or build files.  Their sole purpose
    // is connect things that depend on them with things that they
    // depend on.  Since the build operation for such targets is
    // empty, they do not explicitly maintain a "buildable" platform
    // list; they could be "built" on any platform.  The "build"
    // platform list for items of type "all" is simply the union of
    // the build sets of all their reverse dependencies.

    // The logic of setting up the build graph is quite subtle, and
    // although the code itself is fairly, simple, there are actually
    // quite a number of cases.  Each node in the build graph is a
    // string of the form item_name:platform.  For every item i and
    // every platform p in its build platforms, we add the node i:p to
    // the build graph.  We create a dependency from A:p1 to B:p2 in
    // the build graph whenever A's build on p1 depends on B's build
    // on p2.

    // In the normal case of A:p1 -> B:p2, p1 = p2.  There are two
    // exceptions to this rule:

    //  * If B is platform-independent, then p2 is be "indep"
    //    regardless of what p1 is.

    //  * If the dependency from A to B in A's Abuild.conf is declared
    //    with platform type pt, A must not have target type "all",
    //    and B must have pt as one of its platform types.  In this
    //    case, p2 is the highest priority platform in pt that appears
    //    in B's buildable platform list.  If there are no matching
    //    platforms, it is an error.

    // Ordinarily, for each possible A:p1, if there is no suitable p2
    // in B to create the dependency link, it is an error.  There is
    // one exception to this rule:

    //  * If A is of target type "all", then for each p1 in A's build
    //    platform list, if B cannot be built on p1, the dependency
    //    between A and B is simply ignored for that platform.

    // It is this that allows an item of target type "all" to be a
    // platform-type or even language-independent "pass through"
    // between items that depend on it and items that it depends
    // on.  For example, suppose object-code item O1 and Java item
    // J1 both depend on A and A depends on both object-code item
    // O2 and Java item J2.  Suppose O1 and O2 build on platform p1
    // and J1 and J2 builds on platform p2.  Then A builds on
    // platforms p1 and p2.  In this case, we have A:p1 -> O2:p1 and
    // A:p2 -> J2:p2.  There is no dependency between A:p2 and O2 or
    // between A:p1 and J2.

    for (std::set<std::string>::const_iterator bp_iter =
	     build_platforms.begin();
	 bp_iter != build_platforms.end(); ++bp_iter)
    {
	// For each build platform, add the item/platform pair to the
	// build graph.  Use a colon to separate the item from the
	// platform since colon is an invalid character for both items
	// and platform identifiers.  This ensures that the resulting
	// string will be unambiguous.
	this->build_graph.addItem(item_name + ":" + *bp_iter);
    }

    for (std::list<std::string>::const_iterator dep_iter =
	     deps.begin();
	 dep_iter != deps.end(); ++dep_iter)
    {
	std::string const& dep = *dep_iter;
	BuildItem& dep_item = *(this->buildset[dep]);
	TargetType::target_type_e dep_type = dep_item.getTargetType();
	std::set<std::string> const& dep_platforms =
	    dep_item.getBuildablePlatforms();
	PlatformSelector const* ps = 0;
	std::string const& dep_platform_type = item.getDepPlatformType(dep, ps);

	std::string override_platform;
	if (! dep_platform_type.empty())
	{
	    // If we have declared a specific platform type with this
	    // dependency, pick the best platform of that type from
	    // the dependency's buildabhle platforms.  We have already
	    // verified (in checkDependencyPlatformTypes) that the
	    // item is not of type all, that the dependency has this
	    // platform type, and that the requested platform type is
	    // not "indep".  If there are no qualifying platforms, it
	    // is an error.

	    override_platform =
		dep_item.getBestPlatformForType(dep_platform_type, ps);
	    if (override_platform.empty())
	    {
		QTC::TC("abuild", "Abuild ERR no override platform");
		error(item.getLocation(),
		      "\"" + item_name + "\" wants dependency \"" + dep +
		      "\" on platform type \"" + dep_platform_type +
		      "\", but the dependency has no platforms belonging to"
		      " that type");
		error(dep_item.getLocation(),
		      "here is the location of \"" + dep + "\"");
		continue;
	    }
	}

	for (std::set<std::string>::const_iterator bp_iter =
		 build_platforms.begin();
	     bp_iter != build_platforms.end(); ++bp_iter)
	{
	    std::string const& item_platform = *bp_iter;
	    std::string platform_item = item_name + ":" + item_platform;

	    if (dep_type == TargetType::tt_platform_independent)
	    {
		// Allow any item to depend on a platform-independent
		// item
		QTC::TC("abuild", "Abuild item -> indep",
			((item_platform == PlatformData::PLATFORM_INDEP)
			 ? 0 : 1));
		this->build_graph.addDependency(
		    platform_item, dep + ":" + PlatformData::PLATFORM_INDEP);
	    }
	    else if (! override_platform.empty())
	    {
		// If an override platform is available (a specific
		// platform type was declared on this dependency),
		// create a dependency on that specific platform.
		QTC::TC("abuild", "Abuild override platform");
		dep_item.addBuildPlatform(override_platform);
		this->build_graph.addDependency(
		    platform_item, dep + ":" + override_platform);
	    }
	    else if ((dep_type == TargetType::tt_all) ||
		     (dep_platforms.count(item_platform)))
	    {
		// If the dependency is buildable on the current
		// platform, create a link from the item on this
		// platform to the dependency on the same platform.
		// Note that if the dependency is of type "all", it
		// implicity is buildable on all platforms.  We
		// explicitly add the platform to the dependency's
		// build platform list.  This is how opportunistic
		// platform selection happens.
		QTC::TC("abuild", "Abuild A:p -> B:p",
			((dep_type == TargetType::tt_all) ? 0 : 1));
		dep_item.addBuildPlatform(item_platform);
		this->build_graph.addDependency(
		    platform_item, dep + ":" + item_platform);
	    }
	    else if (item_type == TargetType::tt_all)
	    {
		// If an item is of type all, ignore the case of a
		// dependency not having this platform.  This is how
		// pass-through build items work.
		QTC::TC("abuild", "Abuild ignoring all's dep without platform");
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild ERR unmatched platform");
		error(item.getLocation(),
		      "\"" + item_name + "\" is being built on platform \"" +
		      item_platform + "\", but its dependency \"" +
		      dep + "\" is not");
		error(dep_item.getLocation(),
		      "here is the location of \"" + dep + "\"");
	    }
	}
    }

    return true;
}

void
Abuild::findGnuMakeInPath()
{
    boost::regex gmake_re(".*GNU Make (\\d+)\\.(\\d+).*");
    boost::smatch match;

    std::list<std::string> candidates = Util::findProgramInPath("gmake");
    std::list<std::string> make_candidates = Util::findProgramInPath("make");
    candidates.insert(candidates.end(),
		      make_candidates.begin(), make_candidates.end());

    for (std::list<std::string>::iterator iter = candidates.begin();
	 iter != candidates.end(); ++iter)
    {
	std::string const& candidate = *iter;
	std::string version_string;
	try
	{
	    version_string = Util::getProgramOutput(
		"\"" + candidate + "\" --version");
	    if (boost::regex_match(version_string, match, gmake_re))
	    {
		int major_version = atoi(match[1].str().c_str());
		int minor_version = atoi(match[2].str().c_str());
		if (((major_version == 3) && (minor_version >= 81)) ||
		    (major_version > 3))
		{
		    this->gmake = candidate;
		    break;
		}
	    }
	}
	catch (QEXC::General)
	{
	    // This isn't gnu make; try the next candidate.
	}
    }

    if (this->gmake.empty())
    {
	fatal("gnu make >= 3.81 is required");
    }
}

void
Abuild::findAnt()
{
    boost::regex ant_re("Apache Ant version (\\d+)\\.(\\d+).*");
    boost::smatch match;

    std::list<std::string> candidates;
    std::string ant_home;
    if (Util::getEnv("ANT_HOME", &ant_home))
    {
	candidates.push_back(Util::canonicalizePath(ant_home + "/bin/ant"));
    }
    else
    {
	candidates = Util::findProgramInPath("ant");
    }
    for (std::list<std::string>::iterator iter = candidates.begin();
	 iter != candidates.end(); ++iter)
    {
	std::string const& candidate = *iter;
	try
	{
	    std::string version_string =
		Util::getProgramOutput("\"" + candidate + "\" -version");
	    if (boost::regex_match(version_string, match, ant_re))
	    {
		int major_version = atoi(match[1].str().c_str());
		int minor_version = atoi(match[2].str().c_str());
		if (((major_version == 1) && (minor_version >= 7)) ||
		    (major_version > 1))
		{
		    this->ant = candidate;
		    break;
		}
	    }
	}
	catch (QEXC::General)
	{
	    // This isn't Apache Ant; try the next candidate.
	}
    }

    if (this->ant.empty())
    {
	fatal("ant >= 1.7 is required");
    }

    // If we found ant, try finding the AbuildLogger.  As a special
    // case, if we are running out of the source tree, try to find it
    // in the build directory.  Otherwise, try to find it in lib.

    std::string abuild_dir = Util::dirname(this->program_fullpath);
    std::string base = Util::basename(abuild_dir);
    if (base.substr(0, OUTPUT_DIR_PREFIX.length()) == OUTPUT_DIR_PREFIX)
    {
	std::string candidate =
	    abuild_dir +
	    "/../ant-library/abuild-java/dist/abuild-ant-library.jar";
	// We're running from the source directory
	if (Util::isFile(candidate))
	{
	    this->ant_library = candidate;
	}
    }

    // Abuild with ant will fail without ant_library except when
    // bootstrapping because it will try to load a custom ant task.
    // We handle bootstrapping by setting a property in ant-library's
    // Abuild-ant.properties.  When not bootstrapping, it would be
    // nice if we could fail here if abuild-ant-library.jar is not
    // found, but this would require us to actually invoke abuild with
    // special flags during the bootstrapping process or to otherwise
    // be able to tell.
    if (this->ant_library.empty())
    {
	std::string candidate =
	    this->abuild_top + "/lib/abuild-ant-library.jar";
	if (Util::isFile(candidate))
	{
	    this->ant_library = candidate;
	}
    }
}

bool
Abuild::isThisItemThisPlatform(std::string const& name,
			       std::string const& platform)
{
    return (name == this->this_config->getName() &&
	    platform == this->this_platform);
}

bool
Abuild::isThisItem(std::string const& name, std::string const&)
{
    return (name == this->this_config->getName());
}

bool
Abuild::isAnyItem(std::string const&, std::string const&)
{
    return true;
}

bool
Abuild::itemBuilder(
    std::string builder_string,
    boost::function<bool(std::string const&, std::string const&)> filter)
{
    boost::smatch match;
    boost::regex builder_re(BUILDER_RE);
    assert(boost::regex_match(builder_string, match, builder_re));
    std::string item_name = match[1].str();
    std::string item_platform = match[2].str();
    BuildItem& build_item = *(this->buildset[item_name]);

    // If we are trying to build an item with shadowed dependencies or
    // plugins, then we have a weak point in the integrity checks that
    // needs to be fixed.  Just in case we do, block building of this
    // item here.  If we were actually to attempt to build this item,
    // we might build an item in a backing area with references to an
    // item in our local tree, which would be very bad.  If we were to
    // just skip this item, then certain things the user execpted to
    // get built would silently be ignored.
    assert(! build_item.hasShadowedReferences());

    bool no_op = (this->special_target == s_NO_OP);

    std::string const& abs_path = build_item.getAbsolutePath();
    InterfaceParser parser(item_name, builder_string, abs_path);
    parser.setSupportedFlags(build_item.getSupportedFlags());
    bool status = true;

    if (! no_op)
    {
	status = createItemInterface(
	    builder_string, item_name, item_platform, build_item, parser);
    }

    if (build_item.hasBuildFile())
    {
	// Build the item if we are supposed to.  Otherwise, assume it is
	// built.
	if (status && filter(item_name, item_platform))
	{
	    status = buildItem(item_name, item_platform, build_item);
	}

	// If all is well, read any after-build files.  Disallow them
	// from declaring their own after-build files.
	if (status && (! no_op))
	{
	    status = readAfterBuilds(
		item_platform, item_platform, build_item, parser);
	}
    }
    else
    {
	if (! parser.getAfterBuilds().empty())
	{
	    QTC::TC("abuild", "Abuild ERR after-build with no build file");
	    std::string interface_file =
		abs_path + "/" + ItemConfig::FILE_INTERFACE;
	    error(FileLocation(interface_file, 0, 0),
		  "interfaces for items with no build files may not load"
		  " after-build files");
	    status = false;
	}
	else
	{
	    QTC::TC("abuild", "Abuild not building with no build file",
		    build_item.getTargetType() == TargetType::tt_all ? 0 : 1);
	}
    }

    return status;
}

void
Abuild::stateChangeCallback(std::string const& builder_string,
			    DependencyEvaluator::ItemState state)
{
    boost::smatch match;
    boost::regex builder_re(BUILDER_RE);
    assert(boost::regex_match(builder_string, match, builder_re));
    std::string item_name = match[1].str();
    std::string item_platform = match[2].str();
    std::string item_state = DependencyEvaluator::unparseState(state);
    monitorOutput("state-change " +
		  item_name + " " + item_platform + " " + item_state);
}

bool
Abuild::createItemInterface(std::string const& builder_string,
			    std::string const& item_name,
			    std::string const& item_platform,
			    BuildItem& build_item,
			    InterfaceParser& parser)
{
    verbose("creating interface for " + item_name + " on " + item_platform);

    // Initialize this item's interface.
    build_item.setInterface(item_platform, parser.getInterface());

    // Import the base interface.
    bool status = parser.importInterface(*(this->base_interface));

    // Import plugin interfaces
    std::list<std::string> const& plugins = build_item.getPlugins();
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	verbose("importing interface for plugin " + *iter);
	if (! parser.importInterface(
		this->buildset[*iter]->getInterface(PLUGIN_PLATFORM)))
	{
	    QTC::TC("abuild", "Abuild ERR import of plugin interface");
	    status = false;
	}
    }

    // Import the interfaces of our direct dependencies.  They already
    // include the interfaces of our indirect dependencies.
    std::list<std::string> const& deps =
	this->build_graph.getDirectDependencies(builder_string);
    boost::regex builder_re(BUILDER_RE);
    for (std::list<std::string>::const_iterator iter = deps.begin();
	 iter != deps.end(); ++iter)
    {
	boost::smatch match;
	assert(boost::regex_match(*iter, match, builder_re));
	std::string dep_name = match[1].str();
	std::string dep_platform = match[2].str();

	BuildItem& dep_item = *(this->buildset[dep_name]);
	verbose("importing interface for dependency " + dep_name);
	if (! parser.importInterface(dep_item.getInterface(dep_platform)))
	{
	    QTC::TC("abuild", "Abuild ERR import of dependent interface");
	    status = false;
	}
    }

    // Assign variables that are relevant to this build.  This must be
    // done regardless of whether we have our own interface file
    // because these values may be accessed by the item's local build
    // file well.

    Interface& _interface = *(parser.getInterface());
    FileLocation internal("<internal: " + builder_string + ">", 0, 0);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_THIS", build_item.getName(),
			    Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_TARGET_TYPE",
			    TargetType::getName(build_item.getTargetType()),
			    Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_PLATFORM_TYPE",
			    build_item.getPlatformType(item_platform),
			    Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_PLATFORM", item_platform,
			    Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_OUTPUT_DIR",
			    OUTPUT_DIR_PREFIX + item_platform,
			    Interface::a_override, status);

    // Assign to platform component variables unconditionally of
    // target type so they are empty strings when they don't apply.
    std::string platform_os;
    std::string platform_cpu;
    std::string platform_toolset;
    std::string platform_compiler;
    std::string platform_option;

    TargetType::target_type_e target_type = build_item.getTargetType();
    if (target_type == TargetType::tt_object_code)
    {
	boost::regex object_code_platform_re(
	    "^([^\\.]+)\\.([^\\.]+)\\.([^\\.]+)\\.([^\\.]+)(?:\\.([^\\.]+))?$");
	boost::smatch match;
	if (boost::regex_match(item_platform, match, object_code_platform_re))
	{
	    platform_os = match[1].str();
	    platform_cpu = match[2].str();
	    platform_toolset = match[3].str();
	    platform_compiler = match[4].str();
	    if (match[5].matched)
	    {
		platform_option = match[5].str();
	    }
	}
	else
	{
	    fatal("unable to parse object-code platform " + item_platform);
	}
    }

    assignInterfaceVariable(_interface,
			    internal, "ABUILD_PLATFORM",
			    item_platform, Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_PLATFORM_OS",
			    platform_os, Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_PLATFORM_CPU",
			    platform_cpu, Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_PLATFORM_TOOLSET",
			    platform_toolset, Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_PLATFORM_COMPILER",
			    platform_compiler, Interface::a_override, status);
    assignInterfaceVariable(_interface,
			    internal, "ABUILD_PLATFORM_OPTION",
			    platform_option, Interface::a_override, status);

    // Read our own interface file, if any.
    std::string interface_file =
	build_item.getAbsolutePath() + "/" + ItemConfig::FILE_INTERFACE;
    if (Util::isFile(interface_file))
    {
	// We end up reading the interface file once per platform,
	// which is somewhat inefficient.  We have to evaluate it once
	// per platform because some variables have different values
	// in different contexts.  In order to avoid reading the file,
	// we'd have to separate the reading of the file from the
	// evaluation of the file, and that's probably not worth the
	// trouble.  If there are actual syntax errors in the
	// interface file, then those syntax errors will potentially
	// be reported multiple times for a -k or multithreaded build.
	verbose("loading " + ItemConfig::FILE_INTERFACE);
	if (! parser.parse(interface_file, true))
	{
	    status = false;
	}
    }

    return status;
}

bool
Abuild::createPluginInterface(std::string const& plugin_name,
			      BuildItem& build_item)
{
    std::string dir = build_item.getAbsolutePath();

    // Initialize this item's interface.  We store this in the special
    // "platform" PLUGIN_PLATFORM which is initialized to a value that
    // can never match a real platform name.
    InterfaceParser parser(
	plugin_name, plugin_name + ":" + PLUGIN_PLATFORM, dir);
    build_item.setInterface(PLUGIN_PLATFORM, parser.getInterface());
    bool status = parser.importInterface(*(this->base_interface));

    std::string interface_file = dir + "/" + FILE_PLUGIN_INTERFACE;
    if (Util::isFile(interface_file))
    {
	// Load the interface file
	if (! parser.parse(interface_file, false))
	{
	    QTC::TC("abuild", "Abuild ERR error loading plugin interface");
	    status = false;
	}
    }

    return status;
}

void
Abuild::assignInterfaceVariable(Interface& _interface,
				FileLocation const& location,
				std::string const& variable_name,
				std::string const& value,
				Interface::assign_e assignment_type,
				bool& status)
{
    if (! _interface.assignVariable(
	    location, variable_name, value, assignment_type))
    {
	status = false;
    }
}

bool
Abuild::readAfterBuilds(std::string const& item_name,
			std::string const& item_platform,
			BuildItem& build_item,
			InterfaceParser& parser)
{
    bool status = true;

    std::string const& abs_path = build_item.getAbsolutePath();
    std::string interface_file = abs_path + "/" + ItemConfig::FILE_INTERFACE;

    std::vector<std::string> after_builds = parser.getAfterBuilds();
    for (std::vector<std::string>::iterator iter = after_builds.begin();
	 iter != after_builds.end(); ++iter)
    {
	std::string after_build = *iter;
	if (Util::fileExists(after_build))
	{
	    verbose("loading after build file " +
		    Util::absToRel(after_build, abs_path));
	    if (parser.parse(after_build, false))
	    {
		QTC::TC("abuild", "Abuild good after-build");
	    }
	    else
	    {
		// Errors would have been issued
		status = false;
	    }
	}
	else
	{
	    QTC::TC("abuild", "Abuild ERR missing after-build");
	    error(FileLocation(interface_file, 0, 0),
		  "after-build file " +
		  Util::absToRel(after_build, abs_path) +
		  " does not exist");
	    status = false;
	}
    }

    return status;
}

bool
Abuild::buildItem(std::string const& item_name,
		  std::string const& item_platform,
		  BuildItem& build_item)
{
    if (! build_item.isWritable())
    {
	// Assume that this item has previously been built
	// successfully.
	QTC::TC("abuild", "Abuild not building read-only build item",
		(build_item.getBackingDepth() == 0) ? 0 : 1);
	return true;
    }

    std::string output_dir = OUTPUT_DIR_PREFIX + item_platform;
    if (this->special_target == s_NO_OP)
    {
        QTC::TC("abuild", "Abuild no-op");
        if (! this->silent)
        {
            info(item_name + " (" + output_dir + "): " + this->special_target);
	}
	return true;
    }

    std::string path = build_item.getAbsolutePath();
    std::string abs_output_dir = path + "/" + output_dir;
    std::string rel_output_dir = Util::absToRel(abs_output_dir);

    std::list<std::string>& backend_targets =
	((explicit_target_items.count(item_name) != 0)
	 ? this->targets
	 : default_targets);

    monitorOutput("targets " +
		  item_name + " " + item_platform + " " +
		  Util::join(" ", backend_targets));
    if (! this->silent)
    {
        info(item_name + " (" + output_dir + "): " +
	     Util::join(" ", backend_targets) +
	     (this->verbose_mode ? " in " + rel_output_dir : ""));
    }

    // The .abuild file tells abuild that this is its directory.
    // cleanPath verifies its existence before deleting a directory.
    std::string dot_abuild = abs_output_dir + "/.abuild";
    if (! Util::isFile(dot_abuild))
    {
	Util::makeDirectory(abs_output_dir);
	std::ofstream of(dot_abuild.c_str(),
			 std::ios_base::out |
			 std::ios_base::trunc |
			 std::ios_base::binary);
	if (! of.is_open())
	{
	    throw QEXC::System("create " + dot_abuild, errno);
	}
	of.close();
    }

    bool result = false;

    switch (build_item.getBackend())
    {
      case ItemConfig::b_make:
	result = invoke_gmake(item_name, item_platform, build_item,
			      abs_output_dir, backend_targets);
	break;

      case ItemConfig::b_ant:
	result = invoke_ant(item_name, item_platform, build_item,
			    abs_output_dir, backend_targets);
	break;

      default:
	fatal("unknown backend type for build item " + item_name);
	break;
    }

    return result;
}

bool
Abuild::invoke_gmake(std::string const& item_name,
		     std::string const& item_platform,
		     BuildItem& build_item,
		     std::string const& dir,
		     std::list<std::string> const& targets)
{
    // Create FILE_DYNAMIC_MK
    std::string dynamic_file = dir + "/" + FILE_DYNAMIC_MK;
    // We need Unix newlines regardless of our platform, so open this
    // as a binary file.
    std::ofstream mk(dynamic_file.c_str(),
		     std::ios_base::out |
		     std::ios_base::trunc |
		     std::ios_base::binary);
    if (! mk.is_open())
    {
	throw QEXC::System("create " + dynamic_file, errno);
    }

    // Generate variables that are private to this build and should
    // not be accessible in and from the item's interface.

    // Generate make variables for this item and all its dependencies
    // and plugins.
    std::set<std::string> const& references = build_item.getReferences();
    for (std::set<std::string>::const_iterator iter = references.begin();
	 iter != references.end(); ++iter)
    {
	std::string local_path = Util::absToRel(
	    this->buildset[*iter]->getAbsolutePath(), dir);
        mk << "abDIR_" << *iter << " := " << local_path << "\n";
    }

    // Generate a list of directly accessible build items.
    mk << "ABUILD_ACCESSIBLE :=";
    for (BuildItem_map::iterator iter = this->buildset.begin();
	 iter != this->buildset.end(); ++iter)
    {
	std::string const& oitem = (*iter).first;
	if (accessibleFrom(this->buildset, item_name, oitem))
	{
	    mk << " " << oitem;
	}
    }
    mk << "\n";

    // Generate a list of plugin directories.  We don't provide the
    // names of plugins because people shouldn't be accessing them,
    // and this saves us from writing backend code to resolve them
    // anyway.
    std::list<std::string> const& plugins = build_item.getPlugins();
    mk << "ABUILD_PLUGINS :=";
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	std::string local_path = Util::absToRel(
	    this->buildset[*iter]->getAbsolutePath(), dir);
	mk << " \\\n    " << local_path;
    }
    mk << "\n";

    // Output variables based on the item's interface object.
    Interface const& _interface = build_item.getInterface(item_platform);
    std::map<std::string, Interface::VariableInfo> variables =
	_interface.getVariablesForTargetType(
	    build_item.getTargetType(), build_item.getFlagData());
    for (std::map<std::string, Interface::VariableInfo>::iterator iter =
	     variables.begin();
	 iter != variables.end(); ++iter)
    {
	std::string const& name = (*iter).first;
	Interface::VariableInfo const& info = (*iter).second;
	mk << name << " :=";
	for (std::deque<std::string>::const_iterator viter = info.value.begin();
	     viter != info.value.end(); ++viter)
	{
	    std::string const& val = *viter;
	    if (info.type == Interface::t_filename)
	    {
		mk << " " << Util::absToRel(val, dir);
	    }
	    else
	    {
		mk << " " << val;
	    }
	}
	// Use Unix newlines regardless of our platform.
	mk << "\n";
    }

    mk.close();

    // -r = no builtin rules
    // -R = no builtin variables
    std::vector<std::string> make_argv;
    make_argv.push_back("make");
    if (dir != this->current_directory)
    {
	make_argv.push_back("-C");
	make_argv.push_back(dir);
    }
    make_argv.push_back("-rR");
    make_argv.push_back("ABUILD_TOP=" + this->abuild_top);
    make_argv.push_back("--warn-undefined-variables");
    make_argv.insert(make_argv.end(),
		     this->make_args.begin(), this->make_args.end());
    make_argv.push_back("-f");
    make_argv.insert(make_argv.end(),
		     this->abuild_top + "/make/abuild.mk");
    make_argv.insert(make_argv.end(),
		     targets.begin(), targets.end());

    return invokeBackend(this->gmake, make_argv,
			 std::map<std::string, std::string>(),
			 this->envp, dir);
}

bool
Abuild::invoke_ant(std::string const& item_name,
		   std::string const& item_platform,
		   BuildItem& build_item,
		   std::string const& dir,
		   std::list<std::string> const& targets)
{
    // Create FILE_DYNAMIC_ANT
    std::string dynamic_file = dir + "/" + FILE_DYNAMIC_ANT;
    std::ofstream dyn(dynamic_file.c_str(),
		      std::ios_base::out |
		      std::ios_base::trunc);
    if (! dyn.is_open())
    {
        throw QEXC::System("create " + dynamic_file, errno);
    }

    // Generate variables that are private to this build and should
    // not be accessible in and from the item's interface.

    // Generate properties for this item and all its dependencies and
    // plugins.
    std::set<std::string> const& references = build_item.getReferences();
    for (std::set<std::string>::const_iterator iter = references.begin();
         iter != references.end(); ++iter)
    {
        dyn << "abuild.dir." << *iter << "="
	    << this->buildset[*iter]->getAbsolutePath()
	    << std::endl;
    }

    // Generate a list of directly accessible build items.
    dyn << "abuild.accessible=";
    for (BuildItem_map::iterator iter = this->buildset.begin();
         iter != this->buildset.end(); ++iter)
    {
        std::string const& oitem = (*iter).first;
        if (accessibleFrom(this->buildset, item_name, oitem))
        {
            dyn << " " << oitem;
        }
    }
    dyn << std::endl;

    // Generate a property for the top of abuild
    dyn << "abuild.top=" << this->abuild_top << std::endl;

    // Generate a list of plugin directories.  We don't provide the
    // names of plugins because people shouldn't be accessing them,
    // and this saves us from writing backend code to resolve them
    // anyway.
    std::list<std::string> const& plugins = build_item.getPlugins();
    std::list<std::string> plugin_paths;
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	plugin_paths.push_back(
	    Util::absToRel(this->buildset[*iter]->getAbsolutePath(), dir));
    }
    dyn << "abuild.plugins="
	<< Util::join(",", plugin_paths)
	<< "\n";

    std::string path_element_separator;
#ifdef _WIN32
    path_element_separator = ";";
#else
    path_element_separator = ":";
#endif

    // Output variables based on the item's interface object.
    Interface const& _interface = build_item.getInterface(item_platform);
    std::map<std::string, Interface::VariableInfo> variables =
        _interface.getVariablesForTargetType(
	    build_item.getTargetType(), build_item.getFlagData());
    for (std::map<std::string, Interface::VariableInfo>::iterator iter =
             variables.begin();
         iter != variables.end(); ++iter)
    {
        std::string const& name = (*iter).first;
        Interface::VariableInfo const& info = (*iter).second;
        dyn << name << "=";
	if (info.type == Interface::t_filename)
	{
	    dyn << Util::join(path_element_separator, info.value);
	}
	else
	{
	    dyn << Util::join(" ", info.value);
	}
	dyn << std::endl;
    }

    dyn.close();

    std::vector<std::string> ant_argv;
    ant_argv.push_back(this->ant);
    ant_argv.push_back("-f");
    if (build_item.hasAntBuild())
    {
	ant_argv.push_back(build_item.getAbsolutePath() + "/" +
			   build_item.getBuildFile());
    }
    else
    {
	ant_argv.push_back(this->abuild_top + "/ant/abuild.xml");
    }
    ant_argv.push_back("-Dbasedir=" + dir);
    ant_argv.insert(ant_argv.end(),
		    this->ant_args.begin(), this->ant_args.end());
    if (! this->ant_library.empty())
    {
	ant_argv.push_back("-lib");
	ant_argv.push_back(Util::dirname(this->ant_library));
	if (this->use_abuild_logger)
	{
	    ant_argv.push_back("-logger");
	    ant_argv.push_back("org.abuild.ant.AbuildLogger");
	}
    }

    ant_argv.insert(ant_argv.end(),
		    targets.begin(), targets.end());

    return invokeBackend(this->ant, ant_argv,
			 std::map<std::string, std::string>(),
			 this->envp, dir);
}

bool
Abuild::invokeBackend(std::string const& progname,
		      std::vector<std::string> const& args,
		      std::map<std::string, std::string> const& environment,
		      char* old_env[],
		      std::string const& dir)
{
    if (this->max_workers == 1)
    {
        // If we have only one thread, then only one thread is using
        // the logger.  Flush the logger before we run the backend to
        // prevent its output from being interleaved with our own.
        this->logger.flushLog();
    }
    else
    {
        // Consider doing something to capture the backend's output so
        // that it is not arbitrarily interleaved with other things.
        // We can't call flushLog here because other threads may be
        // logging.  Ideally, we should capture output, prefix it with
        // the build item name, and output it line by line or else
        // de-interleave output from different build items.
    }

    if (this->verbose_mode)
    {
	// Put this in the if statement to avoid the needless cal to
	// join if not in verbose mode.
	verbose("running " + Util::join(" ", args));
    }

    return Util::runProgram(progname, args, environment, old_env, dir);
}

void
Abuild::cleanBuildset()
{
    for (BuildItem_map::iterator iter = this->buildset.begin();
	 iter != this->buildset.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	if (item.isWritable() && item.hasBuildFile())
	{
	    cleanPath(item_name, item.getAbsolutePath());
	}
    }
}

void
Abuild::cleanPath(std::string const& item_name, std::string const& dir)
{
    if (! this->silent)
    {
	std::string path = Util::absToRel(dir);
        if (item_name.empty())
        {
            info("cleaning in " + path);
        }
        else
        {
            info("cleaning " + item_name + " in " + path);
        }
    }

    std::list<std::string> entries = Util::getDirEntries(dir);
    for (std::list<std::string>::iterator iter = entries.begin();
	 iter != entries.end(); ++iter)
    {
	std::string const& entry = *iter;
	std::string fullpath = dir + "/" + entry;
	if ((entry.substr(0, OUTPUT_DIR_PREFIX.length()) == OUTPUT_DIR_PREFIX) &&
	    (Util::isFile(fullpath + "/.abuild")))
	{
	    try
	    {
		Util::removeFileRecursively(fullpath);
	    }
	    catch(QEXC::General& e)
	    {
		error(e.what());
	    }
	}
    }
}

void
Abuild::help()
{
    boost::function<void(std::string const&)> h =
	boost::bind(&Logger::logInfo, &(this->logger), _1);

    // This guide helps keep message to 80 columns.

    //0        1         2         3         4         5         6         7         8
    //12345678901234567890123456789012345678901234567890123456789012345678901234567890

    h("");
    h("Usage: " + whoami + " [options] [targets]");
    h("");
    h("This help message provides a brief synopsis of supported arguments.");
    h("Please see abuild's documentation for additional details.");
    h("");
    h("Options and targets may appear in any order.  Any argument that starts");
    h("with \"-\" is treated as an option.");
    h("");
    h("If no targets are specified, the \"all\" target is built.");
    h("");
    h("The following options are supported:");
    h("");
    h("  -H | --help       print help message and exit");
    h("  -V | --version    print abuild's version number and exit");
    h("");
    h("  --ant             pass all remaining arguments (up to --make) to ant");
    h("  --apply-targets-to-deps   apply explicit targets to dependencies as well;");
    h("                    when cleaning with a clean set, expand to include");
    h("                    dependencies");
    h("  --build=set |     specify a build set");
    h("    -b set");
    h("  -C start-dir      change directories to start-dir before running");
    h("  --clean=set |     specify a clean set");
    h("    -c set");
    h("  --dump-build-graph  dump abuild's internal build graph");
    h("  --dump-data       dump abuild's data to stdout and build no targets");
    h("  -e | --emacs      pass the -e flags to ant and also set a property");
    h("                    telling our ant build file that we are running in");
    h("                    emacs mode.");
    h("  --find-conf       look above the current directory to find a directory");
    h("                    that contains Abuild.conf and run abuild from there");
    h("  --full-integrity  check integrity for all items, not just items being built");
    h("  --jobs=n | -jn    build up to n build items in parallel");
    h("  --keep-going |    don't stop when a build item fails; also tells backend");
    h("    -k              not to stop on failure");
    h("  --list-platforms  list all object-code platforms");
    h("  --list-traits     list all known traits");
    h("  --make-jobs[=n]   passes the -j flag to make allowing each make to use");
    h("                    up to n jobs; omit n to let it use as many as it can");
    h("  --make            pass all remaining arguments (up to --ant) to make");
    h("  --monitored       run in monitored mode");
    h("  -n |              make only: print what make would do without doing it");
    h("    --just-print |");
    h("    --dry-run |");
    h("    --recon");
    h("  --no-abuild-logger  ant only: suppress use of AbuildLogger");
    h("  --no-dep-failures   when used with -k, attempt to build items even when");
    h("                    one or more of their dependencies have failed");
    h("  --only-with-traits trait[,trait,...]   remove all items from build set");
    h("                    that do not have all of the named traits");
    h("  --platform-selector selector |       specify a platform selector");
    h("    -p selector     for object-code platforms; see below");
    h("  --print-abuild-top  print the path to the top of the abuild installation");
    h("  --related-by-traits trait[,trait,...]  add to the build set all items that");
    h("                    relate to any item already in the build set by all of");
    h("                    the named traits");
    h("  --silent          suppress most non-error output");
    h("  --verbose         generate more detailed output");
    h("  --with-deps | -d  short-hand for --build=current");
    h("");
    h("Build/Clean sets:");
    h("");
    h("  all               all buildable/cleanable items in writable build trees");
    h("  local             all items in the local build tree without its externals");
    h("  desc              all items at or below the current directory");
    h("  deps              all expanded dependencies of the current item");
    h("  current           the current item");
    h("  name:name,...     items with the given names");
    h("  pattern:regex     items whose names match the given regular expression");
    h("");
    h("  When building (as opposed to cleaning), all build sets automatically");
    h("  include dependencies that are satisfied in writable build trees.");
    h("");
    h("Platform selectors:");
    h("");
    h("Platform selectors may be specified with --platform-selector and in the");
    h("ABUILD_PLATFORM_SELECTORS environment variable.  A platform selector is of");
    h("this form:");
    h("");
    h("  [platform-type:]match-criterion");
    h("");
    h("A match-criterion may be on the following:");
    h("");
    h("  option=<option>");
    h("  compiler=<compiler>[.<option>]");
    h("  platform=<os>.<cpu>.<toolset>.<compiler>[.<option>]");
    h("  all");
    h("  skip");
    h("");
    h("Any criterion component may be '*'.");
    h("");
    h("The special targets \"clean\" and \"no-op\" are not passed to make or");
    h("ant (unless run from an output subdirectory) and may not be combined");
    h("with any other targets.");
    h("");
    h("The target \"rules-help\" will print help information about what");
    h("Abuild.mk variables can be set for any rules defined in the RULES");
    h("variable in Abuild.mk.  The target \"interface-help\" will print help");
    h("information about what Interface.mk variables can be set for any rules");
    h("defined in Abuild.mk.");
    h("");
}

void
Abuild::usage(std::string const& msg)
{
    if (! msg.empty())
    {
	error(msg);
    }
    fatal("run " + this->whoami + " --help for help");
}

void
Abuild::exitIfErrors()
{
    if (Error::anyErrors())
    {
	fatal("errors detected; exiting");
    }
}


void
Abuild::info(std::string const& msg)
{
    this->logger.logInfo(this->whoami + ": " + msg);
}

void
Abuild::verbose(std::string const& msg)
{
    if (this->verbose_mode)
    {
	this->logger.logInfo(this->whoami + ": (verbose) " + msg);
    }
}

void
Abuild::monitorOutput(std::string const& msg)
{
    if (this->monitored)
    {
	this->logger.logInfo("abuild-monitor: " + msg);
    }
}

void
Abuild::monitorErrorCallback(std::string const& msg)
{
    monitorOutput("error " + msg);
}

void
Abuild::error(std::string const& msg)
{
    error(FileLocation(), msg);
}

void
Abuild::error(FileLocation const& location, std::string const& msg)
{
    this->error_handler.error(location, msg);
}

void
Abuild::fatal(std::string const& msg)
{
    // General exception caught in main().
    throw QEXC::General(this->whoami + ": ERROR: " + msg);
}
