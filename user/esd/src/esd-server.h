#ifndef ESD_SERVER_H
#define ESD_SERVER_H

/* get public information from the public header file */
#include "esd.h"

#include "config.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>

/*******************************************************************/
/* sound daemon data structures */

/* the ESD_*'s are defined from the lsb for client format info, */
/* from the msb for server side info. it should all fit into an int */

/* endianness of data */
#define ESD_MASK_END	( 0xF000000 )
#define ESD_ENDSAME	( 0x0000000 )
#define ESD_ENDDIFF	( 0x1000000 )

/* a client may be in one of several states */
enum esd_client_state {
    ESD_STREAMING_DATA,		/* data from here on is streamed data */
    ESD_CACHING_SAMPLE,		/* midway through caching a sample */
    ESD_NEEDS_REQDATA,		/* more data needed to complere request */
    ESD_NEXT_REQUEST,		/* proceed to next request */
    ESD_CLIENT_STATE_MAX	/* place holder */
};
typedef int esd_client_state_t;

/* mix functions control how the player data is combined */
typedef int (*mix_func_t)( void *player, int length );
typedef int (*translate_func_t)( void *dest_buf, int dest_len, 
				 int dest_rate, esd_format_t dest_format, 
				 void *source_data, int src_len, 
				 int src_rate, esd_format_t src_format );
/* NOTE: in heavy flux! */

/* a client is what contacts the server, and makes requests of daemon */
typedef struct esd_client {
    struct esd_client *next; 	/* it's a list, eh? link 'em */

    esd_client_state_t state;	/* which of above states are we in? */
  
    esd_proto_t request;	/* current request for this client */
    int fd;			/* the clients protocol stream */
#if defined(ENABLE_IPV6)
    struct sockaddr_in6 source6;
#endif
    struct sockaddr_in source;	/* data maintained about source */

    int swap_byte_order;	/* for big/little endian compatibility */
    octet proto_data[ 1024 ];	/* hold the protocol request data */
    int proto_data_length;	/* how much request data we have */
} esd_client_t;

/* a player is what produces data for a sound */
typedef struct esd_player {
    struct esd_player *next;	/* point to next player in list */
    void *parent;		/* client or sample that spawned player */

    esd_format_t format;	/* magic int with the format info */
    int rate;			/* sample rate */
    int left_vol_scale;		/* volume scaling */
    int right_vol_scale;

    int source_id;		/* either a stream fd or sample id */
    octet *data_buffer;		/* buffer to hold sound data */
    int buffer_length;		/* total buffer length */
    int actual_length;		/* actual length of data in buffer */

    /* time_t last_read; */	/* timeout for streams, not used */
    int last_pos;		/* track read position for samples */
    char name[ ESD_NAME_MAX ];	/* name of stream for remote control */

    mix_func_t mix_func;	/* mixes data into the combined buffer */
    translate_func_t translate_func;	/* copies data between players */
} esd_player_t;

/* TODO?: typedef esd_player_t esd_recorder_t, and monitor? */

/* a sample is a pre-cached sound, played by sample_id */
typedef struct esd_sample {
    struct esd_sample *next;	/* point to next sample in list */
    struct esd_client *parent;	/* the client that spawned sample */

    esd_format_t format;	/* magic int with the format info */
    int rate;			/* sample rate */
    int left_vol_scale;		/* volume scaling */
    int right_vol_scale;

    int sample_id;		/* the sample's id number */
    octet *data_buffer;		/* buffer to hold sound data */
    int sample_length;		/* total buffer length */

    int cached_length;		/* amount of data cached so far */
    int ref_count;		/* track players for clean deletion */
    int erase_when_done;	/* track uncache requests */
    char name[ ESD_NAME_MAX ];	/* name of sample for remote control */
} esd_sample_t;

/*******************************************************************/
/* server function prototypes */

/* esd.c - global variables */
extern int esd_is_owned;
extern int esd_is_locked;
extern char esd_owner_key[ESD_KEY_LEN];

extern int esd_on_standby;
extern int esdbg_trace;
extern int esdbg_comms;
extern int esdbg_mixer;

