/*
 * dumpkeys.c
 *
 * derived from version 0.81 - aeb@cwi.nl
 * Fix: escape quotes and backslashes in strings
 * Fix: after  dumpkeys > x; loadkeys x; dumpkeys > y
 *      the files x and y should be identical
 * Added: compose key support
 *
 * for 0.83: output a "+" for KT_LETTER
 * for 0.85: with -i option: also output MAX_DIACR
 * for 0.86: with -l option: also tell about synonyms
 * for 0.87: output "charset iso-8859-x" so that loadkeys
 *      can handle the output of dumpkeys -c
 * for 0.88: handle sparse keymaps
 *
 * for 0.94: support alt_is_meta
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <sysexits.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/kd.h>
#include <linux/keyboard.h>

#include <lct/ksyms.h>
#include <lct/local.h>
#include <lct/utils.h>
#include <lct/console.h>
#include <lct/modifiers.h>

#ifndef KT_LETTER
#define KT_LETTER KT_LATIN
#endif

#ifndef MAX_NR_KEYMAPS
#define MAX_NR_KEYMAPS NR_KEYMAPS
#endif

int verbose;

static int fd;
char* progname;

int keymap_index[MAX_NR_KEYMAPS];		/* inverse of good_keymap */
int good_keymap[MAX_NR_KEYMAPS], keymapnr, allocct;

void get_keymaps(void) {
	int i, j;
	struct kbentry ke;

	keymapnr = allocct = 0;
	for (i=0; i<MAX_NR_KEYMAPS; i++) {
	    ke.kb_index = 0;
	    ke.kb_table = i;
	    j = ioctl(fd, KDGKBENT, (unsigned long)&ke);
	    if (j && errno != EINVAL) {
		fprintf(stderr, _("KDGKBENT at index 0 in table %d: "), i);
		perror("");
		exit (EX_OSERR);
	    }
	    if (!j && ke.kb_value != K_NOSUCHMAP) {
		keymap_index[i] = keymapnr;
		good_keymap[keymapnr++] = i;
		if (ke.kb_value == K_ALLOCATED)
		  allocct++;
	    } else {
		keymap_index[i] = -1;
	    }
	}
	if (keymapnr == 0) {
	    fprintf(stderr, _("%s: cannot find any keymaps?\n"), progname);
	    exit (EX_NOINPUT);
	}
	if (good_keymap[0] != 0) {
	    fprintf(stderr,
		    _("%s: plain map not allocated? very strange ...\n"), progname);
	    /* this is not fatal */
	}
}

void print_keymaps(void) {
	int i,m0,m;

	printf("keymaps ");
	for (i=0; i<keymapnr; i++) {
	    if (i)
	      printf(",");
	    m0 = m = good_keymap[i];
	    while (i+1 < keymapnr && good_keymap[i+1] == m+1)
	      i++, m++;
	    if (m0 == m)
	      printf("%d", m0);
	    else
	      printf("%d-%d", m0, m);
	}
	printf("\n");
}

int get_bind(u_char index, u_char table) {
	struct kbentry ke;

	ke.kb_index = index;
	ke.kb_table = table;
	if (ioctl(fd, KDGKBENT, (unsigned long)&ke)) {
		fprintf(stderr, _("KDGKBENT at index %d in table %d: "),
			index, table);
		perror("");
		exit (EX_OSERR);
	}
	return ke.kb_value;
}

void print_keysym(int code, char numeric) {
	int t;
	int v;
	char *p;

	printf(" ");
	t = KTYP(code);
	v = KVAL(code);
	if (t > KT_LETTER) {
	        printf("U+%04x          ", code ^ 0xf000);
		return;
	}
	if (t == KT_LETTER) {
	        t = KT_LATIN;
		    printf("+");
	}
	if (!numeric && t < syms_size && v < syms[t].size &&
		 (p = syms[t].table[v])[0])
	     printf("%-16s", p);
	else if (!numeric && t == KT_META && v < 128 && v < syms[0].size &&
		 (p = syms[0].table[v])[0])
	     printf("Meta_%-11s", p);
	else
	     printf("0x%04x          ", code);
}

