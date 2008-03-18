
#include "esd-server.h"
#include <errno.h>

/*******************************************************************/
/* globals */

int esd_forced_standby = 0;	/* whether we're forcing the standby */

/*******************************************************************/
/* prototypes */
int esd_check_endian( esd_client_t *client, unsigned int *endian );
int esd_validate_source( esd_client_t *client, 
			 octet *submitted_key, int owner_only );
int esd_proto_unimplemented( esd_client_t *client );
int esd_proto_connect( esd_client_t *client );
int esd_proto_lock( esd_client_t *client );
int esd_proto_unlock( esd_client_t *client );
int esd_proto_standby( esd_client_t *client );
int esd_proto_resume( esd_client_t *client );
int esd_proto_stream_play( esd_client_t *client );
int esd_proto_stream_recorder( esd_client_t *client );
int esd_proto_stream_monitor( esd_client_t *client );
int esd_proto_stream_filter( esd_client_t *client );
int esd_proto_sample_cache( esd_client_t *client );
int esd_proto_sample_getid(esd_client_t *client);
int esd_proto_sample_free( esd_client_t *client );
int esd_proto_sample_play( esd_client_t *client );
int esd_proto_sample_loop( esd_client_t *client );
int esd_proto_sample_stop( esd_client_t *client );
int esd_proto_server_info( esd_client_t *client );
int esd_proto_all_info( esd_client_t *client );
int esd_proto_stream_pan( esd_client_t *client );
int esd_proto_sample_pan( esd_client_t *client );
int esd_proto_standby_mode( esd_client_t *client );
int esd_proto_get_latency( esd_client_t *client );
int poll_client_requests(void);

/*******************************************************************/
/* protocol handlers as function pointers: synch with esd_proto_t enum */
typedef int esd_proto_handler_t( esd_client_t *client );

/* data type to hold the per protocol handler info */
typedef struct esd_proto_handler_info {
    int 		data_length;
    esd_proto_handler_t *handler;
    const char *        description;
} esd_proto_handler_info_t;

/* the big map of protocol handler info */
esd_proto_handler_info_t esd_proto_map[ ESD_PROTO_MAX ] = 
{
    { ESD_KEY_LEN + sizeof(int), &esd_proto_connect, "connect" },
    { ESD_KEY_LEN + sizeof(int), &esd_proto_lock, "lock" },
    { ESD_KEY_LEN + sizeof(int), &esd_proto_unlock, "unlock" },

    { ESD_NAME_MAX + 2 * sizeof(int), &esd_proto_stream_play, "stream play" },
    { ESD_NAME_MAX + 2 * sizeof(int), &esd_proto_stream_recorder, "stream rec" },
    { ESD_NAME_MAX + 2 * sizeof(int), &esd_proto_stream_monitor, "stream mon" },

    { ESD_NAME_MAX + 3 * sizeof(int), &esd_proto_sample_cache, "sample cache" },
    { sizeof(int), &esd_proto_sample_free, "sample free" },
    { sizeof(int), &esd_proto_sample_play, "sample play" },
    { sizeof(int), &esd_proto_sample_loop, "sample loop" },
    { sizeof(int), &esd_proto_sample_stop, "sample stop" },
    { -1, &esd_proto_unimplemented, "TODO: sample kill" },

    { ESD_KEY_LEN + sizeof(int), &esd_proto_standby, "standby" },
    { ESD_KEY_LEN + sizeof(int), &esd_proto_resume, "resume" },

    { ESD_NAME_MAX, &esd_proto_sample_getid, "sample getid" },
    { ESD_NAME_MAX + 2 * sizeof(int), &esd_proto_stream_filter, "stream filter" },

    { sizeof(int), &esd_proto_server_info, "server info" },
    { sizeof(int), &esd_proto_all_info, "all info" },
    { -1, &esd_proto_unimplemented, "TODO: subscribe" },
    { -1, &esd_proto_unimplemented, "TODO: unsubscribe" },

    { 3 * sizeof(int), &esd_proto_stream_pan, "stream pan"},
    { 3 * sizeof(int), &esd_proto_sample_pan, "sample pan" },

    { sizeof(int), &esd_proto_standby_mode, "standby mode" },
    { 0, &esd_proto_get_latency, "get latency" }
};