extern int esd_buf_size_octets;
extern int esd_buf_size_samples;
extern int esd_sample_size;

extern int esd_autostandby_secs;
extern time_t esd_last_activity;
extern int esd_on_autostandby;

int esd_server_standby(void);
int esd_server_resume(void);

/* clients.c - manage the client connections */
extern esd_client_t *esd_clients_list;
void dump_clients(void);
void add_new_client( esd_client_t *new_client );
void erase_client( esd_client_t *client );

int get_new_clients( int listen );
int wait_for_clients_and_data( int listen );

/* filter.c - things with which to handle filters */
extern esd_player_t *esd_filter_list;
extern translate_func_t esd_first_filter_func;

void erase_filter( esd_player_t *filter );

/* proto.c - deal with client protocol requests */
extern int esd_forced_standby;

int poll_client_requests(void);

/* players.c - manage the players, recorder, and monitor */
extern esd_player_t *esd_players_list;
extern esd_player_t *esd_recorder_list;
extern esd_player_t *esd_monitor_list;

void dump_players(void);
void add_player( esd_player_t *player );
void erase_player( esd_player_t *player );
void free_player( esd_player_t *player );
void erase_monitor( esd_player_t *monitor );
void add_recorder( esd_player_t *recorder );
void erase_recorder( esd_player_t *recorder );

esd_player_t *new_stream_player( esd_client_t *client );
esd_player_t *new_sample_player( int id, int loop );

int write_player( esd_player_t *player, void *src_buffer, int src_length, 
		  int src_rate, int src_format );
int read_player( esd_player_t *player );
int recorder_write( void *buffer, int length );
void monitor_write( void *output_buffer, int length );

/* samples.c - manage the players, recorder, and monitor */
extern esd_sample_t *esd_samples_list;
extern int esd_playing_samples;
extern int esd_next_sample_id;

void dump_samples(void);
void add_sample( esd_sample_t *sample );
void erase_sample( int id, int force );
esd_sample_t *new_sample( esd_client_t *client );
esd_sample_t *find_caching_sample( esd_client_t *client );
int read_sample( esd_sample_t *sample );
int play_sample( int sample_id, int loop );
int stop_sample( int sample_id );

/* mix.c - deal with mixing signals, and format conversion */
mix_func_t get_mix_func( esd_player_t *player );
translate_func_t get_translate_func( esd_format_t src_fmt, int src_rate,
				     esd_format_t dst_fmt, int dst_rate );

int refresh_mix_funcs();
int mix_players( void *mixed, int length );

/*******************************************************************/
/* filter.c */
int filter_write( void *buffer, int size, esd_format_t format, int rate );

/*******************************************************************/
/* esdlib.c */
int esd_set_socket_buffers( int sock, int src_format, 
			    int src_rate, int base_rate );

/*******************************************************************/
/* evil evil macros */

/* switch endian order for cross platform playing */
#define swap_endian_16(x) ( ( (x) >> 8 ) | ( ((x)&0xFF) << 8 ) )
#define swap_endian_32(x) \
	( ( (x) >> 24 ) | ( ((x)&0xFF0000) >> 8 ) | \
	  ( ((x)&0xFF00) << 8 ) | ( ((x)&0xFF) << 24 ) )

/* the endian key is transferred in binary, if it's read into int, */
/* and matches ESD_ENDIAN_KEY (ENDN), then the endianness of the */
/* server and the client match; if it's SWAP_ENDIAN_KEY, swap data */
#define ESD_SWAP_ENDIAN_KEY swap_endian_32(ESD_ENDIAN_KEY)
#define maybe_swap_32(c,x) \
	( (c) ? (swap_endian_32( (x) )) : (x) )

#define ESD_DEFAULT_AUTOSTANDBY_SECS 5

/* evil macros for debugging protocol */

#ifdef ESDBG /* expand debug macros to yield diagnostic information */

#define ESDBG_TRACE(x) \
	do { \
	    if ( esdbg_trace ) { \
		printf( ":trace: [%12s,%5d] \t", __FILE__, __LINE__ ); \
    		x; \
	    } \
	} while( 0 );

