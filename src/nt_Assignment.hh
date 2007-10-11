#ifndef __NT_ASSIGNMENT_HH__
#define __NT_ASSIGNMENT_HH__

#include <NonTerminal.hh>
#include <Interface.hh>

class Token;
class nt_Words;

class nt_Assignment: public NonTerminal
{
  public:
    nt_Assignment(Token const* identifier, nt_Words const* words);
    virtual ~nt_Assignment();
    void setAssignmentType(Interface::assign_e assignment_type);
    void setFlag(Token* flag);

    std::string const& getIdentifier() const;
    nt_Words const* getWords() const;
    Interface::assign_e getAssignmentType() const;
    Token const* getFlag() const;

  private:
    std::string identifier;
    Token* flag;
    nt_Words const* words;
    Interface::assign_e assignment_type;
};

#endif // __NT_ASSIGNMENT_HH__
