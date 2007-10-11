#ifndef __NT_FUNCTION_HH__
#define __NT_FUNCTION_HH__

#include <NonTerminal.hh>

#include <vector>

class Token;
class nt_Argument;
class nt_Arguments;

class nt_Function: public NonTerminal
{
  public:
    nt_Function(Token const* function, nt_Arguments const* arguments);
    virtual ~nt_Function();

    Token const* getFunction() const;
    std::vector<nt_Argument const*> const& getArguments() const;

  private:
    Token const* function;
    std::vector<nt_Argument const*> arguments;
};

#endif // __NT_FUNCTION_HH__
