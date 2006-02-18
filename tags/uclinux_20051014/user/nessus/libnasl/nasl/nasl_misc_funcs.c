/* Nessus Attack Scripting Language 
 *
 * Copyright (C) 2002 - 2003 Michel Arboi and Renaud Deraison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Renaud Deraison and Michel Arboi
 * give permission to link the code of this program with any
 * version of the OpenSSL library which is distributed under a
 * license identical to that listed in the included COPYING.OpenSSL
 * file, and distribute linked combinations including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 *
 */
 /*
  * This file contains all the misc. functions found in NASL
  */
#include <includes.h>

#include "nasl_tree.h"
#include "nasl_global_ctxt.h"
#include "nasl_func.h"
#include "nasl_var.h"
#include "nasl_lex_ctxt.h"
#include "exec.h"  

#include "strutils.h"
#include "nasl_packet_forgery.h"
#include "nasl_debug.h"
#include "nasl_misc_funcs.h"


/*---------------------------------------------------------------------*/
tree_cell * nasl_rand(lex_ctxt * lexic)
{
 tree_cell * retc;
 retc = alloc_tree_cell(0, NULL);
 retc->type = CONST_INT;
 retc->x.i_val = lrand48();
 return retc;
}

/*---------------------------------------------------------------------*/
tree_cell * nasl_usleep(lex_ctxt * lexic)
{
 int slp = get_int_var_by_num(lexic, 0, 0);
 usleep(slp);
 return FAKE_CELL;
}

tree_cell * nasl_sleep(lex_ctxt * lexic)
{
 int slp = get_int_var_by_num(lexic, 0, 0);
 sleep(slp);
 return FAKE_CELL;
}


/*---------------------------------------------------------------------*/

tree_cell * nasl_ftp_log_in(lex_ctxt * lexic)
{
 char * u, *p;
 int soc;
 tree_cell *retc;
 int res;

 soc = get_int_local_var_by_name(lexic, "socket", 0);
 if(soc <= 0)
	 return NULL;

 u = get_str_local_var_by_name(lexic, "user");
 if( u == NULL )
	 u = "";
 
 p = get_str_local_var_by_name(lexic, "pass");
 if( p == NULL )
	 p = "";

 res = ftp_log_in(soc, u, p) == 0;
 
 retc = alloc_tree_cell(0, NULL);
 retc->type = CONST_INT;
 retc->x.i_val = res;

 return retc;
}

tree_cell * nasl_ftp_get_pasv_address(lex_ctxt * lexic)
{
 int soc; 
 struct sockaddr_in addr;
 tree_cell * retc; 

 soc = get_int_local_var_by_name(lexic, "socket", 0);
 if(soc <= 0)
	 return NULL;

 bzero(&addr, sizeof(addr));
 ftp_get_pasv_address(soc, &addr);

 retc = alloc_tree_cell(0, NULL);
 retc->type = CONST_INT;
 retc->x.i_val = htons(addr.sin_port);
 return retc;
}

/*---------------------------------------------------------------------*/

tree_cell * nasl_telnet_init(lex_ctxt * lexic)
{
 int soc = get_int_var_by_num(lexic, 0, -1);
 int opts;				/* number of options recorded */
 unsigned char buffer[1024];
#define iac buffer[0]
#define code buffer[1]
#define option buffer[2]
 tree_cell * retc;
 int n = 0, n2;

 if(soc <= 0 )
 {
 	nasl_perror(lexic, "Syntax error in the telnet_init() function\n");
	nasl_perror(lexic, "Correct syntax is : output = telnet_init(<socket>)\n");
 	return NULL;
}

 iac = 255;
 opts = 0;
 while(iac == 255)
 {
  n = read_stream_connection_min(soc, buffer, 3, 3);
  if((iac!=255)||(n<=0)||(n!=3))break;
  if((code == 251)||(code == 252))code = 254; /* WILL , WONT -> DON'T */
  else if((code == 253)||(code == 254))code = 252; /* DO,DONT -> WONT */
  write_stream_connection(soc, buffer,3);
  opts++;
  if (opts>100) break;
 }
 if (n <= 0)
  {
   if (opts == 0)
     return NULL;
   else
     n = 0;
  }

 if (opts>100)				/* remote telnet server is crazy */
  {
	nasl_perror(lexic, "More than 100 options received by telnet_init() function! exiting telnet_init.\n");
	return NULL;
  }

 n2 = read_stream_connection(soc, buffer + n,  sizeof(buffer) - n);
 if (n2 > 0)
   n += n2;
 retc = alloc_typed_cell(CONST_DATA);
 retc->size = n;
 retc->x.str_val = strndup(buffer, n);
#undef iac
#undef data
#undef option

  return retc;
}

