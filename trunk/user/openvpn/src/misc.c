/*
 *  OpenVPN -- An application to securely tunnel IP networks
 *             over a single TCP/UDP port, with support for SSL/TLS-based
 *             session authentication and key exchange,
 *             packet encryption, packet authentication, and
 *             packet compression.
 *
 *  Copyright (C) 2002-2005 OpenVPN Solutions LLC <info@openvpn.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program (see the file COPYING included with this
 *  distribution); if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef WIN32
#include "config-win32.h"
#else
#include "config.h"
#endif

#include "syshead.h"

#include "buffer.h"
#include "misc.h"
#include "tun.h"
#include "error.h"
#include "thread.h"
#include "otime.h"
#include "plugin.h"
#include "options.h"
#include "manage.h"

#include "memdbg.h"

/* Redefine the top level directory of the filesystem
   to restrict access to files for security */
void
do_chroot (const char *path)
{
  if (path)
    {
#ifdef HAVE_CHROOT
      const char *top = "/";
      if (chroot (path))
	msg (M_ERR, "chroot to '%s' failed", path);
      if (openvpn_chdir (top))
	msg (M_ERR, "cd to '%s' failed", top);
      msg (M_INFO, "chroot to '%s' and cd to '%s' succeeded", path, top);
#else
      msg (M_FATAL, "Sorry but I can't chroot to '%s' because this operating system doesn't appear to support the chroot() system call", path);
#endif
    }
}

/* Get/Set UID of process */

bool
get_user (const char *username, struct user_state *state)
{
  bool ret = false;
  CLEAR (*state);
  if (username)
    {
#if defined(HAVE_GETPWNAM) && defined(HAVE_SETUID)
      state->pw = getpwnam (username);
      if (!state->pw)
	msg (M_ERR, "failed to find UID for user %s", username);
      state->username = username;
      ret = true;
#else
      msg (M_FATAL, "Sorry but I can't setuid to '%s' because this operating system doesn't appear to support the getpwname() or setuid() system calls", username);
#endif
    }
  return ret;
}

void
set_user (const struct user_state *state)
{
#if defined(HAVE_GETPWNAM) && defined(HAVE_SETUID)
  if (state->username && state->pw)
    {
      if (setuid (state->pw->pw_uid))
	msg (M_ERR, "setuid('%s') failed", state->username);
      msg (M_INFO, "UID set to %s", state->username);
    }
#endif
}

/* Get/Set GID of process */

bool
get_group (const char *groupname, struct group_state *state)
{
  bool ret = false;
  CLEAR (*state);
  if (groupname)
    {
#if defined(HAVE_GETGRNAM) && defined(HAVE_SETGID)
      state->gr = getgrnam (groupname);
      if (!state->gr)
	msg (M_ERR, "failed to find GID for group %s", groupname);
      state->groupname = groupname;
      ret = true;
#else
      msg (M_FATAL, "Sorry but I can't setgid to '%s' because this operating system doesn't appear to support the getgrnam() or setgid() system calls", groupname);
#endif
    }
  return ret;
}

void
set_group (const struct group_state *state)
{
#if defined(HAVE_GETGRNAM) && defined(HAVE_SETGID)
  if (state->groupname && state->gr)
    {
      if (setgid (state->gr->gr_gid))
	msg (M_ERR, "setgid('%s') failed", state->groupname);
      msg (M_INFO, "GID set to %s", state->groupname);
#ifdef HAVE_SETGROUPS
      {
        gid_t gr_list[1];
	gr_list[0] = state->gr->gr_gid;
	if (setgroups (1, gr_list))
	  msg (M_ERR, "setgroups('%s') failed", state->groupname);
      }
#endif
    }
#endif
}

/* Change process priority */
void
set_nice (int niceval)
{
  if (niceval)
    {
#ifdef HAVE_NICE
      errno = 0;
      nice (niceval);
      if (errno != 0)
	msg (M_WARN | M_ERRNO, "WARNING: nice %d failed", niceval);
      else
	msg (M_INFO, "nice %d succeeded", niceval);
#else
      msg (M_WARN, "WARNING: nice %d failed (function not implemented)", niceval);
#endif
    }
}

/*
 * Pass tunnel endpoint and MTU parms to a user-supplied script.
 * Used to execute the up/down script/plugins.
 */
