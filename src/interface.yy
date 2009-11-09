// -*- c++ -*-
%{
#include "InterfaceParser.hh"
%}

/* Write state tables to interface.output */
%verbose

%union
{
    int not_used;
    Token* token;
    nt_Word* word;
    nt_Words* words;
    nt_AfterBuild* afterbuild;
    nt_TargetType* targettype;
    nt_TypeSpec* typespec;
    nt_Declaration* declaration;
    nt_Function* function;
    nt_Argument* argument;
    nt_Arguments* arguments;
    nt_Conditional* conditional;
    nt_Assignment* assignment;
    nt_Reset* reset;
    nt_Block* block;
    nt_Blocks* blocks;
    nt_IfBlock* ifblock;
    nt_IfClause* ifclause;
    nt_IfClauses* ifclauses;
}

%pure-parser
%lex-param   { InterfaceParser* parser }
%parse-param { InterfaceParser* parser }

%token <token> tok_EOF /* special case handled by Parser class */
%token <not_used> tok_spaces
%token <token> tok_newline
%token <token> tok_quotedchar
%token <token> tok_equal
%token <token> tok_comma
%token <token> tok_clope
%token <token> tok_if
%token <token> tok_else
%token <token> tok_elseif
%token <token> tok_endif
%token <token> tok_reset
%token <token> tok_reset_all
%token <token> tok_no_reset
%token <token> tok_override
%token <token> tok_fallback
%token <token> tok_flag
%token <token> tok_declare
%token <token> tok_boolean
%token <token> tok_string
%token <token> tok_filename
%token <token> tok_list
%token <token> tok_append
%token <token> tok_prepend
%token <token> tok_nonrecursive
%token <token> tok_local
%token <token> tok_afterbuild
%token <token> tok_targettype
%token <token> tok_identifier
%token <token> tok_environment
%token <token> tok_function
%token <token> tok_variable
%token <token> tok_other

%type <not_used> start
%type <blocks> blocks
%type <not_used> ignore
%type <block> block
%type <ifblock> ifblock
%type <ifclause> if
%type <ifclauses> elseifs
%type <ifclause> else
%type <ifclause> elseif
%type <assignment> assignment
%type <reset> reset
%type <conditional> ifstatement
%type <not_used> elsestatement
%type <conditional> elseifstatement
%type <not_used> endifstatement
%type <declaration> declaration
%type <declaration> declbody
%type <typespec> typespec
%type <typespec> listtypespec
%type <typespec> basetypespec
%type <afterbuild> afterbuild
%type <targettype> targettype
%type <words> words
%type <word> word
%type <word> wordfragment
%type <conditional> conditional
%type <function> function
%type <arguments> arguments
%type <argument> argument
%type <token> nospaceendofline
%type <token> endofline

%%

start	: blocks
	  {
	      parser->acceptParseTree($1);
	      $$ = 0;
	  }
	;

blocks	:
	  {
	      $$ = parser->createBlocks();
	  }
	| blocks block
	  {
	      $1->addBlock($2);
	      $$ = $1;
	  }
	| blocks ignore
	  {
	      $$ = $1;
	  }
	;

ignore	: endofline
	  {
	      $$ = 0;
	  }
	| error endofline
	  {
	      parser->error($2->getLocation(),
			    "a parse error occured on or before this line");
	      $$ = 0;
	  }
	;

block	: ifblock
	  {
	      $$ = parser->createBlock($1);
	  }
	| reset
	  {
	      $$ = parser->createBlock($1);
	  }
	| assignment
	  {
	      $$ = parser->createBlock($1);
	  }
	| declaration
	  {
	      $$ = parser->createBlock($1);
	  }
	| afterbuild
	  {
	      $$ = parser->createBlock($1);
	  }
	| targettype
	  {
	      $$ = parser->createBlock($1);
	  }
	;

ifblock	: if endifstatement
	  {
	      $$ = parser->createIfBlock($1, 0, 0);
	  }
	| if elseifs endifstatement
	  {
	      $$ = parser->createIfBlock($1, $2, 0);
	  }
	| if else endifstatement
	  {
	      $$ = parser->createIfBlock($1, 0, $2);
	  }
	| if elseifs else endifstatement
	  {
	      $$ = parser->createIfBlock($1, $2, $3);
	  }
	;

if	: ifstatement blocks
	  {
	      $$ = parser->createIfClause($1, $2, true);
	  }
	;

elseifs	: elseif
	  {
	      $$ = parser->createIfClauses($1->getLocation());
	      $$->addClause($1);
	  }
	| elseifs elseif
	  {
	      $1->addClause($2);
	      $$ = $1;
	  }
	;

elseif	: elseifstatement blocks
	  {
	      $$ = parser->createIfClause($1, $2, true);
	  }
	;

else	: elsestatement blocks
	  {
	      $$ = parser->createIfClause(0, $2, false);
	  }
	;

reset : tok_reset tok_identifier endofline
	  {
	      $$ = parser->createReset($2, false);
	  }
	| tok_reset_all endofline
	  {
	      $$ = parser->createReset($1->getLocation());
	  }
	| tok_no_reset tok_identifier endofline
	  {
	      $$ = parser->createReset($2, true);
	  }
	;

