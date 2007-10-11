#include <A.hh>
#include <B.hh>
#include <X.hh>
#include <Z.hh>
#include <verify-config.h>
#include <iostream>

int main()
{
#ifdef HAVE_PRINTF
    std::cout << "have printf: " << D1 << std::endl;
#endif
#ifdef HAVE_EXCEPTION
    std::cout << "have exception: " << D2 << std::endl;
#endif
    A::hello();
    B::hello();
    X::hello();
    Z::hello();
    return 0;
}
