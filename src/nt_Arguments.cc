#include <nt_Arguments.hh>

#include <nt_Argument.hh>

nt_Arguments::nt_Arguments(FileLocation const& location) :
    NonTerminal(location)
{
}

nt_Arguments::~nt_Arguments()
{
}

void
nt_Arguments::appendArgument(nt_Argument const* argument)
{
    this->arguments.push_back(argument);
}

std::vector<nt_Argument const*> const&
nt_Arguments::getArguments() const
{
    return this->arguments;
}