void
run_up_down (const char *command,
	     const struct plugin_list *plugins,
	     int plugin_type,
	     const char *arg,
	     int tun_mtu,
	     int link_mtu,
	     const char *ifconfig_local,
	     const char* ifconfig_remote,
	     const char *context,
	     const char *signal_text,
	     const char *script_type,
	     struct env_set *es)
{
  struct gc_arena gc = gc_new ();

  if (signal_text)
    setenv_str (es, "signal", signal_text);
  setenv_str (es, "script_context", context);
  setenv_int (es, "tun_mtu", tun_mtu);
  setenv_int (es, "link_mtu", link_mtu);
  setenv_str (es, "dev", arg);

  if (!ifconfig_local)
    ifconfig_local = "";
  if (!ifconfig_remote)
    ifconfig_remote = "";
  if (!context)
    context = "";

  if (plugin_defined (plugins, plugin_type))
    {
      struct buffer cmd = alloc_buf_gc (256, &gc);

      ASSERT (arg);

      buf_printf (&cmd,
		  "%s %d %d %s %s %s",
		  arg,
		  tun_mtu, link_mtu,
		  ifconfig_local, ifconfig_remote,
		  context);

      if (plugin_call (plugins, plugin_type, BSTR (&cmd), es))
	msg (M_FATAL, "ERROR: up/down plugin call failed");
    }

  if (command)
    {
      struct buffer cmd = alloc_buf_gc (256, &gc);

      ASSERT (arg);

      setenv_str (es, "script_type", script_type);

      buf_printf (&cmd,
		  "%s %s %d %d %s %s %s",
		  command,
		  arg,
		  tun_mtu, link_mtu,
		  ifconfig_local, ifconfig_remote,
		  context);
      msg (M_INFO, "%s", BSTR (&cmd));
      system_check (BSTR (&cmd), es, S_SCRIPT|S_FATAL, "script failed");
    }

  gc_free (&gc);
}

/* Get the file we will later write our process ID to */
void
get_pid_file (const char* filename, struct pid_state *state)
{
  CLEAR (*state);
  if (filename)
    {
      state->fp = fopen (filename, "w");
      if (!state->fp)
	msg (M_ERR, "Open error on pid file %s", filename);
      state->filename = filename;
    }
}

/* Write our PID to a file */
void
write_pid (const struct pid_state *state)
{
  if (state->filename && state->fp)
    {
      unsigned int pid = openvpn_getpid (); 
      fprintf(state->fp, "%u\n", pid);
      if (fclose (state->fp))
	msg (M_ERR, "Close error on pid file %s", state->filename);
    }
}

/* Get current PID */
unsigned int
openvpn_getpid ()
{
#ifdef WIN32
  return (unsigned int) GetCurrentProcessId ();
#else
#ifdef HAVE_GETPID
  return (unsigned int) getpid ();
#else
  return 0;
#endif
#endif
}

/* Disable paging */
void
do_mlockall(bool print_msg)
{
#ifdef HAVE_MLOCKALL
  if (mlockall (MCL_CURRENT | MCL_FUTURE))
    msg (M_WARN | M_ERRNO, "WARNING: mlockall call failed");
  else if (print_msg)
    msg (M_INFO, "mlockall call succeeded");
#else
  msg (M_WARN, "WARNING: mlockall call failed (function not implemented)");
#endif
}

#ifndef HAVE_DAEMON

int
daemon(int nochdir, int noclose)
{
#if defined(HAVE_FORK) && defined(HAVE_SETSID)
  switch (fork())
    {
    case -1:
      return (-1);
    case 0:
      break;
    default:
      openvpn_exit (OPENVPN_EXIT_STATUS_GOOD); /* exit point */
    }

  if (setsid() == -1)
    return (-1);

  if (!nochdir)
    openvpn_chdir ("/");

  if (!noclose)
    set_std_files_to_null (false);
#else
  msg (M_FATAL, "Sorry but I can't become a daemon because this operating system doesn't appear to support either the daemon() or fork() system calls");
#endif
  return (0);
}

#endif

/*
 * Set standard file descriptors to /dev/null
 */