/*---------------------------------------------------------------------*/

tree_cell * nasl_start_denial(lex_ctxt * lexic)
{
 struct arglist * script_infos = lexic->script_infos;
 int to = lexic->recv_timeout;
 int port = plug_get_host_open_port(script_infos);
 int soc;
 int alive = 0;
 tree_cell * p;
  
 if(port)
 {
  soc = open_stream_connection(script_infos, port, NESSUS_ENCAPS_IP, to);
  if(soc>=0)
  {
   if(arg_get_value(script_infos, "denial_port") != 0)
    arg_set_value(script_infos, "denial_port", sizeof(int), (void*)port);
   else
    arg_add_value(script_infos, "denial_port", ARG_INT, sizeof(int), (void*)port);
    
   close_stream_connection(soc);
  
   return FAKE_CELL;
  }
 }

 p = nasl_tcp_ping(lexic);
 if (p != NULL) alive = p->x.i_val;

 if(arg_get_value(script_infos, "tcp_ping_result") != 0)
  arg_set_value(script_infos, "tcp_ping_result", sizeof(int), (void*)alive);
 else
  arg_add_value(script_infos, "tcp_ping_result", ARG_INT, sizeof(int), (void*)alive);
 
 deref_cell(p);

 return FAKE_CELL;
}

tree_cell * nasl_end_denial(lex_ctxt * lexic)
{
 int port = (int)arg_get_value(lexic->script_infos, "denial_port");
 int soc;
 int to = lexic->recv_timeout;
 struct arglist * script_infos = lexic->script_infos;
 tree_cell * retc = NULL;
 
 /* 
  * We must wait the time the DoS does its effect
  */
 sleep(10);

 if(!port)
 {
  int ping = (int)arg_get_value(script_infos, "tcp_ping_result");
  
  if(ping) return nasl_tcp_ping(lexic);
  else
    {
      retc = alloc_tree_cell(0, NULL);
      retc->type = CONST_INT;
      retc->x.i_val = 1;
      return retc;
    }
 }
 else 
 {
   retc = alloc_tree_cell(0, NULL);
   retc->type = CONST_INT;

 soc = open_stream_connection(script_infos, port, NESSUS_ENCAPS_IP, to);
 if(soc > 0)
 {
  /* Send some data */
#define BOGUS "are you dead ?"
  if((nsend(soc, BOGUS, sizeof(BOGUS)-1, 0))>=0)
   {
   retc->x.i_val = 1;
   close_stream_connection(soc);
   return retc;
   }
  }
 }

   retc->x.i_val = 0;
   return retc;
 }
 
 
/*---------------------------------------------------------------------*/
 
tree_cell* nasl_dump_ctxt(lex_ctxt* lexic)
{
  dump_ctxt(lexic->up_ctxt);
  return FAKE_CELL;
}





tree_cell*	nasl_do_exit(lex_ctxt* lexic)
{ 
  int		x = get_int_var_by_num(lexic, 0, 0);
  tree_cell	*retc = alloc_tree_cell(0, NULL);
  retc->type = CONST_INT;
  retc->x.i_val = x;

  while (lexic != NULL)
    {
      lexic->ret_val = retc;
      ref_cell(retc);
      lexic = lexic->up_ctxt;
    }
  return retc;
}



/*---------------------------------------------------------------------*/

tree_cell* nasl_isnull(lex_ctxt* lexic)
{
  int		t;
  tree_cell	*retc;

  t = get_var_type_by_num(lexic, 0);
  retc = alloc_tree_cell(0, NULL);
  retc->type = CONST_INT;
  retc->x.i_val = (t == VAR2_UNDEF);
  return retc;
}

