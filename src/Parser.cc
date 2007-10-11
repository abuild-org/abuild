#include <Parser.hh>

#include <iostream>
#include <fstream>
#include <FlexLexer.h>
#include <assert.h>
#include <QEXC.hh>
#include <Token.hh>
#include <Logger.hh>

Parser::Parser(FlexLexer& lexer, int eof_token) :
    debug_parser(false),
    found_eof(false),
    eof_token(eof_token),
    lexer(lexer)
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
	result = this->lexer.yylex();
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
    std::ifstream input(filename.c_str());
    if (! input.is_open())
    {
	throw QEXC::System(std::string("failed to open ") + filename, errno);
    }

    this->lexer.switch_streams(&input, 0);
    this->token_factory.reset();
    this->token_factory.setFilename(filename);
    this->found_eof = false;
    startFile(filename);
    parseFile();
    endFile(filename);
    this->token_factory.reset();
    this->heap.clear();
    this->lexer.switch_streams(0, 0);
    return (this->error_handler.numErrors() == 0);
}
