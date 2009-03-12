#include <nt_Declaration.hh>

#include <Token.hh>
#include <nt_TypeSpec.hh>

nt_Declaration::nt_Declaration(Token const* identifier,
			       nt_TypeSpec const* typespec) :
    NonTerminal(identifier->getLocation()),
    variable_name(identifier->getValue()),
    scope(typespec->getScope()),
    type(typespec->getType()),
    list_type(typespec->getListType()),
    initializer(0)
{
}

nt_Declaration::~nt_Declaration()
{
}

void
nt_Declaration::addInitializer(nt_Words const* words)
{
    this->initializer = words;
}

nt_Words const*
nt_Declaration::getInitializer() const
{
    return this->initializer;
}

std::string const&
nt_Declaration::getVariableName() const
{
    return this->variable_name;
}

Interface::scope_e
nt_Declaration::getScope() const
{
    return this->scope;
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