#define ESDBG_COMMS(x) \
	do { \
	    if ( esdbg_comms ) { \
		printf( ":comms: [%12s,%5d] \t", __FILE__, __LINE__ ); \
    		x; \
	    } \
	} while( 0 );

#define ESDBG_MIXER(x) \
	do { \
	    if ( esdbg_mixer ) { \
		printf( "+mixer+ [%12s,%5d] \t", __FILE__, __LINE__ ); \
    		x; \
	    } \
	} while( 0 );

#define ESD_READ_INT(s,a,l,r,d) \
	do { \
	    unsigned int esd_ri; \
    	    r = read( s, a, l ); \
    	    if ( esdbg_comms ) { \
		printf( "---> rd [%12s,%5d] \t%-10s: %2d, %4d = %4d  (%d)     (", \
			__FILE__, __LINE__, d, s, l, r, *a ); \
    		for ( esd_ri = 0 ; esd_ri < sizeof(int) ; esd_ri++ ) \
    		    printf( "0x%02x ", *(((octet*)a)+esd_ri) ); \
		printf( ")\n" ); \
	    } \
        } while ( 0 );

#define ESD_READ_BIN(s,a,l,r,d) \
	do { \
	    unsigned int esd_rb; \
    	    r = read( s, a, l ); \
    	    if ( esdbg_comms ) { \
		printf( "---> rd [%12s,%5d] \t%-10s: %2d, %4d = %4d  (", \
			__FILE__, __LINE__, d, s, l, r ); \
    		for ( esd_rb = 0 ; esd_rb < 8 ; esd_rb++ ) \
    		    printf( "0x%02x ", *(((octet*)a)+esd_rb) ); \
		printf( "...)\n" ); \
	    } \
        } while ( 0 );

#define ESD_WRITE_INT(s,a,l,r,d) \
	do { \
	    unsigned int esd_wi; \
    	    r = write( s, a, l ); \
    	    if ( esdbg_comms ) { \
		printf( "<--- wr [%12s,%5d] \t%-10s: %2d, %4d = %4d  (%d)     (", \
			__FILE__, __LINE__, d, s, l, r, *a ); \
    		for ( esd_wi = 0 ; esd_wi < sizeof(int) ; esd_wi++ ) \
    		    printf( "0x%02x ", *(((octet*)a)+esd_wi) ); \
		printf( ")\n" ); \
	    } \
        } while ( 0 );

#define ESD_WRITE_BIN(s,a,l,r,d) \
	do { \
	    unsigned int esd_wb; \
    	    r = write( s, a, l ); \
    	    if ( esdbg_comms ) { \
		printf( "<--- wr [%12s,%5d] \t%-10s: %2d, %4d = %4d  (", \
			__FILE__, __LINE__, d, s, l, r ); \
    		for ( esd_wb = 0 ; esd_wb < 8 ; esd_wb++ ) \
    		    printf( "0x%02X ", *(((octet*)a)+esd_wb) ); \
		printf( "...)\n" ); \
	    } \
        } while ( 0 );

#else /* #ifdef ESDBG */ /* expand debug macros to simply do, and not say */

#define ESDBG_TRACE(x)
#define ESDBG_COMMS(x)
#define ESDBG_MIXER(x)

#define ESD_READ_INT(s,a,l,r,d)	 r = read( s, a, l );
#define ESD_READ_BIN(s,a,l,r,d)  r = read( s, a, l );
#define ESD_WRITE_INT(s,a,l,r,d) r = write( s, a, l );
#define ESD_WRITE_BIN(s,a,l,r,d) r = write( s, a, l );

#endif /* #ifdef ESDBG */

/* MkLinux on the PowerMac is weird because it is a big-endian processor,
   but it uses little-endian sound streams */
/* TODO: beef up mix.c, and add function pointers for the basic functions */
#ifdef DRIVER_MKLINUX
#define MAYBE_SWAP_FOR_MK(x) ( (((x)&0xFF00)>>8) | (((x)&0x00FF)<<8) )
#else
#define MAYBE_SWAP_FOR_MK(x) (x)
#endif  

#endif /* #ifndef ESD_SERVER_H */
