
#include <esd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/*********************************************************************/
/* print server into to stdout */
void esd_print_server_info( esd_server_info_t *server_info )
{
    printf( "server version = %d\n", server_info->version );
    printf( "server format  = 0x%08x\n", server_info->format );
    printf( "server rate    = %d\n", server_info->rate );
    return;
}

/*********************************************************************/
/* print player into to stdout */
void esd_print_player_info( esd_player_info_t *player_info )
{
    printf( "player %d name    = %s\n", 
	    player_info->source_id, player_info->name );
    printf( "player %d format  = 0x%08x\n", 
	    player_info->source_id, player_info->format );
    printf( "player %d rate    = %d\n", 
	    player_info->source_id, player_info->rate );
    printf( "player %d left    = %d\n", 
	    player_info->source_id, player_info->left_vol_scale );
    printf( "player %d right   = %d\n", 
	    player_info->source_id, player_info->right_vol_scale );
    return;
}

/*********************************************************************/
/* print server into to stdout */
void esd_print_sample_info( esd_sample_info_t *sample_info )
{
    printf( "sample %d name    = %s\n", 
	    sample_info->sample_id, sample_info->name );
    printf( "sample %d format  = 0x%08x\n", 
	    sample_info->sample_id, sample_info->format );
    printf( "sample %d rate    = %d\n", 
	    sample_info->sample_id, sample_info->rate );
    printf( "sample %d left    = %d\n", 
	    sample_info->sample_id, sample_info->left_vol_scale );
    printf( "sample %d right   = %d\n", 
	    sample_info->sample_id, sample_info->right_vol_scale );
    printf( "sample %d length  = %d\n", 
	    sample_info->sample_id, sample_info->length );
    return;
}

/*********************************************************************/
/* print all info to stdout */
void esd_print_all_info( esd_info_t *all_info )
{
    esd_player_info_t *player_info;
    esd_sample_info_t *sample_info;

    /* dump server info */
    esd_print_server_info( all_info->server );

    /* dump player info */
    for ( player_info = all_info->player_list 
	      ; player_info != NULL ; player_info = player_info->next )
    {
	esd_print_player_info( player_info );
    }

    /* dump sample info */
    for ( sample_info = all_info->sample_list 
	      ; sample_info != NULL ; sample_info = sample_info->next )
    {
	esd_print_sample_info( sample_info );
    }

    return;
}

/*********************************************************************/
/* retrieve server properties (sample rate, format, version number) */
esd_server_info_t *esd_get_server_info( int esd )
{
    int proto = ESD_PROTO_SERVER_INFO;
    int version = 0;

    /* allocate the server info structure */
    esd_server_info_t *server_info 
	= (esd_server_info_t *) malloc( sizeof(esd_server_info_t) );
    if ( !server_info ) return server_info;

    /* tell the server to cough up the info */
    write( esd, &proto, sizeof(proto) );
    if ( write( esd, &version, sizeof(version) ) != sizeof(version) ) {
	free( server_info );
	return NULL;
    }

    /* get the info from the server */
    read( esd, &server_info->version, sizeof(server_info->version) );
    read( esd, &server_info->rate, sizeof(server_info->rate) );
    if ( read( esd, &server_info->format, sizeof(server_info->format) )
	 != sizeof(server_info->format) ) {
	free( server_info );
	return NULL;
    }

    return server_info;
}

/*********************************************************************/
/* release all memory allocated for the server properties structure */
void esd_free_server_info( esd_server_info_t *server_info )
{
    free( server_info );
    return;
}

/*********************************************************************/
/* retrieve all information from server */
esd_info_t *esd_get_all_info( int esd )
{
    int proto = ESD_PROTO_ALL_INFO;
    int version = 0;
    esd_server_info_t *server_info;
    esd_player_info_t *player_info;
    esd_sample_info_t *sample_info;

    /* allocate the entire info structure, and set defaults to NULL */
    esd_info_t *info = (esd_info_t *) malloc( sizeof(esd_info_t) );
    info->player_list = NULL;
    info->sample_list = NULL;

    /* allocate the server info structure */
    server_info = (esd_server_info_t *) malloc( sizeof(esd_server_info_t) );
    if ( !server_info ) {
        esd_free_all_info( info );
        return NULL;
    }

    /* tell the server to cough up the info */
    write( esd, &proto, sizeof(proto) );
    if ( write( esd, &version, sizeof(version) ) != sizeof(version) ) {
	esd_free_all_info( info );
	free( server_info );
	return NULL;
    }

    /* get the server info */
    read( esd, &server_info->version, sizeof(server_info->version) );
    read( esd, &server_info->rate, sizeof(server_info->rate) );
    if ( read( esd, &server_info->format, sizeof(server_info->format) )
	 != sizeof(server_info->format) ) {
	esd_free_all_info( info );
	free( server_info );
	return NULL;
    }
    info->server = server_info;

    /* get the player info */
    do 
    {
	player_info = (esd_player_info_t *) malloc( sizeof(esd_player_info_t) );
	if ( !player_info ) {
	    esd_free_all_info( info );
	    return NULL;
	}

	read( esd, &player_info->source_id, sizeof(player_info->source_id) );
	read( esd, &player_info->name, ESD_NAME_MAX );
	player_info->name[ ESD_NAME_MAX - 1 ] = '\0';
	read( esd, &player_info->rate, sizeof(player_info->rate) );
	read( esd, &player_info->left_vol_scale, sizeof(player_info->left_vol_scale) );
	read( esd, &player_info->right_vol_scale, sizeof(player_info->right_vol_scale) );
	if ( read( esd, &player_info->format, sizeof(player_info->format) )
	     != sizeof(player_info->format) ) {
	    free( player_info );
	    esd_free_all_info( info );	    
	    return NULL;
	}
	
	if ( player_info->source_id > 0 ) {
	    player_info->next = info->player_list;
	    info->player_list = player_info;
	}

    } while( player_info->source_id > 0 );

    free( player_info );

    /* get the sample info */
    do 
    {
	sample_info = (esd_sample_info_t *) malloc( sizeof(esd_sample_info_t) );
	if ( !sample_info ) {
	    esd_free_all_info( info );
	    return NULL;
	}

	read( esd, &sample_info->sample_id, sizeof(sample_info->sample_id) );
	read( esd, &sample_info->name, ESD_NAME_MAX );
	sample_info->name[ ESD_NAME_MAX - 1 ] = '\0';
	read( esd, &sample_info->rate, sizeof(sample_info->rate) );
	read( esd, &sample_info->left_vol_scale, sizeof(sample_info->left_vol_scale) );
	read( esd, &sample_info->right_vol_scale, sizeof(sample_info->right_vol_scale) );
	read( esd, &sample_info->format, sizeof(sample_info->format) );
	if ( read( esd, &sample_info->length, sizeof(sample_info->length) )
	     != sizeof(sample_info->length) ) {
	    free( sample_info );
	    esd_free_all_info( info );	    
	    return NULL;
	}
	
	if ( sample_info->sample_id > 0 ) {
	    sample_info->next = info->sample_list;
	    info->sample_list = sample_info;
	}

    } while( sample_info->sample_id > 0 );

    free( sample_info );

    return info;
}

