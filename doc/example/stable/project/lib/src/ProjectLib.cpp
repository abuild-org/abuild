#include "ProjectLib.hpp"

#include <iostream>

ProjectLib::ProjectLib() :
    cl1(5)
{
}

void
ProjectLib::hello()
{
    this->cl1.countBackwards();
}