char valid_type(int t) {
	struct kbentry ke;
	char status;

	ke.kb_index = 0;
	ke.kb_table = 0;
	ke.kb_value = K(t, 0);
	status = (ioctl(fd, KDSKBENT, (unsigned long)&ke) == 0);
	return status;
}

u_char maximum_val(int t) {
	struct kbentry ke, ke0;
	int i;

	ke.kb_index = 0;
	ke.kb_table = 0;
	ke.kb_value = K_HOLE; /* useless ? */
	ke0 = ke;
	ioctl(fd, KDGKBENT, (unsigned long)&ke0);

	for (i = 0; i < 256; i++) {
		ke.kb_value = K(t, i);
		if (ioctl(fd, KDSKBENT, (unsigned long)&ke))
			break;
	}
	ke.kb_value = K_HOLE;
	ioctl(fd, KDSKBENT, (unsigned long)&ke0);

	return i - 1;
}

#define NR_TYPES 15
int maxval[NR_TYPES];

#ifdef KDGKBDIACR
/* isgraph() does not know about iso-8859; printing the character
   unescaped makes the output easier to check. Maybe this should
   be an option. Use locale? */
void
outchar (unsigned char c) {
        printf("'");
        printf((c == '\'' || c == '\\') ? "\\%c"
	       : (isgraph(c) || c == ' ' || c >= 0200) ? "%c"
	       : "\\%03o", c);
	printf("'");
}

struct kbdiacrs kd;

void get_diacs(void) {
	static int got_diacs = 0;

	if(!got_diacs && ioctl(fd, KDGKBDIACR, (unsigned long)&kd)) {
	    perror("");
	    fprintf(stderr, _("KDGKBDIACR failed\n"));
	    exit (EX_OSERR);
	}
	got_diacs = 1;
}

int nr_of_diacs()
{
  get_diacs();
  return kd.kb_cnt;
}

void dump_diacs()
{
  unsigned i;

  /* NB. Don't translate since these need to be in the right format
   * to be loaded back into 'loadkeys'
   */
  get_diacs();
  for (i = 0; i < kd.kb_cnt; i++) 
    {
      printf("compose ");
      outchar(kd.kbdiacr[i].diacr);
      printf(" ");
      outchar(kd.kbdiacr[i].base);
      printf(" to ");
      outchar(kd.kbdiacr[i].result);
      printf("\n");
    }
}
#endif        

void show_short_info(void) {
	int i;

	printf(_("keycode range supported by kernel:           1 - %d\n"),
	       NR_KEYS - 1);
	printf(_("max number of actions bindable to a key:         %d\n"),
	       MAX_NR_KEYMAPS);
	get_keymaps();
	printf(_("number of keymaps in actual use:                 %d\n"),
	       keymapnr);
	if (allocct)
	  printf(_("of which %d dynamically allocated\n"), allocct);
	printf(_("ranges of action codes supported by kernel:\n"));
	for (i = 0; i < NR_TYPES && valid_type(i); i++) {
	    maxval[i] = maximum_val(i);
	    printf("	0x%04x - 0x%04x\n", K(i, 0), K(i, maxval[i]));
	}
	printf(_("number of function keys supported by kernel: %d\n"),
	       MAX_NR_FUNC);

	printf(_("max nr of compose definitions: %d\n"),
	       MAX_DIACR);
#ifdef KDGKBDIACR
	printf(_("nr of compose definitions in actual use: %d\n"),
	       nr_of_diacs());
#endif
}

struct {
    char *name;
    int bit;
} modifiers[] = {
    { "shift",	KG_SHIFT  },
    { "altgr",	KG_ALTGR  },
    { "control",KG_CTRL   },
    { "alt",	KG_ALT    },
    { "shiftl",	KG_SHIFTL },
    { "shiftr",	KG_SHIFTR },
    { "ctrll",	KG_CTRLL  },
    { "ctrlr",	KG_CTRLR  }
};

