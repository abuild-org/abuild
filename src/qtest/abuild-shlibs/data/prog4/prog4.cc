#include <Shared1.hh>
#include <Static.hh>
#include <iostream>

int main()
{
    std::cout << "prog4 calling shared1: ";
    Shared1::hello();
    std::cout << "prog4 calling static: ";
    Static::printString();
    return 0;
}
