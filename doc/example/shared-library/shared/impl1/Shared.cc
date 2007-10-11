#include <Shared.hh>
#include <iostream>

#ifdef _WIN32
__declspec(dllexport)
#endif
void
Shared::hello()
{
    std::cout << "This is Shared implementation 1." << std::endl;
}
