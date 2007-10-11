#ifndef __TOKEN_HH__
#define __TOKEN_HH__

// This is a simple Token class intended by used by lexers to pass
// tokens to parsers.  The intended mode of operation is that
// everything that the lexer recognizes is converted to a token by
// calling createToken(yylex) with a given TokenFactory object.  If
// this is done, each token object will have a correct FileLocation
// object.

#include <FileLocation.hh>
#include <string>
class TokenFactory;

class Token
{
    friend class TokenFactory;

  public:
    std::string const& getValue() const;
    FileLocation const& getLocation() const;

  private:
    Token(std::string const&, FileLocation const&);

    // Disable copying and assignment
    Token(Token const&);
    Token& operator=(Token const&);

    std::string val;
    FileLocation location;
};

#endif // __TOKEN_HH__
