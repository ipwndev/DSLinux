
#include "esd-server.h"
#include <errno.h>

/*******************************************************************/
/* globals */

esd_player_t *esd_players_list = NULL; 
esd_player_t *esd_recorder_list = NULL;
esd_player_t *esd_monitor_list = NULL;

/*******************************************************************/
/* for debugging purposes, dump the list of the clients and data */
void dump_players(void)
{
    esd_player_t *player = esd_players_list;

    if ( !esdbg_trace ) return;

    while ( player != NULL ) {
	printf( "-%02d- player: [%p]\n", 
		player->source_id, player );
	player = player->next;
    }
    return;
}

/*******************************************************************/
/* deallocate memory for the player */
void free_player( esd_player_t *player )
{
    esd_sample_t *sample;

    /* see if we need to do any housekeeping */
    if ( ( player->format & ESD_MASK_MODE ) == ESD_STREAM ) {
	/* TODO: erase client should be independent of players */
	erase_client( player->parent );
    } else if ( ( player->format & ESD_MASK_MODE ) == ESD_SAMPLE ) {
	sample = (esd_sample_t *) (player->parent);
	sample->ref_count--;
	esd_playing_samples--;

	ESDBG_TRACE( printf( "<%02d> free player: [%p] refs=%d erase?=%d samps=%d\n", 
			     player->source_id, player, sample->ref_count, 
			     sample->erase_when_done, esd_playing_samples ); );

	if ( sample->erase_when_done && !sample->ref_count ) {
	    ESDBG_TRACE( printf( "<%02d> free_player: erasing sample\n", 
				 sample->sample_id ); );

	    erase_sample( sample->sample_id, 0 );
	}
    }

    /* free any memory allocated with the player */
    free( player->data_buffer );

    /* free the player memory itself */
    free( player );

    return;
}

/*******************************************************************/
/* add a complete new player into the list of players at head */
void add_player( esd_player_t *player )
{
    /* printf ( "adding player %p\n", new_player ); */
    if ( !player ) {
	ESDBG_TRACE( printf( "<NIL> can't add non-existent player!\n" ); );
	return;
    }

    player->next = esd_players_list;
    esd_players_list = player;

    return;
}

/*******************************************************************/
/* erase a player from the player list */
void erase_player( esd_player_t *player )
{
    esd_player_t *previous = NULL;
    esd_player_t *current = esd_players_list;

    /* iterate until we hit a NULL */
    while ( current != NULL )
    {
	/* see if we hit the target player */
	if ( current == player ) {
	    if( previous != NULL ){
		/* we are deleting in the middle of the list */
		previous->next = current->next;
	    } else { 
		/* we are deleting the head of the list */
		esd_players_list = current->next;
	    }

	    /* TODO: delete if needed */

	    free_player( player );

	    return;
	}

	/* iterate through the list */
	previous = current;
	current = current->next;
    }

    /* hmm, we didn't find the desired player, just get on with life */
    ESDBG_TRACE( printf( "-%02d- player not found\n", player->source_id ); );
    return;
}


/*******************************************************************/
/* add a complete new recorder into the list of recorders at head */
void add_recorder( esd_player_t *recorder )
{
    /* printf ( "adding player %p\n", new_player ); */
    if ( !recorder ) {
	ESDBG_TRACE( printf( "<NIL> can't add non-existent recorder!\n" ); );
	return;
    }

    recorder->next = esd_recorder_list;
    esd_recorder_list = recorder;

    return;
}

/*******************************************************************/
/* erase a recorder from the recorder list */
void erase_recorder( esd_player_t *recorder )
{
    esd_player_t **prevp = &esd_recorder_list;
    esd_player_t *current = esd_recorder_list;

    /* iterate until we hit a NULL */
    while ( current != NULL )
    {
	/* see if we hit the target player */
	if ( current == recorder ) {
	    *prevp = current->next;

	    /* TODO: delete if needed */

	    free_player( recorder );

	    return;
	}

	/* iterate through the list */
	prevp = &current->next;
	current = current->next;
    }

    /* hmm, we didn't find the desired recorder, just get on with life */
    ESDBG_TRACE( printf( "-%02d- recorder not found\n", recorder->source_id ); );
    return;
}