/***********************************************************************/
/* returns the latency between audio stream data being write() 'en to  */
/* esd and when it finally comes out the speakers of your audio system */
int esd_proto_get_latency( esd_client_t *client )
{
    int lag, amount, actual;

    ESDBG_TRACE( printf( "(%02d) proto: get latency\n", client->fd ); );
  
    if (esd_audio_format & ESD_STEREO)
      {
	if (esd_audio_format & ESD_BITS16)
	  amount = (44100 * (ESD_BUF_SIZE + 64)) / esd_audio_rate;
	else
	  amount = (44100 * (ESD_BUF_SIZE + 128)) / esd_audio_rate;
      }
    else
      {
	if (esd_audio_format & ESD_BITS16)
	  amount = (2 * 44100 * (ESD_BUF_SIZE + 128)) / esd_audio_rate;
	else
	  amount = (2 * 44100 * (ESD_BUF_SIZE + 256)) / esd_audio_rate;
      }
    lag = maybe_swap_32( client->swap_byte_order, amount );

    /* send back the server information */
    ESD_WRITE_INT( client->fd, &lag, sizeof(lag), actual, "lag buf" );
    if ( sizeof( lag ) != actual )
	return 0;
    return 1;
}

/*******************************************************************/
/* checks for client/server endianness */
int esd_check_endian( esd_client_t *client, unsigned int *endian )
{
    if ( *endian == ESD_ENDIAN_KEY ) {
	ESDBG_TRACE( printf( "(%02d) same endian order.\n", client->fd ); );
	client->swap_byte_order = 0;
    } else if ( *endian == ESD_SWAP_ENDIAN_KEY ) {
	ESDBG_TRACE( printf( "(%02d) different endian order!\n", client->fd ); );
	client->swap_byte_order = 1;
    } else {
	ESDBG_TRACE( 
	    printf( "(%02d) unknown endian key: 0x%08x"
		    " (same = 0x%08x, diff = 0x%08x)\n",
		    client->fd, *endian, 
		    ESD_ENDIAN_KEY, ESD_SWAP_ENDIAN_KEY ); );
	return 0;
    }

    /* now we're done */
    return 1;
}

/*******************************************************************/
/* checks for authorization to use the sound daemon */
int esd_validate_source( esd_client_t *client, 
			 octet *submitted_key,
			 int owner_only )
{
  int ok;
    if( !esd_is_owned ) {
	/* noone owns it yet, the first client claims ownership */
	ESDBG_TRACE( printf( "(%02d) esd auth: claiming ownership of esd, auth ok\n", 
			     client->fd ); );

	esd_is_locked = 1;
	memcpy( esd_owner_key, submitted_key, ESD_KEY_LEN );
	esd_is_owned = 1;
        ok = 1;
        write(client->fd, &ok, sizeof(ok));
	return 1;
    }

    if( !esd_is_locked && !owner_only ) {
	/* anyone can connect to it */
	ESDBG_TRACE( printf( "(%02d) esd auth: not locked nor owner only, auth ok.\n", 
			     client->fd ); );
        ok = 1;
        write(client->fd, &ok, sizeof(ok));
	return 1;
    }

    if( !memcmp( esd_owner_key, submitted_key, ESD_KEY_LEN ) ) {
	/* the client key matches the owner, trivial acceptance */
	ESDBG_TRACE( printf( "(%02d) esd auth: key matches, auth ok.\n", 
			     client->fd ); );
        ok = 1;
        write(client->fd, &ok, sizeof(ok));
	return 1;
    }

    /* TODO: maybe check based on source ip? */ 
    /* done with LIBWRAP when client first connects to daemon */
    /* if ( !owner_only ) { check_ip_etc( client->source ); } */

    /* the client is not authorized to connect to the server */
    ESDBG_TRACE( printf( "(%02d) esd auth: NOT authorized to use esd, closing conn.\n",
			 client->fd ); );
    ok = 0;
    write(client->fd, &ok, sizeof(ok));
    return 0;
}

/*******************************************************************/
/* place holder during development */
int esd_proto_unimplemented( esd_client_t *client )
{
    fprintf( stderr, 
	     "(%02d) proto: unimplemented protocol request:  0x%08x\n",
	     client->fd, client->request );
    return 0;
}

