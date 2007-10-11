#include "Token.hh"
#include <iostream>

Token::Token(std::string const& val, FileLocation const& location) :
    val(val),
    location(location)
{
}

std::string const&
Token::getValue() const
{
    return this->val;
}

FileLocation const&
Token::getLocation() const
{
    return this->location;
}
