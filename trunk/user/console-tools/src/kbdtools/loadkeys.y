/*
 * loadkeys.y
 *
 * Changes for 0.82:
 * Merged from version 0.81 of loadkeys.y and mktable.c - aeb@cwi.nl
 * Reason for change:
 *   The original version of mktable would use the current kernel
 *   for getting at the entries. However, this does not work if
 *   e.g. NR_FUNC in keyboard.h has changed. So, instead of getting
 *   the information from the current kernel, we must get it out of
 *   defkeymap.map, just like loadkeys. Thus, mktable is now an
 *   option of loadkeys.
 * (Other advantage: we first do the parsing, and then the key binding.
 *  No bindings are changed in case of a syntax error.)
 * Fix: contrary to the documentation in keytables.5 it was assumed
 * by default that the AltGr modifier had no effect. This assumption
 * has been removed.
 *
 * Changes for 0.83:
 * Added the intl_con patch by Eugene G. Crosser:
 * The modifier + in front of a keysym means that it is a letter,
 * and susceptible for change under CapsLock. For ASCII 'a'-'z'
 * and 'A'-'Z' no + is required (when given as a single entry).
 *
 * Changes for 0.84:
 * More compose key support. Default search path. Option -d.
 *
 * Change for 0.85:
 * Do not change compose key table when no definitions given.
 * Option -c to override this.
 *
 * Changes for 0.86:
 * Added a few synonyms. Comment may now contain non-ASCII symbols.
 *
 * Changes for 0.87:
 * Accept the "charset iso-8859-x" directive.
 *
 * Changes for 0.88:
 * Handle sparse keymaps and many strings. Accept "keymaps" directive.
 * Handle 8 modifiers. Handle Unicode.
 *
 * Change for 0.93:
 * Set K_UNICODE during the loading of keymaps requiring that.
 *
 * Change for 0.94:
 * Add alt_is_meta keyword:
 * Whenever some combination is defined as an ASCII symbol, and there
 * is a corresponding Alt keymap, define by default the corresponding
 * Alt combination as Meta_value.
 *
 * Change for 0.96:
 * Compose now accepts letter names.
 * Add strings_as_usual etc.
 * Add include directive.
 */

%token EOL NUMBER LITERAL CHARSET KEYMAPS KEYCODE EQUALS
%token PLAIN SHIFT CONTROL ALT ALTGR SHIFTL SHIFTR CTRLL CTRLR
%token COMMA DASH STRING STRLITERAL COMPOSE TO CCHAR ERROR PLUS
%token UNUMBER ALT_IS_META STRINGS AS USUAL ON FOR

%{
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
%}

%%
keytable	:
		| keytable line
		;
line		: EOL
		| charsetline
		| altismetaline
		| usualstringsline
		| usualcomposeline
		| keymapline
		| fullline
		| singleline
		| strline
                | compline
		;
charsetline	: CHARSET STRLITERAL EOL
			{
			    set_charset(kbs_buf.kb_string);
			}
		;
altismetaline	: ALT_IS_META EOL
			{
			    alt_is_meta = 1;
			}
		;
usualstringsline: STRINGS AS USUAL EOL
			{
			    strings_as_usual();
			}
		;
usualcomposeline: COMPOSE AS USUAL FOR STRLITERAL EOL
			{
			    compose_as_usual(kbs_buf.kb_string);
			}
		  | COMPOSE AS USUAL EOL
			{
			    compose_as_usual(0);
			}
		;
keymapline	: KEYMAPS range EOL
			{
			    keymaps_line_seen = 1;
			}
		;
range		: range COMMA range0
		| range0
		;
range0		: NUMBER DASH NUMBER
			{
			    int i;
			    for (i = $1; i<= $3; i++)
			      addmap(i,1);
			}
		| NUMBER
			{
			    addmap($1,1);
			}
		;
strline		: STRING LITERAL EQUALS STRLITERAL EOL
			{
			    if (KTYP($2) != KT_FN)
				lkfatal1("'%s' is not a function key symbol",
					syms[KTYP($2)].table[KVAL($2)]);
			    kbs_buf.kb_func = KVAL($2);
			    addfunc(kbs_buf);
			}
		;
compline        : COMPOSE CCHAR CCHAR TO CCHAR EOL
                        {
			    compose($2, $3, $5);
			}
		 | COMPOSE CCHAR CCHAR TO rvalue EOL
			{
			    compose($2, $3, $5);
			}
                ;
singleline	:	{ mod = 0; }
		  modifiers KEYCODE NUMBER EQUALS rvalue EOL
			{
			    addkey($4, mod, $6);
			}
		| PLAIN KEYCODE NUMBER EQUALS rvalue EOL
			{
			    addkey($4, 0, $6);
			}
		;
modifiers	: modifiers modifier
		| modifier
		;
modifier	: SHIFT		{ mod |= M_SHIFT;	}
		| CONTROL	{ mod |= M_CTRL;	}
		| ALT		{ mod |= M_ALT;		}
		| ALTGR		{ mod |= M_ALTGR;	}
		| SHIFTL	{ mod |= M_SHIFTL;	}
		| SHIFTR	{ mod |= M_SHIFTR;	}
		| CTRLL		{ mod |= M_CTRLL;	}
		| CTRLR		{ mod |= M_CTRLR;	}
		;
fullline	: KEYCODE NUMBER EQUALS rvalue0 EOL
	{
	    int i, j;

	    if (rvalct == 1) {
	        /* Some files do not have a keymaps line, and
		   we have to wait until all input has been read
		   before we know which maps to fill. */
	        key_is_constant[$2] = 1;
	    
		/* On the other hand, we now have include files,
		   and it should be possible to override lines
		   from an include file. So, kill old defs. */
		for (j = 0; j < max_keymap; j++)
		    if (defining[j])
		        killkey($2, j);
	    }
	    if (keymaps_line_seen) {
		i = 0;
		for (j = 0; j < max_keymap; j++)
		  if (defining[j]) {
		      if (rvalct != 1 || i == 0)
			addkey($2, j, (i < rvalct) ? key_buf[i] : K_HOLE);
		      i++;
		  }
		if (i < rvalct)
		    lkfatal0("too many (%d) entries on one line", rvalct);
	    } else
	      for (i = 0; i < rvalct; i++)
		addkey($2, i, key_buf[i]);
	}
		;

rvalue0		: 
		| rvalue1 rvalue0
		;
rvalue1		: rvalue
			{
			    if (rvalct >= MAX_NR_KEYMAPS)
				lkfatal(_("too many keydefinitions on one line"));
			    key_buf[rvalct++] = $1;
			}
		;
rvalue		: NUMBER
			{$$=$1;}
		| UNUMBER
			{$$=($1 ^ 0xf000); unicode_used=1;}
                | PLUS NUMBER
                        {$$=K(KT_LETTER, KVAL($2));}
		| LITERAL
			{$$=$1;}
                | PLUS LITERAL
                        {$$=K(KT_LETTER, KVAL($2));}
		;
%%			

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


