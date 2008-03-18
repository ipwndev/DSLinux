#include "esd-server.h"
#include "esd-config.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#ifndef HAVE_NANOSLEEP
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <netdb.h>

/* Older resolvers don't have gethostbyname2() */
#ifndef HAVE_GETHOSTBYNAME2
#define gethostbyname2(host, family) gethostbyname((host))
#endif /* HAVE_GETHOSTBYNAME2 */

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
                     + strlen ((ptr)->sun_path))
#endif

/* AIX defines this */
#ifndef h_errno
extern int h_errno;
#endif

/* max arguments (argc + tokenized esd.conf) can't be more than this */
#define MAX_OPTS 128

typedef const char *(*audio_devices_t)(void);
typedef int (*audio_open_t)(void);
typedef void (*audio_close_t)(void);
typedef void (*audio_pause_t)(void);
typedef void (*audio_flush_t)(void);
typedef int (*audio_write_t)( void *buffer, int buf_size );
typedef int (*audio_read_t)( void *buffer, int buf_size );

audio_devices_t impl_esd_audio_devices = esd_audio_devices;
audio_open_t impl_esd_audio_open = esd_audio_open;
audio_close_t impl_esd_audio_close = esd_audio_close;
audio_pause_t impl_esd_audio_pause = esd_audio_pause;
audio_flush_t impl_esd_audio_flush = esd_audio_flush;
audio_write_t impl_esd_audio_write = esd_audio_write;
audio_read_t impl_esd_audio_read = esd_audio_read;

#define esd_audio_devices impl_esd_audio_devices
#define esd_audio_open impl_esd_audio_open
#define esd_audio_close impl_esd_audio_close
#define esd_audio_pause impl_esd_audio_pause
#define esd_audio_flush impl_esd_audio_flush
#define esd_audio_write impl_esd_audio_write
#define esd_audio_read impl_esd_audio_read

#ifdef DRIVER_ARTS
#include "audio_arts.c"
#endif

/*******************************************************************/
/* esd.c - prototypes */
void set_audio_buffer( void *buf, esd_format_t format, int magl, int magr,
		int freq, int speed, int length, long offset );
void clean_exit( int signum );
void reset_signal( int signum );
void reset_daemon( int signum );
int open_listen_socket( const char *hostname, int port );

/* from esd_config.c */
void esd_config_read(void);

/*******************************************************************/
/* globals */

int esd_is_owned = 0;		 /* start without owner, first client claims it */
int esd_is_locked = 1;		 /* if owned, will prohibit foreign sources */
char esd_owner_key[ESD_KEY_LEN]; /* the key that locks the daemon */

int esd_on_standby = 0;		/* set to ignore incoming audio data */
int esdbg_trace = 0;		/* show warm fuzzy debug messages */
int esdbg_comms = 0;		/* show protocol level debug messages */
int esdbg_mixer = 0;		/* show mixer engine debug messages */

int esd_buf_size_octets = 0; 	/* size of audio buffer in bytes */
int esd_buf_size_samples = 0; 	/* size of audio buffer in samples */
int esd_sample_size = 0;	/* size of sample in bytes */

int esd_beeps = 1;		/* whether or not to beep on startup */
int listen_socket = -1;		/* socket to accept connections on */
int esd_trustval = -1;		/* be paranoid, don't trust the owner of ESD_UNIX_SOCKET_DIR */

int esd_autostandby_secs = -1; 	/* timeout before releasing the audio device, disabled <0 */
time_t esd_last_activity = 0;	/* seconds since last activity */
int esd_on_autostandby = 0;	/* set when auto paused for auto reawaken */

int esd_use_tcpip = 0;          /* use tcp/ip sockets instead of unix domain */
int esd_terminate = 0;          /* terminate after the last client exits */
int esd_public = 0;             /* allow connects from hosts other than localhost */
int esd_spawnpid = 0;           /* The PID of the process that spawned us (for use by esdlib only) */
int esd_spawnfd = 0;            /* The FD of the process that spawned us (for use by esdlib only) */

#if defined ENABLE_IPV6        
int esd_use_ipv6 = 0;          /* We need it in accept () to know if we use 
                                       AF_NET or AF_INET6*/   
#endif
static char *programname = NULL;

/*******************************************************************/
/* create the startup tones for the fun of it */
void set_audio_buffer( void *buf, esd_format_t format,
		       int magl, int magr,
		       int freq, int speed, int length, long offset )
{
    int i;
    float sample;
    float kf = 2.0 * 3.14 * (float)freq / (float)speed;

    unsigned char *uc_buf = (unsigned char *)buf;
    signed short *ss_buf = (signed short *)buf;

    /* printf( "fmt=%d, ml=%d, mr=%d, freq=%d, speed=%d, len=%ld\n",
       format, magl, magr, freq, speed, length ); */

    switch ( format & ESD_MASK_BITS )
    {
    case ESD_BITS8:
	for ( i = 0 ; i < length ; i+=2 ) {
	    sample = sin( (float)(i+offset) * kf );
	    uc_buf[i] = 127 + magl * sample;
	    uc_buf[i+1] = 127 + magr * sample;
	}
	break;
    case ESD_BITS16:	/* assume same endianness */
	for ( i = 0 ; i < length ; i+=2 ) {
	    sample = sin( (float)(i+offset) * kf );
	    ss_buf[i] = magl * sample;
	    ss_buf[i+1] = magr * sample;
	}
	break;
    default:
	fprintf( stderr,
		 "unsupported format for set_audio_buffer: 0x%08x\n",
		 format );
	exit( 1 );
    }


    return;
}

