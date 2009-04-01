#include <Abuild.hh>

#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <Logger.hh>
#include <FileLocation.hh>
#include <ItemConfig.hh>
#include <BackingConfig.hh>
#include <InterfaceParser.hh>
#include <DependencyRunner.hh>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <assert.h>

std::string const Abuild::ABUILD_VERSION = "1.1.a4";
std::string const Abuild::OUTPUT_DIR_PREFIX = "abuild-";
std::string const Abuild::FILE_DYNAMIC_MK = ".ab-dynamic.mk";
std::string const Abuild::FILE_DYNAMIC_ANT = ".ab-dynamic-ant.properties";
std::string const Abuild::FILE_DYNAMIC_GROOVY = ".ab-dynamic.groovy";
std::string const Abuild::FILE_INTERFACE_DUMP = ".ab-interface-dump";
std::string const Abuild::b_ALL = "all";
std::string const Abuild::b_DEPTREES = "deptrees";
std::string const Abuild::b_LOCAL = "local";
std::string const Abuild::b_DESC = "desc";
std::string const Abuild::b_DEPS = "deps";
std::string const Abuild::b_CURRENT = "current";
std::set<std::string> Abuild::valid_buildsets;
std::map<std::string, std::string> Abuild::buildset_aliases;
std::string const Abuild::s_CLEAN = "clean";
std::string const Abuild::s_NO_OP = "no-op";
// PLUGIN_PLATFORM can't match a real platform name.
std::string const Abuild::PLUGIN_PLATFORM = "plugin";
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
    dump_interfaces(false),
    apply_targets_to_deps(false),
    compat_level(CompatLevel::cl_1_0),
    local_build(false),
    error_handler(whoami),
    this_config(0),
#ifdef _WIN32
    have_perl(false),
#endif
    last_assigned_tree_number(0),
    suggest_upgrade(false),
    logger(*(Logger::getInstance()))
{
    Error::setErrorCallback(
	boost::bind(&Abuild::monitorErrorCallback, this, _1));

    boost::posix_time::time_duration epoch =
	boost::posix_time::second_clock::universal_time() -
	boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));
    std::srand(epoch.total_seconds());
    this->last_assigned_tree_number = 0;
}

Abuild::~Abuild()
{
    Error::clearErrorCallback();
}

