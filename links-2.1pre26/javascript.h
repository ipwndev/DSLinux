/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

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

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     BREAK = 31870,
     CASE = 31871,
     CATCH = 31872,
     CONTINUE = 31873,
     DEFAULT = 31874,
     DELETE = 31875,
     DO = 31876,
     ELSE = 31877,
     FINALLY = 31878,
     FOR = 31879,
     FUNCTION = 31880,
     IF = 31881,
     IN = 31882,
     INSTANCEOF = 31883,
     NEW = 31884,
     RETURN = 31885,
     SWITCH = 31886,
     THIS = 31887,
     THROW = 31888,
     TYPEOF = 31889,
     TRY = 31890,
     VAR = 31891,
     VOID = 31892,
     WHILE = 31893,
     WITH = 31894,
     LEXERROR = 31895,
     THREERIGHTEQUAL = 31896,
     IDENTIFIER = 31897,
     NULLLIT = 31898,
     FALSELIT = 31899,
     TRUELIT = 31900,
     NUMLIT = 31901,
     STRINGLIT = 31902,
     REGEXPLIT = 31903,
     BUGGY_TOKEN = 31904,
     PLUSPLUS = 11051,
     MINMIN = 11565,
     SHLEQ = 15676,
     SHREQ = 15678,
     SHLSHL = 15420,
     SHRSHR = 15934,
     SHRSHRSHR = 31905,
     EQEQ = 15677,
     EXCLAMEQ = 15649,
     EQEQEQ = 31906,
     EXCLAMEQEQ = 31907,
     ANDAND = 9766,
     OROR = 31868,
     PLUSEQ = 15659,
     MINEQ = 15661,
     TIMESEQ = 15658,
     MODEQ = 15653,
     DIVEQ = 15663,
     ANDEQ = 15654,
     OREQ = 15740,
     XOREQ = 15710,
     SHLSHLEQ = 31908,
     SHRSHREQ = 31909
   };
#endif
/* Tokens.  */
#define BREAK 31870
#define CASE 31871
#define CATCH 31872
#define CONTINUE 31873
#define DEFAULT 31874
#define DELETE 31875
#define DO 31876
#define ELSE 31877
#define FINALLY 31878
#define FOR 31879
#define FUNCTION 31880
#define IF 31881
#define IN 31882
#define INSTANCEOF 31883
#define NEW 31884
#define RETURN 31885
#define SWITCH 31886
#define THIS 31887
#define THROW 31888
#define TYPEOF 31889
#define TRY 31890
#define VAR 31891
#define VOID 31892
#define WHILE 31893
#define WITH 31894
#define LEXERROR 31895
#define THREERIGHTEQUAL 31896
#define IDENTIFIER 31897
#define NULLLIT 31898
#define FALSELIT 31899
#define TRUELIT 31900
#define NUMLIT 31901
#define STRINGLIT 31902
#define REGEXPLIT 31903
#define BUGGY_TOKEN 31904
#define PLUSPLUS 11051
#define MINMIN 11565
#define SHLEQ 15676
#define SHREQ 15678
#define SHLSHL 15420
#define SHRSHR 15934
#define SHRSHRSHR 31905
#define EQEQ 15677
#define EXCLAMEQ 15649
#define EQEQEQ 31906
#define EXCLAMEQEQ 31907
#define ANDAND 9766
#define OROR 31868
#define PLUSEQ 15659
#define MINEQ 15661
#define TIMESEQ 15658
#define MODEQ 15653
#define DIVEQ 15663
#define ANDEQ 15654
#define OREQ 15740
#define XOREQ 15710
#define SHLSHLEQ 31908
#define SHRSHREQ 31909




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



