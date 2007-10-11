#include <nt_Reset.hh>
#include <Token.hh>

nt_Reset::nt_Reset(Token const* identifier, bool negate) :
    NonTerminal(identifier->getLocation()),
    identifier(identifier->getValue()),
    negate(negate)
{
}

nt_Reset::nt_Reset(FileLocation const& l) :
    NonTerminal(l),
    negate(false)
{
}

nt_Reset::~nt_Reset()
{
}

bool
nt_Reset::isNegate() const
{
    return this->negate;
}

std::string const&
nt_Reset::getIdentifier() const
{
    return this->identifier;
}
