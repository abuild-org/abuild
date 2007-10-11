#include "thing1.hh"

#include <iostream>

static Thing1* static_thing = new Thing1;

Thing1::Thing1()
{
    std::cout << "in thing1 constructor" << std::endl;
}

Thing1::~Thing1()
{
    std::cout << "in thing1 destructor" << std::endl;
}
