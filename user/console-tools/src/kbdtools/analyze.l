%{
#include <stdlib.h>
#include <linux/kd.h>
#include <lct/utils.h>
#include "loadkeys.h"

extern int ksymtocode(char *s);

extern int line_nr;
extern int verbose;				  /* loadkeys.y */
extern char pathname[];				  /* idem */
int	yylval;
int	rvalct;
struct kbsentry kbs_buf;
char *p, *pmax;
char *filename;

#undef yywrap
extern int yywrap(void);
extern int yyerror(char *s);
extern void stringovfl(void);
extern void lkfatal(char *s);
extern void lkfatal1(char *s, char *s2);
extern void open_include(char *s);
void lkfatal(char *);
void lkfatal0(char *, int);
void lkfatal1(char *, char *);
%}

%s RVALUE
%x STR
%x INCLSTR
Comment			#|!
Continuation		\\\n
Eol			\n
Blank			[ \t]
Include			include[ \t]*
Decimal			[1-9][0-9]*
Octal			0[0-7]*
Hex			0[xX][0-9a-fA-F]+
Unicode			U\+([0-9a-fA-F]){4}
Literal			[a-zA-Z][a-zA-Z_0-9]*
Octa			([0-7]){1,3}
Charset			charset|Charset|CharSet|CHARSET
Keymaps			keymaps|Keymaps|KeyMaps|KEYMAPS
Keycode			keycode|Keycode|KeyCode|KEYCODE
String			string|String|STRING
Equals			=
Plain			plain|Plain|PLAIN
Shift			shift|Shift|SHIFT
Control			control|Control|CONTROL
Alt			alt|Alt|ALT
AltGr			altgr|Altgr|AltGr|ALTGR
ShiftL			shiftl|ShiftL|SHIFTL
ShiftR			shiftr|ShiftR|SHIFTR
CtrlL			ctrll|CtrlL|CTRLL
CtrlR			ctrlr|CtrlR|CTRLR
AltIsMeta		[aA][lL][tT][-_][iI][sS][-_][mM][eE][tT][aA]
Strings			strings|Strings|STRINGS
Compose                 compose|Compose|COMPOSE
As			as|As|AS
Usual			usual|Usual|USUAL
For			for|For|FOR
On			on|On|ON
To                      to|To|TO
%%
{Include}               {BEGIN(INCLSTR);}
<INCLSTR>\"[^"\n]+\"	{ int l; char *s;
			 l = strlen(yytext);
			 s = (char*)xmalloc(l);
			 strcpy(s, yytext+1);
			 s[l-2] = 0;		/* wipe out " */
			 open_include(s);
			 BEGIN(0);
			}
<INCLSTR>[^"]|\"\"|\"[^"\n]*{Eol}	{
			  yyerror("expected filename between quotes");
			  BEGIN(0); }
{Continuation}		{line_nr++;}
{Eol}			{line_nr++;BEGIN(0);return(EOL);}
{Blank}+		; /* do nothing */
{Comment}.*/{Eol}	; /* do nothing */
{Equals}		{BEGIN(RVALUE);rvalct=0;return(EQUALS);}
\-			{return(DASH);}
\,			{return(COMMA);}
\+			{return(PLUS);}
{Unicode}		{yylval=strtol(yytext+1,NULL,16);return(UNUMBER);}
{Decimal}|{Octal}|{Hex}	{yylval=strtol(yytext,NULL,0);return(NUMBER);}
<RVALUE>{Literal}	{return((yylval=ksymtocode(yytext))==-1?ERROR:LITERAL);}
{Charset}		{return(CHARSET);}
{Keymaps}		{return(KEYMAPS);}
{Keycode}		{return(KEYCODE);}
{String}		{BEGIN(RVALUE);return(STRING);}
{Plain}			{return(PLAIN);}
{Shift}			{return(SHIFT);}
{Control}		{return(CONTROL);}
{Alt}			{return(ALT);}
{AltGr}			{return(ALTGR);}
{ShiftL}		{return(SHIFTL);}
{ShiftR}		{return(SHIFTR);}
{CtrlL}			{return(CTRLL);}
{CtrlR}			{return(CTRLR);}
{AltIsMeta}		{return(ALT_IS_META);}
{Strings}		{return(STRINGS);}
{Compose}               {return(COMPOSE);}
{As}			{return(AS);}
{Usual}			{return(USUAL);}
{To}                    {BEGIN(RVALUE); return(TO);}
{On}			{return(ON);}
{For}			{return(FOR);}
'\\{Octa}'              {yylval = strtol(yytext+2,NULL,8); return(CCHAR);}
'\\.'                   {yylval = yytext[2]; return(CCHAR);}
'.'                     {yylval = yytext[1]; return(CCHAR);}
\"			{p=kbs_buf.kb_string;
				pmax=p+sizeof(kbs_buf.kb_string)-1;
				BEGIN(STR);}
<STR>\\{Octa}		{if(p>=pmax)stringovfl();*p++=strtol(yytext+1,NULL,8);}
<STR>\\\"               {if(p>=pmax)stringovfl();*p++='"';}
<STR>\\\\               {if(p>=pmax)stringovfl();*p++='\\';}
<STR>\\n		{if(p>=pmax)stringovfl();*p++='\n';}
<STR>[^"\\]*		{char *ptmp=p;p+=strlen(yytext);
				if(p>pmax)stringovfl();strcpy(ptmp,yytext);}