/*******************************************************************/
/* initial connection handler, return boolean ok */
int esd_proto_connect( esd_client_t *client )
{
    int ok;

    if ( esd_validate_source( client, client->proto_data, 0 ) )
    {
	ok = esd_check_endian( client, (unsigned int*)(client->proto_data + ESD_KEY_LEN) );
    } else {
	ok = 0;
    }
    
    ESDBG_TRACE( printf( "(%02d) connecting to sound daemon = %d\n", 
			 client->fd, ok ); );
    return ok;
}

/*******************************************************************/
/* daemon rejects untrusted clients, return boolean ok */
int esd_proto_lock( esd_client_t *client )
{
    int ok, client_ok, actual;

    ok = esd_validate_source( client, client->proto_data, 1 );
    client_ok = maybe_swap_32( client->swap_byte_order, ok );

    if ( ok ) {
	ESDBG_TRACE( printf( "(%02d) locking sound daemon\n", client->fd ); );
	esd_is_locked = 1;
    }

    ESD_WRITE_INT( client->fd, &client_ok, sizeof(client_ok), 
		   actual, "lock ok" );
    
    return ok;
}

/*******************************************************************/
/* allows anyone to connect to the sound daemon, return boolean ok */
int esd_proto_unlock( esd_client_t *client )
{
    int ok, client_ok, actual;

    ok = esd_validate_source( client, client->proto_data, 1 );
    client_ok = maybe_swap_32( client->swap_byte_order, ok );

    if ( ok ) {
	ESDBG_TRACE( printf( "(%02d) unlocking sound daemon\n", client->fd ); );
	esd_is_locked = 0;
    }

    ESD_WRITE_INT( client->fd, &client_ok, sizeof(client_ok), 
		   actual, "unlock ok" );
    return ok;
}

/*******************************************************************/
/* daemon eats sound data, without playing anything, return boolean ok */
int esd_proto_standby( esd_client_t *client )
{
    int ok, client_ok, actual;

    ok = esd_validate_source( client, client->proto_data, 1 );

    if ( ok ) ok = esd_server_standby();
    if(!esd_on_autostandby)
	    esd_forced_standby = 1;

    client_ok = maybe_swap_32( client->swap_byte_order, ok );
    ESD_WRITE_INT( client->fd, &client_ok, sizeof(client_ok), 
		   actual, "stdby ok" );
    return ok;
}

/*******************************************************************/
/* daemon eats sound data, without playing anything, return boolean ok */
int esd_proto_resume( esd_client_t *client )
{
    int ok, client_ok, actual;

    ok = esd_validate_source( client, client->proto_data, 1 );

    if ( ok ) ok = esd_server_resume();
    if ( ok ) esd_forced_standby = 0;

    client_ok = maybe_swap_32( client->swap_byte_order, ok );
    ESD_WRITE_INT( client->fd, &client_ok, sizeof(client_ok), 
		   actual, "resum ok" );
    return ok;
}

/*******************************************************************/
/* add another stream player to the list, returns boolean ok */
int esd_proto_stream_play( esd_client_t *client )
{
    /* spawn a new player to handle this stream */
    esd_player_t *player = NULL;
    player = new_stream_player( client );
    
    /* we got one, right? */
    if ( !player ) {
	return 0;
    }
    
    /* add to the list of players */
    player->parent = client;
    add_player( player );

    client->state = ESD_STREAMING_DATA;
    return 1;
}

/*******************************************************************/
/* manage recorder clients, return boolean ok */
int esd_proto_stream_recorder( esd_client_t *client )
{
    esd_player_t *recorder = NULL;
    
    /* wake up if we're asleep */
    if ( esd_on_autostandby  && !esd_forced_standby ) {
	ESDBG_TRACE( printf( "stuff to record, waking up.\n" ); );
	esd_server_resume();
    }

    /* if we're in standby mode, go away */
    if ( esd_on_standby )
	return 0;

    /* sign up the new recorder client */
    recorder = new_stream_player( client );
    /* we got one, right? */
    if ( !recorder )
	return 0;
    
    if (!(esd_audio_format & ESD_RECORD)) {
	/* let the device know we want to record */
	ESDBG_TRACE( printf( "closing audio for a sec...\n" ); );
	esd_audio_close();
	sleep(1);
	esd_audio_format |= ESD_RECORD;
	ESDBG_TRACE( printf( "reopening audio to record...\n" ); );
	if (esd_audio_open() < 0) {
            /* Failed to record */
            free_player( recorder );
            esd_audio_format &= ~ESD_RECORD;
            sleep(1);
            /* If we fail here, we have a oops */
            esd_audio_open();
            return 0;
        }
	ESDBG_TRACE( printf( "reopened?\n" ); );
    }
    
    add_recorder(recorder);
    
    /* flesh out the recorder */
    recorder->parent = client;
    recorder->translate_func 
	    = get_translate_func( esd_audio_format, esd_audio_rate,
				  recorder->format, recorder->rate );

    ESDBG_TRACE( printf ( "(%02d) recording on client\n", client->fd ); );
    
    client->state = ESD_STREAMING_DATA;
    return 1;
}