bool
Abuild::initializeStatics()
{
    valid_buildsets.insert(b_ALL);
    valid_buildsets.insert(b_DEPTREES);
    valid_buildsets.insert(b_LOCAL);
    valid_buildsets.insert(b_DESC);
    valid_buildsets.insert(b_DEPS);
    valid_buildsets.insert(b_CURRENT);

    buildset_aliases["down"] = b_DESC;
    buildset_aliases["descending"] = b_DESC;

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
	    l("Copyright (c) 2007-2009 Jay Berkenbilt, Argon ST");
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
	else if (strcmp(argv[1], "--upgrade-trees") == 0)
	{
	    return upgradeTrees();
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

    if (this->compat_level.allow_1_0())
    {
	suggestUpgrade();
    }

    boost::shared_ptr<boost::posix_time::time_duration> build_time;
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
	boost::posix_time::ptime now(
	    boost::posix_time::second_clock::local_time());
        okay = buildBuildset();
	if (this->special_target.empty())
	{
	    build_time.reset(
		new boost::posix_time::time_duration(
		    boost::posix_time::second_clock::local_time() - now));
	}
    }

    if (this->java_builder)
    {
	this->java_builder->finish();
    }

    if (! okay)
    {
	error("at least one build failure occurred; summary follows");
	boost::regex builder_re(BUILDER_RE);
	boost::smatch match;
	assert(! this->failed_builds.empty());
	for (std::vector<std::string>::iterator iter =
		 this->failed_builds.begin();
	     iter != this->failed_builds.end(); ++iter)
	{
	    assert(boost::regex_match(*iter, match, builder_re));
	    std::string item_name = match[1].str();
	    std::string item_platform = match[2].str();
	    error("build failure: " + item_name + " on platform " +
		  item_platform);
	}
    }

    if (build_time.get())
    {
	std::ostringstream time;
	time << "total build time: " << *build_time;
	info(time.str());
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
    boost::regex define_re("([^-][^=]*)=(.*)");
    boost::regex compat_re("--compat-level=(\\d+\\.\\d+)");

    boost::smatch match;

    std::list<std::string> platform_selector_strings;
    std::list<std::string> clean_platform_strings;
    std::string platform_selector_env;
    if (Util::getEnv("ABUILD_PLATFORM_SELECTORS", &platform_selector_env))
    {
	QTC::TC("abuild", "Abuild ABUILD_PLATFORM_SELECTORS");
	platform_selector_strings = Util::splitBySpace(platform_selector_env);
    }
    std::string compat_level_version;
    if (! Util::getEnv("ABUILD_COMPAT_LEVEL", &compat_level_version))
    {
	compat_level_version = "1.0";
    }

    bool with_deps = false;
    char** argp = &argv[1];
    while (*argp)
    {
	std::string arg = *argp++;
	if (boost::regex_match(arg, match, define_re))
	{
	    this->defines[match.str(1)] = match.str(2);
	}
	else if (boost::regex_match(arg, match, jobs_re))
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
	    this->java_builder_args.push_back("-k");
	    this->keep_going = true;
	}
	else if (arg == "--no-dep-failures")
	{
	    this->no_dep_failures = true;
	}
	else if (arg == "-n")
	{
	    this->java_builder_args.push_back("-n");
	    this->make_args.push_back("-n");
	}
	else if ((arg == "-e") || (arg == "--emacs"))
	{
	    this->java_builder_args.push_back("-e");
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
	else if (arg == "--clean-platforms")
	{
	    if (! *argp)
	    {
		usage("--clean-platforms requires an argument");
	    }
	    clean_platform_strings.push_back(*argp++);
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
	else if (arg == "--deprecation-is-error")
	{
	    this->make_args.push_back("ABUILD_DEPRECATE_IS_ERROR=1");
	    this->java_builder_args.push_back("-de");
	    Error::setDeprecationIsError(true);
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
	    this->java_builder_args.push_back("-v");
	    this->verbose_mode = true;
	}
	else if (arg == "--silent")
	{
	    this->make_args.push_back("ABUILD_SILENT=1");
	    this->java_builder_args.push_back("-q");
	    this->silent = true;
	}
	else if (arg == "--monitored")
	{
	    this->monitored = true;
	}
	else if (arg == "--dump-interfaces")
	{
	    this->dump_interfaces = true;
	}
	else if (boost::regex_match(arg, match, compat_re))
	{
	    compat_level_version = match.str(1);
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

    // Compatability level
    if (compat_level_version == "1.0")
    {
	this->compat_level.setLevel(CompatLevel::cl_1_0);
    }
    else if (compat_level_version == "1.1")
    {
	this->compat_level.setLevel(CompatLevel::cl_1_1);
	this->java_builder_args.push_back("-cl1_1");
	this->make_args.push_back("ABUILD_SUPPORT_1_0=0");
    }
    else
    {
	usage("invalid compatibility level " + compat_level_version);
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
	PlatformSelector p;
	if (p.initialize(*iter))
	{
	    // Don't validate the platform type or any of the fields
	    // themselves.  It's not incorrect to reference
	    // non-existent platforms or platform types here.  That
	    // way, people can set the ABUILD_PLATFORM_SELECTORS
	    // environment variable based on some build tree they work
	    // in and not worry about it if they build in a different
	    // tree that doesn't have all the same platforms and
	    // platform types.
	    this->platform_selectors[p.getPlatformType()] = p;
	}
	else
	{
	    QTC::TC("abuild", "Abuild ERR invalid platform selector");
	    error("invalid platform selector " + *iter);
	}
    }

    // store clean platforms
    for (std::list<std::string>::iterator iter =
	     clean_platform_strings.begin();
	 iter != clean_platform_strings.end(); ++iter)
    {
	try
	{
	    std::string regex = Util::globToRegex(*iter);
	    this->clean_platforms.insert(regex);
	}
	catch (QEXC::General& e)
	{
	    QTC::TC("abuild", "Abuild ERR bad clean platform");
	    error("invalid clean platform selector " + *iter + ": " + e.what());
	}
    }

    if (this->targets.empty())
    {
	this->targets = default_targets;
    }
}

void
Abuild::checkBuildsetName(std::string const& kind, std::string& name)
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
    else if (buildset_aliases.count(name) != 0)
    {
	QTC::TC("abuild", "Abuild replace build set alias");
	name = buildset_aliases[name];
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
    this->this_config = readConfig(this->this_config_dir, "");

    std::string local_top = findTop(
	this->this_config_dir, "local forest");
    if (local_top.empty())
    {
	QTC::TC("abuild", "Abuild ERR can't find local top");
	fatal("unable to find root of local forest");
    }

    // We explicitly make forests local to readConfigs.  By not
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
    BuildForest_map forests;
    traverse(forests, local_top);

    // Compute the list of all known traits.  This routine also
    // validates to make sure that any traits specified on the command
    // line are valid.
    computeValidTraits(forests);

    if (this->monitored || this->dump_data)
    {
	monitorOutput("begin-dump-data");
	dumpData(forests);
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
	listPlatforms(forests);
	return true;
    }

    BuildForest& local_forest = *(forests[local_top]);
    BuildTree_map& buildtrees = local_forest.getBuildTrees();
    BuildItem_map& builditems = local_forest.getBuildItems();
    computeBuildset(buildtrees, builditems);

    if (! this->full_integrity)
    {
	// Note: we do these checks even when cleaning.  Otherwise,
	// shadowed items won't get cleaned in their backing areas on
	// --clean=all, which is not the expected behavior.
	reportIntegrityErrors(forests, this->buildset, local_top);
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
	local_forest.getSortedItemNames();
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
Abuild::readConfig(std::string const& dir, std::string const& parent_dir)
{
    ItemConfig* result = 0;
    try
    {
	result = ItemConfig::readConfig(
	    this->error_handler, this->compat_level, dir, parent_dir);
    }
    catch (QEXC::General& e)
    {
	fatal(e.what());
    }
    return result;
}

ItemConfig*
Abuild::readExternalConfig(std::string const& start_dir,
			   std::string const& start_external)
{
    if (read_external_config_cache[start_dir].count(start_external))
    {
	return read_external_config_cache[start_dir][start_external];
    }

    verbose("looking for external \"" + start_external + "\" of \"" +
	    start_dir + "\"");

    ItemConfig* config = 0;

    std::set<std::string> seen;
    std::list<std::pair<std::string, std::string> > candidates;
    candidates.push_back(std::make_pair(start_dir, start_external));

    while ((! candidates.empty()) && (config == 0))
    {
	std::string dir = candidates.front().first;
	std::string ext = candidates.front().second;
	candidates.pop_front();
	std::string fulldir = dir;
	if (! ext.empty())
	{
	    fulldir += "/" + ext;
	}
	if (seen.count(fulldir))
	{
	    QTC::TC("abuild", "Abuild ERR readExternalConfig loop");
	    error("loop detected trying to find " + ItemConfig::FILE_CONF +
		  " for \"" + start_external + "\" relative to \"" +
		  start_dir + "\"");
	    break;
	}
	else
	{
	    seen.insert(fulldir);
	}

	verbose("inside tree \"" + dir + "\"");
	verbose("checking directory \"" + fulldir + "\"");
	if ((! ext.empty()) && Util::isDirectory(fulldir))
	{
	    // The external directory exists, so work relative to that
	    // directory rather than our original one.
	    verbose("directory exists; switching context from"
		    " original tree to this tree");
	    dir = fulldir;
	    ext.clear();
	}

	if (Util::isFile(fulldir + "/" + ItemConfig::FILE_CONF))
	{
	    // Simple case: there's an actual Abuild.conf here.
	    config = readConfig(fulldir, "");
	    verbose("found at \"" + config->getAbsolutePath() + "\"");
	}
	else if (Util::isFile(dir + "/" + BackingConfig::FILE_BACKING))
	{
	    // There's an Abuild.backing but no Abuild.conf.  Traverse
	    // the backing chain.  If the external exists, we're
	    // traversing its backing chain.  Otherwise, we're
	    // traversing the original directory's backing chain.
	    QTC::TC("abuild", "Abuild backing without conf");
	    verbose("checking backing areas of \"" + dir + "\"");
	    std::list<std::string> const& backing_areas =
		readBacking(dir)->getBackingAreas();
	    if (! backing_areas.empty())
	    {
		QTC::TC("abuild", "Abuild external in backing area",
			(ext.empty() ? 0 : 1));
		for (std::list<std::string>::const_iterator iter =
			 backing_areas.begin();
		     iter != backing_areas.end(); ++iter)
		{
		    candidates.push_back(std::make_pair(*iter, ext));
		}
	    }
	}
    }

    verbose("done looking for external \"" +
	    start_external + "\" of \"" + start_dir + "\"");

    read_external_config_cache[start_dir][start_external] = config;
    return config;		// might be 0
}

std::string
Abuild::findTop(std::string const& start_dir,
		std::string const& description)
{
    // Find the directory that contains the root of the forest
    // containing the item with the given start directory.

    if (find_top_cache.count(start_dir))
    {
	return find_top_cache[start_dir];
    }

    std::string top;
    std::string dir = start_dir;

    verbose("looking for top of build forest \"" + description + "\"");

    while (true)
    {
	verbose("top-search: checking " + dir);
	ItemConfig* config = readConfig(dir, "");
	if (config->isForestRoot())
	{
	    top = dir;
	    break;
	}
	std::string parent_dir = config->getParentDir();
	if (parent_dir.empty())
	{
	    verbose("top-search: " + dir + " has no parent; ending search");
	    break;
	}
	else
	{
	    dir = parent_dir;
	}
    }

    if (top.empty())
    {
	QTC::TC("abuild", "Abuild ERR can't find top");
	error("unable to find top of build forest containing \"" +
	      start_dir + "\" (" + description + ");"
	      " run with --verbose for details");
    }
    else
    {
	verbose("top-search: " + top + " is forest root");
    }

    find_top_cache[start_dir] = top;
    return top;
}

void
Abuild::traverse(BuildForest_map& forests, std::string const& top_path)
{
    std::set<std::string> visiting;
    DependencyGraph external_graph; // 1.0-compatibility only

    traverseForests(forests, external_graph,
		    top_path, visiting, "root of local forest");
    if (this->compat_level.allow_1_0())
    {
	// If we have A -ext-> B <-ext- C, neither a traversal
	// starting from A nor one starting from C will catch all
	// three build trees.  If, as a result of backing areas, we
	// end up traversing from both A and C, will will end up with
	// too many forests, and we will need to merge some of them.
	// We use items_traversed to prevent the same tree from ever
	// being included in more than one forest.  This is all a
	// result of the forests we are creating in 1.1 actually
	// containing multiple 1.0 trees, each of which looks like the
	// root of a forest.

	if (! external_graph.check())
	{
	    QTC::TC("abuild", "Abuild ERR external_graph failure");
	    error("1.0 compatibility mode was unable to determine"
		  " proper relationships among externals; some errors"
		  " about unknown tree dependencies may be spurious and"
		  " may disappear after you fix tree dependency cycles");
	    reportDirectoryGraphErrors(external_graph, "external-dir");
	}
	else
	{
	    mergeForests(forests, external_graph);
	}

	removeEmptyTrees(forests);
    }

    DependencyGraph backing_graph;
    computeBackingGraph(forests, backing_graph);
    std::list<std::string> const& all_forests = backing_graph.getSortedGraph();
    for (std::list<std::string>::const_iterator iter = all_forests.begin();
	 iter != all_forests.end(); ++iter)
    {
	validateForest(forests, *iter);
    }
}

void
Abuild::reportDirectoryGraphErrors(DependencyGraph& g,
				   std::string const& description)
{
    DependencyGraph::ItemMap unknowns;
    std::vector<DependencyGraph::ItemList> cycles;
    g.getErrors(unknowns, cycles);

    for (DependencyGraph::ItemMap::iterator i1 = unknowns.begin();
	 i1 != unknowns.end(); ++i1)
    {
	std::string const& dir = (*i1).first;
	std::list<std::string> const& deps = (*i1).second;
	for (std::list<std::string>::const_iterator i2 = deps.begin();
	     i2 != deps.end(); ++i2)
	{
	    error("directory \"" + dir + "\" references unknown " +
		  description + " \"" + *i2 + "\"");
	}
    }

    for (std::vector<DependencyGraph::ItemList>::iterator i1 =
	     cycles.begin();
	 i1 != cycles.end(); ++i1)
    {
	DependencyGraph::ItemList const& cycle = *i1;
	error("the following trees are involved in"
	      " a cycle (" + description + "):");
	for (DependencyGraph::ItemList::const_iterator i2 = cycle.begin();
	     i2 != cycle.end(); ++i2)
	{
	    error("  " + *i2);
	}
    }
}


void
Abuild::computeBackingGraph(BuildForest_map& forests,
			    DependencyGraph& g)
{
    for (BuildForest_map::iterator forest_iter = forests.begin();
	 forest_iter != forests.end(); ++forest_iter)
    {
	std::string const& path = (*forest_iter).first;
	g.addItem(path);
	BuildForest& forest = *((*forest_iter).second);
	std::list<std::string> const& backing_areas = forest.getBackingAreas();
	for (std::list<std::string>::const_iterator biter =
		 backing_areas.begin();
	     biter != backing_areas.end(); ++biter)
	{
	    g.addDependency(path, *biter);
	}
    }
    // Even if there were errors, the build tree graph must always be
    // consistent.  Abuild doesn't store invalid backing areas in the
    // build tree structures.
    if (! g.check())
    {
	reportDirectoryGraphErrors(g, "backing area");
	fatal("unable to continue after backing area error");
    }
}

void
Abuild::traverseForests(BuildForest_map& forests,
			DependencyGraph& external_graph,
			std::string const& top_path,
			std::set<std::string>& visiting,
			std::string const& description)
{
    if (visiting.count(top_path))
    {
        QTC::TC("abuild", "Abuild ERR backing cycle");
        fatal("backing area cycle detected for " + top_path +
	      " (" + description + ")");
    }

    if (forests.count(top_path))
    {
	return;
    }

    verbose("traversing forest from " + top_path + ": \"" + description + "\"");
    ItemConfig* config = readConfig(top_path, "");
    assert(config->isForestRoot());

    // DO NOT RETURN BELOW THIS POINT until after we remove top_path
    // from visiting.
    visiting.insert(top_path);

    BuildForest_ptr forest(new BuildForest(top_path));
    std::list<std::string> dirs_with_externals;
    std::list<std::string>& backing_areas = forest->getBackingAreas();
    std::set<std::string>& deleted_trees = forest->getDeletedTrees();
    std::set<std::string>& deleted_items = forest->getDeletedItems();
    verbose("traversing items for " + top_path);
    traverseItems(*forest, external_graph, top_path, dirs_with_externals,
		  backing_areas, deleted_trees, deleted_items);
    verbose("done traversing items for " + top_path);

    if (this->compat_level.allow_1_0())
    {
	verbose("1.0-compatibility: checking for externals in " + top_path);
	while (! dirs_with_externals.empty())
	{
	    std::string dir = dirs_with_externals.front();
	    verbose("looking at externals of " + dir);
	    dirs_with_externals.pop_front();
	    ItemConfig* dconfig = readConfig(dir, "");
	    std::list<std::string> const& externals = dconfig->getExternals();
	    for (std::list<std::string>::const_iterator eiter =
		     externals.begin();
		 eiter != externals.end(); ++eiter)
	    {
		std::string const& edecl = *eiter;
		verbose("checking external " + edecl);

		std::string const& epath =
		    Util::canonicalizePath(dir + "/" + edecl);
		std::string file_conf =
		    epath + "/" + ItemConfig::FILE_CONF;
		std::string file_backing =
		    epath + "/" + BackingConfig::FILE_BACKING;

		if (! (Util::isFile(file_conf) ||
		       Util::isFile(file_backing)))
		{
		    // No additional traversal required.  We've
		    // already tried to resolve this in a backing area
		    // and reported if we were unable to do so.
		    verbose("skipping non-local external " + edecl);
		    continue;
		}

		// If there's an Abuild.backing file, we only care
		// about it if this is a forest root or if there's no
		// Abuild.conf.  In other instances, ItemConfig has
		// already complained.
		if (Util::isFile(file_backing) &&
		    (! Util::isFile(file_conf)))
		{
		    QTC::TC("abuild", "Abuild traverse backing without conf");
		    verbose("getting backing data from and then"
			    " skipping backed external " + edecl);
		    appendBackingData(
			epath, backing_areas, deleted_trees, deleted_items);
		    continue;
		}

		assert(Util::isFile(file_conf));

		std::string ext_top =
		    findTop(epath, "external \"" + edecl + "\" of \"" +
			    dir + "\"");
		if (ext_top.empty())
		{
		    QTC::TC("abuild", "Abuild ERR findTop from external");
		    // An error was already issued by findTop
		    continue;
		}

		// Traverse items in the external forest as if it were
		// part of the forest for which this traverseForests
		// method was called.  This effectively merges the
		// external forest with the referencing forest.  We
		// can't always catch all cases here.  See comments in
		// mergeForests for additional notes.
		verbose("traversing items for external " + ext_top +
			", which is " + edecl + " from " + dir);
		if (epath == ext_top)
		{
		    external_graph.addDependency(top_path, ext_top);
		}
		traverseItems(*forest, external_graph, ext_top,
			      dirs_with_externals, backing_areas,
			      deleted_trees, deleted_items);
		verbose("done traversing items for external " + ext_top);
	    }
	}
	verbose("done with externals");
    }
    else
    {
	assert(dirs_with_externals.empty());
    }

    verbose("checking for backing areas");
    // Normalize backing areas to filter out duplicates and resolve to
    // forest roots.  Traverse backing areas.
    { // private scope
	std::set<std::string> seen;
	std::list<std::string>::iterator iter = backing_areas.begin();
	while (iter != backing_areas.end())
	{
	    std::list<std::string>::iterator next = iter;
	    ++next;
	    verbose("checking for backing area " + *iter);
	    std::string btop = findTop(
		*iter, "backing area of \"" + top_path + "\"");
	    // We've already verified that everything appending to
	    // backing_areas has a valid top.
	    assert(! btop.empty());
	    if (*iter != btop)
	    {
		verbose("this is actually " + btop);
	    }
	    bool keep = false;
	    if (btop == top_path)
	    {
		// This is the most likely location for backing files,
		// though they could have come from 1.0-style
		// Abulid.backing files as well...
		FileLocation l(
		    top_path + "/" + BackingConfig::FILE_BACKING, 0, 0);
		if (*iter == btop)
		{
		    QTC::TC("abuild", "Abuild ERR backs explicitly to self");
		    error(l, "this forest lists itself as a backing area");
		}
		else
		{
		    QTC::TC("abuild", "Abuild ERR backs implicitly to self");
		    error(l, "backing area " + *iter +
			  " belongs to this forest");
		}
	    }
	    else if (seen.count(btop))
	    {
		verbose("this is a duplicate backing area; ignoring");
		QTC::TC("abuild", "Abuild duplicate backing area");
	    }
	    else
	    {
		seen.insert(btop);
		keep = true;
		verbose("traversing backing area");
		traverseForests(forests, external_graph, btop, visiting,
				"backing area of \"" + top_path + "\"");
		verbose("done traversing backing area");
	    }

	    if (keep)
	    {
		*iter = btop;
	    }
	    else
	    {
		backing_areas.erase(iter, next);
	    }
	    iter = next;
	}
    }
    verbose("done with all backing areas of " + top_path);

    visiting.erase(top_path);
    forests[top_path] = forest;
    verbose("done traversing forest from " + top_path);
}

void
Abuild::mergeForests(BuildForest_map& forests,
		     DependencyGraph& external_graph)
{
    assert(this->compat_level.allow_1_0());

    // If A backs to independent forests B and C, which in turn both
    // reference common but independent external D, we can't tell that
    // B, C, and D are all supposed to be part of the same forest
    // until something, such as traversal of A, causes us to traverse
    // both B and C.  In 1.0 compatibility mode, we don't know about
    // externals until we are already finished calling traverseItems,
    // so we are stuck merging forests after the fact.  The
    // items_traversed set prevents us from initially adding any build
    // item or tree to more than one forest, so the merging is
    // relatively straightforward once we determine which forests have
    // to be merged.

    std::map<std::string, std::string> forest_renames;
    std::map<std::string, std::list<std::string> > forest_merges;

    // Figure out independent subsets of the graph formed by external
    // relationships.  We only care about forests that we have
    // previously considered to be forest roots.  (Remember that we
    // are already grouping separate forests together as one in the
    // case of externals.)  For any subset that contains more than one
    // apparent forest root, we need to merge the forests in that
    // subset.
    std::vector<DependencyGraph::ItemList> const& sets =
	external_graph.getIndependentSets();
    for (std::vector<DependencyGraph::ItemList>::const_iterator iter =
	     sets.begin();
	 iter != sets.end(); ++iter)
    {
	DependencyGraph::ItemList const& set = *iter;
	std::list<std::string> merge;
	for (DependencyGraph::ItemList::const_iterator iter =
		 set.begin();
	     iter != set.end(); ++iter)
	{
	    if (forests.count(*iter))
	    {
		merge.push_back(*iter);
	    }
	}
	if (merge.size() <= 1)
	{
	    continue;
	}
	QTC::TC("abuild", "Abuild forest merge required");
	std::string first = merge.back();
	merge.pop_back();
	forest_merges[first] = merge;
	for (DependencyGraph::ItemList::iterator iter = merge.begin();
	     iter != merge.end(); ++iter)
	{
	    std::string const& other = *iter;
	    forest_renames[other] = first;
	}
    }

    // To merge forests, we just merge their trees, items, backing
    // areas, and backing area data.
    for (std::map<std::string, std::list<std::string> >::iterator i1 =
	     forest_merges.begin();
	 i1 != forest_merges.end(); ++i1)
    {
	std::string const& keep_dir = (*i1).first;
	BuildForest& keep = *(forests[keep_dir]);

	std::list<std::string>& backing_areas = keep.getBackingAreas();
	std::set<std::string>& deleted_trees = keep.getDeletedTrees();
	std::set<std::string>& deleted_items = keep.getDeletedItems();

	std::list<std::string> const& to_remove = (*i1).second;
	for (std::list<std::string>::const_iterator i2 = to_remove.begin();
	     i2 != to_remove.end(); ++i2)
	{
	    std::string const& remove_dir = *i2;
	    BuildForest& remove = *(forests[remove_dir]);

	    verbose("merging forest \"" + remove_dir + "\" into \"" +
		    keep_dir);

	    BuildTree_map& o_buildtrees = remove.getBuildTrees();
	    BuildItem_map& o_builditems = remove.getBuildItems();
	    std::list<std::string>& o_backing_areas = remove.getBackingAreas();
	    std::set<std::string>& o_deleted_trees = remove.getDeletedTrees();
	    std::set<std::string>& o_deleted_items = remove.getDeletedItems();

	    for (BuildTree_map::iterator i3 = o_buildtrees.begin();
		 i3 != o_buildtrees.end(); ++i3)
	    {
		std::string const& tree_name = (*i3).first;
		BuildTree_ptr tree = (*i3).second;
		addTreeToForest(keep, tree_name, tree);
	    }

	    for (BuildItem_map::iterator i3 = o_builditems.begin();
		 i3 != o_builditems.end(); ++i3)
	    {
		std::string const& item_name = (*i3).first;
		BuildItem_ptr item = (*i3).second;
		addItemToForest(keep, item_name, item);
	    }

	    backing_areas.insert(
		backing_areas.end(),
		o_backing_areas.begin(), o_backing_areas.end());
	    deleted_trees.insert(
		o_deleted_trees.begin(), o_deleted_trees.end());
	    deleted_items.insert(
		o_deleted_items.begin(), o_deleted_items.end());

	    forests.erase(remove_dir);
	}
    }

    for (BuildForest_map::iterator iter = forests.begin();
	 iter != forests.end(); ++iter)
    {
	std::string const& root = (*iter).first;
	BuildForest& forest = *((*iter).second);
	std::list<std::string>& backing_areas = forest.getBackingAreas();

	// coalesce backing areas
	std::set<std::string> ba_set;
	std::list<std::string>::iterator iter = backing_areas.begin();
	while (iter != backing_areas.end())
	{
	    std::list<std::string>::iterator next = iter;
	    ++next;
	    bool keep = false;
	    std::string ba = *iter;
	    if (forest_renames.count(ba))
	    {
		verbose("in forest \"" + root +
			"\", replacing backing area \"" +
			ba + "\" with \"" + forest_renames[ba] + "\"");
		ba = forest_renames[ba];
	    }
	    if (ba_set.count(ba))
	    {
		// skip duplicate
	    }
	    else
	    {
		ba_set.insert(ba);
		keep = true;
	    }
	    if (keep)
	    {
		*iter = ba;
	    }
	    else
	    {
		backing_areas.erase(iter, next);
	    }
	    iter = next;
	}
    }
}

void
Abuild::removeEmptyTrees(BuildForest_map& forests)
{
    assert(this->compat_level.allow_1_0());

    // Top-level child-only build items look like build tree roots
    // with 1.0 compatibility turned on.  If any of our build trees
    // are like this and have no items, remove them from the forest.
    // Otherwise, we end up with assigned tree names on empty trees.
    for (BuildForest_map::iterator iter = forests.begin();
	 iter != forests.end(); ++iter)
    {
	BuildForest& forest = *((*iter).second);
	std::set<std::string> to_delete;
	BuildTree_map& buildtrees = forest.getBuildTrees();
	for (BuildTree_map::iterator iter = buildtrees.begin();
	     iter != buildtrees.end(); ++iter)
	{
	    std::string const& tree_name = (*iter).first;
	    BuildTree& tree = *((*iter).second);
	    ItemConfig* config = readConfig(tree.getRootPath(), "");
	    if (config->isChildOnly())
	    {
		// We'll only delete this if it has no items and no
		// one depends on it.
		to_delete.insert(tree_name);
	    }
	}

	for (BuildTree_map::iterator iter = buildtrees.begin();
	     iter != buildtrees.end(); ++iter)
	{
	    BuildTree& tree = *((*iter).second);
	    std::list<std::string> const& tree_deps = tree.getTreeDeps();
	    for (std::list<std::string>::const_iterator iter =
		     tree_deps.begin();
		 iter != tree_deps.end(); ++iter)
	    {
		to_delete.erase(*iter);
	    }
	}

	BuildItem_map& builditems = forest.getBuildItems();
	for (BuildItem_map::iterator iter = builditems.begin();
	     iter != builditems.end(); ++iter)
	{
	    BuildItem& item = *((*iter).second);
	    to_delete.erase(item.getTreeName());
	}

	if (! to_delete.empty())
	{
	    QTC::TC("abuild", "Abuild delete unused empty top-level tree");
	    for (std::set<std::string>::iterator iter = to_delete.begin();
		 iter != to_delete.end(); ++iter)
	    {
		verbose("tree name " + *iter + " was assigned to an unused,"
			" child-only item; removing unneeded tree");
		buildtrees.erase(*iter);
	    }
	    if (to_delete.count(this->local_tree))
	    {
		QTC::TC("abuild", "Abuild clear local tree");
		this->local_tree.clear();
	    }
	}
    }
}

void
Abuild::traverseItems(BuildForest& forest, DependencyGraph& external_graph,
		      std::string const& top_path,
		      std::list<std::string>& dirs_with_externals,
		      std::list<std::string>& backing_areas,
		      std::set<std::string>& deleted_trees,
		      std::set<std::string>& deleted_items)
{
    if (this->compat_level.allow_1_0())
    {
	if (this->items_traversed.count(top_path))
	{
	    QTC::TC("abuild", "Abuild forest has already been seen");
	    return;
	}
	external_graph.addItem(top_path);
	this->items_traversed.insert(top_path);
    }

    bool has_backing_area =
	Util::isFile(top_path + "/" + BackingConfig::FILE_BACKING);
    if (has_backing_area)
    {
	verbose("reading backing area data");
	appendBackingData(
	    top_path, backing_areas, deleted_trees, deleted_items);
	verbose("done reading backing area data");
    }

    std::list<std::string> dirs;
    std::map<std::string, std::string> parent_dirs;
    std::map<std::string, std::string> dir_trees;
    std::map<std::string, std::string> dir_tree_roots; // 1.0-compat
    std::set<std::string> upgraded_tree_roots;	       // 1.0-compat
    dirs.push_back(top_path);
    while (! dirs.empty())
    {
	std::string dir = dirs.front();
	dirs.pop_front();
	std::string parent_dir;
	if (parent_dirs.count(dir))
	{
	    parent_dir = parent_dirs[dir];
	}
        ItemConfig* config = readConfig(dir, parent_dir);
	FileLocation location = config->getLocation();

	std::string tree_name;
	std::string tree_root;	// 1.0-compat
	if (dir_trees.count(parent_dir))
	{
	    tree_name = dir_trees[parent_dir];
	    if (this->compat_level.allow_1_0())
	    {
		tree_root = dir_tree_roots[parent_dir];
	    }
	}
	if (config->isTreeRoot())
	{
	    tree_name = registerBuildTree(forest, dir,
					  config, dirs_with_externals);
	    if (this->compat_level.allow_1_0())
	    {
		tree_root = dir;
		if (! config->getTreeName().empty())
		{
		    upgraded_tree_roots.insert(dir);
		}
	    }
	}
	dir_trees[dir] = tree_name;
	if (this->compat_level.allow_1_0())
	{
	    dir_tree_roots[dir] = tree_root;
	    if (config->usesDeprecatedFeatures())
	    {
		this->suggest_upgrade = true;
		if (upgraded_tree_roots.count(tree_root))
		{
		    QTC::TC("abuild", "Abuild basic deprecation warning");
		    deprecate("1.1", config->getLocation(),
			      "this file uses deprecated features, and"
			      " this build item belongs to a tree that"
			      " has already been upgraded");
		}
	    }
	}

	if (dir == this->this_config_dir)
	{
	    this->local_tree = tree_name;
	}

	std::string item_name = config->getName();
        if (! item_name.empty())
        {
	    BuildItem_ptr item(
		new BuildItem(item_name, tree_name, config));
	    addItemToForest(forest, item_name, item);
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
		    dirs.push_back(child_dir);
		    parent_dirs[child_dir] = dir;
                }
                else
                {
                    QTC::TC("abuild", "Abuild ERR no child config");
                    error(location, "child " + child_conf + " is missing");
                }
            }
            else
            {
                if (has_backing_area)
                {
                    // Allow sparse trees if we have a backing area.
                    // No validation is required for the child
                    // directory.
                    QTC::TC("abuild", "Abuild sparse tree");
		    verbose("ignoring non-existence of child dir " +
			    *iter + " from " + Util::absToRel(dir) +
			    " in a tree with a backing area");
                }
                else
                {
                    QTC::TC("abuild", "Abuild ERR can't resolve child");
                    error(location, "unable to find child " + *iter);
                }
            }
        }
    }
}

void
Abuild::addItemToForest(BuildForest& forest, std::string const& item_name,
			BuildItem_ptr item)
{
    BuildItem_map& builditems = forest.getBuildItems();
    if (builditems.count(item_name))
    {
	FileLocation const& loc = item->getLocation();
	FileLocation const& other_loc =
	    builditems[item_name]->getLocation();
	if (loc == other_loc)
	{
	    // This can only happen if the item is encountered from
	    // different parents, which would always be accompanied by
	    // an intervening Abuild.conf.  Don't report this an error
	    // so we avoid reporting the same condition with multiple
	    // error messages.
	    QTC::TC("abuild", "Abuild item added twice");
	}
	else
	{
	    QTC::TC("abuild", "Abuild ERR item multiple locations");
	    error(loc, "build item " + item_name +
		  " appears in multiple locations");
	    error(other_loc, "here is another location");
	}
    }
    else if (item->getTreeName().empty())
    {
	QTC::TC("abuild", "Abuild ERR named item outside of tree");
	error(item->getLocation(),
	      "named build items are not allowed outside of"
	      " build trees");
    }
    else
    {
	// We want to make sure that item->getTreeName will always be
	// a valid tree, so only store the build item if everything
	// checks out.
	item->setForestRoot(forest.getRootPath());
	builditems[item_name] = item;
    }
}

std::string
Abuild::registerBuildTree(BuildForest& forest,
			  std::string const& dir,
			  ItemConfig* config,
			  std::list<std::string>& dirs_with_externals)
{
    std::string tree_name = config->getTreeName();
    std::list<std::string> tree_deps = config->getTreeDeps();

    if (this->compat_level.allow_1_0())
    {
	// Assign a name if needed.
	if (tree_name.empty())
	{
	    tree_name = getAssignedTreeName(dir);
	}

	// Map any externals to tree dependencies.
	std::list<std::string> const& externals = config->getExternals();
	if (! externals.empty())
	{
	    dirs_with_externals.push_back(dir);
	}
	for (std::list<std::string>::const_iterator eiter = externals.begin();
	     eiter != externals.end(); ++eiter)
	{
	    std::string const& edecl = *eiter;
	    ItemConfig* econfig = readExternalConfig(dir, edecl);
	    if (econfig)
	    {
		std::string const& epath = econfig->getAbsolutePath();
		if (econfig->isTreeRoot())
		{
		    std::string ext_tree_name = econfig->getTreeName();
		    if (ext_tree_name.empty())
		    {
			ext_tree_name = getAssignedTreeName(epath);
		    }
		    else
		    {
			this->suggest_upgrade = true;
			deprecate("1.1", config->getLocation(),
				  "external \"" + edecl +
				  "\" of \"" + dir + "\" points to"
				  " an named tree in an upgraded"
				  " build area");
		    }
		    tree_deps.push_back(ext_tree_name);
		}
		else
		{
		    QTC::TC("abuild", "Abuild ERR external not root");
		    error(FileLocation(epath + "/" +
				       ItemConfig::FILE_CONF, 0, 0),
			  "this build item does not appear to be"
			  " a build tree root, but it is referenced"
			  " as external \"" + edecl + "\" of \"" +
			  dir + "\"");
		    error(config->getLocation(), "here is the referring item");
		}
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild ERR can't resolve external");
		error(config->getLocation(),
		      "unable to locate or resolve external \"" +
		      edecl + "\"");
	    }
	}
    }
    else
    {
	assert(! tree_name.empty());
    }

    BuildTree_ptr tree(
	new BuildTree(tree_name, dir, tree_deps,
		      config->getSupportedTraits(),
		      config->getPlugins(),
		      this->internal_platform_data));
    addTreeToForest(forest, tree_name, tree);

    return tree_name;
}

void
Abuild::addTreeToForest(BuildForest& forest, std::string const& tree_name,
			BuildTree_ptr tree)
{
    BuildTree_map& buildtrees = forest.getBuildTrees();
    if (buildtrees.count(tree_name))
    {
	if (tree->getRootPath() == buildtrees[tree_name]->getRootPath())
	{
	    // See comments in addItemToForest for an explanation of
	    // when this can happen.  We ignore this here to avoid
	    // having error messages for the same error.
	}
	else
	{
	    QTC::TC("abuild", "Abuild ERR duplicate tree");
	    error(tree->getLocation(),
		  "another tree with the name \"" + tree_name + "\" has been"
		  " found in this forest");
	    error(buildtrees[tree_name]->getLocation(),
		  "here is the other tree");
	}
    }
    else
    {
	tree->setForestRoot(forest.getRootPath());
	buildtrees[tree_name] = tree;
    }
}

std::string
Abuild::getAssignedTreeName(std::string const& dir,
			    bool use_backing_name_only)
{
    assert(this->compat_level.allow_1_0());

    if (this->assigned_tree_names.count(dir))
    {
	QTC::TC("abuild", "Abuild previously assigned tree name");
	return this->assigned_tree_names[dir];
    }

    // Assign a name to this tree.  In order for inheriting externals
    // from backing areas to work properly, we need to take the
    // backing area's tree name if this is a 1.0 root with a backing
    // file.  This means we end up traversing the backing chain at
    // this point.
    std::set<std::string> visiting;
    return getAssignedTreeName(dir, visiting, use_backing_name_only);
}

std::string
Abuild::getAssignedTreeName(std::string const& dir,
			    std::set<std::string>& visiting,
			    bool use_backing_name_only)
{
    assert(this->compat_level.allow_1_0());

    // We check visiting before calling recursively so we can create a
    // better error message.
    assert(! visiting.count(dir));
    visiting.insert(dir);

    std::string tree_name;

    if (Util::isFile(dir + "/" + BackingConfig::FILE_BACKING))
    {
	BackingConfig* backing = readBacking(dir);
	if (backing->isDeprecated() &&
	    (! backing->getBackingAreas().empty()))
	{
	    // This is an old-style backing area.  It must point to a
	    // tree root which is presumably the root of the
	    // corresponding tree in the backing area.

	    std::list<std::string> const& backing_areas =
		backing->getBackingAreas();
	    assert(backing_areas.size() == 1);
	    std::string const& backing_area = backing_areas.front();
	    if (! Util::isFile(backing_area + "/" + ItemConfig::FILE_CONF))
	    {
		// This may be precluded by other code.  Don't bother
		// with a coverage case.
		fatal("1.0-style " + BackingConfig::FILE_BACKING +
		      " file in " + dir + " appears not to point"
		      " to a directory containing " + ItemConfig::FILE_CONF);
	    }
	    ItemConfig* config = readConfig(backing_area, "");
	    if (! config->isTreeRoot())
	    {
		fatal("1.0-style " + BackingConfig::FILE_BACKING +
		      " file in " + dir + " appears not to point"
		      " to a tree root");
	    }
	    tree_name = config->getTreeName();
	    if (! tree_name.empty())
	    {
		QTC::TC("abuild", "Abuild inherit tree name from backing");
	    }
	    else if (this->assigned_tree_names.count(backing_area))
	    {
		// I don't think this can actually happen because of
		// the order in which we see build trees.
		tree_name = this->assigned_tree_names[backing_area];
	    }
	    else
	    {
		if (visiting.count(backing_area))
		{
		    QTC::TC("abuild", "Abuild ERR backing area loop");
		    fatal("backing area loop found for " + backing_area +
			  ", a backing area of " + dir);
		}
		else
		{
		    QTC::TC("abuild", "Abuild trying backing of backing");
		    tree_name = getAssignedTreeName(
			backing_area, visiting, use_backing_name_only);
		}
	    }
	}
    }

    if (tree_name.empty() && (! use_backing_name_only))
    {
	// If we got here, this tree doesn't have a backing area, so
	// we'll have to generate a tree name.  The random number is
	// there to prevent people from ever being able to rely on
	// what the tree is called, which in turn should hopefully
	// encourage people to upgrade.
	tree_name = "tree." +
	    Util::intToString(++this->last_assigned_tree_number) +
	    ".-" + Util::intToString(std::rand() % 9999) + "-";
	this->assigned_tree_names[dir] = tree_name;
	verbose("assigned tree name " + tree_name + " to " + dir);
    }

    visiting.erase(dir);

    return tree_name;
}

void
Abuild::validateForest(BuildForest_map& forests, std::string const& top_path)
{
    verbose("build tree " + top_path + ": validating");

    BuildForest& forest = *(forests[top_path]);

    // Many of these checks have side effects.  The order of these
    // checks is very sensitive as some checks depend upon side
    // effects of other operations having been completed.
    resolveFromBackingAreas(forests, top_path);
    checkTreeDependencies(forest);
    checkTreeAccess(forest);
    resolveTraits(forest);
    checkPlatformTypes(forest);
    checkPlugins(forest);
    checkItemNames(forest);
    checkBuildAlso(forest);
    checkItemDependencies(forest);
    updatePlatformTypes(forest);
    checkDependencyPlatformTypes(forest);
    checkFlags(forest);
    checkTraits(forest);
    checkIntegrity(forests, top_path);
    if (this->full_integrity)
    {
	reportIntegrityErrors(forests, forest.getBuildItems(), top_path);
    }
    computeBuildablePlatforms(forest);

    verbose("build tree " + top_path + ": validation completed");
}

void
Abuild::checkTreeDependencies(BuildForest& forest)
{
    // Make sure that there are no cycles or errors (references to
    // non-existent trees) in the dependency graph of build trees.
    // This code is essentially identical to checkItemDependencies
    // but with enough surface differences to be different code.

    // Create a dependency graph for all build trees in this forest.

    BuildTree_map& buildtrees = forest.getBuildTrees();

    DependencyGraph g;
    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	std::string const& tree_name = (*iter).first;
	BuildTree& tree = *((*iter).second);
	g.addItem(tree_name);
	std::list<std::string> const& dependencies = tree.getTreeDeps();
	for (std::list<std::string>::const_iterator i2 = dependencies.begin();
	     i2 != dependencies.end(); ++i2)
        {
            g.addDependency(tree_name, *i2);
        }
    }

    bool check_okay = g.check();

    // Whether or not we found errors, create a table indicating which
    // trees can see which other trees.
    std::map<std::string, std::set<std::string> >& tree_access_table =
	forest.getTreeAccessTable();

    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	std::string const& tree_name = (*iter).first;
	BuildTree& tree = *((*iter).second);
	DependencyGraph::ItemList const& sdeps =
	    g.getSortedDependencies(tree_name);
	tree.setExpandedTreeDeps(sdeps);
	for (std::list<std::string>::const_iterator i2 = sdeps.begin();
	     i2 != sdeps.end(); ++i2)
	{
	    tree_access_table[tree_name].insert(*i2);
	}
    }

    forest.setSortedTreeNames(g.getSortedGraph());

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
		QTC::TC("abuild", "Abuild ERR unknown tree dependency");
		error(buildtrees[node]->getLocation(),
		      "tree " + node + " depends on unknown build tree " +
		      unknown);
	    }
	}

	std::set<std::string> cycle_trees;
	for (std::vector<DependencyGraph::ItemList>::const_iterator iter =
		 cycles.begin();
	     iter != cycles.end(); ++iter)
	{
	    DependencyGraph::ItemList const& data = *iter;
	    QTC::TC("abuild", "Abuild ERR circular tree dependency");
	    std::string cycle = Util::join(" -> ", data);
	    cycle += " -> " + data.front();
	    error("circular dependency detected among build trees: " + cycle);
	    for (DependencyGraph::ItemList::const_iterator i2 = data.begin();
		 i2 != data.end(); ++i2)
	    {
		cycle_trees.insert(*i2);
	    }
	}

	for (std::set<std::string>::iterator iter = cycle_trees.begin();
	     iter != cycle_trees.end(); ++iter)
        {
            error(buildtrees[*iter]->getLocation(),
		  *iter + " participates in a circular tree dependency");
        }
    }
}

