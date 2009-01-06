#include "TokenFactory.hh"
#include <iostream>
#include <Token.hh>

TokenFactory::TokenFactory()
{
    reset();
}

Token*
TokenFactory::createToken(std::string val)
{
    this->last_location =
	FileLocation(this->filename, this->cur_lineno, this->cur_colno);
    Token* t = new Token(val, this->last_location);
    heap.insert(boost::shared_ptr<Token>(t));
    if (this->debug)
    {
	std::cerr << "[" << this->last_location << ": *" << val << "*] ";
    }

    for (std::string::iterator iter = val.begin(); iter != val.end(); ++iter)
    {
	if ((*iter) == '\n')
	{
	    ++this->cur_lineno;
	    this->cur_colno = 1;
	}
	else
	{
	    ++this->cur_colno;
	}
    }

    return t;
}

void
TokenFactory::reset()
{
    this->debug = false;
    this->filename.clear();
    this->cur_lineno = 1;
    this->cur_colno = 1;
    this->last_location = FileLocation();
    this->heap.clear();
}

void
TokenFactory::setFilename(std::string const& f)
{
    filename = f;
}

FileLocation const&
TokenFactory::getLastLocation()
{
    return last_location;
}
