/*
 * Copyright 1999 by Albert Cahalan; all rights resered.
 *
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */

/*
 * This file is really gross, and I know it. I looked into several
 * alternate ways to deal with the mess, and they were all ugly.
 *
 * FreeBSD has a fancy hack using offsets into a struct -- that
 * saves code but it is _really_ gross. See the PO macro below.
 *
 * We could have a second column width for wide output format.
 * For example, Digital prints the real-time signals.
 */


/*
 * Data table idea:
 *
 * table 1 maps aix to specifier
 * table 2 maps shortsort to specifier
 * table 3 maps macro to specifiers
 * table 4 maps specifier to title,datatype,offset,vendor,helptext
 * table 5 maps datatype to justification,width,widewidth,sorting,printing
 *
 * Here, "datatype" could be user,uid,u16,pages,deltaT,signals,tty,longtty...
 * It must be enough to determine printing and sorting.
 *
 * After the tables, increase width as needed to fit the header.
 *
 * Table 5 could go in a file with the output functions.
 */

 
/* proc_t offset macro */
#define PO(q) ((unsigned long)(&(((proc_t*)0)->q)))

#include <ctype.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "../proc/readproc.h"
#include "../proc/sysinfo.h"
#include "../proc/status.h"
#include "../proc/procps.h"
#include "../proc/devname.h"
#include "common.h"


/* TODO:
 * Stop assuming system time is local time.
 */

static char  whitespace_and_outbuf[OUTBUF_SIZE + SPACE_AMOUNT + PAGE_SIZE*2];
static char *outbuf = whitespace_and_outbuf+SPACE_AMOUNT;
static char *whitespace = whitespace_and_outbuf;
static unsigned max_rightward = 0x12345678; /* space for RIGHT stuff */
static unsigned max_leftward = 0x12345678; /* space for LEFT stuff */

static int wide_signals;  /* true if we have room */

static proc_t *pp;     /* the process being printed */

unsigned long seconds_since_boot;
static unsigned long seconds_since_1970;
static unsigned long time_of_boot;
static unsigned long bytes_main_memory;
static unsigned long page_shift;


/*************************************************************************/
/************ Lots of sort functions, starting with the NOP **************/

static int sr_nop(const proc_t* a, const proc_t* b){
  (void)a;(void)b; /* shut up gcc */
  return 0;
}

#define CMP_STR(NAME) \
static int sr_ ## NAME(const proc_t* P, const proc_t* Q) { \
    return strcmp(P-> ## NAME, Q-> ## NAME); \
}

