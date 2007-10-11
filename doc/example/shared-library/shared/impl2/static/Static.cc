#include "Static.hh"
#include <iostream>

void
Static::hello()
{
    std::cout << "This is a private static library inside implementation 2."
	      << std::endl;
}
