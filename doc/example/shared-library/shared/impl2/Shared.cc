#include <Shared.hh>
#include <Static.hh>
#include <iostream>

#ifdef _WIN32
__declspec(dllexport)
#endif
void
Shared::hello()
{
    std::cout << "This is Shared implementation 2." << std::endl;
    Static::hello();
}
