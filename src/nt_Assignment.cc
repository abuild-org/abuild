#include <nt_Assignment.hh>
#include <Token.hh>
#include <nt_Words.hh>

nt_Assignment::nt_Assignment(Token const* identifier, nt_Words const* words) :
    NonTerminal(identifier->getLocation()),
    identifier(identifier->getValue()),
    flag(0),
    words(words),
    assignment_type(Interface::a_normal)
{
}

nt_Assignment::~nt_Assignment()
{
}

void
nt_Assignment::setAssignmentType(Interface::assign_e assignment_type)
{
    this->assignment_type = assignment_type;
}

void
nt_Assignment::setFlag(Token* flag)
{
    this->flag = flag;
}

std::string const&
nt_Assignment::getIdentifier() const
{
    return this->identifier;
}

nt_Words const*
nt_Assignment::getWords() const
{
    return this->words;
}

Interface::assign_e
nt_Assignment::getAssignmentType() const
{
    return this->assignment_type;
}

Token const*
nt_Assignment::getFlag() const
{
    return this->flag;
}
