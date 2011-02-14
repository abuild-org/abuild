#ifndef __PLATFORMDATA_HH__
#define __PLATFORMDATA_HH__

#include <TargetType.hh>
#include <string>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <DependencyGraph.hh>
#include <PlatformSelector.hh>

class PlatformData
{
  public:
    static std::string const PLATFORM_INDEP;
    static std::string const PLATFORM_JAVA;
    static std::string const pt_INDEP;
    static std::string const pt_NATIVE;
    static std::string const pt_JAVA;

    PlatformData();

    void addPlatformType(std::string const& platform_type,
			 TargetType::target_type_e target_type,
			 std::string const& parent_platform_type = "");
    void addPlatform(std::string const& platform,
		     std::string const& platform_type,
		     bool lowpri);
    // check() must be called after adding platforms and platform
    // types and before calling any of the querying methods.  If a
    // given platform selector from selectors is used and the key
    // appears in unused_selectors, the corresponding entry will be
    // removed from unused_selectors.  This way we can keep a running
    // tab on whether any platform selectors are unused across all
    // trees.
    void check(std::map<std::string, PlatformSelector> const& selectors,
	       std::map<std::string, std::string>& unused_selectors);

    // check() must have been called.
    bool isPlatformTypeValid(std::string const&) const;

    // Get the platform type for a platform
    std::string const& getPlatformType(std::string const&) const;

    // Get the target type for a platform type
    TargetType::target_type_e getTargetType(std::string const&) const;

    // Return, in descending priority order (best first), the list of
    // platforms that belong to a given platform type.  The boolean
    // value indicates whether or not the platform is selected for
    // build by default.
    typedef std::pair<std::string, bool> selected_platform_t;
    typedef std::vector<selected_platform_t> selected_platforms_t;
    selected_platforms_t const& getPlatformsByType(
	std::string const& platform_type) const;

    // Return a list of platform types that this platform type may
    // depend on in order of search preference
    std::vector<std::string> const& getCompatiblePlatformTypes(
	std::string const& platform_type) const;

    // Return a list of platform types for a given target type
    std::set<std::string> getPlatformTypes(TargetType::target_type_e) const;

  private:
    static std::string const PLATFORM_PREFIX;
    static std::string const PLATFORM_TYPE_PREFIX;

    // Returns true iff platform p1 was declared later than p2.  Using
    // this as a sorting predicate for std::sort will result in a list
    // sorted with the last declared platform first.
    bool declarationOrder(selected_platform_t const& p1,
			  selected_platform_t const& p2) const;

    bool initializing;
    bool checked;

    // platform type -> target type
    std::map<std::string, TargetType::target_type_e> target_types;

    // platform -> platform type
    std::map<std::string, std::string> platform_types;

    // platforms_by_type includes all platforms that belong to a given
    // platform type.  They are sorted in priority order with the
    // first element of the list being the one with the highest
    // priority.
    std::map<std::string, selected_platforms_t> platforms_by_type;

    // compatible_platform_types maps each platform type to a list of
    // compatible platform types in order of search priority.
    std::map<std::string, std::vector<std::string> > compatible_platform_types;

    // The platform_declaration map maps each platform to a number
    // representing the sequence in which it was declared.  Since
    // later declarations take priority over earlier ones, higher
    // numbers therefore have higher priority for platform selection.
    std::map<std::string, int> platform_declaration;

    // We use a dependency graph to handle nested platform types.
    // Every platform and platform type is an item in the graph.
    // Every platform item has no dependencies.  Every platform type
    // item depends on its parent type and its platforms.  Platform
    // types are prepended with PLATFORM_TYPE_PREFIX and platforms
    // with PLATFORM_PREFIX to distinguish them in the graph.  The
    // prefix strings are set so that the error messages generated by
    // check() are more readable.  Using this approach, the direct
    // dependencies of a platform type that are platform items are the
    // platforms in that type, and the recursively expanded
    // dependencies that are platform type items are those that items
    // of this platform type may depend on.
    DependencyGraph platform_graph;
};


#endif // __PLATFORMDATA_HH__
