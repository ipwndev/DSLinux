
/* uucat.c */

#include <stdio.h>
#include <string.h>

#define TRUE	1
#define FALSE	0

#define LENGTH	150

extern void uuread();


void main(argc, argv)
int argc;
char *argv[];
{
  int error, argno;
  FILE *infile;

  if (argc < 2)
    uuread(stdin);
  else
  {
    error = FALSE;
    for (argno = 1; !error && argno < argc; argno++)
    {
      if ((infile = fopen(argv[argno], "r")) == NULL)
      {
	error = TRUE;
	fprintf(stderr, "uucat: Can't open '%s' for input.\n", argv[argno]);
      }
      else
      {
	uuread(infile);
	fclose(infile);
      }
    }
  }

  exit(0);
}


void uuread(infile)
FILE *infile;
{
  char *s2, *s1, *s0, *tmp_s;
  int length;
  static char s[3 * (LENGTH + 1)];
  static int echo_on = FALSE, started = FALSE, just_finished = FALSE;
  static int line_length = 0, lines_to_go = 0;

  s0 = s;
  s1 = s0 + (LENGTH + 1);
  s2 = s1 + (LENGTH + 1);

  s0[0] = s1[0] = s2[0] = '\0';  /* Clear strings */

  while (fgets(s0, LENGTH, infile) != NULL)
  {
    s0[LENGTH] = '\0';  /* Make sure string is terminated */

    if (just_finished)
    {
      if (strncmp(s0, "size ", 5) == 0)
      {
	fputs(s0, stdout);
	s0[0] = '\0';
      }
      just_finished = FALSE;
    }

    if (!started)
    {
      if (strncmp(s0, "begin ", 6) == 0)
      {
	started = echo_on = TRUE;
	line_length = 0;
	lines_to_go = 0;
      }
    }
    else  /* started */
    {
      length = strlen(s0);
      if (line_length == 0)
	line_length = length;

      if (echo_on)
      {
	lines_to_go = 0;
	if (s0[0] != 'M' || length != line_length)
	{
	  echo_on = FALSE;
	  lines_to_go = 2;  /* Lines to go before 'end' is expected */
	  if (s0[0] == ' ' || s0[0] == '`')
	    lines_to_go = 1;
	}
      }
      else  /* !echo_on */
      {
	if (s0[0] == 'M' && length == line_length)
	  echo_on = TRUE;
	else if (lines_to_go > 0)
	{
	  if (lines_to_go == 2)
	  {
	    if (s0[0] == ' ' || s0[0] == '`')
	      lines_to_go = 1;
	    else
	      lines_to_go = 0;  /* Unexpected line, so break off */
	  }
	  else if (lines_to_go == 1)
	  {
	    if (strcmp(s0, "end\n") == 0)
	    {
	      fputs(s2, stdout);
	      fputs(s1, stdout);
	      fputs(s0, stdout);
	      lines_to_go = 0;  /* Done. Break off */
	      just_finished = TRUE;
	      started = FALSE;
	    }
	    else
	      lines_to_go = 0;  /* Unexpected line, so break off */
	  }
	}
      }
    }

    if (echo_on)
    {
      fputs(s0, stdout);
      s0[0] = '\0';
    }

    tmp_s = s2;
    s2 = s1;
    s1 = s0;
    s0 = tmp_s;
  }
}