/*******************************************************************/
/* handle signals properly */

void reset_daemon( int signum )
{
    int tumbler;

    ESDBG_TRACE(
	printf( "(ca) resetting sound daemon on signal %d\n",
		signum ); );

    /* reset the access rights */
    esd_is_owned = 0;
    esd_is_locked = 1;

    /* scramble the stored key */
    srand( time(NULL) );
    for ( tumbler = 0 ; tumbler < ESD_KEY_LEN ; tumbler++ ) {
	esd_owner_key[ tumbler ] = rand() % 256;
    }

    /* close the clients */
    while ( esd_clients_list != NULL )
    {
	erase_client( esd_clients_list );
    }

    /* free samples */
    while ( esd_samples_list != NULL )
    {
	esd_sample_kill( esd_samples_list->sample_id, 1 );
	erase_sample( esd_samples_list->sample_id, 1 );
    }

    /* reset next sample id */
    esd_next_sample_id = 1;

    /* reset signal handler, if not called from a signal, no effect */
    signal( SIGHUP, reset_daemon );
}

void clean_exit(int signum) {
    /* just clean up as best we can and terminate from here */
    esd_client_t * client = esd_clients_list;

/*    fprintf( stderr, "received signal %d: terminating...\n", signum );*/

    /* free the sound device */
    esd_audio_close();

    /* close the listening socket */
    close( listen_socket );

    /* close the clients */
    while ( client != NULL )
    {
	close( client->fd );
	client = client->next;
    }
   if (!esd_use_tcpip)
    {
      unlink(ESD_UNIX_SOCKET_NAME);
      rmdir(ESD_UNIX_SOCKET_DIR);
    }


    /* trust the OS to clean up the memory for the samples and such */
    exit( 0 );
}

void reset_signal(int signum) {
/*    fprintf( stderr, "received signal %d: resetting...\n", signum );*/
    signal( signum, reset_signal);

    return;
}

static int
esd_connect_unix(void)
{
  struct sockaddr_un socket_unix;
  int socket_out = -1;
  int curstate = 1;

  /* create the socket, and set for non-blocking */
  socket_out = socket( AF_UNIX, SOCK_STREAM, 0 );
  if ( socket_out < 0 )
    return -1;
  /* this was borrowed blindly from the Tcl socket stuff */
  if ( fcntl( socket_out, F_SETFD, FD_CLOEXEC ) < 0 ) {
    close (socket_out);
    return -1;
  }
  if ( setsockopt( socket_out, SOL_SOCKET, SO_REUSEADDR,
		  &curstate, sizeof(curstate) ) < 0 ) {
    close (socket_out);
    return -1;
  }

  /* set the connect information */
  socket_unix.sun_family = AF_UNIX;
  strncpy(socket_unix.sun_path, ESD_UNIX_SOCKET_NAME, sizeof(socket_unix.sun_path));
  if ( connect( socket_out,
	       (struct sockaddr *) &socket_unix, SUN_LEN(&socket_unix) ) < 0 )
    return -1;
  return socket_out;
}


/*******************************************************************/
/* safely create directory for socket
   Code inspired by trans_mkdir from XFree86 source code
   For more credits see xc/lib/xtrans/Xtransutil.c. */
int
safe_mksocketdir(void)
{
struct stat dir_stats;

#if defined(S_ISVTX)
#define ESD_UNIX_SOCKET_DIR_MODE (S_IRUSR|S_IWUSR|S_IXUSR|\
				  S_IRGRP|S_IWGRP|S_IXGRP|\
				  S_IROTH|S_IWOTH|S_IXOTH|S_ISVTX)
#else
#define ESD_UNIX_SOCKET_DIR_MODE (S_IRUSR|S_IWUSR|S_IXUSR|\
				  S_IRGRP|S_IWGRP|S_IXGRP|\
				  S_IROTH|S_IWOTH|S_IXOTH)
