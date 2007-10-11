#include "ProjectLib.hpp"
#include "ProjectLib_private.hpp"

#include <iostream>

ProjectLib::ProjectLib() :
    cl1(ProjectLib_private_get_value())
{
}

void
ProjectLib::hello()
{
    this->cl1.countBackwards();
}
