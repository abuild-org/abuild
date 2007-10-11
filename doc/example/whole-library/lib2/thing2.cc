#include "thing2.hh"

#include <iostream>

static Thing2* static_thing = new Thing2;

Thing2::Thing2()
{
    std::cout << "in thing2 constructor" << std::endl;
}

Thing2::~Thing2()
{
    std::cout << "in thing2 destructor" << std::endl;
}
