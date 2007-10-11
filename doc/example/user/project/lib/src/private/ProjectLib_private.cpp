#include <ProjectLib_private.hpp>

static int value = 5;

int ProjectLib_private_get_value()
{
    return value;
}

void ProjectLib_private_set_value(int v)
{
    value = v;
}
