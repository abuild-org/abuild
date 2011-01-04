// Initialization code including static variables, command-line
// parsing, etc.  Also includes constructor/destructor and top-level
// run function.

#include <Abuild.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <ProcessHandler.hh>
#include <Logger.hh>
#include <OptionParser.hh>
#include <InterfaceParser.hh>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

std::string const Abuild::ABUILD_VERSION = "1.1.3+";
std::string const Abuild::OUTPUT_DIR_PREFIX = "abuild-";
std::string const Abuild::FILE_DYNAMIC_MK = ".ab-dynamic.mk";
std::string const Abuild::FILE_DYNAMIC_ANT = ".ab-dynamic-ant.properties";
std::string const Abuild::FILE_DYNAMIC_GROOVY = ".ab-dynamic.groovy";
std::string const Abuild::FILE_INTERFACE_DUMP = ".ab-interface-dump";
std::string const Abuild::b_ALL = "all";
std::string const Abuild::b_DEPTREES = "deptrees";
std::string const Abuild::b_DESCDEPTREES = "descdeptrees";
std::string const Abuild::b_LOCAL = "local";
std::string const Abuild::b_DESC = "desc";
std::string const Abuild::b_DEPS = "deps";
std::string const Abuild::b_CURRENT = "current";
std::set<std::string> Abuild::valid_buildsets;
std::map<std::string, std::string> Abuild::buildset_aliases;
std::string const Abuild::s_CLEAN = "clean";
std::string const Abuild::s_NO_OP = "no-op";
std::string const Abuild::h_HELP = "help";
std::string const Abuild::h_RULES = "rules";
std::string const Abuild::hr_HELP = "help";
std::string const Abuild::hr_LIST = "list";
std::string const Abuild::hr_RULE = "rule:";
std::string const Abuild::hr_TOOLCHAIN = "toolchain:";
// PLUGIN_PLATFORM can't match a real platform name.
std::string const Abuild::PLUGIN_PLATFORM = "plugin";
std::string const Abuild::FILE_PLUGIN_INTERFACE = "plugin.interface";
std::set<std::string> Abuild::special_targets;
std::list<std::string> Abuild::default_targets;

// Initialize this after all other status
bool Abuild::statics_initialized = Abuild::initializeStatics();

Abuild::Abuild(int argc, char* argv[]) :
    argc(argc),
    argv(argv),
    whoami(Util::removeExe(Util::basename(argv[0]))),
    stdout_is_tty(Util::stdoutIsTty()),
    max_workers(1),
    make_njobs(1),
    output_mode(om_unset),
    capture_output(false),
    use_job_prefix(false),
    test_java_builder_bad_java(false),
    keep_going(false),
    no_dep_failures(false),
    explicit_buildset(false),
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
    with_rdeps(false),
    repeat_expansion(false),
    compat_level(CompatLevel::cl_1_1),
    default_writable(true),
    local_build(false),
    error_handler(Logger::NO_JOB, whoami),
    this_config(0),
#ifdef _WIN32
    have_perl(false),