void
Abuild::resolveFromBackingAreas(BuildForest_map& forests,
				std::string const& top_path)
{
    BuildForest& forest = *(forests[top_path]);
    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();
    std::list<std::string>& backing_areas = forest.getBackingAreas();
    std::set<std::string> const& trees_to_delete = forest.getDeletedTrees();
    std::set<std::string> const& items_to_delete = forest.getDeletedItems();
    std::set<std::string> trees_not_deleted = trees_to_delete;
    std::set<std::string> items_not_deleted = items_to_delete;

    // The most likely location for a problem is the forest root
    // Abuild.backing file.  In 1.0 compatibility mode, it might be
    // some Abuild.conf, but we're not going to keep track of the
    // origin of ever single item/tree deletion request.
    FileLocation location(top_path + "/" + BackingConfig::FILE_BACKING, 0, 0);

    // If A backs to B and C and if B backs to C, we want to ignore
    // backing area C of A and let A get C's items through B.  This
    // helps to seeing duplicates needlessly.  We'll make this
    // determination in a manner that allows us to preserve the order
    // in which backing areas were specified to the maximum possible
    // extent.
    DependencyGraph backing_graph;
    computeBackingGraph(forests, backing_graph);
    std::set<std::string> covered;
    for (std::list<std::string>::iterator iter = backing_areas.begin();
	 iter != backing_areas.end(); ++iter)
    {
	std::list<std::string> deps =
	    backing_graph.getSortedDependencies(*iter);
	assert(deps.back() == *iter);
	deps.pop_back();
	covered.insert(deps.begin(), deps.end());
    }

    { // local scope
	std::list<std::string>::iterator iter = backing_areas.begin();
	while (iter != backing_areas.end())
	{
	    std::list<std::string>::iterator next = iter;
	    ++next;
	    if (covered.count(*iter))
	    {
		// It is not necessary to tell the user about this.
		// The user might have done this on purpose to
		// indicate a desire to back from the first backing
		// area's backing area even if the first one is later
		// reconfigured, or it may be a remnant from before
		// one of the backing areas pointed to the other.  As
		// we are able to work around this safely and the
		// situation is not harmful, issuing a warning would
		// just be an annoyance.
		QTC::TC("abuild", "Abuild skipping covered backing area");
		verbose("backing area " + *iter + " is being removed because"
			" it is covered by another backing area");
		backing_areas.erase(iter, next);
	    }
	    iter = next;
	}
    }
    if (backing_areas.size() > 1)
    {
	QTC::TC("abuild", "Abuild multiple backing areas");
    }

    // Copy trees and items from the backing areas.  Exclude any
    // deleted trees, deleted items, or items in deleted trees.

    for (std::list<std::string>::const_iterator iter = backing_areas.begin();
	 iter != backing_areas.end(); ++iter)
    {
	BuildTree_map const& backing_trees = forests[*iter]->getBuildTrees();
	for (BuildTree_map::const_iterator tree_iter = backing_trees.begin();
	     tree_iter != backing_trees.end(); ++tree_iter)
	{
	    std::string const& tree_name = (*tree_iter).first;
	    BuildTree const& tree = *((*tree_iter).second);
	    if (trees_to_delete.count(tree_name))
	    {
		QTC::TC("abuild", "Abuild not copying deleted tree");
		trees_not_deleted.erase(tree_name);
		if (buildtrees.count(tree_name))
		{
		    // This is not an error.  If you want to replace
		    // one tree with another one with the same name,
		    // that's fine as deleting the tree also prevents
		    // copying the old tree's items.  With items, it's
		    // different -- replacing an item is sufficient to
		    // get rid of the old one.  You don't have t do
		    // delete it too.
		    QTC::TC("abuild", "Abuild deleted tree exists locally");
		}
	    }
	    else if (buildtrees.count(tree_name))
	    {
		QTC::TC("abuild", "Abuild override build tree");
	    }
	    else
	    {
		// Copy build tree information from backing area
		buildtrees[tree_name].reset(new BuildTree(tree));
		BuildTree& new_tree = *(buildtrees[tree_name]);
		new_tree.incrementBackingDepth();
	    }
        }

	BuildItem_map const& backing_items = forests[*iter]->getBuildItems();
	for (BuildItem_map::const_iterator item_iter = backing_items.begin();
	     item_iter != backing_items.end(); ++item_iter)
	{
	    std::string const& item_name = (*item_iter).first;
	    BuildItem const& item = *((*item_iter).second);
	    std::string const& tree_name = item.getTreeName();
	    if (trees_to_delete.count(tree_name))
	    {
		QTC::TC("abuild", "Abuild not copying item from deleted tree");
		if (builditems.count(item_name) &&
		    builditems[item_name]->getTreeName() != tree_name)
		{
		    QTC::TC("abuild", "Abuild replace item from deleted tree");
		}
	    }
	    else if (items_to_delete.count(item_name))
	    {
		QTC::TC("abuild", "Abuild not copying deleted item");
		items_not_deleted.erase(item_name);
		if (builditems.count(item_name))
		{
		    QTC::TC("abuild", "Abuild ERR deleted item exists locally");
		    error(location,
			  "item \"" + item_name + "\" is marked for"
			  " deletion, but it appears locally in this"
			  " forest");
		    error(builditems[item_name]->getLocation(),
			  "here is the location of the item in"
			  " this forest");
		}
	    }
	    else if (builditems.count(item_name))
	    {
		if (builditems[item_name]->isLocal())
		{
		    QTC::TC("abuild", "Abuild override build item");
		}
		else
		{
		    FileLocation const& loc = item.getLocation();
		    FileLocation const& other_loc =
			builditems[item_name]->getLocation();
		    // It should be impossible for us to see the same
		    // build item from multiple backing areas.  The
		    // only way this should be able to happen would be
		    // if one backing area depends on the other, and
		    // we've already detected and precluded that case
		    // by checking "covered" above.  If this assertion
		    // fails, there is probably a logic error either
		    // there or in merging forests.
		    assert(! (loc == other_loc));
		    QTC::TC("abuild", "Abuild ERR item multiple backing areas");
		    error(loc, "this item appears in multiple backing areas");
		    error(other_loc, "here is another location for this item");
		}
	    }
	    else
	    {
		// Copy build item information from backing area
		builditems[item_name].reset(new BuildItem(item));
		BuildItem& new_item = *(builditems[item_name]);
		new_item.incrementBackingDepth();
		if (new_item.getBackingDepth() > 1)
		{
		    QTC::TC("abuild", "Abuild backing depth > 1");
		}
	    }
        }
    }

    for (std::set<std::string>::iterator iter = trees_not_deleted.begin();
	 iter != trees_not_deleted.end(); ++iter)
    {
	QTC::TC("abuild", "Abuild ERR deleting non-existent tree");
	error(location,
	      "tree \"" + *iter + "\" was marked for deletion"
	      " but was not seen in a backing area");
    }
    for (std::set<std::string>::iterator iter = items_not_deleted.begin();
	 iter != items_not_deleted.end(); ++iter)
    {
	QTC::TC("abuild", "Abuild ERR deleting non-existent item");
	error(location,
	      "item \"" + *iter + "\" was marked for deletion"
	      " but was not seen in a backing area");
    }
}

