#include <nt_AfterBuild.hh>
#include <nt_Word.hh>

nt_AfterBuild::nt_AfterBuild(nt_Word const* argument) :
    NonTerminal(argument->getLocation()),
    argument(argument)
{
}

nt_AfterBuild::~nt_AfterBuild()
{
}

nt_Word const*
nt_AfterBuild::getArgument() const
{
    return this->argument;
}
