#include <Z.hh>
#include <Y.hh>
#include <iostream>

#ifdef _WIN32
__declspec(dllexport)
#endif
void Z::hello()
{
    Y::hello();
    std::cout << "Hello from Z" << std::endl;
}
