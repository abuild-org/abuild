#ifndef __NT_IFCLAUSE_HH__
#define __NT_IFCLAUSE_HH__

#include <NonTerminal.hh>

class nt_Conditional;
class nt_Blocks;

class nt_IfClause: public NonTerminal
{
  public:
    nt_IfClause(nt_Conditional const*, nt_Blocks const*,
		bool conditional_expected);
    virtual ~nt_IfClause();

    bool getConditionalOkay() const;
    nt_Conditional const* getConditional() const;
    nt_Blocks const* getBlocks() const;

  private:
    bool conditional_okay;
    nt_Conditional const* conditional;
    nt_Blocks const* blocks;
};

#endif // __NT_IFCLAUSE_HH__