/*******************************************************************/
/* manage the single monitoring client, return boolean ok */
int esd_proto_stream_monitor( esd_client_t *client )
{
    esd_player_t *monitor;

    /* sign up the new monitor client */
    monitor = new_stream_player( client );
    if ( monitor != NULL ) {
	/* flesh out the monitor */
	monitor->parent = client;
	monitor->next = esd_monitor_list;
	esd_monitor_list = monitor;

	monitor->translate_func 
	    = get_translate_func( esd_audio_format, esd_audio_rate,
				  monitor->format, monitor->rate );

	ESDBG_TRACE( printf ( "(%02d) monitoring on client\n", client->fd ); );
    } else {
	/* failed to initialize the recorder, kill its client */
	return 0;
    }

    client->state = ESD_STREAMING_DATA;
    return 1;
}

/*******************************************************************/
/* manage the filter client, return boolean ok */
int esd_proto_stream_filter( esd_client_t *client )
{
    esd_player_t *filter;
    
    /* sign up the new filter client */
    filter = new_stream_player( client );
    if ( filter != NULL ) {
	/* flesh out the filter */
	filter->parent = client;
	filter->next = esd_filter_list;
	esd_filter_list = filter;

	ESDBG_TRACE( printf ( "(%02d) filter on client\n", client->fd ); );
    } else {
	/* failed to initialize the filter, kill its client */
	return 0;
    }

    client->state = ESD_STREAMING_DATA;
    return 1;
}

/*******************************************************************/
/* cache a sample from the client, return boolean ok  */
int esd_proto_sample_cache( esd_client_t *client )
{
    esd_sample_t *sample;
    int length;
    int client_id;

    ESDBG_TRACE( printf( "(%02d) proto: caching sample\n", client->fd ); );

    if ( client->state == ESD_CACHING_SAMPLE ) {
	sample = find_caching_sample( client );
    } else {
	sample = new_sample( client );
	add_sample( sample );
    }

    /* add to the list of sample */
    if ( sample != NULL ) {
	sample->parent = client;
	if ( !read_sample( sample ) )
	{
	    return 0;	/* something failed during the read, just bail */
	}

	if ( sample->cached_length < sample->sample_length ) {
	    client->state = ESD_CACHING_SAMPLE;
	    ESDBG_TRACE( printf( "(%02d) continue caching sample next trip\n", 
				 client->fd ); );
	    return 1;
	} else {
	    ESDBG_TRACE( printf( "(%02d) sample cached, moving on\n", 
				 client->fd ); );
	    client->state = ESD_NEXT_REQUEST;
	}
    } else {
	fprintf( stderr, "(%02d) not enough mem for sample, closing\n", 
		 client->fd );
	return 0;
    }

    client_id = maybe_swap_32( client->swap_byte_order, 
			       sample->sample_id );
    ESD_WRITE_INT( client->fd, &client_id, sizeof(client_id), 
		   length, "smp cach" );

    return 1;
}

/*******************************************************************/
/* check for an existing sample name */
int esd_proto_sample_getid(esd_client_t *client)
{
    int client_id, length;
    esd_sample_t *sample = esd_samples_list;
    char namebuf[ESD_NAME_MAX];

    strncpy( namebuf, client->proto_data, ESD_NAME_MAX );
    namebuf[ESD_NAME_MAX - 1] = '\0';

    ESDBG_TRACE( printf( "(%02d) proto: getting sample ID: %s\n", 
			 client->fd, namebuf ); );

    while(sample) {
	if(!strcmp(sample->name, namebuf))
	    break;
	sample = sample->next;
    }

    if(sample)
	client_id = maybe_swap_32( client->swap_byte_order, 
				   sample->sample_id );
    else
	client_id = maybe_swap_32(client->swap_byte_order, -1);

    ESD_WRITE_INT( client->fd, &client_id, sizeof(client_id), 
		   length, "smp getid" );
    return 1;
}


