#include <lib.hh>
#include <iostream>

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
	int n = atoi(argv[i]);
	std::cout << n << "\t" << f(n) << std::endl;
    }
    return 0;
}
