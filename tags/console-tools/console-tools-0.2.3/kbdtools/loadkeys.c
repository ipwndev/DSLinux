
/*  A Bison parser, made from loadkeys.y
 by  GNU Bison version 1.27
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	EOL	257
#define	NUMBER	258
#define	LITERAL	259
#define	CHARSET	260
#define	KEYMAPS	261
#define	KEYCODE	262
#define	EQUALS	263
#define	PLAIN	264
#define	SHIFT	265
#define	CONTROL	266
#define	ALT	267
#define	ALTGR	268
#define	SHIFTL	269
#define	SHIFTR	270
#define	CTRLL	271
#define	CTRLR	272
#define	COMMA	273
#define	DASH	274
#define	STRING	275
#define	STRLITERAL	276
#define	COMPOSE	277
#define	TO	278
#define	CCHAR	279
#define	ERROR	280
#define	PLUS	281
#define	UNUMBER	282
#define	ALT_IS_META	283
#define	STRINGS	284
#define	AS	285
#define	USUAL	286
#define	ON	287
#define	FOR	288

#line 62 "loadkeys.y"

#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/kd.h>
#include <linux/keyboard.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <sysexits.h>
#include <signal.h>

#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>
#include <lct/font.h>		/* findkeymap() */
#include <lct/ksyms.h>
#include <lct/modifiers.h>

int verbose;
void lk_push(void);				  /* in analyze.c */
int lk_pop(void);				  /* idem */
extern int infile_stack_ptr;			  /* idem */

#ifndef KT_LETTER
#define KT_LETTER KT_LATIN
#endif

#undef yywrap
int yywrap(void);

/* externs from analyse.l */
extern int line_nr, rvalct;
extern struct kbsentry kbs_buf;
extern FILE* yyin;

/* name to use in error messages */
char *progname;

/* give error message if using unicode compose strings? */
int unicode_warning = 0;
/* What keymaps are we defining? */
char defining[MAX_NR_KEYMAPS];
char keymaps_line_seen = 0;
int max_keymap = 0;		/* from here on, defining[] is false */
int alt_is_meta = 0;

/* the kernel structures we want to set or print */
u_short *key_map[MAX_NR_KEYMAPS];
char *func_table[MAX_NR_FUNC];
struct kbdiacr accent_table[MAX_DIACR];
unsigned int accent_table_size = 0;

char key_is_constant[NR_KEYS];
char *keymap_was_set[MAX_NR_KEYMAPS];
char func_buf[4096];		/* should be allocated dynamically */
char *fp = func_buf;

#define U(x) ((x) ^ 0xf000)

#undef ECHO

static void addmap(int map, int explicit);
static void addkey(int index, int table, int keycode);
static void addfunc(struct kbsentry kbs_buf);
static void killkey(int index, int table);
static void compose(int diacr, int base, int res);
static void do_constant(void);
static void do_constant_key (int, u_short);
static void loadkeys(void);
static void mktable(void);
static void strings_as_usual(void);
static void keypad_as_usual(char *keyboard);
static void function_keys_as_usual(char *keyboard);
static void consoles_as_usual(char *keyboard);
static void compose_as_usual(char *charset);
void lkfatal(char *);
void lkfatal0(char *, int);
void lkfatal1(char *, char *);
extern char *xstrdup(char *);
int key_buf[MAX_NR_KEYMAPS];
int mod;
int unicode_used;
int private_error_ct = 0;
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		88
#define	YYFLAG		-32768
#define	YYNTBASE	35

#define YYTRANSLATE(x) ((unsigned)(x) <= 288 ? yytranslate[x] : 54)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,     8,    10,    12,    14,    16,    18,
    20,    22,    24,    28,    31,    36,    43,    48,    52,    56,
    58,    62,    64,    70,    77,    84,    85,    93,   100,   103,
   105,   107,   109,   111,   113,   115,   117,   119,   121,   127,
   128,   131,   133,   135,   137,   140,   142
};

static const short yyrhs[] = {    -1,
    35,    36,     0,     3,     0,    37,     0,    38,     0,    39,
     0,    40,     0,    41,     0,    50,     0,    46,     0,    44,
     0,    45,     0,     6,    22,     3,     0,    29,     3,     0,
    30,    31,    32,     3,     0,    23,    31,    32,    34,    22,
     3,     0,    23,    31,    32,     3,     0,     7,    42,     3,
     0,    42,    19,    43,     0,    43,     0,     4,    20,     4,
     0,     4,     0,    21,     5,     9,    22,     3,     0,    23,
    25,    25,    24,    25,     3,     0,    23,    25,    25,    24,
    53,     3,     0,     0,    47,    48,     8,     4,     9,    53,
     3,     0,    10,     8,     4,     9,    53,     3,     0,    48,
    49,     0,    49,     0,    11,     0,    12,     0,    13,     0,
    14,     0,    15,     0,    16,     0,    17,     0,    18,     0,
     8,     4,     9,    51,     3,     0,     0,    52,    51,     0,
    53,     0,     4,     0,    28,     0,    27,     4,     0,     5,
     0,    27,     5,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   151,   152,   154,   155,   156,   157,   158,   159,   160,   161,
   162,   163,   165,   170,   175,   180,   184,   189,   194,   195,
   197,   203,   208,   217,   221,   226,   227,   231,   236,   237,
   239,   240,   241,   242,   243,   244,   245,   246,   248,   281,
   282,   284,   291,   293,   295,   297,   299
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","EOL","NUMBER",
"LITERAL","CHARSET","KEYMAPS","KEYCODE","EQUALS","PLAIN","SHIFT","CONTROL","ALT",
"ALTGR","SHIFTL","SHIFTR","CTRLL","CTRLR","COMMA","DASH","STRING","STRLITERAL",
"COMPOSE","TO","CCHAR","ERROR","PLUS","UNUMBER","ALT_IS_META","STRINGS","AS",
"USUAL","ON","FOR","keytable","line","charsetline","altismetaline","usualstringsline",
"usualcomposeline","keymapline","range","range0","strline","compline","singleline",
"@1","modifiers","modifier","fullline","rvalue0","rvalue1","rvalue", NULL
};
#endif

