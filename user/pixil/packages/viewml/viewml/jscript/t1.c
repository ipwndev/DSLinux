/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

#define yylex yyjscriptlex
#define yylval yyjscriptlval

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     FUNCTION = 258,
     IF = 259,
     ELSE = 260,
     IN = 261,
     WITH = 262,
     WHILE = 263,
     FOR = 264,
     SHIFT_LEFT = 265,
     SHIFT_RIGHT = 266,
     EQ = 267,
     NEQ = 268,
     OR = 269,
     AND = 270,
     THIS = 271,
     B_NULL = 272,
     FLOAT = 273,
     B_TRUE = 274,
     B_FALSE = 275,
     NEW = 276,
     DELETE = 277,
     BREAK = 278,
     CONTINUE = 279,
     RETURN = 280,
     VAR = 281,
     PP = 282,
     MM = 283,
     STRING = 284,
     LEQ = 285,
     GEQ = 286,
     MAS = 287,
     DAS = 288,
     AAS = 289,
     SAS = 290,
     PAS = 291,
     RAS = 292,
     BAAS = 293,
     BOAS = 294,
     NUM = 295,
     IDENTIFIER = 296
   };
#endif
#define FUNCTION 258
#define IF 259
#define ELSE 260
#define IN 261
#define WITH 262
#define WHILE 263
#define FOR 264
#define SHIFT_LEFT 265
#define SHIFT_RIGHT 266
#define EQ 267
#define NEQ 268
#define OR 269
#define AND 270
#define THIS 271
#define B_NULL 272
#define FLOAT 273
#define B_TRUE 274
#define B_FALSE 275
#define NEW 276
#define DELETE 277
#define BREAK 278
#define CONTINUE 279
#define RETURN 280
#define VAR 281
#define PP 282
#define MM 283
#define STRING 284
#define LEQ 285
#define GEQ 286
#define MAS 287
#define DAS 288
#define AAS 289
#define SAS 290
#define PAS 291
#define RAS 292
#define BAAS 293
#define BOAS 294
#define NUM 295
#define IDENTIFIER 296




/* Copy the first part of user declarations.  */
#line 1 "../jscript/t1.y"


#include "bison2cpp.h"

