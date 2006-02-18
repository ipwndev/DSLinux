#define INCL_DOS
#define INCL_NOPM
#define INCL_DOSFILEMGR
#define INCL_DOSMEMMGR
#define INCL_DOSERRORS
/* These 3 are needed for compile if os2.h includes os2tk.h, not os2emx.h */
#define INCL_DOSPROCESS
#define SPU_DISABLESUPPRESSION          0
#define SPU_ENABLESUPPRESSION           1
#include <os2.h>
#include "dlfcn.h"

#include <sys/uflags.h>

/*
 * Various Unix compatibility functions for OS/2
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <process.h>
#include <fcntl.h>

#include "EXTERN.h"
#include "perl.h"

#ifdef USE_THREADS

typedef void (*emx_startroutine)(void *);
typedef void* (*pthreads_startroutine)(void *);

enum pthreads_state {
    pthreads_st_none = 0, 
    pthreads_st_run,
    pthreads_st_exited, 
    pthreads_st_detached, 
    pthreads_st_waited,
};
const char *pthreads_states[] = {
    "uninit",
    "running",
    "exited",
    "detached",
    "waited for",
};

typedef struct {
    void *status;
    perl_cond cond;
    enum pthreads_state state;
} thread_join_t;

thread_join_t *thread_join_data;
int thread_join_count;
perl_mutex start_thread_mutex;

int
pthread_join(perl_os_thread tid, void **status)
{
    MUTEX_LOCK(&start_thread_mutex);
    switch (thread_join_data[tid].state) {
    case pthreads_st_exited:
	thread_join_data[tid].state = pthreads_st_none;	/* Ready to reuse */
	MUTEX_UNLOCK(&start_thread_mutex);
	*status = thread_join_data[tid].status;
	break;
    case pthreads_st_waited:
	MUTEX_UNLOCK(&start_thread_mutex);
	Perl_croak_nocontext("join with a thread with a waiter");
	break;
    case pthreads_st_run:
	thread_join_data[tid].state = pthreads_st_waited;
	COND_INIT(&thread_join_data[tid].cond);
	MUTEX_UNLOCK(&start_thread_mutex);
	COND_WAIT(&thread_join_data[tid].cond, NULL);    
	COND_DESTROY(&thread_join_data[tid].cond);
	thread_join_data[tid].state = pthreads_st_none;	/* Ready to reuse */
	*status = thread_join_data[tid].status;
	break;
    default:
	MUTEX_UNLOCK(&start_thread_mutex);
	Perl_croak_nocontext("join: unknown thread state: '%s'", 
	      pthreads_states[thread_join_data[tid].state]);
	break;
    }
    return 0;
}

void
pthread_startit(void *arg)
{
    /* Thread is already started, we need to transfer control only */
    pthreads_startroutine start_routine = *((pthreads_startroutine*)arg);
    int tid = pthread_self();
    void *retval;
    
    arg = ((void**)arg)[1];
    if (tid >= thread_join_count) {
	int oc = thread_join_count;
	
	thread_join_count = tid + 5 + tid/5;
	if (thread_join_data) {
	    Renew(thread_join_data, thread_join_count, thread_join_t);
	    Zero(thread_join_data + oc, thread_join_count - oc, thread_join_t);
	} else {
	    Newz(1323, thread_join_data, thread_join_count, thread_join_t);
	}
    }
    if (thread_join_data[tid].state != pthreads_st_none)
	Perl_croak_nocontext("attempt to reuse thread id %i", tid);
    thread_join_data[tid].state = pthreads_st_run;
    /* Now that we copied/updated the guys, we may release the caller... */
    MUTEX_UNLOCK(&start_thread_mutex);
    thread_join_data[tid].status = (*start_routine)(arg);
    switch (thread_join_data[tid].state) {
    case pthreads_st_waited:
	COND_SIGNAL(&thread_join_data[tid].cond);    
	break;
    default:
	thread_join_data[tid].state = pthreads_st_exited;
	break;
    }
}

int
pthread_create(perl_os_thread *tid, const pthread_attr_t *attr, 
	       void *(*start_routine)(void*), void *arg)
{
    void *args[2];

    args[0] = (void*)start_routine;
    args[1] = arg;

    MUTEX_LOCK(&start_thread_mutex);
    *tid = _beginthread(pthread_startit, /*stack*/ NULL, 
			/*stacksize*/ 10*1024*1024, (void*)args);
    MUTEX_LOCK(&start_thread_mutex);
    MUTEX_UNLOCK(&start_thread_mutex);
    return *tid ? 0 : EINVAL;
}

int 
pthread_detach(perl_os_thread tid)
{
    MUTEX_LOCK(&start_thread_mutex);
    switch (thread_join_data[tid].state) {
    case pthreads_st_waited:
	MUTEX_UNLOCK(&start_thread_mutex);
	Perl_croak_nocontext("detach on a thread with a waiter");
	break;
    case pthreads_st_run:
	thread_join_data[tid].state = pthreads_st_detached;
	MUTEX_UNLOCK(&start_thread_mutex);
	break;
    default:
	MUTEX_UNLOCK(&start_thread_mutex);
	Perl_croak_nocontext("detach: unknown thread state: '%s'", 
	      pthreads_states[thread_join_data[tid].state]);
	break;
    }
    return 0;
}

/* This is a very bastardized version: */
int
os2_cond_wait(perl_cond *c, perl_mutex *m)
{						
    int rc;
    STRLEN n_a;
    if ((rc = DosResetEventSem(*c,&n_a)) && (rc != ERROR_ALREADY_RESET))
	Perl_croak_nocontext("panic: COND_WAIT-reset: rc=%i", rc);		
    if (m) MUTEX_UNLOCK(m);					
    if (CheckOSError(DosWaitEventSem(*c,SEM_INDEFINITE_WAIT))
	&& (rc != ERROR_INTERRUPT))
	Perl_croak_nocontext("panic: COND_WAIT: rc=%i", rc);		
    if (rc == ERROR_INTERRUPT)
	errno = EINTR;
    if (m) MUTEX_LOCK(m);					
} 
#endif 

/*****************************************************************************/
/* 2.1 would not resolve symbols on demand, and has no ExtLIBPATH. */
static PFN ExtFCN[2];			/* Labeled by ord below. */
static USHORT loadOrd[2] = { 874, 873 }; /* Query=874, Set=873. */
#define ORD_QUERY_ELP	0
#define ORD_SET_ELP	1
struct PMWIN_entries_t PMWIN_entries;

HMODULE
loadModule(char *modname)
{
    HMODULE h = (HMODULE)dlopen(modname, 0);
    if (!h)
	Perl_croak_nocontext("Error loading module '%s': %s", 
			     modname, dlerror());
    return h;
}

APIRET
loadByOrd(char *modname, ULONG ord)
{
    if (ExtFCN[ord] == NULL) {
	static HMODULE hdosc = 0;
	BYTE buf[20];
	PFN fcn;
	APIRET rc;

	
	if (!hdosc) {
	    hdosc = loadModule(modname);
	    if (CheckOSError(DosQueryProcAddr(hdosc, loadOrd[ord], NULL, &fcn)))
		Perl_croak_nocontext(
			"This version of OS/2 does not support %s.%i", 
			modname, loadOrd[ord]);	    
	}
	ExtFCN[ord] = fcn;
    } 
    if ((long)ExtFCN[ord] == -1) 
	Perl_croak_nocontext("panic queryaddr");
}

void 
init_PMWIN_entries(void)
{
    static HMODULE hpmwin = 0;
    static const int ords[] = {
	763,				/* Initialize */
	716,				/* CreateMsgQueue */
	726,				/* DestroyMsgQueue */
	918,				/* PeekMsg */
	915,				/* GetMsg */
	912,				/* DispatchMsg */
	753,				/* GetLastError */
	705,				/* CancelShutdown */
    };
    BYTE buf[20];
    int i = 0;
    unsigned long rc;

    if (hpmwin)
	return;

    hpmwin = loadModule("pmwin");
    while (i < sizeof(ords)/sizeof(int)) {
	if (CheckOSError(DosQueryProcAddr(hpmwin, ords[i], NULL, 
					  ((PFN*)&PMWIN_entries)+i)))
	    Perl_croak_nocontext("This version of OS/2 does not support pmwin.%d", ords[i]);
	i++;
    }
}


/* priorities */
static signed char priors[] = {0, 1, 3, 2}; /* Last two interchanged,
					       self inverse. */
#define QSS_INI_BUFFER 1024

PQTOPLEVEL
get_sysinfo(ULONG pid, ULONG flags)
{
    char *pbuffer;
    ULONG rc, buf_len = QSS_INI_BUFFER;

    New(1322, pbuffer, buf_len, char);
    /* QSS_PROCESS | QSS_MODULE | QSS_SEMAPHORES | QSS_SHARED */
    rc = QuerySysState(flags, pid, pbuffer, buf_len);
    while (rc == ERROR_BUFFER_OVERFLOW) {
	Renew(pbuffer, buf_len *= 2, char);
	rc = QuerySysState(flags, pid, pbuffer, buf_len);
    }
    if (rc) {
	FillOSError(rc);
	Safefree(pbuffer);
	return 0;
    }
    return (PQTOPLEVEL)pbuffer;
}

#define PRIO_ERR 0x1111