/*******************************************************************/
/* erase a monitor from the monitor list */
void erase_monitor( esd_player_t *monitor )
{
    esd_player_t *previous = NULL;
    esd_player_t *current = esd_monitor_list;

    /* iterate until we hit a NULL */
    while ( current != NULL )
    {
	/* see if we hit the target monitor */
	if ( current == monitor ) {
	    if( previous != NULL ){
		/* we are deleting in the middle of the list */
		previous->next = current->next;
	    } else { 
		/* we are deleting the head of the list */
		esd_monitor_list = current->next;
	    }

	    /* TODO: delete if needed */
	    free_player( monitor );

	    return;
	}

	/* iterate through the list */
	previous = current;
	current = current->next;
    }

    /* hmm, we didn't find the desired monitor, just get on with life */
    ESDBG_TRACE( printf( "-%02d- monitor not found\n", monitor->source_id ); );
    return;
}


/*******************************************************************/
/* write block of data from client, return < 0 to have it erased */
int write_player( esd_player_t *player, void *src_buffer, int src_length, 
		  int src_rate, int src_format )
{
    fd_set wr_fds;
    fd_set rd_fds;
    int length = 0, actual = 0, ready = 0;
    struct timeval timeout;
    char message[ 100 ];
    unsigned short data, *pos; /* used for swapping */
    esd_client_t *client;
    unsigned short *buffer;

    /* use select to prevent blocking clients that are ready */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    FD_ZERO( &wr_fds );
    FD_ZERO( &rd_fds );
    FD_SET( player->source_id, &wr_fds );
    /* EOF is only indicated in the read set */
    FD_SET( player->source_id, &rd_fds );
    
    /* if the data is ready, read a block */
    ready = select( player->source_id + 1, 
			&rd_fds, &wr_fds, NULL, &timeout ) ;
    if ( ready > 0 )
    {
	char ch;
	if ( FD_ISSET(player->source_id, &rd_fds)
	     && (recv(player->source_id, &ch, 1, MSG_PEEK) < 1) ) {
	    /* Error or EOF */
	    return -1;
	}

	/* translate the data */
	length = player->translate_func( player->data_buffer, 
					 player->buffer_length, 
					 player->rate, 
					 player->format, 
					 src_buffer, 
					 src_length, 
					 src_rate, 
					 src_format );

	/* endian swap multi-byte data if we need to */
	client = (esd_client_t *) (player->parent);
	if ( client->swap_byte_order 
	     && ( (player->format & ESD_MASK_BITS) == ESD_BITS16 ) )
	{
	    buffer = (unsigned short*) player->data_buffer;
	    for ( pos = buffer 
		      ; pos < buffer + length / sizeof(unsigned short)
		      ; pos ++ )
	    {
		data = swap_endian_16( (*pos) );
		*pos = data;
	    }
	}

	/* write out the data */
	ESD_WRITE_BIN( player->source_id, player->data_buffer, 
		       player->buffer_length, length, "str rd" );

    } else if ( ready < 0 ) {
	sprintf( message, "error writing client (%d)\n", 
		 player->source_id );
	perror( message );
	return -1;
    }

    /* check for end of stream */
    if ( length < 0 ) return -1;
    if ( length == 0 ) return 0;
 
    return actual;
}

