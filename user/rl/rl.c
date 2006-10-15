/*
   rl - Select a random line from stdin or file.

   Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Arthur de Jong

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/


#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else /* not HAVE_GETOPT_H */
#include <unistd.h>
#endif /* not HAVE_GETOPT_H */
#ifndef HAVE_GETOPT_LONG
#include "getopt_long.h"
#endif /* not HAVE_GETOPT_LONG */

#include "rl.h"
#include "alloc.h"
#include "buffer.h"
#include "random.h"


/* the name of the program */
static char *program_name;


/* flag to indicate output */
int quiet=0;


/* Option flags and variables */
static struct option const long_options[] =
{
  {"count",required_argument,NULL,'c'},
  {"reselect",no_argument,NULL,'r'},
  {"output",required_argument,NULL,'o'},
  {"delimiter",required_argument,NULL,'d'},
  {"null",no_argument,NULL,'0'},
  {"line-number",no_argument,NULL,'n'},
  {"line-numbers",no_argument,NULL,'n'},
  {"quiet",no_argument,NULL,'q'},
  {"silent",no_argument,NULL,'q'},
  {"help",no_argument,NULL,'h'},
  {"version",no_argument,NULL,'V'},
  {NULL,0,NULL,0}
};
/* for adding options you should add to
    long_options[]  (directly above)
    OPTION_STRING   (directly below)
    display_usage() (below)
    main()          (for the handling of the option) */
#define OPTION_STRING "c:ro:d:0nqhV"


/* display usage information */
static void
display_usage(FILE *fp)
{
  fprintf(fp,_("Usage: %s [OPTION]... [FILE]...\n"),program_name);
  fprintf(fp,_("Randomize the lines of a file (or stdin).\n\n"));
  fprintf(fp,_("  -c, --count=N  select N lines from the file\n"));
  fprintf(fp,_("  -r, --reselect lines may be selected multiple times\n"));
  fprintf(fp,_("  -o, --output=FILE\n"
               "                 send output to file\n"));
  fprintf(fp,_("  -d, --delimiter=DELIM\n"
               "                 specify line delimiter (one character)\n"));
  fprintf(fp,_("  -0, --null     set line delimiter to null character\n"
               "                 (useful with find -print0)\n"));
  fprintf(fp,_("  -n, --line-number\n"
               "                 print line number with output lines\n"));
  fprintf(fp,_("  -q, --quiet, --silent\n"
               "                 do not output any errors or warnings\n"));
  fprintf(fp,_("  -h, --help     display this help and exit\n"));
  fprintf(fp,_("  -V, --version  output version information and exit\n"));
}


/* display a use --help notice */
static void
display_tryhelp(FILE *fp)
{
  fprintf(fp,_("Try `%s --help' for more information.\n"),program_name);
}


/* display version information */
static void
display_version(FILE *fp)
{
  fprintf(fp,"rl %s\n",VERSION);
  fprintf(fp,_("Written by Arthur de Jong.\n\n"));
  fprintf(fp,_("Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Arthur de Jong.\n"
               "This is free software; see the source for copying conditions.\n"  
               "There is NO warranty; not even for MERCHANTABILITY or FITNESS\n"
               "FOR A PARTICULAR PURPOSE.\n"));
}


/* read the whole file and pick out count random lines and dump to out
   all arguments are assumed to be reasonable values 
   any line has an equal chance of getting picked
   and lines may be selected multiple times 
   lines are delimited by delim */
