/*
!cli cli_add_command ("export_slides", export_slides, "<filename>");

!clid int export_slides ();
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "cli.h"
#include "tree.h"
#include "file.h"

#define indent(count,char)	{int j;for(j=0;j<count;j++)fprintf(file,char);}

int export_slides (char *params, void *data)
{
	Node *pos = (Node *) data;
	char *filename = params;
	Node *tnode;
	int level, flags, startlevel, lastlevel /*, cnt */ ;
	char *cdata;
	FILE *file;

	file_error[0] = 0;
	if (!strcmp (filename, "-"))
		file = stdout;
	else
		file = fopen (filename, "w");
	if (!file) {
		sprintf (file_error, "export slides unable to open \"%s\"", filename);
		return (int) pos;
	}

	startlevel = nodes_left (pos);

	tnode = node_top (pos);
	lastlevel = 0;
	fprintf (file, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n\
<HTML>\n\
<HEAD>\n\
	<TITLE>tree exported from hnb</TITLE>\n\
</HEAD>\n\
<BODY>\n\
--\n");
	while ((tnode != 0) & (nodes_left (tnode) >= startlevel)) {
		level = nodes_left (tnode) - startlevel;
		flags = node_getflags (tnode);
		cdata = node_getdata (tnode);

		if (cdata[0] != 0) {
			if (level == 0)
				fprintf (file, "--\n");
			fprintf (file, "<h%i>%s%s</h%i>\n", level + 1,
					 (flags & F_todo ? (flags & F_done ? "[X] " : "[&nbsp] ")
					  : ""), cdata, level + 1);
		} else {
			fprintf (file, "--\n");
		}

		lastlevel = level;
		tnode = node_recurse (tnode);
	}

	fprintf (file, "</BODY></HTML>");
	if (file != stdout)
		fclose (file);

	return (int) pos;
}
