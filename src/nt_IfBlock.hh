#ifndef __NT_IFBLOCK_HH__
#define __NT_IFBLOCK_HH__

#include <NonTerminal.hh>
#include <vector>

class nt_IfClause;
class nt_IfClauses;

class nt_IfBlock: public NonTerminal
{
  public:
    nt_IfBlock(nt_IfClause const* ifclause,
	       nt_IfClauses const* elseifs,
	       nt_IfClause const* else_clause);
    virtual ~nt_IfBlock();

    std::vector<nt_IfClause const*> const& getClauses() const;

  private:
    std::vector<nt_IfClause const*> clauses;
};

#endif // __NT_IFBLOCK_HH__
