/*
 * file_ascii.c -- generic xml import/export filters for hnb
 *
 * Copyright (C) 2001-2003 �yvind Kol�s <pippin@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xml_tok.h"

#include "cli.h"
#include "tree.h"

#include "file.h"
#include "prefs.h"
#include "query.h"
#include "util_string.h"

#define indent(count,char)	{int j;for(j=0;j<count;j++)fprintf(file,char);}

static int xml_cuddle = 0;

static char *xmlquote[]={
	"<","&lt;",
	">","&gt;",
	"&","&amp;",
	"\"","&quot;",
	"'","&apos;",
	NULL
};

static char *xmlunquote[]={
	"&lt;","<",
	"&gt;",">",
	"&amp;","&",
	"&quot;","\"",
	"&apos;","'",
	NULL
};

/* returns the first occurence of one of the needles, or 0 (termination)
   if not found, return 0*/
static int findchar (char *haystack, char *needles)
{
	int j = 0;
	int k;

	while (haystack[j]) {
		for (k = 0; k < strlen (needles) + 1; k++)
			if (haystack[j] == needles[k])
				return j;
		j++;
	}
	return 0;
}

static void xml_export_nodes (FILE * file, Node *node, int level)
{
	char tag[bufsize];
	int flags;
	char *data;

	static int no_quote = 0;

	while (node) {
		int data_start = 0;

		tag[0] = 0;
		flags = node_getflags (node);
		data = fixnullstring (node_get (node, TEXT));

		indent (level, "  ");

		if (data[0] == '<') {	/* calculate start tag, if any */
			strcpy (tag, data);
			data_start = findchar (tag, ">") + 1;
			tag[data_start] = 0;
			if (data[1] == '!' || data[1] == '?') {
				no_quote++;
			}
		}

		if (no_quote)
			fprintf (file, "%s%s", tag, &data[data_start]);
		else{
			char *quoted=string_replace(&data[data_start],xmlquote);
			fprintf (file, "%s%s", tag, quoted);
			free(quoted);
		}

		if (data[0] == '<') {	/* calculate end tag */
			strcpy (tag, data);
			tag[findchar (tag, " \t>") + 1] = 0;
			tag[findchar (tag, " \t>")] = '>';
			tag[0] = '/';
		}

		if (node_right (node)) {
			fprintf (file, "\n");
			xml_export_nodes (file, node_right (node), level + 1);
			indent (level, "  ");
			if (data[0] == '<') {
				if (data[1] == '!' && data[2] == '-') {
					fprintf (file, " -->\n");
				} else if (tag[1] != '?' && tag[1] != '!') {
					fprintf (file, "<%s\n", tag);
				} else {
					fprintf (file, "\n");
				}
			}
		} else {
			if (data[0] == '<' && data[strlen (data) - 2] != '/') {
				if (data[1] == '!' && data[2] == '-') {
					fprintf (file, " -->\n");
				} else if (tag[1] != '?' && tag[1] != '!') {
					fprintf (file, "<%s\n", tag);
				} else {
					fprintf (file, "\n");
				}
			} else
				fprintf (file, "\n");
		}
		if (data[0] == '<' && (data[1] == '!' || data[1] == '?')) {
			no_quote--;
		}

		node = node_down (node);
	}
}

static int export_xml (int argc, char **argv, void *data)
{
	Node *node = (Node *) data;
	char *filename = argc==2?argv[1]:"";
	FILE *file;

	if (!strcmp (filename, "-"))
		file = stdout;
	else
		file = fopen (filename, "w");
	if (!file) {
		cli_outfunf ("xml export, unable to open \"%s\"", filename);
		return (int) node;
	}

	xml_export_nodes (file, node, 0);

	if (file != stdout)
		fclose (file);

	cli_outfunf ("xml export, wrote data to \"%s\"", filename);


	return (int) node;
}

/* joins up tags with data if there is data as the first child
   of the tag.*/
static Node *xml_cuddle_nodes (Node *node)
{

	Node *tnode;
	char *tdata;
	char data[bufsize];

	tnode = node_root (node);

	while (tnode) {
		if (node_right (tnode)) {
			tdata = fixnullstring (node_get (node_right (tnode), TEXT));
			if (tdata[0] != '<') {	/* not a child tag */
				strcpy (data, fixnullstring (node_get (tnode, TEXT)));
				strcat (data, " ");
				strcat (data, tdata);
				node_set (tnode, TEXT, data);
				node_remove (node_right (tnode));
			}
		}
		tnode = node_recurse (tnode);
	}

	return (node);
}


