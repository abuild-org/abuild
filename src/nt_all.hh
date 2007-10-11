#ifndef __NT_ALL_HH__
#define __NT_ALL_HH__

// This file must be included before interface.tab.hh so that the
// declaration of YYSTYPE can succeed.  We could just forward declare
// the classes here, but we'd have to have exactly the same list of
// includes in other places, so better to just do it all here.

#include <Token.hh>
#include <nt_Word.hh>
#include <nt_Words.hh>
#include <nt_AfterBuild.hh>
#include <nt_TargetType.hh>
#include <nt_TypeSpec.hh>
#include <nt_Declaration.hh>
#include <nt_Function.hh>
#include <nt_Argument.hh>
#include <nt_Arguments.hh>
#include <nt_Conditional.hh>
#include <nt_Assignment.hh>
#include <nt_Reset.hh>
#include <nt_Block.hh>
#include <nt_Blocks.hh>
#include <nt_IfClause.hh>
#include <nt_IfClauses.hh>
#include <nt_IfBlock.hh>

#endif
