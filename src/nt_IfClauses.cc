#include <nt_IfClauses.hh>

#include <nt_IfClause.hh>

nt_IfClauses::nt_IfClauses(FileLocation const& location) :
    NonTerminal(location)
{
}

nt_IfClauses::~nt_IfClauses()
{
}

void
nt_IfClauses::addClause(nt_IfClause* clause)
{
    this->clauses.push_back(clause);
}

std::list<nt_IfClause*> const&
nt_IfClauses::getClauses() const
{
    return this->clauses;
}
