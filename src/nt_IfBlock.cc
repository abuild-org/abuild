#include <nt_IfBlock.hh>

#include <nt_IfClause.hh>
#include <nt_IfClauses.hh>

nt_IfBlock::nt_IfBlock(nt_IfClause const* ifclause,
		       nt_IfClauses const* elseifs,
		       nt_IfClause const* else_clause) :
    NonTerminal(ifclause->getLocation())
{
    this->clauses.push_back(ifclause);
    if (elseifs)
    {
	std::list<nt_IfClause*> const& elseif_clauses = elseifs->getClauses();
	for (std::list<nt_IfClause*>::const_iterator iter =
		 elseif_clauses.begin();
	     iter != elseif_clauses.end(); ++iter)
	{
	    this->clauses.push_back(*iter);
	}
    }
    if (else_clause)
    {
	this->clauses.push_back(else_clause);
    }
}

nt_IfBlock::~nt_IfBlock()
{
}

std::vector<nt_IfClause const*> const&
nt_IfBlock::getClauses() const
{
    return this->clauses;
}
