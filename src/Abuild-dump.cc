// Methods involved with dump-data

#include <Abuild.hh>
#include <QTC.hh>
#include <QEXC.hh>
#include <Util.hh>
#include <Logger.hh>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <cstdio>
#include <assert.h>

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
	std::set<std::string> const& global_plugins =
	    forest.getGlobalPlugins();

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
	if (! global_plugins.empty())
	{
	    o << "  <global-plugins>" << std::endl;
	    for (std::set<std::string>::const_iterator iter =
		     global_plugins.begin();
		 iter != global_plugins.end(); ++iter)
	    {
		o << "   <plugin name=\"" + *iter + "\"/>"
		  << std::endl;
	    }
	    o << "  </global-plugins>" << std::endl;
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
    // XXX parent information
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
    std::set<std::string> const& omitted_deps = tree.getOmittedTreeDeps();

    assert(forest_numbers.count(tree.getForestRoot()));

    o << "  <build-tree" << std::endl
      << "   name=\"" << tree_name << "\"" << std::endl
      << "   absolute-path=\"" << root_path << "\"" << std::endl
      << "   home-forest=\"f-"
      << forest_numbers[tree.getForestRoot()] << "\"" << std::endl
      << "   backing-depth=\"" << tree.getBackingDepth() << "\""
      << std::endl
      << "  >" << std::endl;
    dumpPlatformData(*(tree.getPlatformData()), "   ");
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
    if (! omitted_deps.empty())
    {
	o << "   <omitted-tree-dependencies>" << std::endl;
	for (std::set<std::string>::const_iterator iter = omitted_deps.begin();
	     iter != omitted_deps.end(); ++iter)
	{
	    o << "    <tree-dependency name=\"" << *iter << "\"/>" << std::endl;
	}
	o << "   </omitted-tree-dependencies>" << std::endl;
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
    std::list<ItemConfig::BuildAlso> const& build_also =
	item.getBuildAlso();
    std::list<std::string> const& declared_dependencies =
	item.getDeps();
    std::list<std::string> const& expanded_dependencies =
	item.getExpandedDependencies();
    std::set<std::string> omitted_dependencies;
    std::set<std::string> const& platform_types =
	item.getPlatformTypes();
    std::set<std::string> const& buildable_platforms =
	item.getBuildablePlatforms();
    std::set<std::string> const& supported_flags =
	item.getSupportedFlags();
    TraitData::trait_data_t const& traits =
	item.getTraitData().getTraitData();

    std::map<std::string, bool> const& optional_dep_presence =
	item.getOptionalDependencyPresence();
    for (std::map<std::string, bool>::const_iterator iter =
	     optional_dep_presence.begin();
	 iter != optional_dep_presence.end(); ++iter)
    {
	if (! (*iter).second)
	{
	    omitted_dependencies.insert((*iter).first);
	}
    }

    bool any_subelements =
	(! (build_also.empty() &&
	    declared_dependencies.empty() &&
	    expanded_dependencies.empty() &&
	    omitted_dependencies.empty() &&
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
    if (item.isSerial())
    {
	o << "    serial=\"1\"" << std::endl;
    }
    if (any_subelements)
    {
	FlagData const& flag_data = item.getFlagData();
	o << "   >" << std::endl;
	if (! build_also.empty())
	{
	    std::list<ItemConfig::BuildAlso> build_also_items;
	    std::list<ItemConfig::BuildAlso> build_also_trees;
	    for (std::list<ItemConfig::BuildAlso>::const_iterator biter =
		     build_also.begin();
		 biter != build_also.end(); ++biter)
	    {
		if ((*biter).isTree())
		{
		    build_also_trees.push_back(*biter);
		}
		else
		{
		    build_also_items.push_back(*biter);
		}
	    }
	    dumpBuildAlso(true, build_also_trees);
	    dumpBuildAlso(false, build_also_items);
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
	if (! omitted_dependencies.empty())
	{
	    o << "    <omitted-dependencies>" << std::endl;
	    for (std::set<std::string>::const_iterator dep_iter =
		     omitted_dependencies.begin();
		 dep_iter != omitted_dependencies.end(); ++dep_iter)
	    {
		o << "     <dependency name=\"" << *dep_iter << "\"/>"
		  << std::endl;
	    }
	    o << "    </omitted-dependencies>" << std::endl;
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
Abuild::dumpBuildAlso(bool trees,
		      std::list<ItemConfig::BuildAlso> const& data)
{
    if (data.empty())
    {
	return;
    }

    std::ostream& o = std::cout;
    char const* tag = (trees ? "build-also-trees" : "build-also-items");
    o << "    <" << tag << ">" << std::endl;
    for (std::list<ItemConfig::BuildAlso>::const_iterator biter = data.begin();
	 biter != data.end(); ++biter)
    {
	std::string name;
	bool is_tree;
	bool desc;
	bool with_tree_deps;
	(*biter).getDetails(name, is_tree, desc, with_tree_deps);
	assert(is_tree == trees);
	o << "     <build-also name=\"" << name << "\"";
	if (! (is_tree || desc || with_tree_deps))
	{
	    // Add new attributes to condition here and to else block
	    // below.
	    QTC::TC("abuild", "Abuild-dump build-also just item");
	}
	else
	{
	    // Having these conditions in an else block helps ensure
	    // that above no-attribute condition doesn't accidentally
	    // become true if we add more attributes in the future.
	    if (is_tree)
	    {
		// For backward compatibility, only output is-tree
		// attribute if true -- see comments in abuild_data.dtd.
		o << " is-tree=\"1\"";
		QTC::TC("abuild", "Abuild-dump build-also is-tree");
	    }
	    if (desc)
	    {
		o << " desc=\"1\"";
		QTC::TC("abuild", "Abuild-dump build-also desc");
	    }
	    if (with_tree_deps)
	    {
		o << " with-tree-deps=\"1\"";
		QTC::TC("abuild", "Abuild-dump build-also with-tree-deps");
	    }
	}
	o << "/>" << std::endl;
    }
    o << "    </" << tag << ">" << std::endl;
}
