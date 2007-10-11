#ifndef __NONTERMINAL_HH__
#define __NONTERMINAL_HH__

#include <FileLocation.hh>

// This is a convenient base class for all non-terminals that any of
// our grammars use.  The Parser class uses this to enable dynamically
// non-terminals to be deleted at the end of each parse.  By
// convention, classes derived from NonTerminal start with nt_.
class NonTerminal
{
  public:
    NonTerminal();
    NonTerminal(FileLocation const& l);
    virtual ~NonTerminal();
    FileLocation const& getLocation() const;

  protected:
    FileLocation location;
};

#endif // __NONTERMINAL_HH__
