/* Man page to help file converter
   Copyright (C) 1994, 1995 Janne Kukonlehto
   
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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>		/* O_RDONLY, O_WRONLY */
#include <unistd.h>		/* FIXME: we need HAVE_UNISTD_H or so on */
#include "help.h"

#define BUFFER_SIZE 256

static char *filename;		/* The name of the input file */
static int width;		/* Output width in characters */
static int col = 0;		/* Current output column */
static FILE *index_file;	/* HTML index file */
static int out_row = 1;		/* Current output row */
static int in_row = 0;		/* Current input row */
static int old_heading_level = 0;/* Level of the last heading */
static int no_split_flag = 0;	/* Flag: Don't split section on next ".SH" */
static int skip_flag = 0;	/* Flag: Skip this section.
				   0 = don't skip,
				   1 = skipping title,
				   2 = title skipped, skipping text */
static int link_flag = 0;	/* Flag: Next line is a link */
static int verbatim_flag = 0;	/* Flag: Copy input to output verbatim */

static char *output;		/* The name of the output mc.hlp */
static char *Topics = "Topics:";

static struct node {
    char *node;
    char *lname;
    struct node *next;
} nodes, *cnode;

/* Report error in input */
void print_error (char *message)
{
    fprintf (stderr, "man2hlp: %s in file \"%s\" at row %d\n", message, filename, in_row);
}

/* Change output line */
void newline (void)
{
    out_row ++;
    col = 0;
    printf("\n");
}

/* Calculate the length of string */
int string_len (char *buffer)
{
    static int anchor_flag = 0;	/* Flag: Inside hypertext anchor name */
    static int link_flag = 0;	/* Flag: Inside hypertext link target name */
    int backslash_flag = 0;	/* Flag: Backslash quoting */
    int i;			/* Index */
    int c;			/* Current character */
    int len = 0;		/* Result: the length of the string */

    for (i = 0; i < strlen (buffer); i ++)
    {
	c = buffer [i];
	if (c == CHAR_LINK_POINTER) 
	    link_flag = 1;	/* Link target name starts */
	else if (c == CHAR_LINK_END)
	    link_flag = 0;	/* Link target name ends */
	else if (c == CHAR_NODE_END) {
	    /* Node anchor name starts */
	    anchor_flag = 1;
	    /* Ugly hack to prevent loss of one space */
	    len ++;
	}
	/* Don't add control characters to the length */
	if (c < 32)
	    continue;
	/* Attempt to handle backslash quoting */
	if (c == '\\' && !backslash_flag) {
	    backslash_flag = 1;
	    continue;
	}
	backslash_flag = 0;
	/* Increase length if not inside anchor name or link target name */
	if (!anchor_flag && !link_flag)
	    len ++;
	if (anchor_flag && c == ']') {
	    /* Node anchor name ends */
	    anchor_flag = 0;
	}
    }
    return len;
}

/* Output the string */
void print_string (char *buffer)
{
    int len;	/* The length of current word */
    int i;	/* Index */
    int c;	/* Current character */
    int backslash_flag = 0;

    /* Skipping lines? */
    if (skip_flag)
	return;
    /* Copying verbatim? */
    if (verbatim_flag) {
	printf("%s", buffer);
	return;
    }
    if (width) {
	/* HLP format */
	/* Split into words */
	buffer = strtok (buffer, " \t\n");
	/* Repeat for each word */
	while (buffer) {
	    /* Skip empty strings */  
	    if (strlen (buffer) > 0) {
		len = string_len (buffer);
		/* Change the line if about to break the right margin */
		if (col + len >= width)
		    newline ();
		/* Words are separated by spaces */
		if (col > 0) {
		    printf (" ");
		    col ++;
		}
		/* Attempt to handle backslash quoting */
		for (i = 0; i < strlen (buffer); i++)
		{
		    c = buffer [i];
		    if (c == '\\' && !backslash_flag) {
			backslash_flag = 1;
			continue;
		    }
		    backslash_flag = 0;
		    printf ("%c", c);
		}
		/* Increase column */
		col += len;
	    }
	    /* Get the next word */
	    buffer = strtok (NULL, " \t\n");
	} /* while */
    } else {
	/* HTML format */
	if (strlen (buffer) > 0) {
	    /* Attempt to handle backslash quoting */
	    for (i = 0; i < strlen (buffer); i++)
	    {
		c = buffer [i];
		if (c == '\\' && !backslash_flag) {
		    backslash_flag = 1;
		    continue;
		}
		backslash_flag = 0;
		printf ("%c", c);
	    }
	}
    } /* if (width) */
}

