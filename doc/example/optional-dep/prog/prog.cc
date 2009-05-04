#ifdef HAVE_XDRIVER
# include <xdriver.hh>
#endif

#include <iostream>

int main()
{
    std::cout << 3 << " = " << 3 << std::endl;
#ifdef HAVE_XDRIVER
    std::cout << "xdriver(3) = " << xdriver(3) << std::endl;
#else
    std::cout << "xdriver not available" << std::endl;
#endif
    return 0;
}