void dump_symbols(void) {
	int t;
	int v;
	char *p;

	printf(_("Symbols recognized by %s:\n(numeric value, symbol)\n\n"), progname);
	for (t = 0; t < syms_size; t++) {
	    if (syms[t].size) {
		for (v = 0; v < syms[t].size; v++)
			if ((p = syms[t].table[v])[0])
				printf("0x%04x\t%s\n", K(t, v), p);
	    } else if (t == KT_META) {
		for (v = 0; v < syms[0].size && v < 128; v++)
			if ((p = syms[0].table[v])[0])
				printf("0x%04x\tMeta_%s\n", K(t, v), p);
	    }
	}
	printf(_("\nThe following synonyms are recognized:\n\n"));
	for (t = 0; t < syn_size; t++)
	  printf("%-15s for %s\n", synonyms[t].synonym,
		 synonyms[t].official_name);
	printf(_("\nRecognized modifier names and their column numbers:\n"));
	for (t = 0; t < sizeof(modifiers)/sizeof(modifiers[0]); t++)
	  printf("%s\t\t%3d\n", modifiers[t].name, 1 << modifiers[t].bit);
}

void print_mod(int x) 
{
  unsigned t;
  
  if (!x)
    printf("plain\t");
  else
    for (t = 0; t < sizeof(modifiers)/sizeof(modifiers[0]); t++)
      if (x & (1 << modifiers[t].bit))
	printf("%s\t", modifiers[t].name);
}

void print_bind(int bufj, int i, int j, char numeric) {
  if (j)
    printf("\t");
  print_mod(j);
  printf("keycode %3d =", i);
  print_keysym(bufj, numeric);
  printf("\n");
}

#define DEFAULT		0
#define FULL_TABLE	1	/* one line for each keycode */
#define SEPARATE_LINES	2	/* one line for each (modifier,keycode) pair */
#define	UNTIL_HOLE	3	/* one line for each keycode, until 1st hole */

