#include <Z.hh>
#include <Y.hh>
#include <iostream>

void Z1::hello()
{
    Y::hello();
    std::cout << "Hello from Z1" << std::endl;
}
