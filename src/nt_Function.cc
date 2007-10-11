#include <nt_Function.hh>

#include <Token.hh>
#include <nt_Arguments.hh>

nt_Function::nt_Function(Token const* function, nt_Arguments const* arguments) :
    NonTerminal(function->getLocation()),
    function(function),
    arguments(arguments->getArguments())
{
}

nt_Function::~nt_Function()
{
}

Token const*
nt_Function::getFunction() const
{
    return this->function;
}

std::vector<nt_Argument const*> const&
nt_Function::getArguments() const
{
    return this->arguments;
}
