#include <nt_Argument.hh>
#include <nt_Function.hh>
#include <nt_Words.hh>

nt_Argument::nt_Argument(nt_Function const* function) :
    NonTerminal(function->getLocation()),
    function(function),
    words(0)
{
}

nt_Argument::nt_Argument(nt_Words const* words) :
    NonTerminal(words->getLocation()),
    function(0),
    words(words)
{
}

nt_Argument::~nt_Argument()
{
}

nt_Function const*
nt_Argument::getFunction() const
{
    return this->function;
}

nt_Words const*
nt_Argument::getWords() const
{
    return this->words;
}
