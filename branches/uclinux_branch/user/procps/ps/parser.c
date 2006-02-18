/*
 * Copyright 1998 by Albert Cahalan; all rights reserved.
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */

/* Ought to have debug print stuff like this:
 * #define Print(fmt, args...) printf("Debug: " fmt, ## args)
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* username lookups */
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "../proc/version.h"

#define ARG_GNU  0
#define ARG_END  1
#define ARG_PGRP 2
#define ARG_SYSV 3
#define ARG_PID  4
#define ARG_BSD  5
#define ARG_FAIL 6
#define ARG_SESS 7

static int w_count = 0;

static int ps_argc;    /* global argc */
static char **ps_argv; /* global argv */
static int thisarg;    /* index into ps_argv */
static char *flagptr;  /* current location in ps_argv[thisarg] */
static int not_pure_unix = 0;  /* set by BSD and GNU options */
static int force_bsd = 0;  /* set when normal parsing fails */

#define exclusive(x) if((ps_argc != 2) || strcmp(ps_argv[1],x))\
  return "The " x " option is exclusive."


/********** utility functions **********/

/*
 * Both "-Oppid" and "-O ppid" should be legal, though Unix98
 * does not require it. BSD and Digital Unix allow both.
 * Return the argument or NULL;
 */
static const char *get_opt_arg(void){
  if(*(flagptr+1)){     /* argument is part of ps_argv[thisarg] */
    not_pure_unix = 1;
    return flagptr+1;
  }
  if(thisarg+2 > ps_argc) return NULL;   /* there is nothing left */
  /* argument follows ps_argv[thisarg] */
  if(*(ps_argv[thisarg+1]) == '\0') return NULL;
  return ps_argv[++thisarg];
}

/********** parse lists (of UID, tty, GID, PID...) **********/

static const char *parse_pid(char *str, sel_union *ret){
  char *endp;
  int num;
  static const char *pidrange  = "Process ID out of range.";
  static const char *pidsyntax = "Process ID list syntax error.";
  num = strtol(str, &endp, 0);
  if(*endp != '\0')   return pidsyntax;
  if(num>0x7fff)      return pidrange;  /* Linux PID limit */
  if(num<0)           return pidrange;
  ret->pid = num;
  return 0;
}

static const char *parse_uid(char *str, sel_union *ret){
  struct passwd *passwd_data;
  char *endp;
  int num;
  static const char *uidrange = "User ID out of range.";
  static const char *uidexist = "User name does not exist.";
  num = strtol(str, &endp, 0);
  if(*endp != '\0'){  /* hmmm, try as login name */
    passwd_data = getpwnam(str);
    if(!passwd_data)    return uidexist;
    num = passwd_data->pw_uid;
  }
  if(num>65534)         return uidrange;  /* 65535 is very bad */
  if(num<-32768)        return uidrange;
  if(num==-1)           return uidrange;    /* -1 is very bad */
  num &= 0xffff;
  ret->uid = num;
  return 0;
}

static const char *parse_gid(char *str, sel_union *ret){
  struct group *group_data;
  char *endp;
  int num;
  static const char *gidrange = "Group ID out of range.";
  static const char *gidexist = "Group name does not exist.";
  num = strtol(str, &endp, 0);
  if(*endp != '\0'){  /* hmmm, try as login name */
    group_data = getgrnam(str);
    if(!group_data)    return gidexist;
    num = group_data->gr_gid;
  }
  if(num>65534)        return gidrange;  /* 65535 is very bad */
  if(num<-32768)       return gidrange;
  if(num==-1)          return gidrange;    /* -1 is very bad */
  num &= 0xffff;
  ret->gid = num;
  return 0;
}

static const char *parse_cmd(char *str, sel_union *ret){
  strncpy(ret->cmd, str, 8);  /* strncpy pads to end */
  return 0;
}

static const char *parse_tty(char *str, sel_union *ret){
  struct stat sbuf;
  static const char *missing = "TTY could not be found.";
  static const char *not_tty = "List member was not a TTY.";
  char path[4096];
  if(str[0]=='/'){
    if(stat(str, &sbuf) >= 0) goto found_it;
    return missing;
  }
#define lookup(p) \
  snprintf(path,4096,p,str); \
  if(stat(path, &sbuf) >= 0) goto found_it

  lookup("/dev/pts/%s");  /* New Unix98 ptys go first */
  lookup("/dev/%s");
  lookup("/dev/tty%s");
  lookup("/dev/pty%s");
  lookup("/dev/%snsole"); /* "co" means "console", maybe do all VCs too? */
  if(!strcmp(str,"-")){   /* "-" means no tty (from AIX) */
    ret->tty = -1;  /* processes w/o tty */
    return 0;
  }
  if(!strcmp(str,"?")){   /* "?" means no tty, which bash eats (Reno BSD?) */
    ret->tty = -1;  /* processes w/o tty */
    return 0;
  }
  if(!*(str+1) && (stat(str,&sbuf)>=0)){  /* Kludge! Assume bash ate '?'. */
    ret->tty = -1;  /* processes w/o tty */
    return 0;
  }
#undef lookup
  return missing;
found_it:
  if(!S_ISCHR(sbuf.st_mode)) return not_tty;
  ret->tty = sbuf.st_rdev;
  return 0;
}

