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

#ifndef MISC_H
#define MISC_H

#include "basic.h"
#include "common.h"
#include "integer.h"
#include "buffer.h"

/* socket descriptor passed by inetd/xinetd server to us */
#define INETD_SOCKET_DESCRIPTOR 0

/* forward declarations */
struct plugin_list;

/*
 * Handle environmental variable lists
 */

struct env_item {
  char *string;
  struct env_item *next;
};

struct env_set {
  struct gc_arena *gc;
  struct env_item *list;
};

/* Get/Set UID of process */

struct user_state {
#if defined(HAVE_GETPWNAM) && defined(HAVE_SETUID)
  const char *username;
  struct passwd *pw;
#else
  int dummy;
#endif
};

bool get_user (const char *username, struct user_state *state);
void set_user (const struct user_state *state);

/* Get/Set GID of process */

struct group_state {
#if defined(HAVE_GETGRNAM) && defined(HAVE_SETGID)
  const char *groupname;
  struct group *gr;
#else
  int dummy;
#endif
};

bool get_group (const char *groupname, struct group_state *state);
void set_group (const struct group_state *state);

void set_nice (int niceval);
void do_chroot (const char *path);

void run_up_down (const char *command,
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
		  struct env_set *es);

/* workspace for get_pid_file/write_pid */
struct pid_state {
  FILE *fp;
  const char *filename;
};

void get_pid_file (const char* filename, struct pid_state *state);
void write_pid (const struct pid_state *state);
unsigned int openvpn_getpid (void);

void do_mlockall (bool print_msg); /* Disable paging */

#ifndef HAVE_DAEMON
int daemon (int nochdir, int noclose);
#endif

/* check file protections */
void warn_if_group_others_accessible(const char* filename);

/* system flags */
#define S_SCRIPT (1<<0)
#define S_FATAL  (1<<1)

/* wrapper around the system() call. */
int openvpn_system (const char *command, const struct env_set *es, unsigned int flags);

/* interpret the status code returned by system() */
bool system_ok(int);
int system_executed (int stat);
const char *system_error_message (int, struct gc_arena *gc);

/* run system() with error check, return true if success,
   false if error, exit if error and fatal==true */
bool system_check (const char *command, const struct env_set *es, unsigned int flags, const char *error_message);

#ifdef HAVE_STRERROR
/* a thread-safe version of strerror */
const char* strerror_ts (int errnum, struct gc_arena *gc);
#endif

/* Set standard file descriptors to /dev/null */
void set_std_files_to_null (bool stdin_only);

/* Wrapper for chdir library function */
int openvpn_chdir (const char* dir);

/* dup inetd/xinetd socket descriptor and save */
extern int inetd_socket_descriptor;
void save_inetd_socket_descriptor (void);

/* init random() function, only used as source for weak random numbers, when !USE_CRYPTO */
void init_random_seed(void);

/* set/delete environmental variable */
void setenv_str_ex (struct env_set *es,
		    const char *name,
		    const char *value,
		    const unsigned int name_include,
		    const unsigned int name_exclude,
		    const char name_replace,
		    const unsigned int value_include,
		    const unsigned int value_exclude,
		    const char value_replace);

void setenv_counter (struct env_set *es, const char *name, counter_type value);
void setenv_int (struct env_set *es, const char *name, int value);
void setenv_str (struct env_set *es, const char *name, const char *value);
void setenv_del (struct env_set *es, const char *name);

/* struct env_set functions */

struct env_set *env_set_create (struct gc_arena *gc);
bool env_set_del (struct env_set *es, const char *str);
void env_set_add (struct env_set *es, const char *str);

void env_set_print (int msglevel, const struct env_set *es);

void env_set_inherit (struct env_set *es, const struct env_set *src);

void env_set_add_to_environment (const struct env_set *es);
void env_set_remove_from_environment (const struct env_set *es);

/* Make arrays of strings */

const char **make_env_array (const struct env_set *es, struct gc_arena *gc);
const char **make_arg_array (const char *first, const char *parms, struct gc_arena *gc);

/* convert netmasks for iproute2 */
int count_netmask_bits(const char *);
unsigned int count_bits(unsigned int );

/* go to sleep for n milliseconds */
void sleep_milliseconds (unsigned int n);

/* go to sleep indefinitely */
void sleep_until_signal (void);

/* an analogue to the random() function, but use OpenSSL functions if available */
#ifdef USE_CRYPTO
long int get_random(void);
#else
#define get_random random
#endif

/* return true if filename can be opened for read */
bool test_file (const char *filename);

/* create a temporary filename in directory */
const char *create_temp_filename (const char *directory, struct gc_arena *gc);

/* put a directory and filename together */
const char *gen_path (const char *directory, const char *filename, struct gc_arena *gc);

/* delete a file, return true if succeeded */
bool delete_file (const char *filename);

/* return the next largest power of 2 */
unsigned int adjust_power_of_2 (unsigned int u);

/*
 * Get and store a username/password
 */

struct user_pass
{
  bool defined;
  bool nocache;

/* max length of username/password */
# define USER_PASS_LEN 128
  char username[USER_PASS_LEN];
  char password[USER_PASS_LEN];
};

bool get_console_input (const char *prompt, const bool echo, char *input, const int capacity);

#define GET_USER_PASS_MANAGEMENT  (1<<0)
#define GET_USER_PASS_SENSITIVE   (1<<1)

void get_user_pass (struct user_pass *up,
		    const char *auth_file,
		    const bool password_only,
		    const char *prefix,
		    const unsigned int flags);

void purge_user_pass (struct user_pass *up, const bool force);

/*
 * Process string received by untrusted peer before
 * printing to console or log file.
 * Assumes that string has been null terminated.
 */
const char *safe_print (const char *str, struct gc_arena *gc);

/*
 * A sleep function that services the management layer for n
 * seconds rather than doing nothing.
 */
void openvpn_sleep (const int n);

#endif
