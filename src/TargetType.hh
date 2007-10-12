#ifndef __TARGETTYPE_HH__
#define __TARGETTYPE_HH__

#include <string>
#include <map>

class TargetType
{
  public:
    enum target_type_e
    {
	tt_unknown,
	tt_all,
	tt_platform_independent,
	tt_object_code,
	tt_java,
    };

    static std::string const& getName(target_type_e);
    static target_type_e getID(std::string const&);
    static bool isValid(std::string const&);

  private:
    static bool initializeStatics();

    static bool statics_initialized;
    static std::map<target_type_e, std::string> target_type_names;
    static std::map<std::string, target_type_e> target_type_ids;
};

#endif // __TARGETTYPE_HH__
