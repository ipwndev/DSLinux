/* Nessus
 * Copyright (C) 1999 - 2003 Renaud Deraison
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
 * In addition, as a special exception, Renaud Deraison
 * gives permission to link the code of this program with any
 * version of the OpenSSL library which is distributed under a
 * license identical to that listed in the included COPYING.OpenSSL
 * file, and distribute linked combinations including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 *
 * nasl_plugins.c : Launch a NASL script
 *
 */

#include <includes.h>
#include <nasl.h>
#include "pluginload.h"
#include "plugs_hash.h"
#include "preferences.h"
#include "processes.h"
/*
 *  Initialize the nasl system
 */
static pl_class_t* nasl_plugin_init(struct arglist* prefs,
				    struct arglist* nasl) {
    return &nasl_plugin_class;
}



static void nasl_thread(struct arglist *);

/*
 *  Add *one* .nasl plugin to the plugin list
 */
static struct arglist *
nasl_plugin_add(folder, name, plugins, preferences)
     char * folder;
     char * name;
     struct arglist * plugins;
     struct arglist * preferences;
{
 char fullname[PATH_MAX+1];
 struct arglist *plugin_args;
 struct arglist * prev_plugin = NULL;
 char * lang = "english";
 char * md5;
 
 snprintf(fullname, sizeof(fullname), "%s/%s", folder, name);

 if(arg_get_type(preferences, "language")>=0)
  lang = arg_get_value(preferences, "language");

 md5 = file_hash(fullname);
 plugin_args = store_load_plugin(folder, name, md5, preferences);
 if(plugin_args == NULL )
 {
 plugin_args = emalloc(sizeof(struct arglist));
 arg_add_value(plugin_args, "preferences", ARG_ARGLIST, -1, (void*)preferences);
 
 if(execute_nasl_script(plugin_args, fullname, 1) < 0)
   {
   printf("%s could not be loaded\n", fullname);
   arg_set_value(plugin_args, "preferences", -1, NULL);
   arg_free_all(plugin_args);
   efree(&md5);
   return NULL;
  }
 plug_set_path(plugin_args, fullname);
 if(plug_get_id(plugin_args) > 0)
 	plugin_args = store_plugin(plugin_args, name, md5);
 }
 
 if(plug_get_id(plugin_args) == 0)
 {
  /* Discard invalid plugins */
  plugin_free(plugin_args);
  return NULL;
 }
 
 
 plug_set_launch(plugin_args, 0);
 prev_plugin = arg_get_value(plugins, name);
 if( prev_plugin == NULL )
  arg_add_value(plugins, name, ARG_ARGLIST, -1, plugin_args);
 else
 {
  plugin_free(prev_plugin);
  arg_set_value(plugins, name, -1, plugin_args);
 }
 
 efree(&md5);

 return plugin_args;
}

/*
 * Launch a NASL plugin
 */
int
nasl_plugin_launch(globals, plugin, hostinfos, preferences, kb, name, soc)
 	struct arglist * globals;
	struct arglist * plugin;
	struct arglist * hostinfos;
	struct arglist * preferences;
	struct arglist * kb;
	char * name;
	int soc;
{
 int timeout;
 int category = 0;
 nthread_t module;
 struct arglist * d = emalloc(sizeof(struct arglist));
 
 arg_add_value(plugin, "HOSTNAME", ARG_ARGLIST, -1, hostinfos);
 if(arg_get_value(plugin, "globals"))
   arg_set_value(plugin, "globals", -1, globals);
 else    
   arg_add_value(plugin, "globals", ARG_ARGLIST, -1, globals);
 
 
 arg_set_value(plugin, "preferences", -1, preferences);
 arg_add_value(plugin, "pipe", ARG_INT, sizeof(int), (void*)soc);
 arg_add_value(plugin, "key", ARG_ARGLIST, -1, kb);

 arg_add_value(d, "args", ARG_ARGLIST, -1, plugin);
 arg_add_value(d, "name", ARG_STRING, -1, name);
 
 category = plug_get_category(plugin); 
 timeout = preferences_plugin_timeout(preferences, plug_get_id(plugin));
 if( timeout == 0 )
 {
  if(category == ACT_SCANNER)
  	timeout = -1;
  else 
  	timeout = preferences_plugins_timeout(preferences);
 }

 module = create_process((process_func_t)nasl_thread, d); 
 arg_free(d);
 return module;
}


static void 
nasl_thread(g_args) 
 struct arglist * g_args;
{
 struct arglist * args = arg_get_value(g_args, "args");
 struct arglist * globals = arg_get_value(args, "globals");
 char * name = arg_get_value(g_args, "name");
 int soc = (int)arg_get_value(args, "pipe");
 int soc2 = (int)arg_get_value(args, "SOCKET");
 int i;
 
 
 if(preferences_benice(NULL))nice(-5);
 /* XXX ugly hack */
 arg_set_value(globals, "global_socket", sizeof(int), (void*)soc2);
 for(i=4;i<256;i++)
 {
  if( ( i != soc ) && ( i != soc2 ))
   close(i);
 }
#ifdef RLIMIT_RSS
 {
 struct rlimit rlim;
 getrlimit(RLIMIT_RSS, &rlim);
 rlim.rlim_cur = 1024*1024*20;
 rlim.rlim_max = 1024*1024*20;
 setrlimit(RLIMIT_RSS, &rlim);
 }
#endif

#ifdef RLIMIT_AS
 {
 struct rlimit rlim;
 getrlimit(RLIMIT_AS, &rlim);
 rlim.rlim_cur = 1024*1024*20;
 rlim.rlim_max = 1024*1024*20;
 setrlimit(RLIMIT_AS, &rlim);
 }
#endif

#ifdef RLIMIT_DATA
 {
 struct rlimit rlim;
 getrlimit(RLIMIT_DATA, &rlim);
 rlim.rlim_cur = 1024*1024*20;
 rlim.rlim_max = 1024*1024*20;
 setrlimit(RLIMIT_DATA, &rlim);
 }
#endif
 setproctitle("testing %s (%s)", (char*)arg_get_value(arg_get_value(args, "HOSTNAME"), "NAME"), (char*)arg_get_value(g_args, "name"));
 signal(SIGTERM, _exit);
 
 execute_nasl_script(args, name, 0);
}


pl_class_t nasl_plugin_class = {
    NULL,
    ".nasl",
    nasl_plugin_init,
    nasl_plugin_add,
    nasl_plugin_launch,
};