/* Like print_string but with printf-like syntax */
void printf_string (char *format, ...)
{
    va_list args;
    char buffer [BUFFER_SIZE];

    va_start (args, format);
    vsprintf (buffer, format, args);
    va_end (args);
    print_string (buffer);
}

/* Handle all the roff dot commands */
void handle_command (char *buffer)
{
    int i, len, heading_level;

    /* Get the command name */
    strtok (buffer, " \t");
    if (strcmp (buffer, ".SH") == 0) {
	/* If we already skipped a section, don't skip another */
	if (skip_flag == 2) {
	    skip_flag = 0;
	}
	/* Get the command parameters */
	buffer = strtok (NULL, "");
	if (buffer == NULL) {
	    print_error ("Syntax error: .SH: no title");
	    return;
	} else {
	    /* Remove quotes */
	    if (buffer[0] == '"') {
		buffer ++;
		len = strlen (buffer);
		if (buffer[len-1] == '"') {
		    len --;
		    buffer[len] = 0;
		}
	    }
	    /* Calculate heading level */
	    heading_level = 0;
	    while (buffer [heading_level] == ' ')
		heading_level ++;
	    /* Heading level must be even */
	    if (heading_level & 1)
		print_error ("Syntax error: .SH: odd heading level");
	    if (no_split_flag) {
		/* Don't start a new section */
		if (width) {
		    /* HLP format */
		    newline ();
		    print_string (buffer);
		    newline ();
		    newline ();
		} else {
		    /* HTML format */
		    newline ();
		    printf_string ("<h4>%s</h4>", buffer);
		    newline ();
		}
		no_split_flag = 0;
	    }
	    else if (skip_flag) {
		/* Skipping title and marking text for skipping */
		skip_flag = 2;
	    }
	    else {
		/* Start a new section */
		if (width) {
		    /* HLP format */
		  if (!output)
		    printf ("%c[%s]", CHAR_NODE_END, buffer);
		  else {
		    printf ("%c[%s]", CHAR_NODE_END, buffer + heading_level);
		    if (!cnode) {
			cnode = &nodes;
			cnode->next = NULL;
		    } else {
			cnode->next = malloc (sizeof (nodes));
			cnode = cnode->next;
		    }
		    cnode->node = strdup (buffer);
		    cnode->lname = NULL;
		  }
		    col ++;
		    newline ();
		    print_string (buffer + heading_level);
		    newline ();
		    newline ();
		} else {
		    /* HTML format */
		    if (buffer [0]) {
			if (heading_level > old_heading_level) {
			    for (i = old_heading_level; i < heading_level; i += 2)
				fprintf (index_file, "<ul>");
			} else {
			    for (i = heading_level; i < old_heading_level; i += 2)
				fprintf (index_file, "</ul>");
			}
			old_heading_level = heading_level;
			printf_string ("<h%d><a name=\"%s\">%s</a></h%d>",
				       heading_level / 2 + 2, buffer + heading_level,
				       buffer + heading_level, heading_level / 2 + 2);
			newline ();
			fprintf (index_file, "<li><a href=\"#%s\">%s</a>\n",
				 buffer + heading_level, buffer + heading_level);
		    } else {
			for (i = 0; i < old_heading_level; i += 2)
			    fprintf (index_file, "</ul>");
			old_heading_level = 0;
			fprintf (index_file, "</ul><p><ul>\n");
		    }
		} /* if (width) */
	    } /* Start new section */
	} /* Has parameters */
    } /* Command .SH */
    else if (strcmp (buffer, ".\\\"DONT_SPLIT\"") == 0) {
	no_split_flag = 1;
    }
    else if (strcmp (buffer, ".\\\"SKIP_SECTION\"") == 0) {
	skip_flag = 1;
    }
    else if (strcmp (buffer, ".\\\"LINK\"") == 0) {
	/* Next input line is a link */
	link_flag = 1;
    }
    else if (strcmp (buffer, ".\\\"LINK2\"") == 0) {
	/* Next two input lines form a link */
	link_flag = 2;
    }
    else if (strcmp (buffer, ".PP") == 0) {
	/* End of paragraph */
	if (width) {
	    /* HLP format */
	    if (col > 0) newline();
	} else /* HTML format */
	    print_string ("<p>");
	newline ();
    }
    else if (strcmp (buffer, ".nf") == 0) {
	/* Following input lines are to be handled verbatim */
	verbatim_flag = 1;
	if (width) {
	    /* HLP format */
	    if (col > 0) newline ();
	} else {
	    /* HTML format */
	    print_string ("<pre>");
	    newline ();
	}
    }
    else if (strcmp (buffer, ".I") == 0 || strcmp (buffer, ".B") == 0) {
	/* Bold text or italics text */
	char type = buffer [1];

	buffer = strtok (NULL, "");
	if (buffer == NULL) {
	    print_error ("Syntax error: .I / .B: no text");
	    return;
	    
	if (output)
	    Topics = strdup (buffer);
	else
	    printf ("%s\n", buffer);
	}
	else {
	    if (!width) {
		/* HTML format */
		/* Remove quotes */
		if (buffer[0] == '"') {
		    buffer ++;
		    len = strlen (buffer);
		    if (buffer[len-1] == '"') {
			len --;
			buffer[len] = 0;
		    }
		}
		printf_string ("<%c>%s</%c>", type, buffer, type);
		newline ();
	    } else /* HLP format */
		printf_string ("%c%s%c", 
		    (type == 'I') ? CHAR_ITALIC_ON : CHAR_BOLD_ON, 
		    buffer, CHAR_BOLD_OFF);
	}
    }
    else if (strcmp (buffer, ".TP") == 0) {
	/* End of paragraph? */
	if (width) {
	    /* HLP format */
	    if (col > 0) newline ();
	} else {
	    /* HTML format */
	    print_string ("<p>");
	}
	newline ();
    }
    else {
	/* Other commands are ignored */
    }
}

