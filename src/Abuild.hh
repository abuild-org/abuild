#ifndef __ABUILD_HH__
#define __ABUILD_HH__

#include <string>
#include <list>
#include <set>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <Error.hh>
#include <QEXC.hh>
#include <BuildForest.hh>
#include <DependencyGraph.hh>
#include <DependencyEvaluator.hh>
#include <Interface.hh>
#include <PlatformData.hh>
#include <PlatformSelector.hh>
#include <JavaBuilder.hh>
#include <CompatLevel.hh>

class Logger;
class ItemConfig;
class InterfaceParser;
class UpgradeData;
class BackingConfig;

class Abuild
{
  public:
    Abuild(int argc, char* argv[], char* envp[]);
    ~Abuild();
    bool run();

  private:
    Abuild(Abuild const&);
    Abuild& operator=(Abuild const&);

    typedef boost::shared_ptr<BuildForest> BuildForest_ptr;
    typedef std::map<std::string, BuildForest_ptr> BuildForest_map;

    static bool initializeStatics();

    typedef BuildForest::BuildItem_ptr BuildItem_ptr;
    typedef BuildForest::BuildItem_map BuildItem_map;
    typedef BuildForest::BuildTree_ptr BuildTree_ptr;
    typedef BuildForest::BuildTree_map BuildTree_map;
    typedef boost::function<bool(std::string const&, std::string const&)>
            item_filter_t;