#endif
    last_assigned_tree_number(0),
    suggest_upgrade(false),
    logger(*(Logger::getInstance())),
    process_handler(ProcessHandler::getInstance())
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
    valid_buildsets.insert(b_DESCDEPTREES);
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
    if (this->argc > 1)
    {
	boost::function<void(std::string const&)> l =
	    boost::bind(&Logger::logInfo, &(this->logger), _1, Logger::NO_JOB);
	std::string last_arg = argv[this->argc - 1];
	if ((last_arg == "-V") || (last_arg == "--version"))
	{
	    l(this->whoami + " version " + ABUILD_VERSION);
	    l("");
	    l("Copyright (c) 2007-2011 Jay Berkenbilt, Argon ST");
	    l("This software may be distributed under the terms of version 2 of");
	    l("the Artistic License which may be found in the source and binary");
	    l("distributions.  It is provided \"as is\" without express or");
	    l("implied warranty.");
	    return true;
	}
	else if ((last_arg == "-H") || (last_arg == "--help"))
	{
	    l("");
	    l("Help is available on a variety of topics.");
	    l("");
	    l("For a usage summary, run " + this->whoami + " --help usage");
	    l("");
	    l("For a list of topics and information about the help system, run");
	    l(this->whoami + " --help help");
	    l("");
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

    if (generalHelp())
    {
	return true;
    }

    InterfaceParser::setParameters(this->defines);
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
	if (! this->dump_build_graph)
	{
	    suggestUpgrade();
	}
    }

    boost::shared_ptr<boost::posix_time::time_duration> build_time;
    bool okay = true;
    if (this->special_target == s_CLEAN)
    {
        QTC::TC("abuild", "Abuild-init clean");
	if (! this->this_platform.empty())
	{
	    cleanOutputDir();
	}
	else
	{
	    cleanPath("", this->current_directory);
	}
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
	// Sort for consistent output in test suite.
	std::sort(this->failed_builds.begin(), this->failed_builds.end());
	assert(! this->failed_builds.empty());
	for (std::vector<std::string>::iterator iter =
		 this->failed_builds.begin();
	     iter != this->failed_builds.end(); ++iter)
	{
	    std::string item_name;
	    std::string item_platform;
	    parseBuildGraphNode(*iter, item_name, item_platform);
	    error("build failure: " + item_name + " on platform " +
		  item_platform);
	}
    }

    if ((! this->dump_build_graph) && build_time.get())
    {
	std::ostringstream time;
	time << "total build time:";
	long h = build_time->hours();
	long m = build_time->minutes();
	long s = build_time->seconds();
	if (h)
	{
	    time << " " << h << "h";
	}
	if (h || m)
	{
	    time << " " << m << "m";
	}
	time << " " << s << "s";
	info(time.str());
    }

    return okay;
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
		    QTC::TC("abuild", "Abuild-init ERR addPlatformType error");
		    error(location, e.what());
		}
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-init ERR bad platform type specification");
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
	// Pass native os/cpu/toolset data to list_platforms.
	cmd += " --native-data " + this->native_os + " " +
	    this->native_cpu + " " + this->native_toolset;
	FileLocation location(list_platforms, 0, 0);
	std::string platform_data_output;
	try
	{
	    platform_data_output = Util::getProgramOutput(cmd);
	}
	catch (QEXC::General& e)
	{
#ifndef _WIN32
	    // On non-Windows systems, Windows newlines in a perl
	    // script cause problems.  This situation has caused
	    // confusion a number of times for people who do checkouts
	    // on Windows and access the results on UNIX.  We only
	    // test for this on non-Windows systems to avoid a
	    // misleading error message on a Windows system.
	    std::list<std::string> lines =
		Util::readLinesFromFile(list_platforms, false);
	    if ((! lines.empty()) &&
		((*(lines.begin())).find('\r') != std::string::npos))
	    {
		// This coverage case is referenced in
		// abuild-misc.test; update call to fake_qtc if it
		// changes.
		QTC::TC("abuild", "Abuild-init windows nl in list_platforms");
		error(location, "Windows-style line endings found;"
		      " please ensure this file uses UNIX line endings");
	    }
#endif
	    throw e;
	}
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
		    QTC::TC("abuild", "Abuild-init add platform",
			    lowpri ? 0 : 1);
		}
		catch (QEXC::General& e)
		{
		    QTC::TC("abuild", "Abuild-init ERR addPlatform error");
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
		    QTC::TC("abuild", "Abuild-init add native compiler",
			    lowpri ? 0 : 1);
		}
		catch (QEXC::General& e)
		{
		    QTC::TC("abuild", "Abuild-init ERR add compiler error");
		    error(location, e.what());
		}
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-init ERR bad platform specification");
		error(location, "invalid syntax in list_platforms output");
	    }
	}
    }

    try
    {
	platform_data.check(this->platform_selectors,
			    this->unused_platform_selectors);
    }
    catch (QEXC::General& e)
    {
	fatal("errors detected loading platforms:\n" + e.unparse());
    }
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

// This helper function works around a problem with boost 1.44.0 and
// VC10 (Visual C++ 2010) that prevents boost::bind from working on
// std::list<std::string>::push_back.
static void
list_string_push_back(std::list<std::string>& l, std::string const& s)
{
    l.push_back(s);
}

