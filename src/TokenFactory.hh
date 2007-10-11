#ifndef __TOKENFACTORY_HH__
#define __TOKENFACTORY_HH__

// This is a factory class for Token objects.  This class is used to
// dynamically allocate token objects and store pointers to them for
// later deletion.  When the TokenFactory is destroyed or reset, all
// the tokens that it allocated are deleted.  See also comments in
// Token.hh.

#include <string>
#include <set>
#include <boost/shared_ptr.hpp>
#include <FileLocation.hh>

class Token;

class TokenFactory
{
  public:
    TokenFactory();

    // Returns pointer to dynamically allocated Token.  All
    // dynamically allocated tokens are freed automatically when
    // reset() is called.  This means that the caller is never
    // responsible for deleting a token and is also responsible for
    // making sure that any information pulled from a token is a copy.
    Token* createToken(std::string val);

    // Turn debug true when running parser in debug mode.
    bool debug;

    void reset();
    void setFilename(std::string const& filename);
    FileLocation const& getLastLocation();

  private:
    // Disable copying and assignment
    TokenFactory(Token const&);
    TokenFactory& operator=(Token const&);

    std::string filename;
    int cur_lineno;
    int cur_colno;
    FileLocation last_location;
    std::set<boost::shared_ptr<Token> > heap;
};

#endif // __TOKENFACTORY_HH__
