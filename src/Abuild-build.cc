// Methods involved in doing the actual build

#include <Abuild.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <ProcessHandler.hh>
#include <Logger.hh>
#include <FileLocation.hh>
#include <ItemConfig.hh>
#include <InterfaceParser.hh>
#include <DependencyRunner.hh>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

// Unlock a lock object is in scope
class ScopedUnlock
{
  public:
    ScopedUnlock(boost::mutex::scoped_lock& l) :
	l(l)
    {
	l.unlock();
    }
    virtual ~ScopedUnlock()
    {
	l.lock();
    }
  private:
    boost::mutex::scoped_lock& l;
};

bool
Abuild::buildBuildset()
{
    std::string const& this_name = this->this_config->getName();
    if (this->local_build &&
        (this_name.empty() ||
	 (! this_config->hasBuildFile())))
    {
        QTC::TC("abuild", "Abuild-build no local build");
        notice("nothing to build in this directory");
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
	    QTC::TC("abuild", "Abuild-build some items skipped");
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
	QTC::TC("abuild", "Abuild-build some native items skipped");
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

    if (this->monitored || this->dump_build_graph)
    {
	monitorOutput("begin-dump-build-graph");

	boost::function<void(std::string const&)> o =
	    boost::bind(&Logger::logInfo, &(this->logger), _1, Logger::NO_JOB);

	std::string item_name;
	std::string platform;

	o("<?xml version=\"1.0\"?>");
	o("<build-graph version=\"1\">");

	DependencyGraph::ItemList const& items =
	    this->build_graph.getSortedGraph();
	for (DependencyGraph::ItemList::const_iterator iter =
		 items.begin();
	     iter != items.end(); ++iter)
	{
	    DependencyGraph::ItemList const& deps =
		this->build_graph.getDirectDependencies(*iter);
	    parseBuildGraphNode(*iter, item_name, platform);
	    std::string msg = "item name=\"" + item_name +
		"\" platform=\"" + platform + "\"";
	    if (deps.empty())
	    {
		o(" <" + msg + "/>");
	    }
	    else
	    {
		o(" <" + msg + ">");
		for (DependencyGraph::ItemList::const_iterator dep_iter =
			 deps.begin();
		     dep_iter != deps.end(); ++dep_iter)
		{
		    parseBuildGraphNode(*dep_iter, item_name, platform);
		    o("  <dep name=\"" + item_name +
		      "\" platform=\"" + platform + "\"/>");
		}
		o(" </item>");
	    }
	}

	o("</build-graph>");

	monitorOutput("end-dump-build-graph");
    }

    if (this->dump_build_graph)
    {
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
    InterfaceParser base_parser(
	this->error_handler, ":base", ":base", this->abuild_top);
    if (! base_parser.parse(
	    this->abuild_top + "/private/base.interface", false))
    {
	fatal("errors detected in base interface file");
    }
    this->base_interface = base_parser.getInterface();
    this->base_interface->setTargetType(TargetType::tt_all);

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
	       this->error_handler,
	       internal, "ABUILD_STDOUT_IS_TTY",
	       (this->stdout_is_tty ? "1" : "0"),
	       Interface::a_normal));

    computeItemPrefixes();

    // Build appropriate items in dependency order.
    DependencyRunner r(this->build_graph, this->max_workers,
		       boost::bind(&Abuild::itemBuilder, this,
				   _1, filter, false));
    r.setChangeCallback(boost::bind(&Abuild::stateChangeCallback,
				    this, _1, _2, filter), true);
    info("build starting");
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
    bool status = r.run(stop_on_error, disable_failure_propagation);
    info("build complete");
    return status;
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
    // "as-needed platform selection."  In order for this to work
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
    // the build platform lists of all their reverse dependencies.

    // The logic of setting up the build graph is quite subtle, and
    // although the code itself is fairly simple, there are actually
    // quite a number of cases.  For purposes of discussion, suppose
    // each node in the build graph is a string of the form
    // item_name:platform.  For every item i and every platform p in
    // its build platforms, we add the node i:p to the build graph.
    // We create a dependency from A:p1 to B:p2 in the build graph
    // whenever A's build on p1 depends on B's build on p2.

    // In the normal case of A:p1 -> B:p2, p1 = p2.  There are two
    // exceptions to this rule:

    //  * If B is platform-independent, then p2 is be "indep"
    //    regardless of what p1 is.

    //  * If the dependency from A to B in A's Abuild.conf is declared
    //    with platform type pt, A must not have target type "all",
    //    and B must have pt as one of its platform types.  In this
    //    case, p2 is the highest priority platform in pt that appears
    //    in B's buildable platform list subject to any qualifications
    //    in the platform selector in the dependency declaration.  If
    //    there are no matching platforms, it is an error.

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
	// build graph.
	this->build_graph.addItem(createBuildGraphNode(item_name, *bp_iter));
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

	// XXXp Maybe if dep platform type is empty, this is where we
	// check for parent platform types.  If A depends on B and A
	// builds on platform type t1 and B does not build on t1 but
	// does build on an ancestor of t1, treat this as if there
	// were a platform-specific dependency and resolve in the same
	// way.

	std::string override_platform;
	if (! dep_platform_type.empty())
	{
	    if (ps)
	    {
		QTC::TC("abuild", "Abuild-build platform dependency with selector");
	    }
	    else
	    {
		// If the dependency was declared with a platform type
		// only and no selector, see if the user specified any
		// general selectors.
		if (this->platform_selectors.count(dep_platform_type))
		{
		    QTC::TC("abuild", "Abuild-build specific platform selector");
		    ps = &(this->platform_selectors[dep_platform_type]);
		}
		else if (this->platform_selectors.count(PlatformSelector::ANY))
		{
		    QTC::TC("abuild", "Abuild-build default platform selector");
		    ps = &(this->platform_selectors[PlatformSelector::ANY]);
		}
		else
		{
		    QTC::TC("abuild", "Abuild-build no platform selector");
		}
		if (ps && ps->isSkip())
		{
		    QTC::TC("abuild", "Abuild-build ignoring skip selector");
		    ps = 0;
		}
	    }

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
		QTC::TC("abuild", "Abuild-build ERR no override platform");
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
	    std::string platform_item =
		createBuildGraphNode(item_name, item_platform);

	    // XXXp handle hierarchical types here?  indep should not
	    // have to be a special case.  It should end up as the
	    // override type instead.
	    if (dep_type == TargetType::tt_platform_independent)
	    {
		// Allow any item to depend on a platform-independent
		// item
		QTC::TC("abuild", "Abuild-build item -> indep",
			((item_platform == PlatformData::PLATFORM_INDEP)
			 ? 0 : 1));
		this->build_graph.addDependency(
		    platform_item, createBuildGraphNode(
			dep, PlatformData::PLATFORM_INDEP));
	    }
	    else if (! override_platform.empty())
	    {
		// If an override platform is available (a specific
		// platform type was declared on this dependency),
		// create a dependency on that specific platform.
		QTC::TC("abuild", "Abuild-build override platform");
		dep_item.addBuildPlatform(override_platform);
		this->build_graph.addDependency(
		    platform_item, createBuildGraphNode(
			dep, override_platform));
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
		// build platform list.  This is how as-needed
		// platform selection happens.
		QTC::TC("abuild", "Abuild-build A:p -> B:p",
			((dep_type == TargetType::tt_all) ? 0 : 1));
		if (dep_item.getBuildPlatforms().count(item_platform) == 0)
		{
		    QTC::TC("abuild", "Abuild-build as-needed platform selection",
			    ((dep_type == TargetType::tt_all) ? 0 : 1));
		    dep_item.addBuildPlatform(item_platform);
		}
		this->build_graph.addDependency(
		    platform_item, createBuildGraphNode(
			dep, item_platform));
	    }
	    else if (item_type == TargetType::tt_all)
	    {
		// If an item is of type all, ignore the case of a
		// dependency not having this platform.  This is how
		// pass-through build items work.
		QTC::TC("abuild", "Abuild-build ignoring all's dep without platform");
	    }
	    else
	    {
		QTC::TC("abuild", "Abuild-build ERR unmatched platform");
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

std::string
Abuild::createBuildGraphNode(std::string const& item_name,
			     std::string const& platform)
{
    // Use ! as a separator because it lexically sorts before
    // everything except space, and using space is inconvenient.
    assert(this->buildset.count(item_name));
    std::string const& item_tree = this->buildset[item_name]->getTreeName();
    assert(this->buildgraph_tree_prefixes.count(item_tree));
    return this->buildgraph_tree_prefixes[item_tree] + "!" +
	item_name + "!" + platform;
}

void
Abuild::parseBuildGraphNode(std::string const& node,
			    std::string& item_name,
			    std::string& platform)
{
    boost::regex builder_re("[^!]+!([^!]+)!([^!]+)");
    boost::smatch match;
    assert(boost::regex_match(node, match, builder_re));
    item_name = match[1].str();
    platform = match[2].str();
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
	    QTC::TC("abuild", "Abuild-build infer JAVA_HOME");
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
		QTC::TC("abuild", "Abuild-build infer ANT_HOME");
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

    if (this->test_java_builder_bad_java)
    {
	// For test suite: pass a bad java path to JavaBuilder to
	// exercise having the java backend fail to start.
	java = "/non/existent/java";
    }

    Logger::job_handle_t jb_logger_job =
	logger.requestJobHandle(
	    false, (this->use_job_prefix ? "[JavaBuilder] " : ""));
    this->java_builder.reset(
	new JavaBuilder(
	    this->error_handler,
	    (this->output_mode != om_raw),
	    jb_logger_job,
	    boost::bind(&Abuild::verbose, this, _1, jb_logger_job),
	    this->abuild_top, java, java_home, ant_home,
	    java_libs, this->jvm_xargs,
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
    // This method, and therefore all methods it calls, may be run
    // simultaenously from multiple threads.  With the exception of
    // invoking the backend, we really want these activities to be
    // serialized since there are many operations that could
    // potentially write to shared resources.  As such, we acquire a
    // unique_lock (through scoped_lock) on build_mutex.  Right before
    // invocation of the backend (if any), the lock is explicitly
    // unlocked.  It is then relocked after the backend returns.

    boost::mutex::scoped_lock build_lock(this->build_mutex);

    // NOTE: This function must not return failure without adding the
    // failed builder_string to the failed_builds list.

    boost::smatch match;
    std::string item_name;
    std::string item_platform;
    parseBuildGraphNode(builder_string, item_name, item_platform);
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

    std::string output_dir = OUTPUT_DIR_PREFIX + item_platform;
    std::string item_label = item_name + " (" + output_dir + ")";

    std::string job_prefix;
    if (this->use_job_prefix)
    {
	assert(this->buildgraph_item_prefixes.count(builder_string));
	job_prefix = this->buildgraph_item_prefixes[builder_string] + " ";
    }
    Logger::job_handle_t logger_job = this->logger.requestJobHandle(
	(this->output_mode == om_buffered), job_prefix);

    // Job header is cleared if/when we actually start building, so
    // this is only seen if there are interface errors or verbose
    // output preceding the actual build.
    this->logger.setJobHeader(
	logger_job, this->whoami + ": " + item_label + ":");

    Error item_error(logger_job, this->whoami);
    std::string const& abs_path = build_item.getAbsolutePath();
    InterfaceParser parser(item_error, item_name, item_platform, abs_path);
    parser.setSupportedFlags(build_item.getSupportedFlags());
    bool status = true;

    if (use_interfaces)
    {
	status = createItemInterface(
	    builder_string, item_name, item_platform, build_item,
	    item_error, parser, logger_job);
    }

    if (build_item.hasBuildFile())
    {
	// Ready to build -- clear job header, and let the build's own
	// output take precedence.
	this->logger.setJobHeader(logger_job, "");

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
		info(item_label +
		     " will not be built because of failure of a dependency",
		     logger_job);
	    }
	    else
	    {
		status = buildItem(
		    build_lock, logger_job,
		    item_name, item_platform, build_item);
	    }
	}

	// If all is well, read any after-build files.  Disallow them
	// from declaring their own after-build files.
	if (status && use_interfaces)
	{
	    status = readAfterBuilds(
		item_platform, item_platform, build_item, parser, logger_job);
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
	    QTC::TC("abuild", "Abuild-build ERR after-build with no build file");
	    std::string interface_file =
		abs_path + "/" + ItemConfig::FILE_INTERFACE;
	    error(FileLocation(interface_file, 0, 0),
		  "interfaces for items with no build files may not load"
		  " after-build files",
		  logger_job);
	    status = false;
	}
	else
	{
	    QTC::TC("abuild", "Abuild-build not building with no build file",
		    build_item.getTargetType() == TargetType::tt_all ? 0 : 1);
	}
    }

    if (! status)
    {
        notice(item_label + ": build failed", logger_job);
	this->failed_builds.push_back(builder_string);
    }

    this->logger.closeJob(logger_job);

    return status;
}

void
Abuild::stateChangeCallback(std::string const& builder_string,
			    DependencyEvaluator::ItemState state,
			    item_filter_t filter)
{
    boost::smatch match;
    std::string item_name;
    std::string item_platform;
    parseBuildGraphNode(builder_string, item_name, item_platform);
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
			    Error& item_error,
			    InterfaceParser& parser,
			    Logger::job_handle_t logger_job)
{
    verbose("creating interface for " + item_name + " on " + item_platform,
	    logger_job);

    // Initialize this item's interface.
    build_item.setInterface(item_platform, parser.getInterface());

    // Import the base interface.
    bool status = parser.importInterface(*(this->base_interface));

    // Import plugin interfaces
    std::list<std::string> const& plugins = build_item.getPlugins();
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	verbose("importing interface for plugin " + *iter, logger_job);
	if (! parser.importInterface(
		this->buildset[*iter]->getInterface(PLUGIN_PLATFORM)))
	{
	    QTC::TC("abuild", "Abuild-build ERR import of plugin interface");
	    status = false;
	}
    }

    // Import the interfaces of our direct dependencies.  They already
    // include the interfaces of our indirect dependencies.
    std::list<std::string> const& deps =
	this->build_graph.getDirectDependencies(builder_string);
    for (std::list<std::string>::const_iterator iter = deps.begin();
	 iter != deps.end(); ++iter)
    {
	std::string dep_name;
	std::string dep_platform;
	parseBuildGraphNode(*iter, dep_name, dep_platform);

	BuildItem& dep_item = *(this->buildset[dep_name]);
	verbose("importing interface for dependency " + dep_name, logger_job);
	if (! parser.importInterface(dep_item.getInterface(dep_platform)))
	{
	    QTC::TC("abuild", "Abuild-build ERR import of dependent interface");
	    status = false;
	}
    }

    // Assign variables that are relevant to this build.  This must be
    // done regardless of whether we have our own interface file
    // because these values may be accessed by the item's local build
    // file well.

    Interface& _interface = *(parser.getInterface());
    FileLocation internal("[internal: " + builder_string + "]", 0, 0);

    // We keep ABUILD_THIS around since we have no way of detecting or
    // warning for its use in backend configuration files and
    // therefore can't easily deprecate it.
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_THIS", build_item.getName(),
			    Interface::a_override, status);

    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_ITEM_NAME", build_item.getName(),
			    Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_TREE_NAME",
			    build_item.getTreeName(),
			    Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_TARGET_TYPE",
			    TargetType::getName(build_item.getTargetType()),
			    Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_PLATFORM_TYPE",
			    build_item.getPlatformType(item_platform),
			    Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_PLATFORM", item_platform,
			    Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
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

    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_PLATFORM",
			    item_platform, Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_PLATFORM_OS",
			    platform_os, Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_PLATFORM_CPU",
			    platform_cpu, Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_PLATFORM_TOOLSET",
			    platform_toolset, Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_PLATFORM_COMPILER",
			    platform_compiler, Interface::a_override, status);
    assignInterfaceVariable(item_error, _interface,
			    internal, "ABUILD_PLATFORM_OPTION",
			    platform_option, Interface::a_override, status);

    // Create local variables for any optional dependencies indicating
    // whether or not they are present.
    std::map<std::string, bool> const& optional_deps =
	build_item.getOptionalDependencyPresence();
    for (std::map<std::string, bool>::const_iterator iter =
	     optional_deps.begin();
	 iter != optional_deps.end(); ++iter)
    {
	std::string const& dep = (*iter).first;
	std::string present = ((*iter).second ? "1" : "0");
	std::string var = "ABUILD_HAVE_OPTIONAL_DEP_" + dep;
	declareInterfaceVariable(
	    item_error, _interface, internal, var,
	    Interface::s_local, Interface::t_boolean, Interface::l_scalar,
	    status);
	assignInterfaceVariable(
	    item_error, _interface, internal,
	    var, present, Interface::a_normal, status);
    }

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
	verbose("loading " + ItemConfig::FILE_INTERFACE, logger_job);
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
	this->error_handler, plugin_name, PLUGIN_PLATFORM, dir);
    build_item.setInterface(PLUGIN_PLATFORM, parser.getInterface());
    bool status = parser.importInterface(*(this->base_interface));

    std::string interface_file = dir + "/" + FILE_PLUGIN_INTERFACE;
    if (Util::isFile(interface_file))
    {
	// Load the interface file
	if (! parser.parse(interface_file, false))
	{
	    QTC::TC("abuild", "Abuild-build ERR error loading plugin interface");
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

    if (! isBuildItemWritable(build_item))
    {
	QTC::TC("abuild", "Abuild-build dumpInterface ignoring read-only build item");
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
Abuild::declareInterfaceVariable(Error& item_error,
				 Interface& _interface,
				 FileLocation const& location,
				 std::string const& variable_name,
				 Interface::scope_e scope,
				 Interface::type_e type,
				 Interface::list_e list_type,
				 bool& status)
{
    if (! _interface.declareVariable(
	    item_error, location, variable_name, scope, type, list_type))
    {
	status = false;
    }
}

void
Abuild::assignInterfaceVariable(Error& item_error,
				Interface& _interface,
				FileLocation const& location,
				std::string const& variable_name,
				std::string const& value,
				Interface::assign_e assignment_type,
				bool& status)
{
    if (! _interface.assignVariable(
	    item_error, location, variable_name, value, assignment_type))
    {
	status = false;
    }
}

bool
Abuild::readAfterBuilds(std::string const& item_name,
			std::string const& item_platform,
			BuildItem& build_item,
			InterfaceParser& parser,
			Logger::job_handle_t logger_job)
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
		    Util::absToRel(after_build, abs_path),
		    logger_job);
	    if (parser.parse(after_build, false))
	    {
		QTC::TC("abuild", "Abuild-build good after-build");
	    }
	    else
	    {
		// Errors would have been issued
		status = false;
	    }
	}
	else
	{
	    QTC::TC("abuild", "Abuild-build ERR missing after-build");
	    error(FileLocation(interface_file, 0, 0),
		  "after-build file " +
		  Util::absToRel(after_build, abs_path) +
		  " does not exist",
		  logger_job);
	    status = false;
	}
    }

    return status;
}