static const short yyr1[] = {     0,
    35,    35,    36,    36,    36,    36,    36,    36,    36,    36,
    36,    36,    37,    38,    39,    40,    40,    41,    42,    42,
    43,    43,    44,    45,    45,    47,    46,    46,    48,    48,
    49,    49,    49,    49,    49,    49,    49,    49,    50,    51,
    51,    52,    53,    53,    53,    53,    53
};

static const short yyr2[] = {     0,
     0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     3,     2,     4,     6,     4,     3,     3,     1,
     3,     1,     5,     6,     6,     0,     7,     6,     2,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     5,     0,
     2,     1,     1,     1,     2,     1,     2
};

static const short yydefact[] = {     1,
    26,     3,     0,     0,     0,     0,     0,     0,     0,     0,
     2,     4,     5,     6,     7,     8,    11,    12,    10,     0,
     9,     0,    22,     0,    20,     0,     0,     0,     0,     0,
    14,     0,    31,    32,    33,    34,    35,    36,    37,    38,
     0,    30,    13,     0,    18,     0,    40,     0,     0,     0,
     0,     0,     0,    29,    21,    19,    43,    46,     0,    44,
     0,    40,    42,     0,     0,     0,    17,     0,    15,     0,
    45,    47,    39,    41,     0,    23,     0,     0,     0,     0,
    28,    24,    25,    16,     0,    27,     0,     0
};

static const short yydefgoto[] = {     1,
    11,    12,    13,    14,    15,    16,    24,    25,    17,    18,
    19,    20,    41,    42,    21,    61,    62,    63
};

static const short yypact[] = {-32768,
     3,-32768,   -15,     8,    12,    11,    18,   -23,    26,    -1,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    36,
-32768,    32,    35,    -2,-32768,    47,    53,    49,    34,    29,
-32768,    30,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
    28,-32768,-32768,    56,-32768,     8,    10,    54,    42,    41,
    -3,    63,    64,-32768,-32768,-32768,-32768,-32768,    17,-32768,
    66,    10,-32768,    10,    67,     0,-32768,    45,-32768,    62,
-32768,-32768,-32768,-32768,    69,-32768,    70,    71,    72,    10,
-32768,-32768,-32768,-32768,    73,-32768,    77,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    33,-32768,-32768,
-32768,-32768,-32768,    37,-32768,    19,-32768,   -46
};


#define	YYLAST		81


static const short yytable[] = {    67,
    45,    29,    87,    57,    58,     2,    22,    30,     3,     4,
     5,    23,     6,    57,    58,    26,    46,    75,    27,    78,
    71,    72,    28,     7,    77,     8,    59,    60,    31,    32,
    68,     9,    10,    85,    43,    53,    59,    60,    33,    34,
    35,    36,    37,    38,    39,    40,    33,    34,    35,    36,
    37,    38,    39,    40,    44,    47,    48,    49,    50,    55,
    51,    52,    64,    65,    66,    69,    79,    70,    73,    76,
    80,    81,    82,    83,    84,    86,    88,    54,    56,     0,
    74
};

