/*
 * setfont.c - Eugene Crosser & Andries Brouwer
 * modified 1997 Yann Dirson
 *
 * Loads the console font, and possibly the corresponding screen map(s).
 * We accept two kind of screen maps, one [-m] giving the correspondence
 * between some arbitrary 8-bit character set currently in use and the
 * font positions, and the second [-u] giving the correspondence between
 * font positions and Unicode values.
 */
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <sysexits.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

#include <lct/local.h>
#include <lct/console.h>
#include <lct/font.h>

#define DEFUNIMAP DATADIR "/" TRANSDIR "/def"

int verbose = 0;
int no_act = 0;

char *progname;
char pathname[1024];

static int saveoldfont(int fd, char *outfile, int as_psf, int with_unimap);
static void loadnewfont(int fd,	       /* FD of the console for ioctl() */
		 char* ifil,	       /* input font-file name, or "" for default */
		 int iunit,	       /*  */
		 struct unimapdesc* ud,/* output parameter for sfm if any */
		 int no_u); 	       /* 1 <=> do not load a unimap here [used for PSF] */

static void usage()
{
  version();
  printf(_("Usage:  %s [options] [commands]\n"), progname);
  OPTIONS_ARE();
  OPT(" -v --verbose        ", _("List operations as they are done"));
  OPT(" -n --no-act         ", _("Do not change the console state nor write to any file"));
  OPT(" -H --char-height=N  ", _("(N in 0..32) Choose the right font from a codepage that\n"
				 "contains three fonts (only 8/14/16 allowed then), or choose\n"
				 "default font, ie. \"default8xN\""));
  OPT(" --force-no-sfm      ", _("Suppress loading of a screen-font map [use with care]"));
  OPT(" -1 --g1             ", _("When loading an ACM, activate G1 charset instead of G0"));
  OPT(" --tty=device        ", _("Use `device' as console device for ioctls"));
  COMMANDS_ARE();
  OPT(" -f --font=file      ", _("Load the console-font from specified file"));
  OPT(" -d --default-font   ", _("Load a default font from a file"));
  OPT(" -R --rom-font       ", _("Restore ROM font (does not work with all kernels)"));
  OPT(" -u --sfm --screen-font-map=file", "");
  OPT("                     ", _("Load the SFM from specified file\n"
				 "(instead of the one in font-file, if any)"));
  OPT(" -k --sfm-fallback=file  ", _("Merge SFM fallbacks from file into SFM"));
  OPT(" -m --acm --app-charset-map=file  ", _("Load the ACM from specified file"));
  OPT(" -F --old-font=file  ", _("Write current font to prefered format (now: psf-with-sfm)"));
  OPT(" --old-font-psf=file ", _("Write current font to PSF file before loading a new one"));
  OPT(" --old-font-psf-with-sfm=file", "");
  OPT("                     ", _("Same as -old-font-psf, and add current SFM in the PSF file"));
  OPT(" --old-font-raw=file ", _("Write current font to RAW file before loading a new one"));
  OPT(" -M --old-acm=file   ", _("Write current ACM to file before loading a new one"));
  OPT(" -U --old-sfm=file   ", _("Write current SFM to file before loading a new one"));

  OPT(" -h --help           ", HELPDESC);
  OPT(" -V --version        ", VERSIONDESC);
}

static inline void CHECK_STD(char* file, int* stdfile)
{
  if (!strcmp(file, "-"))
    {
      if (*stdfile)
	  badusage(_("too many `-' as filenames"));
      else
	  *stdfile = 1;
    }
}

#define OPT_or 128
#define OPT_op 129
#define OPT_no_sfm 130
#define OPT_tty 131

const struct option opts[] = 
{
  /* operations */
    {"help", no_argument, 0, 'h'},
    {"version", no_argument, 0, 'V'},
    {"verbose", no_argument, 0, 'v'},
    {"tty", required_argument, 0, OPT_tty},
    {"g1", no_argument, 0, '1'},
  
    {"char-height", required_argument, 0, 'H'},
    {"no-act", no_argument, 0, 'n'},

    {"font", required_argument, 0, 'f'},
    {"old-font", required_argument, 0, 'F'},
  
    {"default-font", no_argument, 0, 'd'},
    {"rom-font", no_argument, 0, 'R'},

  /* default old-font is synonym for old-font-psf-with-sfm */
  /* this may change when better formats are supported */
    {"old-font-psf-with-sfm", required_argument, 0, 'F'},
    {"old-font-psf", required_argument, 0, OPT_op},
    {"old-font-raw", required_argument, 0, OPT_or},
  
    {"screen-font-map", required_argument, 0, 'u'},
    {"sfm", required_argument, 0, 'u'},
    {"app-charset-map", required_argument, 0, 'm'},
    {"acm", required_argument, 0, 'm'},

    {"force-no-sfm", no_argument, 0, OPT_no_sfm},
  
    {"old-sfm", required_argument, 0, 'U'},
    {"old-acm", required_argument, 0, 'M'},
  
    {"sfm-fallback", required_argument, 0, 'k'},
  
  /* end marker */
    {0, 0, 0, 0}
};

