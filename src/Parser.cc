#include <Parser.hh>

#include <stdio.h>
#include <assert.h>
#include <QEXC.hh>
#include <Token.hh>
#include <Logger.hh>
#include <FlexCaller.hh>

Parser::Parser(Error& error_handler, FlexCaller& flex_caller, int eof_token) :
    error_handler(error_handler),
    debug_parser(false),
    flex_caller(flex_caller),
    scanner(0),
    found_eof(false),
    eof_token(eof_token)
{
}

Parser::~Parser()
{
}

int
Parser::getNextToken()
{
    int result = 0;

    if (this->found_eof)
    {
	result = 0;
    }
    else
    {
	// After the lexer returns EOF, fabricate a special EOF token
	// and return it before returning 0.
	result = this->flex_caller.lex(this->scanner);
	if (result == 0)
	{
	    this->found_eof = true;
	    this->setToken(this->token_factory.createToken(""));
	    result = this->eof_token;
	}
    }

    return result;
}


void
Parser::setDebugParser(bool val)
{
    this->debug_parser = val;
    this->token_factory.debug = val;
}

void
Parser::error(FileLocation const& location, std::string const& msg)
{
    this->error_handler.error(location, msg);
}

FileLocation const&
Parser::getLastFileLocation()
{
    return this->token_factory.getLastLocation();
}

Token*
Parser::createToken(std::string const& val)
{
    return this->token_factory.createToken(val);
}

bool
Parser::parse(std::string const& filename)
{
    FILE* input = fopen(filename.c_str(), "rb");
    if (! input)
    {
	throw QEXC::System(std::string("failed to open ") + filename, errno);
    }

    int orig_errors = this->error_handler.numErrors();
    this->flex_caller.init_extra(this, &this->scanner);
    this->flex_caller.set_in(input, this->scanner);
    this->token_factory.reset();
    this->token_factory.setFilename(filename);
    this->found_eof = false;
    startFile(filename);
    parseFile();
    endFile(filename);
    fclose(input);
    this->token_factory.reset();
    this->heap.clear();
    this->flex_caller.lex_destroy(this->scanner);
    this->scanner = 0;
    return (this->error_handler.numErrors() == orig_errors);
}