static const short yycheck[] = {     3,
     3,    25,     0,     4,     5,     3,    22,    31,     6,     7,
     8,     4,    10,     4,     5,     4,    19,    64,     8,    66,
     4,     5,     5,    21,    25,    23,    27,    28,     3,    31,
    34,    29,    30,    80,     3,     8,    27,    28,    11,    12,
    13,    14,    15,    16,    17,    18,    11,    12,    13,    14,
    15,    16,    17,    18,    20,     9,     4,     9,    25,     4,
    32,    32,     9,    22,    24,     3,    22,     4,     3,     3,
     9,     3,     3,     3,     3,     3,     0,    41,    46,    -1,
    62
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/misc/bison.simple"
/* This file comes from bison-1.27.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 216 "/usr/share/misc/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 13:
#line 166 "loadkeys.y"
{
			    set_charset(kbs_buf.kb_string);
			;
    break;}
case 14:
#line 171 "loadkeys.y"
{
			    alt_is_meta = 1;
			;
    break;}
case 15:
#line 176 "loadkeys.y"
{
			    strings_as_usual();
			;
    break;}
case 16:
#line 181 "loadkeys.y"
{
			    compose_as_usual(kbs_buf.kb_string);
			;
    break;}
case 17:
#line 185 "loadkeys.y"
{
			    compose_as_usual(0);
			;
    break;}
case 18:
#line 190 "loadkeys.y"
{
			    keymaps_line_seen = 1;
			;
    break;}
case 21:
#line 198 "loadkeys.y"
{
			    int i;
			    for (i = yyvsp[-2]; i<= yyvsp[0]; i++)
			      addmap(i,1);
			;
    break;}
case 22:
#line 204 "loadkeys.y"
{
			    addmap(yyvsp[0],1);
			;
    break;}
case 23:
#line 209 "loadkeys.y"
{
			    if (KTYP(yyvsp[-3]) != KT_FN)
				lkfatal1("'%s' is not a function key symbol",
					syms[KTYP(yyvsp[-3])].table[KVAL(yyvsp[-3])]);
			    kbs_buf.kb_func = KVAL(yyvsp[-3]);
			    addfunc(kbs_buf);
			;
    break;}
case 24:
#line 218 "loadkeys.y"
{
			    compose(yyvsp[-4], yyvsp[-3], yyvsp[-1]);
			;
    break;}
case 25:
#line 222 "loadkeys.y"
{
			    compose(yyvsp[-4], yyvsp[-3], yyvsp[-1]);
			;
    break;}
case 26:
#line 226 "loadkeys.y"
{ mod = 0; ;
    break;}
case 27:
#line 228 "loadkeys.y"
{
			    addkey(yyvsp[-3], mod, yyvsp[-1]);
			;
    break;}
case 28:
#line 232 "loadkeys.y"
{
			    addkey(yyvsp[-2], 0, yyvsp[0]);
			;
    break;}
case 31:
#line 239 "loadkeys.y"
{ mod |= M_SHIFT;	;
    break;}
case 32:
#line 240 "loadkeys.y"
{ mod |= M_CTRL;	;
    break;}
case 33:
#line 241 "loadkeys.y"
{ mod |= M_ALT;		;
    break;}
case 34:
#line 242 "loadkeys.y"
{ mod |= M_ALTGR;	;
    break;}
case 35:
#line 243 "loadkeys.y"
{ mod |= M_SHIFTL;	;
    break;}
case 36:
#line 244 "loadkeys.y"
{ mod |= M_SHIFTR;	;
    break;}
case 37:
#line 245 "loadkeys.y"
{ mod |= M_CTRLL;	;
    break;}
case 38:
#line 246 "loadkeys.y"
{ mod |= M_CTRLR;	;
    break;}
case 39:
#line 249 "loadkeys.y"
{
	    int i, j;

	    if (rvalct == 1) {
	        /* Some files do not have a keymaps line, and
		   we have to wait until all input has been read
		   before we know which maps to fill. */
	        key_is_constant[yyvsp[-3]] = 1;
	    
		/* On the other hand, we now have include files,
		   and it should be possible to override lines
		   from an include file. So, kill old defs. */
		for (j = 0; j < max_keymap; j++)
		    if (defining[j])
		        killkey(yyvsp[-3], j);
	    }
	    if (keymaps_line_seen) {
		i = 0;
		for (j = 0; j < max_keymap; j++)
		  if (defining[j]) {
		      if (rvalct != 1 || i == 0)
			addkey(yyvsp[-3], j, (i < rvalct) ? key_buf[i] : K_HOLE);
		      i++;
		  }
		if (i < rvalct)
		    lkfatal0("too many (%d) entries on one line", rvalct);
	    } else
	      for (i = 0; i < rvalct; i++)
		addkey(yyvsp[-3], i, key_buf[i]);
	;
    break;}
case 42:
#line 285 "loadkeys.y"
{
			    if (rvalct >= MAX_NR_KEYMAPS)
				lkfatal(_("too many keydefinitions on one line"));
			    key_buf[rvalct++] = yyvsp[0];
			;
    break;}
case 43:
#line 292 "loadkeys.y"
{yyval=yyvsp[0];;
    break;}
case 44:
#line 294 "loadkeys.y"
{yyval=(yyvsp[0] ^ 0xf000); unicode_used=1;;
    break;}
case 45:
#line 296 "loadkeys.y"
{yyval=K(KT_LETTER, KVAL(yyvsp[0]));;
    break;}
case 46:
#line 298 "loadkeys.y"
{yyval=yyvsp[0];;
    break;}
