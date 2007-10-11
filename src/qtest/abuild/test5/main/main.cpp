#include "File1.hpp"
#include <iostream>

int main()
{
    File1 f1;
    f1.touch(); // suppress unused variable warning from old g++
    return 0;
}