void handle_link (char *buffer)
{
    static char old [80];
    int len;

    switch (link_flag) {
    case 1:
	/* Old format link */
	if (width) /* HLP format */
	    printf_string ("%c%s%c%s%c\n", CHAR_LINK_START, buffer, CHAR_LINK_POINTER, buffer, CHAR_LINK_END);
	else {
	    /* HTML format */
	    printf_string ("<a href=\"#%s\">%s</a>", buffer, buffer);
	    newline ();
	}
	link_flag = 0;
	break;
    case 2:
	/* First part of new format link */
	strcpy (old, buffer);
	link_flag = 3;
	break;
    case 3:
	/* Second part of new format link */
	if (buffer [0] == '.')
	    buffer++;
	if (buffer [0] == '\\')
	    buffer++;
	if (buffer [0] == '"')
	    buffer++;
	len = strlen (buffer);
	if (len && buffer [len-1] == '"') {
	    buffer [--len] = 0;
	}
	if (width) /* HLP format */
	    printf_string ("%c%s%c%s%c\n", CHAR_LINK_START, old, CHAR_LINK_POINTER, buffer, CHAR_LINK_END);
	else {
	    /* HTML format */
	    printf_string ("<a href=\"#%s\">%s</a>", buffer, old);
	    newline ();
	}
	link_flag = 0;
	break;
    }
}

