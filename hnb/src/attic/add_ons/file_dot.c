/*
!cli cli_add_command ("export_dot", export_dot, "<filename>");

!clid int export_dot ();
*/


#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "cli.h"
#include "tree.h"
#include "file.h"

#define indent(count,char)	{int j;for(j=0;j<count;j++)fprintf(file,char);}

static void dot_export_node (FILE * file, Node *node)
{
	Node *parent = node_left (node);

	if (parent) {
		fprintf (file, "\"%s\" -> \"%s\";\n", node_getdata (parent),
				 node_getdata (node));
	} else {
		fprintf (file, "\"%s\" -> \"%s\";\n", "hnb", node_getdata (node));
	}
}

int export_dot (char *params, void *data)
{
	Node *node = (Node *) data;
	char *filename = params;
	Node *tnode;
	int startlevel;
	FILE *file;

	file_error[0] = 0;

	if (!strcmp (filename, "-"))
		file = stdout;
	else
		file = fopen (filename, "w");
	if (!file) {
		sprintf (file_error, "dot export, unable to open \"%s\"", filename);
		return (int) node;
	}
	startlevel = nodes_left (node);

	tnode = node;

	fprintf (file, "digraph hnb{\nrankdir=LR;\n");

	while ((tnode != 0) & (nodes_left (tnode) >= startlevel)) {
		dot_export_node (file, tnode);

		tnode = node_recurse (tnode);
	}

	fprintf (file, "}\n");

	if (file != stdout)
		fclose (file);
	return (int) node;
}