assignment : tok_identifier tok_equal words endofline
	  {
	      $$ = parser->createAssignment($1, $3);
	  }
	| tok_override assignment
	  {
	      $2->setAssignmentType(Interface::a_override);
	      $$ = $2;
	  }
	| tok_fallback assignment
	  {
	      $2->setAssignmentType(Interface::a_fallback);
	      $$ = $2;
	  }
	| tok_flag tok_identifier assignment
	  {
	      $3->setFlag($2);
	      $$ = $3;
	  }
	| tok_spaces assignment
	  {
	      $$ = $2;
	  }
	;

ifstatement : tok_if conditional endofline
	  {
	      $$ = $2;
	  }
	| tok_if error endofline
	  {
	      parser->error($1->getLocation(),
			    "unable to parse if statement");
	      $$ = 0;
	  }
	;

elseifstatement : tok_elseif conditional endofline
	  {
	      $$ = $2;
	  }
	| tok_elseif error endofline
	  {
	      parser->error($1->getLocation(),
			    "unable to parse elseif statement");
	      $$ = 0;
	  }
	;

elsestatement : tok_else endofline
	  {
	      $$ = 0;
	  }
	| tok_else error endofline
	  {
	      parser->error($1->getLocation(),
			    "unable to parse else statement");
	      $$ = 0;
	  }
	;

endifstatement : tok_endif endofline
	  {
	      $$ = 0;
	  }
	| tok_endif error endofline
	  {
	      parser->error($1->getLocation(),
			    "unable to parse endif statement");
	      $$ = 0;
	  }
	;

conditional : tok_variable tok_clope
	  {
	      $$ = parser->createConditional($1);
	  }
	| function tok_clope
	  {
	      $$ = parser->createConditional($1);
	  }
	;

arguments : arguments tok_comma argument
	  {
	      $1->appendArgument($3);
	      $$ = $1;
	  }
	| argument
	  {
	      $$ = parser->createArguments($1->getLocation());
	      $$->appendArgument($1);
	  }
	;

argument : words
	  {
	      $$ = parser->createArgument($1);
	  }
	| function
	  {
	      $$ = parser->createArgument($1);
	  }
	;

function : tok_function arguments tok_clope
	  {
	      $$ = parser->createFunction($1, $2);
	  }
	;

declaration : declbody endofline
	  {
	      $$ = $1;
	  }
	| declbody tok_equal words endofline
	  {
	      $1->addInitializer($3);
	      $$ = $1;
	  }
	;

declbody : tok_declare tok_identifier typespec
	  {
	      $$ = parser->createDeclaration($2, $3);
	  }
	;

typespec : listtypespec
	  {
	      $$ = $1;
	  }
	| tok_nonrecursive listtypespec
	  {
	      $2->setScope(Interface::s_nonrecursive);
	      $$ = $2;
	  }
	| tok_local listtypespec
	  {
	      $2->setScope(Interface::s_local);
	      $$ = $2;
	  }
	;

listtypespec : basetypespec
          {
	      $$ = $1;
	  }
        | tok_list basetypespec tok_append
	  {
	      $2->setListType(Interface::l_append);
	      $$ = $2;
	  }
	| tok_list basetypespec tok_prepend
	  {
	      $2->setListType(Interface::l_prepend);
	      $$ = $2;
	  }
	;

basetypespec : tok_boolean
	  {
	      $$ = parser->createTypeSpec(
		  $1->getLocation(), Interface::t_boolean);
	  }
	| tok_string
	  {
	      $$ = parser->createTypeSpec(
		  $1->getLocation(), Interface::t_string);
	  }
	| tok_filename
	  {
	      $$ = parser->createTypeSpec(
		  $1->getLocation(), Interface::t_filename);
	  }
	;

afterbuild : tok_afterbuild words endofline
	  {
	      $$ = parser->createAfterBuild($2);
	  }
	;

targettype : tok_targettype tok_identifier endofline
	  {
	      $$ = parser->createTargetType($2);
	  }
	| tok_targettype tok_variable endofline
	  {
	      $$ = parser->createTargetType($2);
	  }
	;

words	:
          {
	      $$ = parser->createWords(parser->getLastFileLocation());
	  }
	| word
	  {
	      $$ = parser->createWords($1->getLocation());
	      $$->append($1);
	  }
	| words tok_spaces word
	  {
	      $1->append($3);
	      $$ = $1;
	  }
	;

word	: wordfragment
	  {
	      $$ = $1;
	  }
	| word wordfragment
	  {
	      $1->appendWord($2);
	      $$ = $1;
	  }

wordfragment : tok_variable
	  {
	      $$ = parser->createWord();
	      $$->appendVariable($1);
	  }
	| tok_quotedchar
	  {
	      $$ = parser->createWord();
	      $$->appendString($1);
	  }
	| tok_identifier
	  {
	      $$ = parser->createWord();
	      $$->appendString($1);
	  }
	| tok_environment
	  {
	      $$ = parser->createWord();
	      $$->appendEnvironment($1);
	  }
	| tok_other
	  {
	      $$ = parser->createWord();
	      $$->appendString($1);
	  }
	;

endofline : nospaceendofline
	  {
	      $$ = $1;
	  }
	| tok_spaces nospaceendofline
	  {
	      $$ = $2;
	  }
	;

nospaceendofline : tok_EOF
	  {
	      $$ = $1;
	  }
	| tok_newline
	  {
	      $$ = $1;
	  }
	;

%%
