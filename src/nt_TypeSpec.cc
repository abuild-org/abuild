#include <nt_TypeSpec.hh>

nt_TypeSpec::nt_TypeSpec(FileLocation const& location,
			 Interface::type_e type) :
    NonTerminal(location),
    type(type),
    list_type(Interface::l_scalar),
    recursive(true)
{
}

nt_TypeSpec::~nt_TypeSpec()
{
}

void
nt_TypeSpec::setListType(Interface::list_e list_type)
{
    this->list_type = list_type;
}

void
nt_TypeSpec::setNonRecursive()
{
    this->recursive = false;
}

Interface::type_e
nt_TypeSpec::getType() const
{
    return this->type;
}

Interface::list_e
nt_TypeSpec::getListType() const
{
    return this->list_type;
}

bool
nt_TypeSpec::getRecursive() const
{
    return this->recursive;
}
