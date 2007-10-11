#include "Shared1.hh"
#include "Static.hh"
#include <iostream>

#ifdef _WIN32
__declspec(dllexport)
#endif
void
Shared1::hello()
{
    std::cout << "shared1 calling static: ";
    Static::printString();
}
