#include <NonTerminal.hh>

NonTerminal::NonTerminal()
{
}

NonTerminal::NonTerminal(FileLocation const& l) :
    location(l)
{
}

NonTerminal::~NonTerminal()
{
}

FileLocation const&
NonTerminal::getLocation() const
{
    return this->location;
}
