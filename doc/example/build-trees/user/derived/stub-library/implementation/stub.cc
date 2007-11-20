#include <world_peace.hh>
#include <stdio.h>

// Provide a stub version of create_world_peace if we don't have one.

#ifndef HAVE_CREATE_WORLD_PEACE
void create_world_peace()
{
    // Silly example: make this conditional upon whether we have
    // printf.  This is just to illustrate a case that's true as well
    // as a case that's false.
#ifdef HAVE_PRINTF
    printf("I don't know how to create world peace.\n");
    printf("How about visualizing whirled peas?\n");
#else
# error "Can't do this without printf."
#endif
}
#endif
