
#include <CUPublic.hpp>

#include <iostream>
#include "CUPrivate.hpp"

CUPublic::CUPublic()
{
    CUPrivate p;
    p.touch(); // suppress unused variable warning from old g++
    std::cout << "CUPublic" << std::endl;
}
