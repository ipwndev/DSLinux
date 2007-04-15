#include <config.h>

#ifdef HAVE_CHARSETS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "charsets.h"

int n_codepages = 0;

struct codepage_desc *codepages;

unsigned char conv_displ[256];
unsigned char conv_input[256];
unsigned char printable[256];

static void xstrncpy( char *dest, const char *src, int n )
{
    strncpy( dest, src, n );
    dest[n] = '\0';
}

int load_codepages_list ( void )
{
    int result = -1;
    FILE *f;
    char buf[256];

    strcpy( buf, LIBDIR "codepages/Index.txt" );
    if ( !( f = fopen( buf, "r" ) ) )
	return -1;

    for ( n_codepages=0; fgets( buf, 256, f ); )
	if ( buf[0] != '\n' && buf[0] != '\0' )
	    ++n_codepages;
    rewind( f );

    codepages = calloc( n_codepages + 1, sizeof(struct codepage_desc) );

    for( n_codepages = 0; fgets( buf, 256, f ); ) {
	// split string into filename and cpname
	char *p = buf;
	int buflen = strlen( buf );
	if ( *p == '\n' || *p == '\0' )
	    continue;

	if ( buflen > 0 && buf[ buflen - 1 ] == '\n' )
	    buf[ buflen - 1 ] = '\0';
	while ( *p != '\t' && *p != ' ' && *p != '\0' )
	    ++p;
	if ( *p == '\0' )
	    goto fail;

	codepages[n_codepages].filename = malloc( p - buf + 1 );
	xstrncpy( codepages[n_codepages].filename, buf, p - buf );

	while ( *p == '\t' || *p == ' ' )
	    ++p;
	if ( *p == '\0' )
	    goto fail;

	codepages[n_codepages].name = malloc( strlen(p) + 1 );
	strcpy( codepages[n_codepages].name, p );
	++n_codepages;
    }

    result = n_codepages;
fail:
    fclose( f );
    return result;
}

#define OTHER_8BIT "Other_8_bit"

char *get_codepage_filename( int n )
{
    return (n < 0) ? OTHER_8BIT : codepages[ n ].filename;
}

int get_codepage_index( char *filename )
{
    int i;
    if (strcmp( filename, OTHER_8BIT ) == 0)
	return -1;
    for ( i=0; codepages[ i ].filename; ++i )
	if (strcmp( filename, codepages[ i ].filename ) == 0)
	    return i;
    return -1;
}

char errbuf[255];

char* load_codepage( int cpindex )
{
    int n;
    FILE *f;
    char buf[256];
    struct table_entry *table
	= malloc( (128 + 1) * sizeof(struct table_entry) );
    
    strcpy( buf, LIBDIR "codepages/" );
    strcat( buf, codepages[ cpindex ].filename );
    strcat( buf, ".cp" );
    if ( !( f = fopen( buf, "r" ) ) ) {
	sprintf( errbuf, "Error loading codepage from %s!", buf );
	return errbuf;
    }

    for( n = 0; fgets( buf, 256, f ) && n < 128; ) {
	char *p, *pp;
	unsigned char c;
	int u;
	c = strtoul( buf, &p, 0 );
	if (p == buf || c < 128) continue;
	u = strtoul( p, &pp, 0 );
	if (pp == p) continue;

	table[ n ].c = c;
	table[ n ].u = u;
	++n;
    }
    table[ n ].c = 0;
    table[ n ].u = 0;

    codepages[ cpindex ].table = table;
    
    fclose( f );
    return NULL;
}

int char2unichar( struct table_entry *table, unsigned char c )
{
    int i;
    for ( i = 0; table[i].c; ++i )
	if ( table[i].c == c )
	    return table[i].u;
    return -1;
}

unsigned char unichar2char( struct table_entry *table, int u )
{
    int i;
    if (u == -1)
	return UNKNCHAR;
    for ( i = 0; table[i].u; ++i )
	if ( table[i].u == u )
	    return table[i].c;
    return UNKNCHAR;
}

unsigned char translate_character( int cpfrom, int cpto, unsigned char ch )
{
    struct table_entry *fromtable = codepages[ cpfrom ].table;
    struct table_entry *totable = codepages[ cpto ].table;
    int u = char2unichar( fromtable, ch );
    return unichar2char( totable, u );
}

char* init_translation_table( int cpsource, int cpdisplay )
{
    int i;
    char *errmsg = NULL;

    if (cpsource >= 0 && codepages[ cpsource ].table == NULL) {
        errmsg = load_codepage( cpsource );
	if (errmsg) return errmsg;
    }
    if (cpdisplay >= 0 && codepages[ cpdisplay ].table == NULL) {
	errmsg = load_codepage( cpdisplay );
	if (errmsg) return errmsg;
    }
    
    if (cpsource < 0 || cpdisplay < 0) {
	for (i=0; i<=255; ++i) {
	    conv_displ[i] = i;
	    conv_input[i] = i;
	}
    } else {
	for (i=0; i<=127; ++i) {
	    conv_displ[i] = i;
	    conv_input[i] = i;
	}
	for (i=128; i<=255; ++i)
	    conv_displ[i] = translate_character( cpsource, cpdisplay, i );
	for (i=128; i<=255; ++i) {
	    unsigned char ch = translate_character( cpdisplay, cpsource, i );
	    conv_input[i] = (ch == UNKNCHAR) ? i : ch;
	}
    }
    // Fill printable characters table
    for (i=0; i<=127; ++i)
	printable[i] = (i > 31 && i != 127);
	
    if (cpdisplay < 0) {
	for (i=128; i<=255; ++i)
	    printable[i] = i != 155;
    } else {
	struct table_entry *inptable = codepages[ cpdisplay ].table;
	for (i=128; i<=255; ++i)
	    printable[i] = char2unichar( inptable, i ) != -1;
    }
    return NULL;
}

void convert_to_display( char *str )
{
    while ( (*str++ = conv_displ[ (unsigned char) *str ]) ) ;
}


void convert_from_input( char *str )
{
    while ( (*str++ = conv_input[ (unsigned char) *str ]) ) ;
}

#endif /* HAVE_CHARSETS */