    bool runInternal();
    void getThisPlatform();
    void parseArgv();
    std::string findConf();
    void checkBuildsetName(std::string const& kind, std::string& name);
    void initializePlatforms();
    void initializeJavaPlatforms();
    void loadPlatformData(PlatformData&, std::string const& dir);
    bool readConfigs();
    ItemConfig* readConfig(std::string const& dir,
			   std::string const& parent_dir);
    ItemConfig* readExternalConfig(std::string const& dir,
				   std::string const& external);
    std::string findTop(std::string const& start_dir,
			std::string const& description);
    void traverse(BuildForest_map&, std::string const& forest_top,
		  std::set<std::string>& visiting,
		  std::string const& description);
    void traverseItems(BuildForest& forest, std::string const& top_path,
		       std::list<std::string>& dirs_with_externals,
		       std::list<std::string>& backing_areas,
		       std::set<std::string>& deleted_trees,
		       std::set<std::string>& deleted_items);
    std::string registerBuildTree(BuildForest& forest,
				  std::string const& dir,
				  ItemConfig* config,
				  std::list<std::string>& dirs_with_externals);
    std::string getAssignedTreeName(std::string const& dir);
    std::string getAssignedTreeName(std::string const& dir,
				    std::set<std::string>& visiting);
    void validateForest(BuildForest_map& forests,
			std::string const& top_path);
    void resolveFromBackingAreas(BuildForest_map& forests,
				 std::string const& top_path);
    void checkTreeDependencies(BuildForest& forest);
    void checkTreeAccess(BuildForest& forest);
    bool checkAllowedTree(BuildForest& forest,
			  BuildItem& referrer,
			  BuildItem& referent,
			  std::string const& action);
    void resolveTraits(BuildForest& forest);
    void checkPlugins(BuildForest& forest);
    bool isPluginAnywhere(std::string const& item_name);
    void checkPlatformTypes(BuildForest& forest);
    void checkItemNames(BuildForest& forest);
    bool accessibleFrom(BuildItem_map& builditems,
			std::string const& accessor,
			std::string const& accessee);
    void checkBuildAlso(BuildForest& forest);
    void checkItemDependencies(BuildForest& forest);
    void updatePlatformTypes(BuildForest& forest);
    void checkDependencyPlatformTypes(BuildForest& forest);
    void checkFlags(BuildForest& forest);
    void checkTraits(BuildForest& forest);
    void checkIntegrity(BuildForest_map& forests,
			std::string const& top_path);
    void reportIntegrityErrors(BuildForest_map& forests,
			       BuildItem_map& builditems,
			       std::string const& top_path);
    void computeBuildablePlatforms(BuildForest& forest);
    void appendBackingData(std::string const& dir,
			   std::list<std::string>& backing_areas,
			   std::set<std::string>& deleted_trees,
			   std::set<std::string>& deleted_items);
    BackingConfig* readBacking(std::string const& dir);
    void computeValidTraits(BuildForest_map& forests);
    void listTraits();
    void listPlatforms(BuildForest_map& forests);
    void dumpData(BuildForest_map& forests);
    void dumpPlatformData(PlatformData const&, std::string const& indent);
    void dumpBuildTree(BuildTree& tree, std::string const& tree_name,
		       BuildForest& forest,
		       std::map<std::string, int>& forest_numbers);
    void dumpBuildItem(BuildItem& item, std::string const& item_name,
		       std::map<std::string, int>& forest_numbers);
    void computeBuildset(BuildItem_map& builditems);
    void populateBuildset(BuildItem_map& builditems,
			  boost::function<bool(BuildItem const*)> pred);
    bool addBuildAlsoToBuildset(BuildItem_map& builditems);
    bool buildBuildset();
    bool addItemToBuildGraph(std::string const& item_name, BuildItem& item);
    void findGnuMakeInPath();
    void findJava();
    bool isThisItemThisPlatform(std::string const& name,
				std::string const& platform);
    bool isThisItem(std::string const& name, std::string const& platform);
    bool isAnyItem(std::string const& name, std::string const& platform);
    bool itemBuilder(std::string builder_string, item_filter_t filter,
		     bool is_dep_failure);
    bool buildItem(std::string const& item_name,
		   std::string const& item_platform,
		   BuildItem& build_item);
    std::string createOutputDirectory(std::string const& item_platform,
				      BuildItem& build_item);
    void stateChangeCallback(std::string const& builder_string,
			     DependencyEvaluator::ItemState state,
			     item_filter_t filter);
    bool createItemInterface(std::string const& builder_string,
			     std::string const& item_name,
			     std::string const& item_platform,
			     BuildItem& build_item,
			     InterfaceParser& parser);
    bool createPluginInterface(std::string const& plugin_name,
			       BuildItem& build_item);
    void dumpInterface(std::string const& item_platform,
		       BuildItem& build_item,
		       std::string const& suffix = "");
    void assignInterfaceVariable(Interface&, FileLocation const&,
				 std::string const& variable_name,
				 std::string const& value,
				 Interface::assign_e assignment_type,
				 bool& status);
    bool readAfterBuilds(std::string const& item_name,
			 std::string const& item_platform,
			 BuildItem& build_item,
			 InterfaceParser& parser);
    std::list<std::string> getRulePaths(std::string const& item_name,
					BuildItem& build_item,
					std::string const& dir,
					bool relative);
    bool invoke_gmake(std::string const& item_name,
		      std::string const& item_platform,
		      BuildItem& build_item,
		      std::string const& dir,
		      std::list<std::string> const& targets);
    bool invoke_ant(std::string const& item_name,
		    std::string const& item_platform,
		    BuildItem& build_item,
		    std::string const& dir,
		    std::list<std::string> const& targets);
    bool invoke_groovy(std::string const& item_name,
		       std::string const& item_platform,
		       BuildItem& build_item,
		       std::string const& dir,
		       std::list<std::string> const& targets);
    bool invokeJavaBuilder(std::string const& backend,
			   std::string const& build_file,
			   std::string const& dir,
			   std::list<std::string> const& targets);
    bool invokeBackend(std::string const& progname,
		       std::vector<std::string> const& args,
		       std::map<std::string, std::string> const& environment,
		       char* old_env[],
		       std::string const& dir);
    void flushLogIfSingleThreaded();
    void cleanBuildset();
    void cleanPath(std::string const& item_name, std::string const& dir);
    void help();
    void usage(std::string const& msg);
    void exitIfErrors();
    void info(std::string const& msg);
    void verbose(std::string const& msg);
    void monitorOutput(std::string const& msg);
    void monitorErrorCallback(std::string const& msg);
    void error(std::string const& msg);
    void error(FileLocation const&, std::string const& msg);
    void fatal(std::string const& msg);