/*******************************************************************/
/* free a sample cached by the client, return boolean ok */
int esd_proto_sample_free( esd_client_t *client )
{
    int sample_id, client_id, actual;

    client_id = *(int*)(client->proto_data);
    sample_id = maybe_swap_32( client->swap_byte_order, client_id );

    ESDBG_TRACE( printf( "(%02d) proto: erasing sample <%d>\n", 
			 client->fd, sample_id ); );

    erase_sample( sample_id, 0 );

    ESD_WRITE_INT( client->fd, &client_id, sizeof(client_id), 
		   actual, "smp free" );
    if ( sizeof( client_id ) != actual )
	return 0;

    return 1;
}

/*******************************************************************/
/* play a sample cached by the client, return boolean ok */
int esd_proto_sample_play( esd_client_t *client )
{
    int sample_id, client_id, actual;
    
    client_id = *(int*)(client->proto_data);
    sample_id = maybe_swap_32( client->swap_byte_order, client_id );

    ESDBG_TRACE( printf( "(%02d) proto: playing sample <%d>\n", 
			 client->fd, sample_id ); );

    if ( !play_sample( sample_id, 0 ) )
	sample_id = 0;

    ESD_WRITE_INT( client->fd, &client_id, sizeof(client_id), 
		   actual, "smp play" );
    if ( sizeof( client_id ) != actual )
	return 0;

    return 1;
}

/*******************************************************************/
/* play a sample cached by the client, return boolean ok */
int esd_proto_sample_loop( esd_client_t *client )
{
    int sample_id, client_id, actual;

    client_id = *(int*)(client->proto_data);
    sample_id = maybe_swap_32( client->swap_byte_order, client_id );

    ESDBG_TRACE( printf( "(%02d) proto: looping sample <%d>\n", 
			 client->fd, sample_id ); );

    if ( !play_sample( sample_id, 1 ) )
	sample_id = 0;

    ESD_WRITE_INT( client->fd, &client_id, sizeof(client_id), 
		   actual, "smp loop" );
    if ( sizeof( sample_id ) != actual )
	return 0;

    return 1;
}

/*******************************************************************/
/* play a sample cached by the client, return boolean ok */
int esd_proto_sample_stop( esd_client_t *client )
{
    int sample_id, client_id, actual;

    client_id = *(int*)(client->proto_data);
    sample_id = maybe_swap_32( client->swap_byte_order, client_id );

    ESDBG_TRACE( printf( "(%02d) proto: stopping sample <%d>\n", 
			 client->fd, sample_id ); );

    if ( !stop_sample( sample_id ) )
	sample_id = 0;

    ESD_WRITE_INT( client->fd, &client_id, sizeof(client_id), 
		   actual, "smp stop" );
    if ( sizeof( client_id ) != actual )
	return 0;

    return 1;
}

/*******************************************************************/
/* play a sample cached by the client, return boolean ok */
int esd_proto_server_info( esd_client_t *client )
{
    int version, rate, format, actual;

    version = maybe_swap_32( client->swap_byte_order, 0 );
    rate = maybe_swap_32( client->swap_byte_order, esd_audio_rate );
    format = maybe_swap_32( client->swap_byte_order, esd_audio_format );

    ESDBG_TRACE( printf( "(%02d) proto: server info\n", client->fd ); );

    /* send back the server information */
    ESD_WRITE_INT( client->fd, &version, sizeof(version), actual, "si ver" );
    ESD_WRITE_INT( client->fd, &rate, sizeof(rate), actual, "si rate" );
    ESD_WRITE_INT( client->fd, &format, sizeof(format), actual, "si fmt" );
    if ( sizeof( format ) != actual )
	return 0;

    return 1;
}