/*********************************************************************/
/* retrieve all information from server, and update until unsubsribed or closed */
esd_info_t *esd_subscribe_all_info( int esd )
{
    fprintf( stderr, "- esd_subscribe_all_info: not yet implemented!\n" );
    if(esd) {
	fprintf(stderr, "but you passed me something anyways\n");
    }
    return NULL;
}

/*********************************************************************/
/* call to update the info structure with new information, and call callbacks */
esd_info_t *esd_update_info( int esd, esd_info_t *info, 
			     esd_update_info_callbacks_t *callbacks )
{
    fprintf( stderr, "- esd_update_info: not yet implemented!\n" );
    if(esd)
	if(info) 
	    if(callbacks)
		fprintf(stderr, "but you passed me something anyways\n");
    return NULL;
}

/*********************************************************************/
/* call to update the info structure with new information, and call callbacks */
esd_info_t *esd_unsubscribe_info( int esd )
{
    fprintf( stderr, "- esd_unsubscribe_info: not yet implemented!\n" );
	if(esd)
		fprintf(stderr, "but you passed me something anyways\n");
    return NULL;
}

/*********************************************************************/
/* release all memory allocated for the esd info structure */
void esd_free_all_info( esd_info_t *info )
{
    esd_player_info_t *player_info, *next_player_info;
    esd_sample_info_t *sample_info, *next_sample_info;

    free( info->server );

    player_info = info->player_list;
    while ( player_info != NULL ) {
	next_player_info = player_info->next;
	free( player_info );
	player_info = next_player_info;
    }

    sample_info = info->sample_list;
    while ( sample_info != NULL ) {
	next_sample_info = sample_info->next;
	free( sample_info );
	sample_info = next_sample_info;
    }

    free( info );
    return;
}


/* reset the volume panning for a stream */
int esd_set_stream_pan( int esd, int stream_id, 
			int left_scale, int right_scale )
{
    int ok;
    int proto = ESD_PROTO_STREAM_PAN;

    /* send the stream panning information */
    if ( write( esd, &proto, sizeof(proto) ) != sizeof(proto) )
	return -1;
    if ( write( esd, &stream_id, sizeof(stream_id) ) != sizeof(stream_id) )
	return -1;
    if ( write( esd, &left_scale, sizeof(left_scale) ) != sizeof(left_scale) )
	return -1;
    if ( write( esd, &right_scale, sizeof(right_scale) ) != sizeof(right_scale) )
	return -1;

    /* flush the socket */
    /* fsync( sock ); */
    
    /* read the status back from the server */
    if ( read( esd, &ok, sizeof(ok) ) != sizeof(ok) )
	return -1;

    return ok;
}

/* reset the default volume panning for a sample */
int esd_set_default_sample_pan( int esd, int sample_id, 
				int left_scale, int right_scale )
{
    int ok;
    int proto = ESD_PROTO_SAMPLE_PAN;

    /* send the stream panning information */
    if ( write( esd, &proto, sizeof(proto) ) != sizeof(proto) )
	return -1;
    if ( write( esd, &sample_id, sizeof(sample_id) ) != sizeof(sample_id) )
	return -1;
    if ( write( esd, &left_scale, sizeof(left_scale) ) != sizeof(left_scale) )
	return -1;
    if ( write( esd, &right_scale, sizeof(right_scale) ) != sizeof(right_scale) )
	return -1;

    /* flush the socket */
    /* fsync( sock ); */
    
    /* read the status back from the server */
    if ( read( esd, &ok, sizeof(ok) ) != sizeof(ok) )
	return -1;

    return ok;
}

/* see if the server is in stnaby, autostandby, etc */
esd_standby_mode_t esd_get_standby_mode( int esd )
{
    int proto = ESD_PROTO_STANDBY_MODE, mode = ESM_ERROR, version = 0;

    /* diagnostic info */
    /*
    if ( getenv( "ESDBG" ) )
	printf( "esound getting standby mode\n" );
    */

    write( esd, &proto, sizeof(proto) );
    if ( write( esd, &version, sizeof(version) ) != sizeof(version) ) {
	return ESM_ERROR;
    }

    if ( read( esd, &mode, sizeof(mode) ) != sizeof(mode) )
	return ESM_ERROR;

    return mode;
}