static int import_xml (int argc, char **argv, void *data)
{
	Node *node = (Node *) data;
	char *filename = argc==2?argv[1]:"";
	char *rdata;
	int type;
	int level = 0;
	char nodedata[4096];
	xml_tok_state *s;
	import_state_t ist;
	int got_data = 0;

	FILE *file;

	nodedata[0] = 0;

	file = fopen (filename, "r");
	if (!file) {
		cli_outfunf ("xml import, unable to open \"%s\"", filename);
		return (int) node;
	}
	s = xml_tok_init (file);
	init_import (&ist, node);

	while (((type = xml_tok_get (s, &rdata)) != t_eof)) {
		if (type == t_error) {
			cli_outfunf ("xml import error, parsing og '%s', line:%i %s", filename,
						 s->line_no,rdata);
			fclose (file);
			return (int) node;
		}

		switch (type) {
			case t_prolog:
				sprintf (nodedata, "<?%s?>", rdata);
				import_node_text (&ist, level, nodedata);
				nodedata[0] = 0;
				got_data = 0;
				break;
			case t_dtd:
				sprintf (nodedata, "<!%s>", rdata);
				import_node_text (&ist, level, nodedata);
				nodedata[0] = 0;
				got_data = 0;
				break;
			case t_comment:
				sprintf (nodedata, "<!--%s-->", rdata);
				import_node_text (&ist, level, nodedata);
				break;
			case t_tag:
				if (got_data) {
					char *unquoted=string_replace(nodedata,xmlunquote);
					import_node_text (&ist, level, unquoted);
					free(unquoted);
					got_data = 0;
					nodedata[0] = 0;
				}
				sprintf (nodedata, "<%s", rdata);
				break;
			case t_att:
				sprintf (&nodedata[strlen (nodedata)], " %s=", rdata);
				break;
			case t_val:
				if (strchr (rdata, '"')) {
					sprintf (&nodedata[strlen (nodedata)], "'%s'", rdata);
				} else {
					if (strchr (rdata, '\'')) {
						sprintf (&nodedata[strlen (nodedata)], "\"%s\"",
								 rdata);
					} else {
						sprintf (&nodedata[strlen (nodedata)], "\"%s\"",
								 rdata);
					}
				}
				break;
			case t_endtag:
				sprintf (&nodedata[strlen (nodedata)], ">");

				import_node_text (&ist, level, nodedata);
				nodedata[0] = 0;
				level++;
				break;
			case t_closeemptytag:
				sprintf (&nodedata[strlen (nodedata)], "/>");

				import_node_text (&ist, level, nodedata);
				nodedata[0] = 0;
				break;
			case t_closetag:
				if (got_data) {
					char *unquoted=string_replace(nodedata,xmlunquote);
					import_node_text (&ist, level, unquoted);
					free(unquoted);
					got_data = 0;
					nodedata[0] = 0;
				}
				level--;
				sprintf (nodedata, "</%s>", rdata);
				nodedata[0] = 0;
				break;
			case t_whitespace:
				if (got_data) {
					strcpy (&nodedata[strlen (nodedata)], " ");
				}
				break;
			case t_word:
				strcpy (&nodedata[strlen (nodedata)], rdata);
				got_data = 1;
				break;
			case t_entity:
				got_data = 1;
				sprintf (&nodedata[strlen (nodedata)], "&%s;", rdata);
				break;
			default:
				break;
		}
	}

	if (node_getflag (node, F_temp))
		node = node_remove (node);	/* remove temporary node, if tree was empty */

	if (xml_cuddle)
		node = xml_cuddle_nodes (node);

	cli_outfunf ("xml import - imported \"%s\" %i lines", filename, s->line_no);
	xml_tok_cleanup (s);
	return (int) node;
}

/*
!init_file_xml();
*/
void init_file_xml ()
{
	cli_add_command ("export_xml", export_xml, "<filename>");
	cli_add_command ("import_xml", import_xml, "<filename>");
	cli_add_help ("export_xml",
				  "Exports the current node, it's siblings and all sublevels to 'filename' as if it was xml markup.\
(load an xml file with import_xml or hnb -x file.xml to see how it should be inside hnb.");
	cli_add_help ("import_xml",
				  "Imports 'filename' and inserts it's contents at the current level.");
	cli_add_int ("xml_cuddle", &xml_cuddle,
				 "join the data with nodes if no tags within tag");
}