void
Abuild::parseArgv()
{
    boost::smatch match;

    std::list<std::string> platform_selector_strings;
    std::string platform_selector_env;
    if (Util::getEnv("ABUILD_PLATFORM_SELECTORS", &platform_selector_env))
    {
	QTC::TC("abuild", "Abuild-init ABUILD_PLATFORM_SELECTORS");
	platform_selector_strings = Util::splitBySpace(platform_selector_env);
    }
    std::string compat_level_version;
    if (! Util::getEnv("ABUILD_COMPAT_LEVEL", &compat_level_version))
    {
	compat_level_version = "1.1";
    }

    // Increase max PermGen space.  This option is understood by Sun's
    // JVM and appears to be (harmlessly) ignored by others.
    this->jvm_xargs.push_back("-XX:MaxPermSize=200m");

    // for backward compatibility
    std::list<std::string> ant_args;
    boost::regex ant_define_re("-D([^-][^=]*)=(.*)");

    std::list<std::string> clean_platform_strings;

    OptionParser op(boost::bind(&Abuild::usage, this, _1),
		    boost::bind(&Abuild::argPositional, this, _1));

    op.registerListArg(
	"help", "-.*", false,
	boost::bind(&Abuild::argHelp, this, _1));
    op.registerSynonym("H", "help");
    op.registerStringArg(
	"C",
	boost::bind(&Abuild::argSetString, this,
		    boost::ref(this->start_dir), _1));
    op.registerNoArg(
	"find-conf",
	boost::bind(&Abuild::argFindConf, this));
    op.registerNumericArg(
	"jobs",
	boost::bind(&Abuild::argSetJobs, this, _1));
    op.registerSynonym("j", "jobs");
    op.registerOptionalNumericArg(
	"make-jobs", ((unsigned int) -1),
	boost::bind(&Abuild::argSetMakeJobs, this, _1));
    op.registerNoArg(
	"keep-going",
	boost::bind(&Abuild::argSetKeepGoing, this));
    op.registerSynonym("k", "keep-going");
    op.registerNoArg(
	"no-dep-failures",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->no_dep_failures), true));
    op.registerNoArg(
	"raw-output",
	boost::bind(&Abuild::argSetOutputMode, this, om_raw));
    op.registerNoArg(
	"interleaved-output",
	boost::bind(&Abuild::argSetOutputMode, this, om_interleaved));
    op.registerNoArg(
	"buffered-output",
	boost::bind(&Abuild::argSetOutputMode, this, om_buffered));
    op.registerStringArg(
	"output-prefix",
	boost::bind(&Abuild::argSetString, this,
		    boost::ref(this->output_prefix), _1));
    op.registerStringArg(
	"error-prefix",
	boost::bind(&Abuild::argSetString, this,
		    boost::ref(this->error_prefix), _1));
    op.registerNoArg(
	"n",
	boost::bind(&Abuild::argSetNoOp, this));
    op.registerNoArg(
	"emacs",
	boost::bind(&Abuild::argSetEmacs, this));
    op.registerSynonym("e", "emacs");
    op.registerListArg(
	"make", "-?-ant", false,
	boost::bind(&Abuild::argSetBackendArgs, this, _1,
		    boost::ref(this->make_args)));
    // Do not document --ant.
    op.registerListArg(
	"ant", "-?-make", false,
	boost::bind(&Abuild::argSetBackendArgs, this, _1,
		    boost::ref(ant_args)));
    op.registerListArg(
	"jvm-append-args", "-?-end-jvm-args", true,
	boost::bind(&Abuild::argSetJVMXargs, this, _1, false));
    op.registerListArg(
	"jvm-replace-args", "-?-end-jvm-args", true,
	boost::bind(&Abuild::argSetJVMXargs, this, _1, true));
    op.registerStringArg(
	"platform-selector",
	boost::bind(list_string_push_back,
		    boost::ref(platform_selector_strings),
		    _1));
    op.registerSynonym("p", "platform-selector");
    op.registerStringArg(
	"clean-platforms",
	boost::bind(list_string_push_back,
		    boost::ref(clean_platform_strings),
		    _1));
    op.registerNoArg(
	"full-integrity",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->full_integrity), true));
    op.registerNoArg(
	"deprecation-is-error",
	boost::bind(&Abuild::argSetDeprecationIsError, this));
    op.registerNoArg(
	"verbose",
	boost::bind(&Abuild::argSetVerbose, this));
    op.registerNoArg(
	"silent",
	boost::bind(&Abuild::argSetSilent, this));
    op.registerNoArg(
	"monitored",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->monitored), true));
    op.registerNoArg(
	"dump-interfaces",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->dump_interfaces), true));
    op.registerRegexArg(
	"compat-level", "\\d+\\.\\d+",
	boost::bind(&Abuild::argSetCompatLevel, this,
		    boost::ref(compat_level_version), _1));
    op.registerStringArg(
	"ro-path",
	boost::bind(&Abuild::argInsertInSet, this,
		    boost::ref(this->ro_paths), _1));
    op.registerStringArg(
	"rw-path",
	boost::bind(&Abuild::argInsertInSet, this,
		    boost::ref(this->rw_paths), _1));
    op.registerNoArg(
	"no-deps",
	boost::bind(&Abuild::argSetBuildSet, this, ""));
    op.registerNoArg(
	"with-deps",
	boost::bind(&Abuild::argSetBuildSet, this, b_CURRENT));
    op.registerSynonym("d", "with-deps");
    op.registerStringArg(
	"build",
	boost::bind(&Abuild::argSetBuildSet, this, _1));
    op.registerSynonym("b", "build");
    op.registerStringArg(
	"clean",
	boost::bind(&Abuild::argSetCleanSet, this, _1));
    op.registerSynonym("c", "clean");
    op.registerStringArg(
	"only-with-traits",
	boost::bind(&Abuild::argSetStringSplit, this,
		    boost::ref(this->only_with_traits), _1));
    op.registerStringArg(
	"related-by-traits",
	boost::bind(&Abuild::argSetStringSplit, this,
		    boost::ref(this->related_by_traits), _1));
    op.registerNoArg(
	"with-rdeps",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->with_rdeps), true));
    op.registerNoArg(
	"apply-targets-to-deps",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->apply_targets_to_deps), true));
    op.registerNoArg(
	"repeat-expansion",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->repeat_expansion), true));
    op.registerNoArg(
	"dump-build-graph",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->dump_build_graph), true));
    op.registerNoArg(
	"list-traits",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->list_traits), true));
    op.registerNoArg(
	"list-platforms",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->list_platforms), true));
    op.registerNoArg(
	"dump-data",
	boost::bind(&Abuild::argSetBool, this,
		    boost::ref(this->dump_data), true));
    op.registerStringArg(
	"find",
	boost::bind(&Abuild::argSetString, this,
		    boost::ref(this->to_find), _1));

    op.parseOptions(argv + 1);

    // If an alternative start directory was specified, chdir to it
    // before we ever access any of the user's files.
    if (! this->start_dir.empty())
    {
	// Throws an exception on failure
	Util::setCurrentDirectory(this->start_dir);
    }
    this->current_directory = Util::getCurrentDirectory();

    // Must be called before full validation of arguments since
    // certain things are valid or not valid based on whether we're in
    // an output directory
    getThisPlatform();

    // Compatability level
    if (compat_level_version == "1.0")
    {
	this->compat_level.setLevel(CompatLevel::cl_1_0);
	this->make_args.push_back("ABUILD_SUPPORT_1_0=1");
    }
    else if (compat_level_version == "1.1")
    {
	this->compat_level.setLevel(CompatLevel::cl_1_1);
	this->java_builder_args.push_back("-cl1_1");
	this->make_args.push_back("ABUILD_SUPPORT_1_0=0");
    }
    else
    {
	QTC::TC("abuild", "Abuild-init ERR usage compatibility level");
	usage("invalid compatibility level " + compat_level_version);
    }

    if (! ant_args.empty())
    {
	if (this->compat_level.allow_1_0())
	{
	    deprecate("1.1",
		      "the --ant option is deprecated;"
		      " use prop=value instead of --ant -Dprop=value");
	    for (std::list<std::string>::iterator iter = ant_args.begin();
		 iter != ant_args.end(); ++iter)
	    {
		if (boost::regex_match(*iter, match, ant_define_re))
		{
		    this->defines[match.str(1)] = match.str(2);
		}
		else
		{
		    notice("WARNING: ant options other than -Dprop=value"
			   " are ignored");
		}
	    }
	}
	else
	{
	    // Pretend we just didn't recognize the option at all
	    QTC::TC("abuild", "Abuild-init ERR ant unknown");
	    usage("unknown option \"ant\"");
	}
    }

    // Job output/error handling
    if (this->output_mode == om_unset)
    {
	// Capture output if a non-empty error or output prefix was
	// specified.  Otherwise, capture build output for
	// multithreaded builds unless raw output was specifically
	// requested.
	if (! (this->output_prefix.empty() && this->error_prefix.empty()))
	{
	    QTC::TC("abuild", "Abuild-init set interleaved from prefix");
	    this->output_mode = om_interleaved;
	}
	else
	{
	    this->output_mode =
		(this->max_workers == 1 ? om_raw : om_interleaved);
	}
    }
    if (this->output_mode != om_raw)
    {
	assert((this->output_mode == om_buffered) ||
	       (this->output_mode == om_interleaved));
	this->stdout_is_tty = false;
	this->logger.setPrefixes(this->output_prefix, this->error_prefix);
	this->java_builder_args.push_back("-co");
	this->capture_output = true;
	this->use_job_prefix =
	    ((this->output_mode == om_interleaved) && (this->max_workers > 1));
    }
    QTC::TC("abuild", "Abuild-init output mode",
	    (this->output_mode == om_raw ? 0 :
	     this->output_mode == om_interleaved ? 1 :
	     this->output_mode == om_buffered ? 2 :
	     999));		// can't happen
    QTC::TC("abuild", "Abuild-init output prefix",
	    this->output_prefix.empty() ? 0 : 1);
    QTC::TC("abuild", "Abuild-init error prefix",
	    this->output_prefix.empty() ? 0 : 1);

    // called after changing directories
    checkRoRwPaths();

    // Special case: if "clean" is specified with a build set, pretend
    // a clean set was specified instead.  Also, let the user get away
    // with stuff like abuild --clean=all clean.
    if (special_target == s_CLEAN)
    {
	if (! this->buildset_name.empty())
	{
	    this->cleanset_name = this->buildset_name;
	    this->buildset_name.clear();
	}
	if (! this->cleanset_name.empty())
	{
	    special_target.clear();
	}
    }

    // Once we've ensured that we're not doing anything that is not
    // allowed with a build set, enable build set "current" by
    // default.
    if ((! (explicit_buildset || (special_target == s_CLEAN))) &&
	this->this_platform.empty())
    {
	QTC::TC("abuild", "Abuild-init build current by default");
	this->buildset_name = b_CURRENT;
    }

    bool have_buildset = (! (this->buildset_name.empty() &&
			     this->cleanset_name.empty()));
    if (! have_buildset)
    {
	this->local_build = true;
	if (! (this->only_with_traits.empty() &&
	       this->related_by_traits.empty()))
	{
	    QTC::TC("abuild", "Abuild-init ERR usage traits without set");
	    usage("--only-with-traits and --related-by-traits may"
		  " not be used without a build set");
	}
	if (this->with_rdeps)
	{
	    QTC::TC("abuild", "Abuild-init ERR usage rdeps without set");
	    usage("--with-rdeps may not be used without a build set");
	}
    }

    // Make sure we're not trying to clean and build at the same time.
    if (! this->cleanset_name.empty())
    {
	if ((! this->targets.empty()) ||
	    (! this->special_target.empty()))
	{
	    QTC::TC("abuild", "Abuild-init ERR usage --clean with targets");
	    usage("--clean may not be combined with other targets");
	}
    }
    else if (! this->special_target.empty())
    {
	if (! this->targets.empty())
	{
	    QTC::TC("abuild", "Abuild-init ERR usage special with targets");
	    usage("\"" + this->special_target + "\" may not be combined"
		  " with any other targets");
	}
    }

    if (! this->this_platform.empty())
    {
	if (this->special_target == s_CLEAN)
	{
	    // Special case: allow clean to be run from an output
	    // directory.
	    QTC::TC("abuild", "Abuild-init clean from output directory");
	}
	else if (! (this->special_target.empty() &&
		    this->buildset_name.empty() &&
		    this->cleanset_name.empty()))
	{
	    QTC::TC("abuild", "Abuild-init ERR usage special or set in output");
	    usage("special targets, build sets, and clean sets may not be"
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
	    std::string platform_type = p.getPlatformType();
	    this->platform_selectors[platform_type] = p;
	    this->unused_platform_selectors[platform_type] = *iter;
	}
	else
	{
	    QTC::TC("abuild", "Abuild-init ERR invalid platform selector");
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
	    QTC::TC("abuild", "Abuild-init ERR bad clean platform");
	    error("invalid clean platform selector " + *iter + ": " + e.what());
	}
    }

    if (this->targets.empty())
    {
	this->targets = default_targets;
    }

    // Put significant option coverage cases after any potential exit
    // from incorrect usage....
    if (this->buildset_name.empty() && this->this_platform.empty())
    {
	QTC::TC("abuild", "Abuild-init no_deps");
    }
}

