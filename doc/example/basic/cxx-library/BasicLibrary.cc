#include "BasicLibrary.hh"
#include <iostream>

BasicLibrary::BasicLibrary(int n) :
    n(n)
{
}

void
BasicLibrary::hello()
{
    std::cout << "Hello.  This is BasicLibrary(" << n << ")." << std::endl;
}
