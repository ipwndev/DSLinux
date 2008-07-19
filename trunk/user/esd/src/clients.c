#include "esd-server.h"
#include <arpa/inet.h>
#include <time.h>

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

#ifdef USE_LIBWRAP
#include <tcpd.h>
#include <syslog.h>

int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;
#endif

/*******************************************************************/
/* globals */
extern int esd_use_tcpip; 
#if defined (ENABLE_IPV6)
extern int esd_use_ipv6;
#endif

/* the list of the currently connected clients */
esd_client_t *esd_clients_list;

/*******************************************************************/
/* prototypes */
void dump_clients(void);
void free_client( esd_client_t *client );


/*******************************************************************/
/* for debugging purposes, dump the list of the clients and data */


void dump_clients(void)
{
    long addr;
    short port;
    esd_client_t *clients = esd_clients_list;

    if ( !esdbg_trace ) return;

    while ( clients != NULL ) {
#if defined (ENABLE_IPV6)
     if ( esd_use_ipv6 ) {
       char addrbuf[INET6_ADDRSTRLEN];
       port = ntohs( clients->source6.sin6_port );
#ifdef HAVE_INET_NTOP
       addrbuf[0] = '\0';
       if ( inet_ntop( AF_INET6, &(clients->source6.sin6_addr), addrbuf, sizeof(addrbuf) ))
         printf( "Client from: %s/%d [%p]\n", addrbuf, port, clients );
#endif
     }
     else
#endif
     {
	port = ntohs( clients->source.sin_port );
	addr = ntohl( clients->source.sin_addr.s_addr );

	printf( "(%02d) client from: %03u.%03u.%03u.%03u:%05d [%p]\n", 
		clients->fd, (unsigned int) addr >> 24, 
		(unsigned int) (addr >> 16) % 256, 
		(unsigned int) (addr >> 8) % 256, 
		(unsigned int) addr % 256, port, clients );

	clients = clients->next;
     }
    }
    return;
}

/*******************************************************************/
/* deallocate memory for the client */
void free_client( esd_client_t *client )
{
    /* free the client memory */
    free( client );
    return;
}

/*******************************************************************/
/* add a complete new client into the list of clients at head */
void add_new_client( esd_client_t *new_client )
{
    /* printf ( "adding client 0x%08x\n", new_client ); */
    new_client->next = esd_clients_list;
    esd_clients_list = new_client;
    return;
}

/*******************************************************************/
/* erase a client from the client list */
void erase_client( esd_client_t *client )
{
    esd_client_t *previous = NULL;
    esd_client_t *current = esd_clients_list;

    /* iterate until we hit a NULL */
    while ( current != NULL )
    {
	/* see if we hit the target client */
	if ( current == client ) {
	    if( previous != NULL ){
		/* we are deleting in the middle of the list */
		previous->next = current->next;
	    } else { 
		/* we are deleting the head of the list */
		esd_clients_list = current->next;
	    }

	    ESDBG_TRACE( printf ( "(%02d) closing client connection\n", 
				  client->fd ); );

	    close( client->fd );
	    free_client( client );

	    return;
	}

	/* iterate through the list */
	previous = current;
	current = current->next;
    }

    /* hmm, we didn't find the desired client, just get on with life */
    ESDBG_TRACE( printf( "(%02d) client not found\n", client->fd ); );
    return;
}


/*******************************************************************/
/* checks for new connections at listener - zero when done */
int get_new_clients( int listen )
{
    int fd, nbl;
    struct sockaddr_in incoming;
#if defined (ENABLE_IPV6)
    struct sockaddr_in6 incoming6;
    size_t size_in6 = sizeof(struct sockaddr_in6);
#endif
    size_t size_in = sizeof(struct sockaddr_in);
    esd_client_t *new_client = NULL;
    
    unsigned long addr;
    short port;

    /* see who awakened us */
    do {
#if defined (ENABLE_IPV6)
      if ( esd_use_ipv6 ) {
	char addrbuf[INET6_ADDRSTRLEN];

	fd = accept( listen,(struct sockaddr *)&incoming6, &size_in6 );
	if ( fd < 0 )
                goto again;
	port = ntohs( incoming6.sin6_port );
#ifdef HAVE_INET_NTOP
	addrbuf[0] = '\0';
	if ( inet_ntop( AF_INET6, &(incoming6.sin6_addr), addrbuf, sizeof(addrbuf) ))
	  ESDBG_TRACE( printf( "client from :%s/%d\n", addrbuf, port ); );
#endif
      }
      else
#endif
      {
	fd = accept( listen, (struct sockaddr*) &incoming, &size_in );
	if ( fd < 0 )
		goto again;
	    port = ntohs( incoming.sin_port );
	    addr = ntohl( incoming.sin_addr.s_addr );

	    ESDBG_TRACE( 
		printf( "(%02d) new client from: %03u.%03u.%03u.%03u:%05d\n", 
			fd, (unsigned int) addr >> 24, 
			(unsigned int) (addr >> 16) % 256, 
			(unsigned int) (addr >> 8) % 256, 
			(unsigned int) addr % 256, port ); );
      }     

#ifdef USE_LIBWRAP
	    if (esd_use_tcpip)
	    {
		struct request_info req;
		struct servent *serv;

		request_init( &req, RQ_DAEMON, "esound", RQ_FILE, fd, NULL );
		fromhost( &req );

		if ( !hosts_access( &req )) {
		    ESDBG_TRACE( 
			printf( "connection from %s refused by tcp_wrappers\n",
				eval_client( &req ) ); );

		    close( fd );
		    continue;
		}
	    }
#endif

	    ESDBG_COMMS( printf( "================================\n" ); );

	    /* make sure we have the memory to save the client... */
	    new_client = (esd_client_t*) malloc( sizeof(esd_client_t) );
	    if ( new_client == NULL ) {
		close( fd );
		return -1;
	    }

	    /* It appears that not all systems construct the new socket in
	     * a blocking mode, if the listening socket is non-blocking, so
	     * let's set that here...
	     */
	    nbl = 0;
	    if ( ioctl( fd, FIONBIO, &nbl ) < 0 )
	    {
		ESDBG_TRACE( printf( "(%02d) couldn't turn on blocking for client\n", 
				     fd ); );
		close( fd );
		free( new_client );
		return -1;
	    }

	    /* Reduce buffers on sockets to the minimum needed */
	    esd_set_socket_buffers( fd, ESD_BITS16, 44100, esd_audio_rate );

	    /* fill in the new_client structure - sockaddr = works!? */
	    new_client->next = NULL;
	    new_client->state = ESD_NEEDS_REQDATA;
	    new_client->request = ESD_PROTO_CONNECT;
	    new_client->fd = fd;
#if defined (ENABLE_IPV6)
	    if ( esd_use_ipv6 )
		new_client->source6 = incoming6;
	    else
#endif
		new_client->source = incoming; 
	    new_client->proto_data_length = 0;
	    
	    add_new_client( new_client );
again:
           if( fd < 0 )
               ESDBG_TRACE ( printf("Error encountered while accepting connection\n"));
    } while ( fd > 0 );

    return 0;
}

