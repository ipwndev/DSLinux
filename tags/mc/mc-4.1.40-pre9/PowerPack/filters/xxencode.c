#ifndef lint
static char sccsid[] = "@(#)xxencode.c  5.3 (Berkeley) 1/22/85";
#endif
 
/*
 * xxencode [input] output
 *
 * Encode a file so it can be mailed to a remote system.
 */
#include <stdio.h>
#ifdef MSDOS
#include <fcntl.h>
#include <io.h>
#endif /* MSDOS */
 
#ifndef VMCMS
#include <sys/types.h>
#include <sys/stat.h>
#else  /* VMCMS */
#include <types.h>
#include <stat.h>
#define perror(string) fprintf (stderr, "%s\n", string)
#endif /* VMCMS */
 
/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) ( set[ (c) & 077 ] )
static char set[] = 
    "+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
 
main (argc, argv)
int argc;
char * argv [];
 
{
FILE *in;
struct stat sbuf;
int mode;
char buffer [256];
int i;
 
/* optional 1st argument */
if (argc > 2)
    {
    strcpy (buffer, argv [1]);
#ifdef VMCMS
    for (i = 2; i < argc - 1; i++)
        {
        strcat (buffer, " ");
        strcat (buffer, argv [i]);
        }
    strcat (buffer, " (bin");
#endif /* VMCMS */
    if ((in = fopen (buffer, "r")) == NULL)
        {
        perror (buffer);
        exit(1);
        }
    }
else
    in = stdin;
#ifdef MSDOS
if (setmode (fileno (in), O_BINARY) == -1)
    {
    perror ("Cannot open input file as binary\n");
    exit (3);
    }
#endif /* MSDOS */
 
#ifndef VMCMS
if (isatty (fileno (in)) || argc < 2 || argc > 3)
    {
    fprintf (stderr, "Usage: xxencode [infile] remotefile\n");
    exit(2);
    }
#else
if (isatty (fileno (in)) || argc < 2)
    {
    fprintf (stderr, "Usage: xxencode fn ft fm (bin options remotefile ");
    fprintf (stderr, "> fn ft fm\n");
    fprintf (stderr, "   or: xxencode remotefile < fn ft fm (bin options ");
    fprintf (stderr, "> fn ft fm\n");
    fprintf (stderr, "remotefile is a Unix syntax filename.\n");
    exit(2);
    }
#endif /* VMCMS */
 
/* figure out the input file mode */
fstat (fileno (in), &sbuf);
#ifndef VMCMS
mode = sbuf.st_mode & 0777;
#else
mode = 0700;
#endif /* VMCMS */
printf ("begin %o %s\n", mode, argv [argc - 1]);
 
encode (in, stdout);
 
printf ("end\n");
exit (0);
}
 
/*
 * copy from in to out, encoding as you go along.
 */
encode(in, out)
FILE *in;
FILE *out;
{
        char buf[80];
        int i, n;
 
        for (;;) {
                /* 1 (up to) 45 character line */
                n = fr(in, buf, 45);
                putc(ENC(n), out);
 
                for (i=0; i<n; i += 3)
                        outdec(&buf[i], out);
 
                putc('\n', out);
                if (n <= 0)
                        break;
        }
}
 
/*
 * output one group of 3 bytes, pointed at by p, on file f.
 */
outdec(p, f)
char *p;
FILE *f;
{
        int c1, c2, c3, c4;
 
        c1 = *p >> 2;
        c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
        c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
        c4 = p[2] & 077;
        putc(ENC(c1), f);
        putc(ENC(c2), f);
        putc(ENC(c3), f);
        putc(ENC(c4), f);
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
