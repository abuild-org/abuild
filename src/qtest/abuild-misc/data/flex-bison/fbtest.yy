// -*- c++ -*-
%{
#include <iostream>
#include <fbtest.tab.hh>
void fbtesterror(char*);
int fbtestlex(YYSTYPE*);
%}

%union
{
    int not_used;
}

%pure-parser

%token <not_used> tok_newline
%token <not_used> tok_keyval
%token <not_used> tok_other

%%

start	: lines
	;

lines	:
	| lines line
	| lines ignore
	;

line	: tok_keyval tok_newline
	  {
	      std::cout << "read line" << std::endl;
	  }
	;

ignore	: tok_newline
	| error tok_newline
	  {
	      std::cerr << "error" << std::endl;
	  }
	;

%%

void fbtesterror(char *)
{
}
