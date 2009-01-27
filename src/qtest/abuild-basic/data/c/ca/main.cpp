#include "C1.hh"
#include "C2.hh"

int main()
{
    C1 c1(1);
    C2 c2(2);
    // suppress unused variable warnings in old versions of g++
    c1.touch();
    c2.touch();
    return 0;
}
