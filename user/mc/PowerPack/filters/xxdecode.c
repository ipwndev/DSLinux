#ifndef lint
static char sccsid[] = "@(#)xxdecode.c  5.3 (Berkeley) 4/10/85";
#endif

/*
 * xxdecode [input]
 *
 * create the specified file, decoding as you go.
 * used with xxencode.
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifndef MSDOS
#ifndef VMCMS
#include <pwd.h>
#endif /* VMCMS */
#else /* MSDOS */
#include <fcntl.h>
#include <io.h>
#endif /* MSDOS */

#ifndef VMCMS
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <types.h>
#include <stat.h>
#include <dir.h>
DIR * dir;
#define perror(string) fprintf (stderr, "%s\n", string)
#endif /* VMCMS */

/* single character decode */
#define DEC(c)  ( table[ (c) & 0177 ] )

static char set[] = 
    "+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
static char table[128];
int replace;

main (argc, argv)
int argc;
char * argv [];

{
FILE *in, *out;
int mode;
char source [128];
char cdest [128];
char * dest;
char * temp;
int i;
char buf[80];
int did = 0;
int replflg = 0;
int nameptr = 1;
struct stat statbuf;

if (argc > 1)
    {
    if (strcmp (argv [1], "-r") == 0 || strcmp (argv [1], "-R") == 0)
        {
        replflg = 1;
        nameptr = 2;
        }
    }
/* input filename */
if (nameptr <= argc - 1)
    {
    strcpy (source, argv [nameptr]);
#ifdef VMCMS
    for (i = nameptr + 1; i < argc; i++)
        {
        strcat (source, " ");
        strcat (source, argv [i]);
        }
#endif /* VMCMS */
    if ((in = fopen(source, "r")) == NULL)
        {
#ifndef VMCMS
        perror(source);
#else /* VMCMS */
        fprintf (stderr, "Cannot open file <%s>\n", source);
#endif /* VMCMS */
        exit(1);
        }
    }
else
#ifdef VMCMS
    {
    fprintf (stderr, "Usage: xxdecode [-r] fn ft [fm] ");
    fprintf (stderr, "[> fn ft [fm] [(options] (bin]\n");
    exit (2);
    }
#else
    in = stdin;
if (isatty (fileno (in)) || argc > 2)
    {
    fprintf (stderr, "Usage: xxdecode [-r] [infile]\n");
    exit(2);
    }
#endif /* VMCMS */

while (1)
    {
    dest = cdest;
    /* search for header line */
    for (;;)
        {
        if (fgets(buf, sizeof buf, in) == NULL)
            {
            if (! did)
                {
                fprintf (stderr, "No begin line\n");
                exit (3);
                }
            else
                exit (0);
            }
        if (strncmp(buf, "begin ", 6) == 0)
            {
            did = 1;
            break;
            }
        }
    sscanf(buf, "begin %o %s", &mode, dest);

    /* handle ~user/file format */
    if (dest[0] == '~')
        {
#ifndef VMCMS
#ifndef MSDOS
        char *sl;
        struct passwd *getpwnam();
        char *index();
        struct passwd *user;
        char dnbuf[100];

        sl = index(dest, '/');
        if (sl == NULL)
            {
            fprintf(stderr, "Illegal ~user\n");
            exit(3);
            }
        *sl++ = 0;
        user = getpwnam(dest+1);
        if (user == NULL)
            {
            fprintf(stderr, "No such user as %s\n", dest);
            exit(4);
            }
        strcpy(dnbuf, user->pw_dir);
        strcat(dnbuf, "/");
        strcat(dnbuf, sl);
        strcpy(dest, dnbuf);
#else
        dest++;
#endif /* MSDOS */
#endif /* VMCMS */
        }
    replace = replflg;
    if (strcmp (dest, "/dev/stdout") != 0 && strcmp (dest, "-") != 0)
        {
#ifdef VMCMS
        for (i = strlen (dest) - 1; i >= 0 && dest [i] != '/'; i--)
            dest [i] = toupper (dest [i]);
        dest = &dest [i + 1];
        for (i = 0; dest [i] && dest [i] != '.'; i++)
            dest [i] = toupper (dest [i]);
        if (dest [i] == '.')
            dest [i] = ' ';
        for (; dest [i] && dest [i] != '.'; i++)
            ;
        if (dest [i] == '.')
            dest [i] = '\0';
#endif /* VMCMS */
        if (stat (dest, &statbuf) == 0 && ! replflg)
            {
            fprintf (stderr, "File <%s> already exists.\n", dest);
            fprintf (stderr, "Replace? (Y or N): ");
#ifdef VMCMS
            fprintf (stderr, "\n");
#endif /* VMCMS */
            do
                {
                fscanf (stdin, "%s", buf);
                buf [0] = toupper (buf [0]);
                }
                while (buf [0] != 'N' && buf [0] != 'Y');
            replace = (buf [0] == 'Y');
            }
        else
            replace = 1;
#ifdef VMCMS
        strcat (dest, " (bin");
#endif /* VMCMS */
        }
    else
        replace = 1;
    /* create output file */
    if (replace)
        {
        fprintf (stderr, "Opening file: %s\n", dest);
        if (strcmp (dest, "/dev/stdout") == 0 || strcmp (dest, "-") == 0)
            out = stdout;
        else
            out = fopen(dest, "w");
#ifdef MSDOS
        if (setmode (fileno (out), O_BINARY) == -1)
            {
            perror ("Cannot open stdout as binary\n");
            exit (3);
            }
#endif /* MSDOS */
        if (out == NULL)
            {
#ifndef VMCMS
            perror(dest);
#else /* VMCMS */
            fprintf (stderr, "Cannot open file <%s>\n", dest);
#endif /* VMCMS */
            exit(4);
            }
        decode(in, out);
        fclose (out);
#ifndef VMCMS
        chmod(dest, mode);
#endif /* VMCMS */
        }

    if (fgets(buf,sizeof (buf), in) == NULL || strcmp (buf, "end\n"))
        {
        fprintf(stderr, "No end line\n");
        exit(5);
        }
    }
}

