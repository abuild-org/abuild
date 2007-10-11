#ifndef __NT_ARGUMENT_HH__
#define __NT_ARGUMENT_HH__

#include <NonTerminal.hh>

class nt_Function;
class nt_Words;

class nt_Argument: public NonTerminal
{
  public:
    nt_Argument(nt_Function const*);
    nt_Argument(nt_Words const*);
    virtual ~nt_Argument();

    nt_Function const* getFunction() const;
    nt_Words const* getWords() const;

  private:
    // Only one of these will be defined
    nt_Function const* function;
    nt_Words const* words;
};

#endif // __NT_ARGUMENT_HH__
