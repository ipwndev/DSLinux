
#include "esd-server.h"
#include <errno.h>

/*******************************************************************/
/* globals */
esd_sample_t *esd_samples_list = NULL;

int esd_playing_samples = 0;
int esd_next_sample_id = 1;	/* sample id = 0 is an error */

/*******************************************************************/
/* prototypes */
void free_sample( esd_sample_t *sample );
void dump_samples(void);

/*******************************************************************/
/* for debugging purposes, dump the list of the clients and data */
void dump_samples(void)
{
    esd_sample_t *sample = esd_samples_list;

    if ( !esdbg_trace ) return;

    while ( sample != NULL ) {
	printf( "<%02d> %s: %d bytes [%p] \n", 
		sample->sample_id, sample->name, 
		sample->sample_length, sample  );
	sample = sample->next;
    }
    return;
}

/*******************************************************************/
/* deallocate memory for the sample */
void free_sample( esd_sample_t *sample )
{
    /* free any memory allocated with the sample */
    free( sample->data_buffer );

    /* free the sample memory itself */
    free( sample );
    ESDBG_TRACE( printf( "<%02d> freed sample: [0x%p]\n", 
			 sample->sample_id, sample ); );
    return;
}

/*******************************************************************/
/* add a complete new client into the list of samples at head */
void add_sample( esd_sample_t *sample )
{
    /* printf ( "adding sample 0x%p\n", new_sample ); */
    if ( !sample ) {
	ESDBG_TRACE( printf( "<NIL> can't add non-existent sample!\n" ); );
	return;
    }

    sample->next = esd_samples_list;
    esd_samples_list = sample;
    return;
}

/*******************************************************************/
/* erase a sample from the sample list */
/* TODO: add "force kill" boolean option */
void erase_sample( int id, int force )
{
    esd_sample_t *previous = NULL;
    esd_sample_t *current = esd_samples_list;

    ESDBG_TRACE( printf( "<%02d> erasing sample\n", id ); );

    /* iterate until we hit a NULL */
    while ( current != NULL )
    {
	/* see if we hit the target sample */
	if ( current->sample_id == id ) {

	    /* if the ref count is non-zero, just flag it for deletion */
	    if ( current->ref_count && !force ) {
		ESDBG_TRACE( printf( "<%02d> erasing sample - deferred\n", id ); );
		current->erase_when_done = 1;
		return;
	    }

	    /* ref_count is zero, get rid of it */
	    if ( previous != NULL ){
		/* we are deleting in the middle of the list */
		previous->next = current->next;
	    } else { 
		/* we are deleting the head of the list */
		esd_samples_list = current->next;
	    }

	    /* erase last traces of sample from existence */
	    free_sample( current );

	    return;
	}

	/* iterate through the list */
	previous = current;
	current = current->next;
    }

    /* hmm, we didn't find the desired sample, just get on with life */
    printf( "<%02d> sample not found\n", id );
    return;
}

/*******************************************************************/
/* allocate and initialize a sample from client stream */
#define min(a,b) ( ( (a)<(b) ) ? (a) : (b) )
int read_sample( esd_sample_t *sample )
{
    int actual = 0, total = sample->cached_length;
    esd_client_t *client = (esd_client_t *) sample->parent;
    short data, *pos;
    short *buffer = (short*) sample->data_buffer;

    ESDBG_COMMS( printf( "{rs} resuming sample cache at %d\n", total ); );

    while ( ( total < sample->sample_length ) && ( actual >= 0 ) ) {
	ESD_READ_BIN( sample->parent->fd, sample->data_buffer + total,
		      min( esd_buf_size_octets, sample->sample_length-total),
		      actual, "rd_samp" );

	if ( actual == 0 
	     || ( actual < 0 && errno != EAGAIN && errno != EINTR ) )
	    return 0;

	if ( actual > 0 ) {
	    /* endian swap multi-byte data if we need to */
	    if ( client->swap_byte_order 
		 && ( (sample->format & ESD_MASK_BITS) == ESD_BITS16 ) )
	    {
		printf( "swapping...\n" );
		for ( pos = buffer + (total / sizeof(short))
			  ; pos < buffer + (actual / sizeof(short))
			  ; pos ++ ) 
		{
		    data = swap_endian_16( (*pos) );
		    *pos = data;
		}
	    }

	    /* irix may return -1 if no data */
	    total += actual; 
	}
    }

    /* TODO: what if total != sample_length ? */
    ESDBG_TRACE( printf( "<%02d> %d bytes total\n", sample->sample_id, total ); );
    sample->cached_length = total;
    return 1;
}

