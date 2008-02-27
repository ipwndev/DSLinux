/*
 * 	TERMCAP/TERMINFO header file
 *	Copyright
 *		(C) 1992 Joseph H. Allen
 *
 *	This file is part of JOE (Joe's Own Editor)
 */ 
#ifndef _JOE_TERMCAP_H
#define _JOE_TERMCAP_H 1
 
struct sortentry {
	unsigned char	*name;
	unsigned char	*value;
};

struct cap {
	unsigned char	*tbuf;		/* Termcap entry loaded here */

	struct sortentry *sort;	/* Pointers to each capability stored in here */
	int	sortlen;	/* Number of capabilities */

	unsigned char	*abuf;		/* For terminfo compatible version */
	unsigned char	*abufp;

	int	div;		/* tenths of MS per char */
	int	baud;		/* Baud rate */
	unsigned char	*pad;		/* Padding string or NULL to use NUL */
	void	(*out) (unsigned char *, unsigned char);		/* Character output routine */
	void	*outptr;	/* First arg passed to output routine.  Second
				   arg is character to write */
	int	dopadding;	/* Set if pad characters should be used */
};

/* CAP *getcap(char *s,int baud,void (*out)(void *outptr,char c),void *outptr);
 *
 * Get CAP entry for terminal named in 's'.  If 's' is zero, the name in
 * the environment variable 'TERM' is used instead.  Space for the returned
 * CAP is allocated from the heap using malloc.
 *
 * 'baud'   is the baud rate used for 'texec' to calculate number of pad chars
 * 'out'    is the function 'texec' uses to output characters
 * 'outptr' is the passed as the first arg to 'out'
 *          the second arg contains the char to output
 *
 * This is how 'getcap' finds the entry:  First a list of file names is
 * built.  If the environment variable 'TERMCAP' begins with a '/', it
 * is used as the list of file names.  Otherwise, if the environment
 * variable 'TERMPATH' is set, it is used as the list of file names.  If
 * that isn't set, then the string TERMPATH defined above is appended
 * to value of the 'HOME' environment variable, and that is used as the
 * list of names (a '/' is placed between the value of the environment
 * variable and the string).  If HOME isn't set, then TERMPATH alone is
 * used as the list of file names (without prepending a '/').
 *
 * Now the contents of the environment variable 'TERMCAP' (if it's defined and
 * if it doesn't begin with a '/') and the files from the above list are
 * scanned for the terminal name.  The contents of the environment variable
 * are scanned first, then the files are scanned in the order they appear in
 * the named list.
 *
 * If the last part of a matching termcap entry is a 'tc=filename', then
 * the current file is rewound and rescanned for the matching entry (and if
 * it's not found, the next entry in the file name list is searched).  If
 * a matching termcap entry in the TERMCAP environment variable ends with
 * a 'tc=filename', then all of the files in the name list are searched.
 *
 * There is no limit on the size of the termcap entries.  No checking is
 * done for self-refering 'tc=filename' links (so all of core will be
 * allocated if there are any).
 */
CAP *my_getcap PARAMS((unsigned char *name, unsigned int baud, void (*out) (unsigned char *, unsigned char), void *outptr));

/* CAP *setcap(CAP *cap,int baud,void (*out)(void *outptr,char c),void *outptr);
 *
 * Reset baud, out and outptr for a CAP
 */
CAP *setcap PARAMS((CAP *cap, unsigned int baud, void (*out) (unsigned char *, unsigned char), void *outptr));

/* char *jgetstr(CAP *cap,char *name);
 *
 * Get value of string capability or return NULL if it's not found.  A fast
 * binary search is used to find the capability.  The char * returned points into
 * the buffer used to load the termcap entry.  It should not be modified or
 * freed.
 */
unsigned char *jgetstr PARAMS((CAP *cap, unsigned char *name));

/* int getflag(CAP *cap,char *name);
 *
 * Return true if the named capability is found in 'cap'.  A fast binary
 * search is used to lookup the capability.
 */
int getflag PARAMS((CAP *cap, unsigned char *name));

/* int getnum(CAP *cap,char *name);
 *
 * Return value of numeric capability or return -1 if it's not found.  A fast
 * binary search is used to lookup the capability.
 */
int getnum PARAMS((CAP *cap, unsigned char *name));

/* void rmcap(CAP *cap);
 *
 * Eliminate a CAP entry.
 */
void rmcap PARAMS((CAP *cap));

/* void texec(CAP *cap,char *str,int l,int a0,int a1,int a2,int a3);

   Execute and output a termcap string capability.

   'cap' is the CAP returned by getcap which contains the baud rate and output
   function.
   
   'str' is the string to execute.  If 'str'==NULL, nothing happens.
   
   'l' is the number of lines effected by this string.  For example, if you
   use the clear to end of screen capability, the number of lines between
   the current cursor position and the end of the screen should be
   given here.

   'a0' - 'a1' are the arguments for the string
*/
void texec PARAMS((CAP *cap, unsigned char *s, int l, int a0, int a1, int a2, int a3));

/* int tcost(CAP *cap,char *str, int l, int a0, int a1, int a2, int a3);
   Return cost in number of characters which need to be sent
   to execute a termcap string capability.

   'cap' is the CAP returned by getcap which contains the baud rate and output
   functions.
   
   'str' is the string to execute.  If 'str'==NULL, tcost return 10000.
   
   'l' is the number of lines effected by this string.  Ex: if you
   use the clear to end of screen capability, the number of lines between
   the current cursor position and the end of the screen should be
   given here.

   'a0' - 'a3' are arguements passed to the string
*/
int tcost PARAMS((CAP *cap, unsigned char *s, int l, int a0, int a1, int a2, int a3));

/* char *tcompile(CAP *cap,char *str,int a0,int a1,int a2,int a3);

   Compile a string capability.  Returns a pointer to a variable length
   string (see vs.h) containing the compiled string capability.
   Pad characters are not placed in the string.
*/
unsigned char *tcompile PARAMS((CAP *cap, unsigned char *s, int a0, int a1, int a2, int a3));

/* Old termcap support */
#ifdef junk
int tgetent();
unsigned char *tgetstr();
int tgetflag();
int tgetnum();
unsigned char *tgoto();
void tputs();
extern short ospeed;
extern unsigned char PC, *UP, *BC;
#endif

extern int dopadding;
extern unsigned char *joeterm;

#endif
