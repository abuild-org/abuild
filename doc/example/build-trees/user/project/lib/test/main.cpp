#include <ProjectLib.hpp>
#include <ProjectLib_private.hpp>

int main()
{
    ProjectLib_private_set_value(8);
    ProjectLib p;
    p.hello();
    return 0;
}