void
set_std_files_to_null (bool stdin_only)
{
#if defined(HAVE_DUP) && defined(HAVE_DUP2)
  int fd;
  if ((fd = open ("/dev/null", O_RDWR, 0)) != -1)
    {
      dup2 (fd, 0);
      if (!stdin_only)
	{
	  dup2 (fd, 1);
	  dup2 (fd, 2);
	}
      if (fd > 2)
	close (fd);
    }
#endif
}

/*
 * Wrapper for chdir library function
 */
int
openvpn_chdir (const char* dir)
{
#ifdef HAVE_CHDIR
  return chdir (dir);
#else
  return -1;
#endif
}

/*
 *  dup inetd/xinetd socket descriptor and save
 */

int inetd_socket_descriptor = SOCKET_UNDEFINED; /* GLOBAL */

void
save_inetd_socket_descriptor (void)
{
  inetd_socket_descriptor = INETD_SOCKET_DESCRIPTOR;
#if defined(HAVE_DUP) && defined(HAVE_DUP2)
  /* use handle passed by inetd/xinetd */
  if ((inetd_socket_descriptor = dup (INETD_SOCKET_DESCRIPTOR)) < 0)
    msg (M_ERR, "INETD_SOCKET_DESCRIPTOR dup(%d) failed", INETD_SOCKET_DESCRIPTOR);
  set_std_files_to_null (true);
#endif
}

/*
 * Wrapper around the system() call.
 */
int
openvpn_system (const char *command, const struct env_set *es, unsigned int flags)
{
#ifdef HAVE_SYSTEM
  int ret;

  /*
   * We need to bracket this code by mutex because system() doesn't
   * accept an environment list, so we have to use the process-wide
   * list which is shared between all threads.
   */
  mutex_lock_static (L_SYSTEM);
  perf_push (PERF_SCRIPT);

  /*
   * add env_set to environment.
   */
  if (flags & S_SCRIPT)
    env_set_add_to_environment (es);


  /* debugging */
  dmsg (D_SCRIPT, "SYSTEM[%u] '%s'", flags, command);
  if (flags & S_SCRIPT)
    env_set_print (D_SCRIPT, es);

  /*
   * execute the command
   */
  ret = system (command);

  /* debugging */
  dmsg (D_SCRIPT, "SYSTEM return=%u", ret);

  /*
   * remove env_set from environment
   */
  if (flags & S_SCRIPT)
    env_set_remove_from_environment (es);

  perf_pop ();
  mutex_unlock_static (L_SYSTEM);
  return ret;

#else
  msg (M_FATAL, "Sorry but I can't execute the shell command '%s' because this operating system doesn't appear to support the system() call", command);
  return -1; /* NOTREACHED */
#endif
}

/*
 * Warn if a given file is group/others accessible.
 */
void
warn_if_group_others_accessible (const char* filename)
{
#ifdef HAVE_STAT
  struct stat st;
  if (stat (filename, &st))
    {
      msg (M_WARN | M_ERRNO, "WARNING: cannot stat file '%s'", filename);
    }
  else
    {
      if (st.st_mode & (S_IRWXG|S_IRWXO))
	msg (M_WARN, "WARNING: file '%s' is group or others accessible", filename);
    }
#endif
}

/*
 * convert system() return into a success/failure value
 */
bool
system_ok (int stat)
{
#ifdef WIN32
  return stat == 0;
#else
  return stat != -1 && WIFEXITED (stat) && WEXITSTATUS (stat) == 0;
#endif
}

/*
 * did system() call execute the given command?
 */
bool
system_executed (int stat)
{
#ifdef WIN32
  return stat != -1;
#else
  return stat != -1 && WEXITSTATUS (stat) != 127;
#endif
}

/*
 * Print an error message based on the status code returned by system().
 */
const char *
system_error_message (int stat, struct gc_arena *gc)
{
  struct buffer out = alloc_buf_gc (256, gc);
#ifdef WIN32
  if (stat == -1)
    buf_printf (&out, "shell command did not execute -- ");
  buf_printf (&out, "system() returned error code %d", stat);
#else
  if (stat == -1)
    buf_printf (&out, "shell command fork failed");
  else if (!WIFEXITED (stat))
    buf_printf (&out, "shell command did not exit normally");
  else
    {
      const int cmd_ret = WEXITSTATUS (stat);
      if (!cmd_ret)
	buf_printf (&out, "shell command exited normally");
      else if (cmd_ret == 127)
	buf_printf (&out, "could not execute shell command");
      else
	buf_printf (&out, "shell command exited with error status: %d", cmd_ret);
    }
#endif
  return (const char *)out.data;
}

