#include <nt_TargetType.hh>
#include <Token.hh>

nt_TargetType::nt_TargetType(Token const* token) :
    NonTerminal(token->getLocation()),
    token(token)
{
}

nt_TargetType::~nt_TargetType()
{
}

Token const*
nt_TargetType::getToken() const
{
    return this->token;
}