case 47:
#line 300 "loadkeys.y"
{yyval=K(KT_LETTER, KVAL(yyvsp[0]));;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 542 "/usr/share/misc/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 302 "loadkeys.y"
			

void usage()
{
  printf(_("Usage: %s [option...] [mapfile...]\n"), progname);
  OPTIONS_ARE();
  OPT("-c --clearcompose ", _("clear kernel compose table"));
  OPT("-d --default      ", _("load default keymap file")); /* FIXME: should print DEFKMAP */
  OPT("-m --mktable      ", _("output a \"defkeymap.c\" to stdout"));
  OPT("-s --clearstrings ", _("clear kernel string table"));
  OPT("-q --quiet        ", _("be silent"));
  OPT("-v --verbose      ", _("report the changes"));
  OPT("-v --verbose      ", _("report more changes"));

  OPT("-h --help         ", HELPDESC);
  OPT("-V --version      ", VERSIONDESC);
}

char **args;
int optd = 0;
int optm = 0;
int opts = 0;
int quiet = 0;
int nocompose = 0;

int main(int argc, char *argv[])
{
  const char *short_opts = "cdhmsqvV";
  const struct option long_opts[] = {
    { "clearcompose", no_argument, NULL, 'c' },
    { "default",    no_argument, NULL, 'd' },
    { "help",	no_argument, NULL, 'h' },
    { "mktable",    no_argument, NULL, 'm' },
    { "clearstrings", no_argument, NULL, 's' },
    { "quiet",	no_argument, NULL, 'q' },
    { "verbose",    no_argument, NULL, 'v' },
    { "version", no_argument, NULL, 'V' },
    { NULL, 0, NULL, 0 }
  };
  int c;
  sigset_t sigset, old_sigset;
  
  setuplocale();

  progname = strip_path(argv[0]);

  while ((c = getopt_long(argc, argv,
			  short_opts, long_opts, NULL)) != EOF)
    {
      switch (c)
	{
	case 'c':
	  nocompose = 1;
	  break;
	case 'd':
	  optd = 1;
	  break;
	case 'm':
	  optm = 1;
	  break;
	case 's':
	  opts = 1;
	  break;
	case 'q':
	  quiet = 1;
	  break;
	case 'v':
	  verbose++;
	  break;
	case 'V':
	  version(progname);
	  exit(0);
	case 'h':
	  usage();
	  exit (0);
	case '?':
	  badusage(NULL);
	}
    }

  args = argv + optind - 1;
  unicode_used = 0;
  /* set up the first input file, if any */
  yywrap();
  /* block SIGCHLD or it would break the scanner */
  sigemptyset (&sigset);
  sigaddset (&sigset, SIGCHLD);
  sigprocmask (SIG_BLOCK, &sigset, &old_sigset);
  if (yyparse() || private_error_ct)
    {
      fprintf(stderr, _("syntax error in map file\n"));
      if(!optm)
	fprintf(stderr, _("key bindings not changed\n"));
      exit(1);
    }
  /* restore mask */
  sigprocmask (SIG_SETMASK, &old_sigset, NULL);
  do_constant();
  if(optm)
    mktable();
  else
    loadkeys();
  exit(0);
}

char pathname[1024];
char *filename;
int line_nr = 1;

int yyerror(char *s)
{
  fprintf(stderr, "%s:%d: %s\n", pathname, line_nr, s);
  private_error_ct++;
  return(0);
}

/* fatal errors - change to varargs next time */
void lkfatal(char *s)
{
  fprintf(stderr, "%s: %s:%d: %s\n", progname, filename, line_nr, s);
  exit(1);
}

void lkfatal0(char *s, int d)
{
  fprintf(stderr, "%s: %s:%d: ", progname, filename, line_nr);
  fprintf(stderr, s, d);
  fprintf(stderr, "\n");
  exit(1);
}

void lkfatal1(char *s, char *s2)
{
  fprintf(stderr, "%s: %s:%d: ", progname, filename, line_nr);
  fprintf(stderr, s, s2);
  fprintf(stderr, "\n");
  exit(1);
}

/* String file handling - flex-specific. */
int in_string = 0;

void lk_scan_string(char *s)
{
  lk_push();
  in_string = 1;
  yy_scan_string(s);
}

void lk_end_string(void)
{
  lk_pop();
  in_string = 0;
}

#undef yywrap
int yywrap(void) 
{
  FILE *f;
  static int first_file = 1; /* ugly kludge flag */

  if (in_string)
    {
      lk_end_string();
      return 0;
    }

  if (infile_stack_ptr > 0)
    {
      lk_pop();
      return 0;
    }

  line_nr = 1;
  if (optd) {
    /* first read default map */
    optd = 0;
    if((f = findkeymap(DEFKMAP, pathname, sizeof(pathname), stdin, NULL)) == NULL)
      {
	perror("findkeymap");
	fprintf(stderr, _("Cannot find %s\n"), DEFKMAP);
	exit(1);
      }
    goto gotf;
  }
  if (*args)
    args++;
  if (!*args)
    return 1;
  if (!strcmp(*args, "-"))
    {
      f = stdin;
      strcpy(pathname, "<stdin>");
    }
  else if ((f = findkeymap(*args, pathname, sizeof(pathname), stdin, NULL)) == NULL)
    {
      perror("findkeymap");
      fprintf(stderr, _("cannot open file %s\n"), *args);
      exit(1);
    }
  /*
    Can't use yyrestart if this is called before entering yyparse()
    I think assigning directly to yyin isn't necessarily safe in
    other situations, hence the flag.
    */
gotf:
  filename = xstrdup(pathname);
  if (!quiet)
    fprintf(stderr, "Loading %s\n", pathname);
  if (first_file)
    {
      yyin = f;
      first_file = 0;
    }
  else
    yyrestart(f);
  return 0;
}

static void addmap(int i, int explicit)
{
  if (i < 0 || i >= MAX_NR_KEYMAPS)
    lkfatal0(_("addmap called with bad index %d"), i);
  
  if (!defining[i])
    {
      if (keymaps_line_seen && !explicit)
	lkfatal0(_("adding map %d violates explicit keymaps line)"), i);
      
      defining[i] = 1;
      if (max_keymap <= i)
	max_keymap = i+1;
    }
}

/* unset a key */
static void killkey(int index, int table)
{
  /* roughly: addkey(index, table, K_HOLE); */

  if (index < 0 || index >= NR_KEYS)
    lkfatal0(_("killkey called with bad index %d"), index);
  if (table < 0 || table >= MAX_NR_KEYMAPS)
    lkfatal0(_("killkey called with bad table %d"), table);
  if (key_map[table])
    (key_map[table])[index] = K_HOLE;
  if (keymap_was_set[table])
    (keymap_was_set[table])[index] = 0;
}

static void addkey(int index, int table, int keycode)
{
  int i;

  if (keycode == -1)
    return;
  if (index < 0 || index >= NR_KEYS)
    lkfatal0(_("addkey called with bad index %d"), index);
  if (table < 0 || table >= MAX_NR_KEYMAPS)
    lkfatal0(_("addkey called with bad table %d"), table);

  if (!defining[table])
    addmap(table, 0);
  if (!key_map[table])
    {
      key_map[table] = (u_short *)xmalloc(NR_KEYS * sizeof(u_short));
      for (i = 0; i < NR_KEYS; i++)
	(key_map[table])[i] = K_HOLE;
    }
  if (!keymap_was_set[table])
    {
      keymap_was_set[table] = (char *) xmalloc(NR_KEYS);
      for (i = 0; i < NR_KEYS; i++)
	(keymap_was_set[table])[i] = 0;
    }

  if (alt_is_meta && keycode == K_HOLE && (keymap_was_set[table])[index])
    return;

  (key_map[table])[index] = keycode;
  (keymap_was_set[table])[index] = 1;

  if (alt_is_meta)
    {
      int alttable = table | M_ALT;
      int type = KTYP(keycode);
      int val = KVAL(keycode);

      if (alttable != table && defining[alttable] &&
	  (!keymap_was_set[alttable] ||
	   !(keymap_was_set[alttable])[index]) &&
	  (type == KT_LATIN || type == KT_LETTER) && val < 128)
	addkey(index, alttable, K(KT_META, val));
    }
}

static void addfunc(struct kbsentry kbs)
{
  int sh, i;
  char *p, *q, *r;

  if (kbs.kb_func >= MAX_NR_FUNC)
    {
      fprintf(stderr, _("%s: addfunc called with bad func %d\n"),
	      progname, kbs.kb_func);
      exit(1);
    }
  if ((q = func_table[kbs.kb_func]))	  /* throw out old previous def */
    {
      sh = strlen(q) + 1;
      p = q + sh;
      while (p < fp)
	*q++ = *p++;
      fp -= sh;
    }
  p = func_buf;                        /* find place for new def */
  for (i = 0; i < kbs.kb_func; i++)
    if (func_table[i])
      {
	p = func_table[i];
	while(*p++);
      }
  func_table[kbs.kb_func] = p;
  sh = strlen(kbs.kb_string) + 1;
  if (fp + sh > func_buf + sizeof(func_buf))
    {
      fprintf(stderr, _("%s: addfunc: func_buf overflow\n"), progname);
      exit(1);
    }
  q = fp;
  fp += sh;
  r = fp;
  while (q > p)
    *--r = *--q;
  strcpy(p, kbs.kb_string);
  for (i++; i < MAX_NR_FUNC; i++)
    if (func_table[i])
      func_table[i] += sh;
}

static int unicode_problem(void)
{
  /* Return TRUE if this kernel cannot handle unicode compose chars
   *  properly;
   * (Currently struct kbdiacr has 3 chars: { base, diacr, result},
   * But result needs to be a string for proper Unicode handling)
   */
  return 1;
}
     
static void compose(int diacr, int base, int res)
{
  struct kbdiacr *p;
  if (accent_table_size == MAX_DIACR)
    {
      fprintf(stderr, _("compose table overflow\n"));
      exit(1);
    }
  p = &accent_table[accent_table_size++];
  p->diacr = diacr;
  p->base = base;
  p->result = res;
  if (unicode_problem() && res > 0xFF) {
    fprintf(stderr,
	    _("Warning: Compose char %4x ('%c','%c') > 0xFF will be truncated to %2x\n"),
	    res, p->base, p->diacr, p->result);
    if (!unicode_warning) {
      fprintf(stderr,_("  (Linux kernel < 2.3.X can't handle unicode compose chars properly)\n"));
      unicode_warning = 1;
    }
  }
}

static int defkeys(int fd)
{
  struct kbentry ke;
  int ct = 0;
  int i,j,fail;
  int oldm;

  if (unicode_used)
    {
      /* Switch keyboard mode for a moment -
	 do not complain about errors.
	 Do not attempt a reset if the change failed. */
      if (ioctl(fd, KDGKBMODE, &oldm)
	  || (oldm != K_UNICODE && ioctl(fd, KDSKBMODE, K_UNICODE)))
	oldm = K_UNICODE;
    }

  for(i=0; i<MAX_NR_KEYMAPS; i++)
    {
      if (key_map[i])
	{
	  for(j=0; j<NR_KEYS; j++)
	    {
	      if ((keymap_was_set[i])[j])
		{
		  ke.kb_index = j;
		  ke.kb_table = i;
		  ke.kb_value = (key_map[i])[j];

		  fail = ioctl(fd, KDSKBENT, (unsigned long)&ke);
		  if (fail)
		    {
		      if (errno == EPERM)
			{
			  fprintf(stderr, _("Keymap %d: Permission denied\n"), i);
			  j = NR_KEYS;
			  continue;
			}
		      perror("KDSKBENT");
		    }
		  else
		    ct++;
		  if(verbose)
		    printf(_("keycode %d, table %d = %d%s\n"), j, i,
			   (key_map[i])[j], fail ? _("    FAILED") : "");
		  else if (fail)
		    fprintf(stderr, _("failed to bind key %d to value %d\n"),
			    j, (key_map[i])[j]);
		}
	    }
	}
      else if (keymaps_line_seen && !defining[i])
	{
	  /* deallocate keymap */
	  ke.kb_index = 0;
	  ke.kb_table = i;
	  ke.kb_value = K_NOSUCHMAP;

	  if (verbose > 1)
	    printf(_("deallocate keymap %d\n"), i);

	  if(ioctl(fd, KDSKBENT, (unsigned long)&ke))
	    {
	      if (errno != EINVAL)
		{
		  perror("KDSKBENT");
		  fprintf(stderr,
			  _("%s: could not deallocate keymap %d\n"),
			  progname, i);
		  exit(1);
		}
	      /* probably an old kernel */
	      /* clear keymap by hand */
	      for (j = 0; j < NR_KEYS; j++)
		{
		  ke.kb_index = j;
		  ke.kb_table = i;
		  ke.kb_value = K_HOLE;
		  if(ioctl(fd, KDSKBENT, (unsigned long)&ke))
		    {
		      if (errno == EINVAL && i >= 16)
			break; /* old kernel */
		      perror("KDSKBENT");
		      fprintf(stderr,
			      _("%s: cannot deallocate or clear keymap\n"),
			      progname);
		      exit(1);
		    }
		}
	    }
	}
    }

  if(unicode_used && oldm != K_UNICODE)
    {
      if (ioctl(fd, KDSKBMODE, oldm))
	{
	  fprintf(stderr, _("%s: failed to restore keyboard mode\n"),
		  progname);
	}
      fprintf(stderr, _("%s: warning: this map uses Unicode symbols\n"
	      "    (perhaps you want to do `kbd_mode -u'?)\n"),
	      progname);
    }
  return ct;
}

static char * ostr(char *s)
{
  int lth = strlen(s);
  char *ns0 = (char*)xmalloc(4*lth + 1);
  char *ns = ns0;

  while(*s)
    {
      switch(*s)
	{
	case '\n':
	  *ns++ = '\\';
	  *ns++ = 'n';
	  break;
	case '\033':
	  *ns++ = '\\';
	  *ns++ = '0';
	  *ns++ = '3';
	  *ns++ = '3';
	  break;
	default:
	  *ns++ = *s;
	}
      s++;
    }
  *ns = 0;
  return ns0;
}

static int deffuncs(int fd)
{
  int i, ct = 0;
  char *p;

  for (i = 0; i < MAX_NR_FUNC; i++)
    {
      kbs_buf.kb_func = i;
      if ((p = func_table[i]))
	{
	  strcpy(kbs_buf.kb_string, p);
	  if (ioctl(fd, KDSKBSENT, (unsigned long)&kbs_buf))
	    fprintf(stderr, _("failed to bind string '%s' to function %s\n"),
		    ostr(kbs_buf.kb_string), syms[KT_FN].table[kbs_buf.kb_func]);
	  else
	    ct++;
	}
      else if (opts)
	{
	  kbs_buf.kb_string[0] = 0;
	  if (ioctl(fd, KDSKBSENT, (unsigned long)&kbs_buf))
	    fprintf(stderr, _("failed to clear string %s\n"),
		    syms[KT_FN].table[kbs_buf.kb_func]);
	  else
	    ct++;
	}
    }
  return ct;
}

static int defdiacs(int fd)
{
  struct kbdiacrs kd;
  unsigned i;

  kd.kb_cnt = accent_table_size;
  if (kd.kb_cnt > MAX_DIACR) 
    {
      kd.kb_cnt = MAX_DIACR;
      fprintf(stderr, _("too many compose definitions\n"));
    }
  for (i = 0; i < kd.kb_cnt; i++)
      kd.kbdiacr[i] = accent_table[i];
  
  if(ioctl(fd, KDSKBDIACR, (unsigned long) &kd)) 
    {
      fprintf(stderr, _("KDSKBDIACR failed\n"));
      perror("");
      exit(1);
    }
  return kd.kb_cnt;
}

void do_constant_key (int i, u_short key)
{
  int typ, val, j;

  typ = KTYP(key);
  val = KVAL(key);
  if ((typ == KT_LATIN || typ == KT_LETTER) &&
      ((val >= 'a' && val <= 'z') ||
       (val >= 'A' && val <= 'Z')))
    {
      u_short defs[16];
      defs[0] = K(KT_LETTER, val);
      defs[1] = K(KT_LETTER, val ^ 32);
      defs[2] = defs[0];
      defs[3] = defs[1];
      for(j=4; j<8; j++)
	defs[j] = K(KT_LATIN, val & ~96);
      for(j=8; j<16; j++)
	defs[j] = K(KT_META, KVAL(defs[j-8]));
      for(j=0; j<max_keymap; j++)
	{
	  if (!defining[j])
	    continue;
	  if (j > 0 &&
	      keymap_was_set[j] && (keymap_was_set[j])[i])
	    continue;
	  addkey(i, j, defs[j%16]);
	}
    }
  else
    {
      /* do this also for keys like Escape,
	 as promised in the man page */
      for (j=1; j<max_keymap; j++)
	if(defining[j] &&
	   (!(keymap_was_set[j]) || !(keymap_was_set[j])[i]))
	  addkey(i, j, key);
    }
}

static void do_constant (void)
{
  int i, r0 = 0;

  if (keymaps_line_seen)
    while (r0 < max_keymap && !defining[r0])
      r0++;

  for (i=0; i<NR_KEYS; i++)
    {
      if (key_is_constant[i])
	{
	  u_short key;
	  if (!key_map[r0])
	    lkfatal(_("impossible error in do_constant"));
	  key = (key_map[r0])[i];
	  do_constant_key (i, key);
	}
    }
}

static void loadkeys (void)
{
  int fd;
  int keyct, funcct, diacct;

  if (-1 == (fd = get_console_fd(NULL)))
    exit (1);
		
  keyct = defkeys(fd);
  funcct = deffuncs(fd);
  if (accent_table_size > 0 || nocompose)
    diacct = defdiacs(fd);
  if (verbose)
    {
      printf(_("\nChanged %d key%s and %d string%s.\n"),
	     keyct, (keyct == 1) ? "" : "s",
	     funcct, (funcct == 1) ? "" : "s");
      if (accent_table_size > 0 || nocompose)
	printf(_("Loaded %d compose definition%s.\n"),
	       diacct, (diacct == 1) ? "" : "s");
      else
	printf(_("(No change in compose definitions.)\n"));
    }
}

static void strings_as_usual(void)
{
  /*
   * 28 strings, mostly inspired by the VT100 family
   */
  char *stringvalues[30] = {
    /* F1 .. F20 */
    "\033[[A", "\033[[B", "\033[[C", "\033[[D", "\033[[E",
    "\033[17~", "\033[18~", "\033[19~", "\033[20~", "\033[21~",
    "\033[23~", "\033[24~", "\033[25~", "\033[26~",
    "\033[28~", "\033[29~",
    "\033[31~", "\033[32~", "\033[33~", "\033[34~",
    /* Find,    Insert,    Remove,    Select,    Prior */
    "\033[1~", "\033[2~", "\033[3~", "\033[4~", "\033[5~",
    /* Next,    Macro,  Help, Do,  Pause */
    "\033[6~", "\033[M",  0,   0, "\033[P"
  };
  int i;
  for (i=0; i<30; i++)
    if(stringvalues[i])
      {
	struct kbsentry ke;
	ke.kb_func = i;
	strncpy(ke.kb_string, stringvalues[i], sizeof(ke.kb_string));
	ke.kb_string[sizeof(ke.kb_string)-1] = 0;
	addfunc(ke);
      }
}

static void compose_as_usual(char *charset)
{
  if (charset && strcmp(charset, "iso-8859-1"))
    {
      fprintf(stderr, _("loadkeys: don't know how to compose for %s\n"),
	      charset);
      exit(1);
    }
  else
    {
      struct ccc {
	char c1, c2, c3;
      } def_latin1_composes[68] = {
	{ '`', 'A', 0300 }, { '`', 'a', 0340 },
	{ '\'', 'A', 0301 }, { '\'', 'a', 0341 },
	{ '^', 'A', 0302 }, { '^', 'a', 0342 },
	{ '~', 'A', 0303 }, { '~', 'a', 0343 },
	{ '"', 'A', 0304 }, { '"', 'a', 0344 },
	{ 'O', 'A', 0305 }, { 'o', 'a', 0345 },
	{ '0', 'A', 0305 }, { '0', 'a', 0345 },
	{ 'A', 'A', 0305 }, { 'a', 'a', 0345 },
	{ 'A', 'E', 0306 }, { 'a', 'e', 0346 },
	{ ',', 'C', 0307 }, { ',', 'c', 0347 },
	{ '`', 'E', 0310 }, { '`', 'e', 0350 },
	{ '\'', 'E', 0311 }, { '\'', 'e', 0351 },
	{ '^', 'E', 0312 }, { '^', 'e', 0352 },
	{ '"', 'E', 0313 }, { '"', 'e', 0353 },
	{ '`', 'I', 0314 }, { '`', 'i', 0354 },
	{ '\'', 'I', 0315 }, { '\'', 'i', 0355 },
	{ '^', 'I', 0316 }, { '^', 'i', 0356 },
	{ '"', 'I', 0317 }, { '"', 'i', 0357 },
	{ '-', 'D', 0320 }, { '-', 'd', 0360 },
	{ '~', 'N', 0321 }, { '~', 'n', 0361 },
	{ '`', 'O', 0322 }, { '`', 'o', 0362 },
	{ '\'', 'O', 0323 }, { '\'', 'o', 0363 },
	{ '^', 'O', 0324 }, { '^', 'o', 0364 },
	{ '~', 'O', 0325 }, { '~', 'o', 0365 },
	{ '"', 'O', 0326 }, { '"', 'o', 0366 },
	{ '/', 'O', 0330 }, { '/', 'o', 0370 },
	{ '`', 'U', 0331 }, { '`', 'u', 0371 },
	{ '\'', 'U', 0332 }, { '\'', 'u', 0372 },
	{ '^', 'U', 0333 }, { '^', 'u', 0373 },
	{ '"', 'U', 0334 }, { '"', 'u', 0374 },
	{ '\'', 'Y', 0335 }, { '\'', 'y', 0375 },
	{ 'T', 'H', 0336 }, { 't', 'h', 0376 },
	{ 's', 's', 0337 }, { '"', 'y', 0377 },
	{ 's', 'z', 0337 }, { 'i', 'j', 0377 }
      };
      int i;

      for(i=0; i<68; i++)
	{
	  struct ccc p = def_latin1_composes[i];
	  compose(p.c1, p.c2, p.c3);
	}
    }
}

/*
 * mktable.c
 *
 */
static char *modifiers[8] = {
  "shift", "altgr", "ctrl", "alt", "shl", "shr", "ctl", "ctr"
};

static char *mk_mapname(char mod)
{
  static char buf[60];
  int i;

  if (!mod)
    return "plain";
  buf[0] = 0;
  for (i=0; i<8; i++)
    if (mod & (1<<i))
      {
	if (buf[0])
	  strcat(buf, "_");
	strcat(buf, modifiers[i]);
      }
  return buf;
}


static void outchar (unsigned char c, int comma)
{
  printf("'");
  printf((c == '\'' || c == '\\') ? "\\%c" : isgraph(c) ? "%c"
	 : "\\%03o", c);
  printf(comma ? "', " : "'");
}

static void mktable ()
{
  int i, imax, j;

  /*	struct kbsentry kbs;*/
  u_char *p;
  int maxfunc;
  unsigned int keymap_count = 0;

  printf("\n"
	 "/* Do not edit this file! It was automatically generated by   */\n"
	 "/*    %s --mktable defkeymap.map > defkeymap.c          */\n\n"
	 "#include <linux/types.h>\n"
	 "#include <linux/keyboard.h>\n"
	 "#include <linux/kd.h>\n\n"
	 , progname);
  for (i = 0; i < MAX_NR_KEYMAPS; i++)
    if (key_map[i])
      {
	keymap_count++;
	if (i)
	  printf("static ");
	printf("u_short %s_map[NR_KEYS] = {", mk_mapname(i));
	for (j = 0; j < NR_KEYS; j++)
	  {
	    if (!(j % 8))
	      printf("\n");
	    printf("\t0x%04x,", U((key_map[i])[j]));
	  }
	printf("\n};\n\n");
      }

  for (imax = MAX_NR_KEYMAPS-1; imax > 0; imax--)
    if (key_map[imax])
      break;
  printf("ushort *key_maps[MAX_NR_KEYMAPS] = {");
  for (i = 0; i <= imax; i++)
    {
      printf((i%4) ? " " : "\n\t");
      if (key_map[i])
	printf("%s_map,", mk_mapname(i));
      else
	printf("0,");
    }
  if (imax < MAX_NR_KEYMAPS-1)
    printf("\t0");
  printf("\n};\n\nunsigned int keymap_count = %d;\n\n", keymap_count);
	
  printf("\n"
	 "/*\n"
	 " * Philosophy: most people do not define more strings, but they who do\n"
	 " * often want quite a lot of string space. So, we statically allocate\n"
	 " * the default and allocate dynamically in chunks of 512 bytes.\n"
	 " */\n"
	 "\n");
  for (maxfunc = MAX_NR_FUNC; maxfunc; maxfunc--)
    if(func_table[maxfunc-1])
      break;

  printf("char func_buf[] = {\n");
  for (i = 0; i < maxfunc; i++)
    {
      p = func_table[i];
      if (p)
	{
	  printf("\t");
	  for ( ; *p; p++)
	    outchar(*p, 1);
	  printf("0, \n");
	}
    }
  if (!maxfunc)
    printf("\t0\n");
  printf("};\n\n");

  printf("\n"
	 "char *funcbufptr = func_buf;\n"
	 "int funcbufsize = sizeof(func_buf);\n"
	 "int funcbufleft = 0;          /* space left */\n"
	 "\n");

  printf("char *func_table[MAX_NR_FUNC] = {\n");
  for (i = 0; i < maxfunc; i++)
    {
      if (func_table[i])
	printf("\tfunc_buf + %d,\n", func_table[i] - func_buf);
      else
	printf("\t0,\n");
    }
  if (maxfunc < MAX_NR_FUNC)
    printf("\t0,\n");
  printf("};\n");

  printf("\nstruct kbdiacr accent_table[MAX_DIACR] = {\n");
  for (i = 0; i < accent_table_size; i++)
    {
      printf("\t{");
      outchar(accent_table[i].diacr, 1);
      outchar(accent_table[i].base, 1);
      outchar(accent_table[i].result, 0);
      printf("},");
      if(i%2) printf("\n");
    }
  if(i%2) printf("\n");
  printf("};\n\n");
  printf("unsigned int accent_table_size = %d;\n",
	 accent_table_size);

  exit(0);
}


