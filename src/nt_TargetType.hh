#ifndef __NT_TARGETTYPE_HH__
#define __NT_TARGETTYPE_HH__

#include <NonTerminal.hh>
#include <FileLocation.hh>
#include <string>

class Token;

class nt_TargetType: public NonTerminal
{
  public:
    nt_TargetType(Token const*);
    virtual ~nt_TargetType();

    // Token will be an identifier or a variable
    Token const* getToken() const;

  private:
    Token const* token;
};

#endif // __NT_TARGETTYPE_HH__