static int is_paused_here = 0;
/*******************************************************************/
/* blocks waiting for data from the listener, and client conns. */
int wait_for_clients_and_data( int listen )
{
    fd_set rd_fds;
    struct timeval timeout;
    struct timeval *timeout_ptr = NULL;
    esd_client_t *client = esd_clients_list;
    int max_fd = listen, ready;

    /* add the listener to the file descriptor list */
    FD_ZERO( &rd_fds );
    FD_SET( listen, &rd_fds );

    /* add the clients to the list, too */
    while ( client != NULL )
    {
	/* add this client, but only if it's not monitoring */
	if ( client->state == ESD_STREAMING_DATA &&
	     client->request == ESD_PROTO_STREAM_MON )
	{
	    client = client->next;
	    continue;
	}

	FD_SET( client->fd, &rd_fds );

	/* update the maximum fd for the select() */
	if ( client->fd > max_fd )
	    max_fd = client->fd;

	/* next client */
	client = client->next;
    }

    /* if we're doing something useful, make sure we return immediately */
    if ( esd_recorder_list || esd_playing_samples ) {
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	timeout_ptr = &timeout;
    } else {

	/* TODO: any reason not to pause indefinitely here? */
	/* sample players that's why, if no players, can pause indefinitely */
	/* if ( esd_on_autostandby 
	        || (esd_autostandby_secs < 0 && !esd_playing_samples ) )
	       timeout_ptr = NULL; else { ... } */

	if ( is_paused_here ) {

	    ESDBG_TRACE( printf( "paused, awaiting instructions.\n" ); );
	    timeout_ptr = NULL;

	} else {

	    timeout.tv_sec = 0;
	    /* funky math to make sure a long can hold it all, calulate in ms */
	    timeout.tv_usec = (long) esd_buf_size_samples * 1000L
		/ (long) esd_audio_rate / 4L; 	/* divide by two for stereo */
	    timeout.tv_usec *= 1000L; 		/* convert to microseconds */
	    timeout_ptr = &timeout;

	}
    }

    ready = select( max_fd+1, &rd_fds, NULL, NULL, timeout_ptr );

    ESDBG_COMMS( printf( 
	"paused=%d, samples=%d, auto=%d, standby=%d, record=%d, ready=%d\n",
	is_paused_here, esd_playing_samples, 
	esd_autostandby_secs, esd_on_standby, 
	(esd_recorder_list != 0), ready ); );

    /* TODO: return ready, and do this in esd.c */
    if ( ready <= 0 ) {
	/* if < 0, something horrible happened:
	   EBADF   invalid file descriptor - let individual read sort it out
	   EINTR   non blocked signal caught - o well, no big deal
	   EINVAL  n is negative - not bloody likely
	   ENOMEM  unable to allocate internal tables - o well, no big deal */

	if ( !is_paused_here && !esd_playing_samples && (esd_autostandby_secs<0) ) {
	    ESDBG_TRACE( printf( "doing nothing, pausing server.\n" ); );
	    esd_audio_flush();
	    esd_audio_pause();
	    esd_last_activity = time( NULL );
	    is_paused_here = 1;
	}

	if ( !is_paused_here && !esd_playing_samples && !esd_recorder_list ) {

	    if ( esd_autostandby_secs >= 0
		 && ( time(NULL) > esd_last_activity + esd_autostandby_secs ) ) {
		ESDBG_TRACE( printf( "bored, going to standby mode.\n" ); );
		esd_server_standby();
		esd_on_autostandby = 1;
		is_paused_here = 1;
	    }

	}

    } else {

	is_paused_here = 0;

    }

    return ready;
}
