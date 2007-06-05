/* tree.h
 * (c) 2002 Martin 'PerM' Pergel
 * This file is a part of the Links program, released under GPL.
 */

#define  TAND 1
#define  TANDAND 2
#define  TANDEQ 3
#define  TArgumentList 4
#define  TArguments 5
#define  TArrayLiteral 6
#define  TBlock 7
#define  TCallExpressionAR 8
#define  TCallExpressionCA 9
#define  TCallExpressionMA 10
#define  TCallExpressionPT 11
#define  TCondition 12
#define  TDELETE 13
#define  TDIVEQ 14
#define  TElementList 15
#define  TElision 16
#define  TEmptyStatement 17
#define  TEQ 18
#define  TEQEQ 19
#define  TEQEQEQ 20
#define  TEXCLAM 21
#define  TEXCLAMEQ 22
#define  TEXCLAMEQEQ 23
#define  TExpression 24
#define  TFALSELIT 25
#define  TIF 26
#define  TIN 27
#define  TINSTANCEOF 28
#define  TKRAT 29
#define  TLOMENO 30
#define  TMemberExpression 31
#define  TMINEQ 32
#define  TMINMIN 33
#define  TMINUS 34
#define  TMOD 35
#define  TMODEQ 36
#define  TNEW 37
#define  TNEWMemberExpression 38
#define  TNULLLIT 39
#define  TNUMLIT 40
#define  TObjectLiteral 41
#define  TOR 42
#define  TOREQ 43
#define  TOROR 44
#define  TPLUS 45
#define  TPLUSEQ 46
#define  TPLUSPLUS 47
#define  TPostfixExpression 48
#define  TPropertyNameAndValueList 49
#define  TSHL 50
#define  TSHLEQ 51
#define  TSHLSHL 52
#define  TSHLSHLEQ 53
#define  TSHR 54
#define  TSHREQ 55
#define  TSHRSHR 56
#define  TSHRSHREQ 57
#define  TSHRSHRSHR 58
#define  TStatementList 59
#define  TSTRINGLIT 60
#define  TTHIS 61
#define  TTHREERIGHTEQUAL 62
#define  TTIMESEQ 63
#define  TTRUELIT 64
#define  TTYPEOF 65
#define  TVAR 66
#define  TVariableDeclarationList 67
#define  TVOID 68
#define  TXOR 69
#define  TXOREQ 70
#define TDO 71
#define TWHILE 72
#define TFOR 73
#define TFOR1 74
#define TFOR2 75
#define TFOR3 76
#define TCONTINUE 77
#define TBREAK 78
#define TRETURN 79
#define TWITH 80

/* #define TSWITCH 81
   #define TCaseBlock 82
   #define TCaseClauses 83
   #define TCASE 84*/

#define TTHROW 85
#define TTRY 86
#define TSourceElements 87
#define TUnaryExpression 88
#define TIDENTIFIER 89
#define TVariableDeclaration 90
#define TCATCH 91
#define TLabelledStatement 92
#define TFunctionExpression 93
#define TPROGRAM 94
#define TFUNCTIONDECL 95
#define TTIMES 96
#define TSLASH 97
#define TCOMPL 98
#define TParameterList 99
#define TStatements 100
#define TUNMIN 101
#define TTHISCCall 102
#define TECCall 103
#define TConsCall 104
#define TMember 105
#define TArray 106
#define TCARKA 107
#define TPLUSPLUSPOST 108
#define TMINMINPOST 109
#define TFunctionCall 110
#define TCONVERT 111
#define TVariables 112
#define TIdCCall 113
#define TLocAssign 114
#define TREGEXPLIT 115
#define TDWHILE 116

#define TCS 117 /* Case clause - prvni arg - vyraz, druhy arg kod. */
#define TSWITCH 118 /* arg[0] toustovany vyraz, arg[1] casove klauzy. */
/* Jeste je potreba opatchovat break, continue a return */
/* Protokol: TSWITCH vyrobi pytlik, ktery ostatni budou cist, zrusi ho
 * TSWITCH cestou nahoru (tj. jeste se vratime!). TCSS bude asi zmenen 
 * na Statements (zavola leveho a praveho). Levy je TCS, ten vyhodnoti svuj
 * expression, vytahne z bufferu "neznicitelny" token, okopiruje ho, vrati
 * zpatky, porovna a pripadne spusti akci a kazdopadne skonci (bud probehne 
 * nebo ne - jako v softballe). TCASECLAUD by slo zmenit na 2 statementy
 * (a rano to taky asi udelam). TCS po sobe musi nechat (vzdy) jeden pytlik,
 * ktery budto Statements (TCSS resp. TCASECLAUD nebo TSWITCH zrusi - TSWITCH
 * zrusi dva a vrati tam jeden undefined. */

/* Schema: TSWITCH: Vyhodnot leveho syna, vyhodnot praveho syna, zrus dva 
 * pytliky, dej tam undefined.
 * 	   TCS: Vyhodnot leveho syna, vytas dva pytliky, spodni tam vrat.
 * Vyndane pytliky porovnej na rovnost. Rovne \equiv vyhodnot praveho syna,
 * nerovno \equiv soupni na buffer undefined. */