/*
 * Used to parse lists in a generic way. (function pointers)
 */
static const char *parse_list(const char *arg, const char *(*parse_fn)(char *, sel_union *) ){
  selection_node *node;
  char *buf;                      /* temp copy of arg to hack on */
  char *sep_loc;                  /* separator location: " \t," */
  char *walk;
  int items;
  int need_item;
  const char *err;       /* error code that could or did happen */
  /*** prepare to operate ***/
  node = malloc(sizeof(selection_node));
  node->u = malloc(strlen(arg)*sizeof(sel_union)); /* waste is insignificant */
  node->n = 0;
  buf = malloc(strlen(arg)+1);
  strcpy(buf, arg);
  /*** sanity check and count items ***/
  need_item = 1; /* true */
  items = 0;
  walk = buf;
  err = "Improper list.";
  do{
    switch(*walk){
    case ' ': case ',': case '\t': case '\0':
      if(need_item) goto parse_error;
      need_item=1;
      break;
    default:
      if(need_item) items++;
      need_item=0;
    }
  } while (*++walk);
  if(need_item) goto parse_error;
  node->n = items;
  /*** actually parse the list ***/
  walk = buf;
  while(items--){
    sep_loc = strpbrk(walk," ,\t");
    if(sep_loc) *sep_loc = '\0';
    if(( err=(parse_fn)(walk, node->u+items) )) goto parse_error;
    walk = sep_loc + 1; /* point to next item, if any */
  }
  free(buf);
  node->next = selection_list;
  selection_list = node;
  return NULL;
parse_error:
  free(buf);
  free(node->u);
  free(node);
  return err;
}

