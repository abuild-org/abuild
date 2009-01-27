#include "Shared2.hh"
#include "Static.hh"
#include <iostream>

#ifdef _WIN32
__declspec(dllexport)
#endif
void
Shared2::hello()
{
    std::cout << "shared2 calling static: ";
    Static::printString();
}
