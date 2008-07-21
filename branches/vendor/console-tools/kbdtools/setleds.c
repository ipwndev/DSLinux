/*
 * setleds.c - aeb, 940130, 940909
 *
 * Call: setleds [-L] [-D] [-F] [{+|-}{num|caps|scroll}]*
 * will set or clear the indicated flags on the stdin tty,
 * and report the settings before and after.
 * In particular, setleds without arguments will only report.
 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <linux/kd.h>
#include <sys/ioctl.h>

#include <lct/local.h>
#include <lct/utils.h>


#define onoff(a) ((a) ? _("on ") : _("off"))

int verbose;


/* FIXME: restate in GNU terms */
static void usage(char *progname)
{
  printf(_("Usage:"
	    "	%1$s [-v] [-L] [-D] [-F] [[+|-][ num | caps | scroll ]]\n"
	    "Thus,\n"
	    "	%1$s +caps -num\n"
	    "will set CapsLock, clear NumLock and leave ScrollLock unchanged.\n"
	    "The settings before and after the change (if any) are reported\n"
	    "when the -v option is given or when no change is requested.\n"
	    "Normally, %1$s influences the vt flag settings\n"
	    "(and these are usually reflected in the leds).\n"
	    "With -L, %1$s only sets the leds, and leaves the flags alone.\n"
	    "With -D, %1$s sets both the flags and the default flags, so\n"
	    "that a subsequent reset will not change the flags.\n")
	  , progname);
}

static void parse_cmdline (int argc, char *argv[],
			   int *optL, int *optD, int *optF, 
			   char *nval, char *ndef )
{
  char *progname = strip_path (argv[0]);
    const struct option long_opts[] = {
      { "leds-only", no_argument, NULL, 'L' },
      { "help"     , no_argument, NULL, 'h' },
      { "verbose"  , no_argument, NULL, 'v' },
      { "version"  , no_argument, NULL, 'V' },
      { "show-current", no_argument, NULL, 'F' },
      { "set-all"     , no_argument, NULL, 'D' },
      { "caps"    , no_argument, NULL, 'c' },
      { "num",    no_argument, NULL, 'n' },
      { "scroll", no_argument, NULL, 's' },
      { NULL, 0, NULL, 0 }
    };
    int c;
    
    while ( (c = getopt_long_only(argc, argv, "-vhVFDL", long_opts, NULL)) != EOF)       
      switch (c) {
      case 'h':
	usage(progname);
	exit(0);
      case 'V':
	version(progname);
	exit(0);
      case 'v':
	verbose = 1;
	break;
      case 'L':
	*optL = 1;
	break;
      case 'D':
	*optD = 1;
	break;
      case 'F':
	*optF = 1;
	break;
      case 'c':
	*ndef |= LED_CAP;
	break;
      case 's':
	*ndef |= LED_SCR;
	break;
      case 'n':
	*ndef |= LED_NUM;
	break;	
      case 1:			/* non-GNU arguments */
	if (!strcmp(optarg,"+caps")) {
	  *ndef |= LED_CAP;
	  *nval |= LED_CAP;
	} else if (!strcmp(optarg,"+num")) {
	  *ndef |= LED_NUM;
	  *nval |= LED_NUM;
	} else if (!strcmp(optarg, "+scroll")) {
	  *ndef |= LED_SCR;
	  *ndef |= LED_SCR;
	} else {
	  fprintf (stderr, _("%s: unknown argument: %s\n"),
		   progname, optarg);
	  exit (1);
	}	
	break;
      }
}
    
    


void
report(leds) int leds;
{
    printf(_("NumLock %s   CapsLock %s   ScrollLock %s\n"),
	   onoff(leds & LED_NUM),
	   onoff(leds & LED_CAP),
	   onoff(leds & LED_SCR));
}


int
main(int argc, char **argv)
{
    int optL = 0, optD = 0, optF = 0;
    char oleds, nleds, oflags, nflags, odefflags, ndefflags;
    char nval = 0, ndef = 0;

    setuplocale();
    
    parse_cmdline (argc, argv, &optL, &optD, &optF, &nval, &ndef);
    
    /* Use getopt rather than any other mechanism because future support in eg. bash
     * may use it to provide command-line completion of option arguments
     */

    /* Do these after command line handling so 'setleds --help works on a VT, etc. */
    if (ioctl(0, KDGETLED, &oleds)) {
	perror("KDGETLED");
	fprintf(stderr,
		_("Error reading current led setting. Maybe stdin is not a VT?\n"));
	exit(1);
    }

    if (ioctl(0, KDGKBLED, &oflags)) {
	perror("KDGKBLED");
	fprintf(stderr,
		_("Error reading current flags setting. Maybe an old kernel?\n"));
	exit(1);
    }


    odefflags = ndefflags = ((oflags >> 4) & 7);
    oflags = nflags = (oflags & 7);

    if (argc <= 1) {
	if (optL) {
	    nleds = 0xff;
	    if (ioctl(0, KDSETLED, &nleds)) {
		perror("KDSETLED");
		fprintf(stderr, _("Error resetting ledmode\n"));
		exit(1);
	    }
	}

	/* If nothing to do, report, even if not verbose */
	if (!optD && !optL && !optF)
	  optD = optL = optF = 1;
	if (optD) {
	    printf(_("Current default flags:  "));
	    report(odefflags);
	}
	if (optF) {
	    printf(_("Current flags:          "));
	    report(oflags & 07);
	}
	if (optL) {
	    printf(_("Current leds:           "));
	    report(oleds);
	}
	exit(0);
    }

    if (!optL)
      optF = 1;

    if (optD) {
	ndefflags = (odefflags & ~ndef) | nval;
	if (verbose) {
	    printf(_("Old default flags:    "));
	    report(odefflags);
	    printf(_("New default flags:    "));
	    report(ndefflags);
	}
    }
    if (optF) {
	nflags = ((oflags & ~ndef) | nval);
	if (verbose) {
	  printf(_("Old flags:            "));
	    report(oflags & 07);
	    printf(_("New flags:            "));
	    report(nflags & 07);
	}
    }
    if (optD || optF) {
	if (ioctl(0, KDSKBLED, (ndefflags << 4) | nflags)) {
	    perror("KDSKBLED");
	    exit(1);
	}
    }
    if (optL) {
	nleds = (oleds & ~ndef) | nval;
	if (verbose) {
	  printf(_("Old leds:             "));
	  report(oleds);
	  printf(_("New leds:             "));
	  report(nleds);
	}
	if (ioctl(0, KDSETLED, nleds)) {
	    perror("KDSETLED");
	    exit(1);
	}
    }
    exit(0);
}
