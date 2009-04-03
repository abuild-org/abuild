#ifndef __PLATFORMSELECTOR_HH__
#define __PLATFORMSELECTOR_HH__

#include <string>

class PlatformSelector
{
  public:
    static std::string const ANY;

    PlatformSelector();

    // Initializes the PlatformSelector from the given string,
    // returning true iff it is valid.
    bool initialize(std::string const& str);
    bool isSkip() const;
    bool isDefault() const;
    std::string const& getPlatformType() const;

    // Matcher(first_platform, platform_selector)(platform) returns
    // true iff platform matches the pattern in platform_selector.
    // Empty components in platform_selector are resolved from
    // first_platform.
    class Matcher
    {
      public:
	Matcher(std::string const& first_platform,
		PlatformSelector const& p);
	bool matches(std::string const&) const;
	static bool fieldsMatch(std::string const& dfault,
				std::string const& selector,
				std::string const& platform);

      private:
	std::string default_os;
	std::string default_cpu;
	std::string default_toolset;
	std::string default_compiler;
	std::string default_option;
	PlatformSelector const& p;
    };
    friend class Matcher;

  private:
    static void split_platform(std::string const& platform,
			       std::string& os,
			       std::string& cpu,
			       std::string& toolset,
			       std::string& compiler,
			       std::string& option);

    bool skip;
    bool dfault;
    std::string platform_type;
    std::string os;
    std::string cpu;
    std::string toolset;
    std::string compiler;
    std::string option;
};

#endif // __PLATFORMSELECTOR_HH__
