#include "Shared2.hh"
#include "Static.hh"
#include <iostream>

void
Shared2::hello()
{
    std::cout << "shared2 calling static: ";
    Static::printString();
}
