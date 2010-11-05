#include <iostream>
#include <cstring>

int main(int argc, char* argv[])
{
    if ((argc == 2) && (std::strcmp(argv[1], "ver") == 0))
    {
	std::cout << _MSC_VER << std::endl;
	return 0;
    }

    try
    {
#ifdef _MANAGED
	std::cout << "managed" << std::endl;
#else
	std::cout << "not managed" << std::endl;
#endif
	throw 5;
    }
    catch (int x)
    {
	std::cout << "threw " << x << std::endl;
    }
    return 0;
}