    // methods in Abuild-upgrade.cc
    bool upgradeTrees();
    void findBuildItems(UpgradeData&);
    void constructTreeGraph(UpgradeData&, DependencyGraph& g);
    void validateProposedForests(
	UpgradeData&,
	std::vector<DependencyGraph::ItemList> const& forests);
    std::string getForestRoot(std::list<std::string> const& forest);

    static std::string const ABUILD_VERSION;
    static std::string const OUTPUT_DIR_PREFIX;
    static std::string const FILE_DYNAMIC_MK;
    static std::string const FILE_DYNAMIC_ANT;
    static std::string const FILE_DYNAMIC_GROOVY;
    static std::string const FILE_INTERFACE_DUMP;
    static std::string const b_ALL;
    static std::string const b_LOCAL;
    static std::string const b_DESC;
    static std::string const b_DEPS;
    static std::string const b_CURRENT;
    static std::set<std::string> valid_buildsets;
    static std::map<std::string, std::string> buildset_aliases;
    static std::string const s_CLEAN;
    static std::string const s_NO_OP;
    static std::string const PLUGIN_PLATFORM;
    static std::string const FILE_PLUGIN_INTERFACE;
    static std::string const BUILDER_RE;
    static std::set<std::string> special_targets;
    static std::list<std::string> default_targets;

    static bool statics_initialized;

    int argc;
    char** argv;
    char** envp;

    // Parameters determined from the command line or at startup
    std::string whoami;
    bool stdout_is_tty;
    unsigned int max_workers;
    std::list<std::string> make_args;
    std::list<std::string> java_builder_args;
    std::map<std::string, std::string> defines;
    std::string start_dir;
    bool keep_going;
    bool no_dep_failures;
    std::string buildset_name;
    std::set<std::string> buildset_named_items;
    std::string buildset_pattern;
    std::string cleanset_name;
    bool full_integrity;
    bool list_traits;
    bool list_platforms;
    bool dump_data;
    bool dump_build_graph;
    bool verbose_mode;
    bool silent;
    bool monitored;
    bool dump_interfaces;
    std::string special_target;
    std::list<std::string> targets;
    bool apply_targets_to_deps;
    std::set<std::string> explicit_target_items;
    std::list<std::string> only_with_traits;
    std::list<std::string> related_by_traits;
    std::map<std::string, PlatformSelector> platform_selectors;
    std::set<std::string> clean_platforms;
    CompatLevel compat_level;
    bool local_build;

    // Other data
    Error error_handler;
    std::string current_directory;
    std::string program_fullpath;
    std::string abuild_top;
    std::string native_os;
    std::string native_cpu;
    std::string native_toolset;
    std::string this_platform;
    std::string this_config_dir;
    ItemConfig* this_config;
    unsigned int last_assigned_tree_number;
    std::map<std::string, std::string> assigned_tree_names;
    PlatformData internal_platform_data;
    std::set<std::string> valid_traits;
    BuildItem_map buildset;
    std::set<std::string> plugins_anywhere;
#ifdef _WIN32
    bool have_perl;
#endif
    std::string gmake;
    boost::shared_ptr<JavaBuilder> java_builder;
    DependencyGraph build_graph;
    boost::shared_ptr<Interface> base_interface;
    std::vector<std::string> buildset_reverse_order;
    std::vector<std::string> failed_builds;

    // (forest, other_forest) -> [plugin, ...]
    typedef std::map<std::string,
		     std::vector<std::string> > map_string_vec_string;
    typedef std::map<std::string, map_string_vec_string> shadowed_plugin_map;
    shadowed_plugin_map shadowed_plugins;

    Logger& logger;
};

#endif // __ABUILD_HH__