/* const for default-font handling - loadnewfont() wants it to be empty */
char* default_font = "";
char* rom_font = (char*)(-1);

void main(int argc, char *argv[])
{
  char *ifil, *mfil, *ufil, *orfil, *opfil, *omfil, *oufil, *opufil, **fallback_files;
  int nb_fallback_files = 0;
  int fd = 0;
  int requested_height, no_u;
  int stdin_taken, stdout_taken;
  char* tty_name = NULL;	       /* optional name of a tty for the console */
  int g_set = 0;		       /* G0 or G1 */
  
  struct unimapdesc ud;
  
  int result;			       /* option handling */
  int an_option;

  ifil = mfil = ufil = orfil = opfil = omfil = oufil = opufil = NULL;
  fallback_files = NULL;
  requested_height = 0;
  stdin_taken = stdout_taken = 0;      /* 1 means respective FILE is already used for file I/O */
  no_u = 0;			       /* 1 means ignore map inside file */

  setuplocale();
  
  progname = strip_path(argv[0]);

  while (1)
    {
      result = getopt_long(argc, argv, "Vhvn1H:f:F:dRu:m:U:M:k:", opts, &an_option);

      if (result == EOF)
	  break;
      
      switch (result)
	{
	case 'V':
	  version();
	  exit (EX_OK);
	case 'h':
	  usage();
	  exit (EX_OK);
	case 'v':
	  verbose = 1;
	  break;
	case 'n':
	  no_act = verbose = 1;
	  break;
	case OPT_tty:
	  tty_name = optarg;
	  break;
	case '1':
	  g_set = 1;
	  break;
	  
	case 'H':
	  /* numeric height */
	  requested_height = atoi(optarg);
	  if(requested_height <= 0 || requested_height > 32)
	      badusage(_("--char-height argument should be in 1..31"));
	  break;

	case 'f':
	  if (ifil)
	      badusage(_("only one font file is allowed"));
	  ifil = optarg;
	  CHECK_STD(ifil, &stdin_taken);
	  break;

	case 'd':
	  if (ifil)
	      badusage(_("only one font file is allowed"));
	  ifil = default_font;
	  break;
	  
	case 'R':
	  if (ifil)
	      badusage(_("only one font file is allowed"));
	  ifil = rom_font;
	  break;
	  
	case 'm':
	  if (mfil)
	      badusage(_("only one ACM is allowed"));
	  mfil = optarg;
	  CHECK_STD(mfil, &stdin_taken);
	  break;

	case 'u':
	  if (ufil)
	      badusage(_("only one SFM is allowed"));
	  if (no_u)
	      badusage(_("multiple requests for SFM handling"));
	  ufil = optarg;
	  CHECK_STD(ufil, &stdin_taken);
	  no_u = 1;
	  break;
	  
	case OPT_no_sfm:
	  fprintf (stderr, _("WARNING: not using a unimap may lead to erroneous display !\n"));
	  no_u = 1;
	  break;
	  
	case 'k':
	  CHECK_STD(optarg, &stdin_taken);
	  if (NULL ==
	      (fallback_files = (char**)realloc (fallback_files,
						 (nb_fallback_files + 1) * sizeof (char*))))
	    {
	      perror (_("realloc fallback_files"));
	      exit (EX_OSERR);
	    }
	  fallback_files[nb_fallback_files] = optarg;
	  nb_fallback_files++;
	  break;
	  
	case OPT_or:
	  if (orfil)
	      badusage(_("only one output RAW font-file is allowed"));
	  orfil = optarg;
	  CHECK_STD(orfil, &stdout_taken);
	  break;
	  
	case OPT_op:
	  if (opfil)
	      badusage(_("only one output PSF font-file is allowed"));
	  opfil = optarg;
	  CHECK_STD(opfil, &stdout_taken);
	  break;
	  
	case 'F':
	  if (opufil)
	      badusage(_("only one output PSF+SFM font-file is allowed"));
	  opufil = optarg;
	  CHECK_STD(opufil, &stdout_taken);
	  break;

	case 'M':
	  if (omfil)
	      badusage(_("only one output ACM file is allowed"));
	  omfil = optarg;
	  CHECK_STD(omfil, &stdout_taken);
	  break;
	  
	case 'U':
	  if (oufil)
	      badusage(_("only one output SFM file is allowed"));
	  oufil = optarg;
	  CHECK_STD(oufil, &stdout_taken);
	  break;

	case '?':
	  /* message was written by getopt() */
	  badusage((char*)NULL);
	  break;
	  
	default:
	  badusage(_("unknown option"));
	}
    }

  if (optind < argc)
      badusage (_("no non-option arguments are valid"));

  if (!ifil && !mfil && !ufil && !orfil && !opfil && !omfil && !oufil && !opufil && !fallback_files)
      badusage(_("nothing to do"));

  if (-1 == (fd = get_console_fd(tty_name)))
    {
      perror ("get_console_fd");
      exit (1);
    }

  /*
   * save current state
   */
  
  if (orfil)
      if (saveoldfont(fd, orfil, 0, 0))
	  perror (_("Saving raw old font")), exit(1);

  if (opfil)
      if (saveoldfont(fd, opfil, 1, 0))
	  perror (_("Saving PSF old font")), exit(1);

  if (opufil)
      if (saveoldfont(fd, opufil, 1, 1))
	  perror (_("Saving PSF+unimap old font")), exit(1);

  if (omfil)
    {
      if (no_act)
	   fprintf (stderr, _("Would save ACM to file `%s'.\n"), omfil);
      else
	{
	  if (verbose) fprintf (stderr, _("Saving ACM to file `%s'.\n"), omfil);
	  saveoldmap(fd, omfil);
	}
    }
  
  if (oufil)
      saveunicodemap(fd, oufil, verbose, no_act);
  
  /*
   * set new state
   */
  
  if (mfil)
    {
      FILE* f;
      char pathname[1024];
      
      if ((f = findacm(mfil, pathname, sizeof(pathname), stdin, NULL)) == NULL)
	  perror("findacm"), exit(1);
      

      if (no_act)
	  printf(_("Would load ACM from `%s'\n"), pathname);
      else
	{
	  if (verbose) printf(_("Loading ACM from `%s'\n"), pathname);
	  
	  if (-1 == screen_map_load(fd, f))
	    fprintf (stderr, _("Error reading ACM file.\n"));
	  else
	    if (acm_activate(fd, g_set))
	      perror ("acm_activate"), exit (EX_UNAVAILABLE);
	}
    }
  
  /* load a font and get a unimap if any */
  ud.entry_ct = 0;
  if (ifil)
    {
      if (ifil == rom_font)
	restore_rom_font(fd);
      else
	loadnewfont(fd, ifil, requested_height, &ud, no_u);
    }
  
  if (ufil)
    {
      char pathname[1024];
      FILE* mapf;
      
      if (NULL == (mapf = findsfm(ufil, pathname, sizeof(pathname), stdin, NULL)))
	  perror("findsfm"), exit (1);
      
      if (no_act)
	  fprintf (stderr, _("Would read screen-font map from %s.\n"), pathname);
      else if (verbose)
	  fprintf (stderr, _("Reading screen-font map from %s.\n"), pathname);

      if (sfm_read_ascii (mapf, &ud, 512))
	  perror ("sfm_read_ascii"), exit (EX_DATAERR);
    }
  
  if (fallback_files)
    {
      struct unimapdesc new_ud;
      
      /* if no SFM would be loaded otherwise,
       * get the current one so we can merge fallbacks in */
      if (!ud.entry_ct)
	{
	  if (verbose) fprintf (stderr, _("Requesting SFM from kernel.\n"));
	  if (-1 == get_kernel_unimap(fd, &ud))
	    {
	      if (errno == ENXIO)
		fprintf(stderr, _("No valid SFM currently loaded. Aborting.\n"));
	      else
		perror ("get_kernel_unimap");
	      exit (1);
	    }
	}
      
      /* duplicate ud into new_ud */
      /* don't duplicate unipair's themselves */
      new_ud.entry_ct = ud.entry_ct;
      if (NULL ==
	  (new_ud.entries = (struct unipair*)malloc (ud.entry_ct * sizeof (struct unipair))))
	{
	  perror ("malloc new_ud.entries");
	  exit (EX_OSERR);
	}
      memcpy (new_ud.entries, ud.entries, ud.entry_ct * sizeof (struct unipair));
      
      if (ud.entry_ct)	/* mostly a sanity check */
	{
	  FILE* fbfile;
	  char fullname[1024];
	  unsigned sfmfb_ct = 0;
	  unicode** sfmfb;
	  int i;

	  for (i=0; i<nb_fallback_files; i++)
	    {
	      if (NULL == (fbfile = findsfmfallback (fallback_files[i],
						     fullname, sizeof(fullname),
						     stdin, NULL)))
		perror ("findsfmfallback"), exit (1);
	  
	      if (verbose)
		fprintf (stderr, _("Reading SFM fallbacks from `%s'.\n"), fullname);
	  
	      sfmfb = NULL;
	      if (-1 == sfm_fallback_read (fbfile, &sfmfb, &sfmfb_ct))
		perror ("sfm_fallback_read"), exit (1);

#ifndef NDEBUG
	      if (verbose)
		fprintf (stderr, _("Read %u fallback entries.\n"), sfmfb_ct);
#endif
	      assert (sfmfb_ct > 0);
	      
	      if (-1 == sfm_fallback_add (sfmfb, sfmfb_ct, &ud, &new_ud))
		perror ("sfm_fallback_add"), exit (1);
	      
	      fclose (fbfile);

	      /* forget about this fallback table */
	      do
		free (sfmfb[--sfmfb_ct]);
	      while (sfmfb_ct>0);
	      free (sfmfb);
	      sfmfb_ct = 0;
	      sfmfb = NULL;
	    }

	  /* free old ud */
	  free (ud.entries);
	  
	  /* replace ud with new_ud */
	  memcpy (&ud, &new_ud, sizeof(struct unimapdesc));
	}
      else
	  fprintf (stderr, _("WARNING: No SFM found in file or kernel ?  Ignoring fallback file.\n"));
    }
	
  
  if (ud.entry_ct)
    {
      if (no_act)
	  fprintf (stderr, _("Would set kernel SFM.\n"));
      else
	{
	  if (verbose) fprintf (stderr, _("Setting kernel SFM.\n"));
	  if (set_kernel_unimap (fd, &ud))
	      perror ("set_kernel_unimap"), exit (EX_OSERR);
	}
    }
  
  exit (EX_OK);
}


