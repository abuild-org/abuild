#include "InterfaceLexer.hh"

int yy_interfacewrap()
{
    // Define yywrap function that returns 1.  Some versions of flex
    // have a problem with %option noyywrap and %option prefix being
    // combined, so this is a safer way to deal with it.
    return 1;
}