void yyerror(char *s);
int yylex();
void initFlex( const char *_code );



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 12 "../jscript/t1.y"
typedef union YYSTYPE {
     int vali;
     float vald;
     char *name;
     void *ptr;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 174 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 186 "y.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
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
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   296

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  64
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  37
/* YYNRULES -- Number of rules. */
#define YYNRULES  112
/* YYNRULES -- Number of states. */
#define YYNSTATES  200

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   296

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      44,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    54,     2,
      42,    43,    59,    57,    45,    58,    61,    60,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    51,    48,
      55,    49,    56,    50,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,    63,    53,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    46,    52,    47,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     4,     7,    14,    16,    19,    20,    22,
      24,    28,    32,    33,    36,    38,    40,    44,    50,    54,
      62,    70,    76,    79,    82,    88,    92,    94,    97,    98,
     100,   104,   107,   110,   113,   115,   117,   121,   123,   127,
     128,   130,   132,   135,   137,   141,   145,   149,   153,   157,
     161,   165,   169,   173,   175,   181,   183,   187,   189,   193,
     195,   199,   201,   205,   207,   211,   213,   217,   221,   223,
     227,   231,   235,   239,   241,   245,   249,   251,   255,   259,
     261,   265,   269,   271,   274,   277,   280,   283,   286,   289,
     292,   296,   298,   300,   305,   309,   311,   316,   321,   323,
     327,   328,   330,   332,   336,   340,   342,   344,   346,   348,
     350,   352,   354
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      65,     0,    -1,    -1,    65,    66,    -1,     3,    41,    42,
      67,    43,    69,    -1,    70,    -1,     1,    44,    -1,    -1,
      68,    -1,    41,    -1,    41,    45,    68,    -1,    46,    70,
      47,    -1,    -1,    71,    70,    -1,    69,    -1,    48,    -1,
       4,    73,    71,    -1,     4,    73,    71,     5,    71,    -1,
       8,    73,    71,    -1,    74,    48,    79,    48,    79,    43,
      71,    -1,    75,    48,    79,    48,    79,    43,    71,    -1,
      75,     6,    80,    43,    71,    -1,    23,    72,    -1,    24,
      72,    -1,     7,    42,    80,    43,    71,    -1,    25,    79,
      72,    -1,    69,    -1,    76,    72,    -1,    -1,    48,    -1,
      42,    80,    43,    -1,     9,    42,    -1,    74,    76,    -1,
      26,    77,    -1,    81,    -1,    78,    -1,    78,    45,    77,
      -1,    41,    -1,    41,    49,    81,    -1,    -1,    80,    -1,
      81,    -1,    81,    80,    -1,    82,    -1,    82,    49,    81,
      -1,    82,    32,    81,    -1,    82,    33,    81,    -1,    82,
      34,    81,    -1,    82,    35,    81,    -1,    82,    36,    81,
      -1,    82,    37,    81,    -1,    82,    38,    81,    -1,    82,
      39,    81,    -1,    83,    -1,    83,    50,    81,    51,    81,
      -1,    84,    -1,    84,    14,    83,    -1,    85,    -1,    85,
      15,    84,    -1,    86,    -1,    86,    52,    85,    -1,    87,
      -1,    87,    53,    86,    -1,    88,    -1,    88,    54,    87,
      -1,    89,    -1,    89,    12,    88,    -1,    89,    13,    88,
      -1,    90,    -1,    89,    55,    90,    -1,    89,    56,    90,
      -1,    89,    30,    90,    -1,    89,    31,    90,    -1,    91,
      -1,    91,    10,    90,    -1,    91,    11,    90,    -1,    92,
      -1,    92,    57,    91,    -1,    92,    58,    91,    -1,    93,
      -1,    93,    59,    92,    -1,    93,    60,    92,    -1,    96,
      -1,    58,    93,    -1,    27,    96,    -1,    28,    96,    -1,
      96,    27,    -1,    96,    28,    -1,    21,    94,    -1,    22,
      96,    -1,    16,    61,    95,    -1,    95,    -1,    41,    -1,
      41,    42,    98,    43,    -1,    41,    61,    95,    -1,    97,
      -1,    97,    62,    80,    63,    -1,    97,    42,    98,    43,
      -1,   100,    -1,    96,    61,    41,    -1,    -1,    99,    -1,
      81,    -1,    81,    45,    99,    -1,    42,    80,    43,    -1,
      41,    -1,    40,    -1,    18,    -1,    29,    -1,    20,    -1,
      19,    -1,    17,    -1,    16,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    93,    93,    94,    97,    98,    99,   102,   103,   106,
     107,   110,   113,   114,   115,   118,   119,   120,   121,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   133,   134,
     137,   140,   143,   146,   147,   150,   151,   154,   155,   158,
     159,   162,   163,   166,   167,   168,   169,   170,   171,   172,
     173,   174,   175,   178,   179,   182,   183,   186,   187,   190,
     191,   194,   195,   198,   199,   202,   203,   204,   207,   208,
     209,   210,   211,   214,   215,   216,   219,   220,   221,   224,
     225,   226,   229,   230,   231,   232,   233,   234,   235,   236,
     239,   240,   243,   244,   245,   248,   249,   250,   253,   254,
     257,   258,   261,   262,   265,   266,   267,   268,   269,   270,
     271,   272,   273
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "FUNCTION", "IF", "ELSE", "IN", "WITH", 
  "WHILE", "FOR", "SHIFT_LEFT", "SHIFT_RIGHT", "EQ", "NEQ", "OR", "AND", 
  "THIS", "B_NULL", "FLOAT", "B_TRUE", "B_FALSE", "NEW", "DELETE", 
  "BREAK", "CONTINUE", "RETURN", "VAR", "PP", "MM", "STRING", "LEQ", 
  "GEQ", "MAS", "DAS", "AAS", "SAS", "PAS", "RAS", "BAAS", "BOAS", "NUM", 
  "IDENTIFIER", "'('", "')'", "'\\n'", "','", "'{'", "'}'", "';'", "'='", 
  "'?'", "':'", "'|'", "'^'", "'&'", "'<'", "'>'", "'+'", "'-'", "'*'", 
  "'/'", "'.'", "'['", "']'", "$accept", "input", "element", 
  "parameterListOpt", "parameterList", "compoundStatement", "statements", 
  "statement", "semiOpt", "condition", "forParen", "forBegin", 
  "variablesOrExpression", "variables", "variable", "expressionOpt", 
  "expression", "assignmentExpression", "conditionalExpression", 
  "orExpression", "andExpression", "bitwiseOrExpression", 
  "bitwiseXorExpression", "bitwiseAndExpression", "equalityExpression", 
  "relationalExpression", "shiftExpression", "additiveExpression", 
  "multiplicativeExpression", "unaryExpression", "constructor", 
  "constructorCall", "simpleExpression", "memberExpression", 
  "argumentListOpt", "argumentList", "primaryExpression", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,    40,    41,    10,    44,   123,   125,    59,    61,
      63,    58,   124,    94,    38,    60,    62,    43,    45,    42,
      47,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    64,    65,    65,    66,    66,    66,    67,    67,    68,
      68,    69,    70,    70,    70,    71,    71,    71,    71,    71,
      71,    71,    71,    71,    71,    71,    71,    71,    72,    72,
      73,    74,    75,    76,    76,    77,    77,    78,    78,    79,
      79,    80,    80,    81,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    82,    82,    83,    83,    84,    84,    85,
      85,    86,    86,    87,    87,    88,    88,    88,    89,    89,
      89,    89,    89,    90,    90,    90,    91,    91,    91,    92,
      92,    92,    93,    93,    93,    93,    93,    93,    93,    93,
      94,    94,    95,    95,    95,    96,    96,    96,    97,    97,
      98,    98,    99,    99,   100,   100,   100,   100,   100,   100,
     100,   100,   100
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     6,     1,     2,     0,     1,     1,
       3,     3,     0,     2,     1,     1,     3,     5,     3,     7,
       7,     5,     2,     2,     5,     3,     1,     2,     0,     1,
       3,     2,     2,     2,     1,     1,     3,     1,     3,     0,
       1,     1,     2,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     1,     5,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     3,     3,     1,     3,
       3,     3,     3,     1,     3,     3,     1,     3,     3,     1,
       3,     3,     1,     2,     2,     2,     2,     2,     2,     2,
       3,     1,     1,     4,     3,     1,     4,     4,     1,     3,
       0,     1,     1,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,   112,
     111,   107,   110,   109,     0,     0,    28,    28,    39,     0,
       0,     0,   108,   106,   105,     0,    12,    15,     0,     3,
      14,     5,    12,     0,     0,    28,    34,    43,    53,    55,
      57,    59,    61,    63,    65,    68,    73,    76,    79,    82,
      95,    98,     6,     0,     0,     0,     0,     0,    31,     0,
      92,    88,    91,    89,    29,    22,    23,    28,    40,    41,
      37,    33,    35,    84,    85,     0,     0,    83,    13,    39,
      32,     0,    39,    27,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    86,    87,     0,   100,     0,     7,     0,    26,    16,
       0,    18,     0,   100,     0,    25,    42,     0,     0,   104,
      11,     0,     0,     0,    45,    46,    47,    48,    49,    50,
      51,    52,    44,     0,    56,    58,    60,    62,    64,    66,
      67,    71,    72,    69,    70,    74,    75,    77,    78,    80,
      81,    99,   102,     0,   101,     0,     9,     0,     8,    30,
       0,     0,    90,     0,    94,    38,    36,    39,     0,    39,
       0,     0,    97,    96,     0,     0,    17,    24,    93,     0,
      21,     0,    54,   103,    10,     4,     0,     0,    19,    20
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,     1,    29,   167,   168,   118,    31,    32,    65,    55,
      33,    34,    35,    71,    72,    67,    68,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      61,    62,    49,    50,   163,   164,    51
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -77
static const short yypact[] =
{
     -77,   129,   -77,   -32,   -15,    -4,     9,    -4,    15,   -77,
     -77,   -77,   -77,   -77,    -9,     1,    18,    18,   238,     6,
       1,     1,   -77,   -77,   -77,   238,   172,   -77,   238,   -77,
     -77,   -77,   172,   205,     7,    18,   -77,    45,    13,    19,
      78,    44,    49,    55,    -2,   -77,    12,    34,    40,   -13,
     -18,   -77,   -77,    71,   238,   172,   238,   172,   -77,    53,
     -26,   -77,   -77,    54,   -77,   -77,   -77,    18,   -77,   238,
      67,   -77,    74,    54,    54,    77,    75,   -77,   -77,   238,
     -77,   238,   238,   -77,   238,   238,   238,   238,   238,   238,
     238,   238,   238,   238,   238,   238,   238,   238,   238,   238,
     238,   238,   238,   238,   238,   238,   238,   238,   238,   238,
     238,   -77,   -77,    80,   238,   238,    82,    81,   -77,   121,
      84,   -77,    87,   238,    87,   -77,   -77,   238,     6,   -77,
     -77,    83,    91,    92,   -77,   -77,   -77,   -77,   -77,   -77,
     -77,   -77,   -77,    88,   -77,   -77,   -77,   -77,   -77,   -77,
     -77,   -77,   -77,   -77,   -77,   -77,   -77,   -77,   -77,   -77,
     -77,   -77,    90,    98,   -77,    79,   114,   118,   -77,   -77,
     172,   172,   -77,   122,   -77,   -77,   -77,   238,   172,   238,
     238,   238,   -77,   -77,    82,   120,   -77,   -77,   -77,   124,
     -77,   125,   -77,   -77,   -77,   -77,   172,   172,   -77,   -77
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
     -77,   -77,   -77,   -77,   -12,    -1,    24,   -53,    -8,   166,
     -77,   -77,   141,    50,   -77,   -76,   -20,   -17,   -77,    89,
     107,    86,   106,   108,     5,   -77,   -16,     0,     2,   157,
     -77,   -64,    25,   -77,    63,    23,   -77
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      30,    69,   119,   131,   121,    75,   133,    59,    69,    66,
      99,   100,    52,    81,   111,   112,   123,     9,    10,    11,
      12,    13,   105,   106,   114,    30,    53,    83,   101,   102,
      22,    30,    60,    94,   117,   124,   120,    69,    54,    69,
      63,    23,    24,    25,   115,    73,    74,    70,   113,   126,
      76,    56,    69,   103,   104,    82,    78,    58,   172,   125,
     174,   132,    69,    93,    69,    69,    64,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,    84,    85,    86,
      87,    88,    89,    90,    91,   151,   152,   153,   154,   155,
     156,   107,   108,    95,    92,   165,    96,   162,    69,   109,
     110,   189,    97,   191,   149,   150,   162,   157,   158,    98,
     175,   159,   160,   116,   122,   113,   127,   186,   187,   128,
     129,   161,   130,   166,   169,   190,   170,   171,    60,     2,
       3,   177,     4,     5,   178,   181,     6,     7,     8,   180,
     179,   182,   183,   198,   199,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,   184,
      69,   185,    69,   192,   162,   188,    26,   196,   197,    23,
      24,    25,   194,    57,    80,    26,     5,    27,   176,     6,
       7,     8,   146,   144,   195,    77,   173,    28,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,   145,   147,   193,     0,   148,     0,     0,     0,
       0,     0,    23,    24,    25,     0,     0,     0,    26,     0,
      27,     9,    10,    11,    12,    13,    14,    15,     0,     0,
      28,    19,    20,    21,    22,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    23,    24,    25,     0,     0,
       0,     0,     0,    79,     9,    10,    11,    12,    13,    14,
      15,     0,     0,    28,     0,    20,    21,    22,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    23,    24,
      25,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    28
};

static const short yycheck[] =
{
       1,    18,    55,    79,    57,    25,    82,    16,    25,    17,
      12,    13,    44,     6,    27,    28,    42,    16,    17,    18,
      19,    20,    10,    11,    42,    26,    41,    35,    30,    31,
      29,    32,    41,    14,    54,    61,    56,    54,    42,    56,
      15,    40,    41,    42,    62,    20,    21,    41,    61,    69,
      26,    42,    69,    55,    56,    48,    32,    42,   122,    67,
     124,    81,    79,    50,    81,    82,    48,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    32,    33,    34,
      35,    36,    37,    38,    39,   101,   102,   103,   104,   105,
     106,    57,    58,    15,    49,   115,    52,   114,   115,    59,
      60,   177,    53,   179,    99,   100,   123,   107,   108,    54,
     127,   109,   110,    42,    61,    61,    49,   170,   171,    45,
      43,    41,    47,    41,    43,   178,     5,    43,    41,     0,
       1,    48,     3,     4,    43,    45,     7,     8,     9,    51,
      48,    43,    63,   196,   197,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    45,
     177,    43,   179,   180,   181,    43,    46,    43,    43,    40,
      41,    42,   184,     7,    33,    46,     4,    48,   128,     7,
       8,     9,    96,    94,   185,    28,   123,    58,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    95,    97,   181,    -1,    98,    -1,    -1,    -1,
      -1,    -1,    40,    41,    42,    -1,    -1,    -1,    46,    -1,
      48,    16,    17,    18,    19,    20,    21,    22,    -1,    -1,
      58,    26,    27,    28,    29,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    41,    42,    -1,    -1,
      -1,    -1,    -1,    48,    16,    17,    18,    19,    20,    21,
      22,    -1,    -1,    58,    -1,    27,    28,    29,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    40,    41,
      42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    65,     0,     1,     3,     4,     7,     8,     9,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    40,    41,    42,    46,    48,    58,    66,
      69,    70,    71,    74,    75,    76,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    96,
      97,   100,    44,    41,    42,    73,    42,    73,    42,    16,
      41,    94,    95,    96,    48,    72,    72,    79,    80,    81,
      41,    77,    78,    96,    96,    80,    70,    93,    70,    48,
      76,     6,    48,    72,    32,    33,    34,    35,    36,    37,
      38,    39,    49,    50,    14,    15,    52,    53,    54,    12,
      13,    30,    31,    55,    56,    10,    11,    57,    58,    59,
      60,    27,    28,    61,    42,    62,    42,    80,    69,    71,
      80,    71,    61,    42,    61,    72,    80,    49,    45,    43,
      47,    79,    80,    79,    81,    81,    81,    81,    81,    81,
      81,    81,    81,    81,    83,    84,    85,    86,    87,    88,
      88,    90,    90,    90,    90,    90,    90,    91,    91,    92,
      92,    41,    81,    98,    99,    80,    41,    67,    68,    43,
       5,    43,    95,    98,    95,    81,    77,    48,    43,    48,
      51,    45,    43,    63,    45,    43,    71,    71,    43,    79,
      71,    79,    81,    99,    68,    69,    43,    43,    71,    71
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
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
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
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
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

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
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
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

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
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

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
        case 3:
#line 94 "../jscript/t1.y"
    { printf("!!!!! Adding code !!!!!\n"); jsAppendCode( yyvsp[0].ptr ); }
    break;

  case 4:
#line 97 "../jscript/t1.y"
    { printf("Function: '%s'\n",yyvsp[-4].name ); yyval.ptr = newJSFunction( yyvsp[-4].name, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 5:
#line 98 "../jscript/t1.y"
    { printf("Statement\n"); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 6:
#line 99 "../jscript/t1.y"
    { yyerrok; yyval.ptr = 0L; }
    break;

  case 7:
#line 102 "../jscript/t1.y"
    { yyval.ptr = 0L; }
    break;

  case 8:
#line 103 "../jscript/t1.y"
    { yyval.ptr = yyvsp[0].ptr; }
    break;

  case 9:
#line 106 "../jscript/t1.y"
    { printf("param '%s'\n", yyvsp[0].name); yyval.ptr = newJSParameter( yyvsp[0].name, 0L ); }
    break;

  case 10:
#line 107 "../jscript/t1.y"
    { printf("param '%s\n", yyvsp[-2].name); yyval.ptr = newJSParameter( yyvsp[-2].name, yyvsp[0].ptr ); }
    break;

  case 11:
#line 110 "../jscript/t1.y"
    { yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 12:
#line 113 "../jscript/t1.y"
    { yyval.ptr = 0L; }
    break;

  case 13:
#line 114 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSStatement( yyvsp[-1].ptr, yyvsp[0].ptr ); }
    break;

  case 14:
#line 115 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 15:
#line 118 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 16:
#line 119 "../jscript/t1.y"
    { printf("Simple IF\n"); yyval.ptr = 0L; }
    break;

  case 17:
#line 120 "../jscript/t1.y"
    { printf("Complex IF\n"); yyval.ptr = 0L; }
    break;

  case 18:
#line 121 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 19:
#line 122 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 20:
#line 123 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 21:
#line 124 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 22:
#line 125 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 23:
#line 126 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 24:
#line 127 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 25:
#line 128 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 26:
#line 129 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 27:
#line 130 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 29:
#line 134 "../jscript/t1.y"
    { printf(" "); }
    break;

  case 30:
#line 137 "../jscript/t1.y"
    { printf("Condition\n"); }
    break;

  case 31:
#line 140 "../jscript/t1.y"
    { printf(" "); }
    break;

  case 32:
#line 143 "../jscript/t1.y"
    { printf(" "); }
    break;

  case 33:
#line 146 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 34:
#line 147 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 35:
#line 150 "../jscript/t1.y"
    { printf(" "); }
    break;

  case 36:
#line 151 "../jscript/t1.y"
    { printf(" "); }
    break;

  case 37:
#line 154 "../jscript/t1.y"
    { printf("Var: '%s'\n", yyvsp[0].name); }
    break;

  case 38:
#line 155 "../jscript/t1.y"
    { printf("Var with Assignment: '%s'\n", yyvsp[-2].name); }
    break;

  case 39:
#line 158 "../jscript/t1.y"
    { yyval.ptr = 0L; }
    break;

  case 40:
#line 159 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 41:
#line 162 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 42:
#line 163 "../jscript/t1.y"
    { printf(" "); }
    break;

  case 43:
#line 166 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 44:
#line 167 "../jscript/t1.y"
    { printf("Assignment ( = )\n"); yyval.ptr = newJSAssignment( OP_ASSIGN, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 45:
#line 168 "../jscript/t1.y"
    { printf("Assignment ( *= )\n"); }
    break;

  case 46:
#line 169 "../jscript/t1.y"
    { printf("Assignment ( /= )\n"); }
    break;

  case 47:
#line 170 "../jscript/t1.y"
    { printf("Assignment ( += )\n"); }
    break;

  case 48:
#line 171 "../jscript/t1.y"
    { printf("Assignment ( -= )\n"); }
    break;

  case 49:
#line 172 "../jscript/t1.y"
    { printf("Assignment ( ^= )\n"); }
    break;

  case 50:
#line 173 "../jscript/t1.y"
    { printf("Assignment ( %%= )\n"); }
    break;

  case 51:
#line 174 "../jscript/t1.y"
    { printf("Assignment ( &= )\n"); }
    break;

  case 52:
#line 175 "../jscript/t1.y"
    { printf("Assignment ( |= )\n"); }
    break;

  case 53:
#line 178 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 54:
#line 179 "../jscript/t1.y"
    { printf(" "); }
    break;

  case 55:
#line 182 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 56:
#line 183 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_OR, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 57:
#line 186 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 58:
#line 187 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_AND, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 59:
#line 190 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 60:
#line 191 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_BOR, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 61:
#line 194 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 62:
#line 195 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_BXOR, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 63:
#line 198 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 64:
#line 199 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_BAND, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 65:
#line 202 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 66:
#line 203 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_EQ, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 67:
#line 204 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_NEQ, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 68:
#line 207 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 69:
#line 208 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_LT, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 70:
#line 209 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_GT, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 71:
#line 210 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_LEQ, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 72:
#line 211 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_GEQ, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 73:
#line 214 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 74:
#line 215 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_SL, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 75:
#line 216 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBinaryOperator( OP_SR, yyvsp[-2].ptr, yyvsp[0].ptr );}
    break;

  case 76:
#line 219 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 77:
#line 220 "../jscript/t1.y"
    { printf("Add ( + )\n"); yyval.ptr = newJSBinaryOperator( OP_ADD, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 78:
#line 221 "../jscript/t1.y"
    { printf("Sub ( - )\n"); yyval.ptr = newJSBinaryOperator( OP_SUB, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 79:
#line 224 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 80:
#line 225 "../jscript/t1.y"
    { printf("Mul ( * )\n"); yyval.ptr = newJSBinaryOperator( OP_MUL, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 81:
#line 226 "../jscript/t1.y"
    { printf("Div ( / )\n"); yyval.ptr = newJSBinaryOperator( OP_DIV, yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 82:
#line 229 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 83:
#line 230 "../jscript/t1.y"
    { printf("Unary Minus\n"); }
    break;

  case 84:
#line 231 "../jscript/t1.y"
    { printf("++ Prefix\n"); }
    break;

  case 85:
#line 232 "../jscript/t1.y"
    { printf("-- Prefix\n"); }
    break;

  case 86:
#line 233 "../jscript/t1.y"
    { printf("Postfix ++\n"); }
    break;

  case 87:
#line 234 "../jscript/t1.y"
    { printf("Postfix --\n"); }
    break;

  case 88:
#line 235 "../jscript/t1.y"
    { printf("new\n"); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 89:
#line 236 "../jscript/t1.y"
    { printf("delete\n"); }
    break;

  case 90:
#line 239 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 91:
#line 240 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 92:
#line 243 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSConstructorCall( newJSIdentifier( yyvsp[0].name ) , 0L ); }
    break;

  case 93:
#line 244 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSConstructorCall( newJSIdentifier( yyvsp[-3].name ), yyvsp[-1].ptr ); }
    break;

  case 94:
#line 245 "../jscript/t1.y"
    { printf(" "); yyval.ptr = 0L; }
    break;

  case 95:
#line 248 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 96:
#line 249 "../jscript/t1.y"
    { printf("[ ]\n"); yyval.ptr = newJSArrayAccess( yyvsp[-3].ptr, yyvsp[-1].ptr ); }
    break;

  case 97:
#line 250 "../jscript/t1.y"
    { printf("Function Call\n"); yyval.ptr = newJSFunctionCall( yyvsp[-3].ptr, yyvsp[-1].ptr ); }
    break;

  case 98:
#line 253 "../jscript/t1.y"
    { printf(" "); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 99:
#line 254 "../jscript/t1.y"
    { printf("Member ( '%s' )\n", yyvsp[0].name ); yyval.ptr = newJSMember( yyvsp[-2].ptr, yyvsp[0].name ); }
    break;

  case 100:
#line 257 "../jscript/t1.y"
    { yyval.ptr = 0L; }
    break;

  case 101:
#line 258 "../jscript/t1.y"
    { printf("ArgumentList\n"); yyval.ptr = yyvsp[0].ptr; }
    break;

  case 102:
#line 261 "../jscript/t1.y"
    { printf("Argument\n"); yyval.ptr = newJSArgument( yyvsp[0].ptr, 0L ); }
    break;

  case 103:
#line 262 "../jscript/t1.y"
    { printf("Argument (cont)\n"); yyval.ptr = newJSArgument( yyvsp[-2].ptr, yyvsp[0].ptr ); }
    break;

  case 104:
#line 265 "../jscript/t1.y"
    { printf("Paranthesis\n"); yyval.ptr = yyvsp[-1].ptr; }
    break;

  case 105:
#line 266 "../jscript/t1.y"
    { printf("ID '%s'\n",yyvsp[0].name); yyval.ptr = newJSIdentifier( yyvsp[0].name ); }
    break;

  case 106:
#line 267 "../jscript/t1.y"
    { printf("NUM\n"); yyval.ptr = newJSInteger( yyvsp[0].vali ); }
    break;

  case 107:
#line 268 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSFloat( yyvsp[0].vald ); }
    break;

  case 108:
#line 269 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSString( yyvsp[0].name ); }
    break;

  case 109:
#line 270 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBool( 0 ); }
    break;

  case 110:
#line 271 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSBool( 1 ); }
    break;

  case 111:
#line 272 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSNull(); }
    break;

  case 112:
#line 273 "../jscript/t1.y"
    { printf(" "); yyval.ptr = newJSThis(); }
    break;


    }

/* Line 991 of yacc.c.  */
#line 1817 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
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

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 278 "../jscript/t1.y"


void yyerror ( char *s )  /* Called by yyparse on error */
{
    printf ("ERROR: %s\n", s);
}

void mainParse( const char *_code )
{
    initFlex( _code );
    yyparse();
}