#endif

  if (mkdir(ESD_UNIX_SOCKET_DIR, ESD_UNIX_SOCKET_DIR_MODE) == 0) {
    if (chmod(ESD_UNIX_SOCKET_DIR, ESD_UNIX_SOCKET_DIR_MODE) != 0) {
      return -1;
    }
    return 0;
  }
  /* If mkdir failed with EEXIST, test if it is a directory with
     the right modes, else fail */
  if (errno == EEXIST) {
#if !defined(S_IFLNK) && !defined(S_ISLNK)
#define lstat(a,b) stat(a,b)
#endif
    if (lstat(ESD_UNIX_SOCKET_DIR, &dir_stats) != 0) {
      return -1;
    }
    if (S_ISDIR(dir_stats.st_mode)) {
      int updateOwner = 0;
      int updateMode = 0;
      int updatedOwner = 0;
      int updatedMode = 0;
      /* Check if the directory's ownership is OK. */
      if ((dir_stats.st_uid != 0) && (dir_stats.st_uid != getuid()))
	updateOwner = 1;
      /*
       * Check if the directory's mode is OK.  An exact match isn't
       * required, just a mode that isn't more permissive than the
       * one requested.
       */
      if ( ~ESD_UNIX_SOCKET_DIR_MODE & (dir_stats.st_mode & ~S_IFMT))
	updateMode = 1;
#if defined(S_ISVTX)
      if ((dir_stats.st_mode & S_IWOTH) && !(dir_stats.st_mode & S_ISVTX))
	updateMode = 1;
#endif
#if defined(HAVE_FCHOWN) && defined(HAVE_FCHMOD)
      /*
       * If fchown(2) and fchmod(2) are available, try to correct the
       * directory's owner and mode.  Otherwise it isn't safe to attempt
       * to do this.
       */
      if (updateMode || updateOwner) {
	int fd = -1;
	struct stat fdir_stats;
	if ((fd = open(ESD_UNIX_SOCKET_DIR, O_RDONLY)) != -1) {
	  if (fstat(fd, &fdir_stats) == -1) {
	    return esd_trustval;
	  }
	  /*
	   * Verify that we've opened the same directory as was
	   * checked above.
	   */
	  if (!S_ISDIR(fdir_stats.st_mode) ||
	      dir_stats.st_dev != fdir_stats.st_dev ||
	      dir_stats.st_ino != fdir_stats.st_ino) {
	    return esd_trustval;
	  }
	  if (updateOwner && fchown(fd, getuid(), getgid()) == 0)
	    updatedOwner = 1;
	  if (updateMode && fchmod(fd, ESD_UNIX_SOCKET_DIR_MODE) == 0)
	    updatedMode = 1;
	  close(fd);
	}
      }
#endif
      if (updateOwner && !updatedOwner) {
	fprintf(stderr,
		"esd: Failed to fix owner of %s.\n",
	      ESD_UNIX_SOCKET_DIR);
	if (esd_trustval) fprintf(stderr, "Try -trust to force esd to start.\n");
	return esd_trustval;
      }
      if (updateMode && !updatedMode) {
	fprintf(stderr, "esd: Failed to fix mode of %s to %04o.\n",
	      ESD_UNIX_SOCKET_DIR, ESD_UNIX_SOCKET_DIR_MODE);
	if (esd_trustval) fprintf(stderr, "Try -trust to force esd to start.\n");
	return esd_trustval;
      }
      return 0;
    }
  }
  /* In all other cases, fail */
  return -1;
}