int main (int argc, char **argv)
{
    int len;			/* Length of input line */
    FILE *file;			/* Input file */
    char buffer2 [BUFFER_SIZE];	/* Temp input line */
    char buffer [BUFFER_SIZE];	/* Input line */
    int i, j;
    char *node = NULL;

    int file_fd;
    long contents_beginning, file_end;

    /* Validity check for arguments */
    if ((argc != 3 && argc != 5) || ((width = atoi (argv[1])) <= 10)) {
	fprintf (stderr, "Usage: man2hlp <width> <file.man> [template_file helpfile]\n");
	return 3;
    }

    /* Open the input file */
    filename = argv[2];
    file = fopen (filename, "r");
    if (file == NULL) {
	sprintf (buffer, "man2hlp: Can't open file \"%s\"", filename);
	perror (buffer);
	return 3;
    }

    if (argc == 5)
	output = argv[4];

    if (!width) {
	/* HTML format */
	index_file = fopen ("index.html", "w");
	if (index_file == NULL) {
	    perror ("man2hlp: Can't open file \"index.html\"");
	    return 3;
	}
	fprintf (index_file, "<html><head><title>Midnight Commander manual</title>\n");
	fprintf (index_file, "</head><body><pre><img src=\"mc.logo.small.gif\" width=180 height=85 align=\"right\" alt=\""
		 "                                                                            \n"
		 "______________           ____                          ____                 \n"
		 "|////////////#           |//#                          |//#                 \n"
		 "|//##+//##+//#           |//#                          |//#                 \n"
		 "|//# |//# |//# ____      |//#           ____           |//#                 \n"
		 "|//# |//# |//# |//#      |//#           |//#           |//#                 \n"
		 "|//# |//# |//# +###      |//#           +###           |//#        ____     \n"
		 "|//# |//# |//# ____ _____|//# _________ ____ _________ |//#______ _|//#__   \n"
		 "|@@# |@@# |@/# |//# |///////# |///////# |//# |///////# |////////# |/////#   \n"
		 "|@@# |@@# |@@# |@/# |//##+//# |//##+//# |//# |//##+//# |//###+//# +#/@###   \n"
		 "|@@# |@@# |@@# |@@# |@@# |@@# |@/# |//# |//# |//# |/@# |@@#  |@@#  |@@# ____\n"
		 "|@@# |@@# |@@# |@@# |@@#_|@@# |@@# |@@# |@@# |@@#_|@@# |@@#  |@@#  |@@#_|@@#\n"
		 "|@@# |@@# |@@# |@@# |@@@@@@@# |@@# |@@# |@@# |@@@@@@@# |@@#  |@@#  |@@@@@@@#\n"
		 "+### +### +### +### +######## +### +### +### +####+@@# +###  +###  +########\n"
		 "           _______________________________________|@@#                      \n"
		 "           |@@@@@@@@@@@ C O M M A N D E R @@@@@@@@@@@#                      \n"
		 "           +##########################################                      \n"
		 "                                                                            \n"
		 "\"></pre><h1>Manual</h1>\nThis is the manual for Midnight Commander version %s.\n"
		 "This HTML version of the manual has been compiled from the NROFF version at %s.<p>\n",
		 VERSION, __DATE__);
	fprintf (index_file, "<hr><h2>Contents</h2><ul>\n");
    }

    if (argc == 5 && freopen (argv [4], "w", stdout) == NULL) {
	sprintf (buffer, "man2hlp: Cannot open file \"%s\"", argv[4]);
	perror (buffer);
	return 3;
    }
    /* Repeat for each input line */
    while (!feof (file)) {
	/* Read a line */
	if (!fgets (buffer2, BUFFER_SIZE, file)) {
	    break;
	}
	if (!width) {
	    /* HTML format */
	    if (buffer2 [0] == '\\' && buffer2 [1] == '&')
		i = 2;
	    else
		i = 0;
	    for (j = 0; i < strlen (buffer2); i++, j++) {
		if (buffer2 [i] == '<') {
		    buffer [j++] = '&';
		    buffer [j++] = 'l';
		    buffer [j++] = 't';
		    buffer [j] = ';';
		} else if (buffer2 [i] == '>') {
		    buffer [j++] = '&';
		    buffer [j++] = 'g';
		    buffer [j++] = 't';
		    buffer [j] = ';';
		} else
		    buffer [j] = buffer2 [i];
	    }
	    buffer [j] = 0;
	} else {
	    /* HLP format */
	    if (buffer2 [0] == '\\' && buffer2 [1] == '&')
		strcpy (buffer, buffer2 + 2);
	    else
		strcpy (buffer, buffer2);
	}
	in_row ++;
	len = strlen (buffer);
	/* Remove terminating newline */
	if (buffer [len-1] == '\n')
	{
	    len --;
	    buffer [len] = 0;
	}
	if (verbatim_flag) {
	    /* Copy the line verbatim */
	    if (strcmp (buffer, ".fi") == 0) {
		verbatim_flag = 0;
	        if (!width) print_string ("</pre>"); /* HTML format */
	    } else {
		print_string (buffer);
		newline ();
	    }
	}
	else if (link_flag)
	    /* The line is a link */
	    handle_link (buffer);
	else if (buffer[0] == '.')
	    /* The line is a roff command */
	    handle_command (buffer);
	else
	{
	    /* A normal line, just output it */
	    print_string (buffer);
	    if (!width)	newline (); /* HTML format */
	}
    }

    /* All done */
    newline ();
    if (!width) {
	/* HTML format */
	print_string ("<hr></body></html>");
	newline ();
	for (i = 0; i < old_heading_level; i += 2)
	    fprintf (index_file, "</ul>");
	old_heading_level = 0;
	fprintf (index_file, "</ul><hr>\n");
	fclose (index_file);
    }
    fclose (file);

    if (argc != 5)
	return 0;

    /* Open the template file */
    filename = argv[3];
    file = fopen (filename, "r");
    if (file == NULL) {
	sprintf (buffer, "man2hlp: Cannot open file \"%s\"", filename);
	perror (buffer);
	return 3;
    }
    /* Repeat for each input line */
    while (!feof (file)) {
	/* Read a line */
	if (!fgets (buffer2, BUFFER_SIZE, file)) {
	    break;
	}
	if (node) {
	    if (*buffer2 && *buffer2 != '\n') {
		cnode->lname = strdup (buffer2);
		node = strchr (cnode->lname, '\n');
		if (node)
		    *node = 0;
	    }
	    node = NULL;
	} else {
	node = strchr (buffer, CHAR_NODE_END);
	if (node && (*(node+1) == '[')) {
	    char *p = strrchr (node, ']');
	    if (p && strncmp (node+2, "main", 4) == 0 && *(node + 6) == ']') {
		node = 0;
	    } else {
		if (!cnode) {
		    cnode = &nodes;
		    cnode->next = NULL;
		} else {
		    cnode->next = malloc (sizeof (nodes));
		    cnode = cnode->next;
		}
		cnode->node = strdup (node + 2);
		cnode->node [p - node - 2] = 0;
		cnode->lname = NULL;
	    }
	} else
	    node = NULL;
	}
	fputs (buffer, stdout);
    }

    contents_beginning = ftell (stdout);
    printf ("\004[Contents]\n%s\n\n", Topics);

    for (cnode = &nodes; cnode && cnode->node; cnode = cnode->next) {
	char *node = cnode->node;
	int heading_level = 0;
	while (*node == ' ') {
	    heading_level++;
	    node++;
	}
	if (*node)
	    printf ("  %*s\001 %s \002%s\003", heading_level, "", 
		    cnode->lname ? cnode->lname : node,
		    node);
	printf ("\n");

	free (cnode->node);
	if (cnode->lname)
	    free (cnode->lname);
	if (cnode != &nodes)
	    free (cnode);
    }

    file_end = ftell (stdout);
    fclose (file);

    Topics = malloc (file_end);
    file_fd = open (output, O_RDONLY);
    if (file_fd == -1)
	perror (output);
    if (read (file_fd, Topics, file_end) != file_end)
	perror (output);
    if (close (file_fd) == -1)
	perror (output);
    file_fd = open (output, O_WRONLY);
    write (file_fd, Topics + contents_beginning, file_end - contents_beginning);
    write (file_fd, Topics, contents_beginning);
    close (file_fd);

    return 0;
}