/***************** parse SysV options, including Unix98  *****************/
static const char *parse_sysv_option(void){
  const char *arg;
  const char *err;
  flagptr = ps_argv[thisarg];
  while(*++flagptr){
    /* Find any excuse to ignore stupid Unix98 misfeatures. */
    if(!strchr("aAdefgGlnoptuU", *flagptr)) not_pure_unix = 1;
    switch(*flagptr){
    case 'A':
      trace("-A selects all processes.\n");
      all_processes = 1;
      break;
    case 'C': /* end */
      trace("-C select by process name.\n");  /* Why only HP/UX and us? */
      arg=get_opt_arg();
      if(!arg) return "List of command names must follow -C.";
      err=parse_list(arg, parse_cmd);
      if(err) return err;
      selection_list->typecode = SEL_COMM;
      return NULL; /* can't have any more options */
    case 'G': /* end */
      trace("-G select by RGID (supports names)\n");
      arg=get_opt_arg();
      if(!arg) return "List of real groups must follow -G.";
      err=parse_list(arg, parse_gid);
      if(err) return err;
      selection_list->typecode = SEL_RGID;
      return NULL; /* can't have any more options */
    case 'H':     /* another nice HP/UX feature */
      trace("-H Process heirarchy (like ASCII art forest option)\n");
      forest_type = 'u';
      break;
    case 'L':  /*  */
      /* "fucking Sun blows me"... Sun uses this for threads,
       * adding the feature after both IBM & Digital used -m.
       * Maybe this will be supported eventually, after I calm
       * down about Sun's lack of conformity. Hmmm, SCO added it.
       */
      trace("-L Print LWP (thread) info.\n");
      format_modifiers |= FM_L;
      return "Use -m to print threads, not Sun's nonstandard -L.";
      break;
    case 'M':  /* someday, maybe, we will have MAC like SGI's Irix */
      trace("-M Print security label for Mandatory Access Control.\n");
      format_modifiers |= FM_M;
      return "Sorry, no Mandatory Access Control support.";
      break;
    case 'N':
      trace("-N negates.\n");
      negate_selection = 1;
      break;
    case 'O': /* end */
      trace("-O is preloaded -o.\n");
      arg=get_opt_arg();
      if(!arg) return "Format or sort specification must follow -O.";
      defer_sf_option(arg, SF_U_O);
      return NULL; /* can't have any more options */
#ifdef WE_UNDERSTAND_THIS
    case 'P':     /* unknown HP/UX or SunOS 5 feature */
      trace("-P adds columns of PRM info (HP) or PSR column (Sun)\n");
      format_modifiers |= FM_P;
      return "No HP PRM support. No Sun PSR info either.";
      break;
    case 'R':     /* unknown HP/UX feature */
      trace("-R selects PRM groups\n");
      return "Don't understand PRM on Linux.";
      break;
#endif
    case 'U': /* end */
      trace("-U select by RUID (supports names).\n");
      arg=get_opt_arg();
      if(!arg) return "List of real groups must follow -U.";
      err=parse_list(arg, parse_uid);
      if(err) return err;
      selection_list->typecode = SEL_RUID;
      return NULL; /* can't have any more options */
    case 'V': /* single */
      trace("-V prints version.\n");
      exclusive("-V");
      display_version();
      exit(0);
    case 'Z':     /* full Mandatory Access Control level info */
      trace("-Z shows full MAC info\n");
      return "Don't understand MAC on Linux.";
      break;
    case 'a':
      trace("-a select all with a tty, but omit session leaders.\n");
      simple_select |= SS_U_a;
      break;
    case 'c':
      /* HP-UX and SunOS 5 scheduling info modifier */
      trace("-c changes scheduling info.\n");
      format_modifiers |= FM_c;
      break;
    case 'd':
      trace("-d select all, but omit session leaders.\n");
      simple_select |= SS_U_d;
      break;
    case 'e':
      trace("-e selects all processes.\n");
      all_processes = 1;
      break;
    case 'f':
      trace("-f does full listing\n");
      format_flags |= FF_Uf;
      unix_f_option = 1; /* does this matter? */
      break;
    case 'g': /* end */
      trace("-g selects by session leader OR by group name\n");
      arg=get_opt_arg();
      if(!arg) return "List of session leaders OR effective group names must follow -g.";
      err=parse_list(arg, parse_pid);
      if(!err){
        selection_list->typecode = SEL_SESS;
        return NULL; /* can't have any more options */
      }
      err=parse_list(arg, parse_gid);
      if(!err){
        selection_list->typecode = SEL_EGID;
        return NULL; /* can't have any more options */
      }
      return "List of session leaders OR effective group IDs was invalid.";
    case 'j':
      trace("-j jobs format.\n");
      /* Debian uses RD_j and Digital uses JFMT */
      if(sysv_j_format) format_flags |= FF_Uj;
      else format_modifiers |= FM_j;
      break;
    case 'l':
      trace("-l long format.\n");
      format_flags |= FF_Ul;
      break;
    case 'm':
      trace("-m shows threads.\n");
      return "Thread display not implemented.";
      break;
    case 'n': /* end */
      trace("-n sets namelist file.\n");
      arg=get_opt_arg();
      if(!arg) return "System.map or psdatabase must follow -n.";
      namelist_file = arg;
      return NULL; /* can't have any more options */
    case 'o': /* end */
      /* Unix98 has gross behavior regarding this. From the following: */
      /*            ps -o pid,nice=NICE,tty=TERMINAL,comm              */
      /* The result must be 2 columns: "PID NICE,tty=TERMINAL,comm"    */
      /* Yes, the second column has the name "NICE,tty=TERMINAL,comm"  */
      /* This parser looks for any excuse to ignore that braindamage.  */
      trace("-o user-defined format.\n");
      arg=get_opt_arg();
      if(!arg) return "Format specification must follow -o.";
      not_pure_unix |= defer_sf_option(arg, SF_U_o);
      return NULL; /* can't have any more options */
    case 'p': /* end */
      trace("-p select by PID.\n");
      arg=get_opt_arg();
      if(!arg) return "List of process IDs must follow -p.";
      err=parse_list(arg, parse_pid);
      if(err) return err;
      selection_list->typecode = SEL_PID;
      return NULL; /* can't have any more options */
#ifdef KNOW_WHAT_TO_DO_WITH_THIS
    case 'r':
      trace("-r some Digital Unix thing about warnings...\n");
      trace("   or SCO's option to chroot() for new /proc and /dev.\n");
      return "The -r option is reserved.";
      break;
#endif
    case 's': /* end */
      trace("-s Select processes belonging to the sessions given.\n");
      arg=get_opt_arg();
      if(!arg) return "List of session IDs must follow -s.";
      err=parse_list(arg, parse_pid);
      if(err) return err;
      selection_list->typecode = SEL_SESS;
      return NULL; /* can't have any more options */
    case 't': /* end */
      trace("-t select by tty.\n");
      arg=get_opt_arg();
      if(!arg) return "List of terminals (pty, tty...) must follow -t.";
      err=parse_list(arg, parse_tty);
      if(err) return err;
      selection_list->typecode = SEL_TTY;
      return NULL; /* can't have any more options */
    case 'u': /* end */
      trace("-u select by user ID (the EUID?) (supports names).\n");
      arg=get_opt_arg();
      if(!arg) return "List of users must follow -p.";
      err=parse_list(arg, parse_uid);
      if(err) return err;
      selection_list->typecode = SEL_EUID;
      return NULL; /* can't have any more options */
    case 'w':
      trace("-w wide output.\n");
      w_count++;
      break;
#ifdef NOBODY_HAS_BSD_HABITS_ANYMORE
    case 'x':     /* Same as -y, but for System V Release 4 MP */
      trace("-x works like Sun Solaris & SCO Unixware -y option\n");
      format_modifiers |= FM_y;
      break;
#endif
    case 'y':  /* Sun's -l hack (also: Irix "lnode" resource control info) */
      trace("-y Print lnone info in UID/USER column or do Sun -l hack.\n");
      format_modifiers |= FM_y;
      break;
    case 'z':     /* alias of Mandatory Access Control level info */
      trace("-z shows aliased MAC info\n");
      return "Don't understand MAC on Linux.";
      break;
    case '-':
      printf("ARRRGH!!! -\n");
      return "Embedded '-' among SysV options makes no sense.";
      break;
    case '\0':
      printf("ARRRGH!!! \\0\n");
      return "Please report the \"SysV \\0 can't happen\" bug.";
      break;
    default:
      return "Unsupported SysV option.";
    } /* switch */
  } /* while */
  return NULL;
}