void dump_keys(char table_shape, char numeric) {
	int i, j, k;
	int buf[MAX_NR_KEYMAPS];
	int isletter, islatin, isasexpected;
	int typ, val;
	int alt_is_meta = 0;
	int zapped[MAX_NR_KEYMAPS];

	get_keymaps();
	print_keymaps();
	if (!keymapnr)
	  return;

	if (table_shape == FULL_TABLE || table_shape == SEPARATE_LINES)
	  goto no_shorthands;

	/* first pass: determine whether to set alt_is_meta */
	for (j = 0; j < MAX_NR_KEYMAPS; j++) {
	     int ja = (j | M_ALT);
	     if (j != ja && keymap_index[j] >= 0 && keymap_index[ja] >= 0)
		  for (i = 1; i < NR_KEYS; i++) {
		       int buf0, buf1, type;

		       buf0 = get_bind(i, j);
		       type = KTYP(buf0);
		       if ((type == KT_LATIN || type == KT_LETTER)
			   && KVAL(buf0) < 128) {
			    buf1 = get_bind(i, ja);
			    if (buf1 != K(KT_META, KVAL(buf0))) {
				 if (verbose) {
				      printf("# not alt_is_meta: "
		      		"on keymap %d key %d is bound to",
					      ja, i);
				      print_keysym(buf1, numeric);
				      printf("\n");
				 }
				 goto not_alt_is_meta;
			    }
		       }
		  }
	}
	alt_is_meta = 1;
	printf("alt_is_meta\n");
not_alt_is_meta:

no_shorthands:
	for (i = 1; i < NR_KEYS; i++) {
	    for (j = 0; j < keymapnr; j++)
	      buf[j] = get_bind(i, good_keymap[j]);

	    if (table_shape == FULL_TABLE) {
		printf("keycode %3d =", i);
		for (j = 0; j < keymapnr; j++)
		  print_keysym(buf[j], numeric);
		printf("\n");
		continue;
	    }

	    if (table_shape == SEPARATE_LINES) {
		for (j = 0; j < keymapnr; j++)
		  print_bind(buf[j], i, good_keymap[j], numeric);
		printf("\n");
		continue;
	    }

	    typ = KTYP(buf[0]);
	    val = KVAL(buf[0]);
	    islatin = (typ == KT_LATIN || typ == KT_LETTER);
	    isletter = (islatin &&
			((val >= 'A' && val <= 'Z') ||
			 (val >= 'a' && val <= 'z')));
	    isasexpected = 0;
	    if (isletter) {
		u_short defs[16];
		defs[0] = K(KT_LETTER, val);
		defs[1] = K(KT_LETTER, val ^ 32);
		defs[2] = defs[0];
		defs[3] = defs[1];
		for(j=4; j<8; j++)
		  defs[j] = K(KT_LATIN, val & ~96);
		for(j=8; j<16; j++)
		  defs[j] = K(KT_META, KVAL(defs[j-8]));

		for(j = 0; j < keymapnr; j++) {
		    k = good_keymap[j];
		    if ((k >= 16 && buf[j] != K_HOLE) || (k < 16 && buf[j] != defs[k]))
		      goto unexpected;
		}
		isasexpected = 1;
	    }
	  unexpected:

	    for (j = 0; j < keymapnr; j++)
		    zapped[j] = 0;
	    /* wipe out predictable meta bindings */
	    if (alt_is_meta) {
		 for(j = 0; j < keymapnr; j++) {
		      int ka, ja, typ;
		      k = good_keymap[j];
		      ka = (k | M_ALT);
		      ja = keymap_index[ka];
		      if (k != ka && ja >= 0
		       && ((typ=KTYP(buf[j])) == KT_LATIN || typ == KT_LETTER)
		       && KVAL(buf[j]) < 128) {
			   if (buf[ja] != K(KT_META, KVAL(buf[j])))
				fprintf(stderr, _("impossible: not meta?\n"));
			   buf[ja] = K_HOLE;
			   zapped[ja] = 1;
		      }
		 }
	    }

	    printf("keycode %3d =", i);
	    if (isasexpected) {
		/* print only a single entry */
		/* suppress the + for ordinary a-zA-Z */
		print_keysym(K(KT_LATIN, val), numeric);
		printf("\n");
	    } else {
		/* choose between single entry line followed by exceptions,
		   and long line followed by exceptions; avoid VoidSymbol */
		int bad = 0;
		int count = 0;
		for(j = 1; j < keymapnr; j++) if (!zapped[j]) {
		    if (buf[j] != buf[0])
		      bad++;
		    if (buf[j] != K_HOLE)
		      count++;
		}
		if (bad <= count && bad < keymapnr-1) {
		    if (buf[0] != K_HOLE)
		      print_keysym(buf[0], numeric);
		    printf("\n");
		    for (j = 1; j < keymapnr; j++)
		      if (buf[j] != buf[0] && !zapped[j])
			print_bind(buf[j], i, good_keymap[j], numeric);
		} else {
		    for (j = 0; j < keymapnr && buf[j] != K_HOLE &&
				 (j == 0 || table_shape != UNTIL_HOLE ||
				  good_keymap[j] == good_keymap[j-1]+1); j++)
		      print_keysym(buf[j], numeric);
		    printf("\n");
		    for ( ; j < keymapnr; j++)
		      if (buf[j] != K_HOLE)
			print_bind(buf[j], i, good_keymap[j], numeric);
		}
	    }
	}
}

void dump_funcs() {
	int i;
	struct kbsentry fbuf;
	char *p;

	for (i = 0; i < MAX_NR_FUNC; i++) {
		fbuf.kb_func = i;
		if (ioctl(fd, KDGKBSENT, (unsigned long)&fbuf)) {
		    if (errno == EINVAL && i > 0) /* an old kernel */
		      break;
		    fprintf(stderr, _("KDGKBSENT at index %d: "), i);
		    perror("");
		    exit (EX_OSERR);
		}
		if (!fbuf.kb_string[0])
		        continue;
		printf("string %s = \"", syms[KT_FN].table[i]);
		for (p = fbuf.kb_string; *p; p++) {
		        if (*p == '"' || *p == '\\') {
			        putchar('\\'); putchar(*p);
			} else if (isgraph(*p) || *p == ' ')
				putchar(*p);
			else
				printf("\\%03o", *p);
		}
		printf("\"\n");
	}
}

