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
     tok_parameter = 288,
     tok_function = 289,
     tok_variable = 290,
     tok_other = 291
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
#define tok_parameter 288
#define tok_function 289
#define tok_variable 290
#define tok_other 291




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
#line 202 "interface.tab.cc"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 215 "interface.tab.cc"

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
#define YYLAST   364

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  37
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  34
/* YYNRULES -- Number of rules.  */
#define YYNRULES  97
/* YYNRULES -- Number of states.  */
#define YYNSTATES  169

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   291

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
      35,    36
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    11,    13,    15,    17,
      19,    21,    23,    26,    28,    31,    34,    38,    42,    47,
      50,    52,    55,    58,    61,    66,    69,    74,    81,    86,
      90,    94,   100,   104,   108,   112,   116,   119,   123,   126,
     130,   133,   136,   141,   143,   144,   146,   148,   152,   155,
     162,   167,   173,   175,   179,   183,   185,   191,   197,   199,
     201,   203,   208,   213,   218,   220,   224,   226,   229,   231,
     233,   235,   237,   239,   241,   243,   245,   247,   249,   251,
     253,   255,   257,   259,   261,   263,   265,   267,   269,   271,
     273,   275,   277,   279,   282,   284,   286,   287
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      38,     0,    -1,    39,    -1,    -1,    39,    40,    -1,    42,
      -1,    47,    -1,    48,    -1,    57,    -1,    62,    -1,    63,
      -1,    41,    -1,     4,    40,    -1,    69,    -1,     1,    68,
      -1,    43,    52,    -1,    43,    44,    52,    -1,    43,    46,
      52,    -1,    43,    44,    46,    52,    -1,    49,    39,    -1,
      45,    -1,    44,    45,    -1,    50,    39,    -1,    51,    39,
      -1,    14,     4,    31,    68,    -1,    15,    68,    -1,    16,
       4,    31,    68,    -1,    31,    70,     7,    70,    64,    68,
      -1,    31,    70,     7,    68,    -1,    17,     4,    48,    -1,
      18,     4,    48,    -1,    19,     4,    31,     4,    48,    -1,
      10,    53,    68,    -1,    10,     1,    68,    -1,    12,    53,
      68,    -1,    12,     1,    68,    -1,    11,    68,    -1,    11,
       1,    68,    -1,    13,    68,    -1,    13,     1,    68,    -1,
      35,     9,    -1,    56,     9,    -1,    54,     8,    70,    55,
      -1,    55,    -1,    -1,    64,    -1,    56,    -1,    34,    54,
       9,    -1,    58,    68,    -1,    58,    70,     7,    70,    64,
      68,    -1,    58,    70,     7,    68,    -1,    20,     4,    31,
       4,    59,    -1,    60,    -1,    27,     4,    60,    -1,    28,
       4,    60,    -1,    61,    -1,    24,     4,    61,     4,    25,
      -1,    24,     4,    61,     4,    26,    -1,    21,    -1,    22,
      -1,    23,    -1,    29,     4,    65,    68,    -1,    30,     4,
      31,    68,    -1,    30,     4,    35,    68,    -1,    65,    -1,
      64,     4,    65,    -1,    66,    -1,    65,    66,    -1,    35,
      -1,     6,    -1,    31,    -1,    32,    -1,    33,    -1,    36,
      -1,    67,    -1,    14,    -1,    15,    -1,    16,    -1,    17,
      -1,    18,    -1,    19,    -1,    20,    -1,    21,    -1,    22,
      -1,    23,    -1,    24,    -1,    25,    -1,    26,    -1,    27,
      -1,    28,    -1,    29,    -1,    30,    -1,    69,    -1,     4,
      69,    -1,     3,    -1,     5,    -1,    -1,     4,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   106,   106,   114,   117,   127,   131,   135,   139,   143,
     147,   151,   155,   161,   165,   173,   177,   181,   185,   191,
     197,   202,   209,   215,   221,   225,   229,   235,   239,   243,
     248,   253,   260,   264,   272,   276,   284,   288,   296,   300,
     308,   312,   318,   323,   331,   334,   338,   344,   350,   354,
     359,   366,   372,   376,   381,   388,   392,   397,   404,   409,
     414,   421,   427,   431,   437,   442,   449,   453,   459,   464,
     469,   474,   479,   484,   489,   496,   497,   498,   499,   500,
     501,   502,   503,   504,   505,   506,   507,   508,   509,   510,
     511,   512,   515,   519,   525,   529,   536,   537
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
  "tok_environment", "tok_parameter", "tok_function", "tok_variable",
  "tok_other", "$accept", "start", "blocks", "block", "ignore", "ifblock",
  "if", "elseifs", "elseif", "else", "reset", "assignment", "ifstatement",
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
     285,   286,   287,   288,   289,   290,   291
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    37,    38,    39,    39,    40,    40,    40,    40,    40,
      40,    40,    40,    41,    41,    42,    42,    42,    42,    43,
      44,    44,    45,    46,    47,    47,    47,    48,    48,    48,
      48,    48,    49,    49,    50,    50,    51,    51,    52,    52,
      53,    53,    54,    54,    55,    55,    55,    56,    57,    57,
      57,    58,    59,    59,    59,    60,    60,    60,    61,    61,
      61,    62,    63,    63,    64,    64,    65,    65,    66,    66,
      66,    66,    66,    66,    66,    67,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    68,    68,    69,    69,    70,    70
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     2,     1,     2,     2,     3,     3,     4,     2,
       1,     2,     2,     2,     4,     2,     4,     6,     4,     3,
       3,     5,     3,     3,     3,     3,     2,     3,     2,     3,
       2,     2,     4,     1,     0,     1,     1,     3,     2,     6,
       4,     5,     1,     3,     3,     1,     5,     5,     1,     1,
       1,     4,     4,     4,     1,     3,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     1,     1,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     0,    94,     0,    95,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    96,     4,
      11,     5,     0,     6,     7,     3,     8,    96,     9,    10,
      13,     0,    14,    92,    12,     0,    44,     0,     0,     0,
       0,    25,     0,     0,     0,     0,     0,     0,     0,    97,
       0,     0,     0,     0,     0,    20,     0,     3,     3,    15,
       0,    97,    48,     0,    93,    33,    69,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    70,    71,    72,    68,    73,     0,
      43,    46,    45,    64,    66,    74,    40,    32,    41,     0,
       0,    29,    30,     0,     0,     0,     0,     0,    96,     0,
      36,     0,     0,     0,    38,    21,     0,    16,    17,     0,
       0,    96,    96,    47,     0,    67,    24,    26,     0,     0,
      61,    62,    63,    28,     0,    37,    35,    34,    39,    18,
      50,     0,    44,    65,    31,    58,    59,    60,     0,     0,
       0,    51,    52,    55,     0,     0,    42,     0,     0,     0,
       0,    27,    49,     0,    53,    54,     0,    56,    57
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,    19,    20,    21,    22,    54,    55,    56,
      23,    24,    25,    57,    58,    59,    38,    89,    90,    39,
      26,    27,   151,   152,   153,    28,    29,    92,    93,    94,
      95,    32,    33,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -124
