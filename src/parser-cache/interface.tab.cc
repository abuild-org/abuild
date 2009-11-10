/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse interfaceparse
#define yylex   interfacelex
#define yyerror interfaceerror
#define yylval  interfacelval
#define yychar  interfacechar
#define yydebug interfacedebug
#define yynerrs interfacenerrs


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




/* Copy the first part of user declarations.  */
#line 2 "../interface.yy"

#include "InterfaceParser.hh"


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

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
/* Line 193 of yacc.c.  */
#line 200 "interface.tab.cc"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 213 "interface.tab.cc"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   324

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  36
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  34
/* YYNRULES -- Number of rules.  */
#define YYNRULES  96
/* YYNRULES -- Number of states.  */
#define YYNSTATES  170

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   290

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    12,    14,    17,    19,
      21,    23,    25,    27,    29,    32,    36,    40,    45,    48,
      50,    53,    56,    59,    64,    67,    72,    79,    84,    88,
      92,    98,   101,   105,   109,   113,   117,   120,   124,   127,
     131,   134,   137,   142,   144,   145,   147,   149,   153,   156,
     163,   168,   175,   177,   181,   185,   187,   193,   199,   201,
     203,   205,   211,   217,   223,   225,   229,   231,   234,   236,
     238,   240,   242,   244,   246,   248,   250,   252,   254,   256,
     258,   260,   262,   264,   266,   268,   270,   272,   274,   276,
     278,   280,   282,   285,   287,   289,   290
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      37,     0,    -1,    38,    -1,    -1,    38,    40,    -1,    38,
      39,    -1,    67,    -1,     1,    67,    -1,    41,    -1,    46,
      -1,    47,    -1,    56,    -1,    61,    -1,    62,    -1,    42,
      51,    -1,    42,    43,    51,    -1,    42,    45,    51,    -1,
      42,    43,    45,    51,    -1,    48,    38,    -1,    44,    -1,
      43,    44,    -1,    49,    38,    -1,    50,    38,    -1,    14,
       4,    31,    67,    -1,    15,    67,    -1,    16,     4,    31,
      67,    -1,    31,    69,     7,    69,    63,    67,    -1,    31,
      69,     7,    67,    -1,    17,     4,    47,    -1,    18,     4,
      47,    -1,    19,     4,    31,     4,    47,    -1,     4,    47,
      -1,    10,    52,    67,    -1,    10,     1,    67,    -1,    12,
      52,    67,    -1,    12,     1,    67,    -1,    11,    67,    -1,
      11,     1,    67,    -1,    13,    67,    -1,    13,     1,    67,
      -1,    34,     9,    -1,    55,     9,    -1,    53,     8,    69,
      54,    -1,    54,    -1,    -1,    63,    -1,    55,    -1,    33,
      53,     9,    -1,    57,    67,    -1,    57,    69,     7,    69,
      63,    67,    -1,    57,    69,     7,    67,    -1,    69,    20,
       4,    31,     4,    58,    -1,    59,    -1,    27,     4,    59,
      -1,    28,     4,    59,    -1,    60,    -1,    24,     4,    60,
       4,    25,    -1,    24,     4,    60,     4,    26,    -1,    21,
      -1,    22,    -1,    23,    -1,    69,    29,     4,    64,    67,
      -1,    69,    30,     4,    31,    67,    -1,    69,    30,     4,
      34,    67,    -1,    64,    -1,    63,     4,    64,    -1,    65,
      -1,    64,    65,    -1,    34,    -1,     6,    -1,    31,    -1,
      32,    -1,    35,    -1,    66,    -1,    14,    -1,    15,    -1,
      16,    -1,    17,    -1,    18,    -1,    19,    -1,    20,    -1,
      21,    -1,    22,    -1,    23,    -1,    24,    -1,    25,    -1,
      26,    -1,    27,    -1,    28,    -1,    29,    -1,    30,    -1,
      68,    -1,     4,    68,    -1,     3,    -1,     5,    -1,    -1,
       4,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   105,   105,   113,   116,   121,   127,   131,   139,   143,
     147,   151,   155,   159,   165,   169,   173,   177,   183,   189,
     194,   201,   207,   213,   217,   221,   227,   231,   235,   240,
     245,   250,   256,   260,   268,   272,   280,   284,   292,   296,
     304,   308,   314,   319,   327,   330,   334,   340,   346,   350,
     355,   362,   368,   372,   377,   384,   388,   393,   400,   405,
     410,   417,   423,   427,   433,   438,   445,   449,   455,   460,
     465,   470,   475,   480,   487,   488,   489,   490,   491,   492,
     493,   494,   495,   496,   497,   498,   499,   500,   501,   502,
     503,   506,   510,   516,   520,   527,   528
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "tok_EOF", "tok_spaces", "tok_newline",
  "tok_quotedchar", "tok_equal", "tok_comma", "tok_clope", "tok_if",
  "tok_else", "tok_elseif", "tok_endif", "tok_kw_reset",
  "tok_kw_reset_all", "tok_kw_no_reset", "tok_kw_override",
  "tok_kw_fallback", "tok_kw_flag", "tok_kw_declare", "tok_kw_boolean",
  "tok_kw_string", "tok_kw_filename", "tok_kw_list", "tok_kw_append",
  "tok_kw_prepend", "tok_kw_nonrecursive", "tok_kw_local",
  "tok_kw_afterbuild", "tok_kw_targettype", "tok_identifier",
  "tok_environment", "tok_function", "tok_variable", "tok_other",
  "$accept", "start", "blocks", "ignore", "block", "ifblock", "if",
  "elseifs", "elseif", "else", "reset", "assignment", "ifstatement",
  "elseifstatement", "elsestatement", "endifstatement", "conditional",
  "arguments", "argument", "function", "declaration", "declbody",
  "typespec", "listtypespec", "basetypespec", "afterbuild", "targettype",
  "words", "word", "wordfragment", "keyword", "endofline",
  "nospaceendofline", "sp", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    36,    37,    38,    38,    38,    39,    39,    40,    40,
      40,    40,    40,    40,    41,    41,    41,    41,    42,    43,
      43,    44,    45,    46,    46,    46,    47,    47,    47,    47,
      47,    47,    48,    48,    49,    49,    50,    50,    51,    51,
      52,    52,    53,    53,    54,    54,    54,    55,    56,    56,
      56,    57,    58,    58,    58,    59,    59,    59,    60,    60,
      60,    61,    62,    62,    63,    63,    64,    64,    65,    65,
      65,    65,    65,    65,    66,    66,    66,    66,    66,    66,
      66,    66,    66,    66,    66,    66,    66,    66,    66,    66,
      66,    67,    67,    68,    68,    69,    69
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     1,     2,     1,     1,
       1,     1,     1,     1,     2,     3,     3,     4,     2,     1,
       2,     2,     2,     4,     2,     4,     6,     4,     3,     3,
       5,     2,     3,     3,     3,     3,     2,     3,     2,     3,
       2,     2,     4,     1,     0,     1,     1,     3,     2,     6,
       4,     6,     1,     3,     3,     1,     5,     5,     1,     1,
       1,     5,     5,     5,     1,     3,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     1,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     0,    93,    96,    94,     0,     0,
       0,     0,     0,     0,     0,    95,     5,     4,     8,     0,
       9,    10,     3,    11,    95,    12,    13,     6,    91,     0,
       0,     7,     0,    31,    92,     0,    44,     0,     0,     0,
       0,    24,     0,     0,     0,     0,    96,     0,     0,     0,
       0,     0,    19,     0,     3,     3,    14,     0,    96,    48,
       0,     0,     0,     0,    33,    69,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    70,    71,    68,    72,     0,    43,    46,
      45,    64,    66,    73,    40,    32,    41,     0,     0,    28,
      29,     0,    95,     0,    36,     0,     0,     0,    38,    20,
       0,    15,    16,     0,     0,    95,     0,     0,     0,    95,
      47,     0,    67,    23,    25,     0,    27,     0,    37,    35,
      34,    39,    17,    50,     0,     0,     0,     0,     0,    44,
      65,    30,     0,     0,     0,    61,    62,    63,    42,     0,
      26,    49,    58,    59,    60,     0,     0,     0,    51,    52,
      55,     0,     0,     0,     0,    53,    54,     0,    56,    57
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    16,    17,    18,    19,    51,    52,    53,
      20,    21,    22,    54,    55,    56,    38,    87,    88,    39,
      23,    24,   158,   159,   160,    25,    26,    90,    91,    92,
      93,    27,    28,    29
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -116
static const yytype_int16 yypact[] =
{
    -116,    46,   175,  -116,    51,  -116,   110,  -116,     8,     6,
      51,    29,    47,    58,    64,    84,  -116,  -116,  -116,    85,
    -116,  -116,  -116,  -116,   113,  -116,  -116,  -116,  -116,    -7,
      88,  -116,   106,  -116,  -116,    51,   259,    92,    51,   102,
      91,  -116,    95,   106,   106,   105,  -116,   131,    26,    10,
      33,    85,  -116,   136,  -116,  -116,  -116,   197,    88,  -116,
     143,   147,   148,   168,  -116,  -116,  -116,  -116,  -116,  -116,
    -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,  -116,
    -116,  -116,  -116,  -116,  -116,  -116,  -116,    17,  -116,  -116,
     173,   289,  -116,  -116,  -116,  -116,  -116,    51,    51,  -116,
    -116,   182,   113,    51,  -116,    51,    51,    51,  -116,  -116,
     136,  -116,  -116,   219,   241,   113,   165,   289,   -16,    84,
    -116,   289,  -116,  -116,  -116,   106,  -116,   289,  -116,  -116,
    -116,  -116,  -116,  -116,   289,   193,    55,    51,    51,   259,
     289,  -116,   127,   127,   160,  -116,  -116,  -116,  -116,   139,
    -116,  -116,  -116,  -116,  -116,   195,   199,   214,  -116,  -116,
    -116,   125,    43,    43,   215,  -116,  -116,    82,  -116,  -116
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -116,  -116,    -5,  -116,  -116,  -116,  -116,  -116,   170,   174,
    -116,    -4,  -116,  -116,  -116,   -47,   191,  -116,   104,   -33,
    -116,  -116,  -116,   -43,    80,  -116,  -116,  -115,  -101,   -83,
    -116,    -3,    -6,   -10
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -96
static const yytype_int16 yytable[] =
{
      34,    31,    33,    89,   111,    47,   112,    41,   122,    35,
      40,   105,   142,    61,    60,   137,   136,    57,   138,   143,
     140,    59,    62,    63,    34,   119,   120,   103,    33,     5,
      30,     7,    64,    42,   107,    95,     5,    30,     7,    99,
     100,    36,    37,    36,    37,   104,     3,   108,   140,   113,
     114,    43,    34,   122,     5,    30,     7,   122,     5,    30,
       7,    65,    44,   132,   152,   153,   154,   155,    45,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    46,    85,
      86,     5,   127,     7,   123,   124,    48,    49,    50,   126,
     128,    94,   129,   130,   131,   134,    89,   168,   169,   139,
      32,    96,   133,     5,    32,     7,     5,    58,     7,   165,
     166,   141,    97,    12,    13,    14,    98,    12,    13,    14,
       5,   149,     7,   145,   146,   147,   101,    15,   102,   150,
     151,    15,     5,    34,     7,    65,   152,   153,   154,    50,
     115,   116,   117,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,   118,    85,    86,    -2,     4,   121,     5,     6,
       7,   152,   153,   154,   155,     8,   125,   156,   157,     9,
      10,    11,    12,    13,    14,   -95,   135,   144,     4,   161,
       5,     6,     7,   162,   -95,   -95,    15,     8,   -18,   -18,
     -18,     9,    10,    11,    12,    13,    14,   -95,   163,   167,
       4,   109,     5,     6,     7,   110,   -95,   -95,    15,     8,
     -21,   -21,   -21,     9,    10,    11,    12,    13,    14,   -95,
     106,   164,     4,   148,     5,     6,     7,     0,   -95,   -95,
      15,     8,     0,     0,   -22,     9,    10,    11,    12,    13,
      14,   -95,     0,     0,     0,    65,     0,     0,     0,     0,
     -95,   -95,    15,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    36,    85,    86,    65,     0,     0,     0,     0,
       0,     0,     0,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,     0,    85,    86
};

static const yytype_int16 yycheck[] =
{
       6,     4,     6,    36,    51,    15,    53,    10,    91,     1,
       4,     1,   127,    20,    24,    31,   117,    22,    34,   134,
     121,    24,    29,    30,    30,     8,     9,     1,    32,     3,
       4,     5,    35,     4,     1,    38,     3,     4,     5,    43,
      44,    33,    34,    33,    34,    48,     0,    50,   149,    54,
      55,     4,    58,   136,     3,     4,     5,   140,     3,     4,
       5,     6,     4,   110,    21,    22,    23,    24,     4,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,     4,    34,
      35,     3,   102,     5,    97,    98,    11,    12,    13,   102,
     103,     9,   105,   106,   107,   115,   139,    25,    26,   119,
       4,     9,   115,     3,     4,     5,     3,     4,     5,   162,
     163,   125,    31,    17,    18,    19,    31,    17,    18,    19,
       3,     4,     5,   136,   137,   138,    31,    31,     7,   142,
     143,    31,     3,   149,     5,     6,    21,    22,    23,    13,
       7,     4,     4,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,     4,    34,    35,     0,     1,     4,     3,     4,
       5,    21,    22,    23,    24,    10,     4,    27,    28,    14,
      15,    16,    17,    18,    19,    20,    31,     4,     1,     4,
       3,     4,     5,     4,    29,    30,    31,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,     4,     4,
       1,    51,     3,     4,     5,    51,    29,    30,    31,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      49,   161,     1,   139,     3,     4,     5,    -1,    29,    30,
      31,    10,    -1,    -1,    13,    14,    15,    16,    17,    18,
      19,    20,    -1,    -1,    -1,     6,    -1,    -1,    -1,    -1,
      29,    30,    31,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,     6,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    -1,    34,    35
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    37,    38,     0,     1,     3,     4,     5,    10,    14,
      15,    16,    17,    18,    19,    31,    39,    40,    41,    42,
      46,    47,    48,    56,    57,    61,    62,    67,    68,    69,
       4,    67,     4,    47,    68,     1,    33,    34,    52,    55,
       4,    67,     4,     4,     4,     4,     4,    69,    11,    12,
      13,    43,    44,    45,    49,    50,    51,    38,     4,    67,
      69,    20,    29,    30,    67,     6,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    34,    35,    53,    54,    55,
      63,    64,    65,    66,     9,    67,     9,    31,    31,    47,
      47,    31,     7,     1,    67,     1,    52,     1,    67,    44,
      45,    51,    51,    38,    38,     7,     4,     4,     4,     8,
       9,     4,    65,    67,    67,     4,    67,    69,    67,    67,
      67,    67,    51,    67,    69,    31,    64,    31,    34,    69,
      64,    47,    63,    63,     4,    67,    67,    67,    54,     4,
      67,    67,    21,    22,    23,    24,    27,    28,    58,    59,
      60,     4,     4,     4,    60,    59,    59,     4,    25,    26
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (parser, YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, parser)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value, parser); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, InterfaceParser* parser)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, parser)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    InterfaceParser* parser;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (parser);
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, InterfaceParser* parser)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, parser)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    InterfaceParser* parser;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, parser);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, InterfaceParser* parser)
#else
static void
yy_reduce_print (yyvsp, yyrule, parser)
    YYSTYPE *yyvsp;
    int yyrule;
    InterfaceParser* parser;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , parser);
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, parser); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, InterfaceParser* parser)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, parser)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    InterfaceParser* parser;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (parser);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (InterfaceParser* parser);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (InterfaceParser* parser)
#else
int
yyparse (parser)
    InterfaceParser* parser;
