/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

#include <errno.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "rawrec.h"

int main(int argc, char *argv[])
{
  /* Action for ignoring signals we don't want to deal with.  */
  struct sigaction ignorer_act;
    
  /* Command line data goes in it's own structure. */
  parameters_stt command_line;

  /* getopt() related variables */
  int optc;
  /* The --version flag is handled a bit differently, if we see it
     getopt_long stops processing options and sets this flag, which we
     must then check when we exit the getopt loop.  */
  int version_flag = 0;
  struct option long_options[] =
  {
    {"ring-buffer-size", required_argument, NULL, 'B'},
    {"channels", required_argument, NULL, 'c'},
    {"audio-device", required_argument, NULL, 'd'},
    {"end-record-time", required_argument, NULL, 'e'},
    {"end-record-samples", required_argument, NULL, 'E'},
    {"sample-format", required_argument, NULL, 'f'},
    {"fragment-size", required_argument, NULL, 'g'},
    {"hold-audio-device", no_argument, NULL, 'h'},
    {"start-jump-time", required_argument, NULL, 'j'},
    {"start-jump-samples", required_argument, NULL, 'J'},
    {"start-pause-time", required_argument, NULL, 'p'},
    {"start-pause-samples", required_argument, NULL, 'P'},
    {"start-record-time", required_argument, NULL, 'r'},
    {"start-record-samples", required_argument, NULL, 'R'},
    {"sampling-rate", required_argument, NULL, 's'},
    {"time-limit", required_argument, NULL, 't'},
    {"sample-limit", required_argument, NULL, 'T'},
    {"verbose", no_argument, NULL, 'v'},
    /* Note that this option sets a flag and causes getopt to return zero.  */
    {"version", no_argument, &version_flag, 1},
    {"end-pause-time", required_argument, NULL, 'z'},
    {"end-pause-samples", required_argument, NULL, 'Z'},
    {"help", no_argument, NULL, '?'},
    {0, 0, 0, 0}
  };
  
  /* Defaults for options (not everything in command_line has a default).  */
  const long DFLT_RINGBUFSZ = 2097152;        /* Default size two MB. */
  const int DFLT_CHANNELS = 2;                /* default is stereo */
  /* This string would be constant if that could be arranged.  It
     should only be set once, below, and never changed.  */
  char cnst_dflt_audio_dev[NAME_MAX + 1];
  const double DFLT_TIME_ENDREC = 0;
  const long DFLT_SAMP_ENDREC = 0;
  /* This string should be considered constant.  It should only be set
     once, below, and never changed. */
  char cnst_dflt_format[MAX_FORMAT_STRING_LENGTH + 1]; 
  /* Default behavior is to let the kernel pick the kernel buffer
     fragment size. */
  const fragsz_how DFLT_SET_FRAGSZ = AUTO_FRAGSZ;
  const long DFLT_FRAGSZ = 0;                 /* Unused by default.  */
  const boolean DFLT_HOLD_AUDIO_DEVICE = FALSE;
  const double DFLT_TIME_STARTJUMP = 0;
  const long DFLT_SAMP_STARTJUMP = 0;
  const double DFLT_TIME_STARTPAUSE = 0;
  const long DFLT_SAMP_STARTPAUSE = 0;
  const double DFLT_TIME_STARTREC = 0;
  const long DFLT_SAMP_STARTREC = 0;
  const int DFLT_SPEED = 44100;
  /* Default: record until interrupt, or play entire file. */
  const boolean DFLT_TIME_LIMIT_SET = FALSE;
  const boolean DFLT_SAMP_LIMIT_SET = FALSE;
  const double DFLT_TIMELIM = 0;
  const long DFLT_SAMPLIM = 0;  
  const boolean DFLT_VERBOSE = FALSE;
  const double DFLT_TIME_ENDPAUSE = 0;
  const long DFLT_SAMP_ENDPAUSE = 0;

  /* Set global flag indicating root authority or lack thereof.  */
  if ( geteuid() == (uid_t) 0 )
    /* FIXME: Need to make sure no core files are produced, like this:
       #ifdef HAVE_SETRLIMIT
         If we are installed setuid root be careful to not drop core.
    if (original_real_uid != original_effective_uid) {
      struct rlimit rlim;
      rlim.rlim_cur = rlim.rlim_max = 0;
      if (setrlimit(RLIMIT_CORE, &rlim) < 0)
	fatal("setrlimit failed: %.100s", strerror(errno));
    }
       #endif
    This needs HAVE_SETRLIMIT from config.h from autoconf though.  */
    have_root_authority = 1;
  else
    have_root_authority = 0;

  /* Drop root permissions until we need them. */
  if ( have_root_authority )
    if ( seteuid(getuid()) == -1 )
      /* If for some crazy reason we fail to drop root permissions, exit
	 immediately without advertising the fact. */
      exit(EXIT_FAILURE);

  /* These are defaults for options which take string arguments, and
     this is the only place they should ever be modified. */
  strncpy(cnst_dflt_audio_dev, "/dev/dsp", (size_t) (NAME_MAX + 1));
  strncpy(cnst_dflt_format, "s16_le", (size_t) (MAX_FORMAT_STRING_LENGTH + 1));

  /* at the moment, this application goes with the default for most signals */
  ignorer_act.sa_handler = SIG_IGN;
  /* because I'm unclear on how SIGIO is supposed to work, it's not 
     applicable here, and I'm paranoid */
  if ( sigaction(SIGIO, &ignorer_act, NULL) == -1 )
    err_die("sigaction on SIGIO failed: %s\n", strerror(errno));
  /* for now (and probably forever), we will handle broken pipes by
     checking errno, because it's a much simpler way to go in a
     multithreading program */
  if ( sigaction(SIGPIPE, &ignorer_act, NULL) == -1 )
    err_die("sigaction on SIGPIPE failed: %s\n", strerror(errno));
  /* we can't do interactive job control, yet (need full POSIX
     conformance from pthreads with respect to signal handling) */
  if ( sigaction(SIGTSTP, &ignorer_act, NULL) == -1 ) 
    err_die("sigaction on SIGTSTP failed: %s\n", strerror(errno));

  /* Options are initialized with their default values. */
  command_line.ringbufsz = DFLT_RINGBUFSZ;    	 
  command_line.channels = DFLT_CHANNELS;	 
  strncpy(command_line.audio_dev, cnst_dflt_audio_dev, 
               (size_t) (NAME_MAX + 1));
  command_line.time_endrec = DFLT_TIME_ENDREC;
  command_line.samp_endrec = DFLT_SAMP_ENDREC;
  strncpy(command_line.format, cnst_dflt_format, 
               (size_t) (MAX_FORMAT_STRING_LENGTH + 1));
  command_line.set_fragsz = DFLT_SET_FRAGSZ;     
  command_line.fragsz = DFLT_FRAGSZ;	         
  command_line.hold_audio_device = DFLT_HOLD_AUDIO_DEVICE;
  command_line.time_startjump = DFLT_TIME_STARTJUMP;	
  command_line.samp_startjump = DFLT_SAMP_STARTJUMP;	
  command_line.time_startpause = DFLT_TIME_STARTPAUSE;
  command_line.samp_startpause = DFLT_SAMP_STARTPAUSE;
  command_line.time_startrec = DFLT_TIME_STARTREC;
  command_line.samp_startrec = DFLT_SAMP_STARTREC;
  command_line.speed = DFLT_SPEED;        
  command_line.time_limit_set = DFLT_TIME_LIMIT_SET;
  command_line.samp_limit_set = DFLT_SAMP_LIMIT_SET;
  command_line.timelim = DFLT_TIMELIM;
  command_line.samplim = DFLT_SAMPLIM;	
  command_line.verbose = DFLT_VERBOSE;
  command_line.time_endpause = DFLT_TIME_ENDPAUSE;
  command_line.samp_endpause = DFLT_SAMP_ENDPAUSE;

  /* Allocate new storage for progname and copy argv[0] into it.  */
  const size_t max_argv_0_length = 10000;
  size_t argv_0_length = strnlen (argv[0], max_argv_0_length + 1);
  if ( argv_0_length > max_argv_0_length ) {
    err_die ("argv[0] string too long\n");
  }
  progname = calloc (argv_0_length + 1, sizeof (char));
  if ( progname == NULL ) {
    err_die ("calloc failed trying to allocate %llu bytes\n",
	     (unsigned long long int) argv_0_length + 1);
  }
  strncpy (progname, argv[0], argv_0_length + 1);

  /* POSIX basename function is so hard to use correctly its a bit of
     a joke really.  It might modify its argument, so we have to copy
     that, and it might return a point to staticly allocated memory,
     so we have to copy the return as well...  */
  char *argv_0_copy = calloc (argv_0_length + 1, sizeof (char));
  if ( argv_0_copy == NULL ) {
    err_die ("calloc failed trying to allocate %llu bytes\n",
	     (unsigned long long int) argv_0_length + 1);
  }
  strncpy (argv_0_copy, argv[0], argv_0_length + 1);
  char *program_basename = calloc (argv_0_length + 1, sizeof (char));
  if ( program_basename == NULL ) {
    err_die ("calloc failed trying to allocate %llu bytes\n",
	     (unsigned long long int) argv_0_length + 1);
  }
  strncpy (program_basename, basename (argv[0]), argv_0_length + 1);
  free (argv_0_copy);

  /* Find out whether this execution should record or play. */
  if ( strcmp(program_basename, "rawplay") == 0 )
    command_line.recorder = FALSE;
  else if ( (strcmp(program_basename, "rawrec") == 0) )
    command_line.recorder = TRUE;
  else {       		/* Barring executable renamed, shouldn't happen.  */
    usage();
    exit(EXIT_FAILURE);
  }

  free (program_basename);

  /* Parse the options.  They are listed in alphabetic order, as they
     appear in info and the manual entry.  Options may be repeated:
     options without arguments act as toggles (except for -?), options
     with arguments overwrite the arguments of previous occurences.
     If an option with an argument is given the special argument
     'dflt' on the command line, the default behavior is restored (it
     is exactly as if that option never appeared), at least until the
     next occurence of that option.  Potentially confusing yes, but
     allows for full alias customization without sacrificing
     flexibility.  */
  while ( (optc = getopt_long(argc, argv, 
               "b:c:B:d:e:E:f:g:hj:J:p:P:r:R:s:t:T:vz:Z:?", 
	       long_options, NULL)) != EOF ) {
    switch(optc) {
    case 'B': 
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.ringbufsz = atol(optarg);
      else
	command_line.ringbufsz = DFLT_RINGBUFSZ;
      break;
    case 'c':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.channels = atoi(optarg);
      else
	command_line.channels = DFLT_CHANNELS;
      break;
    case 'd':
      if ( strcmp(optarg, "dflt") != 0 )
	strncpy(command_line.audio_dev, optarg, (size_t) (NAME_MAX + 1));
      else
	strncpy(command_line.audio_dev, cnst_dflt_audio_dev, 
                (size_t) (NAME_MAX + 1));
      break;
    case 'e':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.time_endrec = atof(optarg);
      else
	command_line.time_endrec = DFLT_TIME_ENDREC;
      break;
    case 'E':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.samp_endrec = atol(optarg);
      else
	command_line.samp_endrec = DFLT_SAMP_ENDREC;
      break;
    case 'f':
      /* this syntax check should perhaps be moved into process_command_line */
      if ( (strcmp(optarg, "s16_le") == 0)
               || (strcmp(optarg, "u16_le") == 0)
	       || (strcmp(optarg, "s16_be") == 0)
	       || (strcmp(optarg, "u16_be") == 0)
	       || (strcmp(optarg, "s8") == 0)
	       || (strcmp(optarg, "u8") == 0) ) 
	strncpy(command_line.format, optarg, 
		(size_t) (MAX_FORMAT_STRING_LENGTH + 1));
      else
	err_die("%s: invalid argument to -f option\n", optarg);
      break;
    case 'g':
      /* Use user specified kernel audio buffer fragment size. */
      if ( strcmp(optarg, "dflt") != 0 ) {
	command_line.set_fragsz = USER_FRAGSZ;   
	command_line.fragsz = atol(optarg);
      } else {			/* optarg == 0 */
	command_line.set_fragsz = DFLT_SET_FRAGSZ;
	command_line.fragsz = 0;
      }
      break;
    case 'h':
      /* Toggle holding of audio device. */
      if ( command_line.hold_audio_device == FALSE )
	command_line.hold_audio_device = TRUE;
      else
	command_line.hold_audio_device = FALSE;
      break;
    case 'j':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.time_startjump = atof(optarg);
      else
	command_line.time_startjump = DFLT_TIME_STARTJUMP;
      break;
    case 'J':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.samp_startjump = atol(optarg);
      else
	command_line.samp_startjump = DFLT_SAMP_STARTJUMP;
      break;
    case 'p':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.time_startpause = atof(optarg);
      else
	command_line.time_startpause = DFLT_TIME_STARTPAUSE;
      break;
    case 'P':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.samp_startpause = atol(optarg);
      else
	command_line.samp_startpause = DFLT_SAMP_STARTPAUSE;
      break;
    case 'r':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.time_startrec = atof(optarg);
      else
	command_line.time_startrec = DFLT_TIME_STARTREC;
      break;
    case 'R':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.samp_startrec = atol(optarg);
      else
	command_line.samp_startrec = DFLT_SAMP_STARTREC;
      break;
    case 's':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.speed = atoi(optarg);
      else 
	command_line.speed = DFLT_SPEED;
      break;
    case 't':
      if ( strcmp(optarg, "dflt") != 0 ) {
	command_line.time_limit_set = TRUE;
	command_line.timelim = atof(optarg);
      } else {
	command_line.time_limit_set = DFLT_TIME_LIMIT_SET;
	command_line.timelim = DFLT_TIMELIM;
      }
      break;
    case 'T':
      if ( strcmp(optarg, "dflt") != 0 ) {
	command_line.samp_limit_set = TRUE;
	command_line.samplim = atol(optarg);
      } else {
	command_line.samp_limit_set = DFLT_SAMP_LIMIT_SET;
	command_line.samplim = DFLT_SAMPLIM;
      }
      break;
    case 'v':
      /* Toggle verbose. */
      if ( command_line.verbose == FALSE )
	command_line.verbose = TRUE;
      else 
	command_line.verbose = FALSE;
      break;
    case 'z':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.time_endpause = atof(optarg);
      else
	command_line.time_endpause = DFLT_TIME_ENDPAUSE;
      break;
    case 'Z':
      if ( strcmp(optarg, "dflt") != 0 )
	command_line.samp_endpause = atol(optarg);
      else 
	command_line.samp_endpause = DFLT_SAMP_ENDPAUSE;
      break;
    case '?':
      fprintf (stderr, "Try `%s --help' for more information.\n", progname);
      exit(EXIT_SUCCESS);	/* usage() called deliberately */
      break;
    case 0:
      /* In this case, we have had getopt_long use the alternate form
         of long option processing and a flag has been set.  We only
         do this for flags which cause option processing to stop.  */
      break;
    default:
      fprintf (stderr, "Try `%s --help' for more information.\n", progname);
      exit(EXIT_FAILURE);	/* usage() called due to user error */
    }
  }

  /* If getopt long saw the --version flag, version_flag got set.  */
  if ( version_flag ) {
    printf(
"rawrec (invoked as %s) version 0.9.99\n"
"\n"
"Copyright (C) 2006 Britton Kerin (fsblk@uaf.edu)\n"
"This is free software; see the source for copying conditions.  There is NO\n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
            , progname);
    exit(EXIT_SUCCESS);
  }

  /* There is only one argument, which is optional: the file to play
     from or record to. */
  if ( argv[optind] == NULL ) {
    command_line.using_stdio = TRUE;
    strncpy(command_line.arg_file, "", (size_t) (NAME_MAX + 1));
  } else { 
    command_line.using_stdio = FALSE;
    strncpy(command_line.arg_file, argv[optind], (size_t) (NAME_MAX + 1));
  }

  /* Spawn a signal-handling thread.  This is a peculiar must-do way
     of doing things in a multithreaded environment.  Other threads,
     including the main thread (in which this comment is embedded),
     will mask out signals of interest, and this one will deal with
     all of them. */
  /* pthread_create(&sig_th, NULL, sig_catcher, NULL); */

  /* These guys do the remaining parameter interpretation and spawn
     the threads that do the actual work. */
  if ( command_line.recorder ) 
    record(&command_line);
  else
    play(&command_line);

  /* Cancel the signal handling thread, which should always be
     cancelable.  sig_th is not seen without including
     thread_functions.h, so something needs to change here. */
  /* pthread_cancel(&sig_th); */

  exit(EXIT_SUCCESS);
}