static ULONG
sys_prio(pid)
{
  ULONG prio;
  PQTOPLEVEL psi;

  psi = get_sysinfo(pid, QSS_PROCESS);
  if (!psi) {
      return PRIO_ERR;
  }
  if (pid != psi->procdata->pid) {
      Safefree(psi);
      Perl_croak_nocontext("panic: wrong pid in sysinfo");
  }
  prio = psi->procdata->threads->priority;
  Safefree(psi);
  return prio;
}

int 
setpriority(int which, int pid, int val)
{
  ULONG rc, prio;
  PQTOPLEVEL psi;

  prio = sys_prio(pid);

  if (!(_emx_env & 0x200)) return 0; /* Nop if not OS/2. */
  if (priors[(32 - val) >> 5] + 1 == (prio >> 8)) {
      /* Do not change class. */
      return CheckOSError(DosSetPriority((pid < 0) 
					 ? PRTYS_PROCESSTREE : PRTYS_PROCESS,
					 0, 
					 (32 - val) % 32 - (prio & 0xFF), 
					 abs(pid)))
      ? -1 : 0;
  } else /* if ((32 - val) % 32 == (prio & 0xFF)) */ {
      /* Documentation claims one can change both class and basevalue,
       * but I find it wrong. */
      /* Change class, but since delta == 0 denotes absolute 0, correct. */
      if (CheckOSError(DosSetPriority((pid < 0) 
				      ? PRTYS_PROCESSTREE : PRTYS_PROCESS,
				      priors[(32 - val) >> 5] + 1, 
				      0, 
				      abs(pid)))) 
	  return -1;
      if ( ((32 - val) % 32) == 0 ) return 0;
      return CheckOSError(DosSetPriority((pid < 0) 
					 ? PRTYS_PROCESSTREE : PRTYS_PROCESS,
					 0, 
					 (32 - val) % 32, 
					 abs(pid)))
	  ? -1 : 0;
  } 
/*   else return CheckOSError(DosSetPriority((pid < 0)  */
/* 					  ? PRTYS_PROCESSTREE : PRTYS_PROCESS, */
/* 					  priors[(32 - val) >> 5] + 1,  */
/* 					  (32 - val) % 32 - (prio & 0xFF),  */
/* 					  abs(pid))) */
/*       ? -1 : 0; */
}

int 
getpriority(int which /* ignored */, int pid)
{
  TIB *tib;
  PIB *pib;
  ULONG rc, ret;

  if (!(_emx_env & 0x200)) return 0; /* Nop if not OS/2. */
  /* DosGetInfoBlocks has old priority! */
/*   if (CheckOSError(DosGetInfoBlocks(&tib, &pib))) return -1; */
/*   if (pid != pib->pib_ulpid) { */
  ret = sys_prio(pid);
  if (ret == PRIO_ERR) {
      return -1;
  }
/*   } else */
/*       ret = tib->tib_ptib2->tib2_ulpri; */
  return (1 - priors[((ret >> 8) - 1)])*32 - (ret & 0xFF);
}

/*****************************************************************************/
/* spawn */

/* There is no big sense to make it thread-specific, since signals 
   are delivered to thread 1 only.  XXXX Maybe make it into an array? */
static int spawn_pid;
static int spawn_killed;

static Signal_t
spawn_sighandler(int sig)
{
    /* Some programs do not arrange for the keyboard signals to be
       delivered to them.  We need to deliver the signal manually. */
    /* We may get a signal only if 
       a) kid does not receive keyboard signal: deliver it;
       b) kid already died, and we get a signal.  We may only hope
          that the pid number was not reused.
     */
    
    if (spawn_killed) 
	sig = SIGKILL;			/* Try harder. */
    kill(spawn_pid, sig);
    spawn_killed = 1;
}

static int
result(pTHX_ int flag, int pid)
{
	int r, status;
	Signal_t (*ihand)();     /* place to save signal during system() */
	Signal_t (*qhand)();     /* place to save signal during system() */
#ifndef __EMX__
	RESULTCODES res;
	int rpid;
#endif

	if (pid < 0 || flag != 0)
		return pid;

#ifdef __EMX__
	spawn_pid = pid;
	spawn_killed = 0;
	ihand = rsignal(SIGINT, &spawn_sighandler);
	qhand = rsignal(SIGQUIT, &spawn_sighandler);
	do {
	    r = wait4pid(pid, &status, 0);
	} while (r == -1 && errno == EINTR);
	rsignal(SIGINT, ihand);
	rsignal(SIGQUIT, qhand);

	PL_statusvalue = (U16)status;
	if (r < 0)
		return -1;
	return status & 0xFFFF;
#else
	ihand = rsignal(SIGINT, SIG_IGN);
	r = DosWaitChild(DCWA_PROCESS, DCWW_WAIT, &res, &rpid, pid);
	rsignal(SIGINT, ihand);
	PL_statusvalue = res.codeResult << 8 | res.codeTerminate;
	if (r)
		return -1;
	return PL_statusvalue;
#endif
}

#define EXECF_SPAWN 0
#define EXECF_EXEC 1
#define EXECF_TRUEEXEC 2
#define EXECF_SPAWN_NOWAIT 3
#define EXECF_SPAWN_BYFLAG 4

/* const char* const ptypes[] = { "FS", "DOS", "VIO", "PM", "DETACH" }; */

static int
my_type()
{
    int rc;
    TIB *tib;
    PIB *pib;
    
    if (!(_emx_env & 0x200)) return 1; /* not OS/2. */
    if (CheckOSError(DosGetInfoBlocks(&tib, &pib))) 
	return -1; 
    
    return (pib->pib_ultype);
}

static ULONG
file_type(char *path)
{
    int rc;
    ULONG apptype;
    
    if (!(_emx_env & 0x200)) 
	Perl_croak_nocontext("file_type not implemented on DOS"); /* not OS/2. */
    if (CheckOSError(DosQueryAppType(path, &apptype))) {
	switch (rc) {
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	    return -1;
	case ERROR_ACCESS_DENIED:	/* Directory with this name found? */
	    return -3;
	default:			/* Found, but not an
					   executable, or some other
					   read error. */
	    return -2;
	}
    }    
    return apptype;
}

static ULONG os2_mytype;

/* Spawn/exec a program, revert to shell if needed. */
/* global PL_Argv[] contains arguments. */