#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 106 "../interface.yy"
    {
	      parser->acceptParseTree((yyvsp[(1) - (1)].blocks));
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 3:
#line 113 "../interface.yy"
    {
	      (yyval.blocks) = parser->createBlocks();
	  ;}
    break;

  case 4:
#line 117 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].blocks)->addBlock((yyvsp[(2) - (2)].block));
	      (yyval.blocks) = (yyvsp[(1) - (2)].blocks);
	  ;}
    break;

  case 5:
#line 122 "../interface.yy"
    {
	      (yyval.blocks) = (yyvsp[(1) - (2)].blocks);
	  ;}
    break;

  case 6:
#line 128 "../interface.yy"
    {
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 7:
#line 132 "../interface.yy"
    {
	      parser->error((yyvsp[(2) - (2)].token)->getLocation(),
			    "a parse error occured on or before this line");
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 8:
#line 140 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].ifblock));
	  ;}
    break;

  case 9:
#line 144 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].reset));
	  ;}
    break;

  case 10:
#line 148 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].assignment));
	  ;}
    break;

  case 11:
#line 152 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].declaration));
	  ;}
    break;

  case 12:
#line 156 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].afterbuild));
	  ;}
    break;

  case 13:
#line 160 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].targettype));
	  ;}
    break;

  case 14:
#line 166 "../interface.yy"
    {
	      (yyval.ifblock) = parser->createIfBlock((yyvsp[(1) - (2)].ifclause), 0, 0);
	  ;}
    break;

  case 15:
#line 170 "../interface.yy"
    {
	      (yyval.ifblock) = parser->createIfBlock((yyvsp[(1) - (3)].ifclause), (yyvsp[(2) - (3)].ifclauses), 0);
	  ;}
    break;

  case 16:
#line 174 "../interface.yy"
    {
	      (yyval.ifblock) = parser->createIfBlock((yyvsp[(1) - (3)].ifclause), 0, (yyvsp[(2) - (3)].ifclause));
	  ;}
    break;

  case 17:
#line 178 "../interface.yy"
    {
	      (yyval.ifblock) = parser->createIfBlock((yyvsp[(1) - (4)].ifclause), (yyvsp[(2) - (4)].ifclauses), (yyvsp[(3) - (4)].ifclause));
	  ;}
    break;

  case 18:
#line 184 "../interface.yy"
    {
	      (yyval.ifclause) = parser->createIfClause((yyvsp[(1) - (2)].conditional), (yyvsp[(2) - (2)].blocks), true);
	  ;}
    break;

  case 19:
#line 190 "../interface.yy"
    {
	      (yyval.ifclauses) = parser->createIfClauses((yyvsp[(1) - (1)].ifclause)->getLocation());
	      (yyval.ifclauses)->addClause((yyvsp[(1) - (1)].ifclause));
	  ;}
    break;

  case 20:
#line 195 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].ifclauses)->addClause((yyvsp[(2) - (2)].ifclause));
	      (yyval.ifclauses) = (yyvsp[(1) - (2)].ifclauses);
	  ;}
    break;

  case 21:
#line 202 "../interface.yy"
    {
	      (yyval.ifclause) = parser->createIfClause((yyvsp[(1) - (2)].conditional), (yyvsp[(2) - (2)].blocks), true);
	  ;}
    break;

  case 22:
#line 208 "../interface.yy"
    {
	      (yyval.ifclause) = parser->createIfClause(0, (yyvsp[(2) - (2)].blocks), false);
	  ;}
    break;

  case 23:
#line 214 "../interface.yy"
    {
	      (yyval.reset) = parser->createReset((yyvsp[(3) - (4)].token), false);
	  ;}
    break;

  case 24:
#line 218 "../interface.yy"
    {
	      (yyval.reset) = parser->createReset((yyvsp[(1) - (2)].token)->getLocation());
	  ;}
    break;

  case 25:
#line 222 "../interface.yy"
    {
	      (yyval.reset) = parser->createReset((yyvsp[(3) - (4)].token), true);
	  ;}
    break;

  case 26:
#line 228 "../interface.yy"
    {
	      (yyval.assignment) = parser->createAssignment((yyvsp[(1) - (6)].token), (yyvsp[(5) - (6)].words));
	  ;}
    break;

  case 27:
#line 232 "../interface.yy"
    {
	      (yyval.assignment) = parser->createAssignment((yyvsp[(1) - (4)].token), parser->createEmptyWords());
	  ;}
    break;

  case 28:
#line 236 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].assignment)->setAssignmentType(Interface::a_override);
	      (yyval.assignment) = (yyvsp[(3) - (3)].assignment);
	  ;}
    break;

  case 29:
#line 241 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].assignment)->setAssignmentType(Interface::a_fallback);
	      (yyval.assignment) = (yyvsp[(3) - (3)].assignment);
	  ;}
    break;

  case 30:
#line 246 "../interface.yy"
    {
	      (yyvsp[(5) - (5)].assignment)->setFlag((yyvsp[(3) - (5)].token));
	      (yyval.assignment) = (yyvsp[(5) - (5)].assignment);
	  ;}
    break;

  case 31:
#line 251 "../interface.yy"
    {
	      (yyval.assignment) = (yyvsp[(2) - (2)].assignment);
	  ;}
    break;

  case 32:
#line 257 "../interface.yy"
    {
	      (yyval.conditional) = (yyvsp[(2) - (3)].conditional);
	  ;}
    break;

  case 33:
#line 261 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse if statement");
	      (yyval.conditional) = 0;
	  ;}
    break;

  case 34:
#line 269 "../interface.yy"
    {
	      (yyval.conditional) = (yyvsp[(2) - (3)].conditional);
	  ;}
    break;

  case 35:
#line 273 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse elseif statement");
	      (yyval.conditional) = 0;
	  ;}
    break;

  case 36:
#line 281 "../interface.yy"
    {
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 37:
#line 285 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse else statement");
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 38:
#line 293 "../interface.yy"
    {
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 39:
#line 297 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse endif statement");
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 40:
#line 305 "../interface.yy"
    {
	      (yyval.conditional) = parser->createConditional((yyvsp[(1) - (2)].token));
	  ;}
    break;

  case 41:
#line 309 "../interface.yy"
    {
	      (yyval.conditional) = parser->createConditional((yyvsp[(1) - (2)].function));
	  ;}
    break;

  case 42:
#line 315 "../interface.yy"
    {
	      (yyvsp[(1) - (4)].arguments)->appendArgument((yyvsp[(4) - (4)].argument));
	      (yyval.arguments) = (yyvsp[(1) - (4)].arguments);
	  ;}
    break;

  case 43:
#line 320 "../interface.yy"
    {
	      (yyval.arguments) = parser->createArguments((yyvsp[(1) - (1)].argument)->getLocation());
	      (yyval.arguments)->appendArgument((yyvsp[(1) - (1)].argument));
	  ;}
    break;

  case 44:
#line 327 "../interface.yy"
    {
	      (yyval.argument) = parser->createArgument(parser->createEmptyWords());
	  ;}
    break;

  case 45:
#line 331 "../interface.yy"
    {
	      (yyval.argument) = parser->createArgument((yyvsp[(1) - (1)].words));
	  ;}
    break;

  case 46:
#line 335 "../interface.yy"
    {
	      (yyval.argument) = parser->createArgument((yyvsp[(1) - (1)].function));
	  ;}
    break;

  case 47:
#line 341 "../interface.yy"
    {
	      (yyval.function) = parser->createFunction((yyvsp[(1) - (3)].token), (yyvsp[(2) - (3)].arguments));
	  ;}
    break;

  case 48:
#line 347 "../interface.yy"
    {
	      (yyval.declaration) = (yyvsp[(1) - (2)].declaration);
	  ;}
    break;

  case 49:
#line 351 "../interface.yy"
    {
	      (yyvsp[(1) - (6)].declaration)->addInitializer((yyvsp[(5) - (6)].words));
	      (yyval.declaration) = (yyvsp[(1) - (6)].declaration);
	  ;}
    break;

  case 50:
#line 356 "../interface.yy"
    {
	      (yyvsp[(1) - (4)].declaration)->addInitializer(parser->createEmptyWords());
	      (yyval.declaration) = (yyvsp[(1) - (4)].declaration);
	  ;}
    break;

  case 51:
#line 363 "../interface.yy"
    {
	      (yyval.declaration) = parser->createDeclaration((yyvsp[(4) - (6)].token), (yyvsp[(6) - (6)].typespec));
	  ;}
    break;

  case 52:
#line 369 "../interface.yy"
    {
	      (yyval.typespec) = (yyvsp[(1) - (1)].typespec);
	  ;}
    break;

  case 53:
#line 373 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].typespec)->setScope(Interface::s_nonrecursive);
	      (yyval.typespec) = (yyvsp[(3) - (3)].typespec);
	  ;}
    break;

  case 54:
#line 378 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].typespec)->setScope(Interface::s_local);
	      (yyval.typespec) = (yyvsp[(3) - (3)].typespec);
	  ;}
    break;

  case 55:
#line 385 "../interface.yy"
    {
	      (yyval.typespec) = (yyvsp[(1) - (1)].typespec);
	  ;}
    break;

  case 56:
#line 389 "../interface.yy"
    {
	      (yyvsp[(3) - (5)].typespec)->setListType(Interface::l_append);
	      (yyval.typespec) = (yyvsp[(3) - (5)].typespec);
	  ;}
    break;

  case 57:
#line 394 "../interface.yy"
    {
	      (yyvsp[(3) - (5)].typespec)->setListType(Interface::l_prepend);
	      (yyval.typespec) = (yyvsp[(3) - (5)].typespec);
	  ;}
    break;

  case 58:
#line 401 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_boolean);
	  ;}
    break;

  case 59:
#line 406 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_string);
	  ;}
    break;

  case 60:
#line 411 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_filename);
	  ;}
    break;

  case 61:
#line 418 "../interface.yy"
    {
	      (yyval.afterbuild) = parser->createAfterBuild((yyvsp[(4) - (5)].word));
	  ;}
    break;

  case 62:
#line 424 "../interface.yy"
    {
	      (yyval.targettype) = parser->createTargetType((yyvsp[(4) - (5)].token));
	  ;}
    break;

  case 63:
#line 428 "../interface.yy"
    {
	      (yyval.targettype) = parser->createTargetType((yyvsp[(4) - (5)].token));
	  ;}
    break;

  case 64:
#line 434 "../interface.yy"
    {
	      (yyval.words) = parser->createWords((yyvsp[(1) - (1)].word)->getLocation());
	      (yyval.words)->append((yyvsp[(1) - (1)].word));
	  ;}
    break;

  case 65:
#line 439 "../interface.yy"
    {
	      (yyvsp[(1) - (3)].words)->append((yyvsp[(3) - (3)].word));
	      (yyval.words) = (yyvsp[(1) - (3)].words);
	  ;}
    break;

  case 66:
#line 446 "../interface.yy"
    {
	      (yyval.word) = (yyvsp[(1) - (1)].word);
	  ;}
    break;

  case 67:
#line 450 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].word)->appendWord((yyvsp[(2) - (2)].word));
	      (yyval.word) = (yyvsp[(1) - (2)].word);
	  ;}
    break;

  case 68:
#line 456 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendVariable((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 69:
#line 461 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendString((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 70:
#line 466 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendString((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 71:
#line 471 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendEnvironment((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 72:
#line 476 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendString((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 73:
#line 481 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendString((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 91:
#line 507 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;

  case 92:
#line 511 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(2) - (2)].token);
	  ;}
    break;

  case 93:
#line 517 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;

  case 94:
#line 521 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2171 "interface.tab.cc"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (parser, YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (parser, yymsg);
	  }
	else
	  {
	    yyerror (parser, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, parser);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, parser);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (parser, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, parser);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, parser);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 531 "../interface.yy"