void loadnewfont(int fd,	       /* FD of the console for ioctl() */
		 char* ifil,	       /* input font-file name, or "" for default */
		 int requested_height,	       /*  */
		 struct unimapdesc* ud,/* output parameter for sfm if any */
		 int no_u) 	       /* 1 <=> do not load a unimap here [used for PSF] */
{
  FILE *fpi;
  char defname[20];
  int font_magic;
  sigset_t sigset, old_sigset;
  simple_font* the_font;

  /* block SIGCHLD */
  sigemptyset (&sigset);
  sigaddset (&sigset, SIGCHLD);
  sigprocmask (SIG_BLOCK, &sigset, &old_sigset);

  /*
   * First check if we need to find a default font;
   * If yes, open it & set its name for further processing.
   */
  
  if (!*ifil)			       /* ie. if (ifil == "") */
    {
      /* try to find some default file */
      
      if (requested_height < 0 || requested_height > 32)
	requested_height = 0;
      if (requested_height == 0) 
	{
	  if ((fpi = findfont("default", pathname, sizeof(pathname), NULL, &font_magic)) == NULL &&
	      (fpi = findfont("default8x16", pathname, sizeof(pathname), NULL, &font_magic)) == NULL &&
	      (fpi = findfont("default8x14", pathname, sizeof(pathname), NULL, &font_magic)) == NULL &&
	      (fpi = findfont("default8x08", pathname, sizeof(pathname), NULL, &font_magic)) == NULL) 
	    {
	      fprintf(stderr, _("Cannot find a default font file.\n"));
	      exit(1);
	    }
	}
      else
	{
	  sprintf(defname, "default8x%02d", requested_height);
	  if ((fpi = findfont(defname, pathname, sizeof(pathname), NULL, &font_magic)) == NULL) 
	    {
	      fprintf(stderr, _("Cannot find default font file `default8x%02d'.\n"), requested_height);
	      exit(1);
	    }
	}
    }
  else /* i_file was specified */
    {
      if ((fpi = findfont(ifil, pathname, sizeof(pathname), stdin, &font_magic)) == NULL)
	{
	  perror ("findfont");
	  fprintf (stderr, _("Cannot open font file `%s'.\n"), ifil);
	  exit(1);
	}
    }

  /*
   * Find out file size if possible;
   * Maybe correct `pathname' for display purposes
   */
  
  if (!strcmp(pathname,"-"))
    {
      sprintf(pathname, "stdin");
    }

  /* FIXME: use result */
  the_font = read_simple_font (fpi, NULL, font_magic);

  if (!the_font)
    {
      perror ("read_simple_font()");
      exit (1);
    }
  
  fclose(fpi);

  /* FIXME:
   *
   * - insert no-SFM warning somewhere
fprintf (stderr, "WARNING: This font-file does not contain a unimap.\n");
fprintf (stderr, "WARNING: Not using a unimap may lead to erroneous display !\n");
   */

  /* Check that font-size is valid */
#if !defined( PIO_FONTX ) || defined( sparc )
  if (the_font->font.charcount != 256) 
    {
      fprintf(stderr, _("Only fontsize 256 supported.\n"));
      exit(1);
    }
#endif

  
  /*
   * send font to kernel
   */
#warning check pathname validity
  if (no_act)
    fprintf(stderr, _("Would load %d-chars %dx%d font from file `%s'.\n"),
	    the_font->font.charcount,
	    the_font->font.charwidth, the_font->font.charheight,
	    pathname);
  else
    {
      if (verbose)
	fprintf(stderr, _("Loading %d-chars %dx%d font from file `%s'.\n"),
		the_font->font.charcount,
		the_font->font.charwidth, the_font->font.charheight,
		pathname);
      
      if (set_kernel_font (fd, &the_font->font))
	perror ("set_kernel_font"), exit (EX_OSERR);
    }

  /* tell caller about internal SFM */
  memcpy (ud, &the_font->sfm, sizeof(struct unimapdesc));
  
  free (the_font);
  
  /*
   * read a default unimap if we loaded a default font, 
   * and we were not asked otherwise,
   * and none was in font-file.
   */
  if (!no_u && !*ifil && !the_font->sfm.entry_ct)
    {
      FILE* mapf;
      
      if (NULL == (mapf = findsfm(DEFUNIMAP, pathname, sizeof(pathname), NULL, NULL)))
	perror("findsfm"), exit (1);
      
      if (verbose) fprintf (stderr, _("Reading default SFM from `%s'.\n"), pathname);

      sfm_read_ascii (mapf, ud, 512);
      
      fclose (mapf);
    }

  /* unblock SIGCHLD */
  sigprocmask (SIG_SETMASK, &old_sigset, NULL);
}


