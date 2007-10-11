#include <iostream>
#include <Util.hh>

int main(int argc, char* argv[])
{
    std::string result;
    if (Util::getProgramFullPath(argv[0], result))
    {
	std::cout << argv[0] << " -> " << result << std::endl;
    }
    else
    {
	std::cout << argv[0] << " not found" << std::endl;
    }
    return 0;
}
