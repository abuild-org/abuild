#ifndef __NT_RESET_HH__
#define __NT_RESET_HH__

#include <NonTerminal.hh>
#include <Interface.hh>

class Token;

class nt_Reset: public NonTerminal
{
  public:
    nt_Reset(Token const* identifier, bool negate);
    nt_Reset(FileLocation const&); // reset all
    virtual ~nt_Reset();

    bool isNegate() const;
    // empty string means reset all
    std::string const& getIdentifier() const;

  private:
    std::string identifier;
    bool negate;
};

#endif // __NT_RESET_HH__
