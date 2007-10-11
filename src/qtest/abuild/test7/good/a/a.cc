#include "a.hh"
#include <iostream>
void a(bool val)
{
    std::cout << "a" << std::endl;
    if (val)
    {
	b();
    }
}
