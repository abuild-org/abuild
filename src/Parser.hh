#ifndef __PARSER_HH__
#define __PARSER_HH__

#include <list>
#include <string>
#include <boost/shared_ptr.hpp>
#include <NonTerminal.hh>
#include <TokenFactory.hh>
#include <Error.hh>

class Error;
class FlexCaller;
class FileLocation;
class Token;

class Parser
{
  public:
    virtual ~Parser();

    int getNextToken();
    void setDebugParser(bool);

    // Derived class must implement this method to assign the given
    // token to the lexer's lval variable.
    virtual void setToken(Token*) = 0;

    // calls error->error
    void error(FileLocation const&, std::string const& msg);

    // Create a token and store its address for later automatic deletion
    Token* createToken(std::string const& val);

    // Return the file location of the most recently created token
    FileLocation const& getLastFileLocation();

  protected:
    Parser(FlexCaller&, int eof_token);

    // Returns true if there were no errors.
    bool parse(std::string const& filename);

    // Must call the grammar's parse function
    virtual void parseFile() = 0;

    // Called before parsing a file.
    virtual void startFile(std::string const& filename) = 0;

    // Called after parsing a file.  Token::reset is called after this
    // function is called, so there must be no more references to any
    // Token objects after this function returns.  Any non-terminals
    // passed to saveNonTerminal are also deleted.
    virtual void endFile(std::string const& filename) = 0;

    // Save a pointer to dynamically allocated NonTerminal.  Any
    // non-terminals passed to this function will be deleted after the
    // endFile method has been called.
    template<typename T>
    T* saveNonTerminal(T* nt)
    {
	this->heap.push_back(boost::shared_ptr<NonTerminal>(nt));
	return nt;
    }

    bool debug_parser;
    Error error_handler;

  private:
    Parser(Parser const&);
    Parser& operator=(Parser const&);

    FlexCaller& flex_caller;
    void* scanner;
    bool found_eof;
    int eof_token;
    TokenFactory token_factory;
    std::list<boost::shared_ptr<NonTerminal> > heap;
};

#endif // __PARSER_HH__
