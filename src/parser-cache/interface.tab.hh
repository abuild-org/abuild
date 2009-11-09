/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tok_EOF = 258,
     tok_spaces = 259,
     tok_newline = 260,
     tok_quotedchar = 261,
     tok_equal = 262,
     tok_comma = 263,
     tok_clope = 264,
     tok_if = 265,
     tok_else = 266,
     tok_elseif = 267,
     tok_endif = 268,
     tok_kw_reset = 269,
     tok_kw_reset_all = 270,
     tok_kw_no_reset = 271,
     tok_kw_override = 272,
     tok_kw_fallback = 273,
     tok_kw_flag = 274,
     tok_kw_declare = 275,
     tok_kw_boolean = 276,
     tok_kw_string = 277,
     tok_kw_filename = 278,
     tok_kw_list = 279,
     tok_kw_append = 280,
     tok_kw_prepend = 281,
     tok_kw_nonrecursive = 282,
     tok_kw_local = 283,
     tok_kw_afterbuild = 284,
     tok_kw_targettype = 285,
     tok_identifier = 286,
     tok_environment = 287,
     tok_function = 288,
     tok_variable = 289,
     tok_other = 290
   };
#endif
/* Tokens.  */
#define tok_EOF 258
#define tok_spaces 259
#define tok_newline 260
#define tok_quotedchar 261
#define tok_equal 262
#define tok_comma 263
#define tok_clope 264
#define tok_if 265
#define tok_else 266
#define tok_elseif 267
#define tok_endif 268
#define tok_kw_reset 269
#define tok_kw_reset_all 270
#define tok_kw_no_reset 271
#define tok_kw_override 272
#define tok_kw_fallback 273
#define tok_kw_flag 274
#define tok_kw_declare 275
#define tok_kw_boolean 276
#define tok_kw_string 277
#define tok_kw_filename 278
#define tok_kw_list 279
#define tok_kw_append 280
#define tok_kw_prepend 281
#define tok_kw_nonrecursive 282
#define tok_kw_local 283
#define tok_kw_afterbuild 284
#define tok_kw_targettype 285
#define tok_identifier 286
#define tok_environment 287
#define tok_function 288
#define tok_variable 289
#define tok_other 290




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 10 "../interface.yy"
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
/* Line 1529 of yacc.c.  */
#line 141 "interface.tab.hh"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