static const yytype_int16 yypact[] =
{
    -124,    20,   182,  -124,    52,  -124,   333,  -124,     7,    27,
      52,    32,    36,    40,    47,    58,    66,    85,    90,  -124,
    -124,  -124,    55,  -124,  -124,  -124,  -124,    72,  -124,  -124,
    -124,    23,  -124,  -124,  -124,    52,   200,    86,    52,    87,
      71,  -124,    81,     6,     6,    84,   108,   231,   -14,  -124,
     109,    49,    12,    60,    55,  -124,   129,  -124,  -124,  -124,
     267,    23,  -124,   136,  -124,  -124,  -124,  -124,  -124,  -124,
    -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,
    -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,  -124,    77,
    -124,  -124,   140,   231,  -124,  -124,  -124,  -124,  -124,    52,
      52,  -124,  -124,   141,   148,   145,    52,    52,    72,    52,
    -124,    52,    52,    52,  -124,  -124,   129,  -124,  -124,   289,
     311,    72,    90,  -124,   231,  -124,  -124,  -124,     6,    11,
    -124,  -124,  -124,  -124,   231,  -124,  -124,  -124,  -124,  -124,
    -124,   231,   200,   231,  -124,  -124,  -124,  -124,   149,   150,
     151,  -124,  -124,  -124,    76,    76,  -124,    61,    50,    50,
     105,  -124,  -124,   152,  -124,  -124,   -11,  -124,  -124
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -124,  -124,    -9,   173,  -124,  -124,  -124,  -124,   103,   130,
    -124,   -38,  -124,  -124,  -124,   -47,   137,  -124,    46,   -33,
    -124,  -124,  -124,   -71,    33,  -124,  -124,  -123,   -46,   -83,
    -124,    -8,    -2,   -15
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -24
static const yytype_int16 yytable[] =
{
      30,   105,    41,    91,    30,   101,   102,   117,    35,   118,
     125,   154,    63,   111,   167,   168,    60,   106,   155,    62,
       3,   107,   125,    12,    13,    14,     5,    65,     7,    64,
      97,    40,   145,   146,   147,   148,    42,    18,   149,   150,
      43,    36,    37,   110,    44,   114,    36,    37,   119,   120,
     109,    45,     5,    31,     7,     5,    31,     7,    30,    64,
     125,   113,    46,     5,    31,     7,    51,    52,    53,   139,
      47,   145,   146,   147,   148,     5,    61,     7,   143,     5,
     160,     7,   145,   146,   147,   122,   123,   164,   165,    48,
     144,   126,   127,   134,    49,    96,    98,   130,   131,   132,
     133,   135,    99,   136,   137,   138,   141,   142,     5,    91,
       7,    66,   100,   140,   143,   103,   108,    30,    30,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,   104,
      87,    88,    53,   121,   124,   128,   161,   162,     5,    31,
       7,    66,   129,   157,   158,   159,   166,   115,    64,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    34,
      87,    88,    -2,     4,   116,     5,     6,     7,   156,   112,
     163,     0,     8,     0,     0,     0,     9,    10,    11,    12,
      13,    14,    15,     0,     0,     0,    66,     0,     0,     0,
       0,    16,    17,    18,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    36,    87,    88,    66,     0,     0,
       0,     0,     0,     0,     0,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,     0,    87,    88,     4,     0,
       5,     6,     7,     0,     0,     0,     0,     8,   -19,   -19,
     -19,     9,    10,    11,    12,    13,    14,    15,     0,     0,
       4,     0,     5,     6,     7,     0,    16,    17,    18,     8,
     -22,   -22,   -22,     9,    10,    11,    12,    13,    14,    15,
       0,     0,     4,     0,     5,     6,     7,     0,    16,    17,
      18,     8,     0,     0,   -23,     9,    10,    11,    12,    13,
      14,    15,     0,     0,     4,     0,     5,     6,     7,     0,
      16,    17,    18,     8,     0,     0,     0,     9,    10,    11,
      12,    13,    14,    15,     0,     0,     0,     0,     0,     0,
       0,     0,    16,    17,    18
};

static const yytype_int16 yycheck[] =
{
       2,    47,    10,    36,     6,    43,    44,    54,     1,    56,
      93,   134,    27,     1,    25,    26,    25,    31,   141,    27,
       0,    35,   105,    17,    18,    19,     3,    35,     5,    31,
      38,     4,    21,    22,    23,    24,     4,    31,    27,    28,
       4,    34,    35,    51,     4,    53,    34,    35,    57,    58,
       1,     4,     3,     4,     5,     3,     4,     5,    60,    61,
     143,     1,     4,     3,     4,     5,    11,    12,    13,   116,
       4,    21,    22,    23,    24,     3,     4,     5,   124,     3,
       4,     5,    21,    22,    23,     8,     9,   158,   159,     4,
     128,    99,   100,   108,     4,     9,     9,   105,   106,   107,
     108,   109,    31,   111,   112,   113,   121,   122,     3,   142,
       5,     6,    31,   121,   160,    31,     7,   119,   120,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    31,
      35,    36,    13,     7,     4,     4,   154,   155,     3,     4,
       5,     6,     4,     4,     4,     4,     4,    54,   160,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,     6,
      35,    36,     0,     1,    54,     3,     4,     5,   142,    52,
     157,    -1,    10,    -1,    -1,    -1,    14,    15,    16,    17,
      18,    19,    20,    -1,    -1,    -1,     6,    -1,    -1,    -1,
      -1,    29,    30,    31,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,     6,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    -1,    35,    36,     1,    -1,
       3,     4,     5,    -1,    -1,    -1,    -1,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    -1,    -1,
       1,    -1,     3,     4,     5,    -1,    29,    30,    31,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      -1,    -1,     1,    -1,     3,     4,     5,    -1,    29,    30,
      31,    10,    -1,    -1,    13,    14,    15,    16,    17,    18,
      19,    20,    -1,    -1,     1,    -1,     3,     4,     5,    -1,
      29,    30,    31,    10,    -1,    -1,    -1,    14,    15,    16,
      17,    18,    19,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    29,    30,    31
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    38,    39,     0,     1,     3,     4,     5,    10,    14,
      15,    16,    17,    18,    19,    20,    29,    30,    31,    40,
      41,    42,    43,    47,    48,    49,    57,    58,    62,    63,
      69,     4,    68,    69,    40,     1,    34,    35,    53,    56,
       4,    68,     4,     4,     4,     4,     4,     4,     4,     4,
      70,    11,    12,    13,    44,    45,    46,    50,    51,    52,
      39,     4,    68,    70,    69,    68,     6,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    35,    36,    54,
      55,    56,    64,    65,    66,    67,     9,    68,     9,    31,
      31,    48,    48,    31,    31,    65,    31,    35,     7,     1,
      68,     1,    53,     1,    68,    45,    46,    52,    52,    39,
      39,     7,     8,     9,     4,    66,    68,    68,     4,     4,
      68,    68,    68,    68,    70,    68,    68,    68,    68,    52,
      68,    70,    70,    65,    48,    21,    22,    23,    24,    27,
      28,    59,    60,    61,    64,    64,    55,     4,     4,     4,
       4,    68,    68,    61,    60,    60,     4,    25,    26
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
#line 107 "../interface.yy"
    {
	      parser->acceptParseTree((yyvsp[(1) - (1)].blocks));
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 3:
#line 114 "../interface.yy"
    {
	      (yyval.blocks) = parser->createBlocks();
	  ;}
    break;

  case 4:
#line 118 "../interface.yy"
    {
	      if ((yyvsp[(2) - (2)].block))
	      {
		  (yyvsp[(1) - (2)].blocks)->addBlock((yyvsp[(2) - (2)].block));
		  (yyval.blocks) = (yyvsp[(1) - (2)].blocks);
	      }
	  ;}
    break;

  case 5:
#line 128 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].ifblock));
	  ;}
    break;

  case 6:
#line 132 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].reset));
	  ;}
    break;

  case 7:
#line 136 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].assignment));
	  ;}
    break;

  case 8:
#line 140 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].declaration));
	  ;}
    break;

  case 9:
#line 144 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].afterbuild));
	  ;}
    break;

  case 10:
#line 148 "../interface.yy"
    {
	      (yyval.block) = parser->createBlock((yyvsp[(1) - (1)].targettype));
	  ;}
    break;

  case 11:
#line 152 "../interface.yy"
    {
	      (yyval.block) = 0;
	  ;}
    break;

  case 12:
#line 156 "../interface.yy"
    {
	      (yyval.block) = (yyvsp[(2) - (2)].block);
	  ;}
    break;

  case 13:
#line 162 "../interface.yy"
    {
	      (yyval.block) = 0;
	  ;}
    break;

  case 14:
#line 166 "../interface.yy"
    {
	      parser->error((yyvsp[(2) - (2)].token)->getLocation(),
			    "a parse error occured on or before this line");
	      (yyval.block) = 0;
	  ;}
    break;

  case 15:
#line 174 "../interface.yy"
    {
	      (yyval.ifblock) = parser->createIfBlock((yyvsp[(1) - (2)].ifclause), 0, 0);
	  ;}
    break;

  case 16:
#line 178 "../interface.yy"
    {
	      (yyval.ifblock) = parser->createIfBlock((yyvsp[(1) - (3)].ifclause), (yyvsp[(2) - (3)].ifclauses), 0);
	  ;}
    break;

  case 17:
