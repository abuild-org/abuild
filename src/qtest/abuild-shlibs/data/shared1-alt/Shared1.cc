#include "Shared1.hh"
#include "Static.hh"
#include <iostream>

void
Shared1::hello()
{
    std::cout << "shared1-alt calling static: ";
    Static::printString();
}
