#include "CommonLib3.hpp"

#include <iostream>

CommonLib3::CommonLib3(int n) :
    n(n)
{
}

CommonLib3::~CommonLib3()
{
}

void
CommonLib3::count()
{
    if (this->n >= 1)
    {
	std::cout << "counting to " << this->n << ":";
	for (int i = 1; i <= this->n; ++i)
	{
	    std::cout << " " << i;
	}
    }
    else
    {
	std::cout << "not counting to " << this->n;
    }
    std::cout << std::endl;
}

void
CommonLib3::talkAbout()
{
    std::cout << "They say that " << this->n << " is an interesting number."
	      << std::endl;
}

int
CommonLib3::getN()
{
    return this->n;
}