bool
Abuild::buildItem(boost::mutex::scoped_lock& build_lock,
		  Logger::job_handle_t logger_job,
		  std::string const& item_name,
		  std::string const& item_platform,
		  BuildItem& build_item)
{
    if (! isBuildItemWritable(build_item))
    {
	// Assume that this item has previously been built
	// successfully.
	QTC::TC("abuild", "Abuild-build not building read-only build item",
		(build_item.getBackingDepth() == 0) ? 0 : 1);
	return true;
    }

    std::string output_dir = OUTPUT_DIR_PREFIX + item_platform;
    if (this->special_target == s_NO_OP)
    {
        QTC::TC("abuild", "Abuild-build no-op");
	info(item_name + " (" + output_dir + "): " + this->special_target,
	    logger_job);
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

    info(item_name + " (" + output_dir + "): " +
	 Util::join(" ", backend_targets) +
	 (this->verbose_mode ? " in " + rel_output_dir : ""),
	 logger_job);

    bool result = false;

    switch (build_item.getBackend())
    {
      case ItemConfig::b_make:
	result = invoke_gmake(build_lock, logger_job,
			      item_name, item_platform, build_item,
			      abs_output_dir, backend_targets);
	break;

      case ItemConfig::b_ant:
	result = invoke_ant(build_lock, logger_job,
			    item_name, item_platform, build_item,
			    abs_output_dir, backend_targets);
	break;

      case ItemConfig::b_groovy:
	result = invoke_groovy(build_lock, logger_job,
			       item_name, item_platform, build_item,
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

    TargetType::target_type_e target_type = build_item.getTargetType();

    std::list<std::string> dirs;
    for (std::list<std::string>::iterator iter = candidate_paths.begin();
	 iter != candidate_paths.end(); ++iter)
    {
	appendRulePaths(dirs, *iter, target_type);
    }

    // Internal directories are absolute unconditionally.
    std::list<std::string> result;
    appendRulePaths(result, this->abuild_top, target_type);

    for (std::list<std::string>::iterator iter = dirs.begin();
	 iter != dirs.end(); ++iter)
    {
	if (relative)
	{
	    result.push_back(Util::absToRel(*iter, dir));
	}
	else
	{
	    result.push_back(*iter);
	}
    }
    return result;
}

void
Abuild::appendRulePaths(std::list<std::string>& rules_dirs,
			std::string const& dir,
			TargetType::target_type_e desired_type)
{
    std::string const& rules_dir = dir + "/rules/";

    static TargetType::target_type_e types[4] = {
	TargetType::tt_platform_independent,
	TargetType::tt_object_code,
	TargetType::tt_java,
	TargetType::tt_all
    };

    for (unsigned int i = 0; i < 4; ++i)
    {
	if ((types[i] == TargetType::tt_all) ||
	    (desired_type == TargetType::tt_all) ||
	    (types[i] == desired_type))
	{
	    std::string const& type_string = TargetType::getName(types[i]);
	    std::string candidate = rules_dir + type_string;
	    if (Util::isDirectory(candidate))
	    {
		rules_dirs.push_back(candidate);
	    }
	}
    }
}

std::list<std::string>
Abuild::getToolchainPaths(std::string const& item_name,
			  BuildItem& build_item,
			  std::string const& dir,
			  bool relative)
{
    // Generate a list of paths from which we should attempt to load
    // toolchains.  This is the toolchains directory for every plugin
    // that has one as well as the built-in toolchains directory.

    std::list<std::string> dirs;
    std::list<std::string> const& plugins = build_item.getPlugins();
    for (std::list<std::string>::const_iterator iter = plugins.begin();
	 iter != plugins.end(); ++iter)
    {
	appendToolchainPaths(dirs, this->buildset[*iter]->getAbsolutePath());
    }

    std::list<std::string> result;

    // Internal directories are absolute unconditionally.  Note that
    // we can't move toolchains out of "make" without breaking any
    // existing compiler toolchain that loads unix_compiler.mk or any
    // of the other built-in ones.
    appendToolchainPaths(result, this->abuild_top + "/make");

    for (std::list<std::string>::iterator iter = dirs.begin();
	 iter != dirs.end(); ++iter)
    {
	if (relative)
	{
	    result.push_back(Util::absToRel(*iter, dir));
	}
	else
	{
	    result.push_back(*iter);
	}
    }

    return result;
}

void
Abuild::appendToolchainPaths(std::list<std::string>& toolchain_dirs,
			     std::string const& dir)
{
    std::string candidate = dir + "/toolchains";
    if (Util::isDirectory(candidate))
    {
	toolchain_dirs.push_back(candidate);
    }
}

bool
Abuild::invoke_gmake(boost::mutex::scoped_lock& build_lock,
		     Logger::job_handle_t logger_job,
		     std::string const& item_name,
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

    // Generate a list of paths from which we should attempt to load
    // toolchains.
    std::list<std::string> toolchain_paths =
	getToolchainPaths(item_name, build_item, dir, true);
    mk << "ABUILD_TOOLCHAIN_PATHS :=";
    for (std::list<std::string>::iterator iter = toolchain_paths.begin();
	 iter != toolchain_paths.end(); ++iter)
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
    if (this->silent)
    {
	make_argv.push_back("--no-print-directory");
    }
    make_argv.insert(make_argv.end(),
		     this->make_args.begin(), this->make_args.end());
    if (this->make_njobs != 1)
    {
	if (build_item.isSerial())
	{
	    QTC::TC("abuild", "Abuild-build explicit serial");
	    verbose("invoking make serially", logger_job);
	}
	else
	{
	    QTC::TC("abuild", "Abuild-build make_njobs",
		    (this->make_njobs < 0) ? 0 : 1);
	    if (this->make_njobs > 1)
	    {
		make_argv.push_back("-j" + Util::intToString(this->make_njobs));
	    }
	    else
	    {
		make_argv.push_back("-j");
	    }
	}
    }
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

    return invokeBackend(build_lock, logger_job, this->gmake, make_argv,
			 std::map<std::string, std::string>(), dir);
}

bool
Abuild::invoke_ant(boost::mutex::scoped_lock& build_lock,
		   Logger::job_handle_t logger_job,
		   std::string const& item_name,
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

    return invokeJavaBuilder(build_lock, logger_job,
			     "ant", build_file, dir, targets);
}

bool
Abuild::invoke_groovy(boost::mutex::scoped_lock& build_lock,
		      Logger::job_handle_t logger_job,
		      std::string const& item_name,
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

    return invokeJavaBuilder(build_lock, logger_job,
			     "groovy", "", dir, targets);
}

bool
Abuild::invokeJavaBuilder(boost::mutex::scoped_lock& build_lock,
			  Logger::job_handle_t logger_job,
			  std::string const& backend,
			  std::string const& build_file,
			  std::string const& dir,
			  std::list<std::string> const& targets)
{
    flushLogIfSingleThreaded();

    if (this->verbose_mode)
    {
	// Put this in the if statement to avoid the needless cal to
	// join if not in verbose mode.
	verbose("running JavaBuilder backend " + backend, logger_job);
	if (! build_file.empty())
	{
	    verbose("  build file: " + build_file, logger_job);
	}
	verbose("  directory: " + dir, logger_job);
	verbose("  targets: " + Util::join(" ", targets), logger_job);
    }

    ProcessHandler::output_handler_t output_handler = 0;
    if (this->capture_output)
    {
	output_handler = this->logger.getOutputHandler(logger_job);
    }
    // Explicitly unlock the build lock during invocation of the
    // backend
    ScopedUnlock unlock(build_lock);
    return this->java_builder->invoke(
	backend, build_file, dir, targets, output_handler);
}

bool
Abuild::invokeBackend(boost::mutex::scoped_lock& build_lock,
		      Logger::job_handle_t logger_job,
		      std::string const& progname,
		      std::vector<std::string> const& args,
		      std::map<std::string, std::string> const& environment,
		      std::string const& dir)
{
    flushLogIfSingleThreaded();

    if (this->verbose_mode)
    {
	// Put this in the if statement to avoid the needless cal to
	// join if not in verbose mode.
	verbose("running " + Util::join(" ", args), logger_job);
    }

    ProcessHandler::output_handler_t output_handler = 0;
    if (this->capture_output)
    {
	output_handler = this->logger.getOutputHandler(logger_job);
    }
    // Explicitly unlock the build lock during invocation of the
    // backend
    ScopedUnlock unlock(build_lock);
    return process_handler.runProgram(
	progname, args, environment, true, dir, output_handler);
}

void
Abuild::flushLogIfSingleThreaded()
{
    if ((this->max_workers == 1) && (this->output_mode == om_raw))
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
	if (isBuildItemWritable(item))
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
Abuild::cleanOutputDir()
{
    assert(! this->this_platform.empty());
    assert(Util::isFile(".abuild"));
    info("cleaning output directory");

    std::vector<std::string> entries = Util::getDirEntries(".");
    for (std::vector<std::string>::iterator iter = entries.begin();
	 iter != entries.end(); ++iter)
    {
	std::string const& entry = *iter;
	if (entry == ".abuild")
	{
	    continue;
	}
	try
	{
	    Util::removeFileRecursively(entry);
	}
	catch(QEXC::General& e)
	{
	    error(e.what());
	}
    }
}
