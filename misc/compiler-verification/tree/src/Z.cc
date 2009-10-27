#include <Z.hh>
#include <Y.hh>
#include <iostream>

void Z::hello()
{
    Y::hello();
    std::cout << "Hello from Z" << std::endl;
}
