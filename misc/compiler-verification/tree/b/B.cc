#include <B.hh>
#include <iostream>

#ifdef _WIN32
__declspec(dllexport)
#endif
void B::hello()
{
    std::cout << "Hello from B" << std::endl;
}
