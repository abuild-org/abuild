#include "ProjectLib.hpp"

#include <iostream>

ProjectLib::ProjectLib(int n) :
    cl1(n)
{
}

void
ProjectLib::hello()
{
    this->cl1.countBackwards();
}
