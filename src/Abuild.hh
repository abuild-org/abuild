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
    void traverse(BuildForest_map&, std::string const& item_dir,
		  std::set<std::string>& visiting,
		  FileLocation const& referrer,
		  std::string const& description);
    void getBackingAreaData(std::string const& top_path,
			    ItemConfig* config,
			    std::list<std::string>& backing_areas,
			    std::set<std::string>& deleted_trees,
			    std::set<std::string>& deleted_items);
    void traverseTree(BuildTree_map& buildtrees,
		      std::string const& top_path);
    void traverseItems(BuildTree_map& buildtrees,
		       std::string const& top_path);
    void resolveItems(BuildTree_map& buildtrees,
		      std::string const& top_path);
    void resolveTraits(BuildTree_map& buildtrees,
		       std::string const& top_path);
    void checkPlugins(BuildTree& tree_data,
		      BuildItem_map& builditems,
		      std::string const& top_path);
    bool isPluginAnywhere(std::string const& item_name);
    void checkPlatformTypes(BuildTree& tree_data,
			    BuildItem_map& builditems,
			    std::string const& top_path);
    void checkItemNames(BuildItem_map& builditems,
			std::string const& top_path);
    bool accessibleFrom(BuildItem_map& builditems,
			std::string const& accessor,
			std::string const& accessee);
    void checkBuildAlso(BuildItem_map& builditems,
			std::string const& top_path);
    void checkDependencies(BuildTree& tree_data,
			   BuildItem_map& builditems,
			   std::string const& top_path);
    void updatePlatformTypes(BuildTree& tree_data,
			     BuildItem_map& builditems,
			     std::string const& top_path);
    void checkDependencyPlatformTypes(BuildItem_map& builditems);
    void checkFlags(BuildItem_map& builditems);
    void checkTraits(BuildTree& tree_data,
		     BuildItem_map& builditems,
		     std::string const& top_path);
    void checkIntegrity(BuildTree_map& buildtrees,
			std::string const& top_path);
    void reportIntegrityErrors(BuildTree_map& buildtrees,
			       BuildItem_map& builditems,
			       std::string const& top_path);
    void computeBuildablePlatforms(BuildTree& tree_data,
				   BuildItem_map& builditems,
				   std::string const& top_path);
    BackingConfig* readBacking(std::string const& dir);
    bool haveExternal(BuildTree_map&, std::string const& backing_area,
		      ExternalData& external);
    void computeValidTraits(BuildForest_map& forests);
    void listTraits();
    void listPlatforms(BuildTree_map& buildtrees);
    void dumpData(BuildTree_map& buildtrees);
    void dumpPlatformData(PlatformData const&, std::string const& indent);
    void dumpBuildItem(BuildItem& item, std::string const& item_name,
		       std::map<std::string, int>& tree_numbers);
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

    // (tree, other_tree) -> [plugin, ...]
    typedef std::map<std::string,
		     std::vector<std::string> > map_string_vec_string;
    typedef std::map<std::string, map_string_vec_string> shadowed_plugin_map;
    shadowed_plugin_map shadowed_plugins;

    Logger& logger;
};

#endif // __ABUILD_HH__
