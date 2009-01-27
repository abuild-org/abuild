#include <Shared1.hh>
#include <Shared2.hh>
#include <iostream>

int main()
{
    std::cout << "prog3 calling shared1: ";
    Shared1::hello();
    std::cout << "prog3 calling shared2: ";
    Shared2::hello();
    return 0;
}