/*******************************************************************/
/* returns the listening socket descriptor */
int open_listen_socket(const char *hostname, int port )
{
    /*********************/
    /* socket test setup */
#if defined (ENABLE_IPV6)
    struct addrinfo hints, *res, *result = NULL;
#endif
    struct sockaddr_in socket_addr;
    struct sockaddr_un socket_unix;
    int socket_listen = -1;
    struct linger lin;
    size_t socket_len = 0;
    struct sockaddr *saddr = NULL;

    struct hostent *resolved;


    /* create the socket, and set for non-blocking */
    if (esd_use_tcpip) {
#if defined(ENABLE_IPV6)
      if(have_ipv6()) {
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
   
	/* If host name is set then bind to its first address */
	if (hostname) {
		if (getaddrinfo(hostname, NULL, &hints, &result) != 0) {
			fprintf (stderr,"Unable to resolve the host\n");
			return (-1);
		}

		for (res = result; res; res = res->ai_next) 
			if(res->ai_family != AF_INET || res->ai_family != AF_INET6)
				break;

		if(res->ai_family == AF_INET) {
			((struct sockaddr_in *)res->ai_addr)->sin_port = htons(port);	
			esd_use_ipv6 = 0;
		}

		if(res->ai_family == AF_INET6) {
			((struct sockaddr_in6 *)res->ai_addr)->sin6_port = htons(port);
			esd_use_ipv6 = 1;	
		}

		socket_listen = socket(res->ai_family, SOCK_STREAM, 0);
		saddr = res->ai_addr;
		socket_len = res->ai_addrlen;
	}
	else {
		struct sockaddr_in6 socket6_addr;

		memset(&socket6_addr, 0, sizeof(struct sockaddr_in6));
		socket6_addr.sin6_family = AF_INET6;
		socket6_addr.sin6_port = htons(port);

		if (esd_public)
			socket6_addr.sin6_addr = in6addr_any;
		else
			socket6_addr.sin6_addr = in6addr_loopback;

		socket_listen = socket(AF_INET6, SOCK_STREAM, 0);
		saddr = (struct sockaddr *)&socket6_addr;
		socket_len = sizeof(socket6_addr);
		esd_use_ipv6 = 1;
	}
      }
      else
#endif
      {
	  /* set the listening information */
	memset(&socket_addr, 0, sizeof(struct sockaddr_in));
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons( port );

	/* if hostname is set, bind to its first address */
	if (hostname)
        {
		if (!(resolved=gethostbyname2(hostname, AF_INET)))
                {
			return -1;
                }
                memcpy(&(socket_addr.sin_addr), resolved->h_addr_list[0], resolved->h_length);
	} else if (esd_public)
		socket_addr.sin_addr.s_addr = htonl( INADDR_ANY );
	else
		socket_addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );

	socket_listen = socket(AF_INET, SOCK_STREAM, 0);
	saddr = (struct sockaddr *)&socket_addr;
	socket_len = sizeof(socket_addr);
      }
    }
    else
    {
      if (safe_mksocketdir())
	{
	  fprintf(stderr,
		  "esd: Esound sound daemon unable to create unix domain socket:\n"
		  "%s\n"
		  "The socket is not accessible by esd.\n"
		  "Exiting...\n", ESD_UNIX_SOCKET_NAME);
	  exit(1);
	}
      else
	{
	  if (esd_connect_unix() >= 0)
	    {
	      /* not allowed access */
	      fprintf(stderr,
		      "esd: Esound sound daemon already running or stale UNIX socket\n"
		      "%s\n"
		      "This socket already exists indicating esd is already running.\n"
		      "Exiting...\n", ESD_UNIX_SOCKET_NAME);
	      exit(1);
	    }
	}
      unlink(ESD_UNIX_SOCKET_NAME);
      socket_listen=socket(AF_UNIX, SOCK_STREAM, 0);
    }
    if (socket_listen < 0)
    {
	fprintf(stderr,"Unable to create socket\n");
	return( -1 );
    }
    if (fcntl(socket_listen, F_SETFL, O_NONBLOCK) < 0)
    {
	fprintf(stderr,"Unable to set socket to non-blocking\n");
	return( -1 );
    }

    /* set socket for linger? */
    lin.l_onoff=1;	/* block a closing socket for 1 second */
    lin.l_linger=100;	/* if data is waiting to be sent */
    if ( setsockopt( socket_listen, SOL_SOCKET, SO_LINGER,
		     &lin, sizeof(struct linger) ) < 0 )
    {
	fprintf(stderr,"Unable to set socket linger value to %d\n",
		lin.l_linger);
	return( -1 );
    }
    {
      int n = 1;
      setsockopt(socket_listen, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));
      /* if it fails, so what */
    }

    if ( esd_use_tcpip ) {
      if ( bind( socket_listen, saddr, socket_len ) != 0 )
	{
	  fprintf(stderr,"Unable to bind port %d\n", port );
	  exit(1);
	}
#if defined (ENABLE_IPV6)
      if ( result )
	  freeaddrinfo ( result );
#endif
    }
    else
    {
      mode_t old_umask;

      old_umask = umask(0);
      socket_unix.sun_family=AF_UNIX;
      strncpy(socket_unix.sun_path, ESD_UNIX_SOCKET_NAME, sizeof(socket_unix.sun_path));
      if ( bind( socket_listen,
		(struct sockaddr *) &socket_unix, SUN_LEN(&socket_unix) ) < 0 )
	{
	  fprintf(stderr,"Unable to connect to UNIX socket %s\n", ESD_UNIX_SOCKET_NAME);
	  if (!esd_use_tcpip)
	    {
	      unlink(ESD_UNIX_SOCKET_NAME);
	      rmdir(ESD_UNIX_SOCKET_DIR);
	    }
	  exit(1);
	}
      umask(old_umask);
    }
    if (listen(socket_listen,16)<0)
    {
      fprintf(stderr,"Unable to set socket listen buffer length\n");
      if (!esd_use_tcpip)
	{
	  unlink(ESD_UNIX_SOCKET_NAME);
	  rmdir(ESD_UNIX_SOCKET_DIR);
	}
      exit(1);
    }

    return socket_listen;
}

/*******************************************************************/
/* daemon eats sound data, without playing anything, return boolean ok */
int esd_server_standby(void)
{
    int ok = 1;

    /* only bother if we're not already on standby */
    if ( !esd_on_standby ) {
	ESDBG_TRACE( printf( "setting sound daemon to standby\n" ); );
	
	/* TODO: close down any recorders, too */
	esd_on_standby = 1;
	esd_audio_close();
    }	

    return ok;
}

/*******************************************************************/
/* daemon goes back to playing sound data, returns boolean ok */
int esd_server_resume(void)
{
    int ok = 1;

    /* only bother if we're already on standby */
    if ( esd_on_standby ) {
	
	ESDBG_TRACE( printf( "resuming sound daemon\n" ); );
	
	/* reclaim the audio device */
	if ( esd_audio_open() < 0 ) {
	    /* device was busy or something, return error, try again later */
	    ok = 0;
	} else {
	    /* turn ourselves back on */
	    esd_on_standby = 0;
	    esd_on_autostandby = 0;
	    esd_forced_standby = 0;
	}
    }

    return ok;
}