static void
rl_withreplace(FILE *in,FILE *out,int count,char delim,int print_linenumbers)
{
  struct buffer *lines;    /* array of lines */
  struct buffer current;   /* currently read line */
  int line=0;              /* the number of the current line */
  int i;                   /* counter for loops */
  /* initialize buffers */
  lines=xxmalloc(struct buffer,count);
  for (i=0;i<count;i++)
  {
    buffer_init(lines+i,AVGLINELEN*2);
  }
  buffer_init(&current,AVGLINELEN*2);
  /* read all the lines one by one */
  while (buffer_readline(in,&current,delim)!=NULL)
  {
    line++;
    current.linenumber=line;
    /* count times see if we want to include this line */
    for (i=0;i<count;i++)
    {
      if (random_draw(1,line))
      {
        /* copy line into result buffer */
        buffer_copy(lines+i,&current);
      }
    }
  }
  /* check for read errors */
  if (ferror(in))
  {
    fprintf(stderr,_("%s: read error: %s\n"),program_name,strerror(errno));
    exit(1);
  }
  /* dump the result (if any lines were read) */
  if (line>0)
  {
    for (i=0;i<count;i++)
    {
      /* print line number */
      if (print_linenumbers)
        fprintf(out,"%d: ",lines[i].linenumber);
      /* output line */
      if ( fwrite(lines[i].buf,sizeof(char),lines[i].len,out)!=lines[i].len ||
           ferror(out) )
      {
        fprintf(stderr,_("%s: write error: %s\n"),program_name,strerror(errno));
        exit(1);
      }
    }
  }
  /* free the memory! */
  buffer_free(&current);
  for (i=0;i<count;i++)
  {
    buffer_free(lines+i);
  }
  xfree(lines);
}


/* read the whole file and pick out count random lines and dump to out
   all arguments are assumed to be reasonable values 
   (maybe a better functionname, my English is not that good )
   any line can only be picked once
   lines are delimited by delim */
static void
rl_withoutreplace(FILE *in,FILE *out,int count,char delim,int print_linenumbers)
{
  struct buffer **result;       /* array of pointers to selected lines */
  struct buffer *t=NULL;        /* temporary pointer for swapping lines */
  struct buffer *current;       /* currently read line */
  int line=0;                   /* the number of the current line */
  int i;                        /* counter for loops */
  int j;                        /* for swapping */
  /* initialize result */
  result=xxmalloc(struct buffer *,count);
  /* first read count lines */
  while ( (line<count) && ((result[line]=buffer_readline(in,NULL,delim))!=NULL) )
  {
    result[line]->linenumber=line+1;
    line++;
  }
  /* check for read errors */
  if (ferror(in))
  {
    fprintf(stderr,_("%s: read error: %s\n"),program_name,strerror(errno));
    exit(1);
  }
  /* randomize these lines */
  for (i=0;i<line;i++)
  {
    j=random_below(line);
            t=result[i];
    result[i]=result[j];
    result[j]=t;
  }
  t=NULL;
  /* check if the buffer is filled */
  if (line<count)
  {
    if (!quiet)
      fprintf(stderr,_("%s: less than %d lines were read.\n"),program_name,count);
  }
  else
  {
    /* read the lines one by one */
    current=NULL;
    while ((current=buffer_readline(in,current,delim))!=NULL)
    {
      line++;
      current->linenumber=line;
      if (random_draw(count,line))
      {
        i=random_below(count);
        /* swap current with result[i] */
                t=result[i];
        result[i]=current;
          current=t;
      }
    }
    /* check for read errors */
    if (ferror(in))
    {
      fprintf(stderr,_("%s: read error: %s\n"),program_name,strerror(errno));
      exit(1);
    }
  }
  /* dump the result (if any lines were read) */
  for (i=0;(i<line)&&(i<count);i++)
  {
    /* print line number */
    if (print_linenumbers)
      fprintf(out,"%d: ",result[i]->linenumber);
    /* output line */
    if ( fwrite(result[i]->buf,sizeof(char),result[i]->len,out)!=result[i]->len ||
         ferror(out) )
    {
      fprintf(stderr,_("%s: write error: %s\n"),program_name,strerror(errno));
      exit(1);
    }
  }
  /* free the memory! */
  if (t!=NULL) /* t accidentaly points to the last current */
  {
    buffer_free(t);
    xfree(t);
  }
  for (i=0;(i<line)&&(i<count);i++)
  {
    if (result[i]!=NULL)
    {
      buffer_free(result[i]);
      xfree(result[i]);
    }
  }
  xfree(result);
}