void usage() 
{
  struct cs *c;
  
  printf(_("Usage: %s [options...]\n"), progname);
  OPTIONS_ARE();
  OPT("-i --short-info      ", _("display information about keyboard driver"));
  OPT("-l --long-info       ", _("display above and symbols known to loadkeys"));
  OPT("-n --numeric         ", _("display keytable in hexadecimal notation"));
  OPT("-f --full-table      ", _("don't use short-hand notations, one row per keycode"));
  OPT("-1 --separate-lines  ", _("one line per (modifier,keycode) pair"));
  OPT("   --funcs-only      ", _("display only the function key strings"));
  OPT("   --keys-only       ", _("display only key bindings"));
  OPT("-S --shape=<shape>   ", _("choose output shape, where <shape> is:"));
  OPT("                     ", _(" 0 = default"));
  OPT("                     ", _(" 1 = same as --full-table"));
  OPT("                     ", _(" 2 = same as --separate-lines"));
  OPT("                     ", _(" 3 = one line per keycode, until 1st hole"));
#ifdef KDGKBDIACR
  OPT("   --compose-only    ", _("display only compose key combinations"));
#endif
  OPT("-c --charset=<charset-name>", "");
  OPT("                     ", _("interpret character action codes to be from the"));
  OPT("                     ", _("specified character set"));

  OPT("-h --help            ", _("display this help text and exit"));
  OPT("-V --version         ", _("display version information and exit"));

  printf (_("\navailable charsets:\n"));

  for (c = charsets; c->charset ; c++)
    printf("\t%s\n", c->charset);
}

int main (int argc, char *argv[]) 
{
  const char *short_opts = "hilvsnf1S:c:";
  const struct option long_opts[] = 
  {
    { "help",	no_argument,		NULL, 'h' },
    { "short-info",	no_argument,		NULL, 'i' },
    { "long-info",	no_argument,		NULL, 'l' },
    { "numeric",	no_argument,		NULL, 'n' },
    { "full-table",	no_argument,		NULL, 'f' },
    { "separate-lines",no_argument,		NULL, '1' },
    { "shape",	required_argument,	NULL, 'S' },
    { "funcs-only",	no_argument,		NULL, 't' },
    { "keys-only",	no_argument,		NULL, 'k' },
#ifdef KDGKBDIACR
    { "compose-only",no_argument,           NULL, 'd' },
#endif
    { "verbose",	no_argument,		NULL, 'v' },
    { "version",        no_argument,            NULL, 'V' },
    { "charset",	required_argument,	NULL, 'c' },
    { NULL,	0, NULL, 0 }
  };
  int c;
  char long_info = 0;
  char short_info = 0;
  char numeric = 0;
  char table_shape = 0;
  char funcs_only = 0;
  char keys_only = 0;
#ifdef KDGKBDIACR
  char diac_only = 0;
#endif

  setuplocale();
  
  progname = strip_path(argv[0]);
  
  while ((c = getopt_long(argc, argv,
			  short_opts, long_opts, NULL)) != EOF)
    {
      switch (c) 
	{
	case 'i':
	  short_info = 1;
	  break;
	case 's':
	case 'l':
	  long_info = 1;
	  break;
	case 'n':
	  numeric = 1;
	  break;
	case 'f':
	  table_shape = FULL_TABLE;
	  break;
	case '1':
	  table_shape = SEPARATE_LINES;
	  break;
	case 'S':
	  table_shape = atoi(optarg);
	  break;
	case 't':
	  funcs_only = 1;
	  break;
	case 'k':
	  keys_only = 1;
	  break;
#ifdef KDGKBDIACR
	case 'd':
	  diac_only = 1;
	  break;
#endif
	case 'v':
	  verbose = 1;
	  break;
	case 'V':
	  version();
	  exit(0);
	case 'c':
	  if ((set_charset(optarg)) != 0)
	    badusage((char*)NULL);   /* message already printed */
	  printf("charset \"%s\"\n", optarg);
	  break;
	case 'h':
	  usage();
	  exit (0);
	case '?':
	  badusage((char*)NULL);
	}
    }

  if (optind < argc)
    badusage(_("no non-option arguments allowed"));
  
  if (-1 == (fd = get_console_fd(NULL)))
    exit (EX_OSERR);
  
  if (short_info || long_info) 
    {
      show_short_info();
      if (long_info)
	dump_symbols();
      exit (EX_OK);
    }

#ifdef KDGKBDIACR
  if (!diac_only) 
    {
#endif
      if (!funcs_only)
	dump_keys(table_shape, numeric);
      if (!keys_only)
	dump_funcs();
#ifdef KDGKBDIACR
    }
  if (!funcs_only && !keys_only)
    dump_diacs();
#endif
  
  exit (EX_OK);
}
