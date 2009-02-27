#include <nt_Declaration.hh>

#include <Token.hh>
#include <nt_TypeSpec.hh>

nt_Declaration::nt_Declaration(Token const* identifier,
			       nt_TypeSpec const* typespec) :
    NonTerminal(identifier->getLocation()),
    variable_name(identifier->getValue()),
    type(typespec->getType()),
    list_type(typespec->getListType()),
    recursive(typespec->getRecursive())
{
}

nt_Declaration::~nt_Declaration()
{
}

std::string const&
nt_Declaration::getVariableName() const
{
    return this->variable_name;
}

Interface::type_e
nt_Declaration::getType() const
{
    return this->type;
}

Interface::list_e
nt_Declaration::getListType() const
{
    return this->list_type;
}

bool
nt_Declaration::getRecursive() const
{
    return this->recursive;
}