/************************* parse BSD options **********************/
static const char *parse_bsd_option(void){
  const char *arg;
  const char *err;

#if 0
  fprintf(stderr, "(%s)   %sforce_bsd   %s(personality & PER_FORCE_BSD)\n",
    ps_argv[thisarg],
    (force_bsd?"":"!"),
    ((personality & PER_FORCE_BSD)?"":"!")
  );
#endif

  flagptr = ps_argv[thisarg];  /* assume we _have_ a '-' */
  if(flagptr[0]=='-'){
    if(!force_bsd) return "Can't happen!  Problem #1.";
  }else{
    flagptr--; /* off beginning, will increment before use */
    if(personality & PER_FORCE_BSD){
      if(!force_bsd) return "Can't happen!  Problem #2.";
    }else{
      if(force_bsd) return "2nd chance parse failed, not BSD or SysV.";
    }
  }

  while(*++flagptr){
    switch(*flagptr){
    case 'A':
      /* maybe this just does a larger malloc() ? */
      trace("A Increases the argument space (Digital Unix)\n");
      return "Option A is reserved.";
      break;
    case 'C':
      /* should divide result by 1-(e**(foo*log(bar))) */
      trace("C Use raw CPU time for %%CPU instead of decaying ave\n");
      return "Option C is reserved.";
      break;
    case 'L': /* single */
      trace("L List all format specifiers\n");
      exclusive("L");
      print_format_specifiers();
      exit(0);
    case 'M':
      trace("M junk (use alternate core)\n");
      return "Option M is unsupported, try N or -n instead.";
      break;
    case 'N': /* end */
      trace("N Specify namelist file\n");
      arg=get_opt_arg();
      if(!arg) return "System.map or psdatabase must follow N.";
      namelist_file = arg;
      return NULL; /* can't have any more options */
    case 'O': /* end */
      trace("O Like o + defaults, add new columns after PID. Also sort.\n");
      arg=get_opt_arg();
      if(!arg) return "Format or sort specification must follow O.";
      defer_sf_option(arg, SF_B_O);
      return NULL; /* can't have any more options */
      break;
    case 'S':
      trace("S include dead kids in sum\n");
      include_dead_children = 1;
      break;
    case 'T':
      trace("T Select all processes on this terminal\n");
      /* put our tty on a tiny list */
      {
        selection_node *node;
        node = malloc(sizeof(selection_node));
        node->u = malloc(sizeof(sel_union));
        node->u[0].tty = cached_tty;
        node->typecode = SEL_TTY;
        node->n = 1;
        node->next = selection_list;
        selection_list = node;
      }
      break;
    case 'U': /* end */
      trace("U Select processes for specified users.\n");
      arg=get_opt_arg();
      if(!arg) return "List of users must follow U.";
      err=parse_list(arg, parse_uid);
      if(err) return err;
      selection_list->typecode = SEL_EUID;
      return NULL; /* can't have any more options */
    case 'V': /* single */
      trace("V show version info\n");
      exclusive("V");
      display_version();
      exit(0);
    case 'W':
      trace("W N/A get swap info from ... not /dev/drum.\n");
      return "Obsolete W option not supported. (You have a /dev/drum?)";
      break;
    case 'X':
      trace("X Old Linux i386 register format\n");
      format_flags |= FF_LX;
      break;
    case 'a':
      trace("a Select all w/tty, including other users\n");
      /* Now the PER_SUN_MUTATE_a flag is handled elsewhere. */
      /* if(personality & PER_SUN_MUTATE_a) simple_select |= SS_U_a;
      else               */              simple_select |= SS_B_a;
      break;
    case 'c':
      trace("c true command name\n");
      bsd_c_option = 1;
      break;
    case 'e':
      trace("e environment\n");
      bsd_e_option = 1;
      break;
    case 'f':
      trace("f ASCII art forest\n");
      forest_type = 'b';
      break;
    case 'g':
      trace("g _all_, even group leaders!.\n");
      simple_select |= SS_B_g;
      break;
    case 'h':
      trace("h Repeat header... yow.\n");
      if(header_type) return "Only one heading option may be specified.";
      if(personality & PER_BSD_h) header_type = HEAD_MULTI;
      else                        header_type = HEAD_NONE;
      break;
    case 'j':
      trace("j job control format\n");
      format_flags |= FF_Bj;
      break;
    case 'k':
      trace("k N/A Use /vmcore as c-dumpfile\n");
      return "Obsolete k option not supported.";
      break;
    case 'l':
      trace("l Display long format\n");
      format_flags |= FF_Bl;
      break;
    case 'm':
      trace("m all threads, sort on mem use, show mem info\n");
      if(personality & PER_OLD_m){
        format_flags |= FF_Lm;
        break;
      }
      if(personality & PER_BSD_m){
        defer_sf_option("pmem", SF_B_m);
        break;
      }
      return "Thread display not implemented.";
      break;
    case 'n':
      trace("n Numeric output for WCHAN and USER replaced by UID\n");
      wchan_is_number = 1;
      user_is_number = 1;
      /* TODO add tty_is_number too? */
      break;
    case 'o': /* end */
      trace("o Specify user-defined format\n");
      arg=get_opt_arg();
      if(!arg) return "Format specification must follow -o.";
      defer_sf_option(arg, SF_B_o);
      return NULL; /* can't have any more options */
    case 'p': /* end */
      trace("p Select by process ID\n");
      arg=get_opt_arg();
      if(!arg) return "List of process IDs must follow p.";
      err=parse_list(arg, parse_pid);
      if(err) return err;
      selection_list->typecode = SEL_PID;
      return NULL; /* can't have any more options */
    case 'r':
      trace("r Select running processes\n");
      running_only = 1;
      break;
    case 's':
      trace("s Display signal format\n");
      format_flags |= FF_Bs;
      break;
    case 't': /* end */
      trace("t Select by tty.\n");
      /* List of terminals (tty, pty...) _should_ follow t. */
      arg=get_opt_arg();
      if(!arg){
        /* Wow, obsolete BSD syntax. Put our tty on a tiny list. */
        selection_node *node;
        node = malloc(sizeof(selection_node));
        node->u = malloc(sizeof(sel_union));
        node->u[0].tty = cached_tty;
        node->typecode = SEL_TTY;
        node->n = 1;
        node->next = selection_list;
        selection_list = node;
        return NULL;
      }
      err=parse_list(arg, parse_tty);
      if(err) return err;
      selection_list->typecode = SEL_TTY;
      return NULL; /* can't have any more options */
    case 'u':
      trace("u Display user-oriented\n");
      format_flags |= FF_Bu;
      break;
    case 'v':
      trace("v Display virtual memory\n");
      format_flags |= FF_Bv;
      break;
    case 'w':
      trace("w wide output\n");
      w_count++;
      break;
    case 'x':
      trace("x Select processes without controlling ttys\n");
      simple_select |= SS_B_x;
      break;
    case '-':
      printf("ARRRGH!!! -\n");
      return "Embedded '-' among BSD options makes no sense.";
      break;
    case '\0':
      printf("ARRRGH!!! \\0\n");
      return "Please report the \"BSD \\0 can't happen\" bug.";
      break;
    default:
      if (*flagptr >= '0' && *flagptr <= '9') {
        /* BSD syntax allows (in fact, on some systems requires)
	 * being able to include the pid after the options with
	 * NO intervening space.  :-(
	 */
        selection_node *pidnode;

        pidnode = calloc(1, sizeof(selection_node));
	pidnode->u = calloc(1, sizeof(sel_union));
	pidnode->n = 1;
        parse_pid(flagptr, pidnode->u);
	pidnode->next = selection_list;
	selection_list = pidnode;
	selection_list->typecode = SEL_PID;
	return NULL; /* do not iterate over the rest of the digits */
      } else {
        return "Unsupported option (BSD syntax)";
      }
    } /* switch */
  } /* while */
  return NULL;
}

