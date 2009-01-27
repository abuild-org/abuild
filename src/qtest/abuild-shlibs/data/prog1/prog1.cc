#include <Shared1.hh>
#include <iostream>

int main()
{
    std::cout << "prog1 calling shared1: ";
    Shared1::hello();
    return 0;
}
