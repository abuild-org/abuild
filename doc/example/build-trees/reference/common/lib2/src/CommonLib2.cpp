#include "CommonLib2.hpp"

#include <iostream>

CommonLib2::CommonLib2(int n) :
    CommonLib3(n)
{
}

CommonLib2::~CommonLib2()
{
}

void
CommonLib2::talkAbout()
{
    std::cout << "What do you find interesting about " << this->getN() << "?"
	      << std::endl;
}
