#include "C2.hh"
#include "C1.hh"
#include <iostream>

C2::C2(int a)
{
    std::cout << "C2: " << a << std::endl;
    C1 c1(100 + a);
    c1.touch(); // suppress unused variable warning from old g++ versions
}
