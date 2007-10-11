#ifndef __NT_ARGUMENTS_HH__
#define __NT_ARGUMENTS_HH__

#include <NonTerminal.hh>
#include <vector>

class nt_Argument;

class nt_Arguments: public NonTerminal
{
  public:
    nt_Arguments(FileLocation const&);
    virtual ~nt_Arguments();
    void appendArgument(nt_Argument const* argument);

    std::vector<nt_Argument const*> const& getArguments() const;

  private:
    std::vector<nt_Argument const*> arguments;
};

#endif // __NT_ARGUMENTS_HH__