/*******************************************************************/
/* read block of data from client, return < 0 to have it erased */
int read_player( esd_player_t *player )
{
    fd_set rd_fds;
    int actual = 0, actual_2nd = 0, can_read = 0;
    struct timeval timeout;
    char message[ 100 ];
    unsigned short data, *pos; /* used for swapping */
    esd_client_t *client;
    unsigned short *buffer;

    switch( player->format & ESD_MASK_MODE ) {
    case ESD_STREAM:
	/* use select to prevent blocking clients that are ready */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO( &rd_fds );
	FD_SET( player->source_id, &rd_fds );

	/* if the data is ready, read a block */
	can_read = select( player->source_id + 1, 
			   &rd_fds, NULL, NULL, &timeout ) ;
	if ( can_read > 0 )
	{
	    player->actual_length = 0;
	    do {
		ESD_READ_BIN( player->source_id,
			      player->data_buffer + player->actual_length, 
			      player->buffer_length - player->actual_length,
			      actual, "str rd" );
		/* check for end of stream */
		if ( actual == 0 
		     || ( actual < 0 && errno != EAGAIN && errno != EINTR ) )
		    return -1;

		if (actual < player->buffer_length - player->actual_length)
			break;
		
		/* more data, save how much we got */
		if ( actual > 0 )
		   player->actual_length += actual;
	    } while (player->actual_length < player->buffer_length);

	    /* endian swap multi-byte data if we need to */
	    client = (esd_client_t *) (player->parent);
	    if ( client->swap_byte_order 
		 && ( (player->format & ESD_MASK_BITS) == ESD_BITS16 ) )
	    {
		buffer = (unsigned short*) player->data_buffer;
		for ( pos = buffer 
			  ; pos < buffer + player->actual_length / sizeof(short)
			  ; pos ++ )
		{
		    data = swap_endian_16( (*pos) );
		    *pos = data;
		}
	    }

	} else if ( can_read < 0 ) {
	    sprintf( message, "error reading client (%d)\n", 
		     player->source_id );
	    perror( message );
	    return -1;
	}

	break;

    case ESD_SAMPLE:

	/* printf( "player [%p], pos = %d, format = 0x%08x\n", 
		player, player->last_pos, player->format ); */
	
	/* only keep going if we didn't want to stop looping */
	if ( ( player->last_pos ) == 0 &&
	    ( ( ((esd_sample_t*)player->parent)->format & ESD_MASK_FUNC ) 
	      == ESD_STOP ) ) 
	{
	    return -1;
	}

	/* copy the data from the sample to the player */
	actual = ( ((esd_sample_t*)player->parent)->sample_length 
		   - player->last_pos > player->buffer_length )
	    ? player->buffer_length 
	    : ((esd_sample_t*)player->parent)->sample_length - player->last_pos;
	if ( actual > 0 ) {
	    memcpy( player->data_buffer, 
		    ((esd_sample_t*)player->parent)->data_buffer 
		    + player->last_pos, actual );
	    player->last_pos += actual;
	    if ( ( player->format & ESD_MASK_FUNC ) != ESD_LOOP ) {
		/* we're done for this iteration */
		break;
	    }
	} else {
	    /* something horrible has happened to the sample */
	    return -1;
	}

	/* we are looping, see if we need to copy another block */
	if ( player->last_pos >= ((esd_sample_t*)player->parent)->sample_length ) {
	    player->last_pos = 0;
	}

	actual_2nd = ( ((esd_sample_t*)player->parent)->sample_length 
		   - player->last_pos > player->buffer_length - actual )
	    ? player->buffer_length - actual
	    : ((esd_sample_t*)player->parent)->sample_length - player->last_pos;
	if ( actual_2nd >= 0 ) {
	    /* only keep going if we didn't want to stop looping */
	    if ( ( ((esd_sample_t*)player->parent)->format & ESD_MASK_FUNC )
		 != ESD_STOP ) {
		memcpy( player->data_buffer + actual, 
			((esd_sample_t*)player->parent)->data_buffer 
			+ player->last_pos, actual_2nd );
		player->last_pos += actual_2nd;
		actual += actual_2nd;

		/* make sure we're not at the end */
		if ( player->last_pos >= ((esd_sample_t*)player->parent)->sample_length ) {
		    player->last_pos = 0;
		}
	    }

	} else {
	    /* something horrible has happened to the sample */
	    return -1;
	}

	/* sample data is swapped as it's cached, no swap needed here */
	break;

    default:
	ESDBG_TRACE( printf( "-%02d- read_player: format 0x%08x not supported\n", 
			     player->source_id, player->format ); );
	return -1;
    }

    return actual;
}