/* read file and randomize and output all lines 
   this is done in a way that any line will only be returned once */
static void
rl_randomizefile(FILE *in,FILE *out,char delim,int print_linenumbers)
{
  struct buffer buffer;         /* for reading the file */
  int *result;                  /* array of pointers to selected lines */
  struct buffer *lines;         /* an array of all read lines */
  int alloc;                    /* allocated size of result */
  int line;                     /* number of lines in result */
  int i;                        /* multi purpose */
  int j;                        /* multi purpose */
  int t;                        /* for swapping */
  /* read the file */
  buffer_init(&buffer,BLOCKSIZE*2);
  buffer_readfile(in,&buffer);
  /* check for read errors */
  if (ferror(in))
  {
    fprintf(stderr,_("%s: read error: %s\n"),program_name,strerror(errno));
    exit(1);
  }
  /* initialize result, guess number of lines */
  alloc=buffer.len/AVGLINELEN+5;
  result=xxmalloc(int,alloc);
  lines=xxmalloc(struct buffer,alloc);
  line=0;
  /* split in lines */
  for (i=0;i<buffer.len;)
  {
    /* check if realloc is needed */
    if (line>=alloc)
    {
      alloc=alloc*2; /* *4/3 */ /* only slightly grow */
      result=xxrealloc(result,int,alloc);
      lines=xxrealloc(lines,struct buffer,alloc);
    }
    /* find the end of the line */
    for (j=i;(j<buffer.len)&&(buffer.buf[j]!=delim);j++);
    if (buffer.buf[j]==delim) j++;
    /* build the line */
    lines[line].buf=buffer.buf+i;
    lines[line].alloc=0;
    lines[line].len=j-i;
    lines[line].linenumber=line+1;
    result[line]=line;
    /* ready for the next line */
    i=j; /* continue from end of line */
    line++;
  }
  /* randomize lines */
  for (i=0;i<line;i++)
  {
    j=random_below(line);
    /* swap entries */
            t=result[i];
    result[i]=result[j];
    result[j]=t;
  }
  /* dump the result */
  for (i=0;i<line;i++)
  {
    j=result[i];
    /* print line number */
    if (print_linenumbers)
      fprintf(out,"%d: ",lines[j].linenumber);
    /* output line */
    if ( fwrite(lines[j].buf,sizeof(char),lines[j].len,out)!=lines[j].len ||
         ferror(out) )
    {
      fprintf(stderr,_("%s: write error: %s\n"),program_name,strerror(errno));
      exit(1);
    }
  }
  /* free the memory! */
  xfree(result);
  xfree(lines);
  buffer_free(&buffer);
}