void
Abuild::argPositional(std::string const& arg)
{
    boost::regex define_re("([^-][^=]*)=(.*)");
    boost::smatch match;

    if (boost::regex_match(arg, match, define_re))
    {
	this->defines[match.str(1)] = match.str(2);
    }
    else if (arg == " --test-java-builder-bad-java")
    {
	// undocumented option used by the test suite
	this->test_java_builder_bad_java = true;
    }
    else if (arg == " --test-java-builder-protocol")
    {
	// undocumented option used by the test suite
	this->java_builder_args.push_back("-test-protocol");
    }
    else if (special_targets.count(arg))
    {
	if (! this->special_target.empty())
	{
	    QTC::TC("abuild", "Abuild-init ERR usage multiple special targets");
	    usage("only one special target may be specified");
	}
	this->special_target = arg;
    }
    else
    {
	this->targets.push_back(arg);
    }
}

void
Abuild::argHelp(std::vector<std::string> const& args)
{
    if ((args.empty() || (args.size() > 2)))
    {
	QTC::TC("abuild", "Abuild-init ERR usage invalid help");
	usage("invalid --help invocation");
    }
    this->help_topic = args[0];
    if (this->help_topic == h_RULES)
    {
	if (args.size() == 2)
	{
	    this->rules_help_topic = args[1];
	}
	else
	{
	    this->rules_help_topic = hr_HELP;
	}
    }
}

