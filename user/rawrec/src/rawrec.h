/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* Header file.  At present this includes all header material except
   some which is concerned mainly with the threads and thread shared
   data.  */

#include <limits.h>
#include <sys/types.h>

/* GNU C allows us to use really long integers.  */
#ifdef __GNUC__
# define INT64_MAX 9223372036854775807LL
#endif

/* The maximum length for the strings which specify the sample format. */
#define MAX_FORMAT_STRING_LENGTH 10

typedef enum { FALSE, TRUE } boolean;	

/* How to open a file. */
typedef enum { FOR_READING, FOR_WRITING } open_how;

/* Is the kernel setting the audio fragment size (AUTO_FRAGSZ) or is
   the user specifying it (USER_FRAGSZ)? */
typedef enum { AUTO_FRAGSZ, USER_FRAGSZ } fragsz_how;

/* Return codes for test_pcm_params function.  SUCCESS is good, of
   course.  INVALID returns are errors, something will have to be
   changed.  INEXACT_ARG simply means that the exact setting for one
   or more of the arguments was not available, programmer discretion
   advised. */
typedef enum { 
  SUCCESS = 0,	            /* all parameters set exactly as desired */
  INVALID_FORMAT = -1,	    /* the sample format requested could not be
                               set by the driver */
  INVALID_CHANNELS = -2,    /* the number of channels requested could
                               not be set by the driver */
  INVALID_RATE = -3,        /* the sampling rate requested could not
                               be set by the driver */
  INEXACT_ARG  = 1	    /* one or more of the speed, format or channels 
			       parameters had to be modified slightly */
} test_dsp_params_rtn;

/* Command line parameters.  This type must be directly copyable,
   i.e. it can't contain pointers to dynamicly allocated memory.  */
typedef struct {
  boolean recorder;		/* From name of this invocation. */
  boolean using_stdio;		/* Flag is true if using stdio instead
                                   of arg_file. */
  char arg_file[NAME_MAX + 1];	/* The file to play from or record to. */
  /* User definable size for the big buffer, in bytes.  Actual size
     may be slightly greater, as it is rounded up to the next multiple
     of the audio buffer (or audio fragment) size. */   
  long ringbufsz;		
  int channels;                 /* ==1 => 2 channels, ==0 => 1 channel. */
  char audio_dev[NAME_MAX + 1];	/* Typically /dev/audio. */
  double time_endrec;	        /* Silent time to put at end. */
  long samp_endrec;		/* Silent samples to put at end. */
  /* The format to use for audio samples (ex. u8 (unsigned 8 bit),
     s16_le (signed 16 bit little endian). */
  char format[MAX_FORMAT_STRING_LENGTH + 1];
  /* == AUTO_FRAGSZ ==> let kernel set kernel audio buffer fragment
     size, == USER_FRAGSZ ==> user specified kernel audio buffer
     fragment size. */
  fragsz_how set_fragsz;	       
  long fragsz;			/* Fragment size of kernel audio buffer. */
  boolean hold_audio_device;    /* Hold the real audio device, even
                                   though you are using a targetfile
                                   and therefore don't really need it. */
  double time_startjump;	/* Starting skip time in seconds. */
  long samp_startjump;		/* Samples to skip at start. */
  double time_startpause;	/* Seconds to pause at start. */
  long samp_startpause;		/* Samples to pause at start. */
  double time_startrec;	        /* Silent time to put at start. */
  long samp_startrec;		/* Silent samples to put at start. */
  int speed;                    /* Sampling rate. */
  boolean time_limit_set;	/* Is a time limit set? */
  double timelim;		/* Recording time in seconds. */
  boolean samp_limit_set;       /* Is a sample limit set? */
#ifdef __GNUC__
  int64_t samplim;
#else
  /* Using long for samplim means max record time 27h, 3m, 11.548s on
     intel. */
  long samplim;		        /* Recording time in samples. */
#endif /* __GNUC__ */
  boolean verbose;		/* verbose errors and warnings */
  double time_endpause;		/* Seconds to pause at end. */
  long samp_endpause;		/* Samples to pause at end. */
} parameters_stt;

/* Everyone needs this to report errors. */
extern char *progname;

/* Global flag indicating whether or not we were started suid.  */
extern int have_root_authority;

/* Function prototypes.  See the appropriate source file for
   information on these.  The prototypes for the thread functions
   themselves are in 'thread_functions.h'. */

void usage(void);
void err_die(const char *error_message, ...)/* ; is coming, don't worry.  */
/* The GNU C compiler can give us some special help if compiling with
   -Wformat or a warning option that implies it.  */
#ifdef __GNUC__
     /* Function attribute format says function is variadic a la
        printf, with argument 1 its format spec and argument 2 its
        first optional argument corresponding to the spec.  Function
        attribute noreturn says function doesn't return.  */
     __attribute__ ((format (printf, 1, 2), noreturn))
#endif 
; /* <-- Semicolon for err_die prototype.  */
parameters_stt process_command_line(parameters_stt *clp);
test_dsp_params_rtn test_dsp_params(char *audio_dev, open_how how, 
				    int *speedp, char *format, 
				    int *channelsp);
boolean is_pow_two(int x);
void record(parameters_stt *clp);
void play(parameters_stt *clp);
int data_init(const char *data_file, open_how how);
void sleep_on_option(double time, double samples, int speed);
void write_silence_on_option(int fd, boolean using_stdio, 
			     const char *file_name, double time_rec, 
			     long samp_rec, int speed, const char *format, 
			     int bps, int channels);
int audio_init(const char *audio_dev, open_how how, int speed, 
	       const char *format, int channels);
void set_au_blksz(int audio_fd, int fragsz_arg);
int get_au_blksz(int audio_fd);
int get_format_code(const char *format);
long ringbuf_init(long ringbufsz, int segsz);
double min(double x1, double x2);
void audio_close(int audio_fd, const char *audio_dev);
void ringbuf_close(void);
void data_close(int data_fd, const char *data_file);