/*
 * This function takes any kind & any number of arguments and makes
 * an array from them.
 * If an argument is an array, its index are lost
 */

tree_cell*
nasl_make_list(lex_ctxt* lexic)
{
  tree_cell	*retc = NULL;
  int		i, j, vi;
  anon_nasl_var	*v;
  named_nasl_var *vn;
  nasl_array	*a, *a2;
  

  retc = alloc_tree_cell(0, NULL);
  retc->type = DYN_ARRAY;
  retc->x.ref_val = a = emalloc(sizeof(nasl_array));

  for (i = vi = 0; 
       (v = nasl_get_var_by_num(&lexic->ctx_vars, vi, 0)) != NULL;
       vi ++)
    {
      switch (v->var_type)
	{
	case VAR2_INT:
	case VAR2_STRING:
	case VAR2_DATA:
	  add_var_to_list(a, i ++, v);
	  break;

	case VAR2_ARRAY:
	  a2 = &v->v.v_arr;

	  for (j = 0; j < a2->max_idx; j ++)
	    if (add_var_to_list(a, i, a2->num_elt[j]) >= 1)
	      i ++;

	  if (a2->hash_elt != NULL)
	    {
#if NASL_DEBUG > 1
	      nasl_perror(lexic, "make_list: named arguments in array have no order\n");
#endif
	      for (j = 0; j < VAR_NAME_HASH; j++)
		for (vn = a2->hash_elt[j]; vn != NULL; vn = vn->next_var)
		  if (vn->u.var_type != VAR2_UNDEF)
		    if (add_var_to_list(a, i , &vn->u) >= 1)
		      i ++;
	    }

	  break;

	case VAR2_UNDEF:
	  nasl_perror(lexic, "nasl_make_list: undefined variable #%d skipped\n", i);
	  continue;

	default:
	  nasl_perror(lexic, "nasl_make_list: unhandled variable type 0x%x - skipped\n", v->var_type);
	  continue;
	}
    }

  return retc;
}

/*
 * This function takes any _even_ number of arguments and makes
 * an array from them. In each pair, the 1st argument is the index, the 
 * 2nd the value.
 * Illegal types are droped with a warning
 */

tree_cell*
nasl_make_array(lex_ctxt* lexic)
{
  tree_cell	*retc = NULL;
  int		i, vi;
  anon_nasl_var	*v, *v2;
  nasl_array	*a;


  retc = alloc_tree_cell(0, NULL);
  retc->type = DYN_ARRAY;
  retc->x.ref_val = a = emalloc(sizeof(nasl_array));

  i = vi = 0;
  while ((v = nasl_get_var_by_num(&lexic->ctx_vars, vi ++, 0)) != NULL)
    {
      v2 = nasl_get_var_by_num(&lexic->ctx_vars, vi ++, 0);
      if (v2 == NULL)
	{
	  nasl_perror(lexic, "make_array: odd number (%d) of argument?\n", vi);
	  break;
	}

      switch (v2->var_type)
	{
	case VAR2_INT:
	case VAR2_STRING:
	case VAR2_DATA:
	  switch (v->var_type)
	    {
	    case VAR2_INT:
	      add_var_to_list(a, v->v.v_int, v2);
	      break;
	    case VAR2_STRING:
	    case VAR2_DATA:
	      add_var_to_array(a, (char*)var2str(v) , v2);
	      break;
	    }
	  break;
	case VAR2_UNDEF:
	default:
	  nasl_perror(lexic, "make_array: bad value type %d for arg #%d\n",
		      v2->var_type, vi);
	  break;
	}
    }

  return retc;
}