/*******************************************************************/
int main ( int argc, char *argv[] )
{
    /***************************/
    /* Enlightened sound Daemon */

    int esd_port = ESD_DEFAULT_PORT;
    int length = 0;
    int arg = 0;
    int itmp;

    void *output_buffer = NULL;

    char *hostname=NULL;
    char *endptr;

    int fd0;
    int max_fds;

    /* from esd_config.c */
    extern char esd_spawn_options[];
    extern char esd_default_options[];
    extern int  esd_no_spawn;
    int esd_spawning;
    
    /* for merging argv and esd.conf */
    char tmp_str[2*LINEBUF_SIZE] = "";
    int num_opts = 0;
    char *opts[MAX_OPTS];
    char *tok;
    int base;
    
    /* begin test scaffolding parameters */
    /* int format = AFMT_U8; AFMT_S16_LE; */
    /* int stereo = 0; */     /* 0=mono, 1=stereo */
    int default_rate = ESD_DEFAULT_RATE, default_buf_size = ESD_BUF_SIZE;
    int i, j, freq=440;
    int magl, magr;

    int first = 1;

    int default_format = ESD_BITS16 | ESD_STEREO;
    /* end test scaffolding parameters */

    programname = *argv;

#ifdef DRIVER_ARTS
    artschk();
#endif

    /* read in esd.conf, ~/.esd.conf, ESD_SPAWN_OPTIONS or ESD_OPTIONS*/
    esd_config_read();
    
    /* copy default options */
    strncat (tmp_str, esd_default_options, LINEBUF_SIZE);


    /* has server been spawned */
    esd_spawning = 0;
    if (!esd_no_spawn && (argc > 1)) {
     for (i = 1; i < argc - 1 ; i++) {
	    if (argv[i] && strcmp("-spawnfd", argv[i]) == 0) {
		esd_spawning = 1;
		break;		
	    }
      }
    }
	    
    
    /* copy esd_spawn_options to tmp_str because of strtok */
    if (esd_spawning) {
      strncpy(tmp_str, esd_spawn_options, LINEBUF_SIZE);
    }
    
    /* add esd.conf options */
    num_opts = 0;
    tok = DO_STRTOK(tmp_str, " ");
    while (tok && num_opts < MAX_OPTS) {
      opts[num_opts] = tok;
      num_opts++;
      tok = DO_STRTOK(NULL, " ");
    }
    
    /* now add argv to the end of opts, so command line args take
     * precedence over config options.  we don't add argv[0] */
    base = num_opts;
    for (i = 0; i < argc - 1 && num_opts < MAX_OPTS; i++) {
      opts[base + i] = argv[i+1];
      num_opts++;
    }
       
    /* parse all args */
    for ( arg = 0 ; arg < num_opts ; arg++ ) {
	if ( !strcmp( opts[ arg ], "-d" ) ) {
	    if ( ++arg != num_opts ) {
		esd_audio_device = opts[ arg ];
		if ( !esd_audio_device ) {
		    esd_port = ESD_DEFAULT_PORT;
		    fprintf( stderr, "- could not read device: %s\n",
			     opts[ arg ] );
		}
		fprintf( stderr, "- using device %s\n",
			 esd_audio_device );
	    }
	} else if ( !strcmp( opts[ arg ], "-port" ) ) {
	    if ( ++arg != num_opts ) {
		esd_port = atoi( opts[ arg ] );
		if ( !esd_port ) {
		    esd_port = ESD_DEFAULT_PORT;
		    fprintf( stderr, "- could not read port: %s\n",
			     opts[ arg ] );
		}
		fprintf( stderr, "- accepting connections on port %d\n",
			 esd_port );
	    }

	} else if ( !strcmp( opts[ arg ], "-bind" ) ) {
	    if ( ++arg != num_opts )
		{
			hostname = opts[ arg ];
		}
		fprintf( stderr, "- accepting connections on port %d\n",
			 esd_port );
	} else if ( !strcmp( opts[ arg ], "-b" ) ) {
	    fprintf( stderr, "- server format: 8 bit samples\n" );
	    default_format &= ~ESD_MASK_BITS; default_format |= ESD_BITS8;
	} else if ( !strcmp( opts[ arg ], "-r" ) ) {
	    if ( ++arg != num_opts ) {
		default_rate = atoi( opts[ arg ] );
		if ( !default_rate ) {
		    default_rate = ESD_DEFAULT_RATE;
		    fprintf( stderr, "- could not read rate: %s\n",
			     opts[ arg ] );
		}
		fprintf( stderr, "- server format: sample rate = %d Hz\n",
			 default_rate );
	    }
	} else if ( !strcmp( opts[ arg ], "-as" ) ) {
	    if ( ++arg != num_opts ) {
		esd_autostandby_secs = strtol ( opts[arg], &endptr, 10);
		if ( !esd_autostandby_secs && (!endptr || !*endptr) ) {
		    esd_autostandby_secs = ESD_DEFAULT_AUTOSTANDBY_SECS;
		    fprintf( stderr, "- could not read autostandby timeout: %s\n",
			     opts[ arg ] );
		}
/*		fprintf( stderr, "- autostandby timeout: %d seconds\n",
			 esd_autostandby_secs );*/
	    }
#ifdef ESDBG
	} else if ( !strcmp( opts[ arg ], "-vt" ) ) {
	    esdbg_trace = 1;
	    fprintf( stderr, "- enabling trace diagnostic info\n" );
	} else if ( !strcmp( opts[ arg ], "-vc" ) ) {
	    esdbg_comms = 1;
	    fprintf( stderr, "- enabling comms diagnostic info\n" );
	} else if ( !strcmp( opts[ arg ], "-vm" ) ) {
	    esdbg_mixer = 1;
	    fprintf( stderr, "- enabling mixer diagnostic info\n" );
#endif
	} else if ( !strcmp( opts[ arg ], "-nobeeps" ) ) {
	    esd_beeps = 0;
	} else if ( !strcmp( opts[ arg ], "-beeps" ) ) {
	    esd_beeps = 1;
	} else if ( !strcmp( opts[ arg ], "-unix" ) ) {
	    esd_use_tcpip = 0;
	} else if ( !strcmp( opts[ arg ], "-tcp" ) ) {
	    esd_use_tcpip = 1;
	} else if ( !strcmp( opts[ arg ], "-public" ) ) {
	    esd_public = 1;
	} else if ( !strcmp( opts[ arg ], "-promiscuous" ) ) {
	    esd_is_owned = 1;
	    esd_is_locked = 0;
	} else if ( !strcmp( opts[ arg ], "-terminate" ) ) {
	    esd_terminate = 1;
	} else if ( !strcmp( opts[ arg ], "-noterminate" ) ) {
	    esd_terminate = 0;
	} else if ( !strcmp( opts[ arg ], "-spawnpid" ) ) {
	    if ( ++arg < num_opts )
		esd_spawnpid = atoi( opts[ arg ] );
	} else if ( !strcmp( opts[ arg ], "-spawnfd" ) ) {
	    if ( ++arg < num_opts )
		esd_spawnfd = atoi( opts[ arg ] );
	} else if ( !strcmp( opts[ arg ], "-trust" ) ) {
	    esd_trustval = 0;
	} else if ( !strcmp( opts[ arg ], "-v" ) || !strcmp( opts[ arg ], "--version" ) ) {
		fprintf(stderr, "Esound version " VERSION "\n");
		exit (0);
	} else if ( !strcmp( opts[ arg ], "-h" ) || !strcmp( opts[ arg ], "--help" ) ) {
	    fprintf( stderr, "Esound version " VERSION "\n\n");
	    fprintf( stderr, "Usage: esd [options]\n\n" );
            fprintf( stderr, "  -v --version  print version information\n" );
	    fprintf( stderr, "  -d DEVICE     force esd to use sound device DEVICE\n" );
	    fprintf( stderr, "  -b            run server in 8 bit sound mode\n" );
	    fprintf( stderr, "  -r RATE       run server at sample rate of RATE\n" );
	    fprintf( stderr, "  -as SECS      free audio device after SECS of inactivity (-1 to disable)\n" );
	    fprintf( stderr, "  -unix         use unix domain sockets instead of tcp/ip\n" );
	    fprintf( stderr, "  -tcp          use tcp/ip sockets instead of unix domain\n" );
	    fprintf( stderr, "  -public       make tcp/ip access public (other than localhost)\n" );
	    fprintf( stderr, "  -promiscuous  start unlocked and owned (disable authenticaton) NOT RECOMMENDED\n" );
	    fprintf( stderr, "  -terminate    terminate esd daemon after last client exits\n" );
	    fprintf( stderr, "  -noterminate  do not terminate esd daemon after last client exits\n" );
	    fprintf( stderr, "  -nobeeps      disable startup beeps\n" );
	    fprintf( stderr, "  -beeps        enable startup beeps\n" );
	    fprintf( stderr, "  -trust        start esd even if use of %s can be insecure\n",
		     ESD_UNIX_SOCKET_DIR );
#ifdef ESDBG
	    fprintf( stderr, "  -vt           enable trace diagnostic info\n" );
	    fprintf( stderr, "  -vc           enable comms diagnostic info\n" );
	    fprintf( stderr, "  -vm           enable mixer diagnostic info\n" );
#endif
	    fprintf( stderr, "  -port PORT    listen for connections on PORT (only for tcp/ip)\n" );
	    fprintf( stderr, "  -bind ADDRESS binds to ADDRESS (only for tcp/ip)\n" );
	    fprintf( stderr, "\nPossible devices are:  %s\n", esd_audio_devices() );
	    exit( 0 );
	} else {
	    fprintf( stderr, "unrecognized option: %s\n", opts[ arg ] );
	}
    }

    /* cd to / */
    chdir ("/");

    /* close all open file descriptors */
    max_fds = getdtablesize();
    for(i=0;i<max_fds;i++){
      close(i);
    }
    fd0 = open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

    /* open the listening socket */
  listen_socket = open_listen_socket(hostname, esd_port );
  if ( listen_socket < 0 ) {
    fprintf( stderr, "fatal error opening socket\n" );
    if (!esd_use_tcpip)
      {
	unlink(ESD_UNIX_SOCKET_NAME);
	rmdir(ESD_UNIX_SOCKET_DIR);
      }
    exit( 1 );	
  }

#define ESD_AUDIO_STUFF \
    esd_sample_size = ( (esd_audio_format & ESD_MASK_BITS) == ESD_BITS16 ) \
	? sizeof(signed short) : sizeof(unsigned char); \
    esd_buf_size_samples = default_buf_size / 2; \
    esd_buf_size_octets = esd_buf_size_samples * esd_sample_size;

    /* start the initializatin process */
/*    printf( "ESound daemon initializing...\n" );*/

    /* set the data size parameters */
    esd_audio_format = default_format;
    esd_audio_rate = default_rate;
    ESD_AUDIO_STUFF;

  /* open and initialize the audio device, /dev/dsp */
  itmp = esd_audio_open();
  if (itmp == -2) { /* Special return value indicates that opening the device failed. Don't bother
		       trying */
    if(esd_spawnpid)
      kill(esd_spawnpid, SIGALRM); /* Startup failed */
    
    if(esd_spawnfd) {
	char c = 0; /* Startup failed */
	write (esd_spawnfd, &c, 1);
    }

    if (!esd_use_tcpip) {
	unlink(ESD_UNIX_SOCKET_NAME);
	rmdir(ESD_UNIX_SOCKET_DIR);
      }
    exit (2);
  } else if ( itmp < 0 ) {
    fprintf(stderr, "Audio device open for 44.1Khz, stereo, 16bit failed\n"
	    "Trying 44.1Khz, 8bit stereo.\n");
    /* cant do defaults ... try 44.1 kkz 8bit stereo */
    esd_audio_format = ESD_BITS8 | ESD_STEREO;
    esd_audio_rate = 44100;
    ESD_AUDIO_STUFF;
    if ( esd_audio_open() < 0 ) {
      fprintf(stderr, "Audio device open for 44.1Khz, stereo, 8bit failed\n"

	    "Trying 48Khz, 16bit stereo.\n");
    /* cant do defaults ... try 48 kkz 16bit stereo */
    esd_audio_format = ESD_BITS16 | ESD_STEREO;
    esd_audio_rate = 48000;
    ESD_AUDIO_STUFF;
    if ( esd_audio_open() < 0 ) {
      fprintf(stderr, "Audio device open for 48Khz, stereo,16bit failed\n"
	      "Trying 22.05Khz, 8bit stereo.\n");
      /* cant do defaults ... try 22.05 kkz 8bit stereo */
      esd_audio_format = ESD_BITS8 | ESD_STEREO;
      esd_audio_rate = 22050;
      ESD_AUDIO_STUFF;
      if ( esd_audio_open() < 0 ) {
	fprintf(stderr, "Audio device open for 22.05Khz, stereo, 8bit failed\n"
		"Trying 44.1Khz, 16bit mono.\n");
	/* cant do defaults ... try 44.1Khz kkz 16bit mono */
	esd_audio_format = ESD_BITS16;
	esd_audio_rate = 44100;
	ESD_AUDIO_STUFF;
	if ( esd_audio_open() < 0 ) {
	  fprintf(stderr, "Audio device open for 44.1Khz, mono, 8bit failed\n"
		  "Trying 22.05Khz, 8bit mono.\n");
	  /* cant do defaults ... try 22.05 kkz 8bit mono */
	  esd_audio_format = ESD_BITS8;
	  esd_audio_rate = 22050;
	  ESD_AUDIO_STUFF;
	  if ( esd_audio_open() < 0 ) {
	    fprintf(stderr, "Audio device open for 22.05Khz, mono, 8bit failed\n"
		    "Trying 11.025Khz, 8bit stereo.\n");
	    /* cant to defaults ... try 11.025 kkz 8bit stereo */
	    esd_audio_format = ESD_BITS8 | ESD_STEREO;
	    esd_audio_rate = 11025;
	    ESD_AUDIO_STUFF;
	    if ( esd_audio_open() < 0 ) {
	      fprintf(stderr, "Audio device open for 11.025Khz, stereo, 8bit failed\n"
		      "Trying 11.025Khz, 8bit mono.\n");
	      /* cant to defaults ... try 11.025 kkz 8bit mono */
	      esd_audio_format = ESD_BITS8;
	      esd_audio_rate = 11025;
	      ESD_AUDIO_STUFF;
	      if ( esd_audio_open() < 0 ) {
		fprintf(stderr, "Audio device open for 11.025Khz, mono, 8bit failed\n"
			"Trying 8.192Khz, 8bit mono.\n");
	        /* cant to defaults ... try 8.192 kkz 8bit mono */
		esd_audio_format = ESD_BITS8;
		esd_audio_rate = 8192;
		ESD_AUDIO_STUFF;
		if ( esd_audio_open() < 0 ) {
		  fprintf(stderr, "Audio device open for 8.192Khz, mono, 8bit failed\n"
			  "Trying 8Khz, 8bit mono.\n");
		  /* cant to defaults ... try 8 kkz 8bit mono */
		  esd_audio_format = ESD_BITS8;
		  esd_audio_rate = 8000;
		  ESD_AUDIO_STUFF;
		  if ( esd_audio_open() < 0 ) {
		    fprintf(stderr, "Sound device inadequate for Esound. Fatal.\n");
		    if (!esd_use_tcpip)
		      {
			unlink(ESD_UNIX_SOCKET_NAME);
			rmdir(ESD_UNIX_SOCKET_DIR);
		      }
		    if(esd_spawnpid)
		      kill(esd_spawnpid, SIGALRM); /* Startup failed */

		    if(esd_spawnfd) {
			char c = 0; /* Startup failed */
			write (esd_spawnfd, &c, 1);
		    }
		    
		    exit( 1 );
		  }
		}
	      }
	    }
	  }
	}
      }
    }
   }
  }

    /* allocate and zero out buffer */
    output_buffer = (void *) malloc( esd_buf_size_octets );
    memset( output_buffer, 0, esd_buf_size_octets );


    /* install signal handlers for program integrity */
    signal( SIGINT, clean_exit );	/* for ^C */
    signal( SIGTERM, clean_exit );	/* for default kill */
    signal( SIGPIPE, reset_signal );	/* for closed rec/mon clients */
    signal( SIGHUP, reset_daemon );	/* kill -HUP clear ownership */

    /* send some sine waves just to check the sound connection */
    i = 0;
    if ( esd_beeps ) {
	magl = magr = ( (esd_audio_format & ESD_MASK_BITS) == ESD_BITS16)
	    ? 30000 : 100;

	for ( freq = 55 ; freq < esd_audio_rate/2 ; freq *= 2, i++ ) {
	    /* repeat the freq for a few buffer lengths */
	    for ( j = 0 ; j < esd_audio_rate / 4 / esd_buf_size_samples ; j++ ) {
		set_audio_buffer( output_buffer, esd_audio_format,
				  ( (i%2) ? magl : 0 ),  ( (i%2) ? 0 : magr ),
				  freq, esd_audio_rate, esd_buf_size_samples,
				  j * esd_buf_size_samples );
		esd_audio_write( output_buffer, esd_buf_size_octets );
	    }
	}
    }

    /* put some stuff in the sound driver before pausing */
    esd_audio_write( NULL, 0);

    /* pause the sound output */
    esd_audio_pause();

    /* Startup succeeded */
    if(esd_spawnpid)
      kill(esd_spawnpid, SIGUSR1);

    if(esd_spawnfd) {
	char c = 1; /* Startup succeeded */
	write (esd_spawnfd, &c, 1);
    }

    /* until we kill the daemon */
    while ( 1 )
    {
	/* block while waiting for more clients and new data */
	wait_for_clients_and_data( listen_socket );

	/* accept new connections */
	get_new_clients( listen_socket );


	if ((esd_clients_list == NULL) && (!esd_playing_samples) &&
            (!first) && (esd_terminate)) {
/*	  fprintf(stderr, "No clients!\n");*/
	  clean_exit(0);
	  exit(0);
	}

	/* check for new protocol requests */
	poll_client_requests();
	first = 0;

	/* mix new requests, and output to device */
	refresh_mix_funcs(); /* TODO: set a flag to cue when to do this */
	length = mix_players( output_buffer, esd_buf_size_octets );
	
	/* awaken if on autostandby and doing anything */
	if ( esd_on_autostandby && length && !esd_forced_standby ) {
	    ESDBG_TRACE( printf( "stuff to play, waking up.\n" ); );
	    esd_server_resume();
	}

	/* we handle this even when length == 0 because a filter could have
	 * closed, and we don't want to eat the processor if one did.. */
	if ( esd_filter_list && !esd_on_standby ) {
	    length = filter_write( output_buffer, length,
				   esd_audio_format, esd_audio_rate );
	}
	
	if ( length > 0 /* || esd_monitor */ ) {
	    /* do_sleep = 0; */
	    if ( !esd_on_standby ) {
		/* standby check goes in here, so esd will eat sound data */
		/* TODO: eat a round of data with a better algorithm */
		/*        this will cause guaranteed timing issues */
		/* TODO: on monitor, why isn't this a buffer of zeroes? */
		/* esd_audio_write( output_buffer, esd_buf_size_octets ); */
		esd_audio_write( output_buffer, length );
		/* esd_audio_flush(); */ /* this is overkill */
		esd_last_activity = time( NULL );
	    }
	} else {
	    /* should be pausing just fine within wait_for_clients_and_data */
	    /* if so, this isn't really needed */

	    /* be very quiet, and wait for a wabbit to come along */
#if 0
	    if ( !do_sleep ) { ESDBG_TRACE( printf( "pausing in esd.c\n" ); ); }
	    do_sleep = 1;
	    esd_audio_pause();
#endif
	}

	/* if someone's monitoring the sound stream, send them data */
	/* mix_players, above, forces buffer to zero if no players */
	/* this clears out any leftovers from recording, below */
	if ( esd_monitor_list && !esd_on_standby && length ) {
	/* if ( esd_monitor_list && !esd_on_standby ) {  */
	    monitor_write( output_buffer, length );
	}

	/* if someone's recording the sound stream, send them data */
	if ( esd_recorder_list && !esd_on_standby ) {
	    length = esd_audio_read( output_buffer, esd_buf_size_octets );
	    if ( length ) {
		length = recorder_write( output_buffer, length );
		esd_last_activity = time( NULL );
	    }
	}

	if ( esd_on_standby ) {
#ifdef HAVE_NANOSLEEP
	    struct timespec restrain;
	    restrain.tv_sec = 0;
	    /* funky math to make sure a long can hold it all, calulate in ms */
	    restrain.tv_nsec = (long) esd_buf_size_samples * 1000L
		/ (long) esd_audio_rate / 4L;   /* divide by two for stereo */
	    restrain.tv_nsec *= 1000000L;       /* convert to nanoseconds */
	    nanosleep( &restrain, NULL );
#else
	    struct timeval restrain;
	    restrain.tv_sec = 0;
	    /* funky math to make sure a long can hold it all, calulate in ms */
	    restrain.tv_usec = (long) esd_buf_size_samples * 1000L
		/ (long) esd_audio_rate / 4L; 	/* divide by two for stereo */
	    restrain.tv_usec *= 1000L; 		/* convert to microseconds */
	    select( 0, 0, 0, 0, &restrain );
#endif
	}
    } /* while ( 1 ) */

    close (listen_socket);

    /* how we'd get here, i have no idea, should only exit on signal */
    clean_exit( -1 );
    exit( 0 );
}

