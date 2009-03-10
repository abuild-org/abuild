#ifndef __NT_DECLARATION_HH__
#define __NT_DECLARATION_HH__

#include <NonTerminal.hh>
#include <Interface.hh>

class nt_TypeSpec;
class nt_Words;
class Token;

class nt_Declaration: public NonTerminal
{
  public:
    nt_Declaration(Token const* identifier, nt_TypeSpec const* typspec);
    virtual ~nt_Declaration();

    void addInitializer(nt_Words const* words);

    std::string const& getVariableName() const;
    nt_Words const* getInitializer() const;
    Interface::type_e getType() const;
    Interface::list_e getListType() const;
    bool getRecursive() const;

  private:
    std::string variable_name;
    Interface::type_e type;
    Interface::list_e list_type;
    bool recursive;
    nt_Words const* initializer;
};

#endif // __NT_DECLARATION_HH__
