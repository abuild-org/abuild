#ifndef __NT_TYPESPEC_HH__
#define __NT_TYPESPEC_HH__

#include <NonTerminal.hh>
#include <Interface.hh>

class nt_TypeSpec: public NonTerminal
{
  public:
    nt_TypeSpec(FileLocation const&, Interface::type_e);
    virtual ~nt_TypeSpec();
    void setListType(Interface::list_e);
    void setNonRecursive();

    Interface::type_e getType() const;
    Interface::list_e getListType() const;
    bool getRecursive() const;

  private:
    Interface::type_e type;
    Interface::list_e list_type;
    bool recursive;
};

#endif // __NT_TYPESPEC_HH__
