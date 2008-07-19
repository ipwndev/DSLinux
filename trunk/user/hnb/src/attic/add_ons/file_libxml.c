/*
!cli cli_add_command ("export_libxml", export_libxml, "<filename>");
!cli cli_add_command ("import_libxml", import_libxml, "<filename>");

!clid int import_libxml ();
!clid int export_libxml ();
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "cli.h"
#include "tree.h"
#include "file.h"

/* FIXME: the code in this file has not been updated yet */

/*prints a b chars to file */
#define indent(a,b)	{int j;for(j=0;j<a;j++)fprintf(file,b);}


#ifndef USE_LIBXML
int import_libxml(){
	return 0;
};
int export_libxml(){
	return 0;
};
#endif

#ifdef USE_LIBXML

#ifdef USE_LIBXML
#include <libxml/tree.h>
#include <libxml/parser.h>
#define XMLCHAR(n) (const xmlChar *)(const char *)n
static xmlDocPtr xmldoc = NULL;
#endif

void libxml_export_data (FILE * file, char *data)
{
	int i = 0;

	while (data[i]) {
		switch (data[i]) {
			case '<':
				fprintf (file, "&lt;");
				break;
			case '>':
				fprintf (file, "&gt;");
				break;
			case '&':
				fprintf (file, "&amp;");
				break;
			case '\'':
				fprintf (file, "&apos;");
				break;
			case '"':
				fprintf (file, "&quot;");
				break;
			default:
				fputc (data[i], file);
				break;
		}
		i++;
	}
	fprintf (file, "\n");
}

static char *libxml_export_node_pre (FILE * file, Node *node, int flags,
									 char *data_orig)
{
	int i = 1;
	int j;
	char *data;
	char prefix[] = "(hnbnode) ";
	unsigned char priority;

	if (data_orig[0] != '(' && flags & F_todo) {
		data = (char *) malloc (strlen (data_orig) + strlen (prefix) + 1);
		sprintf (data, "%s%s", prefix, data_orig);
	} else {
		data = strdup (data_orig);
	}
	if (data[0] == '(') {
		while (data[i] != ')') {
			if (data[i] == 0) {
				libxml_export_data (file, data);
				return data;
			}
			i++;
		}
		data[i] = 0;
		j = i + 1;
		while (data[j] == ' ' || data[j] == '\t') {
			j++;
		}
		priority = node_getpriority (node);
		fprintf (file, "<%s title=\"%s\" ", &data[1], &data[j]);
		if (flags & F_todo) {
			fprintf (file, "todo=\"%s\" ", (flags & F_done) ? "done" : "");
		}
		if (priority != 0) {
			fprintf (file, "priority=\"%i\" ", priority);
		}
		fprintf (file, ">\n");
	} else {
		libxml_export_data (file, data);
	}
	return data;
}

static void libxml_export_node_post (FILE * file, int flags, char *data)
{
	int i = 1;

	if (data[0] == '(') {
		while (data[i]) {
			if (data[i] == ' ' || data[i] == '\t') {
				data[i] = 0;
				break;
			}
			i++;
		}
		fprintf (file, "</%s>\n", &data[1]);
	}
}

void libxml_rec (Node *node, FILE * file)
{
	int i;
	Node *tnode;
	int flags;
	char *data;
	char *moddata;

	flags = node_getflags (node);
	data = node_getdata (node);

	moddata = libxml_export_node_pre (file, node, flags, data);
	tnode = node_right (node);
	while (tnode) {
		libxml_rec (tnode, file);
		tnode = node_down (tnode);
	}
	libxml_export_node_post (file, flags, moddata);
	free (moddata);
}

void libxml_export (Node *node, char *filename)
{
	FILE *file;

	file_error[0] = 0;
	file = fopen (filename, "w");
	if (!file)
		return -1;
	while (node) {
		libxml_rec (node, file);
		node = node_down (node);
	}
	fclose (file);
	return 0;
}

Node *libxml_populate (import_state_t * is, xmlNodePtr root, int level)
{
	xmlNodePtr cur = root->children;
	xmlAttrPtr prop;
	char *data, *s;
	int flags = 0;
	char attrstring[bufsize];
	char *sbuf = NULL;
	int len = 1;
	static char *notitle = "";
	static char *delim = "\n";
	unsigned char priority = 0;

	data = notitle;
	while (cur) {
		if (!xmlIsBlankNode (cur)) {
			if (xmlNodeIsText (cur)) {
				s = strdup ((char *) cur->content);
				data = strtok (s, delim);
				while (data) {
					import_node (is, level, flags, 0, data);
					data = strtok (NULL, delim);
				}
				free (s);
			} else {



				priority = 0;
				sbuf = (char *) malloc (1);
				sbuf[0] = 0;
				prop = cur->properties;
				while (prop) {
					if (!strcmp ("title", prop->name)) {
						data = xmlGetProp (cur, XMLCHAR ("title"));
					} else if (!strcmp ("priority", prop->name)) {
						priority = (unsigned char)
							atol (xmlGetProp (cur, XMLCHAR ("priority")));
					} else if (!strcmp ("todo", prop->name)) {
						flags |= F_todo;
						if (!strcmp
							("done", xmlGetProp (cur, XMLCHAR ("todo")))) {
							flags |= F_done;
						}
					} else {
						snprintf (attrstring, bufsize, " %s=\"%s\"",
								  prop->name, xmlGetProp (cur,
														  XMLCHAR (prop->
																   name)));
						len += strlen (attrstring);
						sbuf = (char *) realloc (sbuf, len);
						strcat (sbuf, attrstring);
					}
					prop = prop->next;
				}


#if 0
				if (xmlGetProp (cur, XMLCHAR ("title"))) {
					data = xmlGetProp (cur, XMLCHAR ("title"));
				} else {
					data = notitle;
				}
				if (xmlGetProp (cur, XMLCHAR ("todo"))) {
					flags |= F_todo;
					if (!strcmp ("done", xmlGetProp (cur, XMLCHAR ("todo")))) {
						flags |= F_done;
					}
				}
#endif

				s = (char *) malloc (strlen (cur->name) + strlen (data) +
									 strlen (sbuf) + 4);
				if (strcmp ("hnbnode", cur->name)) {
					sprintf (s, "(%s%s) %s", cur->name, sbuf, data);
				} else {
					sprintf (s, "%s", data);
				}
				import_node (is, level, flags, priority, s);
				free (s);
				flags = 0;
			}
			libxml_populate (is, cur, level + 1);
		}
		cur = cur->next;
	}

	return;
}

Node *libxml_import (Node *node, char *filename)
{
	FILE *file;
	char *data;
	unsigned int bsize;
	import_state_t ist;

	file_error[0] = 0;
	bsize = 6;
	data = (char *) malloc (bsize + 1);
	sprintf (data, "<hnb>\n");
	file = fopen (filename, "r");
	if (file == NULL) {
		return node;
	}


	while (!feof (file) && !ferror (file)) {
		data = (char *) realloc ((void *) data, bsize + bufsize);
		bsize += fread (&data[bsize], 1, bufsize, file);
	}
	fclose (file);

	data = (char *) realloc ((void *) data, bsize + 8);
	sprintf (&data[bsize], "</hnb>\n");
	bsize += 8;

	xmldoc = xmlParseMemory (data, bsize);

	init_import (&ist, node);

	libxml_populate (&ist, xmldoc->children, 0);
	if (node_getflags (node) & F_temp)
		node = node_remove (node);
	return (node);
}

#endif
