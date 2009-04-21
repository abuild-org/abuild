#ifndef __FLEXCALLER_HH__
#define __FLEXCALLER_HH__

#include <boost/function.hpp>
#include <stdio.h>

class Parser;

// This class is a stateless proxy used by the Parser class to call
// methods defined in the individual scanners.  The job of creating a
// more object-oriented interface (encapsulating init and destroy, for
// example) is handled in the Parser class.

class FlexCaller
{
    friend class Parser;

  public:
    typedef boost::function<int(Parser*, void**)> init_extra_fn;
    typedef boost::function<void(FILE*, void*)> set_in_fn;
    typedef boost::function<int(void*)> lex_fn;
    typedef boost::function<int(void*)> lex_destroy_fn;

    FlexCaller(init_extra_fn init_extra, set_in_fn set_in,
	       lex_fn lex, lex_destroy_fn lex_destroy) :
	init_extra(init_extra),
	set_in(set_in),
	lex(lex),
	lex_destroy(lex_destroy)
    {
    }

  private:
    init_extra_fn init_extra;
    set_in_fn set_in;
    lex_fn lex;
    lex_destroy_fn lex_destroy;
};

#endif // __FLEXCALLER_HH__