/*******************************************************************/
/* play a sample cached by the client, return boolean ok */
int esd_proto_all_info( esd_client_t *client )
{
    int version, rate, left, right, format;
    int actual, source_id, sample_id, length;
    esd_player_t *player;
    esd_sample_t *sample;
    const char *name;
    char no_name[ ESD_NAME_MAX ] = "";

    version = maybe_swap_32( client->swap_byte_order, 0 );
    rate = maybe_swap_32( client->swap_byte_order, esd_audio_rate );
    format = maybe_swap_32( client->swap_byte_order, esd_audio_format );

    ESDBG_TRACE( printf( "(%02d) proto: server info\n", client->fd ); );

    /* send back the server information */
    ESD_WRITE_INT( client->fd, &version, sizeof(version), actual, "ai ver" );
    ESD_WRITE_INT( client->fd, &rate, sizeof(rate), actual, "ai rate" );
    ESD_WRITE_INT( client->fd, &format, sizeof(format), actual, "ai fmt" );
    if ( sizeof( format ) != actual )
	return 0;

    /* send back the player information */
    for ( player = esd_players_list; /* NULL breaks */ ; player = player->next )
    {
	if ( player ) {
	    source_id = maybe_swap_32( client->swap_byte_order, 
				       player->source_id );
	    name = ( (player->format & ESD_MASK_MODE) == ESD_STREAM ) 
		? player->name : ( (esd_sample_t*) (player->parent) )->name;
	    rate = maybe_swap_32( client->swap_byte_order, 
				  player->rate );
	    left = maybe_swap_32( client->swap_byte_order, 
				  player->left_vol_scale );
	    right = maybe_swap_32( client->swap_byte_order, 
				   player->right_vol_scale );
	    format = maybe_swap_32( client->swap_byte_order, 
				    player->format );
	} else {
	    source_id = rate = format = 0;
	    name = no_name;
	}

	ESD_WRITE_INT( client->fd, &source_id, sizeof(source_id), 
		       actual, "ai p.id" );
	ESD_WRITE_BIN( client->fd, name, ESD_NAME_MAX, 
		       actual, "ai p.nm" );
	ESD_WRITE_INT( client->fd, &rate, sizeof(rate), 
		       actual, "ai p.rate" );
	ESD_WRITE_INT( client->fd, &left, sizeof(left), 
		       actual, "ai p.lt" );
	ESD_WRITE_INT( client->fd, &right, sizeof(right), 
		       actual, "ai p.rt" );
	ESD_WRITE_INT( client->fd, &format, sizeof(format), 
		       actual, "ai p.fmt" );
	if ( sizeof( format ) != actual )
	    return 0;

	if ( !player ) break;
    }

    /* send back the sample information */
    for ( sample = esd_samples_list; /* NULL breaks */ ; sample = sample->next )
    {
	if ( sample ) {
	    sample_id = maybe_swap_32( client->swap_byte_order, 
				       sample->sample_id );
	    name = sample->name;
	    rate = maybe_swap_32( client->swap_byte_order, 
				  sample->rate );
	    left = maybe_swap_32( client->swap_byte_order, 
				  sample->left_vol_scale );
	    right = maybe_swap_32( client->swap_byte_order, 
				   sample->right_vol_scale );
	    format = maybe_swap_32( client->swap_byte_order, 
				    sample->format );
	    length = maybe_swap_32( client->swap_byte_order, 
				    sample->sample_length );
	} else {
	    sample_id = rate = format = length = 0;
	    name = no_name;
	}

	ESD_WRITE_INT( client->fd, &sample_id, sizeof(sample_id), 
		       actual, "ai s.id" );
	ESD_WRITE_BIN( client->fd, name, ESD_NAME_MAX, 
		       actual, "ai s.nm" );
	ESD_WRITE_INT( client->fd, &rate, sizeof(rate), 
		       actual, "ai s.rate" );
	ESD_WRITE_INT( client->fd, &left, sizeof(left), 
		       actual, "ai s.lt" );
	ESD_WRITE_INT( client->fd, &right, sizeof(right), 
		       actual, "ai s.rt" );
	ESD_WRITE_INT( client->fd, &format, sizeof(format), 
		       actual, "ai s.fmt" );
	ESD_WRITE_INT( client->fd, &length, sizeof(length), 
		       actual, "ai s.fmt" );
	if ( sizeof( length ) != actual )
	    return 0;

	if ( !sample ) break;
    }

    return 1;
}


