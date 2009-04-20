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
     tok_comment = 259,
     tok_spaces = 260,
     tok_newline = 261,
     tok_quotedchar = 262,
     tok_equal = 263,
     tok_comma = 264,
     tok_clope = 265,
     tok_continue = 266,
     tok_if = 267,
     tok_else = 268,
     tok_elseif = 269,
     tok_endif = 270,
     tok_reset = 271,
     tok_reset_all = 272,
     tok_no_reset = 273,
     tok_override = 274,
     tok_fallback = 275,
     tok_flag = 276,
     tok_declare = 277,
     tok_boolean = 278,
     tok_string = 279,
     tok_filename = 280,
     tok_list = 281,
     tok_append = 282,
     tok_prepend = 283,
     tok_nonrecursive = 284,
     tok_local = 285,
     tok_afterbuild = 286,
     tok_targettype = 287,
     tok_identifier = 288,
     tok_environment = 289,
     tok_function = 290,
     tok_variable = 291,
     tok_other = 292
   };
#endif
/* Tokens.  */
#define tok_EOF 258
#define tok_comment 259
#define tok_spaces 260
#define tok_newline 261
#define tok_quotedchar 262
#define tok_equal 263
#define tok_comma 264
#define tok_clope 265
#define tok_continue 266
#define tok_if 267
#define tok_else 268
#define tok_elseif 269
#define tok_endif 270
#define tok_reset 271
#define tok_reset_all 272
#define tok_no_reset 273
#define tok_override 274
#define tok_fallback 275
#define tok_flag 276
#define tok_declare 277
#define tok_boolean 278
#define tok_string 279
#define tok_filename 280
#define tok_list 281
#define tok_append 282
#define tok_prepend 283
#define tok_nonrecursive 284
#define tok_local 285
#define tok_afterbuild 286
#define tok_targettype 287
#define tok_identifier 288
#define tok_environment 289
#define tok_function 290
#define tok_variable 291
#define tok_other 292




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
#line 204 "interface.tab.cc"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 217 "interface.tab.cc"

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
#define YYLAST   207

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  31
/* YYNRULES -- Number of rules.  */
#define YYNRULES  72
/* YYNRULES -- Number of states.  */
#define YYNSTATES  125

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   292

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
      35,    36,    37
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     6,     9,    12,    14,    17,    19,
      21,    23,    25,    27,    29,    32,    36,    40,    45,    48,
      50,    53,    56,    59,    63,    66,    70,    75,    78,    81,
      85,    88,    92,    96,   100,   104,   107,   111,   114,   118,
     121,   124,   128,   130,   132,   134,   138,   141,   146,   150,
     152,   155,   158,   160,   164,   168,   170,   172,   174,   178,
     182,   186,   188,   192,   193,   196,   199,   202,   205,   208,
     210,   212,   214
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      39,     0,    -1,    40,    -1,    -1,    40,    42,    -1,    40,
      41,    -1,    68,    -1,     1,    68,    -1,    43,    -1,    48,
      -1,    49,    -1,    58,    -1,    63,    -1,    64,    -1,    44,
      53,    -1,    44,    45,    53,    -1,    44,    47,    53,    -1,
      44,    45,    47,    53,    -1,    50,    40,    -1,    46,    -1,
      45,    46,    -1,    51,    40,    -1,    52,    40,    -1,    16,
      33,    68,    -1,    17,    68,    -1,    18,    33,    68,    -1,
      33,     8,    65,    67,    -1,    19,    49,    -1,    20,    49,
      -1,    21,    33,    49,    -1,     5,    49,    -1,    12,    54,
      68,    -1,    12,     1,    68,    -1,    14,    54,    68,    -1,
      14,     1,    68,    -1,    13,    68,    -1,    13,     1,    68,
      -1,    15,    68,    -1,    15,     1,    68,    -1,    36,    10,
      -1,    57,    10,    -1,    55,     9,    56,    -1,    56,    -1,
      65,    -1,    57,    -1,    35,    55,    10,    -1,    59,    68,
      -1,    59,     8,    65,    67,    -1,    22,    33,    60,    -1,
      61,    -1,    29,    61,    -1,    30,    61,    -1,    62,    -1,
      26,    62,    27,    -1,    26,    62,    28,    -1,    23,    -1,
      24,    -1,    25,    -1,    31,    65,    67,    -1,    32,    33,
      68,    -1,    32,    36,    68,    -1,    66,    -1,    65,     5,
      66,    -1,    -1,    66,    36,    -1,    66,     7,    -1,    66,
      33,    -1,    66,    34,    -1,    66,    37,    -1,     3,    -1,
       6,    -1,    67,    -1,     5,    67,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   105,   105,   113,   116,   121,   127,   131,   139,   143,
     147,   151,   155,   159,   165,   169,   173,   177,   183,   189,
     194,   201,   207,   213,   217,   221,   227,   231,   236,   241,
     246,   252,   256,   264,   268,   276,   280,   288,   292,   300,
     304,   310,   315,   322,   326,   332,   338,   342,   349,   355,
     359,   364,   371,   375,   380,   387,   392,   397,   404,   410,
     414,   420,   425,   433,   436,   441,   446,   451,   456,   464,
     468,   474,   478
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "tok_EOF", "tok_comment", "tok_spaces",
  "tok_newline", "tok_quotedchar", "tok_equal", "tok_comma", "tok_clope",
  "tok_continue", "tok_if", "tok_else", "tok_elseif", "tok_endif",
  "tok_reset", "tok_reset_all", "tok_no_reset", "tok_override",
  "tok_fallback", "tok_flag", "tok_declare", "tok_boolean", "tok_string",
  "tok_filename", "tok_list", "tok_append", "tok_prepend",
  "tok_nonrecursive", "tok_local", "tok_afterbuild", "tok_targettype",
  "tok_identifier", "tok_environment", "tok_function", "tok_variable",
  "tok_other", "$accept", "start", "blocks", "ignore", "block", "ifblock",
  "if", "elseifs", "elseif", "else", "reset", "assignment", "ifstatement",
  "elseifstatement", "elsestatement", "endifstatement", "conditional",
  "arguments", "argument", "function", "declaration", "declbody",
  "typespec", "listtypespec", "basetypespec", "afterbuild", "targettype",
  "words", "word", "nospaceendofline", "endofline", 0
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
     285,   286,   287,   288,   289,   290,   291,   292
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    38,    39,    40,    40,    40,    41,    41,    42,    42,
      42,    42,    42,    42,    43,    43,    43,    43,    44,    45,
      45,    46,    47,    48,    48,    48,    49,    49,    49,    49,
      49,    50,    50,    51,    51,    52,    52,    53,    53,    54,
      54,    55,    55,    56,    56,    57,    58,    58,    59,    60,
      60,    60,    61,    61,    61,    62,    62,    62,    63,    64,
      64,    65,    65,    66,    66,    66,    66,    66,    66,    67,
      67,    68,    68
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     1,     2,     1,     1,
       1,     1,     1,     1,     2,     3,     3,     4,     2,     1,
       2,     2,     2,     3,     2,     3,     4,     2,     2,     3,
       2,     3,     3,     3,     3,     2,     3,     2,     3,     2,
       2,     3,     1,     1,     1,     3,     2,     4,     3,     1,
       2,     2,     1,     3,     3,     1,     1,     1,     3,     3,
       3,     1,     3,     0,     2,     2,     2,     2,     2,     1,
       1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     0,    69,     0,    70,     0,     0,
       0,     0,     0,     0,     0,     0,    63,     0,     0,     5,
       4,     8,     0,     9,    10,     3,    11,     0,    12,    13,
      71,     6,     0,     7,     0,    30,    72,     0,    63,     0,
       0,     0,     0,    24,     0,    27,    28,     0,     0,     0,
      61,     0,     0,    63,     0,     0,     0,     0,    19,     0,
       3,     3,    14,     0,    63,    46,    32,     0,    42,    44,
      43,    39,    31,    40,    23,    25,    29,    55,    56,    57,
       0,     0,     0,    48,    49,    52,    63,    58,    65,    66,
      67,    64,    68,    59,    60,     0,     0,    35,     0,     0,
       0,    37,    20,     0,    15,    16,     0,     0,     0,    63,
      45,     0,    50,    51,    62,    26,    36,    34,    33,    38,
      17,    47,    41,    53,    54
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,    19,    20,    21,    22,    57,    58,    59,
      23,    24,    25,    60,    61,    62,    40,    67,    68,    41,
      26,    27,    83,    84,    85,    28,    29,    70,    50,    30,
      31
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -54
static const yytype_int16 yypact[] =
{
     -54,     3,    98,   -54,    16,   -54,    11,   -54,    19,   -24,
      16,     7,    55,    55,     9,    14,   -54,   -23,    17,   -54,
     -54,   -54,    72,   -54,   -54,   -54,   -54,   103,   -54,   -54,
     -54,   -54,    12,   -54,    55,   -54,   -54,    16,    22,    42,
      16,    53,    16,   -54,    16,   -54,   -54,    55,    54,   201,
      31,    16,    16,   -54,   186,    26,   197,    72,   -54,    51,
     -54,   -54,   -54,   120,   -54,   -54,   -54,   118,   -54,   -54,
      76,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,   -54,
      67,    46,    46,   -54,   -54,   -54,   -54,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   -54,   201,    16,   -54,    16,    16,
      16,   -54,   -54,    51,   -54,   -54,   142,   164,   201,    22,
     -54,     1,   -54,   -54,    31,   -54,   -54,   -54,   -54,   -54,
     -54,   -54,   -54,   -54,   -54
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -54,   -54,    -2,   -54,   -54,   -54,   -54,   -54,    25,    37,
     -54,    -1,   -54,   -54,   -54,   -53,    45,   -54,    -4,   -36,
     -54,   -54,   -54,   -46,    27,   -54,   -54,    -8,    36,    -6,
      -3
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -23
static const yytype_int8 yytable[] =
{
      36,    33,    69,     3,   104,    35,   105,    43,    49,    42,
      51,    45,    46,    52,     5,     5,    34,     7,     7,     5,
      37,    32,     7,    63,    65,    53,    36,    98,   123,   124,
      12,    13,    14,    35,    66,   112,   113,    72,    88,    74,
      44,    75,    47,    87,    18,    95,    76,    48,    93,    94,
     120,    97,    71,   101,    38,    39,   108,    38,   106,   107,
      34,    38,    39,    73,    89,    90,    56,    91,    92,    77,
      78,    79,    80,    69,    12,    13,    14,    77,    78,    79,
      80,    86,   102,    81,    82,    54,    55,    56,    18,   115,
      77,    78,    79,   116,   103,   117,   118,   119,    -2,     4,
      99,     5,   121,     6,     7,   122,     5,   111,    32,     7,
       8,    64,     0,     0,     9,    10,    11,    12,    13,    14,
      15,     4,   114,     5,     0,     6,     7,   109,   110,    16,
      17,    18,     8,   -18,   -18,   -18,     9,    10,    11,    12,
      13,    14,    15,     4,     0,     5,     0,     6,     7,     0,
       0,    16,    17,    18,     8,   -21,   -21,   -21,     9,    10,
      11,    12,    13,    14,    15,     4,     0,     5,     0,     6,
       7,     0,     0,    16,    17,    18,     8,     0,     0,   -22,
       9,    10,    11,    12,    13,    14,    15,    96,     0,     5,
       0,    32,     7,     0,     0,    16,    17,    18,   100,     0,
       5,     0,    32,     7,     5,     0,    86,     7
};

static const yytype_int8 yycheck[] =
{
       6,     4,    38,     0,    57,     6,    59,    10,    16,    33,
      33,    12,    13,    36,     3,     3,     5,     6,     6,     3,
       1,     5,     6,    25,    27,     8,    32,     1,    27,    28,
      19,    20,    21,    34,    37,    81,    82,    40,     7,    42,
      33,    44,    33,    49,    33,    53,    47,    33,    51,    52,
     103,    54,    10,    56,    35,    36,    64,    35,    60,    61,
       5,    35,    36,    10,    33,    34,    15,    36,    37,    23,
      24,    25,    26,   109,    19,    20,    21,    23,    24,    25,
      26,     5,    57,    29,    30,    13,    14,    15,    33,    95,
      23,    24,    25,    96,    57,    98,    99,   100,     0,     1,
      55,     3,   108,     5,     6,   109,     3,    80,     5,     6,
      12,     8,    -1,    -1,    16,    17,    18,    19,    20,    21,
      22,     1,    86,     3,    -1,     5,     6,     9,    10,    31,
      32,    33,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     1,    -1,     3,    -1,     5,     6,    -1,
      -1,    31,    32,    33,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     1,    -1,     3,    -1,     5,
       6,    -1,    -1,    31,    32,    33,    12,    -1,    -1,    15,
      16,    17,    18,    19,    20,    21,    22,     1,    -1,     3,
      -1,     5,     6,    -1,    -1,    31,    32,    33,     1,    -1,
       3,    -1,     5,     6,     3,    -1,     5,     6
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    39,    40,     0,     1,     3,     5,     6,    12,    16,
      17,    18,    19,    20,    21,    22,    31,    32,    33,    41,
      42,    43,    44,    48,    49,    50,    58,    59,    63,    64,
      67,    68,     5,    68,     5,    49,    67,     1,    35,    36,
      54,    57,    33,    68,    33,    49,    49,    33,    33,    65,
      66,    33,    36,     8,    13,    14,    15,    45,    46,    47,
      51,    52,    53,    40,     8,    68,    68,    55,    56,    57,
      65,    10,    68,    10,    68,    68,    49,    23,    24,    25,
      26,    29,    30,    60,    61,    62,     5,    67,     7,    33,
      34,    36,    37,    68,    68,    65,     1,    68,     1,    54,
       1,    68,    46,    47,    53,    53,    40,    40,    65,     9,
      10,    62,    61,    61,    66,    67,    68,    68,    68,    68,
      53,    67,    56,    27,    28
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
	      (yyval.reset) = parser->createReset((yyvsp[(2) - (3)].token), false);
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
	      (yyval.reset) = parser->createReset((yyvsp[(2) - (3)].token), true);
	  ;}
    break;

  case 26:
#line 228 "../interface.yy"
    {
	      (yyval.assignment) = parser->createAssignment((yyvsp[(1) - (4)].token), (yyvsp[(3) - (4)].words));
	  ;}
    break;

  case 27:
#line 232 "../interface.yy"
    {
	      (yyvsp[(2) - (2)].assignment)->setAssignmentType(Interface::a_override);
	      (yyval.assignment) = (yyvsp[(2) - (2)].assignment);
	  ;}
    break;

  case 28:
#line 237 "../interface.yy"
    {
	      (yyvsp[(2) - (2)].assignment)->setAssignmentType(Interface::a_fallback);
	      (yyval.assignment) = (yyvsp[(2) - (2)].assignment);
	  ;}
    break;

  case 29:
#line 242 "../interface.yy"
    {
	      (yyvsp[(3) - (3)].assignment)->setFlag((yyvsp[(2) - (3)].token));
	      (yyval.assignment) = (yyvsp[(3) - (3)].assignment);
	  ;}
    break;

  case 30:
#line 247 "../interface.yy"
    {
	      (yyval.assignment) = (yyvsp[(2) - (2)].assignment);
	  ;}
    break;

  case 31:
#line 253 "../interface.yy"
    {
	      (yyval.conditional) = (yyvsp[(2) - (3)].conditional);
	  ;}
    break;

  case 32:
#line 257 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse if statement");
	      (yyval.conditional) = 0;
	  ;}
    break;

  case 33:
#line 265 "../interface.yy"
    {
	      (yyval.conditional) = (yyvsp[(2) - (3)].conditional);
	  ;}
    break;

  case 34:
#line 269 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse elseif statement");
	      (yyval.conditional) = 0;
	  ;}
    break;

  case 35:
#line 277 "../interface.yy"
    {
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 36:
#line 281 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse else statement");
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 37:
#line 289 "../interface.yy"
    {
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 38:
#line 293 "../interface.yy"
    {
	      parser->error((yyvsp[(1) - (3)].token)->getLocation(),
			    "unable to parse endif statement");
	      (yyval.not_used) = 0;
	  ;}
    break;

  case 39:
#line 301 "../interface.yy"
    {
	      (yyval.conditional) = parser->createConditional((yyvsp[(1) - (2)].token));
	  ;}
    break;

  case 40:
#line 305 "../interface.yy"
    {
	      (yyval.conditional) = parser->createConditional((yyvsp[(1) - (2)].function));
	  ;}
    break;

  case 41:
#line 311 "../interface.yy"
    {
	      (yyvsp[(1) - (3)].arguments)->appendArgument((yyvsp[(3) - (3)].argument));
	      (yyval.arguments) = (yyvsp[(1) - (3)].arguments);
	  ;}
    break;

  case 42:
#line 316 "../interface.yy"
    {
	      (yyval.arguments) = parser->createArguments((yyvsp[(1) - (1)].argument)->getLocation());
	      (yyval.arguments)->appendArgument((yyvsp[(1) - (1)].argument));
	  ;}
    break;

  case 43:
#line 323 "../interface.yy"
    {
	      (yyval.argument) = parser->createArgument((yyvsp[(1) - (1)].words));
	  ;}
    break;

  case 44:
#line 327 "../interface.yy"
    {
	      (yyval.argument) = parser->createArgument((yyvsp[(1) - (1)].function));
	  ;}
    break;

  case 45:
#line 333 "../interface.yy"
    {
	      (yyval.function) = parser->createFunction((yyvsp[(1) - (3)].token), (yyvsp[(2) - (3)].arguments));
	  ;}
    break;

  case 46:
#line 339 "../interface.yy"
    {
	      (yyval.declaration) = (yyvsp[(1) - (2)].declaration);
	  ;}
    break;

  case 47:
#line 343 "../interface.yy"
    {
	      (yyvsp[(1) - (4)].declaration)->addInitializer((yyvsp[(3) - (4)].words));
	      (yyval.declaration) = (yyvsp[(1) - (4)].declaration);
	  ;}
    break;

  case 48:
#line 350 "../interface.yy"
    {
	      (yyval.declaration) = parser->createDeclaration((yyvsp[(2) - (3)].token), (yyvsp[(3) - (3)].typespec));
	  ;}
    break;

  case 49:
#line 356 "../interface.yy"
    {
	      (yyval.typespec) = (yyvsp[(1) - (1)].typespec);
	  ;}
    break;

  case 50:
#line 360 "../interface.yy"
    {
	      (yyvsp[(2) - (2)].typespec)->setScope(Interface::s_nonrecursive);
	      (yyval.typespec) = (yyvsp[(2) - (2)].typespec);
	  ;}
    break;

  case 51:
#line 365 "../interface.yy"
    {
	      (yyvsp[(2) - (2)].typespec)->setScope(Interface::s_local);
	      (yyval.typespec) = (yyvsp[(2) - (2)].typespec);
	  ;}
    break;

  case 52:
#line 372 "../interface.yy"
    {
	      (yyval.typespec) = (yyvsp[(1) - (1)].typespec);
	  ;}
    break;

  case 53:
#line 376 "../interface.yy"
    {
	      (yyvsp[(2) - (3)].typespec)->setListType(Interface::l_append);
	      (yyval.typespec) = (yyvsp[(2) - (3)].typespec);
	  ;}
    break;

  case 54:
#line 381 "../interface.yy"
    {
	      (yyvsp[(2) - (3)].typespec)->setListType(Interface::l_prepend);
	      (yyval.typespec) = (yyvsp[(2) - (3)].typespec);
	  ;}
    break;

  case 55:
#line 388 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_boolean);
	  ;}
    break;

  case 56:
#line 393 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_string);
	  ;}
    break;

  case 57:
#line 398 "../interface.yy"
    {
	      (yyval.typespec) = parser->createTypeSpec(
		  (yyvsp[(1) - (1)].token)->getLocation(), Interface::t_filename);
	  ;}
    break;

  case 58:
#line 405 "../interface.yy"
    {
	      (yyval.afterbuild) = parser->createAfterBuild((yyvsp[(2) - (3)].words));
	  ;}
    break;

  case 59:
#line 411 "../interface.yy"
    {
	      (yyval.targettype) = parser->createTargetType((yyvsp[(2) - (3)].token));
	  ;}
    break;

  case 60:
#line 415 "../interface.yy"
    {
	      (yyval.targettype) = parser->createTargetType((yyvsp[(2) - (3)].token));
	  ;}
    break;

  case 61:
#line 421 "../interface.yy"
    {
	      (yyval.words) = parser->createWords((yyvsp[(1) - (1)].word)->getLocation());
	      (yyval.words)->append((yyvsp[(1) - (1)].word));
	  ;}
    break;

  case 62:
#line 426 "../interface.yy"
    {
	      (yyvsp[(1) - (3)].words)->append((yyvsp[(3) - (3)].word));
	      (yyval.words) = (yyvsp[(1) - (3)].words);
	  ;}
    break;

  case 63:
#line 433 "../interface.yy"
    {
	      (yyval.word) = parser->createWord(parser->getLastFileLocation());
	  ;}
    break;

  case 64:
#line 437 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].word)->appendVariable((yyvsp[(2) - (2)].token));
	      (yyval.word) = (yyvsp[(1) - (2)].word);
	  ;}
    break;

  case 65:
#line 442 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].word)->appendString((yyvsp[(2) - (2)].token));
	      (yyval.word) = (yyvsp[(1) - (2)].word);
	  ;}
    break;

  case 66:
#line 447 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].word)->appendString((yyvsp[(2) - (2)].token));
	      (yyval.word) = (yyvsp[(1) - (2)].word);
	  ;}
    break;

  case 67:
#line 452 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].word)->appendEnvironment((yyvsp[(2) - (2)].token));
	      (yyval.word) = (yyvsp[(1) - (2)].word);
	  ;}
    break;

  case 68:
#line 457 "../interface.yy"
    {
	      (yyvsp[(1) - (2)].word)->appendString((yyvsp[(2) - (2)].token));
	      (yyval.word) = (yyvsp[(1) - (2)].word);
	  ;}
    break;

  case 69:
#line 465 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;

  case 70:
#line 469 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;

  case 71:
#line 475 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(1) - (1)].token);
	  ;}
    break;

  case 72:
#line 479 "../interface.yy"
    {
	      (yyval.token) = (yyvsp[(2) - (2)].token);
	  ;}
    break;


/* Line 1267 of yacc.c.  */
#line 2083 "interface.tab.cc"
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


#line 484 "../interface.yy"