/*************** gnu long options **********************/

/*
 * Return the argument or NULL
 */
static const char *grab_gnu_arg(void){
  switch(*flagptr){     /* argument is part of ps_argv[thisarg] */
  default:
    return NULL;                     /* something bad */
  case '=': case ':':
    if(*++flagptr) return flagptr;   /* found it */
    return NULL;                     /* empty '=' or ':' */
  case '\0': /* try next argv[] */
  }
  if(thisarg+2 > ps_argc) return NULL;   /* there is nothing left */
  /* argument follows ps_argv[thisarg] */
  if(*(ps_argv[thisarg+1]) == '\0') return NULL;
  return ps_argv[++thisarg];
}

typedef struct gnu_table_struct {
  const char *name; /* long option name */
  const void *jump; /* See gcc extension info.   :-)   */
} gnu_table_struct;

static int compare_gnu_table_structs(const void *a, const void *b){
  return strcmp(((gnu_table_struct*)a)->name,((gnu_table_struct*)b)->name);
}

/* Option arguments are after ':', after '=', or in argv[n+1] */
static const char *parse_gnu_option(void){
  const char *arg;
  const char *err;
  char *s;
  size_t sl;
  char buf[16];
  gnu_table_struct findme = { buf, NULL};
  gnu_table_struct *found;
  static const gnu_table_struct gnu_table[] = {
  {"Group",         &&case_Group},       /* rgid */
  {"User",          &&case_User},        /* ruid */
  {"cols",          &&case_cols},
  {"columns",       &&case_columns},
  {"cumulative",    &&case_cumulative},
  {"deselect",      &&case_deselect},    /* -N */
  {"forest",        &&case_forest},      /* f -H */
  {"format",        &&case_format},
  {"group",         &&case_group},       /* egid */
  {"header",        &&case_header},
  {"headers",       &&case_headers},
  {"heading",       &&case_heading},
  {"headings",      &&case_headings},
  {"help",          &&case_help},
  {"html",          &&case_html},
  {"info",          &&case_info},
  {"lines",         &&case_lines},
  {"no-header",     &&case_no_header},
  {"no-headers",    &&case_no_headers},
  {"no-heading",    &&case_no_heading},
  {"no-headings",   &&case_no_headings},
  {"noheader",      &&case_noheader},
  {"noheaders",     &&case_noheaders},
  {"noheading",     &&case_noheading},
  {"noheadings",    &&case_noheadings},
  {"nul",           &&case_nul},
  {"null",          &&case_null},
  {"pid",           &&case_pid},
  {"rows",          &&case_rows},
  {"sid",           &&case_sid},
  {"sort",          &&case_sort},
  {"tty",           &&case_tty},
  {"user",          &&case_user},        /* euid */
  {"version",       &&case_version},
  {"width",         &&case_width},
  {"zero",          &&case_zero}
  };
  const int gnu_table_count = sizeof(gnu_table)/sizeof(gnu_table_struct);

  s = ps_argv[thisarg]+2;
  sl = strcspn(s,":=");
  if(sl > 15) return "Unknown gnu long option.";
  strncpy(buf, s, sl);
  buf[sl] = '\0';
  flagptr = s+sl;

  found = bsearch(&findme, gnu_table, gnu_table_count,
      sizeof(gnu_table_struct), compare_gnu_table_structs
  );

  if(!found) return "Unknown gnu long option.";

  goto *(found->jump);    /* See gcc extension info.  :-)   */

  case_Group:
    trace("--Group\n");
    arg = grab_gnu_arg();
    if(!arg) return "List of real groups must follow --Group.";
    err=parse_list(arg, parse_gid);
    if(err) return err;
    selection_list->typecode = SEL_RGID;
    return NULL;
  case_User:
    trace("--User\n");
    arg = grab_gnu_arg();
    if(!arg) return "List of real users must follow --User.";
    err=parse_list(arg, parse_uid);
    if(err) return err;
    selection_list->typecode = SEL_RUID;
    return NULL;
  case_cols:
  case_width:
  case_columns:
    trace("--cols\n");
    arg = grab_gnu_arg();
    if(arg && *arg){
      long t;
      char *endptr;
      t = strtol(arg, &endptr, 0);
      if(!*endptr && (t>0) && (t<2000000000)){
        screen_cols = (int)t;
        return NULL;
      }
    }
    return "Number of columns must follow --cols, --width, or --columns.";
  case_cumulative:
    trace("--cumulative\n");
    if(s[sl]) return "Option --cumulative does not take an argument.";
    include_dead_children = 1;
    return NULL;
  case_deselect:
    trace("--deselect\n");
    if(s[sl]) return "Option --deselect does not take an argument.";
    negate_selection = 1;
    return NULL;
  case_no_header:
  case_no_headers:
  case_no_heading:
  case_no_headings:
  case_noheader:
  case_noheaders:
  case_noheading:
  case_noheadings:
    trace("--noheaders\n");
    if(s[sl]) return "Option --no-heading does not take an argument.";
    if(header_type) return "Only one heading option may be specified.";
    header_type = HEAD_NONE;
    return NULL;
  case_header:
  case_headers:
  case_heading:
  case_headings:
    trace("--headers\n");
    if(s[sl]) return "Option --heading does not take an argument.";
    if(header_type) return "Only one heading option may be specified.";
    header_type = HEAD_MULTI;
    return NULL;
  case_forest:
    trace("--forest\n");
    if(s[sl]) return "Option --forest does not take an argument.";
    forest_type = 'g';
    return NULL;
  case_format:
    trace("--format\n");
    arg=grab_gnu_arg();
    if(!arg) return "Format specification must follow --format.";
    defer_sf_option(arg, SF_G_format);
    return NULL;
  case_group:
    trace("--group\n");
    arg = grab_gnu_arg();
    if(!arg) return "List of effective groups must follow --group.";
    err=parse_list(arg, parse_gid);
    if(err) return err;
    selection_list->typecode = SEL_EGID;
    return NULL;
  case_help:
    trace("--help\n");
    exclusive("--help");
    fputs(help_message, stdout);
    exit(0);
    return NULL;
  case_info:
    trace("--info\n");
    exclusive("--info");
    self_info();
    exit(0);
    return NULL;
  case_html:
    trace("--html\n");
    if(s[sl]) return "Option --html does not take an argument.";
    return "Sorry, --html is not implemented";
    return NULL;
  case_pid:
    trace("--pid\n");
    arg = grab_gnu_arg();
    if(!arg) return "List of process IDs must follow --pid.";
    err=parse_list(arg, parse_pid);
    if(err) return err;
    selection_list->typecode = SEL_PID;
    return NULL;
  case_rows:
  case_lines:
    trace("--rows\n");
    arg = grab_gnu_arg();
    if(arg && *arg){
      long t;
      char *endptr;
      t = strtol(arg, &endptr, 0);
      if(!*endptr && (t>0) && (t<2000000000)){
        screen_rows = (int)t;
        return NULL;
      }
    }
    return "Number of rows must follow --rows or --lines.";
  case_null:
  case_nul:
  case_zero:
    trace("--null\n");
    if(s[sl]) return "Option --null does not take an argument.";
    return "Sorry, --null is not implemented";
    return NULL;
  case_sid:
    trace("--sid\n");
    arg = grab_gnu_arg();
    if(!arg) return "Some sid thing(s) must follow --sid.";
    err=parse_list(arg, parse_pid);
    if(err) return err;
    selection_list->typecode = SEL_SESS;
    return NULL;
  case_sort:
    trace("--sort\n");
    arg=grab_gnu_arg();
    if(!arg) return "Long sort specification must follow --sort.";
    defer_sf_option(arg, SF_G_sort);
    return NULL;
  case_tty:
    trace("--tty\n");
    arg = grab_gnu_arg();
    if(!arg) return "List of ttys must follow --tty.";
    err=parse_list(arg, parse_tty);
    if(err) return err;
    selection_list->typecode = SEL_TTY;
    return NULL;
  case_user:
    trace("--user\n");
    arg = grab_gnu_arg();
    if(!arg) return "List of effective users must follow --user.";
    err=parse_list(arg, parse_uid);
    if(err) return err;
    selection_list->typecode = SEL_EUID;
    return NULL;
  case_version:
    trace("--version\n");
    exclusive("--version");
    display_version();
    exit(0);
    return NULL;
}

