#include <nt_Conditional.hh>
#include <assert.h>
#include <Token.hh>
#include <nt_Function.hh>

nt_Conditional::nt_Conditional(Token const* variable) :
    NonTerminal(variable->getLocation()),
    variable(variable),
    function(0)
{
}

nt_Conditional::nt_Conditional(nt_Function const* function) :
    NonTerminal(function->getLocation()),
    variable(0),
    function(function)
{
}

nt_Conditional::~nt_Conditional()
{
}

Token const*
nt_Conditional::getVariable() const
{
    return this->variable;
}

nt_Function const*
nt_Conditional::getFunction() const
{
    return this->function;
}