#define CMP_INT(NAME) \
static int sr_ ## NAME (const proc_t* P, const proc_t* Q) { \
    if (P-> ## NAME < Q-> ## NAME) return -1; \
    if (P-> ## NAME > Q-> ## NAME) return  1; \
    return 0; \
}

CMP_INT(cutime)
CMP_INT(cstime)
CMP_INT(priority)                                             /* nice */
CMP_INT(timeout)
CMP_INT(nice)                                                 /* priority */
CMP_INT(rss)      /* resident set size from stat file */ /* vm_rss, resident */
CMP_INT(it_real_value)
CMP_INT(size)      /* total pages */                     /* vm_size, vsize */
CMP_INT(resident)  /* resident pages */                     /* vm_rss, rss */
CMP_INT(share)     /* shared pages */
CMP_INT(trs)       /* executable pages */
CMP_INT(lrs)       /* obsolete "library" pages above 0x60000000 */
CMP_INT(drs)       /* other pages (assumed data?) */
CMP_INT(dt)        /* dirty pages */

CMP_INT(vm_size)    /* kB VM */                             /* size, vsize */
CMP_INT(vm_lock)    /* kB locked */
CMP_INT(vm_rss)     /* kB rss */                          /* rss, resident */
CMP_INT(vm_data)    /* kB "data" == data-stack */
CMP_INT(vm_stack)   /* kB stack */
CMP_INT(vm_exe)     /* kB "exec" == exec-lib */
CMP_INT(vm_lib)     /* kB "libraries" */
CMP_INT(vsize)      /* pages VM */                        /* size, vm_size */
CMP_INT(rss_rlim)
CMP_INT(flags)
CMP_INT(min_flt)
CMP_INT(maj_flt)
CMP_INT(cmin_flt)
CMP_INT(cmaj_flt)
CMP_INT(nswap)
CMP_INT(cnswap)
CMP_INT(utime)
CMP_INT(stime)    /* Old: sort by systime. New: show start time. Uh oh. */
CMP_INT(start_code)
CMP_INT(end_code)
CMP_INT(start_stack)
CMP_INT(kstk_esp)
CMP_INT(kstk_eip)
CMP_INT(start_time)
CMP_INT(wchan)

/* CMP_STR(*environ) */
/* CMP_STR(*cmdline) */

CMP_STR(ruser)
CMP_STR(euser)
CMP_STR(suser)
CMP_STR(fuser)
CMP_STR(rgroup)
CMP_STR(egroup)
CMP_STR(sgroup)
CMP_STR(fgroup)
CMP_STR(cmd)
/* CMP_STR(ttyc) */    /* FIXME -- use strncmp with 8 max */

CMP_INT(ruid)
CMP_INT(rgid)
CMP_INT(euid)
CMP_INT(egid)
CMP_INT(suid)
CMP_INT(sgid)
CMP_INT(fuid)
CMP_INT(fgid)
CMP_INT(pid)
CMP_INT(ppid)
CMP_INT(pgrp)
CMP_INT(session)
CMP_INT(tty)
CMP_INT(tpgid)

CMP_INT(pcpu)

CMP_INT(state)

/***************************************************************************/
/************ Lots of format functions, starting with the NOP **************/

static int pr_nop(void){
  return sprintf(outbuf, "%c", '-');
}


/********* Unix 98 ************/

/***

Only comm and args are allowed to contain blank characters; all others are
not. Any implementation-dependent variables will be specified in the system
documentation along with the default header and indicating if the field
may contain blank characters.

Some headers do not have a standardized specifier!

%CPU	pcpu	The % of cpu time used recently, with unspecified "recently".
ADDR		The address of the process.
C		Processor utilisation for scheduling.
CMD		The command name, or everything with -f.
COMMAND	args	Command + args. May chop as desired. May use either version.
COMMAND	comm	argv[0]
ELAPSED	etime	Elapsed time since the process was started. [[dd-]hh:]mm:ss
F		Flags (octal and additive)
GROUP	group	Effective group ID, prefer text over decimal.
NI	nice	Decimal system scheduling priority, see nice(1).
PGID	pgid	The decimal value of the process group ID.
PID	pid	Decimal PID.
PPID	ppid	Decimal PID.
PRI		Priority. Higher numbers mean lower priority.
RGROUP	rgroup	Real group ID, prefer text over decimal.
RUSER	ruser	Real user ID, prefer text over decimal.
S		The state of the process.
STIME		Starting time of the process.
SZ		The size in blocks of the core image of the process.
TIME	time	Cumulative CPU time. [dd-]hh:mm:ss
TT	tty	Name of tty in format used by who(1).
TTY		The controlling terminal for the process.
UID		UID, or name when -f
USER	user	Effective user ID, prefer text over decimal.
VSZ	vsz	Virtual memory size in decimal kB.
WCHAN		Where waiting/sleeping or blank if running.

The nice value is used to compute the priority.

For some undefined ones, Digital does:

F       flag    Process flags -- but in hex!
PRI     pri     Process priority
S       state   Symbolic process status
TTY     tt,tty,tname,longtname  -- all do "ttyp1", "console", "??"
UID     uid     Process user ID (effective UID)
WCHAN   wchan   Address of event on which a

For some undefined ones, Sun does:

ADDR	addr	memory address of the process
C	c	Processor utilization  for  scheduling  (obsolete).
CMD
F	f
S	s	state: OSRZT
STIME		start time, printed w/o blanks. If 24h old, months & days
SZ		size (in pages) of the swappable process's image in main memory
TTY
UID	uid
WCHAN	wchan

For some undefined ones, SCO does:
ADDR	addr	Virtual address of the process' entry in the process table.
SZ		swappable size in kB of the virtual data and stack
STIME	stime	hms or md time format
***/

/* Source & destination are known. Return bytes or screen characters? */
static int forest_helper(void){
  char *p = forest_prefix;
  char *q = outbuf;
  if(!*p) return 0;
  /* Arrrgh! somebody defined unix as 1 */
  if(forest_type == 'u') goto unixy;
  p++;   /* don't show everything as init's child */
  while(*p){
    switch(*p){
    case ' ': strcpy(q, "    ");  break;
    case 'L': strcpy(q, " \\_ "); break;
    case '+': strcpy(q, " \\_ "); break;
    case '|': strcpy(q, " |  ");  break;
    case '\0': return q-outbuf;    /* redundant & not used */
    }
    q += 4;
    p++;
  }
  return q-outbuf;   /* gcc likes this here */
unixy:
  while(*p){
    switch(*p){
    case ' ': strcpy(q, "  "); break;
    case 'L': strcpy(q, "  "); break;
    case '+': strcpy(q, "  "); break;
    case '|': strcpy(q, "  "); break;
    case '\0': return q-outbuf;    /* redundant & not used */
    }
    q += 2;
    p++;
  }
  return q-outbuf;   /* gcc likes this here */
}


/* XPG4-UNIX, according to Digital:
The "args" and "command" specifiers show what was passed to the command.
Modifications to the arguments are not shown.
*/

/*
 * pp->cmd       short accounting name (comm & ucomm)
 * pp->cmdline   long name with args (args & command)
 * pp->environ   environment
 */

/* "command" is the same thing: long unless c */
static int pr_args(void){
  char *endp;
  endp = outbuf + forest_helper();
  if(bsd_c_option){
    endp += escape_str(endp, pp->cmd, PAGE_SIZE); /* short version */
  }else{
    const char **lc = (const char**)pp->cmdline; /* long version */
    if(lc && *lc) {
      endp += escape_strlist(endp, lc, OUTBUF_SIZE);
    } else {
      char buf[ESC_STRETCH*PAGE_SIZE];
      escape_str(buf, pp->cmd, ESC_STRETCH*PAGE_SIZE);
      endp += sprintf(endp, "[%s]", buf);
    }
  }
  if(bsd_e_option){
    const char **env = (const char**)pp->environ;
    if(env && *env){
      *endp++ = ' ';
      endp += escape_strlist(endp, env, OUTBUF_SIZE);
    }
  }
  return endp - outbuf;
}

/* "ucomm" is the same thing: short unless -f */
static int pr_comm(void){
  char *endp;
  endp = outbuf + forest_helper();
  if(!unix_f_option){ /* does -f matter? */
    endp += escape_str(endp, pp->cmd, PAGE_SIZE); /* short version */
  }else{
    const char **lc = (const char**)pp->cmdline; /* long version */
    if(lc && *lc) {
      endp += escape_strlist(endp, lc, OUTBUF_SIZE);
    } else {
      char buf[ESC_STRETCH*PAGE_SIZE];
      escape_str(buf, pp->cmd, ESC_STRETCH*PAGE_SIZE);
      endp += sprintf(endp, "[%s]", buf);
    }
  }
  if(bsd_e_option){
    const char **env = (const char**)pp->environ;
    if(env && *env){
      *endp++ = ' ';
      endp += escape_strlist(endp, env, OUTBUF_SIZE);
    }
  }
  return endp - outbuf;
}
/* Non-standard, from SunOS 5 */
static int pr_fname(void){
  char *endp;
  endp = outbuf + forest_helper();
  endp += escape_str(endp, pp->cmd, 8);
  return endp - outbuf;
}

/* elapsed wall clock time, [[dd-]hh:]mm:ss format (not same as "time") */
static int pr_etime(void){
  unsigned t, dd,hh,mm,ss;
  int c = 0;
  t = (
          ((unsigned long)seconds_since_boot)
        - ((unsigned long)pp->start_time)
        / Hertz
      );
  ss = t%60;
  t /= 60;
  mm = t%60;
  t /= 60;
  hh = t%24;
  t /= 24;
  dd = t;
  c +=(      dd      ?  sprintf(outbuf, "%u-", dd)    :  sprintf(outbuf, "   ")   );
  c +=(  (dd || hh)  ?  sprintf(outbuf+c, "%02u:", hh)  :  sprintf(outbuf+c, "   ")   );
  c +=(                 sprintf(outbuf+c, "%02u:%02u", mm, ss)             );
  return c;
}
static int pr_nice(void){
  return sprintf(outbuf, "%ld", pp->nice);
}

/* "Processor utilisation for scheduling."  --- we use %cpu w/o fraction */
static int pr_c(void){
  return sprintf(outbuf, "%2u", (unsigned)(pp->pcpu / 10));
}
static int pr_pcpu(void){
  return sprintf(outbuf, "%2u.%u",
  		(unsigned)(pp->pcpu / 10), (unsigned)(pp->pcpu % 10));
}
static int pr_pgid(void){
  return sprintf(outbuf, "%u", pp->pgrp);
}
static int pr_pid(void){
  return sprintf(outbuf, "%u", pp->pid);
}
static int pr_ppid(void){
  return sprintf(outbuf, "%u", pp->ppid);
}


/* cumulative CPU time, [dd-]hh:mm:ss format (not same as "etime") */
static int pr_time(void){
  int t, dd,hh,mm,ss;
  int c = 0;
  t = pp->utime + pp->stime;
  t /= Hertz;
  ss = t%60;
  t /= 60;
  mm = t%60;
  t /= 60;
  hh = t%24;
  t /= 24;
  dd = t;
  c +=( dd ? sprintf(outbuf, "%d-", dd) : 0              );
  c +=( sprintf(outbuf+c, "%02d:%02d:%02d", hh, mm, ss)    );
  return c;
}

/* HP-UX puts this in kB and uses "sz" for pages */
static int pr_vsz(void){
  return sprintf(outbuf, "%lu", pp->vm_size);
}

/*
 * internal terms:  ruid  euid  suid  fuid
 * kernel vars:      uid  euid  suid fsuid
 * command args:    ruid   uid svuid   n/a
 */

static int pr_ruser(void){
  if(user_is_number || (strlen(pp->ruser)>max_rightward))
    return sprintf(outbuf, "%d", pp->ruid);
  return sprintf(outbuf, "%s", pp->ruser);
}
static int pr_egroup(void){
  if(strlen(pp->egroup)>max_rightward) return sprintf(outbuf, "%d", pp->egid);
  return sprintf(outbuf, "%s", pp->egroup);
}
static int pr_rgroup(void){
  if(strlen(pp->rgroup)>max_rightward) return sprintf(outbuf, "%d", pp->rgid);
  return sprintf(outbuf, "%s", pp->rgroup);
}
static int pr_euser(void){
  if(user_is_number || (strlen(pp->euser)>max_rightward)) return sprintf(outbuf, "%d", pp->euid);
  return sprintf(outbuf, "%s", pp->euser);
}

/********* maybe standard (Unix98 only defines the header) **********/


/*
 * "PRI" is created by "opri", or by "pri" when -c is used.
 *
 * Unix98 only specifies that a high "PRI" is low priority.
 * Sun and SCO add the -c behavior. Sun defines "pri" and "opri".
 * Linux may use "priority" for historical purposes.
 */
static int pr_priority(void){    /* -20..20 */
    return sprintf(outbuf, "%ld", pp->priority);
}
static int pr_pri(void){         /* 20..60 */
    return sprintf(outbuf, "%ld", 39 - pp->priority);
}
static int pr_opri(void){        /* 39..79 */
    return sprintf(outbuf, "%ld", 60 + pp->priority);
}

static int pr_wchan(void){
/*
 * Unix98 says "blank if running" and also "no blanks"! :-(
 * Unix98 also says to use '-' if something is meaningless.
 * Digital uses both '*' and '-', with undocumented differences.
 * (the '*' for -1 (rare) and the '-' for 0)
 * Sun claims to use a blank AND use '-', in the same man page.
 * Perhaps "blank" should mean '-'.
 *
 * AIX uses '-' for running processes, the location when there is
 * only one thread waiting in the kernel, and '*' when there is
 * more than one thread waiting in the kernel.
 */
    if(!(pp->wchan & 0xffffff)) return sprintf(outbuf, "%s", "-");
    if(wchan_is_number) return sprintf(outbuf, "%lx", pp->wchan & 0xffffff);
    return sprintf(outbuf, "%s", wchan(pp->wchan));
}

/* Terrible trunctuation, like BSD crap uses: I999 J999 K999 */
/* FIXME: disambiguate /dev/tty69 and /dev/pts/69. */
static int pr_tty4(void){
/* sprintf(outbuf, "%02x:%02x", pp->tty>>8, pp->tty&0xff); */
  return dev_to_tty(outbuf, 4, pp->tty, pp->pid, ABBREV_DEV|ABBREV_TTY|ABBREV_PTS);
}

/* Unix98: format is unspecified, but must match that used by who(1). */
static int pr_tty8(void){
/* sprintf(outbuf, "%02x:%02x", pp->tty>>8, pp->tty&0xff); */
  return dev_to_tty(outbuf, PAGE_SIZE-1, pp->tty, pp->pid, ABBREV_DEV);
}

#if 0
/* This BSD state display may contain spaces, which is illegal. */
static int pr_oldstate(void){
    return sprintf(outbuf, "%s", status(pp));
}
#endif

/* This state display is Unix98 compliant and has lots of info like BSD. */
static int pr_stat(void){
    int end = 0;
    outbuf[end++] = pp->state;
    if(pp->rss == 0 && pp->state != 'Z')    outbuf[end++] = 'W';
    if(pp->nice < 0)                        outbuf[end++] = '<';
    if(pp->nice > 0)                        outbuf[end++] = 'N';
    if(pp->vm_lock)                         outbuf[end++] = 'L';
    outbuf[end] = '\0';
    return end;
}

/* This minimal state display is Unix98 compliant, like SCO and SunOS 5 */
static int pr_s(void){
    outbuf[0] = pp->state;
    outbuf[1] = '\0';
    return 1;
}

static int pr_flag(void){
    /* Unix98 requires octal -- good thing Linux hex looks octal! */
    return sprintf(outbuf, "%03lx", (pp->flags)&0x777);
}

static int pr_euid(void){
  return sprintf(outbuf, "%d", pp->euid);
}

/*********** non-standard ***********/

/*** BSD
sess	session pointer
(SCO has:Process session leader ID as a decimal value. (SESSION))
jobc	job control count
cpu	short-term cpu usage factor (for scheduling)
sl	sleep time (in seconds; 127 = infinity)
re	core residency time (in seconds; 127 = infinity)
pagein	pageins (same as majflt)
lim	soft memory limit
tsiz	text size (in Kbytes)
***/

static int pr_stackp(void){
    return sprintf(outbuf, "%08lx", pp->start_stack);
}

static int pr_esp(void){
    return sprintf(outbuf, "%08lx", pp->kstk_esp);
}

static int pr_eip(void){
    return sprintf(outbuf, "%08lx", pp->kstk_eip);
}

/* This function helps print old-style time formats */
static int old_time_helper(char *dst, unsigned long t, unsigned long rel) {
    if(!t)      return sprintf(dst, "    -");
    if((long)t == -1) return sprintf(dst, "   xx");
    if((long)(t-=rel) < 0)  t=0;
    if(t>9999)  return sprintf(dst, "%5lu", t/100);
    else        return sprintf(dst, "%2lu.%02lu", t/100, t%100);
}

static int pr_bsdtime(void){
    unsigned long t;
    t = pp->utime + pp->stime;
    if(include_dead_children) t += (pp->cutime + pp->cstime);
    t /= Hertz;
    return sprintf(outbuf, "%3ld:%02d", t/60, (int)(t%60));
}

static int pr_bsdstart(void){
  time_t start;
  time_t seconds_ago;
  start = time_of_boot + pp->start_time/Hertz;
  seconds_ago = seconds_since_1970 - start;
  if(seconds_ago < 0) seconds_ago=0;
  if(seconds_ago > 3600*24)  strcpy(outbuf, ctime(&start)+4);
  else                       strcpy(outbuf, ctime(&start)+10);
  outbuf[6] = '\0';
  return 6;
}

static int pr_timeout(void){
    return old_time_helper(outbuf, pp->timeout, seconds_since_boot*Hertz);
}

static int pr_alarm(void){
    return old_time_helper(outbuf, pp->it_real_value, 0);
}

/* HP-UX puts this in pages and uses "vsz" for kB */
static int pr_sz(void){
  return sprintf(outbuf, "%lu", (pp->vm_size)/(PAGE_SIZE/1024));
}


/*
 * FIXME: trs,drs,tsiz,dsiz,m_trs,m_drs,vm_exe,vm_data,trss
 * I suspect some/all of those are broken. They seem to have been
 * inherited by Linux and AIX from early BSD systems. FreeBSD only
 * retains tsiz. The prefixed versions come from Debian.
 * Sun and Digital have none of this crap. The code here comes
 * from an old Linux ps, and might not be correct for ELF executables.
 *
 * AIX            TRS    size of resident-set (real memory) of text
 * AIX            TSIZ   size of text (shared-program) image
 * FreeBSD        tsiz   text size (in Kbytes)
 * 4.3BSD NET/2   trss   text resident set size (in Kbytes)
 * 4.3BSD NET/2   tsiz   text size (in Kbytes)
 */

/* kB data size. See drs, tsiz & trs. */
static int pr_dsiz(void){
    long dsiz = 0;
    if(pp->vsize) dsiz += (pp->vsize - pp->end_code + pp->start_code) >> 10;
    return sprintf(outbuf, "%ld", dsiz);
}

/* kB text (code) size. See trs, dsiz & drs. */
static int pr_tsiz(void){
    long tsiz = 0;
    if(pp->vsize) tsiz += (pp->end_code - pp->start_code) >> 10;
    return sprintf(outbuf, "%ld", tsiz);
}

/* kB _resident_ data size. See dsiz, tsiz & trs. */
static int pr_drs(void){
    long drs = 0;
    if(pp->vsize) drs += (pp->vsize - pp->end_code + pp->start_code) >> 10;
    return sprintf(outbuf, "%ld", drs);
}

/* kB text _resident_ (code) size. See tsiz, dsiz & drs. */
static int pr_trs(void){
    long trs = 0;
    if(pp->vsize) trs += (pp->end_code - pp->start_code) >> 10;
    return sprintf(outbuf, "%ld", trs);
}



static int pr_minflt(void){
    long flt = pp->min_flt;
    if(include_dead_children) flt += pp->cmin_flt;
    return sprintf(outbuf, "%ld", flt);
}

static int pr_majflt(void){
    long flt = pp->maj_flt;
    if(include_dead_children) flt += pp->cmaj_flt;
    return sprintf(outbuf, "%ld", flt);
}

static int pr_lim(void){
    if(pp->rss_rlim == RLIM_INFINITY) return sprintf(outbuf, "%s", "xx");
    return sprintf(outbuf, "%5ld", pp->rss_rlim >> 10);
}

static int pr_nwchan(void){
    if(!(pp->wchan & 0xffffff)) return sprintf(outbuf, "-");
    return sprintf(outbuf, "%lx", pp->wchan & 0xffffff);
}

static int pr_rss(void){
  return sprintf(outbuf, "%lu", pp->rss << (page_shift - 10));
}

static int pr_pmem(void){
  unsigned long pmem = 0;
  pmem = pp->rss * 1000 / (bytes_main_memory >> page_shift);
  return sprintf(outbuf, "%2u.%u", (unsigned)(pmem/10), (unsigned)(pmem%10));
}

static int pr_lstart(void){
  time_t t;
  t = (
           ((unsigned long)time_of_boot)
         + ((unsigned long)pp->start_time)
         / Hertz
  );
  return sprintf(outbuf, "%24.24s", ctime(&t));
}

/* Unix98 specifies a STIME header for a column that shows the start
 * time of the process, but does not specify a format or format specifier.
 * From the general Unix98 rules, we know there must not be any spaces.
 * Most systems violate that rule, though the Solaris documentation
 * claims to print the column without spaces. (NOT!)
 *
 * So this isn't broken, but could be renamed to u98_std_stime,
 * as long as it still shows as STIME when using the -f option.
 */
static int pr_stime(void){
  struct tm *proc_time;
  struct tm *our_time;
  time_t t;
  char *fmt;
  int tm_year;
  int tm_yday;
  our_time = localtime(&seconds_since_1970);   /* not reentrant */
  tm_year = our_time->tm_year;
  tm_yday = our_time->tm_yday;
  t = (time_t)(
            ((unsigned long)time_of_boot)
          + ((unsigned long)pp->start_time)
          / Hertz
  );
  proc_time = localtime(&t); /* not reentrant, this corrupts our_time */
  fmt = "%H:%M";                                   /* 03:02 23:59 */
  if(tm_yday != proc_time->tm_yday) fmt = "%b%d";  /* Jun06 Aug27 */
  if(tm_year != proc_time->tm_year) fmt = "%Y";    /* 1991 2001 */
  return strftime(outbuf, 42, fmt, proc_time);
}

static int pr_start(void){
  time_t t;
  char *str;
  t = (
           ((unsigned long)time_of_boot)
         + ((unsigned long)pp->start_time)
         / Hertz
  );
  str = ctime(&t);
  if(str[8]==' ')  str[8]='0';
  if(str[11]==' ') str[11]='0';
  if((unsigned long)t+60*60*24 > seconds_since_1970)
    return sprintf(outbuf, "%8.8s", str+11);
  return sprintf(outbuf, "  %6.6s", str+4);
}


#ifdef SIGNAL_STRING
static int help_pr_sig(const char *sig){
  long len = 0;
  len = strlen(sig);
  if(wide_signals){
    if(len>8) return sprintf(outbuf, "%s", sig);
    return sprintf(outbuf, "00000000%s", sig);
  }
  if(len-strspn(sig,"0") > 8)
    return sprintf(outbuf, "<%s", sig+len-8);
  return sprintf(outbuf,  "%s", sig+len-8);
}
#else
static int help_pr_sig(unsigned long long sig){
  if(wide_signals) return sprintf(outbuf, "%016Lx", sig);
  if(sig>>32)      return sprintf(outbuf, "<%08Lx", sig&0xffffffffLL);
  return                  sprintf(outbuf,  "%08Lx", sig&0xffffffffLL);
}
#endif

static int pr_sig(void){
  return help_pr_sig(pp->signal);
}
static int pr_sigmask(void){
  return help_pr_sig(pp->blocked);
}
static int pr_sigignore(void){
  return help_pr_sig(pp->sigignore);
}
static int pr_sigcatch(void){
  return help_pr_sig(pp->sigcatch);
}


static int pr_egid(void){
  return sprintf(outbuf, "%d", pp->egid);
}
static int pr_rgid(void){
  return sprintf(outbuf, "%d", pp->rgid);
}
static int pr_sgid(void){
  return sprintf(outbuf, "%d", pp->sgid);
}
static int pr_fgid(void){
  return sprintf(outbuf, "%d", pp->fgid);
}
static int pr_ruid(void){
  return sprintf(outbuf, "%d", pp->ruid);
}
static int pr_suid(void){
  return sprintf(outbuf, "%d", pp->suid);
}
static int pr_fuid(void){
  return sprintf(outbuf, "%d", pp->fuid);
}


static int pr_fgroup(void){
  if(strlen(pp->fgroup)>max_rightward) return sprintf(outbuf, "%d", pp->fgid);
  return sprintf(outbuf, "%s", pp->fgroup);
}
static int pr_sgroup(void){
  if(strlen(pp->sgroup)>max_rightward) return sprintf(outbuf, "%d", pp->sgid);
  return sprintf(outbuf, "%s", pp->sgroup);
}
static int pr_fuser(void){
  if(user_is_number || (strlen(pp->fuser)>max_rightward)) return sprintf(outbuf, "%d", pp->fuid);
  return sprintf(outbuf, "%s", pp->fuser);
}
static int pr_suser(void){
  if(user_is_number || (strlen(pp->suser)>max_rightward)) return sprintf(outbuf, "%d", pp->suid);
  return sprintf(outbuf, "%s", pp->suser);
}


static int pr_sess(void){
  return sprintf(outbuf, "%u", pp->session);
}
static int pr_tpgid(void){
  return sprintf(outbuf, "%d", pp->tpgid);
}


/* SGI uses "cpu" to print the processor ID with header "P" */
static int pr_sgi_p(void){          /* FIXME */
  if(pp->state == 'R') return sprintf(outbuf, "%d", 0);
  return sprintf(outbuf, "*");
}



/***************************************************************************/
/*************************** other stuff ***********************************/

/*
 * Old header specifications.
 *
 * short   Up  "  PID TTY STAT  TIME COMMAND"
 * long  l Pp  " FLAGS   UID   PID  PPID PRI  NI   SIZE   RSS WCHAN       STA TTY TIME COMMAND
 * user  u up  "USER       PID %CPU %MEM  SIZE   RSS TTY STAT START   TIME COMMAND
 * jobs  j gPp " PPID   PID  PGID   SID TTY TPGID  STAT   UID   TIME COMMAND
 * sig   s p   "  UID   PID SIGNAL   BLOCKED  IGNORED  CATCHED  STAT TTY   TIME COMMAND
 * vm    v r   "  PID TTY STAT  TIME  PAGEIN TSIZ DSIZ  RSS   LIM %MEM COMMAND
 * m     m r   "  PID TTY MAJFLT MINFLT   TRS   DRS  SIZE  SWAP   RSS  SHRD   LIB  DT COMMAND
 * regs  X p   "NR   PID    STACK      ESP      EIP TMOUT ALARM STAT TTY   TIME COMMAND
 */

/*
 * Unix98 requires that the heading for tty is TT, though XPG4, Digital,
 * and BSD use TTY. The Unix98 headers are:
 *              args,comm,etime,group,nice,pcpu,pgid
 *              pid,ppid,rgroup,ruser,time,tty,user,vsz
 *
 * BSD c:   "command" becomes accounting name ("comm" or "ucomm")
 * BSD n:   "user" becomes "uid" and "wchan" becomes "nwchan" (number)
 */

/* short names to save space */
#define MEM PROC_FILLMEM     /* read statm  */
#define CMD PROC_FILLCMD     /* read cmdline */
#define ENV PROC_FILLENV     /* read environ */
#define USR PROC_FILLUSR     /* uid_t and gid_t -> user and group names */
#define BUG PROC_FILLBUG     /* what does this need? */

/* TODO
 *      pull out annoying BSD aliases into another table (to macro table?)
 *      add sorting functions here (to unify names)
 */

/* temporary hack -- mark new stuff grabbed from Debian ps */
#define LNx LNX

/* there are about 194 listed */

/* Many of these are placeholders for unsupported options. */
static const format_struct format_array[] = {
/* code       header     print()      sort()    width  ?   vendor flags  */
{"%cpu",      "%CPU",    pr_pcpu,     sr_pcpu,    4,   0,    BSD, RIGHT}, /*pcpu*/
{"%mem",      "%MEM",    pr_pmem,     sr_nop,     4,   0,    BSD, RIGHT}, /*pmem*/
{"acflag",    "ACFLG",   pr_nop,      sr_nop,     5,   0,    XXX, RIGHT}, /*acflg*/
{"acflg",     "ACFLG",   pr_nop,      sr_nop,     5,   0,    BSD, RIGHT}, /*acflag*/
{"addr",      "ADDR",    pr_nop,      sr_nop,     4,   0,    XXX, RIGHT},
{"alarm",     "ALARM",   pr_alarm,    sr_it_real_value, 5, 0, LNX, RIGHT},
{"argc",      "ARGC",    pr_nop,      sr_nop,     4,   0,    LNX, RIGHT},
{"args",      "COMMAND", pr_args,     sr_nop,    16,   0,    U98, UNLIMITED}, /*command*/
{"blocked",   "BLOCKED", pr_sigmask,  sr_nop,     9,   0,    BSD, SIGNAL}, /*sigmask*/
{"bnd",       "BND",     pr_nop,      sr_nop,     1,   0,    AIX, RIGHT},
{"bsdstart",  "START",   pr_bsdstart, sr_nop,     6,   0,    LNX, RIGHT},
{"bsdtime",   "TIME",    pr_bsdtime,  sr_nop,     6,   0,    LNX, RIGHT},
{"c",         "C",       pr_c,        sr_pcpu,    2,   0,    SUN, RIGHT},
{"caught",    "CAUGHT",  pr_sigcatch, sr_nop,     9,   0,    BSD, SIGNAL}, /*sigcatch*/
{"class",     "CLS",     pr_nop,      sr_nop,     5,   0,    XXX, RIGHT},
{"cls",       "-",       pr_nop,      sr_nop,     1,   0,    HPU, RIGHT},
{"cmaj_flt",  "-",       pr_nop,      sr_cmaj_flt, 1,  0,    LNX, RIGHT},
{"cmd",       "CMD",     pr_args,     sr_cmd,    16,   0,    DEC, UNLIMITED}, /*ucomm*/
{"cmin_flt",  "-",       pr_nop,      sr_cmin_flt, 1,  0,    LNX, RIGHT},
{"cnswap",    "-",       pr_nop,      sr_cnswap,  1,   0,    LNX, RIGHT},
{"comm",      "COMMAND", pr_comm,     sr_nop,    16,   0,    U98, UNLIMITED}, /*ucomm*/
{"command",   "COMMAND", pr_args,     sr_nop,    16,   0,    XXX, UNLIMITED}, /*args*/
{"cp",        "CP",      pr_nop,      sr_nop,     2,   0,    DEC, RIGHT}, /*cpu*/
{"cpu",       "CPU",     pr_nop,      sr_nop,     3,   0,    BSD, RIGHT}, /* FIXME ... HP-UX wants this as the CPU number for SMP? */
{"cputime",   "TIME",    pr_time,     sr_nop,     8,   0,    DEC, RIGHT}, /*time*/
{"cstime",    "-",       pr_nop,      sr_cstime,  1,   0,    LNX, RIGHT},
{"cursig",    "CURSIG",  pr_nop,      sr_nop,     6,   0,    DEC, RIGHT},
{"cutime",    "-",       pr_nop,      sr_cutime,  1,   0,    LNX, RIGHT},
{"cwd",       "CWD",     pr_nop,      sr_nop,     3,   0,    LNX, LEFT},
{"drs",       "DRS",     pr_drs,      sr_drs,     4,   0,    LNX, RIGHT},
{"dsiz",      "DSIZ",    pr_dsiz,     sr_nop,     4,   0,    LNX, RIGHT},
{"egid",      "EGID",    pr_egid,     sr_egid,    5,   0,    LNX, RIGHT},
{"egroup",    "EGROUP",  pr_egroup,   sr_egroup,  8,   0,    LNX, USER},
{"eip",       "EIP",     pr_eip,      sr_kstk_eip, 8,  0,    LNX, RIGHT},
{"end_code",  "E_CODE",  pr_nop,      sr_end_code, 8,  0,    LNx, RIGHT},
{"environ","ENVIRONMENT",pr_nop,      sr_nop,    11,   0,    LNx, UNLIMITED},
{"esp",       "ESP",     pr_esp,      sr_kstk_esp, 8,  0,    LNX, RIGHT},
{"etime",     "ELAPSED", pr_etime,    sr_nop,    11,   0,    U98, RIGHT}, /* was 7 wide */
{"euid",      "EUID",    pr_euid,     sr_euid,    5,   0,    LNX, RIGHT},
{"euser",     "EUSER",   pr_euser,    sr_euser,   8,   0,    LNX, USER},
{"f",         "F",       pr_flag,     sr_nop,     3,   0,    XXX, RIGHT}, /*flags*/
{"fgid",      "FGID",    pr_fgid,     sr_fgid,    5,   0,    LNX, RIGHT},
{"fgroup",    "FGROUP",  pr_fgroup,   sr_fgroup,  8,   0,    LNX, USER},
{"flag",      "F",       pr_flag,     sr_flags,   3,   0,    DEC, RIGHT},
{"flags",     "F",       pr_flag,     sr_flags,   3,   0,    BSD, RIGHT}, /*f*/ /* was FLAGS, 8 wide */
{"fname",     "COMMAND", pr_fname,    sr_nop,     8,   0,    SUN, LEFT},
{"fsgid",     "FSGID",   pr_fgid,     sr_fgid,    5,   0,    LNX, RIGHT},
{"fsgroup",   "FSGROUP", pr_fgroup,   sr_fgroup,  8,   0,    LNX, USER},
{"fsuid",     "FSUID",   pr_fuid,     sr_fuid,    5,   0,    LNX, RIGHT},
{"fsuser",    "FSUSER",  pr_fuser,    sr_fuser,   8,   0,    LNX, USER},
{"fuid",      "FUID",    pr_fuid,     sr_fuid,    5,   0,    LNX, RIGHT},
{"fuser",     "FUSER",   pr_fuser,    sr_fuser,   8,   0,    LNX, USER},
{"gid",       "GID",     pr_egid,     sr_egid,    5,   0,    SUN, RIGHT},
{"group",     "GROUP",   pr_egroup,   sr_egroup,  5,   0,    U98, USER}, /* was 8 wide */
{"ignored",   "IGNORED", pr_sigignore,sr_nop,     9,   0,    BSD, SIGNAL}, /*sigignore*/
{"inblk",     "INBLK",   pr_nop,      sr_nop,     5,   0,    BSD, RIGHT}, /*inblock*/
{"inblock",   "INBLK",   pr_nop,      sr_nop,     5,   0,    DEC, RIGHT}, /*inblk*/
{"intpri",    "PRI",     pr_opri,     sr_priority, 3,  0,    HPU, RIGHT},
{"jobc",      "JOBC",    pr_nop,      sr_nop,     4,   0,    XXX, RIGHT},
{"ktrace",    "KTRACE",  pr_nop,      sr_nop,     8,   0,    BSD, RIGHT},
{"ktracep",   "KTRACEP", pr_nop,      sr_nop,     8,   0,    BSD, RIGHT},
{"label",     "LABEL",   pr_nop,      sr_nop,    25,  0,     SGI, LEFT},
{"lim",       "LIM",     pr_lim,      sr_rss_rlim, 5,  0,    BSD, RIGHT},
{"login",     "LOGNAME", pr_nop,      sr_nop,     8,   0,    BSD, LEFT}, /*logname*/   /* double check */
{"logname",   "LOGNAME", pr_nop,      sr_nop,     8,   0,    XXX, LEFT}, /*login*/
{"longtname", "TTY",     pr_tty8,     sr_tty,     8,   0,    DEC, LEFT},
{"lstart",    "STARTED", pr_lstart,   sr_nop,    24,   0,    XXX, RIGHT},
{"luid",      "LUID",    pr_nop,      sr_nop,     5,   0,    LNX, RIGHT}, /* login ID */
{"luser",     "LUSER",   pr_nop,      sr_nop,     8,   0,    LNX, USER}, /* login USER */
{"m_drs",     "DRS",     pr_drs,      sr_drs,     5,   0,    LNx, RIGHT},
{"m_dt",      "DT",      pr_nop,      sr_dt,      4,   0,    LNx, RIGHT},
{"m_lrs",     "LRS",     pr_nop,      sr_lrs,     5,   0,    LNx, RIGHT},
{"m_resident", "RES",    pr_nop,      sr_resident, 5,  0,    LNx, RIGHT},
{"m_share",   "SHRD",    pr_nop,      sr_share,   5,   0,    LNx, RIGHT},
{"m_size",    "SIZE",    pr_nop,      sr_size,    5,   0,    LNx, RIGHT},
{"m_swap",    "SWAP",    pr_nop,      sr_nop,     5,   0,    LNx, RIGHT},
{"m_trs",     "TRS",     pr_trs,      sr_trs,     5,   0,    LNx, RIGHT},
{"maj_flt",   "MAJFL",   pr_majflt,   sr_maj_flt, 6,   0,    LNX, CUMUL|RIGHT},
{"majflt",    "MAJFLT",  pr_majflt,   sr_maj_flt, 6,   0,    XXX, RIGHT},
{"min_flt",   "MINFL",   pr_minflt,   sr_min_flt, 6,   0,    LNX, CUMUL|RIGHT},
{"minflt",    "MINFLT",  pr_minflt,   sr_min_flt, 6,   0,    XXX, RIGHT},
{"msgrcv",    "MSGRCV",  pr_nop,      sr_nop,     6,   0,    XXX, RIGHT},
{"msgsnd",    "MSGSND",  pr_nop,      sr_nop,     6,   0,    XXX, RIGHT},
{"ni",        "NI",      pr_nice,     sr_nice,    3,   0,    BSD, RIGHT}, /*nice*/
{"nice",      "NI",      pr_nice,     sr_nice,    3,   0,    U98, RIGHT}, /*ni*/
{"nivcsw",    "IVCSW",   pr_nop,      sr_nop,     5,   0,    XXX, RIGHT},
{"nsignals",  "NSIGS",   pr_nop,      sr_nop,     5,   0,    DEC, RIGHT}, /*nsigs*/
{"nsigs",     "NSIGS",   pr_nop,      sr_nop,     5,   0,    BSD, RIGHT}, /*nsignals*/
{"nswap",     "NSWAP",   pr_nop,      sr_nswap,   5,   0,    XXX, RIGHT},
{"nvcsw",     "VCSW",    pr_nop,      sr_nop,     5,   0,    XXX, RIGHT},
{"nwchan",    "WCHAN",   pr_nwchan,   sr_nop,     6,   0,    XXX, RIGHT},
{"opri",      "PRI",     pr_opri,     sr_priority, 3,  0,    SUN, RIGHT},
{"osz",       "SZ",      pr_nop,      sr_nop,     2,   0,    SUN, RIGHT},
{"oublk",     "OUBLK",   pr_nop,      sr_nop,     5,   0,    BSD, RIGHT}, /*oublock*/
{"oublock",   "OUBLK",   pr_nop,      sr_nop,     5,   0,    DEC, RIGHT}, /*oublk*/
{"p_ru",      "P_RU",    pr_nop,      sr_nop,     6,   0,    BSD, RIGHT},
{"paddr",     "PADDR",   pr_nop,      sr_nop,     6,   0,    BSD, RIGHT},
{"pagein",    "PAGEIN",  pr_majflt,   sr_nop,     6,   0,    XXX, RIGHT},
{"pcpu",      "%CPU",    pr_pcpu,     sr_pcpu,    4,   0,    U98, RIGHT}, /*%cpu*/
{"pending",   "PENDING", pr_sig,      sr_nop,     9,   0,    BSD, SIGNAL}, /*sig*/
{"pgid",      "PGID",    pr_pgid,     sr_pgrp,    5,   0,    U98, RIGHT},
{"pgrp",      "PGRP",    pr_pgid,     sr_pgrp,    5,   0,    LNX, RIGHT},
{"pid",       "PID",     pr_pid,      sr_pid,     5,   0,    U98, RIGHT},
{"pmem",      "%MEM",    pr_pmem,     sr_nop,     4,   0,    XXX, RIGHT}, /*%mem*/
{"poip",      "-",       pr_nop,      sr_nop,     1,   0,    BSD, RIGHT},
{"policy",    "POL",     pr_nop,      sr_nop,     3,   0,    DEC, RIGHT},
{"ppid",      "PPID",    pr_ppid,     sr_ppid,    5,   0,    U98, RIGHT},
{"pri",       "PRI",     pr_pri,      sr_nop,     3,   0,    XXX, RIGHT},
{"priority",  "PRI",     pr_priority, sr_priority, 3,  0,    LNX, RIGHT}, /*ni,nice*/ /* from Linux sorting names */
{"prmgrp",    "-",       pr_nop,      sr_nop,     1,   0,    HPU, RIGHT},
{"prmid",     "-",       pr_nop,      sr_nop,     1,   0,    HPU, RIGHT},
{"pset",      "PSET",    pr_nop,      sr_nop,     4,   0,    DEC, RIGHT},
{"psr",       "PSR",     pr_nop,      sr_nop,     3,   0,    DEC, RIGHT},
{"psxpri",    "PPR",     pr_nop,      sr_nop,     3,   0,    DEC, RIGHT},
{"re",        "RE",      pr_nop,      sr_nop,     3,   0,    BSD, RIGHT},
{"resident",  "-",       pr_nop,      sr_resident, 5,  0,    LNX, RIGHT},
{"rgid",      "RGID",    pr_rgid,     sr_rgid,    5,   0,    XXX, RIGHT},
{"rgroup",    "RGROUP",  pr_rgroup,   sr_rgroup,  6,   0,    U98, USER}, /* was 8 wide */
{"rlink",     "RLINK",   pr_nop,      sr_nop,     8,   0,    BSD, RIGHT},
{"rss",       "RSS",     pr_rss,      sr_rss,     4,   0,    XXX, RIGHT}, /* was 5 wide */
{"rssize",    "RSS",     pr_rss,      sr_vm_rss,  4,   0,    DEC, RIGHT}, /*rsz*/
{"rsz",       "RSZ",     pr_rss,      sr_vm_rss,  4,   0,    BSD, RIGHT}, /*rssize*/
{"rtprio",    "RTPRIO",  pr_nop,      sr_nop,     7,   0,    BSD, RIGHT},
{"ruid",      "RUID",    pr_ruid,     sr_ruid,    5,   0,    XXX, RIGHT},
{"ruser",     "RUSER",   pr_ruser,    sr_ruser,   8,   0,    U98, USER},
{"s",         "S",       pr_s,        sr_state,   1,   0,    SUN, LEFT}, /*stat,state*/
{"sched",     "SCH",     pr_nop,      sr_nop,     1,   0,    AIX, RIGHT},
{"scnt",      "SCNT",    pr_nop,      sr_nop,     4,   0,    DEC, RIGHT},  /* man page misspelling of scount? */
{"scount",    "SC",      pr_nop,      sr_nop,     4,   0,    AIX, RIGHT},  /* scnt==scount, DEC claims both */
{"sess",      "SESS",    pr_sess,     sr_session, 5,   0,    XXX, RIGHT},
{"session",   "SESS",    pr_sess,     sr_session, 5,   0,    LNX, RIGHT},
{"sgi_p",     "P",       pr_sgi_p,    sr_nop,     1,   0,    LNX, RIGHT}, /* "cpu" number */
{"sgi_rss",   "RSS",     pr_rss,      sr_nop,     4,   0,    LNX, LEFT}, /* SZ:RSS */
{"sgid",      "SGID",    pr_sgid,     sr_sgid,    5,   0,    LNX, RIGHT},
{"sgroup",    "SGROUP",  pr_sgroup,   sr_sgroup,  8,   0,    LNX, USER},
{"share",     "-",       pr_nop,      sr_share,   1,   0,    LNX, RIGHT},
{"sid",       "SID",     pr_sess,     sr_session, 5,   0,    XXX, RIGHT}, /* Sun & HP */
{"sig",       "PENDING", pr_sig,      sr_nop,     9,   0,    XXX, SIGNAL}, /*pending*/
{"sig_block", "BLOCKED",  pr_sigmask, sr_nop,     9,   0,    LNX, SIGNAL},
{"sig_catch", "CATCHED", pr_sigcatch, sr_nop,     9,   0,    LNX, SIGNAL},
{"sig_ignore", "IGNORED",pr_sigignore, sr_nop,    9,   0,    LNX, SIGNAL},
{"sig_pend",  "SIGNAL",   pr_sig,     sr_nop,     9,   0,    LNX, SIGNAL},
{"sigcatch",  "CAUGHT",  pr_sigcatch, sr_nop,     9,   0,    XXX, SIGNAL}, /*caught*/
{"sigignore", "IGNORED", pr_sigignore,sr_nop,     9,   0,    XXX, SIGNAL}, /*ignored*/
{"sigmask",   "BLOCKED", pr_sigmask,  sr_nop,     9,   0,    XXX, SIGNAL}, /*blocked*/
{"size",      "-",       pr_nop,      sr_size,    1,   0,    SCO, RIGHT},
{"sl",        "SL",      pr_nop,      sr_nop,     3,   0,    XXX, RIGHT},
{"stackp",    "STACKP",  pr_stackp,   sr_nop,     8,   0,    LNX, RIGHT}, /*start_stack*/
{"start",     "STARTED", pr_start,    sr_nop,     8,   0,    XXX, RIGHT},
{"start_code", "S_CODE",  pr_nop,     sr_start_code, 8, 0,   LNx, RIGHT},
{"start_stack", "STACKP", pr_stackp,  sr_start_stack, 8, 0,  LNX, RIGHT}, /*stackp*/
{"start_time", "START",  pr_stime,    sr_start_time, 5, 0,   LNx, RIGHT},
{"stat",      "STAT",    pr_stat,     sr_state,   4,   0,    BSD, LEFT}, /*state,s*/
{"state",     "S",       pr_s,        sr_state,   1,   0,    XXX, LEFT}, /*stat,s*/ /* was STAT */
{"status",    "STATUS",  pr_nop,      sr_nop,     6,   0,    DEC, RIGHT},
{"stime",     "STIME",   pr_stime,    sr_stime,   5,   0,    XXX, /* CUMUL| */RIGHT}, /* was 6 wide */
{"suid",      "SUID",    pr_suid,     sr_suid,    5,   0,    LNx, RIGHT},
{"suser",     "SUSER",   pr_suser,    sr_suser,   8,   0,    LNx, USER},
{"svgid",     "SVGID",   pr_sgid,     sr_sgid,    5,   0,    XXX, RIGHT},
{"svgroup",   "SVGROUP", pr_sgroup,   sr_sgroup,  8,   0,    LNX, USER},
{"svuid",     "SVUID",   pr_suid,     sr_suid,    5,   0,    XXX, RIGHT},
{"svuser",    "SVUSER",  pr_suser,    sr_suser,   8,   0,    LNX, USER},
{"systime",   "SYSTEM",  pr_nop,      sr_nop,     6,   0,    DEC, RIGHT},
{"sz",        "SZ",      pr_sz,       sr_nop,     5,   0,    HPU, RIGHT},
{"tdev",      "TDEV",    pr_nop,      sr_nop,     4,   0,    XXX, RIGHT},
{"thcount",   "THCNT",   pr_nop,      sr_nop,     1,   0,    AIX, RIGHT},
{"tid",       "TID",     pr_nop,      sr_nop,     1,   0,    AIX, RIGHT},
{"time",      "TIME",    pr_time,     sr_nop,     8,   0,    U98, CUMUL|RIGHT}, /*cputime*/ /* was 6 wide */
{"timeout",   "TMOUT",   pr_timeout,  sr_timeout, 5,   0,    LNX, RIGHT},
{"tmout",     "TMOUT",   pr_timeout,  sr_timeout, 5,   0,    LNX, RIGHT},
{"tname",     "TTY",     pr_tty8,     sr_tty,     8,   0,    DEC, LEFT},
{"tpgid",     "TPGID",   pr_tpgid,    sr_tpgid,   5,   0,    XXX, RIGHT},
{"trs",       "TRS",     pr_trs,      sr_trs,     4,   0,    AIX, RIGHT},
{"trss",      "TRSS",    pr_trs,      sr_trs,     4,   0,    BSD, RIGHT}, /* 4.3BSD NET/2 */
{"tsess",     "TSESS",   pr_nop,      sr_nop,     5,   0,    BSD, RIGHT},
{"tsession",  "TSESS",   pr_nop,      sr_nop,     5,   0,    DEC, RIGHT},
{"tsiz",      "TSIZ",    pr_tsiz,     sr_nop,     4,   0,    BSD, RIGHT},
{"tt",        "TT",      pr_tty8,     sr_tty,     8,   0,    BSD, LEFT},
{"tty",       "TT",      pr_tty8,     sr_tty,     8,   0,    U98, LEFT}, /* Unix98 requires "TT" but has "TTY" too. :-( */  /* was 3 wide */
{"tty4",      "TTY",     pr_tty4,     sr_tty,     4,   0,    LNX, LEFT},
{"tty8",      "TTY",     pr_tty8,     sr_tty,     8,   0,    LNX, LEFT},
{"u_procp",   "UPROCP",  pr_nop,      sr_nop,     6,   0,    DEC, RIGHT},
{"ucmd",      "CMD",     pr_comm,     sr_cmd,    16,   0,    DEC, UNLIMITED}, /*ucomm*/
{"ucomm",     "COMMAND", pr_comm,     sr_nop,    16,   0,    XXX, UNLIMITED}, /*comm*/
{"uid",       "UID",     pr_euid,     sr_euid,    5,   0,    XXX, RIGHT},
{"uid_hack",  "UID",     pr_euser,    sr_nop,     8,   0,    XXX, USER},
{"umask",     "UMASK",   pr_nop,      sr_nop,     5,   0,    DEC, RIGHT},
{"uname",     "USER",    pr_euser,    sr_euser,   8,   0,    DEC, USER}, /* man page misspelling of user? */
{"upr",       "UPR",     pr_nop,      sr_nop,     3,   0,    BSD, RIGHT}, /*usrpri*/
{"uprocp",    "-",       pr_nop,      sr_nop,     1,   0,    BSD, RIGHT},
{"user",      "USER",    pr_euser,    sr_euser,   8,   0,    U98, USER}, /* BSD n forces this to UID */
{"usertime",  "USER",    pr_nop,      sr_nop,     4,   0,    DEC, RIGHT},
{"usrpri",    "UPR",     pr_nop,      sr_nop,     3,   0,    DEC, RIGHT}, /*upr*/
{"utime",     "UTIME",   pr_nop,      sr_utime,   6,   0,    LNx, CUMUL|RIGHT},
{"vm_data",   "DATA",    pr_nop,      sr_vm_data, 5,   0,    LNx, RIGHT},
{"vm_exe",    "EXE",     pr_nop,      sr_vm_exe,  5,   0,    LNx, RIGHT},
{"vm_lib",    "LIB",     pr_nop,      sr_vm_lib,  5,   0,    LNx, RIGHT},
{"vm_lock",   "LCK",     pr_nop,      sr_vm_lock, 3,   0,    LNx, RIGHT},
{"vm_stack",  "STACK",   pr_nop,      sr_vm_stack, 5,  0,    LNx, RIGHT},
{"vsize",     "VSZ",     pr_vsz,      sr_vsize,   5,   0,    DEC, RIGHT}, /*vsz*/
{"vsz",       "VSZ",     pr_vsz,      sr_vm_size, 5,   0,    U98, RIGHT}, /*vsize*/
{"wchan",     "WCHAN",   pr_wchan,    sr_wchan,   6,   0,    XXX, WCHAN}, /* BSD n forces this to nwchan */ /* was 10 wide */
{"xstat",     "XSTAT",   pr_nop,      sr_nop,     5,   0,    BSD, RIGHT},
{"~",         "-",       pr_nop,      sr_nop,     1,   0,    LNX, RIGHT}  /* NULL would ruin alphabetical order */
};

static const int format_array_count = sizeof(format_array)/sizeof(format_struct);


/****************************** Macro formats *******************************/
/* First X field may be NR, which is p->start_code>>26 printed with %2ld */
/* That seems useless though, and Debian already killed it. */
/* The ones marked "Digital" have the name defined, not just the data. */
const macro_struct macro_array[] = {
{"DFMT",     "pid,tname,state,cputime,cmd"},         /* Digital's default */
{"DefBSD",   "pid,tname,stat,bsdtime,args"},               /* Our BSD default */
{"DefSysV",  "pid,tname,time,cmd"},                     /* Our SysV default */
{"END_BSD",  "state,tname,cputime,comm"},                 /* trailer for O */
{"END_SYS5", "state,tname,time,command"},                 /* trailer for -O */
{"F5FMT",    "uname,pid,ppid,c,start,tname,time,cmd"},       /* Digital -f */

{"FB_",      "pid,tt,stat,time,command"},                          /* FreeBSD default */
{"FB_j",     "user,pid,ppid,pgid,sess,jobc,stat,tt,time,command"},     /* FreeBSD j */
{"FB_l",     "uid,pid,ppid,cpu,pri,nice,vsz,rss,wchan,stat,tt,time,command"},   /* FreeBSD l */
{"FB_u",     "user,pid,pcpu,pmem,vsz,rss,tt,stat,start,time,command"},     /* FreeBSD u */
{"FB_v",     "pid,stat,time,sl,re,pagein,vsz,rss,lim,tsiz,pcpu,pmem,command"},   /* FreeBSD v */

{"FD_",      "pid,tty,time,comm"},                                 /* Fictional Debian SysV default */
{"FD_f",     "user,pid,ppid,start_time,tty,time,comm"},                /* Fictional Debian -f */
{"FD_fj",    "user,pid,ppid,start_time,tty,time,pgid,sid,comm"},        /* Fictional Debian -jf */
{"FD_j",     "pid,tty,time,pgid,sid,comm"},                                  /* Fictional Debian -j */
{"FD_l",     "flags,state,uid,pid,ppid,priority,nice,vsz,wchan,tty,time,comm"},    /* Fictional Debian -l */
{"FD_lj",    "flags,state,uid,pid,ppid,priority,nice,vsz,wchan,tty,time,pgid,sid,comm"}, /* Fictional Debian -jl */

{"FL5FMT",   "f,state,uid,pid,ppid,pcpu,pri,nice,rss,wchan,start,time,command"},  /* Digital -fl */

{"HP_",      "pid,tty,time,comm"},  /* HP default */
{"HP_f",     "user,pid,ppid,cpu,stime,tty,time,args"},  /* HP -f */
{"HP_fl",    "flags,state,user,pid,ppid,cpu,intpri,nice,addr,sz,wchan,stime,tty,time,args"},  /* HP -fl */
{"HP_l",     "flags,state,uid,pid,ppid,cpu,intpri,nice,addr,sz,wchan,tty,time,comm"},  /* HP -l */

{"JFMT",     "user,pid,ppid,pgid,sess,jobc,state,tname,cputime,command"},   /* Digital j and -j */
{"L5FMT",    "f,state,uid,pid,ppid,c,pri,nice,addr,sz,wchan,tt,time,ucmd"},   /* Digital -l */
{"LFMT",     "uid,pid,ppid,cp,pri,nice,vsz,rss,wchan,state,tname,cputime,command"},   /* Digital l */

{"OL_X",     "pid,start_stack,esp,eip,timeout,alarm,stat,tname,bsdtime,args"},      /* Old i386 Linux X */
{"OL_j",     "ppid,pid,pgid,sid,tname,tpgid,stat,uid,bsdtime,args"},                   /* Old Linux j */
{"OL_l",     "flags,uid,pid,ppid,priority,nice,vsz,rss,wchan,stat,tname,bsdtime,args"},     /* Old Linux l */
{"OL_m",     "pid,tname,majflt,minflt,m_trs,m_drs,m_size,m_swap,rss,m_share,vm_lib,m_dt,args"}, /* Old Linux m */
{"OL_s",     "uid,pid,pending,sig_block,sig_ignore,caught,stat,tname,bsdtime,args"},  /* Old Linux s */
{"OL_u",     "user,pid,pcpu,pmem,vsz,rss,tname,stat,start_time,bsdtime,args"},       /* Old Linux u */
{"OL_v",     "pid,tname,stat,bsdtime,maj_flt,m_trs,m_drs,rss,pmem,args"},            /* Old Linux v */

{"RD_",      "pid,tname,state,bsdtime,comm"},                                       /* Real Debian default */
{"RD_f",     "uid,pid,ppid,start_time,tname,bsdtime,args"},                         /* Real Debian -f */
{"RD_fj",    "uid,pid,ppid,start_time,tname,bsdtime,pgid,sid,args"},                /* Real Debian -jf */
{"RD_j",     "pid,tname,state,bsdtime,pgid,sid,comm"},                               /* Real Debian -j */
{"RD_l",     "flags,state,uid,pid,ppid,priority,nice,wchan,tname,bsdtime,comm"},           /* Real Debian -l */
{"RD_lj",    "flags,state,uid,pid,ppid,priority,nice,wchan,tname,bsdtime,pgid,sid,comm"},  /* Real Debian -jl */

{"RUSAGE",   "minflt,majflt,nswap,inblock,oublock,msgsnd,msgrcv,nsigs,nvcsw,nivcsw"}, /* Digital -o "RUSAGE" */
{"SCHED",    "user,pcpu,pri,usrpri,nice,psxpri,psr,policy,pset"},                /* Digital -o "SCHED" */
{"SFMT",     "uid,pid,cursig,sig,sigmask,sigignore,sigcatch,stat,tname,command"},  /* Digital s */

{"Std_f",    "uid_hack,pid,ppid,c,stime,tname,time,cmd"},                     /* new -f */
{"Std_fl",   "f,s,uid_hack,pid,ppid,c,opri,ni,addr,sz,wchan,stime,tname,time,cmd"}, /* -fl */
{"Std_l",    "f,s,uid,pid,ppid,c,opri,ni,addr,sz,wchan,tname,time,ucmd"},  /* new -l */

{"THREAD",   "user,pcpu,pri,scnt,wchan,usertime,systime"},                /* Digital -o "THREAD" */
{"UFMT",     "uname,pid,pcpu,pmem,vsz,rss,tt,state,start,time,command"},   /* Digital u */
{"VFMT",     "pid,tt,state,time,sl,pagein,vsz,rss,pcpu,pmem,command"},   /* Digital v */
{"~", "~"} /* NULL would ruin alphabetical order */
};

const int macro_array_count = sizeof(macro_array)/sizeof(macro_struct);


/*************************** AIX formats ********************/
/* Convert AIX format codes to normal format specifiers. */
const aix_struct aix_array[] = {
{'C', "pcpu",   "%CPU"},
{'G', "group",  "GROUP"},
{'P', "ppid",   "PPID"},
{'U', "user",   "USER"},
{'a', "args",   "COMMAND"},
{'c', "comm",   "COMMAND"},
{'g', "rgroup", "RGROUP"},
{'n', "nice",   "NI"},
{'p', "pid",    "PID"},
{'r', "pgid",   "PGID"},
{'t', "etime",  "ELAPSED"},
{'u', "ruser",  "RUSER"},
{'x', "time",   "TIME"},
{'y', "tty",    "TTY"},
{'z', "vsz",    "VSZ"},
{'~', "~",      "~"} /* NULL would ruin alphabetical order */
};
const int aix_array_count = sizeof(aix_array)/sizeof(aix_struct);


/********************* sorting ***************************/
/* Convert short sorting codes to normal format specifiers. */
const shortsort_struct shortsort_array[] = {
{'C', "pcpu"       },
{'G', "tpgid"      },
{'J', "cstime"     },
/* {'K', "stime"      }, */  /* conflict, system vs. start time */
{'M', "maj_flt"    },
{'N', "cmaj_flt"   },
{'P', "ppid"       },
{'R', "resident"   },
{'S', "share"      },
{'T', "start_time" },
{'U', "uid"        }, /* euid */
{'c', "cmd"        },
{'f', "flags"      },
{'g', "pgrp"       },
{'j', "cutime"     },
{'k', "utime"      },
{'m', "min_flt"    },
{'n', "cmin_flt"   },
{'o', "session"    },
{'p', "pid"        },
{'r', "rss"        },
{'s', "size"       },
{'t', "tty"        },
{'u', "user"       },
{'v', "vsize"      },
{'y', "priority"   }, /* nice */
{'~', "~"          } /* NULL would ruin alphabetical order */
};
const int shortsort_array_count = sizeof(shortsort_array)/sizeof(shortsort_struct);


/*********** print format_array **********/
/* called by the parser in another file */
void print_format_specifiers(void){
  const format_struct *walk = format_array;
  while(*(walk->spec) != '~'){
    if(walk->pr != pr_nop) printf("%-12.12s %-8.8s\n", walk->spec, walk->head);
    walk++;
  }
}

/************ comparison functions for bsearch *************/

static int compare_format_structs(const void *a, const void *b){
  return strcmp(((format_struct*)a)->spec,((format_struct*)b)->spec);
}

static int compare_macro_structs(const void *a, const void *b){
  return strcmp(((macro_struct*)a)->spec,((macro_struct*)b)->spec);
}

/******** look up structs as needed by the sort & format parsers ******/

const shortsort_struct *search_shortsort_array(const int findme){
  const shortsort_struct *walk = shortsort_array;
  while(walk->desc != '~'){
    if(walk->desc == findme) return walk;
    walk++;
  }
  return NULL;
}

const aix_struct *search_aix_array(const int findme){
  const aix_struct *walk = aix_array;
  while(walk->desc != '~'){
    if(walk->desc == findme) return walk;
    walk++;
  }
  return NULL;
}

const format_struct *search_format_array(const char *findme){
  format_struct key;
  key.spec = findme;
  return bsearch(&key, format_array, format_array_count,
    sizeof(format_struct), compare_format_structs
  );
}

const macro_struct *search_macro_array(const char *findme){
  macro_struct key;
  key.spec = findme;
  return bsearch(&key, macro_array, macro_array_count,
    sizeof(macro_struct), compare_macro_structs
  );
}

static unsigned int active_cols;  /* some multiple of screen_cols */

/***** Last chance, avoid needless trunctuation. */
static void check_header_width(void){
  format_node *walk = format_list;
  unsigned int total = 0;
  int was_normal = 0;
  unsigned int i = 0;
  unsigned int sigs = 0;
  while(walk){
    switch((walk->flags) & JUST_MASK){
    default:
      total += walk->width;
      total += was_normal;
      was_normal = 1;
      break;
    case SIGNAL:
      sigs++;
      total += walk->width;
      total += was_normal;
      was_normal = 1;
      break;
    case UNLIMITED:  /* could chop this a bit */
      if(walk->next) total += walk->width;
      else total += 3; /* not strlen(walk->name) */
      total += was_normal;
      was_normal = 1;
      break;
    case 0:  /* AIX */
      total += walk->width;
      was_normal = 0;
      break;
    }
    walk = walk->next;
  }
  for(;;){
    i++;
    active_cols = screen_cols * i;
    if(active_cols>=total) break;
    if(screen_cols*i >= OUTBUF_SIZE/2) break; /* can't go over */
  }
  wide_signals = (total+sigs*7 <= active_cols);
  
#if 0
  printf("123456789-123456789-123456789-123456789-"
         "123456789-123456789-123456789-123456789\n");
  printf("need %d, using %d\n", total, active_cols);
#endif
}


/********** show one process (NULL proc prints header) **********/
void show_one_proc(proc_t* p){
  /* unknown: maybe set correct & actual to 1, remove +/- 1 below */
  int correct  = 0;  /* screen position we should be at */
  int actual   = 0;  /* screen position we are at */
  int camax    = 0;  /* MAX(correct,actual) */
  int amount   = 0;  /* amount of text that this data is */
  int leftpad  = 0;  /* amount of space this column _could_ need */
  int space    = 0;  /* amount of space we actually need to print */
  int dospace  = 0;  /* previous column determined that we need a space */
  int legit    = 0;  /* legitimately stolen extra space */
  format_node *fmt = format_list;
  static int did_stuff = 0;  /* have we ever printed anything? */

  if(-1==(long)p){    /* true only once, at the end */
    check_header_width();  /* temporary test code */
    if(did_stuff) return;
    /* have _never_ printed anything, but might need a header */
    if(!--lines_to_next_header){
      lines_to_next_header = header_gap;
      show_one_proc(NULL);
    }
    /* fprintf(stderr, "No processes available.\n"); */  /* legal? */
    exit(1);
  }
  if(p){   /* not header, maybe we should call ourselves for it */
    if(!--lines_to_next_header){
      lines_to_next_header = header_gap;
      show_one_proc(NULL);
    }
  }
  did_stuff = 1;
  pp = p;                 /* global, the proc_t struct */
  if(active_cols>(int)OUTBUF_SIZE) fprintf(stderr,"Fix bigness error.\n");

  /* print row start sequence */
  do{
    legit = 0;
    camax = (correct>actual) ? correct : actual;
    /* set width suggestion which might be ignored */
    if(fmt->next) max_rightward = fmt->width;
    else max_rightward = active_cols-camax;
    max_leftward  = fmt->width + actual - correct; /* TODO check this */
    /* prepare data and calculate leftpad */
    if(p && fmt->pr) amount = (*fmt->pr)();
    else amount = strlen(strcpy(outbuf, fmt->name)); /* AIX or headers */
    switch((fmt->flags) & JUST_MASK){
    case 0:  /* for AIX, assigned outside this file */
      leftpad = 0;
      break;
    case LEFT:          /* bad */
      leftpad = 0;
      break;
    case RIGHT:     /* OK */
      leftpad = fmt->width - amount;
      if(leftpad < 0) leftpad = 0;
      break;
    case SIGNAL:
      /* if the screen is wide enough, use full 16-character output */
      if(wide_signals){
        leftpad = 16 - amount;
        legit = 7;
      }else{
        leftpad =  9 - amount;
      }
      if(leftpad < 0) leftpad = 0;
      break;
    case USER:       /* bad */
      leftpad = fmt->width - amount;
      if(leftpad < 0) leftpad = 0;
      if(!user_is_number) leftpad = 0;
      break;
    case WCHAN:       /* bad */
      if(wchan_is_number){
        leftpad = fmt->width - amount;
        if(leftpad < 0) leftpad = 0;
        break;
      }else{
        if(fmt->next){
          outbuf[fmt->width] = '\0';  /* Must chop, more columns! */
        }else{
          int chopspot;  /* place to chop */
          int tmpspace;  /* need "space" before it is calculated below */
          tmpspace = correct - actual;
          if(tmpspace<1) tmpspace = dospace;
          chopspot = active_cols-actual-tmpspace;
          if(chopspot<1) chopspot=1;  /* oops, we (mostly) lose this column... */
          outbuf[chopspot] = '\0';    /* chop at screen/buffer limit */
        }
        leftpad = 0;
        break;
      }
    case UNLIMITED:
      if(fmt->next){
        outbuf[fmt->width] = '\0';  /* Must chop, more columns! */
      }else{
        int chopspot;  /* place to chop */
        int tmpspace;  /* need "space" before it is calculated below */
        tmpspace = correct - actual;
        if(tmpspace<1) tmpspace = dospace;
        chopspot = active_cols-actual-tmpspace;
        if(chopspot<1) chopspot=1;  /* oops, we (mostly) lose this column... */
        outbuf[chopspot] = '\0';    /* chop at screen/buffer limit */
      }
      leftpad = 0;
      break;
    default:
      fputs("bad alignment code\n", stderr);
      break;
    }
    /* At this point:
     *
     * amount     garbage
     * actual     from previous column
     * correct    from previous column
     * space      gets toasted by the next line of code
     */
    space = correct - actual + leftpad;
    if(space<1) space=dospace;
    if(space>SPACE_AMOUNT) space=SPACE_AMOUNT;
/* printf("[%d %d %d %d %d %d]", correct,actual,amount,leftpad,space,dospace); */
    /* print data, set x position stuff */
    actual += fprintf(stdout, "%s", outbuf-space);
    correct += fmt->width;
    correct += legit;        /* adjust for SIGNAL expansion */
    if(fmt->pr && fmt->next && fmt->next->pr){ /* neither is AIX filler */
       correct++;
       dospace=1;
     } else dospace = 0;              /* TODO need this? */
  } while(( fmt = fmt->next ));
  fputs("\n", stdout); /* print row end sequence */
}

#ifdef TESTING
static void sanity_check(void){
  format_struct *fs = format_array;
  while((fs->spec)[0] != '~'){
    if(strlen(fs->head) > fs->width) printf("%d %s\n",strlen(fs->head),fs->spec);
    fs++;
  }
}
#endif

void init_output(void){
  memset(whitespace, ' ', PAGE_SIZE);
#if 0
  mprotect(whitespace, PAGE_SIZE, PROT_READ); /* FIXME may fail if unaligned */
  mprotect(
    (void *)((unsigned long)(whitespace_and_outbuf-PAGE_SIZE) &~ (PAGE_SIZE-1)),
    PAGE_SIZE, PROT_NONE
  );
#endif
  seconds_since_boot = uptime(0,0);
  seconds_since_1970 = time(NULL);
  time_of_boot = seconds_since_1970 - seconds_since_boot;
  bytes_main_memory = read_total_main();
  switch(getpagesize()){
  case 65536: page_shift = 16; break;
  case 32768: page_shift = 15; break;
  case 16384: page_shift = 14; break;
  case  8192: page_shift = 13; break;
  default: fprintf(stderr, "Unknown page size! (assume 4096)\n");
  case  4096: page_shift = 12; break;
  case  2048: page_shift = 11; break;
  case  1024: page_shift = 10; break;
  }
  check_header_width();
}