/*******************************************************************/
/* send the players buffer to it's associated socket, erase if EOF */
void monitor_write( void *output_buffer, int length ) {
    fd_set wr_fds;
    int can_write;
    struct timeval timeout;
    esd_player_t *monitor, *remove = NULL;

    /* make sure we have a monitor connected */
    if ( !esd_monitor_list ) 
	return;

    /* shuffle through the list of monitors */
    monitor = esd_monitor_list;
    while ( monitor != NULL ) {

	/* see if we can write to the socket */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO( &wr_fds );
	FD_SET( monitor->source_id, &wr_fds );
	can_write = select( monitor->source_id + 1, NULL, &wr_fds, NULL, &timeout );
	if ( can_write > 0) 
	{

	/* mix down the monitor's buffer */
	length = monitor->translate_func( monitor->data_buffer, 
					  monitor->buffer_length,
					  monitor->rate, 
					  monitor->format, 
					  output_buffer, 
					  length, 
					  esd_audio_rate,
					  esd_audio_format );

	/* write the data buffer to the socket */
	ESD_WRITE_BIN( monitor->source_id, monitor->data_buffer, 
		       monitor->buffer_length, length, "mon wr" );

	if ( length < 0 ) {
	    /* error on write, close it down */
	    ESDBG_TRACE( printf( "(%02d) closing monitor\n", monitor->source_id ); );
	    remove = monitor;
	}
	}

	monitor = monitor->next;

	if ( remove ) {
	    erase_client( remove->parent );
	    erase_monitor( remove );
	    remove = NULL;
	}
    }
    
    return;
}

int recorder_write( void *buffer, int length ) {

    esd_player_t *recorder, *remove = NULL;
    int wrote_len = 0;
    
    recorder = esd_recorder_list;

    while (recorder != NULL) {
	/* write it out */
	ESDBG_TRACE( printf( "(%02d) writing recorder data\n", 
			recorder->source_id ); );
	wrote_len = write_player(recorder, buffer,  length, 
			esd_audio_rate, esd_audio_format);
	
	/* see how it went */
	if ( wrote_len < 0 ) {
		/* couldn't send anything, close it down */
		ESDBG_TRACE( printf( "(%02d) closing recorder\n", 
			recorder->source_id ); );
		
		remove = recorder;
	}

	recorder = recorder->next;

	if ( remove ) {
		erase_client( remove->parent );
		erase_recorder( remove );
		remove = NULL;
	}
    }
    
    if (!esd_recorder_list) {
	/* stop recording */
	esd_audio_close();
	esd_audio_format &= ~ESD_RECORD;
	esd_audio_open();
    }
    
    return wrote_len;
}