int
do_spawn_ve(pTHX_ SV *really, U32 flag, U32 execf, char *inicmd, U32 addflag)
{
	int trueflag = flag;
	int rc, pass = 1;
	char *tmps;
	char buf[256], *s = 0, scrbuf[280];
	char *args[4];
	static char * fargs[4] 
	    = { "/bin/sh", "-c", "\"$@\"", "spawn-via-shell", };
	char **argsp = fargs;
	char nargs = 4;
	int force_shell;
 	int new_stderr = -1, nostderr = 0, fl_stderr;
	STRLEN n_a;
	
	if (flag == P_WAIT)
		flag = P_NOWAIT;

      retry:
	if (strEQ(PL_Argv[0],"/bin/sh")) 
	    PL_Argv[0] = PL_sh_path;

	if (PL_Argv[0][0] != '/' && PL_Argv[0][0] != '\\'
	    && !(PL_Argv[0][0] && PL_Argv[0][1] == ':' 
		 && (PL_Argv[0][2] == '/' || PL_Argv[0][2] != '\\'))
	    ) /* will spawnvp use PATH? */
	    TAINT_ENV();	/* testing IFS here is overkill, probably */
	/* We should check PERL_SH* and PERLLIB_* as well? */
	if (!really || !*(tmps = SvPV(really, n_a)))
	    tmps = PL_Argv[0];

      reread:
	force_shell = 0;
	if (_emx_env & 0x200) { /* OS/2. */ 
	    int type = file_type(tmps);
	  type_again:
	    if (type == -1) {		/* Not found */
		errno = ENOENT;
		rc = -1;
		goto do_script;
	    }
	    else if (type == -2) {		/* Not an EXE */
		errno = ENOEXEC;
		rc = -1;
		goto do_script;
	    }
	    else if (type == -3) {		/* Is a directory? */
		/* Special-case this */
		char tbuf[512];
		int l = strlen(tmps);

		if (l + 5 <= sizeof tbuf) {
		    strcpy(tbuf, tmps);
		    strcpy(tbuf + l, ".exe");
		    type = file_type(tbuf);
		    if (type >= -3)
			goto type_again;
		}
		
		errno = ENOEXEC;
		rc = -1;
		goto do_script;
	    }
	    switch (type & 7) {
		/* Ignore WINDOWCOMPAT and FAPI, start them the same type we are. */
	    case FAPPTYP_WINDOWAPI: 
	    {
		if (os2_mytype != 3) {	/* not PM */
		    if (flag == P_NOWAIT)
			flag = P_PM;
		    else if ((flag & 7) != P_PM && (flag & 7) != P_SESSION)
			Perl_warner(aTHX_ WARN_EXEC, "Starting PM process with flag=%d, mytype=%d",
			     flag, os2_mytype);
		}
	    }
	    break;
	    case FAPPTYP_NOTWINDOWCOMPAT: 
	    {
		if (os2_mytype != 0) {	/* not full screen */
		    if (flag == P_NOWAIT)
			flag = P_SESSION;
		    else if ((flag & 7) != P_SESSION)
			Perl_warner(aTHX_ WARN_EXEC, "Starting Full Screen process with flag=%d, mytype=%d",
			     flag, os2_mytype);
		}
	    }
	    break;
	    case FAPPTYP_NOTSPEC: 
		/* Let the shell handle this... */
		force_shell = 1;
		goto doshell_args;
		break;
	    }
	}

	if (addflag) {
	    addflag = 0;
	    new_stderr = dup(2);		/* Preserve stderr */
	    if (new_stderr == -1) {
		if (errno == EBADF)
		    nostderr = 1;
		else {
		    rc = -1;
		    goto finish;
		}
	    } else
		fl_stderr = fcntl(2, F_GETFD);
	    rc = dup2(1,2);
	    if (rc == -1)
		goto finish;
	    fcntl(new_stderr, F_SETFD, FD_CLOEXEC);
	}

#if 0
	rc = result(aTHX_ trueflag, spawnvp(flag,tmps,PL_Argv));
#else
	if (execf == EXECF_TRUEEXEC)
	    rc = execvp(tmps,PL_Argv);
	else if (execf == EXECF_EXEC)
	    rc = spawnvp(trueflag | P_OVERLAY,tmps,PL_Argv);
	else if (execf == EXECF_SPAWN_NOWAIT)
	    rc = spawnvp(flag,tmps,PL_Argv);
        else				/* EXECF_SPAWN, EXECF_SPAWN_BYFLAG */
	    rc = result(aTHX_ trueflag, 
			spawnvp(flag,tmps,PL_Argv));
#endif 
	if (rc < 0 && pass == 1
	    && (tmps == PL_Argv[0])) { /* Cannot transfer `really' via shell. */
	      do_script:
	    {
	    int err = errno;

	    if (err == ENOENT || err == ENOEXEC) {
		/* No such file, or is a script. */
		/* Try adding script extensions to the file name, and
		   search on PATH. */
		char *scr = find_script(PL_Argv[0], TRUE, NULL, 0);

		if (scr) {
		    FILE *file;
		    char *s = 0, *s1;
		    int l;

                    l = strlen(scr);
		
                    if (l >= sizeof scrbuf) {
                       Safefree(scr);
                     longbuf:
                       Perl_warner(aTHX_ WARN_EXEC, "Size of scriptname too big: %d", l);
		       rc = -1;
		       goto finish;
                    }
                    strcpy(scrbuf, scr);
                    Safefree(scr);
                    scr = scrbuf;

		    file = fopen(scr, "r");
		    PL_Argv[0] = scr;
		    if (!file)
			goto panic_file;
		    if (!fgets(buf, sizeof buf, file)) { /* Empty... */

			buf[0] = 0;
			fclose(file);
			/* Special case: maybe from -Zexe build, so
			   there is an executable around (contrary to
			   documentation, DosQueryAppType sometimes (?)
			   does not append ".exe", so we could have
			   reached this place). */
			if (l + 5 < sizeof scrbuf) {
			    strcpy(scrbuf + l, ".exe");
			    if (PerlLIO_stat(scrbuf,&PL_statbuf) >= 0
				&& !S_ISDIR(PL_statbuf.st_mode)) {
				/* Found */
				tmps = scr;
				pass++;
				goto reread;
			    } else
				scrbuf[l] = 0;
			} else
			    goto longbuf;
		    }
		    if (fclose(file) != 0) { /* Failure */
		      panic_file:
			Perl_warner(aTHX_ WARN_EXEC, "Error reading \"%s\": %s", 
			     scr, Strerror(errno));
			buf[0] = 0;	/* Not #! */
			goto doshell_args;
		    }
		    if (buf[0] == '#') {
			if (buf[1] == '!')
			    s = buf + 2;
		    } else if (buf[0] == 'e') {
			if (strnEQ(buf, "extproc", 7) 
			    && isSPACE(buf[7]))
			    s = buf + 8;
		    } else if (buf[0] == 'E') {
			if (strnEQ(buf, "EXTPROC", 7)
			    && isSPACE(buf[7]))
			    s = buf + 8;
		    }
		    if (!s) {
			buf[0] = 0;	/* Not #! */
			goto doshell_args;
		    }
		    
		    s1 = s;
		    nargs = 0;
		    argsp = args;
		    while (1) {
			/* Do better than pdksh: allow a few args,
			   strip trailing whitespace.  */
			while (isSPACE(*s))
			    s++;
			if (*s == 0) 
			    break;
			if (nargs == 4) {
			    nargs = -1;
			    break;
			}
			args[nargs++] = s;
			while (*s && !isSPACE(*s))
			    s++;
			if (*s == 0) 
			    break;
			*s++ = 0;
		    }
		    if (nargs == -1) {
			Perl_warner(aTHX_ WARN_EXEC, "Too many args on %.*s line of \"%s\"",
			     s1 - buf, buf, scr);
			nargs = 4;
			argsp = fargs;
		    }
		  doshell_args:
		    {
			char **a = PL_Argv;
			char *exec_args[2];

			if (force_shell 
			    || (!buf[0] && file)) { /* File without magic */
			    /* In fact we tried all what pdksh would
			       try.  There is no point in calling
			       pdksh, we may just emulate its logic. */
			    char *shell = getenv("EXECSHELL");
			    char *shell_opt = NULL;

			    if (!shell) {
				char *s;

				shell_opt = "/c";
				shell = getenv("OS2_SHELL");
				if (inicmd) { /* No spaces at start! */
				    s = inicmd;
				    while (*s && !isSPACE(*s)) {
					if (*s++ = '/') {
					    inicmd = NULL; /* Cannot use */
					    break;
					}
				    }
				}
				if (!inicmd) {
				    s = PL_Argv[0];
				    while (*s) { 
					/* Dosish shells will choke on slashes
					   in paths, fortunately, this is
					   important for zeroth arg only. */
					if (*s == '/') 
					    *s = '\\';
					s++;
				    }
				}
			    }
			    /* If EXECSHELL is set, we do not set */
			    
			    if (!shell)
				shell = ((_emx_env & 0x200)
					 ? "c:/os2/cmd.exe"
					 : "c:/command.com");
			    nargs = shell_opt ? 2 : 1;	/* shell file args */
			    exec_args[0] = shell;
			    exec_args[1] = shell_opt;
			    argsp = exec_args;
			    if (nargs == 2 && inicmd) {
				/* Use the original cmd line */
				/* XXXX This is good only until we refuse
				        quoted arguments... */
				PL_Argv[0] = inicmd;
				PL_Argv[1] = Nullch;
			    }
			} else if (!buf[0] && inicmd) { /* No file */
			    /* Start with the original cmdline. */
			    /* XXXX This is good only until we refuse
			            quoted arguments... */

			    PL_Argv[0] = inicmd;
			    PL_Argv[1] = Nullch;
			    nargs = 2;	/* shell -c */
			} 

			while (a[1])		/* Get to the end */
			    a++;
			a++;			/* Copy finil NULL too */
			while (a >= PL_Argv) {
			    *(a + nargs) = *a;	/* PL_Argv was preallocated to be
						   long enough. */
			    a--;
			}
			while (--nargs >= 0)
			    PL_Argv[nargs] = argsp[nargs];
			/* Enable pathless exec if #! (as pdksh). */
			pass = (buf[0] == '#' ? 2 : 3);
			goto retry;
		    }
		}
		/* Not found: restore errno */
		errno = err;
	    }
	  }
	} else if (rc < 0 && pass == 2 && errno == ENOENT) { /* File not found */
	    char *no_dir = strrchr(PL_Argv[0], '/');

	    /* Do as pdksh port does: if not found with /, try without
	       path. */
	    if (no_dir) {
		PL_Argv[0] = no_dir + 1;
		pass++;
		goto retry;
	    }
	}
	if (rc < 0 && ckWARN(WARN_EXEC))
	    Perl_warner(aTHX_ WARN_EXEC, "Can't %s \"%s\": %s\n", 
		 ((execf != EXECF_EXEC && execf != EXECF_TRUEEXEC) 
		  ? "spawn" : "exec"),
		 PL_Argv[0], Strerror(errno));
	if (rc < 0 && (execf != EXECF_SPAWN_NOWAIT) 
	    && ((trueflag & 0xFF) == P_WAIT)) 
	    rc = -1;

  finish:
    if (new_stderr != -1) {	/* How can we use error codes? */
	dup2(new_stderr, 2);
	close(new_stderr);
	fcntl(2, F_SETFD, fl_stderr);
    } else if (nostderr)
       close(2);
    return rc;
}