void
Abuild::argFindConf()
{
    this->start_dir = findConf();
}

void
Abuild::argSetJobs(unsigned int arg)
{
    if (arg == 0)
    {
	QTC::TC("abuild", "Abuild-init ERR usage j = 0");
	usage("-j's argument must be > 0");
    }
    else
    {
	this->max_workers = arg;
    }
}

void
Abuild::argSetMakeJobs(unsigned int arg)
{
    this->make_njobs = (int)arg;
}

void
Abuild::argSetKeepGoing()
{
    this->make_args.push_back("-k");
    this->java_builder_args.push_back("-k");
    this->keep_going = true;
}

void
Abuild::argSetOutputMode(output_mode_e mode)
{
    this->output_mode = mode;
}

void
Abuild::argSetNoOp()
{
    this->java_builder_args.push_back("-n");
    this->make_args.push_back("-n");
}

void
Abuild::argSetEmacs()
{
    this->java_builder_args.push_back("-e");
}

void
Abuild::argSetBackendArgs(std::vector<std::string> const& from,
			  std::list<std::string>& to)
{
    to.insert(to.end(), from.begin(), from.end());
}

void
Abuild::argSetJVMXargs(std::vector<std::string> const& val, bool replace)
{
    if (replace)
    {
	this->jvm_xargs.clear();
    }
    this->jvm_xargs.insert(this->jvm_xargs.end(), val.begin(), val.end());
}