/*
 * Run system(), exiting on error.
 */
bool
system_check (const char *command, const struct env_set *es, unsigned int flags, const char *error_message)
{
  struct gc_arena gc = gc_new ();
  const int stat = openvpn_system (command, es, flags);
  int ret = false;

  if (system_ok (stat))
    ret = true;
  else
    {
      if (error_message)
	msg (((flags & S_FATAL) ? M_FATAL : M_WARN), "%s: %s",
	     error_message,
	     system_error_message (stat, &gc));
    }
  gc_free (&gc);
  return ret;
}

/*
 * Initialize random number seed.  random() is only used
 * when "weak" random numbers are acceptable.
 * OpenSSL routines are always used when cryptographically
 * strong random numbers are required.
 */

void
init_random_seed(void)
{
#ifdef HAVE_GETTIMEOFDAY
  struct timeval tv;

  if (!gettimeofday (&tv, NULL))
    {
      const unsigned int seed = (unsigned int) tv.tv_sec ^ tv.tv_usec;
      srandom (seed);
    }
#else /* HAVE_GETTIMEOFDAY */
  const time_t current = time (NULL);
  srandom ((unsigned int)current);
#endif /* HAVE_GETTIMEOFDAY */
}

/* thread-safe strerror */

const char *
strerror_ts (int errnum, struct gc_arena *gc)
{
#ifdef HAVE_STRERROR
  struct buffer out = alloc_buf_gc (256, gc);

  mutex_lock_static (L_STRERR);
  buf_printf (&out, "%s", openvpn_strerror (errnum, gc));
  mutex_unlock_static (L_STRERR);
  return BSTR (&out);
#else
  return "[error string unavailable]";
#endif
}

/*
 * Set environmental variable (int or string).
 *
 * On Posix, we use putenv for portability,
 * and put up with its painful semantics
 * that require all the support code below.
 */

/* General-purpose environmental variable set functions */

static char *
construct_name_value (const char *name, const char *value, struct gc_arena *gc)
{
  struct buffer out;

  ASSERT (name);
  if (!value)
    value = "";
  out = alloc_buf_gc (strlen (name) + strlen (value) + 2, gc);
  buf_printf (&out, "%s=%s", name, value);
  return BSTR (&out);
}

bool
deconstruct_name_value (const char *str, const char **name, const char **value, struct gc_arena *gc)
{
  char *cp;

  ASSERT (str);
  ASSERT (name && value);

  *name = cp = string_alloc (str, gc);
  *value = NULL;

  while ((*cp))
    {
      if (*cp == '=' && !*value)
	{
	  *cp = 0;
	  *value = cp + 1;
	}
      ++cp;
    }
  return *name && *value;
}

static bool
env_string_equal (const char *s1, const char *s2)
{
  int c1, c2;
  ASSERT (s1);
  ASSERT (s2);

  while (true)
    {
      c1 = *s1++;
      c2 = *s2++;
      if (c1 == '=')
	c1 = 0;
      if (c2 == '=')
	c2 = 0;
      if (!c1 && !c2)
	return true;
      if (c1 != c2)
	break;
    }
  return false;
}

static bool
remove_env_item (const char *str, const bool do_free, struct env_item **list)
{
  struct env_item *current, *prev;

  ASSERT (str);
  ASSERT (list);

  for (current = *list, prev = NULL; current != NULL; current = current->next)
    {
      if (env_string_equal (current->string, str))
	{
	  if (prev)
	    prev->next = current->next;
	  else
	    *list = current->next;
	  if (do_free)
	    {
	      memset (current->string, 0, strlen (current->string));
	      free (current->string);
	      free (current);
	    }
	  return true;
	}
      prev = current;
    }
  return false;
}

static void
add_env_item (char *str, const bool do_alloc, struct env_item **list, struct gc_arena *gc)
{
  struct env_item *item;

  ASSERT (str);
  ASSERT (list);

  ALLOC_OBJ_GC (item, struct env_item, gc);
  item->string = do_alloc ? string_alloc (str, gc): str;
  item->next = *list;
  *list = item;
}