/* Try converting 1-arg form to (usually shell-less) multi-arg form. */
int
do_spawn3(pTHX_ char *cmd, int execf, int flag)
{
    register char **a;
    register char *s;
    char flags[10];
    char *shell, *copt, *news = NULL;
    int rc, err, seenspace = 0, mergestderr = 0;
    char fullcmd[MAXNAMLEN + 1];

#ifdef TRYSHELL
    if ((shell = getenv("EMXSHELL")) != NULL)
    	copt = "-c";
    else if ((shell = getenv("SHELL")) != NULL)
    	copt = "-c";
    else if ((shell = getenv("COMSPEC")) != NULL)
    	copt = "/C";
    else
    	shell = "cmd.exe";
#else
    /* Consensus on perl5-porters is that it is _very_ important to
       have a shell which will not change between computers with the
       same architecture, to avoid "action on a distance". 
       And to have simple build, this shell should be sh. */
    shell = PL_sh_path;
    copt = "-c";
#endif 

    while (*cmd && isSPACE(*cmd))
	cmd++;

    if (strnEQ(cmd,"/bin/sh",7) && isSPACE(cmd[7])) {
	STRLEN l = strlen(PL_sh_path);
	
	New(1302, news, strlen(cmd) - 7 + l + 1, char);
	strcpy(news, PL_sh_path);
	strcpy(news + l, cmd + 7);
	cmd = news;
    }

    /* save an extra exec if possible */
    /* see if there are shell metacharacters in it */

    if (*cmd == '.' && isSPACE(cmd[1]))
	goto doshell;

    if (strnEQ(cmd,"exec",4) && isSPACE(cmd[4]))
	goto doshell;

    for (s = cmd; *s && isALPHA(*s); s++) ;	/* catch VAR=val gizmo */
    if (*s == '=')
	goto doshell;

    for (s = cmd; *s; s++) {
	if (*s != ' ' && !isALPHA(*s) && strchr("$&*(){}[]'\";\\|?<>~`\n",*s)) {
	    if (*s == '\n' && s[1] == '\0') {
		*s = '\0';
		break;
	    } else if (*s == '\\' && !seenspace) {
		continue;		/* Allow backslashes in names */
	    } else if (*s == '>' && s >= cmd + 3
			&& s[-1] == '2' && s[1] == '&' && s[2] == '1'
			&& isSPACE(s[-2]) ) {
		char *t = s + 3;

		while (*t && isSPACE(*t))
		    t++;
		if (!*t) {
		    s[-2] = '\0';
		    mergestderr = 1;
		    break;		/* Allow 2>&1 as the last thing */
		}
	    }
	    /* We do not convert this to do_spawn_ve since shell
	       should be smart enough to start itself gloriously. */
	  doshell:
	    if (execf == EXECF_TRUEEXEC)
                rc = execl(shell,shell,copt,cmd,(char*)0);		
	    else if (execf == EXECF_EXEC)
                rc = spawnl(P_OVERLAY,shell,shell,copt,cmd,(char*)0);
	    else if (execf == EXECF_SPAWN_NOWAIT)
                rc = spawnl(P_NOWAIT,shell,shell,copt,cmd,(char*)0);
	    else if (execf == EXECF_SPAWN_BYFLAG)
                rc = spawnl(flag,shell,shell,copt,cmd,(char*)0);
	    else {
		/* In the ak code internal P_NOWAIT is P_WAIT ??? */
		rc = result(aTHX_ P_WAIT,
			    spawnl(P_NOWAIT,shell,shell,copt,cmd,(char*)0));
		if (rc < 0 && ckWARN(WARN_EXEC))
		    Perl_warner(aTHX_ WARN_EXEC, "Can't %s \"%s\": %s", 
			 (execf == EXECF_SPAWN ? "spawn" : "exec"),
			 shell, Strerror(errno));
		if (rc < 0)
		    rc = -1;
	    }
	    if (news)
		Safefree(news);
	    return rc;
	} else if (*s == ' ' || *s == '\t') {
	    seenspace = 1;
	}
    }

    /* cmd="a" may lead to "sh", "-c", "\"$@\"", "a", "a.cmd", NULL */
    New(1303,PL_Argv, (s - cmd + 11) / 2, char*);
    PL_Cmd = savepvn(cmd, s-cmd);
    a = PL_Argv;
    for (s = PL_Cmd; *s;) {
	while (*s && isSPACE(*s)) s++;
	if (*s)
	    *(a++) = s;
	while (*s && !isSPACE(*s)) s++;
	if (*s)
	    *s++ = '\0';
    }
    *a = Nullch;
    if (PL_Argv[0])
	rc = do_spawn_ve(aTHX_ NULL, flag, execf, cmd, mergestderr);
    else
    	rc = -1;
    if (news)
	Safefree(news);
    do_execfree();
    return rc;
}

/* Array spawn.  */
int
os2_do_aspawn(pTHX_ SV *really, register SV **mark, register SV **sp)
{
    register char **a;
    int rc;
    int flag = P_WAIT, flag_set = 0;
    STRLEN n_a;

    if (sp > mark) {
	New(1301,PL_Argv, sp - mark + 3, char*);
	a = PL_Argv;

	if (mark < sp && SvNIOKp(*(mark+1)) && !SvPOKp(*(mark+1))) {
		++mark;
		flag = SvIVx(*mark);
		flag_set = 1;

	}

	while (++mark <= sp) {
	    if (*mark)
		*a++ = SvPVx(*mark, n_a);
	    else
		*a++ = "";
	}
	*a = Nullch;

	if (flag_set && (a == PL_Argv + 1)) { /* One arg? */
	    rc = do_spawn3(aTHX_ a[-1], EXECF_SPAWN_BYFLAG, flag);
	} else
	    rc = do_spawn_ve(aTHX_ really, flag, EXECF_SPAWN, NULL, 0);
    } else
    	rc = -1;
    do_execfree();
    return rc;
}

int
os2_do_spawn(pTHX_ char *cmd)
{
    return do_spawn3(aTHX_ cmd, EXECF_SPAWN, 0);
}

int
do_spawn_nowait(pTHX_ char *cmd)
{
    return do_spawn3(aTHX_ cmd, EXECF_SPAWN_NOWAIT,0);
}

bool
Perl_do_exec(pTHX_ char *cmd)
{
    do_spawn3(aTHX_ cmd, EXECF_EXEC, 0);
    return FALSE;
}

bool
os2exec(pTHX_ char *cmd)
{
    return do_spawn3(aTHX_ cmd, EXECF_TRUEEXEC, 0);
}

PerlIO *
my_syspopen(pTHX_ char *cmd, char *mode)
{
#ifndef USE_POPEN

    int p[2];
    register I32 this, that, newfd;
    register I32 pid, rc;
    PerlIO *res;
    SV *sv;
    int fh_fl;
    
    /* `this' is what we use in the parent, `that' in the child. */
    this = (*mode == 'w');
    that = !this;
    if (PL_tainting) {
	taint_env();
	taint_proper("Insecure %s%s", "EXEC");
    }
    if (pipe(p) < 0)
	return Nullfp;
    /* Now we need to spawn the child. */
    if (p[this] == (*mode == 'r')) {	/* if fh 0/1 was initially closed. */
	int new = dup(p[this]);

	if (new == -1)
	    goto closepipes;
	close(p[this]);
	p[this] = new;
    }
    newfd = dup(*mode == 'r');		/* Preserve std* */
    if (newfd == -1) {		
	/* This cannot happen due to fh being bad after pipe(), since
	   pipe() should have created fh 0 and 1 even if they were
	   initially closed.  But we closed p[this] before.  */
	if (errno != EBADF) {
	  closepipes:
	    close(p[0]);
	    close(p[1]);
	    return Nullfp;
	}
    } else
	fh_fl = fcntl(*mode == 'r', F_GETFD);
    if (p[that] != (*mode == 'r')) {	/* if fh 0/1 was initially closed. */
	dup2(p[that], *mode == 'r');
	close(p[that]);
    }
    /* Where is `this' and newfd now? */
    fcntl(p[this], F_SETFD, FD_CLOEXEC);
    if (newfd != -1)
	fcntl(newfd, F_SETFD, FD_CLOEXEC);
    pid = do_spawn_nowait(aTHX_ cmd);
    if (newfd == -1)
	close(*mode == 'r');		/* It was closed initially */
    else if (newfd != (*mode == 'r')) {	/* Probably this check is not needed */
	dup2(newfd, *mode == 'r');	/* Return std* back. */
	close(newfd);
	fcntl(*mode == 'r', F_SETFD, fh_fl);
    } else
	fcntl(*mode == 'r', F_SETFD, fh_fl);
    if (p[that] == (*mode == 'r'))
	close(p[that]);
    if (pid == -1) {
	close(p[this]);
	return Nullfp;
    }
    if (p[that] < p[this]) {		/* Make fh as small as possible */
	dup2(p[this], p[that]);
	close(p[this]);
	p[this] = p[that];
    }
    sv = *av_fetch(PL_fdpid,p[this],TRUE);
    (void)SvUPGRADE(sv,SVt_IV);
    SvIVX(sv) = pid;
    PL_forkprocess = pid;
    return PerlIO_fdopen(p[this], mode);

#else  /* USE_POPEN */

    PerlIO *res;
    SV *sv;

#  ifdef TRYSHELL
    res = popen(cmd, mode);
#  else
    char *shell = getenv("EMXSHELL");

    my_setenv("EMXSHELL", PL_sh_path);
    res = popen(cmd, mode);
    my_setenv("EMXSHELL", shell);
#  endif 
    sv = *av_fetch(PL_fdpid, PerlIO_fileno(res), TRUE);
    (void)SvUPGRADE(sv,SVt_IV);
    SvIVX(sv) = -1;			/* A cooky. */
    return res;

#endif /* USE_POPEN */

}

/******************************************************************/

#ifndef HAS_FORK
int
fork(void)
{
    Perl_croak_nocontext(PL_no_func, "Unsupported function fork");
    errno = EINVAL;
    return -1;
}
#endif

/*******************************************************************/
/* not implemented in EMX 0.9d */

char *	ctermid(char *s)	{ return 0; }

#ifdef MYTTYNAME /* was not in emx0.9a */
void *	ttyname(x)	{ return 0; }
#endif

/******************************************************************/
/* my socket forwarders - EMX lib only provides static forwarders */

static HMODULE htcp = 0;

static void *
tcp0(char *name)
{
    PFN fcn;

    if (!(_emx_env & 0x200)) Perl_croak_nocontext("%s requires OS/2", name); /* Die if not OS/2. */
    if (!htcp)
	htcp = loadModule("tcp32dll");
    if (htcp && DosQueryProcAddr(htcp, 0, name, &fcn) == 0)
	return (void *) ((void * (*)(void)) fcn) ();
    return 0;
}

