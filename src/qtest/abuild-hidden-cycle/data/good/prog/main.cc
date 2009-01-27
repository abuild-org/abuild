#include "a.hh"
#include "b.hh"

int main()
{
    a(true);			// a -> b -> a
    b();			// b -> a
}