/*******************************************************************/
/* set the stereo panning for a stream */
int esd_proto_stream_pan( esd_client_t *client )
{
    int client_id, client_left, client_right, client_ok, actual;
    int stream_id, left, right, ok;
    esd_player_t *player;
    
    client_id = *(int*)(client->proto_data);
    client_left = *(int*)(client->proto_data + sizeof(int));
    client_right = *(int*)(client->proto_data + 2 * sizeof(int));

    stream_id = maybe_swap_32( client->swap_byte_order, client_id );
    left = maybe_swap_32( client->swap_byte_order, client_left );
    right = maybe_swap_32( client->swap_byte_order, client_right );

    ESDBG_TRACE( printf( "(%02d) proto: panning stream <%d> [%d, %d]\n", 
			 client->fd, stream_id, left, right ); );

    /* find the stream, and reset panning */
    ok = 0;
    for ( player = esd_players_list ; player != NULL ; player = player->next )
    {
	if ( player->source_id == stream_id 
	     && ( (player->format & ESD_MASK_MODE) == ESD_STREAM ) )
	{
	    player->left_vol_scale = left;
	    player->right_vol_scale = right;
	    player->mix_func = get_mix_func( player );
	    ok = 1;
	    break;
	}
    }

    /* let the client know how it went */
    client_ok = maybe_swap_32( client->swap_byte_order, ok );
    ESD_WRITE_INT( client->fd, &client_ok, sizeof(client_ok), 
		   actual, "panstr ok" );
    if ( sizeof( client_ok ) != actual )
	return 0;

    return 1;
}


/*******************************************************************/
/* set the default panning for a stream */
int esd_proto_sample_pan( esd_client_t *client )
{
    int client_id, client_left, client_right, client_ok, actual;
    int sample_id, left, right, ok;
    esd_sample_t *sample;
    
    client_id = *(int*)(client->proto_data);
    client_left = *(int*)(client->proto_data + sizeof(int));
    client_right = *(int*)(client->proto_data + 2 * sizeof(int));

    sample_id = maybe_swap_32( client->swap_byte_order, client_id );
    left = maybe_swap_32( client->swap_byte_order, client_left );
    right = maybe_swap_32( client->swap_byte_order, client_right );

    ESDBG_TRACE( printf( "(%02d) proto: panning sample <%d> [%d, %d]\n", 
			 client->fd, sample_id, left, right ); );

    /* find the stream, and reset panning */
    ok = 0;
    for ( sample = esd_samples_list ; sample != NULL ; sample = sample->next )
    {
	if ( sample->sample_id == sample_id )
	{
	    sample->left_vol_scale = left;
	    sample->right_vol_scale = right;
	    ok = 1;
	    break;
	}
    }

    /* let the client know how it went */
    client_ok = maybe_swap_32( client->swap_byte_order, ok );
    ESD_WRITE_INT( client->fd, &client_ok, sizeof(client_ok), 
		   actual, "panstr ok" );
    if ( sizeof( client_ok ) != actual )
	return 0;

    return 1;
}


/*******************************************************************/
/* daemon rejects untrusted clients, return boolean ok */
int esd_proto_standby_mode( esd_client_t *client )
{
    int ok = 1, mode, client_mode, actual;

    if ( esd_on_autostandby && !esd_forced_standby ) 
	mode = ESM_ON_AUTOSTANDBY;
    else if ( esd_on_standby )
	mode = ESM_ON_STANDBY;
    else
	mode = ESM_RUNNING;

    client_mode = maybe_swap_32( client->swap_byte_order, mode );
    ESDBG_TRACE( printf( "(%02d) getting standby mode\n", client->fd ); );

    ESD_WRITE_INT( client->fd, &client_mode, sizeof(client_mode), 
		   actual, "stby mode" );
    if ( sizeof( client_mode ) != actual )
	return 0;
    
    return ok;
}


