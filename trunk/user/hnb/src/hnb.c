/*
 * hnb.c -- the main app, of hierarchical notebook, an personal database
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

/*
	TODO: noder som forsvinner ved:
		std. oppretting
		redigering
		g� til parent
		
		--
		sannsynlig grunn: feil h�ndtering av temporary attributte
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tree.h"

#include "ui_cli.h"
#include "cli.h"
#include "ui.h"
#include "prefs.h"
#include "file.h"
#include "evilloop.h"

static void usage (const char *av0)
{
	fprintf (stderr,
			 "\nusage: %s [database] [options] [command [command] ..]\n",
			 av0);
	fprintf (stderr, "\n\
Hierarchical NoteBook by �yvind Kol�s <pippin@users.sourceforge.net>\n\
It is distributed under the GNU General Public License\n\
\n\
default database: '%s'\n", prefs.default_db_file);
	fprintf (stderr, "\n\
Options:\n\
\n\
\t-h --help     this message\n\
\t-v --version  prints the version\n\
\t-t --tutorial loads the tutorial instead of a database\n\
\n\
\t-a --ascii    load ascii ascii\n\
\t   --hnb      load hnb DTD\n\
\t-o --opml     load OPML DTD\n\
\t-x --xml      load general xml\n");
#ifdef USE_LIBXML
	fprintf (stderr, "\t-s --stylized load stylized xml (using libxml2)\n");
#endif
	fprintf (stderr, "\n\
\t-rc <file>        specify other config file\n\
\t-ui <interface>   interface to use, ( curses(default) or cli)\n\
\t-e                execute commands\n\
\n\n");
}

void init_subsystems ();


int main (int argc, char **argv)
{
	Node *pos;
	int argno;

	/* current commandline argument in focus */

	struct {					/* initilaized defaults */
		int version;
		int usage;
		int def_db;
		char format[64];
		int ui;
		int tutorial;
		char *dbfile;
		char *rcfile;
		char *cmd;
	} cmdline = {
		0,						/* version */
			0,					/* usage */
			1,					/* load default db */
			"",					/*format to load by default */
			1,					/* ui */
			0,					/* tutorial */
	NULL, NULL, NULL};
	{							/*parse commandline */
		for (argno = 1; argno < argc; argno++) {
			if (!strcmp (argv[argno], "-h")
				|| !strcmp (argv[argno], "--help")) {
				cmdline.usage = 1;
			} else if (!strcmp (argv[argno], "-v")
					   || !strcmp (argv[argno], "--version")) {
				cmdline.version = 1;
			} else if (!strcmp (argv[argno], "-t")
					   || !strcmp (argv[argno], "--tutorial")) {
				cmdline.tutorial = 1;
			} else if (!strcmp (argv[argno], "-a")
					   || !strcmp (argv[argno], "--ascii")) {
				strcpy(cmdline.format,"ascii");
			} else if (!strcmp (argv[argno], "-hnb")
					   || !strcmp (argv[argno], "--hnb")) {
				strcpy(cmdline.format,"hnb");
			} else if (!strcmp (argv[argno], "-o")
					   || !strcmp (argv[argno], "-opml")
					   || !strcmp (argv[argno], "--opml")) {
				strcpy(cmdline.format,"opml");
			} else if (!strcmp (argv[argno], "-x")
					   || !strcmp (argv[argno], "-gx")
					   || !strcmp (argv[argno], "--xml")) {
				strcpy(cmdline.format,"xml");
#ifdef USE_LIBXML
			} else if (!strcmp (argv[argno], "-s")
					   || !strcmp (argv[argno], "-sx")
					   || !strcmp (argv[argno], "--stylized")) {
				strcpy(cmdline.format,"sxml");
#endif
			} else if (!strcmp (argv[argno], "-ui")) {
				if (!strcmp (argv[++argno], "curses")) {
					cmdline.ui = 1;
				} else if (!strcmp (argv[argno], "cli")) {
					cmdline.ui = 2;
				} else if (!strcmp (argv[argno], "gtk")
						   || !strcmp (argv[argno], "gtk+")) {
					cmdline.ui = 3;
				} else if (!strcmp (argv[argno], "keygrab")) {
					cmdline.ui = 4;
				} else {
					fprintf (stderr, "unknown interface %s\n", argv[argno]);
					exit (1);
				}
			} else if (!strcmp (argv[argno], "-rc")) {
				cmdline.rcfile = argv[++argno];
			} else if (!strcmp (argv[argno], "-e")) {
				/* actually just a dummy option to specify default db */
				if (!cmdline.dbfile) {
					cmdline.def_db = 1;
					cmdline.dbfile = (char *) -1;
				}
			} else {
				if (argv[argno][0] == '-') {
					fprintf (stderr, "unknown option %s\n", argv[argno]);
					exit (1);
				} else if (!cmdline.dbfile) {
					cmdline.dbfile = argv[argno];
					cmdline.def_db = 0;
				} else {
					cmdline.cmd = argv[argno];
					cmdline.ui = 0;
					argno++;
					break;		/* stop processing cmd args */
				}
			}
		}
	}

	init_subsystems ();

	if (cmdline.usage) {
		usage (argv[0]);
		exit (0);
	}

	if (cmdline.version) {
		fprintf (stderr, "%s %s\n", PACKAGE, VERSION);
		exit (0);
	}

	if (cmdline.rcfile) {
		strcpy (prefs.rc_file, cmdline.rcfile);
	}


	if (!file_check (prefs.rc_file)) {
		write_default_prefs ();
		fprintf (stderr, "created %s for hnb preferences file\n",
				 prefs.rc_file);
		sleep (1);
	}

	if (cmdline.ui == 1)
		ui_init ();

	load_prefs ();


	/* ovveride the prefs with commandline specified options */
	if (cmdline.tutorial)
		prefs.tutorial = 1;
	if (cmdline.format[0] ) {	/* format specified */
		strcpy(prefs.format, cmdline.format);
	}

	if (cmdline.def_db) {
		strcpy (prefs.db_file, prefs.default_db_file);
		if (!file_check (prefs.db_file))
			prefs.tutorial = 2;
	} else {
		strcpy (prefs.db_file, cmdline.dbfile);
	}

	pos = tree_new ();

	if (!prefs.tutorial) {
		int oldpos = -1;

		if (!strcmp(prefs.format,"hnb") || 
		!strcmp(prefs.format,"opml") ||
		!strcmp(prefs.format,"xml") 
		) {

			if (!xml_check (prefs.db_file)) {
				fprintf (stderr,
						 "%s does not seem to be a xml file, aborting.\n",
						 prefs.db_file);
				if (ui_inited)
					ui_end ();
				exit (1);
			}
			if (prefs.savepos)
				oldpos = xml_getpos (prefs.db_file);
		}


		{
			char buf[4096];

			sprintf (buf, "import_%s %s", prefs.format,  prefs.db_file);
			pos = docmd (pos, buf);
		}

		if (oldpos != -1) {
			while (oldpos--)
				pos = node_recurse (pos);
		}
	}


	if (prefs.tutorial) {
		if (prefs.tutorial != 2)
			prefs.db_file[0] = (char) 255;	/* disable saving */
		pos = docmd (pos, "import_help");
		pos = docmd (pos, "status ''");
		pos = docmd (pos, "status 'navigate the documentation with your cursor keys'");
	}


	switch (cmdline.ui) {
		case 1:
			pos = evilloop (pos);
			ui_end ();
			break;
		case 0:
			pos = (Node *) cli_docmd (cmdline.cmd, pos);
			while (argno < argc) {
				pos = (Node *) cli_docmd (argv[argno++], pos);
			}
			break;
		case 2:
			pos = cli (pos);
			break;
		case 3:
			printf ("gtk+ interface not implemented\n");
			break;
		case 4:
			ui_init ();
			{
				int c = 0;

				while (c != 'q') {
					char buf[100];

					c = getch ();
					sprintf (buf, "[%i] [%c]\n", c, c);
					addstr (buf);
			}}
			ui_end ();
			break;
	}

	cli_cleanup ();
	tree_free (pos);

	return 0;
}