/*************** process trailing PIDs  **********************/
static const char *parse_trailing_pids(void){
  selection_node *pidnode;  /* pid */
  selection_node *grpnode;  /* process group */
  selection_node *sidnode;  /* session */
  char **argp;     /* pointer to pointer to text of PID */
  const char *err;       /* error code that could or did happen */
  int i;

  i = ps_argc - thisarg;  /* how many trailing PIDs, SIDs, PGRPs?? */
  argp = ps_argv + thisarg;
  thisarg = ps_argc - 1;   /* we must be at the end now */

  pidnode = malloc(sizeof(selection_node));
  pidnode->u = malloc(i*sizeof(sel_union)); /* waste is insignificant */
  pidnode->n = 0;

  grpnode = malloc(sizeof(selection_node));
  grpnode->u = malloc(i*sizeof(sel_union)); /* waste is insignificant */
  grpnode->n = 0;

  sidnode = malloc(sizeof(selection_node));
  sidnode->u = malloc(i*sizeof(sel_union)); /* waste is insignificant */
  sidnode->n = 0;

  while(i--){
    char *data;
    data = *(argp++);
    switch(*data){
    default:   err = parse_pid(  data, pidnode->u + pidnode->n++); break;
    case '-':  err = parse_pid(++data, grpnode->u + grpnode->n++); break;
    case '+':  err = parse_pid(++data, sidnode->u + sidnode->n++); break;
    }
    if(err) return err;     /* the node gets freed with the list */
  }

  if(pidnode->n){
    pidnode->next = selection_list;
    selection_list = pidnode;
    selection_list->typecode = SEL_PID;
  }  /* else free both parts */

  if(grpnode->n){
    grpnode->next = selection_list;
    selection_list = grpnode;
    selection_list->typecode = SEL_PGRP;
  }  /* else free both parts */

  if(sidnode->n){
    sidnode->next = selection_list;
    selection_list = sidnode;
    selection_list->typecode = SEL_SESS;
  }  /* else free both parts */

  return NULL;
}