/* struct env_set functions */

static bool
env_set_del_nolock (struct env_set *es, const char *str)
{
  return remove_env_item (str, false, &es->list);
}

static void
env_set_add_nolock (struct env_set *es, const char *str)
{
  remove_env_item (str, false, &es->list);  
  add_env_item ((char *)str, true, &es->list, es->gc);
}

struct env_set *
env_set_create (struct gc_arena *gc)
{
  struct env_set *es;
  ASSERT (gc);
  mutex_lock_static (L_ENV_SET);
  ALLOC_OBJ_CLEAR_GC (es, struct env_set, gc);
  es->list = NULL;
  es->gc = gc;
  mutex_unlock_static (L_ENV_SET);
  return es;
}

bool
env_set_del (struct env_set *es, const char *str)
{
  bool ret;
  ASSERT (es);
  ASSERT (str);
  mutex_lock_static (L_ENV_SET);
  ret = env_set_del_nolock (es, str);
  mutex_unlock_static (L_ENV_SET);
  return ret;
}

void
env_set_add (struct env_set *es, const char *str)
{
  ASSERT (es);
  ASSERT (str);
  mutex_lock_static (L_ENV_SET);
  env_set_add_nolock (es, str);
  mutex_unlock_static (L_ENV_SET);
}

void
env_set_print (int msglevel, const struct env_set *es)
{
  if (check_debug_level (msglevel))
    {
      const struct env_item *e;
      int i;

      if (es)
	{
	  mutex_lock_static (L_ENV_SET);
	  e = es->list;
	  i = 0;

	  while (e)
	    {
	      msg (msglevel, "ENV [%d] '%s'", i, e->string);
	      ++i;
	      e = e->next;
	    }
	  mutex_unlock_static (L_ENV_SET);
	}
    }
}

void
env_set_inherit (struct env_set *es, const struct env_set *src)
{
  const struct env_item *e;

  ASSERT (es);

  if (src)
    {
      mutex_lock_static (L_ENV_SET);
      e = src->list;
      while (e)
	{
	  env_set_add_nolock (es, e->string);
	  e = e->next;
	}
      mutex_unlock_static (L_ENV_SET);
    }
}

void
env_set_add_to_environment (const struct env_set *es)
{
  if (es)
    {
      struct gc_arena gc = gc_new ();
      const struct env_item *e;

      mutex_lock_static (L_ENV_SET);
      e = es->list;

      while (e)
	{
	  const char *name;
	  const char *value;

	  if (deconstruct_name_value (e->string, &name, &value, &gc))
	    setenv_str (NULL, name, value);

	  e = e->next;
	}
      mutex_unlock_static (L_ENV_SET);
      gc_free (&gc);
    }
}

void
env_set_remove_from_environment (const struct env_set *es)
{
  if (es)
    {
      struct gc_arena gc = gc_new ();
      const struct env_item *e;

      mutex_lock_static (L_ENV_SET);
      e = es->list;

      while (e)
	{
	  const char *name;
	  const char *value;

	  if (deconstruct_name_value (e->string, &name, &value, &gc))
	    setenv_del (NULL, name);

	  e = e->next;
	}
      mutex_unlock_static (L_ENV_SET);
      gc_free (&gc);
    }
}

#ifdef HAVE_PUTENV

/* companion functions to putenv */

static struct env_item *global_env = NULL; /* GLOBAL */

static void
manage_env (char *str)
{
  remove_env_item (str, true, &global_env);
  add_env_item (str, false, &global_env, NULL);
}

#endif

/* add/modify/delete environmental strings */

void
setenv_counter (struct env_set *es, const char *name, counter_type value)
{
  char buf[64];
  openvpn_snprintf (buf, sizeof(buf), counter_format, value);
  setenv_str (es, name, buf);
}

void
setenv_int (struct env_set *es, const char *name, int value)
{
  char buf[64];
  openvpn_snprintf (buf, sizeof(buf), "%d", value);
  setenv_str (es, name, buf);
}

void
setenv_str (struct env_set *es, const char *name, const char *value)
{
  setenv_str_ex (es, name, value, CC_NAME, 0, 0, CC_PRINT, 0, 0);
}

void
setenv_del (struct env_set *es, const char *name)
{
  ASSERT (name);
  setenv_str (es, name, NULL);
}

