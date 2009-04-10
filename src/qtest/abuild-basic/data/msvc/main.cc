#include <iostream>

int main()
{
    try
    {
	throw 5;
    }
    catch (int x)
    {
	std::cout << "threw " << x << std::endl;
    }
    return 0;
}