/************** misc stuff ***********/

static void reset_parser(void){
  w_count = 0;
}

static int arg_type(const char *str){
  int tmp = str[0];
  if((tmp>='a') && (tmp<='z'))   return ARG_BSD;
  if((tmp>='A') && (tmp<='Z'))   return ARG_BSD;
  if((tmp>='0') && (tmp<='9'))   return ARG_PID;
  if(tmp=='+')                   return ARG_SESS;
  if(tmp!='-')                   return ARG_FAIL;
  tmp = str[1];
  if((tmp>='a') && (tmp<='z'))   return ARG_SYSV;
  if((tmp>='A') && (tmp<='Z'))   return ARG_SYSV;
  if((tmp>='0') && (tmp<='9'))   return ARG_PGRP;
  if(tmp!='-')                   return ARG_FAIL;
  tmp = str[2];
  if((tmp>='a') && (tmp<='z'))   return ARG_GNU;
  if((tmp>='A') && (tmp<='Z'))   return ARG_GNU;
  if(tmp=='\0')                  return ARG_END;
                                 return ARG_FAIL;
}

/* First assume sysv, because that is the POSIX and Unix98 standard. */
static const char *parse_all_options(void){
  const char *err = NULL;
  int at;
  while(++thisarg < ps_argc){
  trace("parse_all_options calling arg_type for \"%s\"\n", ps_argv[thisarg]);
    at = arg_type(ps_argv[thisarg]);
    trace("ps_argv[thisarg] is %s\n", ps_argv[thisarg]);
    if(at != ARG_SYSV) not_pure_unix = 1;
    switch(at){
    case ARG_GNU:
      err = parse_gnu_option();
      break;
    case ARG_SYSV:
      if(!force_bsd){   /* else go past case ARG_BSD */
        err = parse_sysv_option();
        break;
    case ARG_BSD:
        if(force_bsd && !(personality & PER_FORCE_BSD)) return "way bad";
      }
      prefer_bsd = 1;
      err = parse_bsd_option();
      break;
    case ARG_PGRP:
    case ARG_SESS:
    case ARG_PID:
      prefer_bsd = 1;
      err = parse_trailing_pids();
      break;
    case ARG_END:
    case ARG_FAIL:
      trace("              FAIL/END on [%s]\n",ps_argv[thisarg]);
      return "Garbage option.";
      break;
    default:
      printf("                  ?    %s\n",ps_argv[thisarg]);
      return "Something broke.";
    } /* switch */
    if(err) return err;
  } /* while */
  return NULL;
}