/*******************************************************************/
/* allocate and initialize a sample from client stream */
esd_sample_t *new_sample( esd_client_t *client )
{
    esd_sample_t *sample;
    int client_id, actual;

    /* make sure we have the memory to save the client... */
    sample = (esd_sample_t*) malloc( sizeof(esd_sample_t) );
    if ( sample == NULL ) {
	return NULL;
    }
    
    /* and initialize the sample */
    sample->next = NULL;
    sample->parent = NULL;

    sample->format = *(int*)(client->proto_data);
    sample->rate = *(int*)(client->proto_data + sizeof(int));
    sample->sample_length = *(int*)(client->proto_data + 2 * sizeof(int));

    sample->format = maybe_swap_32( client->swap_byte_order, sample->format );
    sample->rate = maybe_swap_32( client->swap_byte_order, sample->rate );
    sample->sample_length = maybe_swap_32( client->swap_byte_order, 
					   sample->sample_length );

    strncpy( sample->name, client->proto_data + 3 * sizeof(int), ESD_NAME_MAX );
    sample->name[ ESD_NAME_MAX - 1 ] = '\0';

    sample->sample_id = esd_next_sample_id++;

    ESDBG_TRACE( printf( "<%02d> sample %s: 0x%08x at %d Hz\n", sample->sample_id, 
			 sample->name, sample->format, sample->rate ); );

    /* force to an even multiple of 4, do it in the player */
    sample->data_buffer
	= (void *) malloc( sample->sample_length );

    /* if not enough room for data buffer, clean up, and return NULL */
    if ( sample->data_buffer == NULL ) {
	free( sample );
	return NULL;
    }

    /* set ref. count, cached_length, and other housekeeping values */
    sample->cached_length = 0;
    sample->ref_count = 0;
    sample->erase_when_done = 0;
    sample->left_vol_scale = sample->right_vol_scale = ESD_VOLUME_BASE;

    ESDBG_TRACE( printf( "<%02d> sample %s: [0x%p] - %d bytes\n", 
			 sample->sample_id, 
			 sample->name, sample, 
			 sample->sample_length ); );

    client_id = maybe_swap_32( client->swap_byte_order, 
			       sample->sample_id );
    ESD_WRITE_INT( client->fd, &client_id, sizeof(client_id), actual, "ns ack" );

    return sample;
}

/*******************************************************************/
/* find a half-cached sample from client stream */
esd_sample_t *find_caching_sample( esd_client_t *client )
{
    esd_sample_t *sample = esd_samples_list;

    ESDBG_TRACE( printf( "{fs} finding sample [%p]\n", client ); );

    /* iterate until we hit the end */
    while ( sample != NULL )
    {
	/* see if we hit the target sample */
	if ( sample->parent == client ) {
	    ESDBG_TRACE( printf( "<%02d> resuming sample %s: [%p] - %d bytes\n", 
				 sample->sample_id, sample->name, 
				 sample, sample->sample_length ); );
	    break;
	}
    }

    return sample;
}

/*******************************************************************/
/* spawn a player for this sample */
int play_sample( int sample_id, int loop )
{
    esd_player_t *player = NULL;

    player = new_sample_player( sample_id, loop );
    if ( player == NULL )
	return 0;

    add_player( player );
    return 1;
}

/*******************************************************************/
/*  stopa sample from the sample list */
int stop_sample( int id )
{
    esd_player_t *player = esd_players_list;

    ESDBG_TRACE( printf( "{ss} stopping sample <%02d>\n", id ); );

    /* iterate until we hit a NULL */
    while ( player != NULL )
    {
	/* printf( "checking player [0x%p], format = 0x%08x, id = %d\n",
		player, player->format, player->source_id ); */

	/* see if we hit the target sample, and it's really a sample */
	if ( ( ( (player->format) & ESD_MASK_MODE ) == ESD_SAMPLE )
	     && ( player->source_id == id ) ) {

	    /* remove the loop setting on the sample */
	    player->format &= ~ESD_MASK_FUNC;
	    player->format |= ESD_STOP;

	    ESDBG_TRACE( printf( "<%02d> player found, prepared for removal\n", 
				 id ); );
	    
	    return 1;
	}

	/* iterate through the list */
	player = player->next;
    }

    /* hmm, we didn't find the desired sample, just get on with life */
    printf( "{ss} player for sample <%02d> not found\n", id );
    return 0;
}

