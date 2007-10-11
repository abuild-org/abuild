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

    Interface::type_e getType() const;
    Interface::list_e getListType() const;

  private:
    Interface::type_e type;
    Interface::list_e list_type;
};

#endif // __NT_TYPESPEC_HH__