static void choose_dimensions(void){
  if(w_count && (screen_cols<132)) screen_cols=132;
  if(w_count>1) screen_cols=OUTBUF_SIZE;
  /* perhaps --html and --null should set unlimited width */
}

int arg_parse(int argc, char *argv[]){
  const char *err = NULL;
  const char *err2 = NULL;
  ps_argc = argc;
  ps_argv = argv;
  thisarg = 0;

#if 0
  {int debugloop = 0; while(debugloop<argc){
  trace("argv[%d]=%s\n", debugloop, argv[debugloop]); debugloop++;}}
#endif

  if(personality & PER_FORCE_BSD) goto try_bsd;

  err = parse_all_options();
  if(err) goto try_bsd;
  err = process_sf_options(!not_pure_unix);
  if(err) goto try_bsd;
  err = select_bits_setup();
  if(err) goto try_bsd;

  choose_dimensions();
  return 0;

try_bsd:
  trace("--------- now try BSD ------\n");

  reset_global();
  reset_parser();
  reset_sortformat();
  reset_sort_options();
  format_flags = 0;
  ps_argc = argc;
  ps_argv = argv;
  thisarg = 0;
  /* no need to reset flagptr */
  not_pure_unix=1;
  force_bsd=1;

  err2 = parse_all_options();
  if(err2) goto total_failure;
  err2 = process_sf_options(!not_pure_unix);
  if(err2) goto total_failure;
  err2 = select_bits_setup();
  if(err2) goto total_failure;

#if 0
  /* I promised people that this would go away with the new version... */
  if(!(personality & PER_FORCE_BSD))
    fprintf(stderr, "Bad syntax, perhaps a bogus '-'?\n");
#endif

  choose_dimensions();
  return 0;

total_failure:
  reset_parser();
  if(personality & PER_FORCE_BSD) fprintf(stderr, "ps: error: %s\n", err2);
  else fprintf(stderr, "ps: error: %s\n", err);
  fputs(usage_message, stderr);
  exit(1);
  /* return 1; */ /* useless */
}