/*
 * copy from in to out, decoding as you go along.
 */
decode(in, out)
FILE *in;
FILE *out;
{
        char buf[80];
        char *bp;
        int n;

        bp=table;       /* clear table */
        for( n=128 ; n ; --n ) {
          *(bp++) = 0;
        };

        bp=set;         /* fill table */
        for( n=64 ; n ; --n ) {
          table[ *(bp++) & 0177 ] = 64-n;
        };

        for (;;) {
                /* for each input line */
                if (fgets(buf, sizeof buf, in) == NULL) {
                        printf("Short file\n");
                        exit(10);
                }
                n = DEC(buf[0]);
                if (n <= 0)
                        break;

                bp = &buf[1];
                while (n > 0) {
                        if (replace)
                            outdec(bp, out, n);
                        bp += 4;
                        n -= 3;
                }
        }
}

/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.  n is used to tell us not to
 * output all of them at the end of the file.
 */
outdec(p, f, n)
char *p;
FILE *f;
int n;
{
        int c1, c2, c3;

        c1 = DEC(*p) << 2 | DEC(p[1]) >> 4;
        c2 = DEC(p[1]) << 4 | DEC(p[2]) >> 2;
        c3 = DEC(p[2]) << 6 | DEC(p[3]);
        if (n >= 1)
                putc(c1, f);
        if (n >= 2)
                putc(c2, f);
        if (n >= 3)
                putc(c3, f);
}


/* fr: like read but stdio */
int
fr(fd, buf, cnt)
FILE *fd;
char *buf;
int cnt;
{
        int c, i;

        for (i=0; i<cnt; i++) {
                c = getc(fd);
                if (c == EOF)
                        return(i);
                buf[i] = c;
        }
        return (cnt);
}

/*
 * Return the ptr in sp at which the character c appears;
 * NULL if not found
 */

#define NULL    0

char *
index(sp, c)
register char *sp, c;
{
        do {
                if (*sp == c)
                        return(sp);
        } while (*sp++);
        return(NULL);
}
