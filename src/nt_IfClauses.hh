#ifndef __NT_IFCLAUSES_HH__
#define __NT_IFCLAUSES_HH__

#include <NonTerminal.hh>
#include <list>

class nt_IfClause;

class nt_IfClauses: public NonTerminal
{
  public:
    nt_IfClauses(FileLocation const&);
    virtual ~nt_IfClauses();
    void addClause(nt_IfClause* clause);
    std::list<nt_IfClause*> const& getClauses() const;

  private:
    std::list<nt_IfClause*> clauses;
};

#endif // __NT_IFCLAUSES_HH__