/* the main program */
int
main(int argc,char **argv)
{
  int c;           /* option charaters */
  FILE *fp;        /* for reading command-line specified files */
  char *outputf=NULL;   /* name of file to send output to */
  FILE *output;    /* where to write random lines to */
  int count=-1;    /* the command-line parameter */
  char *endptr;    /* used for command-line parsing */
  int uniq=1;      /* wether to return same lines or not */
  char delim='\n'; /* line delimiter */
  int print_linenumbers=0; /* wether to output line numbers */

  program_name=argv[0];
  
  /* parse command-line options */
  while ((c=getopt_long(argc,argv,OPTION_STRING,
                        long_options,(int *)NULL))!=-1)
  { 
    /* find out which option was specified */
    switch (c)
    {
    case 'V': /* -V, --version */
      display_version(stdout);
      exit(0);
    case 'h': /* -h, --help */
      display_usage(stdout);
      exit(0);
    case 'c': /* -c, --count=N */
      count=strtol(optarg,&endptr,0);
      if ( (optarg[0]=='\0') || (endptr[0]!='\0') || 
           (count<1) )
      {
        if (!quiet)
        {
          fprintf(stderr,_("%s: invalid argument to %s option\n"),program_name,"count");
          display_tryhelp(stderr);
        }
        exit(1);
      }
      break;
    case 'r': /* -r, --reselect */
      uniq=0;
      break;
    case 'o': /* -o, --output=FILE */
      if (outputf!=NULL)
      {
        if (!quiet)
          fprintf(stderr,_("%s: can only specify --output once\n"),program_name);
        exit(1);
      }
      outputf=strdup(optarg);
      break;
    case 'd': /* -d, --delimiter=DELIM */
      if ( (optarg[0]=='\0' || optarg[1]!='\0') )
      {
        if (!quiet)
        {
          fprintf(stderr,_("%s: invalid argument to %s option\n"),program_name,"delim");
          display_tryhelp(stderr);
        }
        exit(1);
      }
      delim=optarg[0];
      break;
    case '0': /* -0, --null */
      delim='\0';
      break;
    case 'n': /* -n, --line-number */
      print_linenumbers=1; /* true */
      break;
    case 'q': /* -q, --quiet, --silent */
      quiet=1; /* true */
      opterr=0; /* disable error-reporting of getopt() */
      break;
    case ':': /* missing parameter of an option */
    case '?': /* unknown option */
    default:  /* undefined */
      if (!quiet)
        display_tryhelp(stderr);
      exit(1);
    }
  }

  /* intialize the random-number generator */
  randomize();
  
  /* open the output if specified */
  if ( (outputf!=NULL) && (strcmp(outputf,"-")!=0) )
  {
    if ((output=fopen(outputf,"w"))==NULL)
    {
      if (!quiet)
        fprintf(stderr,_("%s: error opening %s: %s\n"),program_name,outputf,strerror(errno));
      exit(1);
    }
  }
  else
  {
    output=stdout;
  }

  /* rest of parameters are filenames */
  if (optind>=argc)
  {
    if (count<=0)
      rl_randomizefile(stdin,output,delim,print_linenumbers);
    else if (uniq)
      rl_withoutreplace(stdin,output,count,delim,print_linenumbers);
    else
      rl_withreplace(stdin,output,count,delim,print_linenumbers);
  }
  else
  {
    for (;optind<argc;optind++)
    {
      if (strncmp("-",argv[optind],2)==0)
      {
       if (count<=0)
         rl_randomizefile(stdin,output,delim,print_linenumbers);
       else if (uniq)
         rl_withoutreplace(stdin,output,count,delim,print_linenumbers);
       else
         rl_withreplace(stdin,output,count,delim,print_linenumbers);
      }
      else
      {
        if ((fp=fopen(argv[optind],"r"))==NULL)
        {
          if (!quiet)
            fprintf(stderr,_("%s: error opening %s: %s\n"),program_name,argv[optind],strerror(errno));
        }
        else
        {
          if (count<=0)
            rl_randomizefile(fp,output,delim,print_linenumbers);
          else if (uniq)
            rl_withoutreplace(fp,output,count,delim,print_linenumbers);
          else
            rl_withreplace(fp,output,count,delim,print_linenumbers);
          /* check for read errors */
          if (fclose(fp))
          {
            fprintf(stderr,_("%s: read error: %s\n"),program_name,strerror(errno));
            exit(1);
          }
        }
      }
    }
  }

  /* close output when we're done */
  if ( (outputf!=NULL) && (strcmp(outputf,"-")!=0) )
  {
    if (fclose(output))
    {
      fprintf(stderr,_("%s: write error: %s\n"),program_name,strerror(errno));
      exit(1);
    }
  }
  else
  {
    if (fflush(output))
    {
      fprintf(stderr,_("%s: write error: %s\n"),program_name,strerror(errno));
      exit(1);
    }
  }

  exit(0);
}
