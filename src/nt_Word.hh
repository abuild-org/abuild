#ifndef __NT_WORD_HH__
#define __NT_WORD_HH__

#include <NonTerminal.hh>
#include <FileLocation.hh>
#include <string>
#include <list>

class Token;

class nt_Word: public NonTerminal
{
  public:
    nt_Word(FileLocation const&);
    virtual ~nt_Word();

    enum word_type_e { w_string, w_variable, w_environment };
    typedef std::pair<Token const*, word_type_e> word_t;

    void appendString(Token const* t);
    void appendVariable(Token const* t);
    void appendEnvironment(Token const* t);

    std::list<word_t> const& getTokens() const;

  private:
    void maybeSetLocation(Token const* t);

    std::list<word_t> tokens;
};

#endif // __NT_WORD_HH__
