#include "Public.hh"
#include <Hidden.hh>

void
Public::performOperation()
{
    Hidden h;
    h.doSomething();
    h.doSomethingElse();
}