static int saveoldfont(int fd, char *outfile, int as_psf, int with_unimap)
{
  FILE *fpo;
  struct unimapdesc descr;
  cfontdesc *cfd;

  /* invocation check */
  if (with_unimap && !as_psf)
    {
      fprintf(stderr, _("Cannot write SFM into non-PSF font-file.\n"));
      errno = EINVAL;
      return -1;
    }

  if (NULL == (cfd = get_kernel_font (fd)))
    return -1;
  
  if (cfd->charwidth != 8)
    {
      fprintf (stderr, 
	       _("Can only save 8bit-wide fonts for now,"
	       " and font is %d-bit wide.\n"), cfd->charwidth);
      errno = EBFONT;		       /* FIXME: should be something else ? */
      return -1;
    }
  
  /* RAW files can only have 256 chars */
  if (!as_psf && (cfd->charcount != 256))
    {
      fprintf (stderr, 
	       _("Can only save 256-chars fonts in RAW files,"
		 " and font has %d chars.\n"), cfd->charcount);
      errno = EBFONT;		       /* FIXME: should be something else ? */
      return -1;
    }
  
  if (cfd->charheight == 0)
      fprintf(stderr, _("Found nothing to save.\n"));
  else 
    {
      /* get unimap if requested */
      if (with_unimap)
	  if (-1 == get_kernel_unimap (fd, &descr))
	      with_unimap = 0;	       /* no valid unimap */

      if (!no_act)
	{
	  /* open file */
	  if ((fpo = fopen(outfile, "w")) == NULL) 
	      return -1;
      
	  /* save as PSF file if requested */
	  if (as_psf)
	      write_as_psf_header (fpo, cfd->charheight, cfd->charcount, with_unimap);
      
	  /* write font data, unimap if any */
	  if (fontdata_write_binary (fpo, cfd)
	      || ((with_unimap)
		  && sfm_write_binary(fpo, &descr, cfd->charcount)))
	      return -1;
      
	  /* close output file */
	  fclose(fpo);
	}
      
      if (verbose)
	  fprintf(stderr, 
		  no_act 
		  ? _("Would have saved 8x%d %s font file on `%s'%s.\n")
		  : _("Saved 8x%d %s font file on `%s'%s.\n"), 
		  cfd->charheight,
		  as_psf ? "PSF" : "RAW",
		  outfile,
		  as_psf ? ( with_unimap ? _(", with SFM") : _(", without SFM")) : "");

    }

  free (cfd->chardata);
  
  return 0;
}