#line 182 "../interface.yy"
    {
	      (yyval.ifblock) = parser->createIfBlock((yyvsp[(1) - (3)].ifclause), 0, (yyvsp[(2) - (3)].ifclause));
	  ;}
    break;

  case 18:
#line 186 "../interface.yy"
    {
	      (yyval.ifblock) = parser->createIfBlock((yyvsp[(1) - (4)].ifclause), (yyvsp[(2) - (4)].ifclauses), (yyvsp[(3) - (4)].ifclause));
	  ;}
    break;

  case 19:
#line 192 "../interface.yy"
    {
	      (yyval.ifclause) = parser->createIfClause((yyvsp[(1) - (2)].conditional), (yyvsp[(2) - (2)].blocks), true);
	  ;}
    break;

  case 20:
#line 198 "../interface.yy"
    {
	      (yyval.ifclauses) = parser->createIfClauses((yyvsp[(1) - (1)].ifclause)->getLocation());
	      (yyval.ifclauses)->addClause((yyvsp[(1) - (1)].ifclause));
	  ;}
    break;

  case 21:
#line 203 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].ifclauses)->addClause((yyvsp[(2) - (2)].ifclause));
	      (yyval.ifclauses) = (yyvsp[(1) - (2)].ifclauses);
	  ;}
    break;

  case 22:
#line 210 "../interface.yy"
    {
	      (yyval.ifclause) = parser->createIfClause((yyvsp[(1) - (2)].conditional), (yyvsp[(2) - (2)].blocks), true);
	  ;}
    break;

  case 23:
#line 216 "../interface.yy"
    {
	      (yyval.ifclause) = parser->createIfClause(0, (yyvsp[(2) - (2)].blocks), false);
	  ;}
    break;

  case 24:
#line 222 "../interface.yy"
    {
	      (yyval.reset) = parser->createReset((yyvsp[(3) - (4)].token), false);
	  ;}
    break;

  case 25:
#line 226 "../interface.yy"
    {
	      (yyval.reset) = parser->createReset((yyvsp[(1) - (2)].token)->getLocation());
	  ;}
    break;

  case 26:
#line 230 "../interface.yy"
    {
	      (yyval.reset) = parser->createReset((yyvsp[(3) - (4)].token), true);
	  ;}
    break;

  case 27:
#line 236 "../interface.yy"
    {
	      (yyval.assignment) = parser->createAssignment((yyvsp[(1) - (6)].token), (yyvsp[(5) - (6)].words));
	  ;}
    break;

  case 28:
#line 240 "../interface.yy"
    {
	      (yyval.assignment) = parser->createAssignment((yyvsp[(1) - (4)].token), parser->createEmptyWords());
	  ;}
    break;

  case 29:
#line 244 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].assignment)->setAssignmentType(Interface::a_override);
	      (yyval.assignment) = (yyvsp[(3) - (3)].assignment);
	  ;}
    break;

  case 30:
#line 249 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].assignment)->setAssignmentType(Interface::a_fallback);
	      (yyval.assignment) = (yyvsp[(3) - (3)].assignment);
	  ;}
    break;

  case 31:
#line 254 "../interface.yy"
    {
	      (yyvsp[(5) - (5)].assignment)->setFlag((yyvsp[(3) - (5)].token));
	      (yyval.assignment) = (yyvsp[(5) - (5)].assignment);
	  ;}
    break;

  case 32:
#line 261 "../interface.yy"
    {
	      (yyval.conditional) = (yyvsp[(2) - (3)].conditional);
	  ;}
    break;

  case 33:
#line 265 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse if statement");
	      (yyval.conditional) = 0;
	  ;}
    break;

  case 34:
#line 273 "../interface.yy"
    {
	      (yyval.conditional) = (yyvsp[(2) - (3)].conditional);
	  ;}
    break;

  case 35:
#line 277 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse elseif statement");
	      (yyval.conditional) = 0;
	  ;}
    break;

  case 36:
#line 285 "../interface.yy"
    {
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 37:
#line 289 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse else statement");
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 38:
#line 297 "../interface.yy"
    {
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 39:
#line 301 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse endif statement");
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 40:
#line 309 "../interface.yy"
    {
	      (yyval.conditional) = parser->createConditional((yyvsp[(1) - (2)].token));
	  ;}
    break;

  case 41:
#line 313 "../interface.yy"
    {
	      (yyval.conditional) = parser->createConditional((yyvsp[(1) - (2)].function));
	  ;}
    break;

  case 42:
#line 319 "../interface.yy"
    {
	      (yyvsp[(1) - (4)].arguments)->appendArgument((yyvsp[(4) - (4)].argument));
	      (yyval.arguments) = (yyvsp[(1) - (4)].arguments);
	  ;}
    break;

  case 43:
#line 324 "../interface.yy"
    {
	      (yyval.arguments) = parser->createArguments((yyvsp[(1) - (1)].argument)->getLocation());
	      (yyval.arguments)->appendArgument((yyvsp[(1) - (1)].argument));
	  ;}
    break;

  case 44:
#line 331 "../interface.yy"
    {
	      (yyval.argument) = parser->createArgument(parser->createEmptyWords());
	  ;}
    break;

  case 45:
#line 335 "../interface.yy"
    {
	      (yyval.argument) = parser->createArgument((yyvsp[(1) - (1)].words));
	  ;}
    break;

  case 46:
#line 339 "../interface.yy"
    {
	      (yyval.argument) = parser->createArgument((yyvsp[(1) - (1)].function));
	  ;}
    break;

  case 47:
#line 345 "../interface.yy"
    {
	      (yyval.function) = parser->createFunction((yyvsp[(1) - (3)].token), (yyvsp[(2) - (3)].arguments));
	  ;}
    break;

  case 48:
#line 351 "../interface.yy"
    {
	      (yyval.declaration) = (yyvsp[(1) - (2)].declaration);
	  ;}
    break;

  case 49:
#line 355 "../interface.yy"
    {
	      (yyvsp[(1) - (6)].declaration)->addInitializer((yyvsp[(5) - (6)].words));
	      (yyval.declaration) = (yyvsp[(1) - (6)].declaration);
	  ;}
    break;

  case 50:
#line 360 "../interface.yy"
    {
	      (yyvsp[(1) - (4)].declaration)->addInitializer(parser->createEmptyWords());
	      (yyval.declaration) = (yyvsp[(1) - (4)].declaration);
	  ;}
    break;

  case 51:
#line 367 "../interface.yy"
    {
	      (yyval.declaration) = parser->createDeclaration((yyvsp[(3) - (5)].token), (yyvsp[(5) - (5)].typespec));
	  ;}
    break;

  case 52:
#line 373 "../interface.yy"
    {
	      (yyval.typespec) = (yyvsp[(1) - (1)].typespec);
	  ;}
    break;

  case 53:
#line 377 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].typespec)->setScope(Interface::s_nonrecursive);
	      (yyval.typespec) = (yyvsp[(3) - (3)].typespec);
	  ;}
    break;

  case 54:
#line 382 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].typespec)->setScope(Interface::s_local);
	      (yyval.typespec) = (yyvsp[(3) - (3)].typespec);
	  ;}
    break;

  case 55:
#line 389 "../interface.yy"
    {
	      (yyval.typespec) = (yyvsp[(1) - (1)].typespec);
	  ;}
    break;

  case 56:
#line 393 "../interface.yy"
    {
	      (yyvsp[(3) - (5)].typespec)->setListType(Interface::l_append);
	      (yyval.typespec) = (yyvsp[(3) - (5)].typespec);
	  ;}
    break;

  case 57:
#line 398 "../interface.yy"
    {
	      (yyvsp[(3) - (5)].typespec)->setListType(Interface::l_prepend);
	      (yyval.typespec) = (yyvsp[(3) - (5)].typespec);
	  ;}
    break;

  case 58:
#line 405 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_boolean);
	  ;}
    break;

  case 59:
#line 410 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_string);
	  ;}
    break;

  case 60:
#line 415 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_filename);
	  ;}
    break;

  case 61:
#line 422 "../interface.yy"
    {
	      (yyval.afterbuild) = parser->createAfterBuild((yyvsp[(3) - (4)].word));
	  ;}
    break;

  case 62:
#line 428 "../interface.yy"
    {
	      (yyval.targettype) = parser->createTargetType((yyvsp[(3) - (4)].token));
	  ;}
    break;

  case 63:
#line 432 "../interface.yy"
    {
	      (yyval.targettype) = parser->createTargetType((yyvsp[(3) - (4)].token));
	  ;}
    break;

  case 64:
#line 438 "../interface.yy"
    {
	      (yyval.words) = parser->createWords((yyvsp[(1) - (1)].word)->getLocation());
	      (yyval.words)->append((yyvsp[(1) - (1)].word));
	  ;}
    break;

  case 65:
#line 443 "../interface.yy"
    {
	      (yyvsp[(1) - (3)].words)->append((yyvsp[(3) - (3)].word));
	      (yyval.words) = (yyvsp[(1) - (3)].words);
	  ;}
    break;

  case 66:
#line 450 "../interface.yy"
    {
	      (yyval.word) = (yyvsp[(1) - (1)].word);
	  ;}
    break;

  case 67:
#line 454 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].word)->appendWord((yyvsp[(2) - (2)].word));
	      (yyval.word) = (yyvsp[(1) - (2)].word);
	  ;}
    break;

  case 68:
#line 460 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendVariable((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 69:
#line 465 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendString((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 70:
#line 470 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendString((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 71:
#line 475 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendEnvironment((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 72:
#line 480 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendParameter((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 73:
#line 485 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendString((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 74:
#line 490 "../interface.yy"
    {
	      (yyval.word) = parser->createWord();
	      (yyval.word)->appendString((yyvsp[(1) - (1)].token));
	  ;}
    break;

  case 92:
#line 516 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;

  case 93:
#line 520 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(2) - (2)].token);
	  ;}
    break;

  case 94:
#line 526 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;

  case 95:
#line 530 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2191 "interface.tab.cc"
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


#line 540 "../interface.yy"