void
Abuild::argSetDeprecationIsError()
{
    this->make_args.push_back("ABUILD_DEPRECATE_IS_ERROR=1");
    this->java_builder_args.push_back("-de");
    Error::setDeprecationIsError(true);
}

void
Abuild::argSetVerbose()
{
    this->make_args.push_back("ABUILD_VERBOSE=1");
    this->java_builder_args.push_back("-v");
    this->verbose_mode = true;
}

void
Abuild::argSetSilent()
{
    this->make_args.push_back("ABUILD_SILENT=1");
    this->java_builder_args.push_back("-q");
    this->silent = true;
}

void
Abuild::argSetCompatLevel(std::string& var, boost::smatch const& m)
{
    var = m[0].str();
}

void
Abuild::argSetBuildSet(std::string const& arg)
{
    this->buildset_name = arg;
    this->explicit_buildset = true;
    this->cleanset_name.clear();
    if (! arg.empty())
    {
	checkBuildsetName("build", this->buildset_name);
    }
}

void
Abuild::argSetCleanSet(std::string const& arg)
{
    this->cleanset_name = arg;
    this->explicit_buildset = true;
    this->buildset_name.clear();
    checkBuildsetName("clean", this->cleanset_name);
}

void
Abuild::argSetBool(bool& var, bool val)
{
    var = val;
}

