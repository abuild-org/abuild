#include <ProjectLib.hpp>
#include <CommonLib2.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "This is derived-main." << std::endl;
    ProjectLib l;
    l.hello();
    CommonLib2 cl2(6);
    cl2.talkAbout();
    cl2.count();
    return 0;
}