tree_cell*
nasl_keys(lex_ctxt* lexic)
{
  tree_cell		*retc = NULL;
  anon_nasl_var		*v, myvar;
  named_nasl_var	*vn;
  nasl_array		*a, *a2;
  int		i, j, vi;

  retc = alloc_tree_cell(0, NULL);
  retc->type = DYN_ARRAY;
  retc->x.ref_val = a2 = emalloc(sizeof(nasl_array));

  bzero(&myvar, sizeof(myvar));

  for (i = vi = 0; 
       (v = nasl_get_var_by_num(&lexic->ctx_vars, vi, 0)) != NULL;
       vi ++)
    {
      if (v->var_type == VAR2_ARRAY)
	{
	  a = &v->v.v_arr;
	  /* First the numerical index */
	  for (j = 0; j < a->max_idx; j ++)
	    if (a->num_elt[j] != NULL && a->num_elt[j]->var_type != VAR2_UNDEF)
	      {
		myvar.var_type = VAR2_INT;
		myvar.v.v_int = j;
		add_var_to_list(a2, i ++, &myvar);
	      }
	  /* Then the string index */
	  if (a->hash_elt != NULL)
	    for (j = 0; j < VAR_NAME_HASH; j++)
	      for (vn = a->hash_elt[j]; vn != NULL; vn = vn->next_var)
		if (vn->u.var_type != VAR2_UNDEF)
		  {
		    myvar.var_type = VAR2_STRING;
		    myvar.v.v_str.s_val = vn->var_name;
		    myvar.v.v_str.s_siz = strlen(vn->var_name);
		    add_var_to_list(a2, i ++, &myvar);
		  }
	}
      else
	nasl_perror(lexic, "nasl_keys: bad variable #%d skipped\n", vi);
    }
	 
  return retc;
}

tree_cell*
nasl_max_index(lex_ctxt* lexic)
{
  tree_cell	*retc;
  anon_nasl_var	*v;
  nasl_array	*a;

  v = nasl_get_var_by_num(&lexic->ctx_vars, 0, 0);
  if (v == NULL)
    return NULL;
  if (v->var_type != VAR2_ARRAY)
    return NULL;

  a = &v->v.v_arr;

  retc = alloc_tree_cell(0, NULL);
  retc->type = CONST_INT;
  retc->x.i_val = array_max_index(a);

  return retc;
}

tree_cell*
nasl_typeof(lex_ctxt* lexic)
{
  tree_cell	*retc;
  anon_nasl_var	*u;
  const char*	s;

  retc = alloc_tree_cell(0, NULL); retc->type = CONST_DATA;
  u = nasl_get_var_by_num(&lexic->ctx_vars, 0, 0);

  if (u == NULL)
    s = "null";
  else
    switch (u->var_type)
      {
      case VAR2_UNDEF:
	s= "undef";
	break;
      case VAR2_INT:
	s = "int";
	break;
      case VAR2_STRING:
	s = "string";
	break;
      case VAR2_DATA:
	s = "data";
	break;
      case VAR2_ARRAY:
	s = "array";
	break;
      default:
	s = "unknown";
	break;
      }
  retc->size = strlen(s);
  retc->x.str_val = emalloc(retc->size);
  strcpy(retc->x.str_val, s);
  return retc;
}
    
tree_cell*
nasl_defined_func(lex_ctxt* lexic)
{
  void		*f; 
  char		*s;
  tree_cell	*retc;

  s = get_str_var_by_num(lexic, 0);
  if (s == NULL)
    {
      nasl_perror(lexic, "defined_func: missing parameter\n");
      return NULL;
    }

  f = get_func_ref_by_name(lexic, s);
  retc = alloc_tree_cell(0, NULL);
  retc->type = CONST_INT;
  retc->x.i_val = (f != NULL);
  return retc;
}

/* Sorts an array */

static lex_ctxt	*mylexic = NULL;

static int
var_cmp(const void * a, const void * b)
{
  anon_nasl_var ** pv1 = (anon_nasl_var **)a, ** pv2 = (anon_nasl_var**)b;
  tree_cell	*t1, *t2;

  t1 = var2cell((anon_nasl_var*)*pv1);
  t2 = var2cell((anon_nasl_var*)*pv2);
  return cell_cmp(mylexic, t1, t2);
}

tree_cell*
nasl_sort_array(lex_ctxt* lexic)
{
  tree_cell	*retc = NULL;
  nasl_array		*a;

  if (mylexic != NULL)
    {
      nasl_perror(lexic, "sort: this function is not reentrant!\n");
      return NULL;
    }
  mylexic = lexic;
  retc = nasl_make_list(lexic);
  if (retc != NULL)
    {
      a = retc->x.ref_val;
      qsort(a->num_elt, a->max_idx, sizeof(a->num_elt[0]), var_cmp);
    }
  mylexic = NULL;
  return retc;
}