/*******************************************************************/
/* allocate and initialize a player from client stream */
esd_player_t *new_stream_player( esd_client_t *client )
{
    esd_player_t *player;

    /* make sure we have the memory to save the client... */
    player = (esd_player_t*) malloc( sizeof(esd_player_t) );
    if ( player == NULL ) {
	return NULL;
    }
    
    /* and initialize the player */
    player->next = NULL;
    player->parent = NULL;

    player->format = *(int*)(client->proto_data);
    player->rate = *(int*)(client->proto_data + sizeof(int));

    player->format = maybe_swap_32( client->swap_byte_order, player->format );
    player->format &= ~ESD_MASK_MODE; /* force to ESD_STREAM */
    player->rate = maybe_swap_32( client->swap_byte_order, player->rate );

    player->source_id = client->fd;
    strncpy( player->name, client->proto_data + 2 * sizeof(int), ESD_NAME_MAX );
    player->name[ ESD_NAME_MAX - 1 ] = '\0';

    ESDBG_TRACE( printf( "(%02d) stream %s: 0x%08x at %d Hz\n", client ->fd, 
			 player->name, player->format, player->rate ); );

    /* Reduce buffers on sockets to the minimum needed */
    esd_set_socket_buffers( player->source_id, player->format, 
			    player->rate, esd_audio_rate );

    /* calculate buffer length to match the mix buffer duration */
    player->buffer_length = esd_buf_size_octets * player->rate / esd_audio_rate;
    if ( (player->format & ESD_MASK_BITS) == ESD_BITS8 )
	player->buffer_length /= 2;
    if ( (player->format & ESD_MASK_CHAN) == ESD_MONO )
	player->buffer_length /= 2;

    /* force to an even multiple of 4 */
    player->buffer_length += ( 4 - (player->buffer_length % 4) ) % 4;

    player->data_buffer
	= (void *) malloc( player->buffer_length );

    /* if not enough room for data buffer, clean up, and return NULL */
    if ( player->data_buffer == NULL ) {
	free( player );
	return NULL;
    }

    /* everything's ok, set the easy stuff */
    player->left_vol_scale = player->right_vol_scale = ESD_VOLUME_BASE;
    player->mix_func = get_mix_func( player );
    player->translate_func = NULL; /* no translating, just mixing */

    ESDBG_TRACE( printf( "(%02d) player: [%p]\n", player->source_id, player ); );

    return player;
}

/*******************************************************************/
/* allocate and initialize a player from client stream */
esd_player_t *new_sample_player( int sample_id, int loop )
{
    esd_player_t *player;
    esd_sample_t *sample;

    /* find the sample we want to play */
    for ( sample = esd_samples_list ; sample != NULL 
	      ; sample = sample->next )
    {
	if ( sample->sample_id == sample_id ) {
	    break;
	}
    }
    /* if we didn't find it, return NULL */
    if ( sample == NULL ) {
	return NULL;
    }
        
    /* make sure we have the memory to save the player... */
    player = (esd_player_t*) malloc( sizeof(esd_player_t) );
    if ( player == NULL ) {
	return NULL;
    }
    
    /* and initialize the player */
    player->next = NULL;
    player->parent = sample;
    player->format = sample->format & ~ESD_MASK_MODE;
    player->format |= ESD_SAMPLE;
    if ( loop ) {
	player->format &= ~ESD_MASK_FUNC;
	player->format |= ESD_LOOP;
    }
    player->rate = sample->rate;
    player->source_id = sample->sample_id;
    player->left_vol_scale = sample->left_vol_scale;
    player->right_vol_scale = sample->right_vol_scale;

    ESDBG_TRACE( printf( "<%02d> connection format: 0x%08x at %d Hz\n", 
			 player->source_id, player->format, player->rate ); );

    /* calculate buffer length to match the mix buffer duration */
    player->buffer_length = esd_buf_size_octets * player->rate / esd_audio_rate;
    if ( (player->format & ESD_MASK_BITS) == ESD_BITS8 )
	player->buffer_length /= 2;
    if ( (player->format & ESD_MASK_CHAN) == ESD_MONO )
	player->buffer_length /= 2;

    /* force to an even multiple of 4 */
    player->buffer_length += ( 4 - (player->buffer_length % 4) ) % 4;

    player->data_buffer
	= (void *) malloc( player->buffer_length );

    /* if not enough room for data buffer, clean up, and return NULL */
    if ( player->data_buffer == NULL ) {
	free( player );
	return NULL;
    }

    /* update housekeeping values */
    esd_playing_samples++;
    player->last_pos = 0;
    sample->ref_count++;
    player->mix_func = get_mix_func( player );
    player->translate_func = NULL; /* no translating, just mixing */

    ESDBG_TRACE( printf( "<%02d> new player: refs=%d samps=%d [%p]\n", 
			 player->source_id, sample->ref_count, 
			 esd_playing_samples, player ); );

    /* everything's ok, return the allocated player */
    return player;
}