static void
tcp1(char *name, int arg)
{
    static BYTE buf[20];
    PFN fcn;

    if (!(_emx_env & 0x200)) Perl_croak_nocontext("%s requires OS/2", name); /* Die if not OS/2. */
    if (!htcp)
	DosLoadModule(buf, sizeof buf, "tcp32dll", &htcp);
    if (htcp && DosQueryProcAddr(htcp, 0, name, &fcn) == 0)
	((void (*)(int)) fcn) (arg);
}

struct hostent *	gethostent()	{ return tcp0("GETHOSTENT");  }
struct netent *		getnetent()	{ return tcp0("GETNETENT");   }
struct protoent *	getprotoent()	{ return tcp0("GETPROTOENT"); }
struct servent *	getservent()	{ return tcp0("GETSERVENT");  }

void	sethostent(x)	{ tcp1("SETHOSTENT",  x); }
void	setnetent(x)	{ tcp1("SETNETENT",   x); }
void	setprotoent(x)	{ tcp1("SETPROTOENT", x); }
void	setservent(x)	{ tcp1("SETSERVENT",  x); }
void	endhostent()	{ tcp0("ENDHOSTENT");  }
void	endnetent()	{ tcp0("ENDNETENT");   }
void	endprotoent()	{ tcp0("ENDPROTOENT"); }
void	endservent()	{ tcp0("ENDSERVENT");  }

/*****************************************************************************/
/* not implemented in C Set++ */

#ifndef __EMX__
int	setuid(x)	{ errno = EINVAL; return -1; }
int	setgid(x)	{ errno = EINVAL; return -1; }
#endif

/*****************************************************************************/
/* stat() hack for char/block device */

#if OS2_STAT_HACK

    /* First attempt used DosQueryFSAttach which crashed the system when
       used with 5.001. Now just look for /dev/. */

int
os2_stat(char *name, struct stat *st)
{
    static int ino = SHRT_MAX;

    if (stricmp(name, "/dev/con") != 0
     && stricmp(name, "/dev/tty") != 0)
	return stat(name, st);

    memset(st, 0, sizeof *st);
    st->st_mode = S_IFCHR|0666;
    st->st_ino = (ino-- & 0x7FFF);
    st->st_nlink = 1;
    return 0;
}

#endif

#ifdef USE_PERL_SBRK

/* SBRK() emulation, mostly moved to malloc.c. */

void *
sys_alloc(int size) {
    void *got;
    APIRET rc = DosAllocMem(&got, size, PAG_COMMIT | PAG_WRITE);

    if (rc == ERROR_NOT_ENOUGH_MEMORY) {
	return (void *) -1;
    } else if ( rc ) 
	Perl_croak_nocontext("Got an error from DosAllocMem: %li", (long)rc);
    return got;
}

#endif /* USE_PERL_SBRK */

/* tmp path */

char *tmppath = TMPPATH1;

void
settmppath()
{
    char *p = getenv("TMP"), *tpath;
    int len;

    if (!p) p = getenv("TEMP");
    if (!p) return;
    len = strlen(p);
    tpath = (char *)malloc(len + strlen(TMPPATH1) + 2);
    if (tpath) {
	strcpy(tpath, p);
	tpath[len] = '/';
	strcpy(tpath + len + 1, TMPPATH1);
	tmppath = tpath;
    }
}

#include "XSUB.h"

XS(XS_File__Copy_syscopy)
{
    dXSARGS;
    if (items < 2 || items > 3)
	Perl_croak_nocontext("Usage: File::Copy::syscopy(src,dst,flag=0)");
    {
	STRLEN n_a;
	char *	src = (char *)SvPV(ST(0),n_a);
	char *	dst = (char *)SvPV(ST(1),n_a);
	U32	flag;
	int	RETVAL, rc;

	if (items < 3)
	    flag = 0;
	else {
	    flag = (unsigned long)SvIV(ST(2));
	}

	RETVAL = !CheckOSError(DosCopy(src, dst, flag));
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}

#include "patchlevel.h"

char *
mod2fname(pTHX_ SV *sv)
{
    static char fname[9];
    int pos = 6, len, avlen;
    unsigned int sum = 0;
    AV  *av;
    SV  *svp;
    char *s;
    STRLEN n_a;

    if (!SvROK(sv)) Perl_croak_nocontext("Not a reference given to mod2fname");
    sv = SvRV(sv);
    if (SvTYPE(sv) != SVt_PVAV) 
      Perl_croak_nocontext("Not array reference given to mod2fname");

    avlen = av_len((AV*)sv);
    if (avlen < 0) 
      Perl_croak_nocontext("Empty array reference given to mod2fname");

    s = SvPV(*av_fetch((AV*)sv, avlen, FALSE), n_a);
    strncpy(fname, s, 8);
    len = strlen(s);
    if (len < 6) pos = len;
    while (*s) {
	sum = 33 * sum + *(s++);	/* Checksumming first chars to
					 * get the capitalization into c.s. */
    }
    avlen --;
    while (avlen >= 0) {
	s = SvPV(*av_fetch((AV*)sv, avlen, FALSE), n_a);
	while (*s) {
	    sum = 33 * sum + *(s++);	/* 7 is primitive mod 13. */
	}
	avlen --;
    }
#ifdef USE_THREADS
    sum++;				/* Avoid conflict of DLLs in memory. */
#endif 
    sum += PERL_VERSION * 200 + PERL_SUBVERSION * 2;  /*  */
    fname[pos] = 'A' + (sum % 26);
    fname[pos + 1] = 'A' + (sum / 26 % 26);
    fname[pos + 2] = '\0';
    return (char *)fname;
}

XS(XS_DynaLoader_mod2fname)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: DynaLoader::mod2fname(sv)");
    {
	SV *	sv = ST(0);
	char *	RETVAL;

	RETVAL = mod2fname(aTHX_ sv);
	ST(0) = sv_newmortal();
	sv_setpv((SV*)ST(0), RETVAL);
    }
    XSRETURN(1);
}

char *
os2error(int rc)
{
	static char buf[300];
	ULONG len;

        if (!(_emx_env & 0x200)) return ""; /* Nop if not OS/2. */
	if (rc == 0)
		return NULL;
	if (DosGetMessage(NULL, 0, buf, sizeof buf - 1, rc, "OSO001.MSG", &len))
		sprintf(buf, "OS/2 system error code %d=0x%x", rc, rc);
	else {
		buf[len] = '\0';
		if (len && buf[len - 1] == '\n')
			buf[--len] = 0;
		if (len && buf[len - 1] == '\r')
			buf[--len] = 0;
		if (len && buf[len - 1] == '.')
			buf[--len] = 0;
	}
	return buf;
}

char *
os2_execname(pTHX)
{
  char buf[300], *p, *o = PL_origargv[0], ok = 1;

  if (_execname(buf, sizeof buf) != 0)
	return o;
  p = buf;
  while (*p) {
    if (*p == '\\')
	*p = '/';
    if (*p == '/') {
	if (ok && *o != '/' && *o != '\\')
	    ok = 0;
    } else if (ok && tolower(*o) != tolower(*p))
	ok = 0;	
    p++;
    o++;
  }
  if (ok) { /* PL_origargv[0] matches the real name.  Use PL_origargv[0]: */
     strcpy(buf, PL_origargv[0]);	/* _execname() is always uppercased */
     p = buf;
     while (*p) {
       if (*p == '\\')
           *p = '/';
       p++;
     }     
  }
  p = savepv(buf);
  SAVEFREEPV(p);
  return p;
}

char *
perllib_mangle(char *s, unsigned int l)
{
    static char *newp, *oldp;
    static int newl, oldl, notfound;
    static char ret[STATIC_FILE_LENGTH+1];
    
    if (!newp && !notfound) {
	newp = getenv("PERLLIB_PREFIX");
	if (newp) {
	    char *s;
	    
	    oldp = newp;
	    while (*newp && !isSPACE(*newp) && *newp != ';') {
		newp++; oldl++;		/* Skip digits. */
	    }
	    while (*newp && (isSPACE(*newp) || *newp == ';')) {
		newp++;			/* Skip whitespace. */
	    }
	    newl = strlen(newp);
	    if (newl == 0 || oldl == 0) {
		Perl_croak_nocontext("Malformed PERLLIB_PREFIX");
	    }
	    strcpy(ret, newp);
	    s = ret;
	    while (*s) {
		if (*s == '\\') *s = '/';
		s++;
	    }
	} else {
	    notfound = 1;
	}
    }
    if (!newp) {
	return s;
    }
    if (l == 0) {
	l = strlen(s);
    }
    if (l < oldl || strnicmp(oldp, s, oldl) != 0) {
	return s;
    }
    if (l + newl - oldl > STATIC_FILE_LENGTH || newl > STATIC_FILE_LENGTH) {
	Perl_croak_nocontext("Malformed PERLLIB_PREFIX");
    }
    strcpy(ret + newl, s + oldl);
    return ret;
}

unsigned long 
Perl_hab_GET()			/* Needed if perl.h cannot be included */
{
    return perl_hab_GET();
}