void
setenv_str_ex (struct env_set *es,
	       const char *name,
	       const char *value,
	       const unsigned int name_include,
	       const unsigned int name_exclude,
	       const char name_replace,
	       const unsigned int value_include,
	       const unsigned int value_exclude,
	       const char value_replace)
{
  struct gc_arena gc = gc_new ();
  const char *name_tmp;
  const char *val_tmp = NULL;

  ASSERT (name && strlen (name) > 1);

  name_tmp = string_mod_const (name, name_include, name_exclude, name_replace, &gc);

  if (value)
    val_tmp = string_mod_const (value, value_include, value_exclude, value_replace, &gc);

  if (es)
    {
      if (val_tmp)
	{
	  const char *str = construct_name_value (name_tmp, val_tmp, &gc);
	  env_set_add (es, str);
	}
      else
	env_set_del (es, name_tmp);
    }
  else
    {
#if defined(WIN32)
      {
	/*msg (M_INFO, "SetEnvironmentVariable '%s' '%s'", name_tmp, val_tmp ? val_tmp : "NULL");*/
	if (!SetEnvironmentVariable (name_tmp, val_tmp))
	  msg (M_WARN | M_ERRNO, "SetEnvironmentVariable failed, name='%s', value='%s'",
	       name_tmp,
	       val_tmp ? val_tmp : "NULL");
      }
#elif defined(HAVE_PUTENV)
      {
	char *str = construct_name_value (name_tmp, val_tmp, NULL);
	int status;

	mutex_lock_static (L_PUTENV);
	status = putenv (str);
	/*msg (M_INFO, "PUTENV '%s'", str);*/
	if (!status)
	  manage_env (str);
	mutex_unlock_static (L_PUTENV);
	if (status)
	  msg (M_WARN | M_ERRNO, "putenv('%s') failed", str);
      }
#endif
    }

  gc_free (&gc);
}

/*
 * taken from busybox networking/ifupdown.c
 */
unsigned int
count_bits(unsigned int a)
{
  unsigned int result;
  result = (a & 0x55) + ((a >> 1) & 0x55);
  result = (result & 0x33) + ((result >> 2) & 0x33);
  return((result & 0x0F) + ((result >> 4) & 0x0F));
}

