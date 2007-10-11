#include <Util.hh>
#include <iostream>

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
	std::string canonical = Util::canonicalizePath(argv[i]);
	std::cout << argv[i] << " -> " << canonical << std::endl;
    }
    return 0;
}