<STR>\"			{*p='\0';BEGIN(0);return(STRLITERAL); /*"*/}
.			{return(ERROR); /* report any unknown characters */}
%%
#include <string.h>
#include <lct/ksyms.h>
#include <linux/keyboard.h>

void
stringovfl(void) {
	lkfatal("string too long");
}

/* Include file handling - unfortunately flex-specific. */
#define MAX_INCLUDE_DEPTH 20
struct infile {
	int linenr;
	char *filename;
	YY_BUFFER_STATE bs;
} infile_stack[MAX_INCLUDE_DEPTH];
int infile_stack_ptr = 0;

void lk_push(void)
{
  if (infile_stack_ptr >= MAX_INCLUDE_DEPTH)
    lkfatal("includes nested too deeply");

  /* preserve current state */
  infile_stack[infile_stack_ptr].filename = filename;
  infile_stack[infile_stack_ptr].linenr = line_nr;
  infile_stack[infile_stack_ptr++].bs =
    YY_CURRENT_BUFFER;
}

int lk_pop(void)
{
  if (--infile_stack_ptr >= 0)
    {
      filename = infile_stack[infile_stack_ptr].filename;
      line_nr = infile_stack[infile_stack_ptr].linenr;
      yy_delete_buffer(YY_CURRENT_BUFFER);
      yy_switch_to_buffer(infile_stack[infile_stack_ptr].bs);
      return 0;
    }
  return 1;
}

/*
 * Where shall we look for an include file?
 * Current strategy (undocumented, may change):
 *
 * 1. Look for a user-specified LOADKEYS_INCLUDE_PATH
 * 2. Try . and ../include and ../../include
 * 3. Try D and D/../include and D/../../include
 *    where D is the directory from where we are loading the current file.
 * 4. Try KD/include and KD/#/include where KD = DATADIR/KEYMAPDIR.
 *
 * Expected layout:
 * KD has subdirectories amiga, atari, sun, i386, include
 * KD/include contains architecture-independent stuff
 * like strings and iso-8859-x compose tables.
 * KD/i386 has subdirectories qwerty, ... and include;
 * this latter include dir contains stuff with keycode=...
 *
 * (Of course, if the present setup turns out to be reasonable,
 * then later also the other architectures will grow and get
 * subdirectories, and the hard-coded i386 below will go again.)
 *
 * People that dislike a dozen lookups for loadkeys
 * can easily do "loadkeys file_with_includes; dumpkeys > my_keymap"
 * and afterwards use only "loadkeys /fullpath/mykeymap", where no
 * lookups are required.
 */
const char *include_dirpath0[] = { "", 0 };
const char *include_dirpath1[] = { "", "../include/", "../../include/", 0 };
char *include_dirpath2[] = { 0, 0, 0, 0 };
const char *include_dirpath3[] = { DATADIR "/" KEYMAPDIR "/include/",
			     DATADIR "/" KEYMAPDIR "/i386/include/", 0 };
const char *include_suffixes[] = { "", ".inc", 0 };

FILE *find_standard_incl_file(char *s)
{
  FILE *f;
  char *t, *te, *t1, *t2;
  int len;

  /* Try . and ../include and ../../include */
  f = findfile_simple(s, include_dirpath1, include_suffixes);
  if (f)
    return f;

  /* Try D and D/../include and D/../../include */
  t = xstrdup(filename);
  te = rindex(t, '/');
  if (te)
    {
      te[1] = 0;
      include_dirpath2[0] = t;
      len = strlen(t);
      include_dirpath2[1] = t1 = (char*)xmalloc(len + 12);
      include_dirpath2[2] = t2 = (char*)xmalloc(len + 15);
      strcpy(t1, t);
      strcat(t1, "../include/");
      strcpy(t2, t);
      strcat(t2, "../../include/");
      f = findfile_simple(s, (const char**)include_dirpath2, include_suffixes);
      if (f)
	return f;
    }

  /* Try KD/include and KD/#/include */
  return findfile_simple(s, include_dirpath3, include_suffixes);
}

FILE *find_incl_file(char *s)
{
  FILE *f;
  char *ev;
  if (!s || !*s)
    return NULL;
  if (*s == '/')		/* no path required */
    return (findfile_simple(s, include_dirpath0, include_suffixes));

  if((ev = getenv("LOADKEYS_INCLUDE_PATH")) != NULL)
    {
      /* try user-specified path */
      char *user_dir[2] = { 0, 0 };
      while(ev)
	{
	  char *t = index((const char*)ev, ':');
	  char sv;
	  if (t)
	    {
	      sv = *t;
	      *t = 0;
	    }
	  user_dir[0] = ev;
	  if (*ev)
	    f = findfile_simple(s, (const char**)user_dir, include_suffixes);
	  else	/* empty string denotes system path */
	    f = find_standard_incl_file(s);
	  if (f)
	    return f;
	  if (t)
	    *t++ = sv;
	  ev = t;
	}
      return NULL;
    }
  return find_standard_incl_file(s);
}

void open_include(char *s)
{
  if (verbose)
    fprintf(stderr, "switching to %s\n", s);

  lk_push();

  yyin = find_incl_file(s);
  if (!yyin)
    lkfatal1("cannot open include file %s", s);
  filename = xstrdup(pathname);
  line_nr = 1;
  yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
}
