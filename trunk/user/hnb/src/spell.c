/*
 * spell.c -- spell checking for hnb
 *
 * Copyright (C) 2003 �yvind Kol�s <pippin@users.sourceforge.net>
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

/**************/
#include "tree.h"
#include "cli.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

static char spell_command[255] = "ispell";

#include <fcntl.h>

/*
 * this is simplistic approach,.. should perhaps have another one that checks for 
 * url/email address substring,.. and launches an app based on that?
 *
 */
static void spell_node (Node *node)
{
	char tempfilename[32] = "/tmp/hnb-XXXXXX";
	char commandline[255];
	char corrected[4096];
	int ui_was_inited = ui_inited;
	int tempfile = mkstemp (tempfilename);

	write (tempfile, fixnullstring (node_get (node, TEXT)),
		   strlen (node_get (node, TEXT)));
	sprintf (commandline, "%s %s", spell_command, tempfilename);
	if (ui_was_inited)
		ui_end ();
	system (commandline);
	close (tempfile);
	tempfile = open (tempfilename, O_RDONLY);

	{
		int len = read (tempfile, corrected, sizeof (corrected));

		corrected[len] = 0;
		node_set (node, TEXT, corrected);
	}

	close (tempfile);
	unlink (tempfilename);
	if (ui_was_inited)
		ui_init ();
}

static int spell_cmd (int argc, char **argv, void *data)
{
	Node *pos = (Node *) data;

	if (argc==2 && (!strcmp (argv[1], "-r"))) {
		int startlevel;
		Node *node = pos;

		startlevel = nodes_left (node);

		while ((node != 0) && (nodes_left (node) >= startlevel)) {
			spell_node (node);
			node = node_recurse (node);
		}
	} else {
		spell_node (pos);
	}
	return (int) pos;
}

/*
!init_spell();
*/
void init_spell ()
{
	cli_add_command ("spell", spell_cmd, "[-r]");
	cli_add_help ("spell",
				  "Spellchecks the current node, or all children and following siblings recursively (if -r specified), using the command defined in 'spell_command'");
	cli_add_string ("spell_command", spell_command,
					"Command executed when spell checking a node, a temporary file is written and passed as an argument");
}