void
Abuild::checkTreeAccess(BuildForest& forest)
{
    // Check to make sure no item references an item in a tree that
    // its tree does not depend on

    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	std::list<std::string> const& alldeps = item.getExpandedDependencies();
	for (std::list<std::string>::const_iterator diter = alldeps.begin();
	     diter != alldeps.end(); ++diter)
	{
	    std::string const& dep_name = (*diter);
	    if (! builditems.count(dep_name))
	    {
		continue;
	    }
	    BuildItem& dep_item = *(builditems[dep_name]);
	    if (! checkAllowedTree(forest, item, dep_item, "depend on"))
	    {
		QTC::TC("abuild", "Abuild ERR depend on invisible item");
	    }
	}
    }
}

bool
Abuild::checkAllowedTree(BuildForest& forest,
			 BuildItem& referrer,
			 BuildItem& referent,
			 std::string const& action)
{
    bool okay = true;
    std::map<std::string, std::set<std::string> > const& tree_access_table =
	forest.getTreeAccessTable();
    std::string const& referrer_name = referrer.getName();
    std::string const& referrer_tree = referrer.getTreeName();
    std::string const& referent_name = referent.getName();
    std::string const& referent_tree = referent.getTreeName();

    std::set<std::string> const& allowed_trees =
	(*(tree_access_table.find(referrer_tree))).second;
    if (allowed_trees.count(referent_tree) == 0)
    {
	okay = false;
	error(referrer.getLocation(),
	      "build item \"" + referrer_name + "\" may not " +
	      action + " build item \"" + referent_name + "\" because \"" +
	      referrer_name + "\"'s tree (\"" + referrer_tree +
	      "\") does not depend on \"" + referent_name +
	      "\"'s tree (\"" + referent_tree + "\")");
    }
    return okay;
}