/*******************************************************************/
/* checks for new client requiests - returns 1 */
int poll_client_requests(void)
{
    int can_read, length = 0, is_ok = 0;
    esd_client_t *client = NULL;
    esd_client_t *erase = NULL;
    fd_set rd_fds;
    struct timeval timeout;

    /* check all clients, as some may become readable between the
       previous blocking select() and now */

    /* for each client */
    client = esd_clients_list;
    while ( client != NULL )
    {
	/* if it's a streaming client connection, just skip it 
	   data will be read (if available) during the mix phase */
	if ( client->state == ESD_STREAMING_DATA ) {
	    client = client->next;
	    continue;
	}
	    
	/* find out if this client wants to do anything yet */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO( &rd_fds );
	FD_SET( client->fd, &rd_fds );
	can_read = select( client->fd + 1, &rd_fds, NULL, NULL, &timeout );
	if ( !can_read ) {
	    client = client->next;
	    continue;
	}

	/* see what the client needs to do next */
	ESDBG_TRACE( printf( "(%02d) client state %d.\n", 
			     client->fd, client->state ); );
	switch ( client->state ) {

	case ESD_NEEDS_REQDATA:

	    /* check for insanity */
	    if ( client->proto_data_length 
		 > esd_proto_map[ client->request ].data_length )
	    {
		ESDBG_TRACE( 
		    printf( "(%02d) REQDATA insanity detected, expecting %d, got %d\n",
			    client->fd,
			    esd_proto_map[ client->request ].data_length,
			    client->proto_data_length); );
		is_ok = 0;
		break;
	    }

	    /* read another chunk of data, if any more is required */
	    if ( esd_proto_map[ client->request ].data_length ) {
		ESD_READ_BIN( client->fd, 
			      client->proto_data + client->proto_data_length, 
			      esd_proto_map[ client->request ].data_length 
			      - client->proto_data_length , length, "req dat" );
		client->proto_data_length += length;
	    }

	    /* check length, as EOF returns readable */
	    if ( !length 
		 || ( length < 0 && errno != EAGAIN && errno != EINTR ) )
	    {
		ESDBG_TRACE( printf( "(%02d) interrupted request %d, %s.\n", 
				     client->fd, client->request, 
				     esd_proto_map[ client->request ].description ); );
		is_ok = 0;
		break;
	    }

	    /* see if we have it all */
	    if ( client->proto_data_length 
		 == esd_proto_map[ client->request ].data_length )
	    {
		ESDBG_TRACE( printf( "(%02d) handling request %d, %s.\n", 
				     client->fd, client->request, 
				     esd_proto_map[ client->request ].description ); );
		client->state = ESD_NEXT_REQUEST; /* handler may override */
		is_ok = esd_proto_map[ client->request ].handler( client );
	    } else {
		ESDBG_TRACE( printf( "(%02d) need more data.\n", client->fd ); );
		is_ok = 1;
	    }

	    break;

	case ESD_CACHING_SAMPLE:
	    is_ok = esd_proto_sample_cache(client);
	    break;

	case ESD_NEXT_REQUEST:

 	    /* make sure there's a request as EOF may return as readable */
	    ESDBG_COMMS( printf( "--------------------------------\n" ); );
	    ESD_READ_INT( client->fd, &client->request, 
			  sizeof(client->request), length, "request" );

	    if ( client->swap_byte_order )
		client->request = swap_endian_32( client->request );

 	    if ( length == 0
		 || ( length < 0 && errno != EAGAIN && errno != EINTR ) )
	    {
 		/* no more data available from that client, close it */
		ESDBG_TRACE( printf( "(%02d) no more protocol requests for client\n", 
				     client->fd ); );
 		is_ok = 0;
		break;
 	    } 
	    else if ( client->request > ESD_PROTO_CONNECT 
		       && client->request < ESD_PROTO_MAX ) 
	    {
		/* set up to read more data */
		client->state = ESD_NEEDS_REQDATA;
		client->proto_data_length = 0;
		/* TODO: do one read, and handle if we get all the data */
		/* this should fix the "all handlers requrie data oddity */
		is_ok = 1;
	        /* if this request requires no additional param data... */
	        if (esd_proto_map[client->request].data_length == 0)
		  {
		    ESDBG_TRACE( printf( "(%02d) handling request %d, %s.\n", 
					client->fd, client->request, 
					esd_proto_map[ client->request ].description ); );
		    client->state = ESD_NEXT_REQUEST; /* handler may override */
		    is_ok = esd_proto_map[ client->request ].handler( client );
		  }
	    } 
	    else {
		ESDBG_TRACE( printf( "(%02d) invalid request: %d\n",
				     client->fd, client->request ); );
		is_ok = 0;
	    }

	    break;

	default:
	    ESDBG_TRACE( printf( "(%02d) invalid state: %d\n",
				 client->fd, client->state ); );
	    is_ok = 0;
	}

	/* if there was a problem, erase the client */
	if ( !is_ok ) {
	    ESDBG_TRACE( printf( "(%02d) error handling request %d, %s.\n", 
				 client->fd, client->request, 
				 esd_proto_map[ client->request ].description ); );
	    erase = client; 
	}

	/* update the iterator before removing */
	client = client->next;
	if ( erase != NULL ) {
	    erase_client( erase );
	    erase = NULL;
	}
    }

    return 1;
}

