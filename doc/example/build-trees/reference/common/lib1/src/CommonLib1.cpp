#include "CommonLib1.hpp"

#include <iostream>

CommonLib1::CommonLib1(int n) :
    n(n)
{
}

void
CommonLib1::countBackwards()
{
    if (this->n >= 1)
    {
	std::cout << "counting backwards from " << this->n << ":";
	for (int i = this->n; i >= 1; --i)
	{
	    std::cout << " " << i;
	}
    }
    else
    {
	std::cout << "not counting backwards from " << this->n;
    }
    std::cout << std::endl;
}