void
Abuild::resolveTraits(BuildForest& forest)
{
    // Merge supported traits of each tree's dependencies into the tree.

    BuildTree_map& buildtrees = forest.getBuildTrees();
    std::list<std::string> treenames = forest.getSortedTreeNames();
    for (std::list<std::string>::iterator iter = treenames.begin();
	 iter != treenames.end(); ++iter)
    {
	std::string const& tree_name = *iter;
	BuildTree& tree = *(buildtrees[tree_name]);
	std::list<std::string> const& deps = tree.getExpandedTreeDeps();
	for (std::list<std::string>::const_iterator i2 = deps.begin();
	     i2 != deps.end(); ++i2)
	{
	    if (buildtrees.count(*i2) == 0)
	    {
		// An error would have reported already
		continue;
	    }
	    BuildTree& dtree = *(buildtrees[*i2]);
	    tree.addTraits(dtree.getSupportedTraits());
	}
    }
}

void
Abuild::checkPlugins(BuildForest& forest)
{
    // Make sure each plugin exists and lives in an accessible tree,
    // and set each tree's plugin list.

    BuildItem_map& builditems = forest.getBuildItems();
    BuildTree_map& buildtrees = forest.getBuildTrees();

    // tree -> plugins in that tree
    std::map<std::string, std::set<std::string> > plugin_data;

    std::map<std::string, std::set<std::string> > const& access_table =
	forest.getTreeAccessTable();
    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	std::string const& tree_name = (*iter).first;
	plugin_data[tree_name].empty(); // force this entry to exist
	BuildTree& tree = *((*iter).second);
	std::set<std::string> const& allowed_trees =
	    (*(access_table.find(tree_name))).second;
	std::list<std::string> const& plugins = tree.getPlugins();
	for (std::list<std::string>::const_iterator iter = plugins.begin();
	     iter != plugins.end(); ++iter)
	{
	    std::string const& item_name = *iter;
	    if (builditems.count(item_name) == 0)
	    {
		QTC::TC("abuild", "Abuild ERR invalid plugin");
		error(tree.getLocation(),
		      "plugin \"" + item_name + "\" does not exist");
		continue;
	    }
	    BuildItem& item = *(builditems[item_name]);
	    std::string const& item_tree = item.getTreeName();
	    if (allowed_trees.count(item_tree) == 0)
	    {
		QTC::TC("abuild", "Abuild ERR plugin in invisible tree");
		error(tree.getLocation(),
		      "this tree may not declare item \"" +
		      item_name + "\" as a plugin because because its"
		      " tree does not depend on \"" + item_name +
		      "\"'s tree (\"" + item_tree + "\")");
		continue;
	    }

	    this->plugins.insert(item_name);
	    plugin_data[tree_name].insert(item_name);

	    bool item_error = false;
	    // Although whether or not an item has dependencies is not
	    // context-specific, whether or not an item is a plugin
	    // is.
	    if (! item.getDeps().empty())
	    {
		QTC::TC("abuild", "Abuild ERR plugin with dependencies");
		item_error = true;
		error(tree.getLocation(),
		      "item \"" + item_name + "\" is declared as a plugin,"
		      " but it has dependencies");
	    }
	    if (! item.getBuildAlso().empty())
	    {
		QTC::TC("abuild", "Abuild ERR plugin with build-also");
		item_error = true;
		error(tree.getLocation(),
		      "item \"" + item_name + "\" is declared as a plugin,"
		      " but it specifies other items to build");
	    }

	    // checkPlatformTypes() must have already been called.
	    // Require all plugins to be build items of target type
	    // "all".  This means that the don't build anything or
	    // have an Abuild.interface file.
	    if (item.getTargetType() != TargetType::tt_all)
	    {
		QTC::TC("abuild", "Abuild ERR plugin with target type !all");
		item_error = true;
		error(tree.getLocation(),
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
    }

    // Check to make sure no item depends on an item that has been
    // declared as a plugin for its tree.  Also store the list of
    // plugins each build item should see.  Do this only for items
    // local to this forest.

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}

	// Store the list of plugins
	std::string const& tree_name = item.getTreeName();
	BuildTree& tree = *(buildtrees[tree_name]);
	item.setPlugins(tree.getPlugins());

	// Check dependencies
	std::list<std::string> const& deps = item.getDeps();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
	{
	    std::string const& dep_name = (*dep_iter);
	    // If the dependency doesn't exist, we'll be reporting
	    // that later.
	    if (builditems.count(dep_name) != 0)
	    {
		if (plugin_data[tree_name].count(dep_name))
		{
		    QTC::TC("abuild", "Abuild ERR item depends on plugin");
		    error(item.getLocation(),
			  "this item depends on \"" + dep_name + "\","
			  " which is declared as a plugin");
		    error(tree.getLocation(),
			  "the dependency is declared as a plugin here");
		}
	    }
	}
    }
}

bool
Abuild::isPlugin(std::string const& item)
{
    return (this->plugins.count(item) != 0);
}