HMQ
Perl_Register_MQ(int serve)
{
    PPIB pib;
    PTIB tib;

    if (Perl_os2_initial_mode++)
	return Perl_hmq;
    DosGetInfoBlocks(&tib, &pib);
    Perl_os2_initial_mode = pib->pib_ultype;
    /* Try morphing into a PM application. */
    if (pib->pib_ultype != 3)		/* 2 is VIO */
	pib->pib_ultype = 3;		/* 3 is PM */
    init_PMWIN_entries();
    /* 64 messages if before OS/2 3.0, ignored otherwise */
    Perl_hmq = (*PMWIN_entries.CreateMsgQueue)(perl_hab_GET(), 64); 
    if (!Perl_hmq) {
	static int cnt;

	SAVEINT(cnt);			/* Allow catch()ing. */
	if (cnt++)
	    _exit(188);			/* Panic can try to create a window. */
	Perl_croak_nocontext("Cannot create a message queue, or morph to a PM application");
    }
    if (serve) {
	if ( Perl_hmq_servers <= 0	/* Safe to inform us on shutdown, */
	     && Perl_hmq_refcnt > 0 )	/* this was switched off before... */
	    (*PMWIN_entries.CancelShutdown)(Perl_hmq, 0);
	Perl_hmq_servers++;
    } else if (!Perl_hmq_servers)	/* Do not inform us on shutdown */
	(*PMWIN_entries.CancelShutdown)(Perl_hmq, 1);
    Perl_hmq_refcnt++;
    return Perl_hmq;
}

int
Perl_Serve_Messages(int force)
{
    int cnt = 0;
    QMSG msg;

    if (Perl_hmq_servers > 0 && !force)
	return 0;
    if (Perl_hmq_refcnt <= 0)
	Perl_croak_nocontext("No message queue");
    while ((*PMWIN_entries.PeekMsg)(Perl_hab, &msg, NULLHANDLE, 0, 0, PM_REMOVE)) {
	cnt++;
	if (msg.msg == WM_QUIT)
	    Perl_croak_nocontext("QUITing...");
	(*PMWIN_entries.DispatchMsg)(Perl_hab, &msg);
    }
    return cnt;
}

int
Perl_Process_Messages(int force, I32 *cntp)
{
    QMSG msg;

    if (Perl_hmq_servers > 0 && !force)
	return 0;
    if (Perl_hmq_refcnt <= 0)
	Perl_croak_nocontext("No message queue");
    while ((*PMWIN_entries.GetMsg)(Perl_hab, &msg, NULLHANDLE, 0, 0)) {
	if (cntp)
	    (*cntp)++;
	(*PMWIN_entries.DispatchMsg)(Perl_hab, &msg);
	if (msg.msg == WM_DESTROY)
	    return -1;
	if (msg.msg == WM_CREATE)
	    return +1;
    }
    Perl_croak_nocontext("QUITing...");
}

void
Perl_Deregister_MQ(int serve)
{
    PPIB pib;
    PTIB tib;

    if (serve)
	Perl_hmq_servers--;
    if (--Perl_hmq_refcnt <= 0) {
	init_PMWIN_entries();			/* To be extra safe */
	(*PMWIN_entries.DestroyMsgQueue)(Perl_hmq);
	Perl_hmq = 0;
	/* Try morphing back from a PM application. */
	DosGetInfoBlocks(&tib, &pib);
	if (pib->pib_ultype == 3)		/* 3 is PM */
	    pib->pib_ultype = Perl_os2_initial_mode;
	else
	    Perl_warn_nocontext("Unexpected program mode %d when morphing back from PM",
		 pib->pib_ultype);
    } else if (serve && Perl_hmq_servers <= 0)	/* Last server exited */
	(*PMWIN_entries.CancelShutdown)(Perl_hmq, 1);
}

#define sys_is_absolute(path) ( isALPHA((path)[0]) && (path)[1] == ':' \
				&& ((path)[2] == '/' || (path)[2] == '\\'))
#define sys_is_rooted _fnisabs
#define sys_is_relative _fnisrel
#define current_drive _getdrive

#undef chdir				/* Was _chdir2. */
#define sys_chdir(p) (chdir(p) == 0)
#define change_drive(d) (_chdrive(d), (current_drive() == toupper(d)))

static int DOS_harderr_state = -1;    

XS(XS_OS2_Error)
{
    dXSARGS;
    if (items != 2)
	Perl_croak_nocontext("Usage: OS2::Error(harderr, exception)");
    {
	int	arg1 = SvIV(ST(0));
	int	arg2 = SvIV(ST(1));
	int	a = ((arg1 ? FERR_ENABLEHARDERR : FERR_DISABLEHARDERR)
		     | (arg2 ? FERR_ENABLEEXCEPTION : FERR_DISABLEEXCEPTION));
	int	RETVAL = ((arg1 ? 1 : 0) | (arg2 ? 2 : 0));
	unsigned long rc;

	if (CheckOSError(DosError(a)))
	    Perl_croak_nocontext("DosError(%d) failed", a);
	ST(0) = sv_newmortal();
	if (DOS_harderr_state >= 0)
	    sv_setiv(ST(0), DOS_harderr_state);
	DOS_harderr_state = RETVAL;
    }
    XSRETURN(1);
}

static signed char DOS_suppression_state = -1;    

XS(XS_OS2_Errors2Drive)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: OS2::Errors2Drive(drive)");
    {
	STRLEN n_a;
	SV  *sv = ST(0);
	int	suppress = SvOK(sv);
	char	*s = suppress ? SvPV(sv, n_a) : NULL;
	char	drive = (s ? *s : 0);
	unsigned long rc;

	if (suppress && !isALPHA(drive))
	    Perl_croak_nocontext("Non-char argument '%c' to OS2::Errors2Drive()", drive);
	if (CheckOSError(DosSuppressPopUps((suppress
					    ? SPU_ENABLESUPPRESSION 
					    : SPU_DISABLESUPPRESSION),
					   drive)))
	    Perl_croak_nocontext("DosSuppressPopUps(%c) failed", drive);
	ST(0) = sv_newmortal();
	if (DOS_suppression_state > 0)
	    sv_setpvn(ST(0), &DOS_suppression_state, 1);
	else if (DOS_suppression_state == 0)
	    sv_setpvn(ST(0), "", 0);
	DOS_suppression_state = drive;
    }
    XSRETURN(1);
}

static const char * const si_fields[QSV_MAX] = {
  "MAX_PATH_LENGTH",
  "MAX_TEXT_SESSIONS",
  "MAX_PM_SESSIONS",
  "MAX_VDM_SESSIONS",
  "BOOT_DRIVE",
  "DYN_PRI_VARIATION",
  "MAX_WAIT",
  "MIN_SLICE",
  "MAX_SLICE",
  "PAGE_SIZE",
  "VERSION_MAJOR",
  "VERSION_MINOR",
  "VERSION_REVISION",
  "MS_COUNT",
  "TIME_LOW",
  "TIME_HIGH",
  "TOTPHYSMEM",
  "TOTRESMEM",
  "TOTAVAILMEM",
  "MAXPRMEM",
  "MAXSHMEM",
  "TIMER_INTERVAL",
  "MAX_COMP_LENGTH",
  "FOREGROUND_FS_SESSION",
  "FOREGROUND_PROCESS"
};

XS(XS_OS2_SysInfo)
{
    dXSARGS;
    if (items != 0)
	Perl_croak_nocontext("Usage: OS2::SysInfo()");
    {
	ULONG   si[QSV_MAX] = {0};	/* System Information Data Buffer */
	APIRET  rc	= NO_ERROR;	/* Return code            */
	int i = 0, j = 0;

	if (CheckOSError(DosQuerySysInfo(1L, /* Request all available system */
					 QSV_MAX, /* information */
					 (PVOID)si,
					 sizeof(si))))
	    Perl_croak_nocontext("DosQuerySysInfo() failed");
	EXTEND(SP,2*QSV_MAX);
	while (i < QSV_MAX) {
	    ST(j) = sv_newmortal();
	    sv_setpv(ST(j++), si_fields[i]);
	    ST(j) = sv_newmortal();
	    sv_setiv(ST(j++), si[i]);
	    i++;
	}
    }
    XSRETURN(2 * QSV_MAX);
}

XS(XS_OS2_BootDrive)
{
    dXSARGS;
    if (items != 0)
	Perl_croak_nocontext("Usage: OS2::BootDrive()");
    {
	ULONG   si[1] = {0};	/* System Information Data Buffer */
	APIRET  rc    = NO_ERROR;	/* Return code            */
	char c;
	
	if (CheckOSError(DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
					 (PVOID)si, sizeof(si))))
	    Perl_croak_nocontext("DosQuerySysInfo() failed");
	ST(0) = sv_newmortal();
	c = 'a' - 1 + si[0];
	sv_setpvn(ST(0), &c, 1);
    }
    XSRETURN(1);
}

XS(XS_OS2_MorphPM)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: OS2::MorphPM(serve)");
    {
	bool  serve = SvOK(ST(0));
	unsigned long   pmq = perl_hmq_GET(serve);

	ST(0) = sv_newmortal();
	sv_setiv(ST(0), pmq);
    }
    XSRETURN(1);
}

XS(XS_OS2_UnMorphPM)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: OS2::UnMorphPM(serve)");
    {
	bool  serve = SvOK(ST(0));

	perl_hmq_UNSET(serve);
    }
    XSRETURN(0);
}

XS(XS_OS2_Serve_Messages)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: OS2::Serve_Messages(force)");
    {
	bool  force = SvOK(ST(0));
	unsigned long   cnt = Perl_Serve_Messages(force);

	ST(0) = sv_newmortal();
	sv_setiv(ST(0), cnt);
    }
    XSRETURN(1);
}

