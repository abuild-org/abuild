#include <nt_AfterBuild.hh>
#include <nt_Words.hh>

nt_AfterBuild::nt_AfterBuild(nt_Words const* argument) :
    NonTerminal(argument->getLocation()),
    argument(argument)
{
}

nt_AfterBuild::~nt_AfterBuild()
{
}

nt_Words const*
nt_AfterBuild::getArgument() const
{
    return this->argument;
}
