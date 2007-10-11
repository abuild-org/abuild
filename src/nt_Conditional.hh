#ifndef __NT_CONDITIONAL_HH__
#define __NT_CONDITIONAL_HH__

#include <NonTerminal.hh>

class Token;
class nt_Function;

class nt_Conditional: public NonTerminal
{
  public:
    nt_Conditional(Token const*);
    nt_Conditional(nt_Function const*);
    virtual ~nt_Conditional();

    Token const* getVariable() const;
    nt_Function const* getFunction() const;

  private:
    // Only one of these will be defined
    Token const* variable;
    nt_Function const* function;
};

#endif // __NT_CONDITIONAL_HH__
