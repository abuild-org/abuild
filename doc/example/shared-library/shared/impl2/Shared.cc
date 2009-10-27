#include <Shared.hh>
#include <Static.hh>
#include <iostream>

void
Shared::hello()
{
    std::cout << "This is Shared implementation 2." << std::endl;
    Static::hello();
}
