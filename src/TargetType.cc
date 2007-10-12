#include <TargetType.hh>
#include <assert.h>

std::map<TargetType::target_type_e, std::string> TargetType::target_type_names;
std::map<std::string, TargetType::target_type_e> TargetType::target_type_ids;

// Declare after all other statics.
bool TargetType::statics_initialized = initializeStatics();

bool
TargetType::initializeStatics()
{
    if (statics_initialized)
    {
	return true;
    }

    target_type_names[tt_unknown] = "unknown";
    target_type_names[tt_all] = "all";
    target_type_names[tt_platform_independent] = "platform-independent";
    target_type_names[tt_object_code] = "object-code";
    target_type_names[tt_java] = "java";

    for (std::map<target_type_e, std::string>::iterator iter =
	     target_type_names.begin();
	 iter != target_type_names.end(); ++iter)
    {
	target_type_ids[(*iter).second] = (*iter).first;
    }

    return true;
}

std::string const&
TargetType::getName(target_type_e target_type)
{
    assert(target_type_names.count(target_type) > 0);
    return target_type_names[target_type];
}

TargetType::target_type_e
TargetType::getID(std::string const& target_type)
{
    assert(target_type_ids.count(target_type) > 0);
    return target_type_ids[target_type];
}

bool
TargetType::isValid(std::string const& target_type)
{
    return ((target_type_ids.count(target_type) > 0) &&
	    (target_type_ids[target_type] != tt_unknown));
}