void
Abuild::checkPlatformTypes(BuildForest& forest)
{
    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();

    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	BuildTree& tree = *((*iter).second);
	PlatformData& platform_data = tree.getPlatformData();

	// Load any additional platform data from our plugins
	std::list<std::string> const& plugins = tree.getPlugins();
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
    }

    // Validate all local build items to ensure that they have valid
    // platform types.

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);

	// Check only local build items
	if (! item.isLocal())
	{
	    continue;
	}

	BuildTree& tree = *(buildtrees[item.getTreeName()]);
	PlatformData& platform_data = tree.getPlatformData();
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
Abuild::checkItemNames(BuildForest& forest)
{
    BuildItem_map& builditems = forest.getBuildItems();
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

void
Abuild::checkBuildAlso(BuildForest& forest)
{
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	std::string const& item_name = (*iter).first;
	FileLocation const& item_location = item.getLocation();

	std::list<std::string> const& build_also = item.getBuildAlso();
	for (std::list<std::string>::const_iterator biter =
		 build_also.begin();
	     biter != build_also.end(); ++biter)
	{
	    std::string const& other_item = *biter;
	    if (builditems.count(other_item) == 0)
	    {
		QTC::TC("abuild", "Abuild ERR invalid build also");
		error(item_location,
		      item_name + " requests building of unknown build item " +
		      other_item);
		continue;
	    }
	    if (! checkAllowedTree(forest, item, *(builditems[other_item]),
				   "request build of"))
	    {
		QTC::TC("abuild", "Abuild ERR build-also invisible item");
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
Abuild::checkItemDependencies(BuildForest& forest)
{
    // Make sure that there are no cycles or errors (references to
    // non-existent build items) in the dependency graph.

    BuildItem_map& builditems = forest.getBuildItems();
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

    forest.setSortedItemNames(g.getSortedGraph());

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
Abuild::updatePlatformTypes(BuildForest& forest)
{
    // For every build item with target type "all", if all the item's
    // direct dependencies have the same platform types, inherit
    // platform types and target type from them.  We have to perform
    // this check in reverse dependency order so that any dependencies
    // that were originally target type "all" but didn't have to be
    // have already been updated.

    BuildItem_map& builditems = forest.getBuildItems();
    std::list<std::string> const& sorted_items =
	forest.getSortedItemNames();
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
Abuild::checkDependencyPlatformTypes(BuildForest& forest)
{
    // Verify that each dependency that is declared with a specific
    // platform type is from an item whose target type is not tt_all
    // and to an item that has the given platform type.  Must be
    // called after updatePlatformTypes.

    BuildItem_map& builditems = forest.getBuildItems();
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
Abuild::checkFlags(BuildForest& forest)
{
    BuildItem_map& builditems = forest.getBuildItems();
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
Abuild::checkTraits(BuildForest& forest)
{
    // For each build item, make sure each trait is allowed in the
    // current build tree and that any referent build items are
    // accessible.  Check only items that are local to the current
    // tree.

    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}
	std::set<std::string> const& supported_traits =
	    buildtrees[item.getTreeName()]->getSupportedTraits();
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
		if (builditems.count(referent_item) == 0)
		{
		    QTC::TC("abuild", "Abuild ERR invalid trait referent");
		    error(location, "trait " + trait +
			  " refers to item " + referent_item +
			  " which does not exist");
		}
		else if (! checkAllowedTree(
			     forest, item, *builditems[referent_item],
			     "refer by trait to"))
		{
		    QTC::TC("abuild", "Abuild ERR invisible trait referent");
		}
	    }
	}
    }
}

void
Abuild::checkIntegrity(BuildForest_map& forests,
		       std::string const& top_path)
{
    // Check abuild's basic integrity guarantee.  Refer to the manual
    // for details.

    std::set<std::string> plugin_check_forests;

    // Find items with shadowed dependencies.  An item has a shadowed
    // dependency if the local forest resolves the item to a different
    // path than the item's native forest.  Local items therefore
    // can't have shadowed dependencies.  (They can still have
    // dependencies with shadowed dependencies, so they can still have
    // integrity errors.)  Also store a list of all forests for which
    // we have to perform plugin checks.
    BuildForest& forest = *(forests[top_path]);
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (item.isLocal())
	{
	    continue;
	}

	std::string const& item_forest = item.getForestRoot();
	plugin_check_forests.insert(item_forest);
	BuildItem_map& item_forest_items =
	    forests[item_forest]->getBuildItems();
	std::set<std::string> shadowed_dependencies;

	std::list<std::string> const& deps = item.getExpandedDependencies();
	for (std::list<std::string>::const_iterator dep_iter = deps.begin();
	     dep_iter != deps.end(); ++dep_iter)
        {
	    std::string const& dep_name = (*dep_iter);
	    if ((builditems.count(dep_name) == 0) ||
		(item_forest_items.count(dep_name) == 0))
	    {
		// unknown dependency error already reported
		continue;
	    }

	    if (builditems[dep_name]->getAbsolutePath() !=
		item_forest_items[dep_name]->getAbsolutePath())
	    {
		// The instance of dep_name that item would see is
		// shadowed.
		shadowed_dependencies.insert(dep_name);
	    }
        }

	item.setShadowedDependencies(shadowed_dependencies);
    }

    // Create a mapping of name to path for each plugin that is
    // declared by the local forest and also present natively in the
    // local forest.  Any such plugin could potentially be shadowing a
    // plugin in a more distant forest.
    std::map<std::string, std::string> plugin_paths;
    BuildTree_map& buildtrees = forest.getBuildTrees();
    for (BuildTree_map::iterator iter = buildtrees.begin();
	 iter != buildtrees.end(); ++iter)
    {
	BuildTree& tree = *((*iter).second);
	std::list<std::string> const& plugins = tree.getPlugins();
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
    }

    // For each plugin check forest, see if that forest defines any of
    // the same plugins the local forest does and resolves them to a
    // different location.
    for (std::set<std::string>::const_iterator iter =
	     plugin_check_forests.begin();
	 iter != plugin_check_forests.end(); ++iter)
    {
	std::string const& other_top_path = *iter;
	BuildForest& other_forest = *(forests[other_top_path]);
	BuildTree_map& other_trees = other_forest.getBuildTrees();
	BuildItem_map& other_items = other_forest.getBuildItems();
	for (BuildTree_map::iterator iter = other_trees.begin();
	     iter != other_trees.end(); ++iter)
	{
	    BuildTree& other_tree = *((*iter).second);
	    std::list<std::string> const& other_plugins =
		other_tree.getPlugins();
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
    }

    // For each non-local build item, if it references shadowed
    // plugins, store this fact.

    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (item.isLocal())
	{
	    continue;
	}

	std::string const& item_forest = item.getForestRoot();
	if ((this->shadowed_plugins.count(top_path) != 0) &&
	    (this->shadowed_plugins[top_path].count(item_forest) != 0))
	{
	    item.addShadowedPlugin(top_path, item_forest);
	}
    }
}

void
Abuild::reportIntegrityErrors(BuildForest_map& forests,
			      BuildItem_map& builditems,
			      std::string const& top_path)
{
    std::set<std::string> integrity_error_items;
    std::map<std::string, std::set<std::string> > plugin_error_forests;

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
	    std::string const& local_forest = (*i2).first;
	    if (this->full_integrity && (local_forest != top_path))
	    {
		// In full integrity mode, let each forest report its
		// own errors.
		continue;
	    }
	    std::set<std::string> const& remote_forests = (*i2).second;
	    for (std::set<std::string>::const_iterator i3 =
		     remote_forests.begin();
		 i3 != remote_forests.end(); ++i3)
	    {
		std::string const& remote_forest = *i3;
		plugin_error_forests[local_forest].insert(remote_forest);
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
	     plugin_error_forests.begin();
	 i1 != plugin_error_forests.end(); ++i1)
    {
	std::string const& local_forest = (*i1).first;
	if (this->full_integrity && (local_forest != top_path))
	{
	    // In full integrity mode, let each forest report its
	    // own errors.
	    continue;
	}
	std::set<std::string> const& remote_forests = (*i1).second;
	for (std::set<std::string>::const_iterator i2 = remote_forests.begin();
	     i2 != remote_forests.end(); ++i2)
	{
	    std::string const& remote_forest = *i2;
	    std::vector<std::string> plugins =
		this->shadowed_plugins[local_forest][remote_forest];
	    FileLocation local_forest_location(
		local_forest + "/" + ItemConfig::FILE_CONF, 0, 0);
	    FileLocation remote_forest_location(
		remote_forest + "/" + ItemConfig::FILE_CONF, 0, 0);
	    BuildItem_map& local_items =
		forests[local_forest]->getBuildItems();
	    QTC::TC("abuild", "Abuild ERR plugin integrity");
	    error(remote_forest_location,
		  "some plugins declared by this forest are shadowed");
	    error(local_forest_location,
		  "this is the forest that contains the shadowing plugins");
	    std::set<FileLocation> reported;
	    for (std::vector<std::string>::const_iterator p_iter =
		     plugins.begin();
		 p_iter != plugins.end(); ++p_iter)
	    {
		std::string const& plugin_name = *p_iter;
		BuildItem& plugin = *(local_items[plugin_name]);
		if (! reported.count(plugin.getLocation()))
		{
		    // avoid reporting the same error more than once,
		    // which can happen for plugins that shadow
		    // something defined more than one backing area
		    // away.
		    reported.insert(plugin.getLocation());
		    error(plugin.getLocation(),
			  "here is the shadowing copy of plugin \"" +
			  plugin_name + "\"");
		}
	    }
	}
    }
}

void
Abuild::computeBuildablePlatforms(BuildForest& forest)
{
    // For each build item, determine the list of platforms that are
    // to be built for that item.  Do this only for items that are
    // native to the local forest.
    BuildTree_map& buildtrees = forest.getBuildTrees();
    BuildItem_map& builditems = forest.getBuildItems();
    for (BuildItem_map::iterator iter = builditems.begin();
	 iter != builditems.end(); ++iter)
    {
	BuildItem& item = *((*iter).second);
	if (! item.isLocal())
	{
	    continue;
	}

	BuildTree& item_tree = *(buildtrees[item.getTreeName()]);
	PlatformData& platform_data = item_tree.getPlatformData();
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
Abuild::appendBackingData(std::string const& dir,
			  std::list<std::string>& backing_areas,
			  std::set<std::string>& deleted_trees,
			  std::set<std::string>& deleted_items)
{
    std::string file_backing = dir + "/" + BackingConfig::FILE_BACKING;
    if (Util::isFile(file_backing))
    {
	BackingConfig* backing = readBacking(dir);
	std::list<std::string> const& dirs = backing->getBackingAreas();
	for (std::list<std::string>::const_iterator iter = dirs.begin();
	     iter != dirs.end(); ++iter)
	{
	    // The backing area must be a directory that contains an
	    // Abuild.conf (which is guaranteed by BackingConfig), and
	    // we must be able to find the root of the forest from
	    // there.  We detect this here when we have enough
	    // information to create a good error message.
	    std::string const& bdir = *iter;
	    std::string btop = findTop(
		bdir, "backing area \"" + bdir +
		"\" from \"" + file_backing + "\"");
	    if (btop.empty())
	    {
		QTC::TC("abuild", "Abuild ERR can't find backing top");
		error(backing->getLocation(),
		      "unable to determine top of forest containing"
		      " backing area \"" + bdir + "\"");
	    }
	    else
	    {
		backing_areas.push_back(bdir);
	    }
	}
	backing->appendBackingData(deleted_trees, deleted_items);
    }

    if (this->compat_level.allow_1_0())
    {
	if (Util::isFile(dir + "/" + ItemConfig::FILE_CONF))
	{
	    ItemConfig* config = readConfig(dir, "");
	    std::set<std::string> const& d = config->getDeleted();
	    deleted_items.insert(d.begin(), d.end());
	}
    }
}

BackingConfig*
Abuild::readBacking(std::string const& dir)
{
    BackingConfig* backing = BackingConfig::readBacking(
	this->error_handler, this->compat_level, dir);
    std::list<std::string> const& backing_areas = backing->getBackingAreas();
    if (backing_areas.empty())
    {
        QTC::TC("abuild", "Abuild ERR invalid backing file");
        error(FileLocation(dir + "/" + BackingConfig::FILE_BACKING, 0, 0),
	      "unable to get backing area data");
    }
    if (backing->isDeprecated())
    {
	this->suggest_upgrade = true;
    }
    return backing;
}

void
Abuild::computeValidTraits(BuildForest_map& forests)
{
    for (BuildForest_map::iterator i1 = forests.begin();
	 i1 != forests.end(); ++i1)
    {
	BuildTree_map& buildtrees = ((*i1).second)->getBuildTrees();
	for (BuildTree_map::iterator iter = buildtrees.begin();
	     iter != buildtrees.end(); ++iter)
	{
	    BuildTree& tree = *((*iter).second);
	    std::set<std::string> const& traits = tree.getSupportedTraits();
	    this->valid_traits.insert(traits.begin(), traits.end());
	}
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
Abuild::listPlatforms(BuildForest_map& forests)
{
    for (BuildForest_map::iterator forest_iter = forests.begin();
	 forest_iter != forests.end(); ++forest_iter)
    {
	bool forest_root_output = false;
	std::string const& forest_root = (*forest_iter).first;
	BuildForest& forest = *((*forest_iter).second);
	BuildTree_map& buildtrees = forest.getBuildTrees();
	for (BuildTree_map::iterator tree_iter = buildtrees.begin();
	     tree_iter != buildtrees.end(); ++tree_iter)
	{
	    bool tree_name_output = false;
	    std::string const& tree_name = (*tree_iter).first;
	    BuildTree& tree = *((*tree_iter).second);
	    PlatformData& platform_data = tree.getPlatformData();
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
		    if (! forest_root_output)
		    {
			std::cout << "forest " << forest_root << std::endl;
			forest_root_output = true;
		    }
		    if (! tree_name_output)
		    {
			std::cout << "  tree " << tree_name << std::endl;
			tree_name_output = true;
		    }
		    if (! platform_type_output)
		    {
			std::cout << "    platform type " << platform_type
				  << std::endl;
			platform_type_output = true;
		    }
		    std::string const& platform = (*p_iter).first;
		    bool selected = (*p_iter).second;
		    std::cout << "      platform " << platform
			      << "; built by default: "
			      << (selected ? "yes" : "no")
			      << std::endl;
		}
	    }
	}
    }
}

void
Abuild::dumpData(BuildForest_map& forests)
{
    this->logger.flushLog();

    std::ostream& o = std::cout;

    // Create a dependency graph of build forests based on backing
    // areas.  Output forests in order based on that graph.

    DependencyGraph g;
    computeBackingGraph(forests, g);
    std::list<std::string> const& all_forests = g.getSortedGraph();

    o << "<?xml version=\"1.0\"?>" << std::endl
      << "<abuild-data version=\"2\"";
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

    std::map<std::string, int> forest_numbers;
    int cur_forest = 0;
    for (std::list<std::string>::const_iterator forest_iter =
	     all_forests.begin();
	 forest_iter != all_forests.end(); ++forest_iter)
    {
	std::string const& forest_root = *forest_iter;
	forest_numbers[forest_root] = ++cur_forest;
	BuildForest& forest = *(forests[forest_root]);
	std::list<std::string> const& backing_areas = forest.getBackingAreas();
	std::set<std::string> const& deleted_trees = forest.getDeletedTrees();
	std::set<std::string> const& deleted_items = forest.getDeletedItems();

	o << " <forest" << std::endl
	  << "  id=\"f-" << cur_forest << "\"" << std::endl
	  << "  absolute-path=\"" << forest_root << "\"" << std::endl
	  << " >" << std::endl;

	for (std::list<std::string>::const_iterator biter =
		 backing_areas.begin();
	     biter != backing_areas.end(); ++biter)
	{
	    o << "  <backing-area forest=\"f-"
	      << forest_numbers[*biter]
	      << "\"/>" << std::endl;
	}
	if (! deleted_trees.empty())
	{
	    o << "  <deleted-trees>" << std::endl;
	    for (std::set<std::string>::const_iterator iter =
		     deleted_trees.begin();
		 iter != deleted_trees.end(); ++iter)
	    {
		o << "   <deleted-tree name=\"" + *iter + "\"/>" << std::endl;
	    }
	    o << "  </deleted-trees>" << std::endl;
	}
	if (! deleted_items.empty())
	{
	    o << "  <deleted-items>" << std::endl;
	    for (std::set<std::string>::const_iterator iter =
		     deleted_items.begin();
		 iter != deleted_items.end(); ++iter)
	    {
		o << "   <deleted-item name=\"" + *iter + "\"/>" << std::endl;
	    }
	    o << "  </deleted-items>" << std::endl;
	}

	std::list<std::string> const& sorted_trees =
	    forest.getSortedTreeNames();
	BuildTree_map& buildtrees = forest.getBuildTrees();
	for (std::list<std::string>::const_iterator iter = sorted_trees.begin();
	     iter != sorted_trees.end(); ++iter)
	{
	    std::string const& tree_name = *iter;
	    BuildTree& tree = *(buildtrees[tree_name]);
	    dumpBuildTree(tree, tree_name, forest, forest_numbers);
	}

	o << " </forest>" << std::endl;
    }

    o << "</abuild-data>" << std::endl;
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
Abuild::dumpBuildTree(BuildTree& tree, std::string const& tree_name,
		      BuildForest& forest,
		      std::map<std::string, int>& forest_numbers)
{
    std::ostream& o = std::cout;

    std::string const& root_path = tree.getRootPath();
    std::set<std::string> const& traits = tree.getSupportedTraits();
    std::list<std::string> const& plugins = tree.getPlugins();
    std::list<std::string> const& deps = tree.getTreeDeps();
    std::list<std::string> const& alldeps = tree.getExpandedTreeDeps();

    assert(forest_numbers.count(tree.getForestRoot()));

    o << "  <build-tree" << std::endl
      << "   name=\"" << tree_name << "\"" << std::endl
      << "   absolute-path=\"" << root_path << "\"" << std::endl
      << "   home-forest=\"f-"
      << forest_numbers[tree.getForestRoot()] << "\"" << std::endl
      << "   backing-depth=\"" << tree.getBackingDepth() << "\""
      << std::endl
      << "  >" << std::endl;
    dumpPlatformData(tree.getPlatformData(), "   ");
    if (! traits.empty())
    {
	o << "   <supported-traits>" << std::endl;
	for (std::set<std::string>::const_iterator trait_iter = traits.begin();
	     trait_iter != traits.end(); ++trait_iter)
	{
	    o << "    <supported-trait name=\"" << *trait_iter << "\"/>"
	      << std::endl;
	}
	o << "   </supported-traits>" << std::endl;
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
    if (! deps.empty())
    {
	o << "   <declared-tree-dependencies>" << std::endl;
	for (std::list<std::string>::const_iterator iter = deps.begin();
	     iter != deps.end(); ++iter)
	{
	    o << "    <tree-dependency name=\"" << *iter << "\"/>" << std::endl;
	}
	o << "   </declared-tree-dependencies>" << std::endl;
    }
    if (! alldeps.empty())
    {
	o << "   <expanded-tree-dependencies>" << std::endl;
	for (std::list<std::string>::const_iterator iter = alldeps.begin();
	     iter != alldeps.end(); ++iter)
	{
	    o << "    <tree-dependency name=\"" << *iter << "\"/>" << std::endl;
	}
	o << "   </expanded-tree-dependencies>" << std::endl;
    }

    std::list<std::string> const& sorted_items =
	forest.getSortedItemNames();
    BuildItem_map& builditems = forest.getBuildItems();

    for (std::list<std::string>::const_iterator item_iter =
	     sorted_items.begin();
	 item_iter != sorted_items.end(); ++item_iter)
    {
	std::string const& name = (*item_iter);
	BuildItem& item = *(builditems[name]);
	if (item.getTreeName() == tree_name)
	{
	    dumpBuildItem(item, name, forest_numbers);
	}
    }

    o << "  </build-tree>" << std::endl;
}

void
Abuild::dumpBuildItem(BuildItem& item, std::string const& name,
		      std::map<std::string, int>& forest_numbers)
{
    std::ostream& o = std::cout;

    std::string description = Util::XMLify(item.getDescription(), true);
    std::string visible_to = item.getVisibleTo();
    std::list<std::string> const& build_also =
	item.getBuildAlso();
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
	(! (build_also.empty() &&
	    declared_dependencies.empty() &&
	    expanded_dependencies.empty() &&
	    platform_types.empty() &&
	    buildable_platforms.empty() &&
	    supported_flags.empty() &&
	    traits.empty()));

    assert(forest_numbers.count(item.getForestRoot()));

    o << "   <build-item" << std::endl
      << "    name=\"" << name << "\"" << std::endl;
    if (! description.empty())
    {
	o << "    description=\"" << description << "\"" << std::endl;
    }
    o << "    home-forest=\"f-"
      << forest_numbers[item.getForestRoot()] << "\"" << std::endl
      << "    absolute-path=\"" << item.getAbsolutePath() << "\""
      << std::endl
      << "    backing-depth=\"" << item.getBackingDepth() << "\""
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
      << "    is-plugin=\""
      << (isPlugin(name) ? "1" : "0") << "\""
      << std::endl;
    if (any_subelements)
    {
	FlagData const& flag_data = item.getFlagData();
	o << "   >" << std::endl;
	if (! build_also.empty())
	{
	    o << "    <build-also-items>" << std::endl;
	    for (std::list<std::string>::const_iterator biter =
		     build_also.begin();
		 biter != build_also.end(); ++biter)
	    {
		o << "     <build-also name=\"" << *biter << "\"/>"
		  << std::endl;
	    }
	    o << "    </build-also-items>" << std::endl;
	}
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
	    for (TraitData::trait_data_t::const_iterator t_iter =
		     traits.begin();
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
Abuild::computeBuildset(BuildTree_map& buildtrees, BuildItem_map& builditems)
{
    // Generate the build set.
    std::string const& this_name = this->this_config->getName();
    BuildTree_ptr this_buildtree;
    if (! this->local_tree.empty())
    {
        if (buildtrees.count(this->local_tree) == 0)
        {
            fatal("INTERNAL ERROR: the current build tree is not known");
        }
        this_buildtree = buildtrees[this->local_tree];
    }
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

	if (this->local_tree.empty() &&
	    ((set_name == b_DEPTREES) || (set_name == b_LOCAL)))
	{
	    QTC::TC("abuild", "Abuild ERR bad tree-based build set");
	    error("build set " + set_name + " contains no items when"
		  " the current build item is not part of any tree");
	}
	else if (! this->buildset_named_items.empty())
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
	else if (set_name == b_DEPTREES)
        {
            QTC::TC("abuild", "Abuild buildset deptrees", cleaning ? 1 : 0);
	    std::set<std::string> trees;
	    std::list<std::string> const& deptrees =
		this_buildtree->getExpandedTreeDeps();
	    trees.insert(deptrees.begin(), deptrees.end());
	    trees.insert(this->local_tree);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isInTrees, _1, trees));
        }
        else if (set_name == b_LOCAL)
        {
            QTC::TC("abuild", "Abuild buildset local", cleaning ? 1 : 0);
	    populateBuildset(builditems,
			     boost::bind(&BuildItem::isInTree, _1,
					 this->local_tree));
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

    if (addBuildAlsoToBuildset(builditems))
    {
	QTC::TC("abuild", "Abuild non-trivial build-also");
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

    // We always add dependencies of any items initially in the build
    // set to the build set if we are building.  If we are cleaning,
    // we only do this when the --apply-targets-to-deps option is
    // specified.
    bool add_dependencies = (! cleaning) || this->apply_targets_to_deps;

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
    bool expanding = true;
    while (expanding)
    {
	expanding = false;
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
		if (this->buildset.count(*iter) == 0)
		{
		    QTC::TC("abuild", "Abuild non-trivial dep expansion");
		    expanding = true;
		    this->buildset[*iter] = builditems[*iter];
		}
	    }
	}

	if (! this->related_by_traits.empty())
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
		if (this->buildset.count(*iter) == 0)
		{
		    QTC::TC("abuild", "Abuild non-trivial trait expansion");
		    expanding = true;
		    this->buildset[*iter] = builditems[*iter];
		}
		if (! this->apply_targets_to_deps)
		{
		    this->explicit_target_items.insert(*iter);
		}
	    }
	}
	if (addBuildAlsoToBuildset(builditems))
	{
	    QTC::TC("abuild", "Abuild additional build-also expansion");
	    expanding = true;
	}
    }

    // Expand the build set to include all plugins of all items in the
    // buildset.
    if (add_dependencies)
    {
	std::set<std::string> to_add;
	for (BuildItem_map::iterator iter = this->buildset.begin();
	     iter != this->buildset.end(); ++iter)
	{
	    BuildItem& item = *((*iter).second);
	    std::list<std::string> const& plugins = item.getPlugins();
	    for (std::list<std::string>::const_iterator p_iter =
		     plugins.begin();
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
Abuild::addBuildAlsoToBuildset(BuildItem_map& builditems)
{
    // For every item in the build set, if that item specifies any
    // other items to build, add them as well.
    bool adding = true;
    bool added_any = false;
    while (adding)
    {
	adding = false;
	std::set<std::string> to_add;
	for (BuildItem_map::iterator iter = this->buildset.begin();
	     iter != this->buildset.end(); ++iter)
	{
	    BuildItem& item = *((*iter).second);
	    std::list<std::string> const& build_also = item.getBuildAlso();
	    for (std::list<std::string>::const_iterator biter =
		     build_also.begin();
		 biter != build_also.end(); ++biter)
	    {
		to_add.insert(*biter);
	    }
	}
	for (std::set<std::string>::iterator iter = to_add.begin();
	     iter != to_add.end(); ++iter)
	{
	    if ((this->buildset.count(*iter) == 0) &&
		builditems[*iter]->hasTraits(this->only_with_traits))
	    {
		adding = true;
		this->buildset[*iter] = builditems[*iter];
	    }
	}
	if (adding)
	{
	    added_any = true;
	}
    }
    return added_any;
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
    bool need_java = false;
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
	      case ItemConfig::b_groovy:
		need_java = true;
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

    if ((! need_java) && (! need_gmake) && some_native_items_skipped)
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

    if (! (this->special_target == s_NO_OP))
    {
	if (need_gmake)
	{
	    findGnuMakeInPath();
	}
	if (need_java)
	{
	    findJava();
	}
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
	if (isPlugin(item_name))
	{
	    verbose("loading interface for plugin " + item_name);
	    if (createPluginInterface(item_name, item))
	    {
		dumpInterface(PLUGIN_PLATFORM, item);
	    }
	    else
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
    FileLocation internal("[global-initialization]", 0, 0);
    assert(this->base_interface->assignVariable(
	       internal, "ABUILD_STDOUT_IS_TTY",
	       (this->stdout_is_tty ? "1" : "0"),
	       Interface::a_normal));

    // Build appropriate items in dependency order.
    DependencyRunner r(this->build_graph, this->max_workers,
		       boost::bind(&Abuild::itemBuilder, this,
				   _1, filter, false));
    r.setChangeCallback(boost::bind(&Abuild::stateChangeCallback,
				    this, _1, _2, filter), true);
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
    // graph between items, as we constructed and verified earlier,
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
    // reverse dependency needs to build on.  For example, suppose A
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
	    // the dependency's buildable platforms.  We have already
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
		// implicitly is buildable on all platforms.  We
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

    verbose("looking for gnu make");
    for (std::list<std::string>::iterator iter = candidates.begin();
	 iter != candidates.end(); ++iter)
    {
	std::string const& candidate = *iter;
	std::string version_string;
	try
	{
	    verbose("trying " + candidate);
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
		    verbose("using " + this->gmake + " for gnu make");
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
Abuild::findJava()
{
    boost::regex ant_home_re("(?s:.*\\[echo\\] (.*?)\r?\n.*)");
    boost::smatch match;

    verbose("locating abuild-java-support.jar");
    std::string java_support_jar;
    std::string abuild_dir = Util::dirname(this->program_fullpath);
    std::string base = Util::basename(abuild_dir);
    if (base.substr(0, OUTPUT_DIR_PREFIX.length()) == OUTPUT_DIR_PREFIX)
    {
	std::string candidate =
	    abuild_dir +
	    "/../java-support/abuild-java/dist/abuild-java-support.jar";
	if (Util::isFile(candidate))
	{
	    java_support_jar = Util::canonicalizePath(candidate);
	    verbose("found support jar in source directory: " +
		    java_support_jar);
	}
    }

    std::string java_libdir = this->abuild_top + "/lib";
    if (java_support_jar.empty())
    {
	java_support_jar = java_libdir + "/abuild-java-support.jar";
	verbose("using support jar from abuild's lib directory: " +
		java_support_jar);
    }

    if (! Util::isFile(java_support_jar))
    {
	fatal("unable to locate abuild-java-support.jar; run " + this->whoami +
	      " with --verbose for details");
    }

    verbose("trying to determine JAVA_HOME");
    std::string java_home;
    if (Util::getEnv("JAVA_HOME", &java_home))
    {
	verbose("using JAVA_HOME environment variable value: " + java_home);
    }
    else
    {
	verbose("attempting to guess JAVA_HOME from java in path");
	std::list<std::string> candidates;
	candidates = Util::findProgramInPath("java");
	if (candidates.empty())
	{
	    fatal("JAVA_HOME is not set, and there is no java"
		  " program in your path.");
	}
	std::string candidate = candidates.front();
	verbose("running " + candidate + " to find JAVA_HOME");
	try
	{
	    java_home =  Util::getProgramOutput(
		"\"" + candidate + "\" -cp " +
		java_support_jar + " org.abuild.javabuilder.PrintJavaHome");
	    QTC::TC("abuild", "Abuild infer JAVA_HOME");
	    verbose("inferred value for JAVA_HOME: " + java_home);
	}
	catch (QEXC::General)
	{
	    fatal("unable to determine JAVA_HOME; run " + this->whoami +
		  " with --verbose for details, or set JAVA_HOME explicitly");
	}
    }

    std::string java = java_home + "/bin/java";
    verbose("using java command " + java);

    std::list<std::string> java_libs; // jars and directories
    java_libs.push_back(java_support_jar);
    java_libs.push_back(java_libdir);
    java_libs.push_back(java_home + "/lib/tools.jar");

    verbose("trying to determine ANT_HOME");
    std::string ant_home;
    if (Util::getEnv("ANT_HOME", &ant_home))
    {
	verbose("using ANT_HOME environment variable value: " + ant_home);
    }
    else
    {
	verbose("attempting to guess ANT_HOME from ant in path");
	std::list<std::string> candidates;
	candidates = Util::findProgramInPath("ant");
	if (candidates.empty())
	{
	    fatal("ANT_HOME is not set, and there is no ant"
		  " program in your path.");
	}
	std::string candidate = candidates.front();
	verbose("running " + candidate + " to find ANT_HOME");
	try
	{
	    std::string output = Util::getProgramOutput(
		"\"" + candidate + "\" -q -f " +
		abuild_top + "/ant/find-ant-home.xml");
	    if (boost::regex_match(output, match, ant_home_re))
	    {
		ant_home = Util::canonicalizePath(match.str(1));
		QTC::TC("abuild", "Abuild infer ANT_HOME");
		verbose("inferred value for ANT_HOME: " + ant_home);
	    }
	    else
	    {
		verbose("ant output:\n" + output);
		fatal("unable to determine ANT_HOME output from ant output;"
		      " run " + this->whoami + " with --verbose for details,"
		      " or set ANT_HOME explicitly");
	    }
	}
	catch (QEXC::General)
	{
	    fatal("unable to determine ANT_HOME; run " + this->whoami +
		  " with --verbose for details, or set ANT_HOME explicitly");
	}
    }
    java_libs.push_back(ant_home + "/lib");

    this->java_builder.reset(
	new JavaBuilder(
	    this->error_handler,
	    boost::bind(&Abuild::verbose, this, _1),
	    this->abuild_top, java, java_libs, this->envp,
	    this->java_builder_args, this->defines));
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
Abuild::itemBuilder(std::string builder_string, item_filter_t filter,
		    bool is_dep_failure)
{
    // NOTE: This function must not return failure without adding the
    // failed builder_string to the failed_builds list.

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
    // just skip this item, then certain things the user expected to
    // get built would silently be ignored.
    assert(! build_item.hasShadowedReferences());

    bool no_op = (this->special_target == s_NO_OP);
    bool use_interfaces = (! (no_op || is_dep_failure));

    std::string const& abs_path = build_item.getAbsolutePath();
    InterfaceParser parser(item_name, item_platform, abs_path);
    parser.setSupportedFlags(build_item.getSupportedFlags());
    bool status = true;

    std::string output_dir = OUTPUT_DIR_PREFIX + item_platform;

    if (use_interfaces)
    {
	status = createItemInterface(
	    builder_string, item_name, item_platform, build_item, parser);
    }

    if (build_item.hasBuildFile())
    {
	if (status && use_interfaces)
	{
	    dumpInterface(item_platform, build_item, ".before-build");
	}

	// Build the item if we are supposed to.  Otherwise, assume it is
	// built.
	if (status && filter(item_name, item_platform))
	{
	    if (is_dep_failure)
	    {
		info(item_name +
		     " (" + output_dir +
		     ") will not be built because of failure of a dependency");
	    }
	    else
	    {
		status = buildItem(item_name, item_platform, build_item);
	    }
	}

	// If all is well, read any after-build files.  Disallow them
	// from declaring their own after-build files.
	if (status && use_interfaces)
	{
	    status = readAfterBuilds(
		item_platform, item_platform, build_item, parser);
	}
	if (status && use_interfaces)
	{
	    dumpInterface(item_platform, build_item, ".after-build");
	}
    }
    else
    {
	if (status && use_interfaces)
	{
	    dumpInterface(item_platform, build_item);
	}

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

    if (! status)
    {
        info(item_name + " (" + output_dir + "): build failed");
	this->failed_builds.push_back(builder_string);
    }

    return status;
}

void
Abuild::stateChangeCallback(std::string const& builder_string,
			    DependencyEvaluator::ItemState state,
			    item_filter_t filter)
{
    boost::smatch match;
    boost::regex builder_re(BUILDER_RE);
    assert(boost::regex_match(builder_string, match, builder_re));
    std::string item_name = match[1].str();
    std::string item_platform = match[2].str();
    std::string item_state = DependencyEvaluator::unparseState(state);
    monitorOutput("state-change " +
		  item_name + " " + item_platform + " " + item_state);
    if (state == DependencyEvaluator::i_depfailed)
    {
	itemBuilder(builder_string, filter, true);
    }
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
    FileLocation internal("[internal: " + builder_string + "]", 0, 0);
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
    InterfaceParser parser(plugin_name, PLUGIN_PLATFORM, dir);
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
Abuild::dumpInterface(std::string const& item_platform,
		      BuildItem& build_item,
		      std::string const& suffix)
{
    if (! this->dump_interfaces)
    {
	return;
    }

    if (! build_item.isWritable()) // XXX
    {
	QTC::TC("abuild", "Abuild dumpInterface ignoring read-only build item");
	return;
    }

    std::string output_dir = createOutputDirectory(item_platform, build_item);
    std::string dumpfile =
	output_dir + "/" + FILE_INTERFACE_DUMP + suffix + ".xml";
    std::ofstream of(dumpfile.c_str(),
		     std::ios_base::out |
		     std::ios_base::trunc |
		     std::ios_base::binary);
    if (! of.is_open())
    {
	throw QEXC::System("create " + dumpfile, errno);
    }

    Interface const& _interface = build_item.getInterface(item_platform);
    _interface.dump(of);

    of.close();
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
    if (! build_item.isWritable()) // XXX
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

    std::string abs_output_dir =
	createOutputDirectory(item_platform, build_item);
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

      case ItemConfig::b_groovy:
	result = invoke_groovy(item_name, item_platform, build_item,
			       abs_output_dir, backend_targets);
	break;

      default:
	fatal("unknown backend type for build item " + item_name);
	break;
    }

    return result;
}

std::string
Abuild::createOutputDirectory(std::string const& item_platform,
			      BuildItem& build_item)
{
    // If it doesn't already exist, create the output directory and
    // put an empty .abuild file in it.  The .abuild file tells abuild
    // that this is its directory.  cleanPath verifies its existence
    // before deleting a directory.  Knowledge of the name of the
    // output directory is duplicated in several places.

    std::string output_dir =
	build_item.getAbsolutePath() + "/" + OUTPUT_DIR_PREFIX + item_platform;
    std::string dot_abuild = output_dir + "/.abuild";
    if (! Util::isFile(dot_abuild))
    {
	Util::makeDirectory(output_dir);
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

    return output_dir;
}

std::list<std::string>
Abuild::getRulePaths(std::string const& item_name,
		     BuildItem& build_item,
		     std::string const& dir,
		     bool relative)
{

    // Generate a list of paths from which we should attempt to load
    // rules.  For every plugin and accessible dependency (in that
    // order), try the target type directory and the "all" directory.

    std::list<std::string> candidate_paths;
    std::list<std::string> const& plugins = build_item.getPlugins();
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	candidate_paths.push_back(this->buildset[*iter]->getAbsolutePath());
    }
    std::list<std::string> const& alldeps =
	build_item.getExpandedDependencies();
    for (std::list<std::string>::const_iterator iter = alldeps.begin();
	 iter != alldeps.end(); ++iter)
    {
	if (accessibleFrom(this->buildset, item_name, *iter))
	{
	    candidate_paths.push_back(this->buildset[*iter]->getAbsolutePath());
	}
    }
    candidate_paths.push_back(build_item.getAbsolutePath());

    std::list<std::string> result;
    std::string const& target_type =
	TargetType::getName(build_item.getTargetType());

    // Internal directories are absolute unconditionally.
    result.push_back(this->abuild_top + "/rules/" + target_type);
    result.push_back(this->abuild_top + "/rules/all");

    std::string dirs[2];

    for (std::list<std::string>::iterator iter = candidate_paths.begin();
	 iter != candidate_paths.end(); ++iter)
    {
	dirs[0] = *iter + "/rules/" + target_type;
	dirs[1] = *iter + "/rules/all";
	for (unsigned int i = 0; i < 2; ++i)
	{
	    if (Util::isDirectory(dirs[i]))
	    {
		if (relative)
		{
		    dirs[i] = Util::absToRel(dirs[i], dir);
		}
		result.push_back(dirs[i]);
	    }
	}
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

    // Generate a list of paths from which we should attempt to load
    // rules.
    std::list<std::string> rule_paths =
	getRulePaths(item_name, build_item, dir, true);
    mk << "ABUILD_RULE_PATHS :=";
    for (std::list<std::string>::iterator iter = rule_paths.begin();
	 iter != rule_paths.end(); ++iter)
    {
	mk << " " << *iter;
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
    for (std::map<std::string, std::string>::iterator iter =
	     this->defines.begin();
	 iter != this->defines.end(); ++iter)
    {
	make_argv.push_back((*iter).first + "=" + (*iter).second);
    }
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
	plugin_paths.push_back(this->buildset[*iter]->getAbsolutePath());
    }
    dyn << "abuild.plugins="
	<< Util::join(",", plugin_paths)
	<< "\n";

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
	    dyn << Util::join(Util::pathSeparator(), info.value);
	}
	else
	{
	    dyn << Util::join(" ", info.value);
	}
	dyn << std::endl;
    }

    dyn.close();

    std::string build_file;
    if (build_item.hasAntBuild())
    {
	build_file = build_item.getAbsolutePath() + "/" +
	    build_item.getBuildFile();
    }
    else
    {
	build_file = this->abuild_top + "/ant/abuild.xml";
    }

    return invokeJavaBuilder("ant", build_file, dir, targets);
}

bool
Abuild::invoke_groovy(std::string const& item_name,
		      std::string const& item_platform,
		      BuildItem& build_item,
		      std::string const& dir,
		      std::list<std::string> const& targets)
{
    // We're not doing anything to protect against groovy syntax in
    // strings.  Then again, the other backends don't protect their
    // output either.

    // Create FILE_DYNAMIC_GROOVY
    std::string dynamic_file = dir + "/" + FILE_DYNAMIC_GROOVY;
    std::ofstream dyn(dynamic_file.c_str(),
		      std::ios_base::out |
		      std::ios_base::trunc);
    if (! dyn.is_open())
    {
        throw QEXC::System("create " + dynamic_file, errno);
    }

    // We need to create our own script object explicitly so that
    // groovy won't try to create a java class called ".ab-dynamic",
    // which is not a legal class name.
    dyn << "class ABDynamic extends Script { Object run() {"
	<< std::endl << std::endl;

    // Supply information that is private to this build and should not
    // be accessible in and from the item's interface.  Since this
    // backend is a full programming language, we can provide this
    // information separately from the interface.

    // Supply paths for this item and all its dependencies and
    // plugins.
    std::set<std::string> const& references = build_item.getReferences();
    for (std::set<std::string>::const_iterator iter = references.begin();
         iter != references.end(); ++iter)
    {
        dyn << "abuild.itemPaths['" << *iter << "'] = '"
	    << this->buildset[*iter]->getAbsolutePath()
	    << "'" << std::endl;
    }

    // Generate a property for the top of abuild
    dyn << "abuild.abuildTop = '"
	<< this->abuild_top << "'" << std::endl;

    // Generate a list of plugin directories.  We don't provide the
    // names of plugins because people shouldn't be accessing them,
    // and this saves us from writing backend code to resolve them
    // anyway.
    std::list<std::string> const& plugins = build_item.getPlugins();
    std::list<std::string> plugin_paths;
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	plugin_paths.push_back(this->buildset[*iter]->getAbsolutePath());
    }
    dyn << "abuild.pluginPaths = [";
    if (! plugin_paths.empty())
    {
	dyn << "'" << Util::join("', '", plugin_paths) << "'";
    }
    dyn << "]\n";

    // Generate a list of paths from which we should attempt to load
    // rules.
    std::list<std::string> rule_paths =
	getRulePaths(item_name, build_item, dir, false);
    dyn << "abuild.rulePaths = ['" << Util::join("', '", rule_paths) << "']\n";

    // Provide data from the item's interface object.  We use
    // "interfaceVars" because "interface" is a reserved word in
    // Groovy.
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
        dyn << "abuild.interfaceVars['" << name << "'] = ";
	// Scalars should have at most one value, so we can treat
	// scalars and lists together.
	if (info.list_type != Interface::l_scalar)
	{
	    dyn << "[";
	}
	bool first = true;
	for (std::deque<std::string>::const_iterator viter = info.value.begin();
	     viter != info.value.end(); ++viter)
	{
	    if (first)
	    {
		first = false;
	    }
	    else
	    {
		dyn << ", ";
	    }
	    dyn << "'" << *viter << "'";
	}
	if (info.list_type != Interface::l_scalar)
	{
	    dyn << "]";
	}
	dyn << std::endl;
    }

    dyn << std::endl << "}}" << std::endl;

    dyn.close();

    return invokeJavaBuilder("groovy", "", dir, targets);
}

bool
Abuild::invokeJavaBuilder(std::string const& backend,
			  std::string const& build_file,
			  std::string const& dir,
			  std::list<std::string> const& targets)
{
    flushLogIfSingleThreaded();

    if (this->verbose_mode)
    {
	// Put this in the if statement to avoid the needless cal to
	// join if not in verbose mode.
	verbose("running JavaBuilder backend " + backend);
	if (! build_file.empty())
	{
	    verbose("  build file: " + build_file);
	}
	verbose("  directory: " + dir);
	verbose("  targets: " + Util::join(" ", targets));
    }

    return this->java_builder->invoke(
	backend, build_file, dir, targets);
}

bool
Abuild::invokeBackend(std::string const& progname,
		      std::vector<std::string> const& args,
		      std::map<std::string, std::string> const& environment,
		      char* old_env[],
		      std::string const& dir)
{
    flushLogIfSingleThreaded();

    if (this->verbose_mode)
    {
	// Put this in the if statement to avoid the needless cal to
	// join if not in verbose mode.
	verbose("running " + Util::join(" ", args));
    }

    return Util::runProgram(progname, args, environment, old_env, dir);
}

void
Abuild::flushLogIfSingleThreaded()
{
    if (this->max_workers == 1)
    {
        // If we have only one thread, then only one thread is using
        // the logger.  Flush the logger before we run the backend to
        // prevent its output from being interleaved with our own.
        this->logger.flushLog();
    }
}

void
Abuild::cleanBuildset()
{
    for (BuildItem_map::iterator iter = this->buildset.begin();
	 iter != this->buildset.end(); ++iter)
    {
	std::string const& item_name = (*iter).first;
	BuildItem& item = *((*iter).second);
	if (item.isWritable())	// XXX
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

    boost::smatch match;

    std::vector<std::string> entries = Util::getDirEntries(dir);
    // Sort for consistent output in test suite.
    std::sort(entries.begin(), entries.end());
    for (std::vector<std::string>::iterator iter = entries.begin();
	 iter != entries.end(); ++iter)
    {
	std::string const& entry = *iter;
	std::string fullpath = dir + "/" + entry;
	if ((entry.substr(0, OUTPUT_DIR_PREFIX.length()) ==
	     OUTPUT_DIR_PREFIX) &&
	    (Util::isFile(fullpath + "/.abuild")))
	{
	    bool remove = false;
	    if (this->clean_platforms.empty())
	    {
		remove = true;
	    }
	    else
	    {
		std::string platform = entry.substr(OUTPUT_DIR_PREFIX.length());
		// Remove this platform if it matches one of the clean
		// platforms.
		for (std::set<std::string>::iterator iter =
			 this->clean_platforms.begin();
		     iter != this->clean_platforms.end(); ++iter)
		{
		    boost::regex re(*iter);
		    if (boost::regex_match(platform, match, re))
		    {
			remove = true;
			break;
		    }
		}
		if (remove)
		{
		    info("  removing " + entry);
		}
		else
		{
		    info("  not removing " + entry);
		}
	    }
	    if (remove)
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
    h("Usage: " + whoami + " [options] [defines] [targets]");
    h("");
    h("This help message provides a brief synopsis of supported arguments.");
    h("Please see abuild's documentation for additional details.");
    h("");
    h("Options and targets may appear in any order.  Any argument that starts");
    h("with \"-\" is treated as an option.");
    h("");
    h("Any option not starting with - that contains an = is treated as a variable");
    h("definition, with the variable name being everything prior to the first =.");
    h("These are passed as variables to make, properties to ant, keys in");
    h("abuild.defines for groovy.");
    h("");
    h("If no targets are specified, the \"all\" target is built.");
    h("");
    h("The following options are supported:");
    h("");
    h("  -H | --help       print help message and exit");
    h("  -V | --version    print abuild's version number and exit");
    h("");
    h("  --apply-targets-to-deps   apply explicit targets to dependencies as well;");
    h("                    when cleaning with a clean set, expand to include");
    h("                    dependencies");
    h("  --build=set |     specify a build set");
    h("    -b set");
    h("  -C start-dir      change directories to start-dir before running");
    h("  --clean=set |     specify a clean set");
    h("    -c set");
    h("  --clean-platforms pattern   when cleaning, only remove platforms that");
    h("                    match the given shell-style filename pattern");
    h("  --compat-level=x.y       disable backward compatible for constructs that");
    h("                    were deprecated at or before version x.y");
    h("  --deprecation-is-error   treat deprecation warnings as errors");
    h("  --dump-build-graph  dump abuild's internal build graph");
    h("  --dump-data       dump abuild's data to stdout and build no targets");
    h("  --dump-interfaces   write details about items' interfaces to files");
    h("                    in the output directory");
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
    h("  --make            pass all remaining arguments to make");
    h("  --monitored       run in monitored mode");
    h("  -n                pass no-op flag to backend");
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
    h("  --upgrade-trees   run special mode to upgrade build trees");
    h("  --verbose         generate more detailed output");
    h("  --with-deps | -d  short-hand for --build=current");
    h("");
    h("Build/Clean sets:");
    h("");
    h("  all               all buildable/cleanable items in writable build trees");
    h("  deptrees          all items in the local tree and its full tree-deps chain");
    h("  local             all items in the local build tree");
    h("  desc              all items at or below the current directory");
    h("  descending        alias for desc");
    h("  down              alias for desc");
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
Abuild::deprecate(std::string const& version, std::string const& msg)
{
    deprecate(version, FileLocation(), msg);
}

void
Abuild::deprecate(std::string const& version,
		  FileLocation const& location, std::string const& msg)
{
    this->error_handler.deprecate(version, location, msg);
}

void
Abuild::suggestUpgrade()
{
    assert(this->compat_level.allow_1_0());
    if (! this->suggest_upgrade)
    {
	return;
    }

    this->logger.logInfo("");
    this->logger.logInfo("******************** " + this->whoami +
			 " ********************");
    this->logger.logInfo("WARNING: Build items/trees with"
			 " deprecated 1.0 features were found.");
    this->logger.logInfo("Consider upgrading your build trees,"
			 " which you can do automatically by");
    this->logger.logInfo("running");
    this->logger.logInfo("");
    this->logger.logInfo("  " + this->whoami + " --upgrade-trees");
    this->logger.logInfo("");
    this->logger.logInfo("from the appropriate location.");
    this->logger.logInfo("");
    this->logger.logInfo("For details, please see \"Upgrading Build"
			 " Trees from 1.0 to 1.1\" in");
    this->logger.logInfo("the user's manual");
    this->logger.logInfo("******************** " + this->whoami +
			 " ********************");
    this->logger.logInfo("");
}

void
Abuild::fatal(std::string const& msg)
{
    // General exception caught in main().
    throw QEXC::General(this->whoami + ": ERROR: " + msg);
}