void
Abuild::argSetString(std::string& var, std::string const& val)
{
    var = val;
}

void
Abuild::argSetStringSplit(std::list<std::string>& var, std::string const& val)
{
    var = Util::split(',', val);
}

void
Abuild::argInsertInSet(std::set<std::string>& var, std::string const& val)
{
    var.insert(val);
}

void
Abuild::checkRoRwPaths()
{
    if (this->ro_paths.empty() && this->rw_paths.empty())
    {
	return;
    }

    if (! this->start_dir.empty())
    {
	QTC::TC("abuild", "Abuild-init ro/rw path with start dir");
    }

    checkValidPaths(this->ro_paths);
    checkValidPaths(this->rw_paths);

    if (this->ro_paths.empty())
    {
	QTC::TC("abuild", "Abuild-init only rw paths");
	this->default_writable = false;
	return;
    }

    if (this->rw_paths.empty())
    {
	QTC::TC("abuild", "Abuild-init only ro paths");
	this->default_writable = true;
	return;
    }

    bool any_ro_not_under_some_rw =
	anyFirstNotUnderSomeSecond(this->ro_paths, this->rw_paths);
    bool any_rw_not_under_some_ro =
	anyFirstNotUnderSomeSecond(this->rw_paths, this->ro_paths);

    if (any_ro_not_under_some_rw && any_rw_not_under_some_ro)
    {
	QTC::TC("abuild", "Abuild-init ERR crossing ro/rw paths");
	error("when both --ro-path and --rw-path are specified, EITHER"
	      " each ro path must be under some rw path OR"
	      " each rw path must be under some ro path");
	return;
    }

    if (! any_rw_not_under_some_ro)
    {
	QTC::TC("abuild", "Abuild-init ro on top");
	this->default_writable = true;
    }
    else
    {
	assert(! any_ro_not_under_some_rw);
	QTC::TC("abuild", "Abuild-init rw on top");
	this->default_writable = false;
    }
}

void
Abuild::checkValidPaths(std::set<std::string>& paths)
{
    std::set<std::string> work;
    for (std::set<std::string>::iterator iter = paths.begin();
	 iter != paths.end(); ++iter)
    {
	std::string const& path = *iter;
	if (Util::isDirectory(path))
	{
	    work.insert(Util::canonicalizePath(path));
	}
	else
	{
	    QTC::TC("abuild", "Abuild-init ERR invalid ro/rw path");
	    error("ro/rw path \"" + path + "\" does not exist"
		  " or is not a directory");
	}
    }
    paths = work;
}

bool
Abuild::anyFirstNotUnderSomeSecond(std::set<std::string> const& first,
				   std::set<std::string> const& second)
{
    bool any_not_under_some = false;
    for (std::set<std::string>::const_iterator f = first.begin();
	 f != first.end(); ++f)
    {
	bool under_some = false;
	for (std::set<std::string>::const_iterator s = second.begin();
	     s != second.end(); ++s)
	{
	    if (Util::isDirUnder(*f, *s))
	    {
		under_some = true;
		break;
	    }
	}
	if (! under_some)
	{
	    any_not_under_some = true;
	    break;
	}
    }
    return any_not_under_some;
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
	    QTC::TC("abuild", "Abuild-init ERR bad buildset pattern");
	    usage("invalid regular expression " + pattern);
	}
    }
    else if (buildset_aliases.count(name) != 0)
    {
	QTC::TC("abuild", "Abuild-init replace build set alias");
	name = buildset_aliases[name];
    }
    else if (valid_buildsets.count(name) == 0)
    {
	QTC::TC("abuild", "Abuild-init ERR usage invalid build set");
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
