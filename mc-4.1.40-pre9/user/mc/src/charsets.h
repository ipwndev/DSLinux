#include <config.h>

#ifdef HAVE_CHARSETS
#ifndef __CHARSETS_H__
#define __CHARSETS_H__

#define UNKNCHAR '\001'

typedef unsigned char uchar;

extern int n_codepages;

extern uchar conv_displ[256];
extern uchar conv_input[256];
extern uchar printable[256];

struct table_entry {
    uchar c;
    int u;
};

struct codepage_desc {
    char *filename;
    char *name;
    struct table_entry *table;
};

extern struct codepage_desc *codepages;

char* load_codepage( int cpindex );
int load_codepages_list ( void );
unsigned char translate_character( int cpfrom, int cpto, unsigned char ch );
int char2unichar( struct table_entry *table, unsigned char c );
char *get_codepage_filename( int n );
int get_codepage_index( char *filename );
int load_codepages_list();
char* init_translation_table( int cpsource, int cpdisplay );
void convert_to_display( char *str );
void convert_from_input( char *str );
void convert_string( uchar *str );

#endif /* __CHARSETS_H__ */
#endif /* HAVE_CHARSETS */
