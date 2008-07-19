/*
 *	Various utilities
 *	
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *		(C) 2001 Marek 'Marx' Grac
 *
 *	This file is part of JOE (Joe's Own Editor)
 */
#ifndef _JOE_UTILS_H
#define _JOE_UTILS_H 1

/* Destructors */

#define AUTO_DESTRUCT GC *gc = 0;

typedef struct gc GC;

struct gc {
	struct gc *next;	/* List */
	void **var;		/* Address of pointer variable */
	void (*rm)(void *val);	/* Destructor which takes pointer variable */
};

/* Add a variable to GC list */

void gc_add PARAMS((GC **gc, void **var, void (*rm)(void *val)));

/* Call destructors */

void gc_collect PARAMS((GC **gc));

/* Version of return which calls destructors before returning */

#define RETURN(val) do { \
	if (gc) gc_collect(&gc); \
	return (val); \
	} while(0)

/* Pool allocation functions using singly-linked lists */

extern void *ITEM; /* Temporary global variable (from queue.c) */

/* Allocate item from free-list.  If free-list empty, replenish it. */

void *replenish PARAMS((void **list,int size));

#define al_single(list,type) ( \
	(ITEM = *(void **)(list)) ? \
	  ( (*(void **)(list) = *(void **)ITEM), ITEM ) \
	: \
	  replenish((void **)(list),sizeof(type)) \
)

/* Put item on free list */

#define fr_single(list,item) do { \
	*(void **)(item) = *(void **)(list); \
	*(void **)(list) = (void *)(item); \
} while(0)

/* JOE uses 'unsigned char *', never 'char *'.  This is that when a
   character is loaded into an 'int', the codes 0-255 are used,
   not -128 - 127. */

/* Zero terminated strings */

typedef struct zs ZS;

struct zs {
	unsigned char *s;
};

/* Create zs in local gc */

#define mk_zs(var,s,len) do { \
	(var) = raw_mk_zs((s),(len)); \
	gc_add(&gc, &(var), rm_zs); \
} while(0)

ZS raw_mk_zs PARAMS((GC **gc,unsigned char *s,int len));

/* Destructor for zs */

void rm_zs PARAMS((ZS z));

/* Unsigned versions of regular string functions */

/* JOE uses 'unsigned char *', never 'char *'.  This is so that when a
   character is loaded from a string into an 'int', the codes 0-255 are
   used, not -128 - 127. */

size_t zlen PARAMS((unsigned char *s));
int zcmp PARAMS((unsigned char *a, unsigned char *b));
int zncmp PARAMS((unsigned char *a, unsigned char *b, size_t len));
unsigned char *zdup PARAMS((unsigned char *s));
unsigned char *zcpy PARAMS((unsigned char *a, unsigned char *b));
unsigned char *zncpy PARAMS((unsigned char *a, unsigned char *b,size_t len));
unsigned char *zstr PARAMS((unsigned char *a, unsigned char *b));
unsigned char *zchr PARAMS((unsigned char *s, int c));
unsigned char *zrchr PARAMS((unsigned char *s, int c));
unsigned char *zcat PARAMS((unsigned char *a, unsigned char *b));

/*
 * Functions which return minimum/maximum of two numbers  
 */
unsigned int uns_min PARAMS((unsigned int a, unsigned int b));
signed int int_min PARAMS((signed int a, int signed b));
signed long long_max PARAMS((signed long a, signed long b));
signed long long_min PARAMS((signed long a, signed long b));

/* Versions of 'read' and 'write' which automatically retry when interrupted */
ssize_t joe_read PARAMS((int fd, void *buf, size_t siz));
ssize_t joe_write PARAMS((int fd, void *buf, size_t siz));

/* wrappers to *alloc routines */
void *joe_malloc PARAMS((size_t size));
unsigned char *joe_strdup PARAMS((unsigned char *ptr));
void *joe_calloc PARAMS((size_t nmemb, size_t size));
void *joe_realloc PARAMS((void *ptr, size_t size));
void joe_free PARAMS((void *ptr));

#ifndef HAVE_SIGHANDLER_T
typedef RETSIGTYPE (*sighandler_t)(int);
#endif

#ifdef NEED_TO_REINSTALL_SIGNAL
#define REINSTALL_SIGHANDLER(sig, handler) joe_set_signal(sig, handler)
#else
#define REINSTALL_SIGHANDLER(sig, handler) do {} while(0)
#endif

/* wrapper to hide signal interface differrencies */
int joe_set_signal PARAMS((int signum, sighandler_t handler));

/* Simple parsers */
int parse_ws PARAMS((unsigned char **p,int cmt));
int parse_ident PARAMS((unsigned char **p,unsigned char *buf,int len));
int parse_kw PARAMS((unsigned char **p,unsigned char *kw));
long parse_num PARAMS((unsigned char **p));
int parse_tows PARAMS((unsigned char **p,unsigned char *buf));
int parse_field PARAMS((unsigned char **p,unsigned char *field));
int parse_char PARAMS((unsigned char  **p,unsigned char c));
int parse_int PARAMS((unsigned char **p,int *buf));
int parse_long PARAMS((unsigned char **p,long  *buf));
int parse_string PARAMS((unsigned char **p,unsigned char *buf,int len));
int parse_range PARAMS((unsigned char **p,int *first,int *second));
void emit_string PARAMS((FILE *f,unsigned char *s,int len));

#endif
