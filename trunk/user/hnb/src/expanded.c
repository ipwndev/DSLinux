/*
 * expanded.c -- functions to expand/unexpand nodes
 *
 * Copyright (C) 2001-2003 Øyvind Kolås <pippin@users.sourceforge.net>
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

#include "tree.h"
#include "cli.h"
#define NULL 0

#include "evilloop.h"
#include "ctype.h"
#include "ui_binding.h"


static int cmd_expand (int argc,char **argv, void *data)
{
	Node *pos = (Node *) data;
	if(argc==1){
		if(inputbuf[0] && isprint(lastbinding->key)){ /* evil workaround */
			if (lastbinding->key > 31 && lastbinding->key < 255) {	/*  input for buffer */
				inputbuf[strlen (inputbuf) + 1] = 0;
				inputbuf[strlen (inputbuf)] = lastbinding->key;
			}
			return (int)pos;
		}
		node_setflag(pos,F_expanded,1);
	} else if((!strcmp(argv[1],"-a"))||(!strcmp(argv[1],"--all"))){
		Node *tnode = node_root (pos);

		while (tnode) {
			node_setflag(tnode,F_expanded,1);
			tnode = node_recurse (tnode);
		}
		cli_outfun ("expanded all nodes");
	}
	return (int) pos;
}

static int cmd_collapse (int argc,char **argv, void *data)
{
	Node *pos = (Node *) data;
	if(argc==1){
		if(inputbuf[0] && isprint(lastbinding->key)){ /* evil workaround */
			if (lastbinding->key > 31 && lastbinding->key < 255) {	/*  input for buffer */
				inputbuf[strlen (inputbuf) + 1] = 0;
				inputbuf[strlen (inputbuf)] = lastbinding->key;
			}		
			return (int)pos;
		}
		node_setflag(pos,F_expanded,0);
	} else if((!strcmp(argv[1],"-a"))||(!strcmp(argv[1],"--all"))){
		Node *tnode = node_root (pos);

		while (tnode) {
			node_setflag(tnode,F_expanded,0);
			tnode = node_recurse (tnode);
		}
		cli_outfun ("collapsed all nodes");
	}
	
	return (int) pos;
}

/*
!init_expanded();
*/
void init_expanded ()
{
	cli_add_command ("expand", cmd_expand, "[--all|-a]");
	cli_add_help ("expand",
				  "expand the current entry, thus showing it's subentries, if the\
parameter -a is given, all nodes in the tree will be expaned");
	cli_add_command ("collapse", cmd_collapse, "");
	cli_add_help ("collapse",
				  "collapse the current entrys subentries, if the\
parameter -a is given, all nodes in the tree will be expaned");
}
