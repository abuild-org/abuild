#include <ProjectLib.hpp>
#include <CommonLib2.hpp>
#include <iostream>
#include <world_peace.hh>
#include "auto.h"

int main(int argc, char* argv[])
{
    std::cout << "This is derived-main." << std::endl;
    ProjectLib l;
    l.hello();
    CommonLib2 cl2(6);
    cl2.talkAbout();
    cl2.count();
    std::cout << "Number is " << getNumber() << "." << std::endl;
    // We don't have to know or care whether this is the stub
    // implementation or the real implementation.
    create_world_peace();
    return 0;
}