XS(XS_OS2_Process_Messages)
{
    dXSARGS;
    if (items < 1 || items > 2)
	Perl_croak_nocontext("Usage: OS2::Process_Messages(force [, cnt])");
    {
	bool  force = SvOK(ST(0));
	unsigned long   cnt;

	if (items == 2) {
	    I32 cntr;
	    SV *sv = ST(1);
	    int fake = SvIV(sv);	/* Force SvIVX */
	    
	    if (!SvIOK(sv))
		Perl_croak_nocontext("Can't upgrade count to IV");
	    cntr = SvIVX(sv);
	    cnt =  Perl_Process_Messages(force, &cntr);
	    SvIVX(sv) = cntr;
	} else {
	    cnt =  Perl_Process_Messages(force, NULL);
        }
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), cnt);
    }
    XSRETURN(1);
}

XS(XS_Cwd_current_drive)
{
    dXSARGS;
    if (items != 0)
	Perl_croak_nocontext("Usage: Cwd::current_drive()");
    {
	char	RETVAL;

	RETVAL = current_drive();
	ST(0) = sv_newmortal();
	sv_setpvn(ST(0), (char *)&RETVAL, 1);
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_chdir)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: Cwd::sys_chdir(path)");
    {
	STRLEN n_a;
	char *	path = (char *)SvPV(ST(0),n_a);
	bool	RETVAL;

	RETVAL = sys_chdir(path);
	ST(0) = boolSV(RETVAL);
	if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_change_drive)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: Cwd::change_drive(d)");
    {
	STRLEN n_a;
	char	d = (char)*SvPV(ST(0),n_a);
	bool	RETVAL;

	RETVAL = change_drive(d);
	ST(0) = boolSV(RETVAL);
	if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_is_absolute)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: Cwd::sys_is_absolute(path)");
    {
	STRLEN n_a;
	char *	path = (char *)SvPV(ST(0),n_a);
	bool	RETVAL;

	RETVAL = sys_is_absolute(path);
	ST(0) = boolSV(RETVAL);
	if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_is_rooted)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: Cwd::sys_is_rooted(path)");
    {
	STRLEN n_a;
	char *	path = (char *)SvPV(ST(0),n_a);
	bool	RETVAL;

	RETVAL = sys_is_rooted(path);
	ST(0) = boolSV(RETVAL);
	if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_is_relative)
{
    dXSARGS;
    if (items != 1)
	Perl_croak_nocontext("Usage: Cwd::sys_is_relative(path)");
    {
	STRLEN n_a;
	char *	path = (char *)SvPV(ST(0),n_a);
	bool	RETVAL;

	RETVAL = sys_is_relative(path);
	ST(0) = boolSV(RETVAL);
	if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_cwd)
{
    dXSARGS;
    if (items != 0)
	Perl_croak_nocontext("Usage: Cwd::sys_cwd()");
    {
	char p[MAXPATHLEN];
	char *	RETVAL;
	RETVAL = _getcwd2(p, MAXPATHLEN);
	ST(0) = sv_newmortal();
	sv_setpv((SV*)ST(0), RETVAL);
    }
    XSRETURN(1);
}

XS(XS_Cwd_sys_abspath)
{
    dXSARGS;
    if (items < 1 || items > 2)
	Perl_croak_nocontext("Usage: Cwd::sys_abspath(path, dir = NULL)");
    {
	STRLEN n_a;
	char *	path = (char *)SvPV(ST(0),n_a);
	char *	dir;
	char p[MAXPATHLEN];
	char *	RETVAL;

	if (items < 2)
	    dir = NULL;
	else {
	    dir = (char *)SvPV(ST(1),n_a);
	}
	if (path[0] == '.' && (path[1] == '/' || path[1] == '\\')) {
	    path += 2;
	}
	if (dir == NULL) {
	    if (_abspath(p, path, MAXPATHLEN) == 0) {
		RETVAL = p;
	    } else {
		RETVAL = NULL;
	    }
	} else {
	    /* Absolute with drive: */
	    if ( sys_is_absolute(path) ) {
		if (_abspath(p, path, MAXPATHLEN) == 0) {
		    RETVAL = p;
		} else {
		    RETVAL = NULL;
		}
	    } else if (path[0] == '/' || path[0] == '\\') {
		/* Rooted, but maybe on different drive. */
		if (isALPHA(dir[0]) && dir[1] == ':' ) {
		    char p1[MAXPATHLEN];

		    /* Need to prepend the drive. */
		    p1[0] = dir[0];
		    p1[1] = dir[1];
		    Copy(path, p1 + 2, strlen(path) + 1, char);
		    RETVAL = p;
		    if (_abspath(p, p1, MAXPATHLEN) == 0) {
			RETVAL = p;
		    } else {
			RETVAL = NULL;
		    }
		} else if (_abspath(p, path, MAXPATHLEN) == 0) {
		    RETVAL = p;
		} else {
		    RETVAL = NULL;
		}
	    } else {
		/* Either path is relative, or starts with a drive letter. */
		/* If the path starts with a drive letter, then dir is
		   relevant only if 
		   a/b)	it is absolute/x:relative on the same drive.  
		   c)	path is on current drive, and dir is rooted
		   In all the cases it is safe to drop the drive part
		   of the path. */
		if ( !sys_is_relative(path) ) {
		    int is_drived;

		    if ( ( ( sys_is_absolute(dir)
			     || (isALPHA(dir[0]) && dir[1] == ':' 
				 && strnicmp(dir, path,1) == 0)) 
			   && strnicmp(dir, path,1) == 0)
			 || ( !(isALPHA(dir[0]) && dir[1] == ':')
			      && toupper(path[0]) == current_drive())) {
			path += 2;
		    } else if (_abspath(p, path, MAXPATHLEN) == 0) {
			RETVAL = p; goto done;
		    } else {
			RETVAL = NULL; goto done;
		    }
		}
		{
		    /* Need to prepend the absolute path of dir. */
		    char p1[MAXPATHLEN];

		    if (_abspath(p1, dir, MAXPATHLEN) == 0) {
			int l = strlen(p1);

			if (p1[ l - 1 ] != '/') {
			    p1[ l ] = '/';
			    l++;
			}
			Copy(path, p1 + l, strlen(path) + 1, char);
			if (_abspath(p, p1, MAXPATHLEN) == 0) {
			    RETVAL = p;
			} else {
			    RETVAL = NULL;
			}
		    } else {
			RETVAL = NULL;
		    }
		}
	      done:
	    }
	}
	ST(0) = sv_newmortal();
	sv_setpv((SV*)ST(0), RETVAL);
    }
    XSRETURN(1);
}
typedef APIRET (*PELP)(PSZ path, ULONG type);

APIRET
ExtLIBPATH(ULONG ord, PSZ path, ULONG type)
{
    loadByOrd("doscalls",ord);		/* Guarantied to load or die! */
    return (*(PELP)ExtFCN[ord])(path, type);
}

#define extLibpath(type) 						\
    (CheckOSError(ExtLIBPATH(ORD_QUERY_ELP, to, ((type) ? END_LIBPATH	\
						 : BEGIN_LIBPATH)))	\
     ? NULL : to )

#define extLibpath_set(p,type) 					\
    (!CheckOSError(ExtLIBPATH(ORD_SET_ELP, (p), ((type) ? END_LIBPATH	\
						 : BEGIN_LIBPATH))))

XS(XS_Cwd_extLibpath)
{
    dXSARGS;
    if (items < 0 || items > 1)
	Perl_croak_nocontext("Usage: Cwd::extLibpath(type = 0)");
    {
	bool	type;
	char	to[1024];
	U32	rc;
	char *	RETVAL;

	if (items < 1)
	    type = 0;
	else {
	    type = (int)SvIV(ST(0));
	}

	RETVAL = extLibpath(type);
	ST(0) = sv_newmortal();
	sv_setpv((SV*)ST(0), RETVAL);
    }
    XSRETURN(1);
}

XS(XS_Cwd_extLibpath_set)
{
    dXSARGS;
    if (items < 1 || items > 2)
	Perl_croak_nocontext("Usage: Cwd::extLibpath_set(s, type = 0)");
    {
	STRLEN n_a;
	char *	s = (char *)SvPV(ST(0),n_a);
	bool	type;
	U32	rc;
	bool	RETVAL;

	if (items < 2)
	    type = 0;
	else {
	    type = (int)SvIV(ST(1));
	}

	RETVAL = extLibpath_set(s, type);
	ST(0) = boolSV(RETVAL);
	if (SvREFCNT(ST(0))) sv_2mortal(ST(0));
    }
    XSRETURN(1);
}

#define get_control87()		_control87(0,0)
#define set_control87		_control87

XS(XS_OS2__control87)
{
    dXSARGS;
    if (items != 2)
	croak("Usage: OS2::_control87(new,mask)");
    {
	unsigned	new = (unsigned)SvIV(ST(0));
	unsigned	mask = (unsigned)SvIV(ST(1));
	unsigned	RETVAL;

	RETVAL = _control87(new, mask);
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}

XS(XS_OS2_get_control87)
{
    dXSARGS;
    if (items != 0)
	croak("Usage: OS2::get_control87()");
    {
	unsigned	RETVAL;

	RETVAL = get_control87();
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}


XS(XS_OS2_set_control87)
{
    dXSARGS;
    if (items < 0 || items > 2)
	croak("Usage: OS2::set_control87(new=MCW_EM, mask=MCW_EM)");
    {
	unsigned	new;
	unsigned	mask;
	unsigned	RETVAL;

	if (items < 1)
	    new = MCW_EM;
	else {
	    new = (unsigned)SvIV(ST(0));
	}

	if (items < 2)
	    mask = MCW_EM;
	else {
	    mask = (unsigned)SvIV(ST(1));
	}

	RETVAL = set_control87(new, mask);
	ST(0) = sv_newmortal();
	sv_setiv(ST(0), (IV)RETVAL);
    }
    XSRETURN(1);
}

int
Xs_OS2_init(pTHX)
{
    char *file = __FILE__;
    {
	GV *gv;

	if (_emx_env & 0x200) {	/* OS/2 */
            newXS("File::Copy::syscopy", XS_File__Copy_syscopy, file);
            newXS("Cwd::extLibpath", XS_Cwd_extLibpath, file);
            newXS("Cwd::extLibpath_set", XS_Cwd_extLibpath_set, file);
	}
        newXS("OS2::Error", XS_OS2_Error, file);
        newXS("OS2::Errors2Drive", XS_OS2_Errors2Drive, file);
        newXS("OS2::SysInfo", XS_OS2_SysInfo, file);
        newXS("OS2::BootDrive", XS_OS2_BootDrive, file);
        newXS("OS2::MorphPM", XS_OS2_MorphPM, file);
        newXS("OS2::UnMorphPM", XS_OS2_UnMorphPM, file);
        newXS("OS2::Serve_Messages", XS_OS2_Serve_Messages, file);
        newXS("OS2::Process_Messages", XS_OS2_Process_Messages, file);
        newXS("DynaLoader::mod2fname", XS_DynaLoader_mod2fname, file);
        newXS("Cwd::current_drive", XS_Cwd_current_drive, file);
        newXS("Cwd::sys_chdir", XS_Cwd_sys_chdir, file);
        newXS("Cwd::change_drive", XS_Cwd_change_drive, file);
        newXS("Cwd::sys_is_absolute", XS_Cwd_sys_is_absolute, file);
        newXS("Cwd::sys_is_rooted", XS_Cwd_sys_is_rooted, file);
        newXS("Cwd::sys_is_relative", XS_Cwd_sys_is_relative, file);
        newXS("Cwd::sys_cwd", XS_Cwd_sys_cwd, file);
        newXS("Cwd::sys_abspath", XS_Cwd_sys_abspath, file);
        newXSproto("OS2::_control87", XS_OS2__control87, file, "$$");
        newXSproto("OS2::get_control87", XS_OS2_get_control87, file, "");
        newXSproto("OS2::set_control87", XS_OS2_set_control87, file, ";$$");
	gv = gv_fetchpv("OS2::is_aout", TRUE, SVt_PV);
	GvMULTI_on(gv);
#ifdef PERL_IS_AOUT
	sv_setiv(GvSV(gv), 1);
#endif 
	gv = gv_fetchpv("OS2::emx_rev", TRUE, SVt_PV);
	GvMULTI_on(gv);
	sv_setiv(GvSV(gv), _emx_rev);
	sv_setpv(GvSV(gv), _emx_vprt);
	SvIOK_on(GvSV(gv));
	gv = gv_fetchpv("OS2::emx_env", TRUE, SVt_PV);
	GvMULTI_on(gv);
	sv_setiv(GvSV(gv), _emx_env);
	gv = gv_fetchpv("OS2::os_ver", TRUE, SVt_PV);
	GvMULTI_on(gv);
	sv_setnv(GvSV(gv), _osmajor + 0.001 * _osminor);
    }
}

OS2_Perl_data_t OS2_Perl_data;

void
Perl_OS2_init(char **env)
{
    char *shell;

    MALLOC_INIT;
    settmppath();
    OS2_Perl_data.xs_init = &Xs_OS2_init;
    _uflags (_UF_SBRK_MODEL, _UF_SBRK_ARBITRARY);
    if (environ == NULL && env) {
	environ = env;
    }
    if ( (shell = getenv("PERL_SH_DRIVE")) ) {
	New(1304, PL_sh_path, strlen(SH_PATH) + 1, char);
	strcpy(PL_sh_path, SH_PATH);
	PL_sh_path[0] = shell[0];
    } else if ( (shell = getenv("PERL_SH_DIR")) ) {
	int l = strlen(shell), i;
	if (shell[l-1] == '/' || shell[l-1] == '\\') {
	    l--;
	}
	New(1304, PL_sh_path, l + 8, char);
	strncpy(PL_sh_path, shell, l);
	strcpy(PL_sh_path + l, "/sh.exe");
	for (i = 0; i < l; i++) {
	    if (PL_sh_path[i] == '\\') PL_sh_path[i] = '/';
	}
    }
    MUTEX_INIT(&start_thread_mutex);
    os2_mytype = my_type();		/* Do it before morphing.  Needed? */
    /* Some DLLs reset FP flags on load.  We may have been linked with them */
    _control87(MCW_EM, MCW_EM);
}

#undef tmpnam
#undef tmpfile

char *
my_tmpnam (char *str)
{
    char *p = getenv("TMP"), *tpath;
    int len;

    if (!p) p = getenv("TEMP");
    tpath = tempnam(p, "pltmp");
    if (str && tpath) {
	strcpy(str, tpath);
	return str;
    }
    return tpath;
}

FILE *
my_tmpfile ()
{
    struct stat s;

    stat(".", &s);
    if (s.st_mode & S_IWOTH) {
	return tmpfile();
    }
    return fopen(my_tmpnam(NULL), "w+b"); /* Race condition, but
					     grants TMP. */
}

#undef rmdir

int
my_rmdir (__const__ char *s)
{
    char buf[MAXPATHLEN];
    STRLEN l = strlen(s);

    if (s[l-1] == '/' || s[l-1] == '\\') {	/* EMX rmdir fails... */
	strcpy(buf,s);
	buf[l - 1] = 0;
	s = buf;
    }
    return rmdir(s);
}

#undef mkdir

int
my_mkdir (__const__ char *s, long perm)
{
    char buf[MAXPATHLEN];
    STRLEN l = strlen(s);

    if (s[l-1] == '/' || s[l-1] == '\\') {	/* EMX mkdir fails... */
	strcpy(buf,s);
	buf[l - 1] = 0;
	s = buf;
    }
    return mkdir(s, perm);
}

#undef flock

/* This code was contributed by Rocco Caputo. */
int 
my_flock(int handle, int o)
{
  FILELOCK      rNull, rFull;
  ULONG         timeout, handle_type, flag_word;
  APIRET        rc;
  int           blocking, shared;
  static int	use_my = -1;

  if (use_my == -1) {
    char *s = getenv("USE_PERL_FLOCK");
    if (s)
	use_my = atoi(s);
    else 
	use_my = 1;
  }
  if (!(_emx_env & 0x200) || !use_my) 
    return flock(handle, o);	/* Delegate to EMX. */
  
                                        // is this a file?
  if ((DosQueryHType(handle, &handle_type, &flag_word) != 0) ||
      (handle_type & 0xFF))
  {
    errno = EBADF;
    return -1;
  }
                                        // set lock/unlock ranges
  rNull.lOffset = rNull.lRange = rFull.lOffset = 0;
  rFull.lRange = 0x7FFFFFFF;
                                        // set timeout for blocking
  timeout = ((blocking = !(o & LOCK_NB))) ? 100 : 1;
                                        // shared or exclusive?
  shared = (o & LOCK_SH) ? 1 : 0;
                                        // do not block the unlock
  if (o & (LOCK_UN | LOCK_SH | LOCK_EX)) {
    rc = DosSetFileLocks(handle, &rFull, &rNull, timeout, shared);
    switch (rc) {
      case 0:
        errno = 0;
        return 0;
      case ERROR_INVALID_HANDLE:
        errno = EBADF;
        return -1;
      case ERROR_SHARING_BUFFER_EXCEEDED:
        errno = ENOLCK;
        return -1;
      case ERROR_LOCK_VIOLATION:
        break;                          // not an error
      case ERROR_INVALID_PARAMETER:
      case ERROR_ATOMIC_LOCK_NOT_SUPPORTED:
      case ERROR_READ_LOCKS_NOT_SUPPORTED:
        errno = EINVAL;
        return -1;
      case ERROR_INTERRUPT:
        errno = EINTR;
        return -1;
      default:
        errno = EINVAL;
        return -1;
    }
  }
                                        // lock may block
  if (o & (LOCK_SH | LOCK_EX)) {
                                        // for blocking operations
    for (;;) {
      rc =
        DosSetFileLocks(
                handle,
                &rNull,
                &rFull,
                timeout,
                shared
        );
      switch (rc) {
        case 0:
          errno = 0;
          return 0;
        case ERROR_INVALID_HANDLE:
          errno = EBADF;
          return -1;
        case ERROR_SHARING_BUFFER_EXCEEDED:
          errno = ENOLCK;
          return -1;
        case ERROR_LOCK_VIOLATION:
          if (!blocking) {
            errno = EWOULDBLOCK;
            return -1;
          }
          break;
        case ERROR_INVALID_PARAMETER:
        case ERROR_ATOMIC_LOCK_NOT_SUPPORTED:
        case ERROR_READ_LOCKS_NOT_SUPPORTED:
          errno = EINVAL;
          return -1;
        case ERROR_INTERRUPT:
          errno = EINTR;
          return -1;
        default:
          errno = EINVAL;
          return -1;
      }
                                        // give away timeslice
      DosSleep(1);
    }
  }

  errno = 0;
  return 0;
}