int
count_netmask_bits(const char *dotted_quad)
{
  unsigned int result, a, b, c, d;
  /* Found a netmask...  Check if it is dotted quad */
  if (sscanf(dotted_quad, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
    return -1;
  result = count_bits(a);
  result += count_bits(b);
  result += count_bits(c);
  result += count_bits(d);
  return ((int)result);
}

/*
 * Go to sleep for n milliseconds.
 */
void
sleep_milliseconds (unsigned int n)
{
#ifdef WIN32
  Sleep (n);
#else
  struct timeval tv;
  tv.tv_sec = n / 1000;
  tv.tv_usec = (n % 1000) * 1000;
  select (0, NULL, NULL, NULL, &tv);
#endif
}

/*
 * Go to sleep indefinitely.
 */
void
sleep_until_signal (void)
{
#ifdef WIN32
  ASSERT (0);
#else
  select (0, NULL, NULL, NULL, NULL);
#endif
}

/* return true if filename can be opened for read */
bool
test_file (const char *filename)
{
  bool ret = false;
  if (filename)
    {
      FILE *fp = fopen (filename, "r");
      if (fp)
	{
	  fclose (fp);
	  ret = true;
	}
    }

  dmsg (D_TEST_FILE, "TEST FILE '%s' [%d]",
       filename ? filename : "UNDEF",
       ret);

  return ret;
}

/* create a temporary filename in directory */
const char *
create_temp_filename (const char *directory, struct gc_arena *gc)
{
  static unsigned int counter;
  struct buffer fname = alloc_buf_gc (256, gc);

  mutex_lock_static (L_CREATE_TEMP);
  ++counter;
  mutex_unlock_static (L_CREATE_TEMP);

  buf_printf (&fname, PACKAGE "_%u_%u.tmp",
	      openvpn_getpid (),
	      counter);

  return gen_path (directory, BSTR (&fname), gc);
}

/*
 * Put a directory and filename together.
 */
const char *
gen_path (const char *directory, const char *filename, struct gc_arena *gc)
{
  const char *safe_filename = string_mod_const (filename, CC_ALNUM|CC_UNDERBAR|CC_DASH|CC_DOT|CC_AT, 0, '_', gc);

  if (safe_filename
      && strcmp (safe_filename, ".")
      && strcmp (safe_filename, ".."))
    {
      struct buffer out = alloc_buf_gc (256, gc);
      char dirsep[2];

      dirsep[0] = OS_SPECIFIC_DIRSEP;
      dirsep[1] = '\0';

      if (directory)
	buf_printf (&out, "%s%s", directory, dirsep);
      buf_printf (&out, "%s", safe_filename);

      return BSTR (&out);
    }
  else
    return NULL;
}

/* delete a file, return true if succeeded */
bool
delete_file (const char *filename)
{
#if defined(WIN32)
  return (DeleteFile (filename) != 0);
#elif defined(HAVE_UNLINK)
  return (unlink (filename) == 0);
#else
  return false;
#endif
}

/*
 * Return the next largest power of 2
 * or u if u is a power of 2.
 */
unsigned int
adjust_power_of_2 (unsigned int u)
{
  unsigned int ret = 1;

  while (ret < u)
    {
      ret <<= 1;
      ASSERT (ret > 0);
    }

  return ret;
}

#ifdef HAVE_GETPASS

static FILE *
open_tty (const bool write)
{
  FILE *ret;
  ret = fopen ("/dev/tty", write ? "w" : "r");
  if (!ret)
    ret = write ? stderr : stdin;
  return ret;
}

static void
close_tty (FILE *fp)
{
  if (fp != stderr && fp != stdin)
    fclose (fp);
}

#endif

/*
 * Get input from console
 */
bool
get_console_input (const char *prompt, const bool echo, char *input, const int capacity)
{
  bool ret = false;
  ASSERT (prompt);
  ASSERT (input);
  ASSERT (capacity > 0);
  input[0] = '\0';

#if defined(WIN32)
  return get_console_input_win32 (prompt, echo, input, capacity);
#elif defined(HAVE_GETPASS)
  if (echo)
    {
      FILE *fp;

      fp = open_tty (true);
      fprintf (fp, "%s", prompt);
      fflush (fp);
      close_tty (fp);

      fp = open_tty (false);
      if (fgets (input, capacity, fp) != NULL)
	{
	  chomp (input);
	  ret = true;
	}
      close_tty (fp);
    }
  else
    {
      char *gp = getpass (prompt);
      if (gp)
	{
	  strncpynt (input, gp, capacity);
	  memset (gp, 0, strlen (gp));
	  ret = true;
	}
    }
#else
  msg (M_FATAL, "Sorry, but I can't get console input on this OS");
#endif
  return ret;
}

/*
 * Get and store a username/password
 */

void
get_user_pass (struct user_pass *up,
	       const char *auth_file,
	       const bool password_only,
	       const char *prefix,
	       const unsigned int flags)
{
  struct gc_arena gc = gc_new ();

  if (!up->defined)
    {
      const bool from_stdin = (!auth_file || !strcmp (auth_file, "stdin"));

#ifdef ENABLE_MANAGEMENT
      /*
       * Get username/password from standard input?
       */
      if (management
	  && ((auth_file && streq (auth_file, "management")) || (from_stdin && (flags & GET_USER_PASS_MANAGEMENT)))
	  && management_query_user_pass_enabled (management))
	{
	  if (!management_query_user_pass (management, up, prefix, password_only))
	    msg (M_FATAL, "ERROR: could not read %s username/password from management interface", prefix);
	}
      else
#endif
      /*
       * Get username/password from standard input?
       */
      if (from_stdin)
	{
	  struct buffer user_prompt = alloc_buf_gc (128, &gc);
	  struct buffer pass_prompt = alloc_buf_gc (128, &gc);

	  buf_printf (&user_prompt, "Enter %s Username:", prefix);
	  buf_printf (&pass_prompt, "Enter %s Password:", prefix);

	  if (!password_only)
	    {
	      if (!get_console_input (BSTR (&user_prompt), true, up->username, USER_PASS_LEN))
		msg (M_FATAL, "ERROR: could not read %s username from stdin", prefix);
	      if (strlen (up->username) == 0)
		msg (M_FATAL, "ERROR: %s username is empty", prefix);
	    }

	  if (!get_console_input (BSTR (&pass_prompt), false, up->password, USER_PASS_LEN))
	    msg (M_FATAL, "ERROR: could not not read %s password from stdin", prefix);
	}
      else
	{
	  /*
	   * Get username/password from a file.
	   */
	  FILE *fp;
      
#ifndef ENABLE_PASSWORD_SAVE
	  /*
	   * Unless ENABLE_PASSWORD_SAVE is defined, don't allow sensitive passwords
	   * to be read from a file.
	   */
	  if (flags & GET_USER_PASS_SENSITIVE)
	    msg (M_FATAL, "Sorry, '%s' password cannot be read from a file", prefix);
#endif

	  warn_if_group_others_accessible (auth_file);

	  fp = fopen (auth_file, "r");
	  if (!fp)
	    msg (M_ERR, "Error opening '%s' auth file: %s", prefix, auth_file);

	  if (password_only)
	    {
	      if (fgets (up->password, USER_PASS_LEN, fp) == NULL)
		msg (M_FATAL, "Error reading password from %s authfile: %s",
		     prefix,
		     auth_file);
	    }
	  else
	    {
	      if (fgets (up->username, USER_PASS_LEN, fp) == NULL
		  || fgets (up->password, USER_PASS_LEN, fp) == NULL)
		msg (M_FATAL, "Error reading username and password (must be on two consecutive lines) from %s authfile: %s",
		     prefix,
		     auth_file);
	    }
      
	  fclose (fp);
      
	  chomp (up->username);
	  chomp (up->password);
      
	  if (!password_only && strlen (up->username) == 0)
	    msg (M_FATAL, "ERROR: username from %s authfile '%s' is empty", prefix, auth_file);
	}

      string_mod (up->username, CC_PRINT, CC_CRLF, 0);
      string_mod (up->password, CC_PRINT, CC_CRLF, 0);

      up->defined = true;
    }

#if 0
  msg (M_INFO, "GET_USER_PASS %s u='%s' p='%s'", prefix, up->username, up->password);
#endif

  gc_free (&gc);
}

void
purge_user_pass (struct user_pass *up, const bool force)
{
  const bool nocache = up->nocache;
  if (nocache || force)
    {
      CLEAR (*up);
      up->nocache = nocache;
    }
}

/*
 * Process string received by untrusted peer before
 * printing to console or log file.
 *
 * Assumes that string has been null terminated.
 */
const char *
safe_print (const char *str, struct gc_arena *gc)
{
  return string_mod_const (str, CC_PRINT, CC_CRLF, '.', gc);
}

/* Make arrays of strings */

const char **
make_env_array (const struct env_set *es, struct gc_arena *gc)
{
  char **ret = NULL;
  struct env_item *e = NULL;
  int i = 0, n = 0;

  /* figure length of es */
  if (es)
    {
      for (e = es->list; e != NULL; e = e->next)
	++n;
    }

  /* alloc return array */
  ALLOC_ARRAY_CLEAR_GC (ret, char *, n+1, gc);

  /* fill return array */
  if (es)
    {
      e = es->list;
      for (i = 0; i < n; ++i)
	{
	  ASSERT (e);
	  ret[i] = e->string;
	  e = e->next;
	}
    }

  ret[i] = NULL;
  return (const char **)ret;
}

const char **
make_arg_array (const char *first, const char *parms, struct gc_arena *gc)
{
  char **ret = NULL;
  int base = 0;
  const int max_parms = MAX_PARMS + 2;
  int n = 0;

  /* alloc return array */
  ALLOC_ARRAY_CLEAR_GC (ret, char *, max_parms, gc);

  /* process first parameter, if provided */
  if (first)
    {
      ret[base++] = string_alloc (first, gc);
    }

  if (parms)
    {
      n = parse_line (parms, &ret[base], max_parms - base - 1, "make_arg_array", 0, M_WARN, gc);
      ASSERT (n >= 0 && n + base + 1 <= max_parms);
    }
  ret[base + n] = NULL;

  return (const char **)ret;
}

void
openvpn_sleep (const int n)
{
#ifdef ENABLE_MANAGEMENT
  if (management)
    {
      management_event_loop_n_seconds (management, n);
      return;
    }
#endif
  sleep (n);
}
