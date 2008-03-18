#include "esd-server.h"

esd_player_t *esd_filter_list = NULL;

translate_func_t esd_first_filter_func;

/*******************************************************************/
/* send the filter's buffer to it's associated socket, erase if EOF */
int filter_write( void *buffer, int size, esd_format_t format, int rate )
{
    int actual=0, total_data=0, data_size=0;
    esd_player_t *filter=esd_filter_list, *erase=NULL;

    /* if no filters, skip it */
    if( !esd_filter_list ) {
	return size;
    }

    /* translate into the first filter buffer */
    data_size = esd_first_filter_func( filter->data_buffer, 
				       filter->buffer_length, 
				       filter->rate, 
				       filter->format, 
				       buffer, 
				       size, 
				       rate, 
				       format );

    /* hop through the list of filters */
    while( filter ) {
	/* fprintf( stderr, "filter_write: writing to new filter...\n" ); */
	
	/* write the data data_buffer to the socket */
	total_data = 0;
	actual = 0;
	erase = NULL;
	
	/* All of the data from the previous read has to be written.
	 * There's no way I can think of to get around this w/o losing data.
	 * Maybe by using fread and fwrite, to buffer the stuff, but in the 
	 * end, all the data still has to be written from here to the buffer.
	 */
	while( total_data < data_size )
	{
	    ESD_WRITE_BIN( filter->source_id, filter->data_buffer + total_data, 
			   data_size - total_data, actual, "flt wr" );
	    
	    if ( actual <= 0 ) {
		erase = filter;
		total_data = data_size;
	    } else {
		total_data += actual;
	    }
	    /* fprintf( stderr, "filter_write: just wrote %d bytes\n", actual ); */
	}
	
	/* make sure the filter is still alive before we read from it */
	if ( !erase ) {
	    actual = read_player( filter );
	}

	/* translate data to next filter whether current one is alive or not */
	if ( filter->next ) {
	    /* translate to next filter */
	    data_size = filter->translate_func( filter->next->data_buffer, 
						filter->next->buffer_length, 
						filter->next->rate, 
						filter->next->format, 
						filter->data_buffer, 
						filter->buffer_length, 
						filter->rate, 
						filter->format );
	} else {
	    /* translate to audio buffer */
	    data_size = filter->translate_func( buffer, 
						size, 
						rate, 
						format, 
						filter->data_buffer, 
						filter->buffer_length, 
						filter->rate, 
						filter->format );
	}
	
	/* next filter... */
	filter = filter->next;
	
	/* clean up any finished filters */
	if ( erase != NULL ) {
	    erase_filter( erase );
	    erase = NULL;
	}
    }
    
    return data_size;
}

/*******************************************************************/
/* erase a filter from the filter list */
void erase_filter( esd_player_t *filter )
{
    esd_player_t *previous = NULL;
    esd_player_t *current = esd_filter_list;

    /* iterate until we hit a NULL */
    while ( current != NULL )
    {
	/* see if we hit the target filter */
	if ( current == filter ) {
	    if( previous != NULL ){
		/* we are deleting in the middle of the list */
		previous->next = current->next;
	    } else { 
		/* we are deleting the head of the list */
		esd_filter_list = current->next;
	    }

	    /* TODO: delete if needed */
	    free_player( filter );

	    return;
	}

	/* iterate through the list */
	previous = current;
	current = current->next;
    }

    /* hmm, we didn't find the desired filter, just get on with life */
    ESDBG_TRACE( printf( "-%02d- filter not found\n", filter->source_id ); );
    return;
}

