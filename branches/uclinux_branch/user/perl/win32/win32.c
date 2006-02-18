/* WIN32.C
 *
 * (c) 1995 Microsoft Corporation. All rights reserved. 
 * 		Developed by hip communications inc., http://info.hip.com/info/
 * Portions (c) 1993 Intergraph Corporation. All rights reserved.
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 */

#define WIN32_LEAN_AND_MEAN
#define WIN32IO_IS_STDIO
#include <tchar.h>
#ifdef __GNUC__
#define Win32_Winsock
#endif
#include <windows.h>
#ifndef __MINGW32__	/* GCC/Mingw32-2.95.2 forgot the WINAPI on CommandLineToArgvW() */
#  include <shellapi.h>
#else
   LPWSTR* WINAPI CommandLineToArgvW(LPCWSTR lpCommandLine, int * pNumArgs);
#endif
#include <winnt.h>
#include <io.h>

/* #include "config.h" */

#define PERLIO_NOT_STDIO 0 
#if !defined(PERLIO_IS_STDIO) && !defined(USE_SFIO)
#define PerlIO FILE
#endif

#include <sys/stat.h>
#include "EXTERN.h"
#include "perl.h"

#define NO_XSLOCKS
#define PERL_NO_GET_CONTEXT
#include "XSUB.h"

#include "Win32iop.h"
#include <fcntl.h>
#ifndef __GNUC__
/* assert.h conflicts with #define of assert in perl.h */
#include <assert.h>
#endif
#include <string.h>
#include <stdarg.h>
#include <float.h>
#include <time.h>
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#ifdef __GNUC__
/* Mingw32 defaults to globing command line 
 * So we turn it off like this:
 */
int _CRT_glob = 0;
#endif

#if defined(__MINGW32__)
/* Mingw32 is missing some prototypes */
FILE * _wfopen(LPCWSTR wszFileName, LPCWSTR wszMode);
FILE * _wfdopen(int nFd, LPCWSTR wszMode);
FILE * _freopen(LPCWSTR wszFileName, LPCWSTR wszMode, FILE * pOldStream);
int _flushall();
int _fcloseall();
#endif

#if defined(__BORLANDC__)
#  define _stat stat
#  define _utimbuf utimbuf
#endif

#define EXECF_EXEC 1
#define EXECF_SPAWN 2
#define EXECF_SPAWN_NOWAIT 3

#if defined(PERL_IMPLICIT_SYS)
#  undef win32_get_privlib
#  define win32_get_privlib g_win32_get_privlib
#  undef win32_get_sitelib
#  define win32_get_sitelib g_win32_get_sitelib
#  undef win32_get_vendorlib
#  define win32_get_vendorlib g_win32_get_vendorlib
#  undef do_spawn
#  define do_spawn g_do_spawn
#  undef getlogin
#  define getlogin g_getlogin
#endif

#if defined(PERL_OBJECT)
#  undef do_aspawn
#  define do_aspawn g_do_aspawn
#  undef Perl_do_exec
#  define Perl_do_exec g_do_exec
#endif

static void		get_shell(void);
static long		tokenize(const char *str, char **dest, char ***destv);
	int		do_spawn2(char *cmd, int exectype);
static BOOL		has_shell_metachars(char *ptr);
static long		filetime_to_clock(PFILETIME ft);
static BOOL		filetime_from_time(PFILETIME ft, time_t t);
static char *		get_emd_part(SV **leading, char *trailing, ...);
static void		remove_dead_process(long deceased);
static long		find_pid(int pid);
static char *		qualified_path(const char *cmd);
static char *		win32_get_xlib(const char *pl, const char *xlib,
				       const char *libname);

#ifdef USE_ITHREADS
static void		remove_dead_pseudo_process(long child);
static long		find_pseudo_pid(int pid);
#endif

START_EXTERN_C
HANDLE	w32_perldll_handle = INVALID_HANDLE_VALUE;
char	w32_module_name[MAX_PATH+1];
END_EXTERN_C

static DWORD	w32_platform = (DWORD)-1;

#define ONE_K_BUFSIZE	1024

int 
IsWin95(void)
{
    return (win32_os_id() == VER_PLATFORM_WIN32_WINDOWS);
}

int
IsWinNT(void)
{
    return (win32_os_id() == VER_PLATFORM_WIN32_NT);
}

EXTERN_C void
set_w32_module_name(void)
{
    char* ptr;
    GetModuleFileName((HMODULE)((w32_perldll_handle == INVALID_HANDLE_VALUE)
				? GetModuleHandle(NULL)
				: w32_perldll_handle),
		      w32_module_name, sizeof(w32_module_name));

    /* try to get full path to binary (which may be mangled when perl is
     * run from a 16-bit app) */
    /*PerlIO_printf(Perl_debug_log, "Before %s\n", w32_module_name);*/
    (void)win32_longpath(w32_module_name);
    /*PerlIO_printf(Perl_debug_log, "After  %s\n", w32_module_name);*/

    /* normalize to forward slashes */
    ptr = w32_module_name;
    while (*ptr) {
	if (*ptr == '\\')
	    *ptr = '/';
	++ptr;
    }
}

/* *svp (if non-NULL) is expected to be POK (valid allocated SvPVX(*svp)) */
static char*
get_regstr_from(HKEY hkey, const char *valuename, SV **svp)
{
    /* Retrieve a REG_SZ or REG_EXPAND_SZ from the registry */
    HKEY handle;
    DWORD type;
    const char *subkey = "Software\\Perl";
    char *str = Nullch;
    long retval;

    retval = RegOpenKeyEx(hkey, subkey, 0, KEY_READ, &handle);
    if (retval == ERROR_SUCCESS) {
	DWORD datalen;
	retval = RegQueryValueEx(handle, valuename, 0, &type, NULL, &datalen);
	if (retval == ERROR_SUCCESS
	    && (type == REG_SZ || type == REG_EXPAND_SZ))
	{
	    dTHXo;
	    if (!*svp)
		*svp = sv_2mortal(newSVpvn("",0));
	    SvGROW(*svp, datalen);
	    retval = RegQueryValueEx(handle, valuename, 0, NULL,
				     (PBYTE)SvPVX(*svp), &datalen);
	    if (retval == ERROR_SUCCESS) {
		str = SvPVX(*svp);
		SvCUR_set(*svp,datalen-1);
	    }
	}
	RegCloseKey(handle);
    }
    return str;
}

/* *svp (if non-NULL) is expected to be POK (valid allocated SvPVX(*svp)) */
static char*
get_regstr(const char *valuename, SV **svp)
{
    char *str = get_regstr_from(HKEY_CURRENT_USER, valuename, svp);
    if (!str)
	str = get_regstr_from(HKEY_LOCAL_MACHINE, valuename, svp);
    return str;
}

/* *prev_pathp (if non-NULL) is expected to be POK (valid allocated SvPVX(sv)) */
static char *
get_emd_part(SV **prev_pathp, char *trailing_path, ...)
{
    char base[10];
    va_list ap;
    char mod_name[MAX_PATH+1];
    char *ptr;
    char *optr;
    char *strip;
    int oldsize, newsize;
    STRLEN baselen;

    va_start(ap, trailing_path);
    strip = va_arg(ap, char *);

    sprintf(base, "%d.%d", (int)PERL_REVISION, (int)PERL_VERSION);
    baselen = strlen(base);

    if (!*w32_module_name) {
	set_w32_module_name();
    }
    strcpy(mod_name, w32_module_name);
    ptr = strrchr(mod_name, '/');
    while (ptr && strip) {
        /* look for directories to skip back */
	optr = ptr;
	*ptr = '\0';
	ptr = strrchr(mod_name, '/');
	/* avoid stripping component if there is no slash,
	 * or it doesn't match ... */
	if (!ptr || stricmp(ptr+1, strip) != 0) {
	    /* ... but not if component matches m|5\.$patchlevel.*| */
	    if (!ptr || !(*strip == '5' && *(ptr+1) == '5'
			  && strncmp(strip, base, baselen) == 0
			  && strncmp(ptr+1, base, baselen) == 0))
	    {
		*optr = '/';
		ptr = optr;
	    }
	}
	strip = va_arg(ap, char *);
    }
    if (!ptr) {
	ptr = mod_name;
	*ptr++ = '.';
	*ptr = '/';
    }
    va_end(ap);
    strcpy(++ptr, trailing_path);

    /* only add directory if it exists */
    if (GetFileAttributes(mod_name) != (DWORD) -1) {
	/* directory exists */
	dTHXo;
	if (!*prev_pathp)
	    *prev_pathp = sv_2mortal(newSVpvn("",0));
	sv_catpvn(*prev_pathp, ";", 1);
	sv_catpv(*prev_pathp, mod_name);
	return SvPVX(*prev_pathp);
    }

    return Nullch;
}

char *
win32_get_privlib(const char *pl)
{
    dTHXo;
    char *stdlib = "lib";
    char buffer[MAX_PATH+1];
    SV *sv = Nullsv;

    /* $stdlib = $HKCU{"lib-$]"} || $HKLM{"lib-$]"} || $HKCU{"lib"} || $HKLM{"lib"} || "";  */
    sprintf(buffer, "%s-%s", stdlib, pl);
    if (!get_regstr(buffer, &sv))
	(void)get_regstr(stdlib, &sv);

    /* $stdlib .= ";$EMD/../../lib" */
    return get_emd_part(&sv, stdlib, ARCHNAME, "bin", Nullch);
}

static char *
win32_get_xlib(const char *pl, const char *xlib, const char *libname)
{
    dTHXo;
    char regstr[40];
    char pathstr[MAX_PATH+1];
    DWORD datalen;
    int len, newsize;
    SV *sv1 = Nullsv;
    SV *sv2 = Nullsv;

    /* $HKCU{"$xlib-$]"} || $HKLM{"$xlib-$]"} . ---; */
    sprintf(regstr, "%s-%s", xlib, pl);
    (void)get_regstr(regstr, &sv1);

    /* $xlib .=
     * ";$EMD/" . ((-d $EMD/../../../$]) ? "../../.." : "../.."). "/$libname/$]/lib";  */
    sprintf(pathstr, "%s/%s/lib", libname, pl);
    (void)get_emd_part(&sv1, pathstr, ARCHNAME, "bin", pl, Nullch);

    /* $HKCU{$xlib} || $HKLM{$xlib} . ---; */
    (void)get_regstr(xlib, &sv2);

    /* $xlib .=
     * ";$EMD/" . ((-d $EMD/../../../$]) ? "../../.." : "../.."). "/$libname/lib";  */
    sprintf(pathstr, "%s/lib", libname);
    (void)get_emd_part(&sv2, pathstr, ARCHNAME, "bin", pl, Nullch);

    if (!sv1 && !sv2)
	return Nullch;
    if (!sv1)
	return SvPVX(sv2);
    if (!sv2)
	return SvPVX(sv1);

    sv_catpvn(sv1, ";", 1);
    sv_catsv(sv1, sv2);

    return SvPVX(sv1);
}

char *
win32_get_sitelib(const char *pl)
{
    return win32_get_xlib(pl, "sitelib", "site");
}

#ifndef PERL_VENDORLIB_NAME
#  define PERL_VENDORLIB_NAME	"vendor"
#endif

char *
win32_get_vendorlib(const char *pl)
{
    return win32_get_xlib(pl, "vendorlib", PERL_VENDORLIB_NAME);
}

static BOOL
has_shell_metachars(char *ptr)
{
    int inquote = 0;
    char quote = '\0';

    /*
     * Scan string looking for redirection (< or >) or pipe
     * characters (|) that are not in a quoted string.
     * Shell variable interpolation (%VAR%) can also happen inside strings.
     */
    while (*ptr) {
	switch(*ptr) {
	case '%':
	    return TRUE;
	case '\'':
	case '\"':
	    if (inquote) {
		if (quote == *ptr) {
		    inquote = 0;
		    quote = '\0';
		}
	    }
	    else {
		quote = *ptr;
		inquote++;
	    }
	    break;
	case '>':
	case '<':
	case '|':
	    if (!inquote)
		return TRUE;
	default:
	    break;
	}
	++ptr;
    }
    return FALSE;
}

#if !defined(PERL_IMPLICIT_SYS)
/* since the current process environment is being updated in util.c
 * the library functions will get the correct environment
 */
PerlIO *
Perl_my_popen(pTHX_ char *cmd, char *mode)
{
#ifdef FIXCMD
#define fixcmd(x)   {					\
			char *pspace = strchr((x),' ');	\
			if (pspace) {			\
			    char *p = (x);		\
			    while (p < pspace) {	\
				if (*p == '/')		\
				    *p = '\\';		\
				p++;			\
			    }				\
			}				\
		    }
#else
#define fixcmd(x)
#endif
    fixcmd(cmd);
    PERL_FLUSHALL_FOR_CHILD;
    return win32_popen(cmd, mode);
}

long
Perl_my_pclose(pTHX_ PerlIO *fp)
{
    return win32_pclose(fp);
}
#endif

DllExport unsigned long
win32_os_id(void)
{
    static OSVERSIONINFO osver;

    if (osver.dwPlatformId != w32_platform) {
	memset(&osver, 0, sizeof(OSVERSIONINFO));
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osver);
	w32_platform = osver.dwPlatformId;
    }
    return (unsigned long)w32_platform;
}

DllExport int
win32_getpid(void)
{
    int pid;
#ifdef USE_ITHREADS
    dTHXo;
    if (w32_pseudo_id)
	return -((int)w32_pseudo_id);
#endif
    pid = _getpid();
    /* Windows 9x appears to always reports a pid for threads and processes
     * that has the high bit set. So we treat the lower 31 bits as the
     * "real" PID for Perl's purposes. */
    if (IsWin95() && pid < 0)
	pid = -pid;
    return pid;
}

/* Tokenize a string.  Words are null-separated, and the list
 * ends with a doubled null.  Any character (except null and
 * including backslash) may be escaped by preceding it with a
 * backslash (the backslash will be stripped).
 * Returns number of words in result buffer.
 */
static long
tokenize(const char *str, char **dest, char ***destv)
{
    char *retstart = Nullch;
    char **retvstart = 0;
    int items = -1;
    if (str) {
	dTHXo;
	int slen = strlen(str);
	register char *ret;
	register char **retv;
	New(1307, ret, slen+2, char);
	New(1308, retv, (slen+3)/2, char*);

	retstart = ret;
	retvstart = retv;
	*retv = ret;
	items = 0;
	while (*str) {
	    *ret = *str++;
	    if (*ret == '\\' && *str)
		*ret = *str++;
	    else if (*ret == ' ') {
		while (*str == ' ')
		    str++;
		if (ret == retstart)
		    ret--;
		else {
		    *ret = '\0';
		    ++items;
		    if (*str)
			*++retv = ret+1;
		}
	    }
	    else if (!*str)
		++items;
	    ret++;
	}
	retvstart[items] = Nullch;
	*ret++ = '\0';
	*ret = '\0';
    }
    *dest = retstart;
    *destv = retvstart;
    return items;
}

static void
get_shell(void)
{
    dTHXo;
    if (!w32_perlshell_tokens) {
	/* we don't use COMSPEC here for two reasons:
	 *  1. the same reason perl on UNIX doesn't use SHELL--rampant and
	 *     uncontrolled unportability of the ensuing scripts.
	 *  2. PERL5SHELL could be set to a shell that may not be fit for
	 *     interactive use (which is what most programs look in COMSPEC
	 *     for).
	 */
	const char* defaultshell = (IsWinNT()
				    ? "cmd.exe /x/c" : "command.com /c");
	const char *usershell = getenv("PERL5SHELL");
	w32_perlshell_items = tokenize(usershell ? usershell : defaultshell,
				       &w32_perlshell_tokens,
				       &w32_perlshell_vec);
    }
}

int
do_aspawn(void *vreally, void **vmark, void **vsp)
{
    dTHXo;
    SV *really = (SV*)vreally;
    SV **mark = (SV**)vmark;
    SV **sp = (SV**)vsp;
    char **argv;
    char *str;
    int status;
    int flag = P_WAIT;
    int index = 0;

    if (sp <= mark)
	return -1;

    get_shell();
    New(1306, argv, (sp - mark) + w32_perlshell_items + 2, char*);

    if (SvNIOKp(*(mark+1)) && !SvPOKp(*(mark+1))) {
	++mark;
	flag = SvIVx(*mark);
    }

    while (++mark <= sp) {
	if (*mark && (str = SvPV_nolen(*mark)))
	    argv[index++] = str;
	else
	    argv[index++] = "";
    }
    argv[index++] = 0;
   
    status = win32_spawnvp(flag,
			   (const char*)(really ? SvPV_nolen(really) : argv[0]),
			   (const char* const*)argv);

    if (status < 0 && (errno == ENOEXEC || errno == ENOENT)) {
	/* possible shell-builtin, invoke with shell */
	int sh_items;
	sh_items = w32_perlshell_items;
	while (--index >= 0)
	    argv[index+sh_items] = argv[index];
	while (--sh_items >= 0)
	    argv[sh_items] = w32_perlshell_vec[sh_items];
   
	status = win32_spawnvp(flag,
			       (const char*)(really ? SvPV_nolen(really) : argv[0]),
			       (const char* const*)argv);
    }

    if (flag == P_NOWAIT) {
	if (IsWin95())
	    PL_statusvalue = -1;	/* >16bits hint for pp_system() */
    }
    else {
	if (status < 0) {
	    if (ckWARN(WARN_EXEC))
		Perl_warner(aTHX_ WARN_EXEC, "Can't spawn \"%s\": %s", argv[0], strerror(errno));
	    status = 255 * 256;
	}
	else
	    status *= 256;
	PL_statusvalue = status;
    }
    Safefree(argv);
    return (status);
}

int
do_spawn2(char *cmd, int exectype)
{
    dTHXo;
    char **a;
    char *s;
    char **argv;
    int status = -1;
    BOOL needToTry = TRUE;
    char *cmd2;

    /* Save an extra exec if possible. See if there are shell
     * metacharacters in it */
    if (!has_shell_metachars(cmd)) {
	New(1301,argv, strlen(cmd) / 2 + 2, char*);
	New(1302,cmd2, strlen(cmd) + 1, char);
	strcpy(cmd2, cmd);
	a = argv;
	for (s = cmd2; *s;) {
	    while (*s && isSPACE(*s))
		s++;
	    if (*s)
		*(a++) = s;
	    while (*s && !isSPACE(*s))
		s++;
	    if (*s)
		*s++ = '\0';
	}
	*a = Nullch;
	if (argv[0]) {
	    switch (exectype) {
	    case EXECF_SPAWN:
		status = win32_spawnvp(P_WAIT, argv[0],
				       (const char* const*)argv);
		break;
	    case EXECF_SPAWN_NOWAIT:
		status = win32_spawnvp(P_NOWAIT, argv[0],
				       (const char* const*)argv);
		break;
	    case EXECF_EXEC:
		status = win32_execvp(argv[0], (const char* const*)argv);
		break;
	    }
	    if (status != -1 || errno == 0)
		needToTry = FALSE;
	}
	Safefree(argv);
	Safefree(cmd2);
    }
    if (needToTry) {
	char **argv;
	int i = -1;
	get_shell();
	New(1306, argv, w32_perlshell_items + 2, char*);
	while (++i < w32_perlshell_items)
	    argv[i] = w32_perlshell_vec[i];
	argv[i++] = cmd;
	argv[i] = Nullch;
	switch (exectype) {
	case EXECF_SPAWN:
	    status = win32_spawnvp(P_WAIT, argv[0],
				   (const char* const*)argv);
	    break;
	case EXECF_SPAWN_NOWAIT:
	    status = win32_spawnvp(P_NOWAIT, argv[0],
				   (const char* const*)argv);
	    break;
	case EXECF_EXEC:
	    status = win32_execvp(argv[0], (const char* const*)argv);
	    break;
	}
	cmd = argv[0];
	Safefree(argv);
    }
    if (exectype == EXECF_SPAWN_NOWAIT) {
	if (IsWin95())
	    PL_statusvalue = -1;	/* >16bits hint for pp_system() */
    }
    else {
	if (status < 0) {
	    if (ckWARN(WARN_EXEC))
		Perl_warner(aTHX_ WARN_EXEC, "Can't %s \"%s\": %s",
		     (exectype == EXECF_EXEC ? "exec" : "spawn"),
		     cmd, strerror(errno));
	    status = 255 * 256;
	}
	else
	    status *= 256;
	PL_statusvalue = status;
    }
    return (status);
}

int
do_spawn(char *cmd)
{
    return do_spawn2(cmd, EXECF_SPAWN);
}

int
do_spawn_nowait(char *cmd)
{
    return do_spawn2(cmd, EXECF_SPAWN_NOWAIT);
}

bool
Perl_do_exec(pTHX_ char *cmd)
{
    do_spawn2(cmd, EXECF_EXEC);
    return FALSE;
}

/* The idea here is to read all the directory names into a string table
 * (separated by nulls) and when one of the other dir functions is called
 * return the pointer to the current file name.
 */
DllExport DIR *
win32_opendir(char *filename)
{
    dTHXo;
    DIR			*dirp;
    long		len;
    long		idx;
    char		scanname[MAX_PATH+3];
    struct stat		sbuf;
    WIN32_FIND_DATAA	aFindData;
    WIN32_FIND_DATAW	wFindData;
    HANDLE		fh;
    char		buffer[MAX_PATH*2];
    WCHAR		wbuffer[MAX_PATH+1];
    char*		ptr;

    len = strlen(filename);
    if (len > MAX_PATH)
	return NULL;

    /* check to see if filename is a directory */
    if (win32_stat(filename, &sbuf) < 0 || !S_ISDIR(sbuf.st_mode))
	return NULL;

    /* Get us a DIR structure */
    Newz(1303, dirp, 1, DIR);

    /* Create the search pattern */
    strcpy(scanname, filename);

    /* bare drive name means look in cwd for drive */
    if (len == 2 && isALPHA(scanname[0]) && scanname[1] == ':') {
	scanname[len++] = '.';
	scanname[len++] = '/';
    }
    else if (scanname[len-1] != '/' && scanname[len-1] != '\\') {
	scanname[len++] = '/';
    }
    scanname[len++] = '*';
    scanname[len] = '\0';

    /* do the FindFirstFile call */
    if (USING_WIDE()) {
	A2WHELPER(scanname, wbuffer, sizeof(wbuffer));
	fh = FindFirstFileW(PerlDir_mapW(wbuffer), &wFindData);
    }
    else {
	fh = FindFirstFileA(PerlDir_mapA(scanname), &aFindData);
    }
    dirp->handle = fh;
    if (fh == INVALID_HANDLE_VALUE) {
	DWORD err = GetLastError();
	/* FindFirstFile() fails on empty drives! */
	switch (err) {
	case ERROR_FILE_NOT_FOUND:
	    return dirp;
	case ERROR_NO_MORE_FILES:
	case ERROR_PATH_NOT_FOUND:
	    errno = ENOENT;
	    break;
	case ERROR_NOT_ENOUGH_MEMORY:
	    errno = ENOMEM;
	    break;
	default:
	    errno = EINVAL;
	    break;
	}
	Safefree(dirp);
	return NULL;
    }

    /* now allocate the first part of the string table for
     * the filenames that we find.
     */
    if (USING_WIDE()) {
	W2AHELPER(wFindData.cFileName, buffer, sizeof(buffer));
	ptr = buffer;
    }
    else {
	ptr = aFindData.cFileName;
    }
    idx = strlen(ptr)+1;
    if (idx < 256)
	dirp->size = 128;
    else
	dirp->size = idx;
    New(1304, dirp->start, dirp->size, char);
    strcpy(dirp->start, ptr);
    dirp->nfiles++;
    dirp->end = dirp->curr = dirp->start;
    dirp->end += idx;
    return dirp;
}


/* Readdir just returns the current string pointer and bumps the
 * string pointer to the nDllExport entry.
 */
DllExport struct direct *
win32_readdir(DIR *dirp)
{
    long         len;

    if (dirp->curr) {
	/* first set up the structure to return */
	len = strlen(dirp->curr);
	strcpy(dirp->dirstr.d_name, dirp->curr);
	dirp->dirstr.d_namlen = len;

	/* Fake an inode */
	dirp->dirstr.d_ino = dirp->curr - dirp->start;

	/* Now set up for the next call to readdir */
	dirp->curr += len + 1;
	if (dirp->curr >= dirp->end) {
	    dTHXo;
	    char*		ptr;
	    BOOL		res;
	    WIN32_FIND_DATAW	wFindData;
	    WIN32_FIND_DATAA	aFindData;
	    char		buffer[MAX_PATH*2];

	    /* finding the next file that matches the wildcard
	     * (which should be all of them in this directory!).
	     */
	    if (USING_WIDE()) {
		res = FindNextFileW(dirp->handle, &wFindData);
		if (res) {
		    W2AHELPER(wFindData.cFileName, buffer, sizeof(buffer));
		    ptr = buffer;
		}
	    }
	    else {
		res = FindNextFileA(dirp->handle, &aFindData);
		if (res)
		    ptr = aFindData.cFileName;
	    }
	    if (res) {
		long endpos = dirp->end - dirp->start;
		long newsize = endpos + strlen(ptr) + 1;
		/* bump the string table size by enough for the
		 * new name and it's null terminator */
		while (newsize > dirp->size) {
		    long curpos = dirp->curr - dirp->start;
		    dirp->size *= 2;
		    Renew(dirp->start, dirp->size, char);
		    dirp->curr = dirp->start + curpos;
		}
		strcpy(dirp->start + endpos, ptr);
		dirp->end = dirp->start + newsize;
		dirp->nfiles++;
	    }
	    else
		dirp->curr = NULL;
	}
	return &(dirp->dirstr);
    } 
    else
	return NULL;
}

/* Telldir returns the current string pointer position */
DllExport long
win32_telldir(DIR *dirp)
{
    return (dirp->curr - dirp->start);
}


/* Seekdir moves the string pointer to a previously saved position
 * (returned by telldir).
 */
DllExport void
win32_seekdir(DIR *dirp, long loc)
{
    dirp->curr = dirp->start + loc;
}

/* Rewinddir resets the string pointer to the start */
DllExport void
win32_rewinddir(DIR *dirp)
{
    dirp->curr = dirp->start;
}

/* free the memory allocated by opendir */
DllExport int
win32_closedir(DIR *dirp)
{
    dTHXo;
    if (dirp->handle != INVALID_HANDLE_VALUE)
	FindClose(dirp->handle);
    Safefree(dirp->start);
    Safefree(dirp);
    return 1;
}


/*
 * various stubs
 */


/* Ownership
 *
 * Just pretend that everyone is a superuser. NT will let us know if
 * we don\'t really have permission to do something.
 */

#define ROOT_UID    ((uid_t)0)
#define ROOT_GID    ((gid_t)0)

uid_t
getuid(void)
{
    return ROOT_UID;
}

uid_t
geteuid(void)
{
    return ROOT_UID;
}

gid_t
getgid(void)
{
    return ROOT_GID;
}

gid_t
getegid(void)
{
    return ROOT_GID;
}

int
setuid(uid_t auid)
{ 
    return (auid == ROOT_UID ? 0 : -1);
}

int
setgid(gid_t agid)
{
    return (agid == ROOT_GID ? 0 : -1);
}

char *
getlogin(void)
{
    dTHXo;
    char *buf = w32_getlogin_buffer;
    DWORD size = sizeof(w32_getlogin_buffer);
    if (GetUserName(buf,&size))
	return buf;
    return (char*)NULL;
}

int
chown(const char *path, uid_t owner, gid_t group)
{
    /* XXX noop */
    return 0;
}

static long
find_pid(int pid)
{
    dTHXo;
    long child = w32_num_children;
    while (--child >= 0) {
	if (w32_child_pids[child] == pid)
	    return child;
    }
    return -1;
}

static void
remove_dead_process(long child)
{
    if (child >= 0) {
	dTHXo;
	CloseHandle(w32_child_handles[child]);
	Move(&w32_child_handles[child+1], &w32_child_handles[child],
	     (w32_num_children-child-1), HANDLE);
	Move(&w32_child_pids[child+1], &w32_child_pids[child],
	     (w32_num_children-child-1), DWORD);
	w32_num_children--;
    }
}

#ifdef USE_ITHREADS
static long
find_pseudo_pid(int pid)
{
    dTHXo;
    long child = w32_num_pseudo_children;
    while (--child >= 0) {
	if (w32_pseudo_child_pids[child] == pid)
	    return child;
    }
    return -1;
}

static void
remove_dead_pseudo_process(long child)
{
    if (child >= 0) {
	dTHXo;
	CloseHandle(w32_pseudo_child_handles[child]);
	Move(&w32_pseudo_child_handles[child+1], &w32_pseudo_child_handles[child],
	     (w32_num_pseudo_children-child-1), HANDLE);
	Move(&w32_pseudo_child_pids[child+1], &w32_pseudo_child_pids[child],
	     (w32_num_pseudo_children-child-1), DWORD);
	w32_num_pseudo_children--;
    }
}
#endif

DllExport int
win32_kill(int pid, int sig)
{
    dTHXo;
    HANDLE hProcess;
    long child;
#ifdef USE_ITHREADS
    if (pid < 0) {
	/* it is a pseudo-forked child */
	child = find_pseudo_pid(-pid);
	if (child >= 0) {
	    if (!sig)
		return 0;
	    hProcess = w32_pseudo_child_handles[child];
	    if (TerminateThread(hProcess, sig)) {
		remove_dead_pseudo_process(child);
		return 0;
	    }
	}
	else if (IsWin95()) {
	    pid = -pid;
	    goto alien_process;
	}
    }
    else
#endif
    {
	child = find_pid(pid);
	if (child >= 0) {
	    if (!sig)
		return 0;
	    hProcess = w32_child_handles[child];
	    if (TerminateProcess(hProcess, sig)) {
		remove_dead_process(child);
		return 0;
	    }
	}
	else {
alien_process:
	    hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE,
				   (IsWin95() ? -pid : pid));
	    if (hProcess) {
		if (!sig)
		    return 0;
		if (TerminateProcess(hProcess, sig)) {
		    CloseHandle(hProcess);
		    return 0;
		}
	    }
	}
    }
    errno = EINVAL;
    return -1;
}

/*
 * File system stuff
 */

DllExport unsigned int
win32_sleep(unsigned int t)
{
    Sleep(t*1000);
    return 0;
}

DllExport int
win32_stat(const char *path, struct stat *sbuf)
{
    dTHXo;
    char	buffer[MAX_PATH+1]; 
    int		l = strlen(path);
    int		res;
    WCHAR	wbuffer[MAX_PATH+1];
    WCHAR*	pwbuffer;
    HANDLE      handle;
    int         nlink = 1;

    if (l > 1) {
	switch(path[l - 1]) {
	/* FindFirstFile() and stat() are buggy with a trailing
	 * backslash, so change it to a forward slash :-( */
	case '\\':
	    strncpy(buffer, path, l-1);
	    buffer[l - 1] = '/';
	    buffer[l] = '\0';
	    path = buffer;
	    break;
	/* FindFirstFile() is buggy with "x:", so add a dot :-( */
	case ':':
	    if (l == 2 && isALPHA(path[0])) {
		buffer[0] = path[0];
		buffer[1] = ':';
		buffer[2] = '.';
		buffer[3] = '\0';
		l = 3;
		path = buffer;
	    }
	    break;
	}
    }

    /* We *must* open & close the file once; otherwise file attribute changes */
    /* might not yet have propagated to "other" hard links of the same file.  */
    /* This also gives us an opportunity to determine the number of links.    */
    if (USING_WIDE()) {
	A2WHELPER(path, wbuffer, sizeof(wbuffer));
	pwbuffer = PerlDir_mapW(wbuffer);
	handle = CreateFileW(pwbuffer, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
    }
    else {
	path = PerlDir_mapA(path);
	l = strlen(path);
	handle = CreateFileA(path, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
    }
    if (handle != INVALID_HANDLE_VALUE) {
	BY_HANDLE_FILE_INFORMATION bhi;
	if (GetFileInformationByHandle(handle, &bhi))
	    nlink = bhi.nNumberOfLinks;
	CloseHandle(handle);
    }

    /* pwbuffer or path will be mapped correctly above */
    if (USING_WIDE()) {
	res = _wstat(pwbuffer, (struct _stat *)sbuf);
    }
    else {
	res = stat(path, sbuf);
    }
    sbuf->st_nlink = nlink;

    if (res < 0) {
	/* CRT is buggy on sharenames, so make sure it really isn't.
	 * XXX using GetFileAttributesEx() will enable us to set
	 * sbuf->st_*time (but note that's not available on the
	 * Windows of 1995) */
	DWORD r;
	if (USING_WIDE()) {
	    r = GetFileAttributesW(pwbuffer);
	}
	else {
	    r = GetFileAttributesA(path);
	}
	if (r != 0xffffffff && (r & FILE_ATTRIBUTE_DIRECTORY)) {
	    /* sbuf may still contain old garbage since stat() failed */
	    Zero(sbuf, 1, struct stat);
	    sbuf->st_mode = S_IFDIR | S_IREAD;
	    errno = 0;
	    if (!(r & FILE_ATTRIBUTE_READONLY))
		sbuf->st_mode |= S_IWRITE | S_IEXEC;
	    return 0;
	}
    }
    else {
	if (l == 3 && isALPHA(path[0]) && path[1] == ':'
	    && (path[2] == '\\' || path[2] == '/'))
	{
	    /* The drive can be inaccessible, some _stat()s are buggy */
	    if (USING_WIDE()
		? !GetVolumeInformationW(pwbuffer,NULL,0,NULL,NULL,NULL,NULL,0)
		: !GetVolumeInformationA(path,NULL,0,NULL,NULL,NULL,NULL,0)) {
		errno = ENOENT;
		return -1;
	    }
	}
#ifdef __BORLANDC__
	if (S_ISDIR(sbuf->st_mode))
	    sbuf->st_mode |= S_IWRITE | S_IEXEC;
	else if (S_ISREG(sbuf->st_mode)) {
	    int perms;
	    if (l >= 4 && path[l-4] == '.') {
		const char *e = path + l - 3;
		if (strnicmp(e,"exe",3)
		    && strnicmp(e,"bat",3)
		    && strnicmp(e,"com",3)
		    && (IsWin95() || strnicmp(e,"cmd",3)))
		    sbuf->st_mode &= ~S_IEXEC;
		else
		    sbuf->st_mode |= S_IEXEC;
	    }
	    else
		sbuf->st_mode &= ~S_IEXEC;
	    /* Propagate permissions to _group_ and _others_ */
	    perms = sbuf->st_mode & (S_IREAD|S_IWRITE|S_IEXEC);
	    sbuf->st_mode |= (perms>>3) | (perms>>6);
	}
#endif
    }
    return res;
}

/* Find the longname of a given path.  path is destructively modified.
 * It should have space for at least MAX_PATH characters. */
DllExport char *
win32_longpath(char *path)
{
    WIN32_FIND_DATA fdata;
    HANDLE fhand;
    char tmpbuf[MAX_PATH+1];
    char *tmpstart = tmpbuf;
    char *start = path;
    char sep;
    if (!path)
	return Nullch;

    /* drive prefix */
    if (isALPHA(path[0]) && path[1] == ':' &&
	(path[2] == '/' || path[2] == '\\'))
    {
	start = path + 2;
	*tmpstart++ = path[0];
	*tmpstart++ = ':';
    }
    /* UNC prefix */
    else if ((path[0] == '/' || path[0] == '\\') &&
	     (path[1] == '/' || path[1] == '\\'))
    {
	start = path + 2;
	*tmpstart++ = path[0];
	*tmpstart++ = path[1];
	/* copy machine name */
	while (*start && *start != '/' && *start != '\\')
	    *tmpstart++ = *start++;
	if (*start) {
	    *tmpstart++ = *start;
	    start++;
	    /* copy share name */
	    while (*start && *start != '/' && *start != '\\')
		*tmpstart++ = *start++;
	}
    }
    sep = *start++;
    if (sep == '/' || sep == '\\')
	*tmpstart++ = sep;
    *tmpstart = '\0';
    while (sep) {
	/* walk up to slash */
	while (*start && *start != '/' && *start != '\\')
	    ++start;

	/* discard doubled slashes */
	while (*start && (start[1] == '/' || start[1] == '\\'))
	    ++start;
	sep = *start;

	/* stop and find full name of component */
	*start = '\0';
	fhand = FindFirstFile(path,&fdata);
	if (fhand != INVALID_HANDLE_VALUE) {
	    strcpy(tmpstart, fdata.cFileName);
	    tmpstart += strlen(fdata.cFileName);
	    if (sep)
		*tmpstart++ = sep;
	    *tmpstart = '\0';
	    *start++ = sep;
	    FindClose(fhand);
	}
	else {
	    /* failed a step, just return without side effects */
	    /*PerlIO_printf(Perl_debug_log, "Failed to find %s\n", path);*/
	    *start = sep;
	    return Nullch;
	}
    }
    strcpy(path,tmpbuf);
    return path;
}

DllExport char *
win32_getenv(const char *name)
{
    dTHXo;
    WCHAR wBuffer[MAX_PATH+1];
    DWORD needlen;
    SV *curitem = Nullsv;

    if (USING_WIDE()) {
	A2WHELPER(name, wBuffer, sizeof(wBuffer));
	needlen = GetEnvironmentVariableW(wBuffer, NULL, 0);
    }
    else
	needlen = GetEnvironmentVariableA(name,NULL,0);
    if (needlen != 0) {
	curitem = sv_2mortal(newSVpvn("", 0));
	if (USING_WIDE()) {
	    SV *acuritem;
	    do {
		SvGROW(curitem, (needlen+1)*sizeof(WCHAR));
		needlen = GetEnvironmentVariableW(wBuffer,
						  (WCHAR*)SvPVX(curitem),
						  needlen);
	    } while (needlen >= SvLEN(curitem)/sizeof(WCHAR));
	    SvCUR_set(curitem, (needlen*sizeof(WCHAR))+1);
	    acuritem = sv_2mortal(newSVsv(curitem));
	    W2AHELPER((WCHAR*)SvPVX(acuritem), SvPVX(curitem), SvCUR(curitem));
	}
	else {
	    do {
		SvGROW(curitem, needlen+1);
		needlen = GetEnvironmentVariableA(name,SvPVX(curitem),
						  needlen);
	    } while (needlen >= SvLEN(curitem));
	    SvCUR_set(curitem, needlen);
	}
    }
    else {
	/* allow any environment variables that begin with 'PERL'
	   to be stored in the registry */
	if (strncmp(name, "PERL", 4) == 0)
	    (void)get_regstr(name, &curitem);
    }
    if (curitem && SvCUR(curitem))
	return SvPVX(curitem);

    return Nullch;
}

DllExport int
win32_putenv(const char *name)
{
    dTHXo;
    char* curitem;
    char* val;
    WCHAR* wCuritem;
    WCHAR* wVal;
    int length, relval = -1;

    if (name) {
	if (USING_WIDE()) {
	    length = strlen(name)+1;
	    New(1309,wCuritem,length,WCHAR);
	    A2WHELPER(name, wCuritem, length*sizeof(WCHAR));
	    wVal = wcschr(wCuritem, '=');
	    if (wVal) {
		*wVal++ = '\0';
		if (SetEnvironmentVariableW(wCuritem, *wVal ? wVal : NULL))
		    relval = 0;
	    }
	    Safefree(wCuritem);
	}
	else {
	    New(1309,curitem,strlen(name)+1,char);
	    strcpy(curitem, name);
	    val = strchr(curitem, '=');
	    if (val) {
		/* The sane way to deal with the environment.
		 * Has these advantages over putenv() & co.:
		 *  * enables us to store a truly empty value in the
		 *    environment (like in UNIX).
		 *  * we don't have to deal with RTL globals, bugs and leaks.
		 *  * Much faster.
		 * Why you may want to enable USE_WIN32_RTL_ENV:
		 *  * environ[] and RTL functions will not reflect changes,
		 *    which might be an issue if extensions want to access
		 *    the env. via RTL.  This cuts both ways, since RTL will
		 *    not see changes made by extensions that call the Win32
		 *    functions directly, either.
		 * GSAR 97-06-07
		 */
		*val++ = '\0';
		if (SetEnvironmentVariableA(curitem, *val ? val : NULL))
		    relval = 0;
	    }
	    Safefree(curitem);
	}
    }
    return relval;
}

static long
filetime_to_clock(PFILETIME ft)
{
    __int64 qw = ft->dwHighDateTime;
    qw <<= 32;
    qw |= ft->dwLowDateTime;
    qw /= 10000;  /* File time ticks at 0.1uS, clock at 1mS */
    return (long) qw;
}

DllExport int
win32_times(struct tms *timebuf)
{
    FILETIME user;
    FILETIME kernel;
    FILETIME dummy;
    if (GetProcessTimes(GetCurrentProcess(), &dummy, &dummy, 
                        &kernel,&user)) {
	timebuf->tms_utime = filetime_to_clock(&user);
	timebuf->tms_stime = filetime_to_clock(&kernel);
	timebuf->tms_cutime = 0;
	timebuf->tms_cstime = 0;
        
    } else { 
        /* That failed - e.g. Win95 fallback to clock() */
        clock_t t = clock();
	timebuf->tms_utime = t;
	timebuf->tms_stime = 0;
	timebuf->tms_cutime = 0;
	timebuf->tms_cstime = 0;
    }
    return 0;
}

/* fix utime() so it works on directories in NT */
static BOOL
filetime_from_time(PFILETIME pFileTime, time_t Time)
{
    struct tm *pTM = localtime(&Time);
    SYSTEMTIME SystemTime;
    FILETIME LocalTime;

    if (pTM == NULL)
	return FALSE;

    SystemTime.wYear   = pTM->tm_year + 1900;
    SystemTime.wMonth  = pTM->tm_mon + 1;
    SystemTime.wDay    = pTM->tm_mday;
    SystemTime.wHour   = pTM->tm_hour;
    SystemTime.wMinute = pTM->tm_min;
    SystemTime.wSecond = pTM->tm_sec;
    SystemTime.wMilliseconds = 0;

    return SystemTimeToFileTime(&SystemTime, &LocalTime) &&
           LocalFileTimeToFileTime(&LocalTime, pFileTime);
}

DllExport int
win32_unlink(const char *filename)
{
    dTHXo;
    int ret;
    DWORD attrs;

    if (USING_WIDE()) {
	WCHAR wBuffer[MAX_PATH+1];
	WCHAR* pwBuffer;

	A2WHELPER(filename, wBuffer, sizeof(wBuffer));
	pwBuffer = PerlDir_mapW(wBuffer);
	attrs = GetFileAttributesW(pwBuffer);
	if (attrs == 0xFFFFFFFF)
	    goto fail;
	if (attrs & FILE_ATTRIBUTE_READONLY) {
	    (void)SetFileAttributesW(pwBuffer, attrs & ~FILE_ATTRIBUTE_READONLY);
	    ret = _wunlink(pwBuffer);
	    if (ret == -1)
		(void)SetFileAttributesW(pwBuffer, attrs);
	}
	else
	    ret = _wunlink(pwBuffer);
    }
    else {
	filename = PerlDir_mapA(filename);
	attrs = GetFileAttributesA(filename);
	if (attrs == 0xFFFFFFFF)
	    goto fail;
	if (attrs & FILE_ATTRIBUTE_READONLY) {
	    (void)SetFileAttributesA(filename, attrs & ~FILE_ATTRIBUTE_READONLY);
	    ret = unlink(filename);
	    if (ret == -1)
		(void)SetFileAttributesA(filename, attrs);
	}
	else
	    ret = unlink(filename);
    }
    return ret;
fail:
    errno = ENOENT;
    return -1;
}

DllExport int
win32_utime(const char *filename, struct utimbuf *times)
{
    dTHXo;
    HANDLE handle;
    FILETIME ftCreate;
    FILETIME ftAccess;
    FILETIME ftWrite;
    struct utimbuf TimeBuffer;
    WCHAR wbuffer[MAX_PATH+1];
    WCHAR* pwbuffer;

    int rc;
    if (USING_WIDE()) {
	A2WHELPER(filename, wbuffer, sizeof(wbuffer));
	pwbuffer = PerlDir_mapW(wbuffer);
	rc = _wutime(pwbuffer, (struct _utimbuf*)times);
    }
    else {
	filename = PerlDir_mapA(filename);
	rc = utime(filename, times);
    }
    /* EACCES: path specifies directory or readonly file */
    if (rc == 0 || errno != EACCES /* || !IsWinNT() */)
	return rc;

    if (times == NULL) {
	times = &TimeBuffer;
	time(&times->actime);
	times->modtime = times->actime;
    }

    /* This will (and should) still fail on readonly files */
    if (USING_WIDE()) {
	handle = CreateFileW(pwbuffer, GENERIC_READ | GENERIC_WRITE,
			    FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
			    OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    }
    else {
	handle = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE,
			    FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
			    OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    }
    if (handle == INVALID_HANDLE_VALUE)
	return rc;

    if (GetFileTime(handle, &ftCreate, &ftAccess, &ftWrite) &&
	filetime_from_time(&ftAccess, times->actime) &&
	filetime_from_time(&ftWrite, times->modtime) &&
	SetFileTime(handle, &ftCreate, &ftAccess, &ftWrite))
    {
	rc = 0;
    }

    CloseHandle(handle);
    return rc;
}

DllExport int
win32_uname(struct utsname *name)
{
    struct hostent *hep;
    STRLEN nodemax = sizeof(name->nodename)-1;
    OSVERSIONINFO osver;

    memset(&osver, 0, sizeof(OSVERSIONINFO));
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osver)) {
	/* sysname */
	switch (osver.dwPlatformId) {
	case VER_PLATFORM_WIN32_WINDOWS:
	    strcpy(name->sysname, "Windows");
	    break;
	case VER_PLATFORM_WIN32_NT:
	    strcpy(name->sysname, "Windows NT");
	    break;
	case VER_PLATFORM_WIN32s:
	    strcpy(name->sysname, "Win32s");
	    break;
	default:
	    strcpy(name->sysname, "Win32 Unknown");
	    break;
	}

	/* release */
	sprintf(name->release, "%d.%d",
		osver.dwMajorVersion, osver.dwMinorVersion);

	/* version */
	sprintf(name->version, "Build %d",
		osver.dwPlatformId == VER_PLATFORM_WIN32_NT
		? osver.dwBuildNumber : (osver.dwBuildNumber & 0xffff));
	if (osver.szCSDVersion[0]) {
	    char *buf = name->version + strlen(name->version);
	    sprintf(buf, " (%s)", osver.szCSDVersion);
	}
    }
    else {
	*name->sysname = '\0';
	*name->version = '\0';
	*name->release = '\0';
    }

    /* nodename */
    hep = win32_gethostbyname("localhost");
    if (hep) {
	STRLEN len = strlen(hep->h_name);
	if (len <= nodemax) {
	    strcpy(name->nodename, hep->h_name);
	}
	else {
	    strncpy(name->nodename, hep->h_name, nodemax);
	    name->nodename[nodemax] = '\0';
	}
    }
    else {
	DWORD sz = nodemax;
	if (!GetComputerName(name->nodename, &sz))
	    *name->nodename = '\0';
    }

    /* machine (architecture) */
    {
	SYSTEM_INFO info;
	char *arch;
	GetSystemInfo(&info);

#if (defined(__BORLANDC__)&&(__BORLANDC__<=0x520)) || defined(__MINGW32__)
	switch (info.u.s.wProcessorArchitecture) {
#else
	switch (info.wProcessorArchitecture) {
#endif
	case PROCESSOR_ARCHITECTURE_INTEL:
	    arch = "x86"; break;
	case PROCESSOR_ARCHITECTURE_MIPS:
	    arch = "mips"; break;
	case PROCESSOR_ARCHITECTURE_ALPHA:
	    arch = "alpha"; break;
	case PROCESSOR_ARCHITECTURE_PPC:
	    arch = "ppc"; break;
	default:
	    arch = "unknown"; break;
	}
	strcpy(name->machine, arch);
    }
    return 0;
}

DllExport int
win32_waitpid(int pid, int *status, int flags)
{
    dTHXo;
    DWORD timeout = (flags & WNOHANG) ? 0 : INFINITE;
    int retval = -1;
    long child;
    if (pid == -1)				/* XXX threadid == 1 ? */
	return win32_wait(status);
#ifdef USE_ITHREADS
    else if (pid < 0) {
	child = find_pseudo_pid(-pid);
	if (child >= 0) {
	    HANDLE hThread = w32_pseudo_child_handles[child];
	    DWORD waitcode = WaitForSingleObject(hThread, timeout);
	    if (waitcode == WAIT_TIMEOUT) {
		return 0;
	    }
	    else if (waitcode != WAIT_FAILED) {
		if (GetExitCodeThread(hThread, &waitcode)) {
		    *status = (int)((waitcode & 0xff) << 8);
		    retval = (int)w32_pseudo_child_pids[child];
		    remove_dead_pseudo_process(child);
		    return -retval;
		}
	    }
	    else
		errno = ECHILD;
	}
	else if (IsWin95()) {
	    pid = -pid;
	    goto alien_process;
	}
    }
#endif
    else {
	HANDLE hProcess;
	DWORD waitcode;
	child = find_pid(pid);
	if (child >= 0) {
	    hProcess = w32_child_handles[child];
	    waitcode = WaitForSingleObject(hProcess, timeout);
	    if (waitcode == WAIT_TIMEOUT) {
		return 0;
	    }
	    else if (waitcode != WAIT_FAILED) {
		if (GetExitCodeProcess(hProcess, &waitcode)) {
		    *status = (int)((waitcode & 0xff) << 8);
		    retval = (int)w32_child_pids[child];
		    remove_dead_process(child);
		    return retval;
		}
	    }
	    else
		errno = ECHILD;
	}
	else {
alien_process:
	    hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE,
				   (IsWin95() ? -pid : pid));
	    if (hProcess) {
		waitcode = WaitForSingleObject(hProcess, timeout);
		if (waitcode == WAIT_TIMEOUT) {
		    return 0;
		}
		else if (waitcode != WAIT_FAILED) {
		    if (GetExitCodeProcess(hProcess, &waitcode)) {
			*status = (int)((waitcode & 0xff) << 8);
			CloseHandle(hProcess);
			return pid;
		    }
		}
		CloseHandle(hProcess);
	    }
	    else
		errno = ECHILD;
	}
    }
    return retval >= 0 ? pid : retval;                
}

DllExport int
win32_wait(int *status)
{
    /* XXX this wait emulation only knows about processes
     * spawned via win32_spawnvp(P_NOWAIT, ...).
     */
    dTHXo;
    int i, retval;
    DWORD exitcode, waitcode;

#ifdef USE_ITHREADS
    if (w32_num_pseudo_children) {
	waitcode = WaitForMultipleObjects(w32_num_pseudo_children,
					  w32_pseudo_child_handles,
					  FALSE,
					  INFINITE);
	if (waitcode != WAIT_FAILED) {
	    if (waitcode >= WAIT_ABANDONED_0
		&& waitcode < WAIT_ABANDONED_0 + w32_num_pseudo_children)
		i = waitcode - WAIT_ABANDONED_0;
	    else
		i = waitcode - WAIT_OBJECT_0;
	    if (GetExitCodeThread(w32_pseudo_child_handles[i], &exitcode)) {
		*status = (int)((exitcode & 0xff) << 8);
		retval = (int)w32_pseudo_child_pids[i];
		remove_dead_pseudo_process(i);
		return -retval;
	    }
	}
    }
#endif

    if (!w32_num_children) {
	errno = ECHILD;
	return -1;
    }

    /* if a child exists, wait for it to die */
    waitcode = WaitForMultipleObjects(w32_num_children,
				      w32_child_handles,
				      FALSE,
				      INFINITE);
    if (waitcode != WAIT_FAILED) {
	if (waitcode >= WAIT_ABANDONED_0
	    && waitcode < WAIT_ABANDONED_0 + w32_num_children)
	    i = waitcode - WAIT_ABANDONED_0;
	else
	    i = waitcode - WAIT_OBJECT_0;
	if (GetExitCodeProcess(w32_child_handles[i], &exitcode) ) {
	    *status = (int)((exitcode & 0xff) << 8);
	    retval = (int)w32_child_pids[i];
	    remove_dead_process(i);
	    return retval;
	}
    }

FAILED:
    errno = GetLastError();
    return -1;
}

#ifndef PERL_OBJECT

static UINT timerid = 0;

static VOID CALLBACK TimerProc(HWND win, UINT msg, UINT id, DWORD time)
{
    dTHXo;
    KillTimer(NULL,timerid);
    timerid=0;  
    CALL_FPTR(PL_sighandlerp)(14);
}
#endif	/* !PERL_OBJECT */

DllExport unsigned int
win32_alarm(unsigned int sec)
{
#ifndef PERL_OBJECT
    /* 
     * the 'obvious' implentation is SetTimer() with a callback
     * which does whatever receiving SIGALRM would do 
     * we cannot use SIGALRM even via raise() as it is not 
     * one of the supported codes in <signal.h>
     *
     * Snag is unless something is looking at the message queue
     * nothing happens :-(
     */ 
    dTHXo;
    if (sec)
     {
      timerid = SetTimer(NULL,timerid,sec*1000,(TIMERPROC)TimerProc);
      if (!timerid)
       Perl_croak_nocontext("Cannot set timer");
     } 
    else
     {
      if (timerid)
       {
        KillTimer(NULL,timerid);
        timerid=0;  
       }
     }
#endif	/* !PERL_OBJECT */
    return 0;
}

#ifdef HAVE_DES_FCRYPT
extern char *	des_fcrypt(const char *txt, const char *salt, char *cbuf);
#endif

DllExport char *
win32_crypt(const char *txt, const char *salt)
{
    dTHXo;
#ifdef HAVE_DES_FCRYPT
    return des_fcrypt(txt, salt, w32_crypt_buffer);
#else
    Perl_croak(aTHX_ "The crypt() function is unimplemented due to excessive paranoia.");
    return Nullch;
#endif
}

#ifdef USE_FIXED_OSFHANDLE

#define FOPEN			0x01	/* file handle open */
#define FNOINHERIT		0x10	/* file handle opened O_NOINHERIT */
#define FAPPEND			0x20	/* file handle opened O_APPEND */
#define FDEV			0x40	/* file handle refers to device */
#define FTEXT			0x80	/* file handle is in text mode */

/***
*int my_open_osfhandle(long osfhandle, int flags) - open C Runtime file handle
*
*Purpose:
*       This function allocates a free C Runtime file handle and associates
*       it with the Win32 HANDLE specified by the first parameter. This is a
*	temperary fix for WIN95's brain damage GetFileType() error on socket
*	we just bypass that call for socket
*
*	This works with MSVC++ 4.0+ or GCC/Mingw32
*
*Entry:
*       long osfhandle - Win32 HANDLE to associate with C Runtime file handle.
*       int flags      - flags to associate with C Runtime file handle.
*
*Exit:
*       returns index of entry in fh, if successful
*       return -1, if no free entry is found
*
*Exceptions:
*
*******************************************************************************/

/*
 * we fake up some parts of the CRT that aren't exported by MSVCRT.dll
 * this lets sockets work on Win9X with GCC and should fix the problems
 * with perl95.exe
 *	-- BKS, 1-23-2000
*/

/* create an ioinfo entry, kill its handle, and steal the entry */

static int
_alloc_osfhnd(void)
{
    HANDLE hF = CreateFile("NUL", 0, 0, NULL, OPEN_ALWAYS, 0, NULL);
    int fh = _open_osfhandle((long)hF, 0);
    CloseHandle(hF);
    if (fh == -1)
        return fh;
    EnterCriticalSection(&(_pioinfo(fh)->lock));
    return fh;
}

static int
my_open_osfhandle(long osfhandle, int flags)
{
    int fh;
    char fileflags;		/* _osfile flags */

    /* copy relevant flags from second parameter */
    fileflags = FDEV;

    if (flags & O_APPEND)
	fileflags |= FAPPEND;

    if (flags & O_TEXT)
	fileflags |= FTEXT;

    if (flags & O_NOINHERIT)
	fileflags |= FNOINHERIT;

    /* attempt to allocate a C Runtime file handle */
    if ((fh = _alloc_osfhnd()) == -1) {
	errno = EMFILE;		/* too many open files */
	_doserrno = 0L;		/* not an OS error */
	return -1;		/* return error to caller */
    }

    /* the file is open. now, set the info in _osfhnd array */
    _set_osfhnd(fh, osfhandle);

    fileflags |= FOPEN;		/* mark as open */

    _osfile(fh) = fileflags;	/* set osfile entry */
    LeaveCriticalSection(&_pioinfo(fh)->lock);

    return fh;			/* return handle */
}

#endif	/* USE_FIXED_OSFHANDLE */

/* simulate flock by locking a range on the file */

#define LK_ERR(f,i)	((f) ? (i = 0) : (errno = GetLastError()))
#define LK_LEN		0xffff0000

DllExport int
win32_flock(int fd, int oper)
{
    OVERLAPPED o;
    int i = -1;
    HANDLE fh;

    if (!IsWinNT()) {
	dTHXo;
	Perl_croak_nocontext("flock() unimplemented on this platform");
	return -1;
    }
    fh = (HANDLE)_get_osfhandle(fd);
    memset(&o, 0, sizeof(o));

    switch(oper) {
    case LOCK_SH:		/* shared lock */
	LK_ERR(LockFileEx(fh, 0, 0, LK_LEN, 0, &o),i);
	break;
    case LOCK_EX:		/* exclusive lock */
	LK_ERR(LockFileEx(fh, LOCKFILE_EXCLUSIVE_LOCK, 0, LK_LEN, 0, &o),i);
	break;
    case LOCK_SH|LOCK_NB:	/* non-blocking shared lock */
	LK_ERR(LockFileEx(fh, LOCKFILE_FAIL_IMMEDIATELY, 0, LK_LEN, 0, &o),i);
	break;
    case LOCK_EX|LOCK_NB:	/* non-blocking exclusive lock */
	LK_ERR(LockFileEx(fh,
		       LOCKFILE_EXCLUSIVE_LOCK|LOCKFILE_FAIL_IMMEDIATELY,
		       0, LK_LEN, 0, &o),i);
	break;
    case LOCK_UN:		/* unlock lock */
	LK_ERR(UnlockFileEx(fh, 0, LK_LEN, 0, &o),i);
	break;
    default:			/* unknown */
	errno = EINVAL;
	break;
    }
    return i;
}

#undef LK_ERR
#undef LK_LEN

/*
 *  redirected io subsystem for all XS modules
 *
 */

DllExport int *
win32_errno(void)
{
    return (&errno);
}

DllExport char ***
win32_environ(void)
{
    return (&(_environ));
}

/* the rest are the remapped stdio routines */
DllExport FILE *
win32_stderr(void)
{
    return (stderr);
}

DllExport FILE *
win32_stdin(void)
{
    return (stdin);
}

DllExport FILE *
win32_stdout()
{
    return (stdout);
}

DllExport int
win32_ferror(FILE *fp)
{
    return (ferror(fp));
}


DllExport int
win32_feof(FILE *fp)
{
    return (feof(fp));
}

/*
 * Since the errors returned by the socket error function 
 * WSAGetLastError() are not known by the library routine strerror
 * we have to roll our own.
 */

DllExport char *
win32_strerror(int e) 
{
#ifndef __BORLANDC__		/* Borland intolerance */
    extern int sys_nerr;
#endif
    DWORD source = 0;

    if (e < 0 || e > sys_nerr) {
        dTHXo;
	if (e < 0)
	    e = GetLastError();

	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, &source, e, 0,
			  w32_strerror_buffer,
			  sizeof(w32_strerror_buffer), NULL) == 0) 
	    strcpy(w32_strerror_buffer, "Unknown Error");

	return w32_strerror_buffer;
    }
    return strerror(e);
}

DllExport void
win32_str_os_error(void *sv, DWORD dwErr)
{
    DWORD dwLen;
    char *sMsg;
    dwLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER
			  |FORMAT_MESSAGE_IGNORE_INSERTS
			  |FORMAT_MESSAGE_FROM_SYSTEM, NULL,
			   dwErr, 0, (char *)&sMsg, 1, NULL);
    /* strip trailing whitespace and period */
    if (0 < dwLen) {
	do {
	    --dwLen;	/* dwLen doesn't include trailing null */
	} while (0 < dwLen && isSPACE(sMsg[dwLen]));
	if ('.' != sMsg[dwLen])
	    dwLen++;
	sMsg[dwLen] = '\0';
    }
    if (0 == dwLen) {
	sMsg = (char*)LocalAlloc(0, 64/**sizeof(TCHAR)*/);
	if (sMsg)
	    dwLen = sprintf(sMsg,
			    "Unknown error #0x%lX (lookup 0x%lX)",
			    dwErr, GetLastError());
    }
    if (sMsg) {
	dTHXo;
	sv_setpvn((SV*)sv, sMsg, dwLen);
	LocalFree(sMsg);
    }
}


DllExport int
win32_fprintf(FILE *fp, const char *format, ...)
{
    va_list marker;
    va_start(marker, format);     /* Initialize variable arguments. */

    return (vfprintf(fp, format, marker));
}

DllExport int
win32_printf(const char *format, ...)
{
    va_list marker;
    va_start(marker, format);     /* Initialize variable arguments. */

    return (vprintf(format, marker));
}

DllExport int
win32_vfprintf(FILE *fp, const char *format, va_list args)
{
    return (vfprintf(fp, format, args));
}

DllExport int
win32_vprintf(const char *format, va_list args)
{
    return (vprintf(format, args));
}

DllExport size_t
win32_fread(void *buf, size_t size, size_t count, FILE *fp)
{
    return fread(buf, size, count, fp);
}

DllExport size_t
win32_fwrite(const void *buf, size_t size, size_t count, FILE *fp)
{
    return fwrite(buf, size, count, fp);
}

#define MODE_SIZE 10

DllExport FILE *
win32_fopen(const char *filename, const char *mode)
{
    dTHXo;
    WCHAR wMode[MODE_SIZE], wBuffer[MAX_PATH+1];
    FILE *f;
    
    if (!*filename)
	return NULL;

    if (stricmp(filename, "/dev/null")==0)
	filename = "NUL";

    if (USING_WIDE()) {
	A2WHELPER(mode, wMode, sizeof(wMode));
	A2WHELPER(filename, wBuffer, sizeof(wBuffer));
	f = _wfopen(PerlDir_mapW(wBuffer), wMode);
    }
    else
	f = fopen(PerlDir_mapA(filename), mode);
    /* avoid buffering headaches for child processes */
    if (f && *mode == 'a')
	win32_fseek(f, 0, SEEK_END);
    return f;
}

#ifndef USE_SOCKETS_AS_HANDLES
#undef fdopen
#define fdopen my_fdopen
#endif

DllExport FILE *
win32_fdopen(int handle, const char *mode)
{
    dTHXo;
    WCHAR wMode[MODE_SIZE];
    FILE *f;
    if (USING_WIDE()) {
	A2WHELPER(mode, wMode, sizeof(wMode));
	f = _wfdopen(handle, wMode);
    }
    else
	f = fdopen(handle, (char *) mode);
    /* avoid buffering headaches for child processes */
    if (f && *mode == 'a')
	win32_fseek(f, 0, SEEK_END);
    return f;
}

DllExport FILE *
win32_freopen(const char *path, const char *mode, FILE *stream)
{
    dTHXo;
    WCHAR wMode[MODE_SIZE], wBuffer[MAX_PATH+1];
    if (stricmp(path, "/dev/null")==0)
	path = "NUL";

    if (USING_WIDE()) {
	A2WHELPER(mode, wMode, sizeof(wMode));
	A2WHELPER(path, wBuffer, sizeof(wBuffer));
	return _wfreopen(PerlDir_mapW(wBuffer), wMode, stream);
    }
    return freopen(PerlDir_mapA(path), mode, stream);
}

DllExport int
win32_fclose(FILE *pf)
{
    return my_fclose(pf);	/* defined in win32sck.c */
}

DllExport int
win32_fputs(const char *s,FILE *pf)
{
    return fputs(s, pf);
}

DllExport int
win32_fputc(int c,FILE *pf)
{
    return fputc(c,pf);
}

DllExport int
win32_ungetc(int c,FILE *pf)
{
    return ungetc(c,pf);
}

DllExport int
win32_getc(FILE *pf)
{
    return getc(pf);
}

DllExport int
win32_fileno(FILE *pf)
{
    return fileno(pf);
}

DllExport void
win32_clearerr(FILE *pf)
{
    clearerr(pf);
    return;
}

DllExport int
win32_fflush(FILE *pf)
{
    return fflush(pf);
}

DllExport long
win32_ftell(FILE *pf)
{
    return ftell(pf);
}

DllExport int
win32_fseek(FILE *pf,long offset,int origin)
{
    return fseek(pf, offset, origin);
}

DllExport int
win32_fgetpos(FILE *pf,fpos_t *p)
{
    return fgetpos(pf, p);
}

DllExport int
win32_fsetpos(FILE *pf,const fpos_t *p)
{
    return fsetpos(pf, p);
}

DllExport void
win32_rewind(FILE *pf)
{
    rewind(pf);
    return;
}

DllExport FILE*
win32_tmpfile(void)
{
    return tmpfile();
}

DllExport void
win32_abort(void)
{
    abort();
    return;
}

DllExport int
win32_fstat(int fd,struct stat *sbufptr)
{
#ifdef __BORLANDC__
    /* A file designated by filehandle is not shown as accessible
     * for write operations, probably because it is opened for reading.
     * --Vadim Konovalov
     */ 
    int rc = fstat(fd,sbufptr);
    BY_HANDLE_FILE_INFORMATION bhfi;
    if (GetFileInformationByHandle((HANDLE)_get_osfhandle(fd), &bhfi)) {
        sbufptr->st_mode &= 0xFE00;
        if (bhfi.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
            sbufptr->st_mode |= (S_IREAD + (S_IREAD >> 3) + (S_IREAD >> 6));
        else
            sbufptr->st_mode |= ((S_IREAD|S_IWRITE) + ((S_IREAD|S_IWRITE) >> 3)
              + ((S_IREAD|S_IWRITE) >> 6));
    }
    return rc;
#else
    return my_fstat(fd,sbufptr);
#endif
}

DllExport int
win32_pipe(int *pfd, unsigned int size, int mode)
{
    return _pipe(pfd, size, mode);
}

/*
 * a popen() clone that respects PERL5SHELL
 */

DllExport FILE*
win32_popen(const char *command, const char *mode)
{
#ifdef USE_RTL_POPEN
    return _popen(command, mode);
#else
    int p[2];
    int parent, child;
    int stdfd, oldfd;
    int ourmode;
    int childpid;

    /* establish which ends read and write */
    if (strchr(mode,'w')) {
        stdfd = 0;		/* stdin */
        parent = 1;
        child = 0;
    }
    else if (strchr(mode,'r')) {
        stdfd = 1;		/* stdout */
        parent = 0;
        child = 1;
    }
    else
        return NULL;

    /* set the correct mode */
    if (strchr(mode,'b'))
        ourmode = O_BINARY;
    else if (strchr(mode,'t'))
        ourmode = O_TEXT;
    else
        ourmode = _fmode & (O_TEXT | O_BINARY);

    /* the child doesn't inherit handles */
    ourmode |= O_NOINHERIT;

    if (win32_pipe( p, 512, ourmode) == -1)
        return NULL;

    /* save current stdfd */
    if ((oldfd = win32_dup(stdfd)) == -1)
        goto cleanup;

    /* make stdfd go to child end of pipe (implicitly closes stdfd) */
    /* stdfd will be inherited by the child */
    if (win32_dup2(p[child], stdfd) == -1)
        goto cleanup;

    /* close the child end in parent */
    win32_close(p[child]);

    /* start the child */
    {
	dTHXo;
	if ((childpid = do_spawn_nowait((char*)command)) == -1)
	    goto cleanup;

	/* revert stdfd to whatever it was before */
	if (win32_dup2(oldfd, stdfd) == -1)
	    goto cleanup;

	/* close saved handle */
	win32_close(oldfd);

	LOCK_FDPID_MUTEX;
	sv_setiv(*av_fetch(w32_fdpid, p[parent], TRUE), childpid);
	UNLOCK_FDPID_MUTEX;

	/* set process id so that it can be returned by perl's open() */
	PL_forkprocess = childpid;
    }

    /* we have an fd, return a file stream */
    return (win32_fdopen(p[parent], (char *)mode));

cleanup:
    /* we don't need to check for errors here */
    win32_close(p[0]);
    win32_close(p[1]);
    if (oldfd != -1) {
        win32_dup2(oldfd, stdfd);
        win32_close(oldfd);
    }
    return (NULL);

#endif /* USE_RTL_POPEN */
}

/*
 * pclose() clone
 */

DllExport int
win32_pclose(FILE *pf)
{
#ifdef USE_RTL_POPEN
    return _pclose(pf);
#else
    dTHXo;
    int childpid, status;
    SV *sv;

    LOCK_FDPID_MUTEX;
    sv = *av_fetch(w32_fdpid, win32_fileno(pf), TRUE);

    if (SvIOK(sv))
	childpid = SvIVX(sv);
    else
	childpid = 0;

    if (!childpid) {
	errno = EBADF;
        return -1;
    }

    win32_fclose(pf);
    SvIVX(sv) = 0;
    UNLOCK_FDPID_MUTEX;

    if (win32_waitpid(childpid, &status, 0) == -1)
        return -1;

    return status;

#endif /* USE_RTL_POPEN */
}

static BOOL WINAPI
Nt4CreateHardLinkW(
    LPCWSTR lpFileName,
    LPCWSTR lpExistingFileName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    HANDLE handle;
    WCHAR wFullName[MAX_PATH+1];
    LPVOID lpContext = NULL;
    WIN32_STREAM_ID StreamId;
    DWORD dwSize = (char*)&StreamId.cStreamName - (char*)&StreamId;
    DWORD dwWritten;
    DWORD dwLen;
    BOOL bSuccess;

    BOOL (__stdcall *pfnBackupWrite)(HANDLE, LPBYTE, DWORD, LPDWORD,
				     BOOL, BOOL, LPVOID*) =
	(BOOL (__stdcall *)(HANDLE, LPBYTE, DWORD, LPDWORD,
			    BOOL, BOOL, LPVOID*))
	GetProcAddress(GetModuleHandle("kernel32.dll"), "BackupWrite");
    if (pfnBackupWrite == NULL)
	return 0;

    dwLen = GetFullPathNameW(lpFileName, MAX_PATH, wFullName, NULL);
    if (dwLen == 0)
	return 0;
    dwLen = (dwLen+1)*sizeof(WCHAR);

    handle = CreateFileW(lpExistingFileName, FILE_WRITE_ATTRIBUTES,
			 FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			 NULL, OPEN_EXISTING, 0, NULL);
    if (handle == INVALID_HANDLE_VALUE)
	return 0;

    StreamId.dwStreamId = BACKUP_LINK;
    StreamId.dwStreamAttributes = 0;
    StreamId.dwStreamNameSize = 0;
#if defined(__BORLANDC__) || defined(__MINGW32__)
    StreamId.Size.u.HighPart = 0;
    StreamId.Size.u.LowPart = dwLen;
#else
    StreamId.Size.HighPart = 0;
    StreamId.Size.LowPart = dwLen;
#endif

    bSuccess = pfnBackupWrite(handle, (LPBYTE)&StreamId, dwSize, &dwWritten,
			      FALSE, FALSE, &lpContext);
    if (bSuccess) {
	bSuccess = pfnBackupWrite(handle, (LPBYTE)wFullName, dwLen, &dwWritten,
				  FALSE, FALSE, &lpContext);
	pfnBackupWrite(handle, NULL, 0, &dwWritten, TRUE, FALSE, &lpContext);
    }

    CloseHandle(handle);
    return bSuccess;
}

DllExport int
win32_link(const char *oldname, const char *newname)
{
    dTHXo;
    BOOL (__stdcall *pfnCreateHardLinkW)(LPCWSTR,LPCWSTR,LPSECURITY_ATTRIBUTES);
    WCHAR wOldName[MAX_PATH+1];
    WCHAR wNewName[MAX_PATH+1];

    if (IsWin95())
	Perl_croak(aTHX_ PL_no_func, "link");

    pfnCreateHardLinkW =
	(BOOL (__stdcall *)(LPCWSTR, LPCWSTR, LPSECURITY_ATTRIBUTES))
	GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateHardLinkW");
    if (pfnCreateHardLinkW == NULL)
	pfnCreateHardLinkW = Nt4CreateHardLinkW;

    if ((A2WHELPER(oldname, wOldName, sizeof(wOldName))) &&
	(A2WHELPER(newname, wNewName, sizeof(wNewName))) &&
	(wcscpy(wOldName, PerlDir_mapW(wOldName)),
	pfnCreateHardLinkW(PerlDir_mapW(wNewName), wOldName, NULL)))
    {
	return 0;
    }
    errno = (GetLastError() == ERROR_FILE_NOT_FOUND) ? ENOENT : EINVAL;
    return -1;
}

DllExport int
win32_rename(const char *oname, const char *newname)
{
    WCHAR wOldName[MAX_PATH+1];
    WCHAR wNewName[MAX_PATH+1];
    char szOldName[MAX_PATH+1];
    char szNewName[MAX_PATH+1];
    BOOL bResult;
    dTHXo;

    /* XXX despite what the documentation says about MoveFileEx(),
     * it doesn't work under Windows95!
     */
    if (IsWinNT()) {
	DWORD dwFlags = MOVEFILE_COPY_ALLOWED;
	if (USING_WIDE()) {
	    A2WHELPER(oname, wOldName, sizeof(wOldName));
	    A2WHELPER(newname, wNewName, sizeof(wNewName));
	    if (wcsicmp(wNewName, wOldName))
		dwFlags |= MOVEFILE_REPLACE_EXISTING;
	    wcscpy(wOldName, PerlDir_mapW(wOldName));
	    bResult = MoveFileExW(wOldName,PerlDir_mapW(wNewName), dwFlags);
	}
	else {
	    if (stricmp(newname, oname))
		dwFlags |= MOVEFILE_REPLACE_EXISTING;
	    strcpy(szOldName, PerlDir_mapA(oname));
	    bResult = MoveFileExA(szOldName,PerlDir_mapA(newname), dwFlags);
	}
	if (!bResult) {
	    DWORD err = GetLastError();
	    switch (err) {
	    case ERROR_BAD_NET_NAME:
	    case ERROR_BAD_NETPATH:
	    case ERROR_BAD_PATHNAME:
	    case ERROR_FILE_NOT_FOUND:
	    case ERROR_FILENAME_EXCED_RANGE:
	    case ERROR_INVALID_DRIVE:
	    case ERROR_NO_MORE_FILES:
	    case ERROR_PATH_NOT_FOUND:
		errno = ENOENT;
		break;
	    default:
		errno = EACCES;
		break;
	    }
	    return -1;
	}
	return 0;
    }
    else {
	int retval = 0;
	char szTmpName[MAX_PATH+1];
	char dname[MAX_PATH+1];
	char *endname = Nullch;
	STRLEN tmplen = 0;
	DWORD from_attr, to_attr;

	strcpy(szOldName, PerlDir_mapA(oname));
	strcpy(szNewName, PerlDir_mapA(newname));

	/* if oname doesn't exist, do nothing */
	from_attr = GetFileAttributes(szOldName);
	if (from_attr == 0xFFFFFFFF) {
	    errno = ENOENT;
	    return -1;
	}

	/* if newname exists, rename it to a temporary name so that we
	 * don't delete it in case oname happens to be the same file
	 * (but perhaps accessed via a different path)
	 */
	to_attr = GetFileAttributes(szNewName);
	if (to_attr != 0xFFFFFFFF) {
	    /* if newname is a directory, we fail
	     * XXX could overcome this with yet more convoluted logic */
	    if (to_attr & FILE_ATTRIBUTE_DIRECTORY) {
		errno = EACCES;
		return -1;
	    }
	    tmplen = strlen(szNewName);
	    strcpy(szTmpName,szNewName);
	    endname = szTmpName+tmplen;
	    for (; endname > szTmpName ; --endname) {
		if (*endname == '/' || *endname == '\\') {
		    *endname = '\0';
		    break;
		}
	    }
	    if (endname > szTmpName)
		endname = strcpy(dname,szTmpName);
	    else
		endname = ".";

	    /* get a temporary filename in same directory
	     * XXX is this really the best we can do? */
	    if (!GetTempFileName((LPCTSTR)endname, "plr", 0, szTmpName)) {
		errno = ENOENT;
		return -1;
	    }
	    DeleteFile(szTmpName);

	    retval = rename(szNewName, szTmpName);
	    if (retval != 0) {
		errno = EACCES;
		return retval;
	    }
	}

	/* rename oname to newname */
	retval = rename(szOldName, szNewName);

	/* if we created a temporary file before ... */
	if (endname != Nullch) {
	    /* ...and rename succeeded, delete temporary file/directory */
	    if (retval == 0)
		DeleteFile(szTmpName);
	    /* else restore it to what it was */
	    else
		(void)rename(szTmpName, szNewName);
	}
	return retval;
    }
}

DllExport int
win32_setmode(int fd, int mode)
{
    return setmode(fd, mode);
}

DllExport long
win32_lseek(int fd, long offset, int origin)
{
    return lseek(fd, offset, origin);
}

DllExport long
win32_tell(int fd)
{
    return tell(fd);
}

DllExport int
win32_open(const char *path, int flag, ...)
{
    dTHXo;
    va_list ap;
    int pmode;
    WCHAR wBuffer[MAX_PATH+1];

    va_start(ap, flag);
    pmode = va_arg(ap, int);
    va_end(ap);

    if (stricmp(path, "/dev/null")==0)
	path = "NUL";

    if (USING_WIDE()) {
	A2WHELPER(path, wBuffer, sizeof(wBuffer));
	return _wopen(PerlDir_mapW(wBuffer), flag, pmode);
    }
    return open(PerlDir_mapA(path), flag, pmode);
}

DllExport int
win32_close(int fd)
{
    return close(fd);
}

DllExport int
win32_eof(int fd)
{
    return eof(fd);
}

DllExport int
win32_dup(int fd)
{
    return dup(fd);
}

DllExport int
win32_dup2(int fd1,int fd2)
{
    return dup2(fd1,fd2);
}

#ifdef PERL_MSVCRT_READFIX

#define LF		10	/* line feed */
#define CR		13	/* carriage return */
#define CTRLZ		26      /* ctrl-z means eof for text */
#define FOPEN		0x01	/* file handle open */
#define FEOFLAG		0x02	/* end of file has been encountered */
#define FCRLF		0x04	/* CR-LF across read buffer (in text mode) */
#define FPIPE		0x08	/* file handle refers to a pipe */
#define FAPPEND		0x20	/* file handle opened O_APPEND */
#define FDEV		0x40	/* file handle refers to device */
#define FTEXT		0x80	/* file handle is in text mode */
#define MAX_DESCRIPTOR_COUNT	(64*32) /* this is the maximun that MSVCRT can handle */

int __cdecl
_fixed_read(int fh, void *buf, unsigned cnt)
{
    int bytes_read;                 /* number of bytes read */
    char *buffer;                   /* buffer to read to */
    int os_read;                    /* bytes read on OS call */
    char *p, *q;                    /* pointers into buffer */
    char peekchr;                   /* peek-ahead character */
    ULONG filepos;                  /* file position after seek */
    ULONG dosretval;                /* o.s. return value */

    /* validate handle */
    if (((unsigned)fh >= (unsigned)MAX_DESCRIPTOR_COUNT) ||
         !(_osfile(fh) & FOPEN))
    {
	/* out of range -- return error */
	errno = EBADF;
	_doserrno = 0;  /* not o.s. error */
	return -1;
    }

    /*
     * If lockinitflag is FALSE, assume fd is device
     * lockinitflag is set to TRUE by open.
     */
    if (_pioinfo(fh)->lockinitflag)
	EnterCriticalSection(&(_pioinfo(fh)->lock));  /* lock file */

    bytes_read = 0;                 /* nothing read yet */
    buffer = (char*)buf;

    if (cnt == 0 || (_osfile(fh) & FEOFLAG)) {
        /* nothing to read or at EOF, so return 0 read */
        goto functionexit;
    }

    if ((_osfile(fh) & (FPIPE|FDEV)) && _pipech(fh) != LF) {
        /* a pipe/device and pipe lookahead non-empty: read the lookahead
         * char */
        *buffer++ = _pipech(fh);
        ++bytes_read;
        --cnt;
        _pipech(fh) = LF;           /* mark as empty */
    }

    /* read the data */

    if (!ReadFile((HANDLE)_osfhnd(fh), buffer, cnt, (LPDWORD)&os_read, NULL))
    {
        /* ReadFile has reported an error. recognize two special cases.
         *
         *      1. map ERROR_ACCESS_DENIED to EBADF
         *
         *      2. just return 0 if ERROR_BROKEN_PIPE has occurred. it
         *         means the handle is a read-handle on a pipe for which
         *         all write-handles have been closed and all data has been
         *         read. */

        if ((dosretval = GetLastError()) == ERROR_ACCESS_DENIED) {
            /* wrong read/write mode should return EBADF, not EACCES */
            errno = EBADF;
            _doserrno = dosretval;
            bytes_read = -1;
	    goto functionexit;
        }
        else if (dosretval == ERROR_BROKEN_PIPE) {
            bytes_read = 0;
	    goto functionexit;
        }
        else {
            bytes_read = -1;
	    goto functionexit;
        }
    }

    bytes_read += os_read;          /* update bytes read */

    if (_osfile(fh) & FTEXT) {
        /* now must translate CR-LFs to LFs in the buffer */

        /* set CRLF flag to indicate LF at beginning of buffer */
        /* if ((os_read != 0) && (*(char *)buf == LF))   */
        /*    _osfile(fh) |= FCRLF;                      */
        /* else                                          */
        /*    _osfile(fh) &= ~FCRLF;                     */

        _osfile(fh) &= ~FCRLF;

        /* convert chars in the buffer: p is src, q is dest */
        p = q = (char*)buf;
        while (p < (char *)buf + bytes_read) {
            if (*p == CTRLZ) {
                /* if fh is not a device, set ctrl-z flag */
                if (!(_osfile(fh) & FDEV))
                    _osfile(fh) |= FEOFLAG;
                break;              /* stop translating */
            }
            else if (*p != CR)
                *q++ = *p++;
            else {
                /* *p is CR, so must check next char for LF */
                if (p < (char *)buf + bytes_read - 1) {
                    if (*(p+1) == LF) {
                        p += 2;
                        *q++ = LF;  /* convert CR-LF to LF */
                    }
                    else
                        *q++ = *p++;    /* store char normally */
                }
                else {
                    /* This is the hard part.  We found a CR at end of
                       buffer.  We must peek ahead to see if next char
                       is an LF. */
                    ++p;

                    dosretval = 0;
                    if (!ReadFile((HANDLE)_osfhnd(fh), &peekchr, 1,
                                    (LPDWORD)&os_read, NULL))
                        dosretval = GetLastError();

                    if (dosretval != 0 || os_read == 0) {
                        /* couldn't read ahead, store CR */
                        *q++ = CR;
                    }
                    else {
                        /* peekchr now has the extra character -- we now
                           have several possibilities:
                           1. disk file and char is not LF; just seek back
                              and copy CR
                           2. disk file and char is LF; store LF, don't seek back
                           3. pipe/device and char is LF; store LF.
                           4. pipe/device and char isn't LF, store CR and
                              put char in pipe lookahead buffer. */
                        if (_osfile(fh) & (FDEV|FPIPE)) {
                            /* non-seekable device */
                            if (peekchr == LF)
                                *q++ = LF;
                            else {
                                *q++ = CR;
                                _pipech(fh) = peekchr;
                            }
                        }
                        else {
                            /* disk file */
                            if (peekchr == LF) {
                                /* nothing read yet; must make some
                                   progress */
                                *q++ = LF;
                                /* turn on this flag for tell routine */
                                _osfile(fh) |= FCRLF;
                            }
                            else {
				HANDLE osHandle;        /* o.s. handle value */
                                /* seek back */
				if ((osHandle = (HANDLE)_get_osfhandle(fh)) != (HANDLE)-1)
				{
				    if ((filepos = SetFilePointer(osHandle, -1, NULL, FILE_CURRENT)) == -1)
					dosretval = GetLastError();
				}
                                if (peekchr != LF)
                                    *q++ = CR;
                            }
                        }
                    }
                }
            }
        }

        /* we now change bytes_read to reflect the true number of chars
           in the buffer */
        bytes_read = q - (char *)buf;
    }

functionexit:	
    if (_pioinfo(fh)->lockinitflag)
	LeaveCriticalSection(&(_pioinfo(fh)->lock));    /* unlock file */

    return bytes_read;
}

#endif	/* PERL_MSVCRT_READFIX */

DllExport int
win32_read(int fd, void *buf, unsigned int cnt)
{
#ifdef PERL_MSVCRT_READFIX
    return _fixed_read(fd, buf, cnt);
#else
    return read(fd, buf, cnt);
#endif
}

DllExport int
win32_write(int fd, const void *buf, unsigned int cnt)
{
    return write(fd, buf, cnt);
}

DllExport int
win32_mkdir(const char *dir, int mode)
{
    dTHXo;
    if (USING_WIDE()) {
	WCHAR wBuffer[MAX_PATH+1];
	A2WHELPER(dir, wBuffer, sizeof(wBuffer));
	return _wmkdir(PerlDir_mapW(wBuffer));
    }
    return mkdir(PerlDir_mapA(dir)); /* just ignore mode */
}

DllExport int
win32_rmdir(const char *dir)
{
    dTHXo;
    if (USING_WIDE()) {
	WCHAR wBuffer[MAX_PATH+1];
	A2WHELPER(dir, wBuffer, sizeof(wBuffer));
	return _wrmdir(PerlDir_mapW(wBuffer));
    }
    return rmdir(PerlDir_mapA(dir));
}

DllExport int
win32_chdir(const char *dir)
{
    dTHXo;
    if (USING_WIDE()) {
	WCHAR wBuffer[MAX_PATH+1];
	A2WHELPER(dir, wBuffer, sizeof(wBuffer));
	return _wchdir(wBuffer);
    }
    return chdir(dir);
}

DllExport  int
win32_access(const char *path, int mode)
{
    dTHXo;
    if (USING_WIDE()) {
	WCHAR wBuffer[MAX_PATH+1];
	A2WHELPER(path, wBuffer, sizeof(wBuffer));
	return _waccess(PerlDir_mapW(wBuffer), mode);
    }
    return access(PerlDir_mapA(path), mode);
}

DllExport  int
win32_chmod(const char *path, int mode)
{
    dTHXo;
    if (USING_WIDE()) {
	WCHAR wBuffer[MAX_PATH+1];
	A2WHELPER(path, wBuffer, sizeof(wBuffer));
	return _wchmod(PerlDir_mapW(wBuffer), mode);
    }
    return chmod(PerlDir_mapA(path), mode);
}


static char *
create_command_line(const char* command, const char * const *args)
{
    dTHXo;
    int index;
    char *cmd, *ptr, *arg;
    STRLEN len = strlen(command) + 1;

    for (index = 0; (ptr = (char*)args[index]) != NULL; ++index)
	len += strlen(ptr) + 1;

    New(1310, cmd, len, char);
    ptr = cmd;
    strcpy(ptr, command);

    for (index = 0; (arg = (char*)args[index]) != NULL; ++index) {
	ptr += strlen(ptr);
	*ptr++ = ' ';
	strcpy(ptr, arg);
    }

    return cmd;
}

static char *
qualified_path(const char *cmd)
{
    dTHXo;
    char *pathstr;
    char *fullcmd, *curfullcmd;
    STRLEN cmdlen = 0;
    int has_slash = 0;

    if (!cmd)
	return Nullch;
    fullcmd = (char*)cmd;
    while (*fullcmd) {
	if (*fullcmd == '/' || *fullcmd == '\\')
	    has_slash++;
	fullcmd++;
	cmdlen++;
    }

    /* look in PATH */
    pathstr = win32_getenv("PATH");
    New(0, fullcmd, MAX_PATH+1, char);
    curfullcmd = fullcmd;

    while (1) {
	DWORD res;

	/* start by appending the name to the current prefix */
	strcpy(curfullcmd, cmd);
	curfullcmd += cmdlen;

	/* if it doesn't end with '.', or has no extension, try adding
	 * a trailing .exe first */
	if (cmd[cmdlen-1] != '.'
	    && (cmdlen < 4 || cmd[cmdlen-4] != '.'))
	{
	    strcpy(curfullcmd, ".exe");
	    res = GetFileAttributes(fullcmd);
	    if (res != 0xFFFFFFFF && !(res & FILE_ATTRIBUTE_DIRECTORY))
		return fullcmd;
	    *curfullcmd = '\0';
	}

	/* that failed, try the bare name */
	res = GetFileAttributes(fullcmd);
	if (res != 0xFFFFFFFF && !(res & FILE_ATTRIBUTE_DIRECTORY))
	    return fullcmd;

	/* quit if no other path exists, or if cmd already has path */
	if (!pathstr || !*pathstr || has_slash)
	    break;

	/* skip leading semis */
	while (*pathstr == ';')
	    pathstr++;

	/* build a new prefix from scratch */
	curfullcmd = fullcmd;
	while (*pathstr && *pathstr != ';') {
	    if (*pathstr == '"') {	/* foo;"baz;etc";bar */
		pathstr++;		/* skip initial '"' */
		while (*pathstr && *pathstr != '"') {
		    if (curfullcmd-fullcmd < MAX_PATH-cmdlen-5)
			*curfullcmd++ = *pathstr;
		    pathstr++;
		}
		if (*pathstr)
		    pathstr++;		/* skip trailing '"' */
	    }
	    else {
		if (curfullcmd-fullcmd < MAX_PATH-cmdlen-5)
		    *curfullcmd++ = *pathstr;
		pathstr++;
	    }
	}
	if (*pathstr)
	    pathstr++;			/* skip trailing semi */
	if (curfullcmd > fullcmd	/* append a dir separator */
	    && curfullcmd[-1] != '/' && curfullcmd[-1] != '\\')
	{
	    *curfullcmd++ = '\\';
	}
    }
GIVE_UP:
    Safefree(fullcmd);
    return Nullch;
}

/* The following are just place holders.
 * Some hosts may provide and environment that the OS is
 * not tracking, therefore, these host must provide that
 * environment and the current directory to CreateProcess
 */

void*
get_childenv(void)
{
    return NULL;
}

void
free_childenv(void* d)
{
}

char*
get_childdir(void)
{
    dTHXo;
    char* ptr;
    char szfilename[(MAX_PATH+1)*2];
    if (USING_WIDE()) {
	WCHAR wfilename[MAX_PATH+1];
	GetCurrentDirectoryW(MAX_PATH+1, wfilename);
	W2AHELPER(wfilename, szfilename, sizeof(szfilename));
    }
    else {
	GetCurrentDirectoryA(MAX_PATH+1, szfilename);
    }

    New(0, ptr, strlen(szfilename)+1, char);
    strcpy(ptr, szfilename);
    return ptr;
}

void
free_childdir(char* d)
{
    dTHXo;
    Safefree(d);
}


/* XXX this needs to be made more compatible with the spawnvp()
 * provided by the various RTLs.  In particular, searching for
 * *.{com,bat,cmd} files (as done by the RTLs) is unimplemented.
 * This doesn't significantly affect perl itself, because we
 * always invoke things using PERL5SHELL if a direct attempt to
 * spawn the executable fails.
 * 
 * XXX splitting and rejoining the commandline between do_aspawn()
 * and win32_spawnvp() could also be avoided.
 */

DllExport int
win32_spawnvp(int mode, const char *cmdname, const char *const *argv)
{
#ifdef USE_RTL_SPAWNVP
    return spawnvp(mode, cmdname, (char * const *)argv);
#else
    dTHXo;
    int ret;
    void* env;
    char* dir;
    child_IO_table tbl;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    DWORD create = 0;

    char *cmd = create_command_line(cmdname, strcmp(cmdname, argv[0]) == 0
			     	             ? &argv[1] : argv);
    char *fullcmd = Nullch;

    env = PerlEnv_get_childenv();
    dir = PerlEnv_get_childdir();

    switch(mode) {
    case P_NOWAIT:	/* asynch + remember result */
	if (w32_num_children >= MAXIMUM_WAIT_OBJECTS) {
	    errno = EAGAIN;
	    ret = -1;
	    goto RETVAL;
	}
	/* FALL THROUGH */
    case P_WAIT:	/* synchronous execution */
	break;
    default:		/* invalid mode */
	errno = EINVAL;
	ret = -1;
	goto RETVAL;
    }
    memset(&StartupInfo,0,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    memset(&tbl,0,sizeof(tbl));
    PerlEnv_get_child_IO(&tbl);
    StartupInfo.dwFlags		= tbl.dwFlags;
    StartupInfo.dwX		= tbl.dwX; 
    StartupInfo.dwY		= tbl.dwY; 
    StartupInfo.dwXSize		= tbl.dwXSize; 
    StartupInfo.dwYSize		= tbl.dwYSize; 
    StartupInfo.dwXCountChars	= tbl.dwXCountChars; 
    StartupInfo.dwYCountChars	= tbl.dwYCountChars; 
    StartupInfo.dwFillAttribute	= tbl.dwFillAttribute; 
    StartupInfo.wShowWindow	= tbl.wShowWindow; 
    StartupInfo.hStdInput	= tbl.childStdIn;
    StartupInfo.hStdOutput	= tbl.childStdOut;
    StartupInfo.hStdError	= tbl.childStdErr;
    if (StartupInfo.hStdInput != INVALID_HANDLE_VALUE &&
	StartupInfo.hStdOutput != INVALID_HANDLE_VALUE &&
	StartupInfo.hStdError != INVALID_HANDLE_VALUE)
    {
	StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
    }
    else {
	create |= CREATE_NEW_CONSOLE;
    }

RETRY:
    if (!CreateProcess(cmdname,		/* search PATH to find executable */
		       cmd,		/* executable, and its arguments */
		       NULL,		/* process attributes */
		       NULL,		/* thread attributes */
		       TRUE,		/* inherit handles */
		       create,		/* creation flags */
		       (LPVOID)env,	/* inherit environment */
		       dir,		/* inherit cwd */
		       &StartupInfo,
		       &ProcessInformation))
    {
	/* initial NULL argument to CreateProcess() does a PATH
	 * search, but it always first looks in the directory
	 * where the current process was started, which behavior
	 * is undesirable for backward compatibility.  So we
	 * jump through our own hoops by picking out the path
	 * we really want it to use. */
	if (!fullcmd) {
	    fullcmd = qualified_path(cmdname);
	    if (fullcmd) {
		cmdname = fullcmd;
		goto RETRY;
	    }
	}
	errno = ENOENT;
	ret = -1;
	goto RETVAL;
    }

    if (mode == P_NOWAIT) {
	/* asynchronous spawn -- store handle, return PID */
	ret = (int)ProcessInformation.dwProcessId;
	if (IsWin95() && ret < 0)
	    ret = -ret;

	w32_child_handles[w32_num_children] = ProcessInformation.hProcess;
	w32_child_pids[w32_num_children] = (DWORD)ret;
	++w32_num_children;
    }
    else  {
	DWORD status;
	WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
	GetExitCodeProcess(ProcessInformation.hProcess, &status);
	ret = (int)status;
	CloseHandle(ProcessInformation.hProcess);
    }

    CloseHandle(ProcessInformation.hThread);

RETVAL:
    PerlEnv_free_childenv(env);
    PerlEnv_free_childdir(dir);
    Safefree(cmd);
    Safefree(fullcmd);
    return ret;
#endif
}

DllExport int
win32_execv(const char *cmdname, const char *const *argv)
{
#ifdef USE_ITHREADS
    dTHXo;
    /* if this is a pseudo-forked child, we just want to spawn
     * the new program, and return */
    if (w32_pseudo_id)
	return spawnv(P_WAIT, cmdname, (char *const *)argv);
#endif
    return execv(cmdname, (char *const *)argv);
}

DllExport int
win32_execvp(const char *cmdname, const char *const *argv)
{
#ifdef USE_ITHREADS
    dTHXo;
    /* if this is a pseudo-forked child, we just want to spawn
     * the new program, and return */
    if (w32_pseudo_id)
	return win32_spawnvp(P_WAIT, cmdname, (char *const *)argv);
#endif
    return execvp(cmdname, (char *const *)argv);
}

DllExport void
win32_perror(const char *str)
{
    perror(str);
}

DllExport void
win32_setbuf(FILE *pf, char *buf)
{
    setbuf(pf, buf);
}

DllExport int
win32_setvbuf(FILE *pf, char *buf, int type, size_t size)
{
    return setvbuf(pf, buf, type, size);
}

DllExport int
win32_flushall(void)
{
    return flushall();
}

DllExport int
win32_fcloseall(void)
{
    return fcloseall();
}

DllExport char*
win32_fgets(char *s, int n, FILE *pf)
{
    return fgets(s, n, pf);
}

DllExport char*
win32_gets(char *s)
{
    return gets(s);
}

DllExport int
win32_fgetc(FILE *pf)
{
    return fgetc(pf);
}

DllExport int
win32_putc(int c, FILE *pf)
{
    return putc(c,pf);
}

DllExport int
win32_puts(const char *s)
{
    return puts(s);
}

DllExport int
win32_getchar(void)
{
    return getchar();
}

DllExport int
win32_putchar(int c)
{
    return putchar(c);
}

#ifdef MYMALLOC

#ifndef USE_PERL_SBRK

static char *committed = NULL;
static char *base      = NULL;
static char *reserved  = NULL;
static char *brk       = NULL;
static DWORD pagesize  = 0;
static DWORD allocsize = 0;

void *
sbrk(int need)
{
 void *result;
 if (!pagesize)
  {SYSTEM_INFO info;
   GetSystemInfo(&info);
   /* Pretend page size is larger so we don't perpetually
    * call the OS to commit just one page ...
    */
   pagesize = info.dwPageSize << 3;
   allocsize = info.dwAllocationGranularity;
  }
 /* This scheme fails eventually if request for contiguous
  * block is denied so reserve big blocks - this is only 
  * address space not memory ...
  */
 if (brk+need >= reserved)
  {
   DWORD size = 64*1024*1024;
   char *addr;
   if (committed && reserved && committed < reserved)
    {
     /* Commit last of previous chunk cannot span allocations */
     addr = (char *) VirtualAlloc(committed,reserved-committed,MEM_COMMIT,PAGE_READWRITE);
     if (addr)
      committed = reserved;
    }
   /* Reserve some (more) space 
    * Note this is a little sneaky, 1st call passes NULL as reserved
    * so lets system choose where we start, subsequent calls pass
    * the old end address so ask for a contiguous block
    */
   addr  = (char *) VirtualAlloc(reserved,size,MEM_RESERVE,PAGE_NOACCESS);
   if (addr)
    {
     reserved = addr+size;
     if (!base)
      base = addr;
     if (!committed)
      committed = base;
     if (!brk)
      brk = committed;
    }
   else
    {
     return (void *) -1;
    }
  }
 result = brk;
 brk += need;
 if (brk > committed)
  {
   DWORD size = ((brk-committed + pagesize -1)/pagesize) * pagesize;
   char *addr = (char *) VirtualAlloc(committed,size,MEM_COMMIT,PAGE_READWRITE);
   if (addr)
    {
     committed += size;
    }
   else
    return (void *) -1;
  }
 return result;
}

#endif
#endif

DllExport void*
win32_malloc(size_t size)
{
    return malloc(size);
}

DllExport void*
win32_calloc(size_t numitems, size_t size)
{
    return calloc(numitems,size);
}

DllExport void*
win32_realloc(void *block, size_t size)
{
    return realloc(block,size);
}

DllExport void
win32_free(void *block)
{
    free(block);
}


int
win32_open_osfhandle(long handle, int flags)
{
#ifdef USE_FIXED_OSFHANDLE
    if (IsWin95())
	return my_open_osfhandle(handle, flags);
#endif
    return _open_osfhandle(handle, flags);
}

long
win32_get_osfhandle(int fd)
{
    return _get_osfhandle(fd);
}

DllExport void*
win32_dynaload(const char* filename)
{
    dTHXo;
    HMODULE hModule;
    char buf[MAX_PATH+1];
    char *first;

    /* LoadLibrary() doesn't recognize forward slashes correctly,
     * so turn 'em back. */
    first = strchr(filename, '/');
    if (first) {
	STRLEN len = strlen(filename);
	if (len <= MAX_PATH) {
	    strcpy(buf, filename);
	    filename = &buf[first - filename];
	    while (*filename) {
		if (*filename == '/')
		    *(char*)filename = '\\';
		++filename;
	    }
	    filename = buf;
	}
    }
    if (USING_WIDE()) {
	WCHAR wfilename[MAX_PATH+1];
	A2WHELPER(filename, wfilename, sizeof(wfilename));
	hModule = LoadLibraryExW(PerlDir_mapW(wfilename), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    }
    else {
	hModule = LoadLibraryExA(PerlDir_mapA(filename), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    }
    return hModule;
}

/*
 * Extras.
 */

static
XS(w32_GetCwd)
{
    dXSARGS;
    /* Make the host for current directory */
    char* ptr = PerlEnv_get_childdir();
    /* 
     * If ptr != Nullch 
     *   then it worked, set PV valid, 
     *   else return 'undef' 
     */
    if (ptr) {
	SV *sv = sv_newmortal();
	sv_setpv(sv, ptr);
	PerlEnv_free_childdir(ptr);

	EXTEND(SP,1);
	SvPOK_on(sv);
	ST(0) = sv;
	XSRETURN(1);
    }
    XSRETURN_UNDEF;
}

static
XS(w32_SetCwd)
{
    dXSARGS;
    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::SetCurrentDirectory($cwd)");
    if (!PerlDir_chdir(SvPV_nolen(ST(0))))
	XSRETURN_YES;

    XSRETURN_NO;
}

static
XS(w32_GetNextAvailDrive)
{
    dXSARGS;
    char ix = 'C';
    char root[] = "_:\\";

    EXTEND(SP,1);
    while (ix <= 'Z') {
	root[0] = ix++;
	if (GetDriveType(root) == 1) {
	    root[2] = '\0';
	    XSRETURN_PV(root);
	}
    }
    XSRETURN_UNDEF;
}

static
XS(w32_GetLastError)
{
    dXSARGS;
    EXTEND(SP,1);
    XSRETURN_IV(GetLastError());
}

static
XS(w32_SetLastError)
{
    dXSARGS;
    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::SetLastError($error)");
    SetLastError(SvIV(ST(0)));
    XSRETURN_EMPTY;
}

static
XS(w32_LoginName)
{
    dXSARGS;
    char *name = w32_getlogin_buffer;
    DWORD size = sizeof(w32_getlogin_buffer);
    EXTEND(SP,1);
    if (GetUserName(name,&size)) {
	/* size includes NULL */
	ST(0) = sv_2mortal(newSVpvn(name,size-1));
	XSRETURN(1);
    }
    XSRETURN_UNDEF;
}

static
XS(w32_NodeName)
{
    dXSARGS;
    char name[MAX_COMPUTERNAME_LENGTH+1];
    DWORD size = sizeof(name);
    EXTEND(SP,1);
    if (GetComputerName(name,&size)) {
	/* size does NOT include NULL :-( */
	ST(0) = sv_2mortal(newSVpvn(name,size));
	XSRETURN(1);
    }
    XSRETURN_UNDEF;
}


static
XS(w32_DomainName)
{
    dXSARGS;
    HINSTANCE hNetApi32 = LoadLibrary("netapi32.dll");
    DWORD (__stdcall *pfnNetApiBufferFree)(LPVOID Buffer);
    DWORD (__stdcall *pfnNetWkstaGetInfo)(LPWSTR servername, DWORD level,
					  void *bufptr);

    if (hNetApi32) {
	pfnNetApiBufferFree = (DWORD (__stdcall *)(void *))
	    GetProcAddress(hNetApi32, "NetApiBufferFree");
	pfnNetWkstaGetInfo = (DWORD (__stdcall *)(LPWSTR, DWORD, void *))
	    GetProcAddress(hNetApi32, "NetWkstaGetInfo");
    }
    EXTEND(SP,1);
    if (hNetApi32 && pfnNetWkstaGetInfo && pfnNetApiBufferFree) {
	/* this way is more reliable, in case user has a local account. */
	char dname[256];
	DWORD dnamelen = sizeof(dname);
	struct {
	    DWORD   wki100_platform_id;
	    LPWSTR  wki100_computername;
	    LPWSTR  wki100_langroup;
	    DWORD   wki100_ver_major;
	    DWORD   wki100_ver_minor;
	} *pwi;
	/* NERR_Success *is* 0*/
	if (0 == pfnNetWkstaGetInfo(NULL, 100, &pwi)) {
	    if (pwi->wki100_langroup && *(pwi->wki100_langroup)) {
		WideCharToMultiByte(CP_ACP, NULL, pwi->wki100_langroup,
				    -1, (LPSTR)dname, dnamelen, NULL, NULL);
	    }
	    else {
		WideCharToMultiByte(CP_ACP, NULL, pwi->wki100_computername,
				    -1, (LPSTR)dname, dnamelen, NULL, NULL);
	    }
	    pfnNetApiBufferFree(pwi);
	    FreeLibrary(hNetApi32);
	    XSRETURN_PV(dname);
	}
	FreeLibrary(hNetApi32);
    }
    else {
	/* Win95 doesn't have NetWksta*(), so do it the old way */
	char name[256];
	DWORD size = sizeof(name);
	if (hNetApi32)
	    FreeLibrary(hNetApi32);
	if (GetUserName(name,&size)) {
	    char sid[ONE_K_BUFSIZE];
	    DWORD sidlen = sizeof(sid);
	    char dname[256];
	    DWORD dnamelen = sizeof(dname);
	    SID_NAME_USE snu;
	    if (LookupAccountName(NULL, name, (PSID)&sid, &sidlen,
				  dname, &dnamelen, &snu)) {
		XSRETURN_PV(dname);		/* all that for this */
	    }
	}
    }
    XSRETURN_UNDEF;
}

static
XS(w32_FsType)
{
    dXSARGS;
    char fsname[256];
    DWORD flags, filecomplen;
    if (GetVolumeInformation(NULL, NULL, 0, NULL, &filecomplen,
			 &flags, fsname, sizeof(fsname))) {
	if (GIMME_V == G_ARRAY) {
	    XPUSHs(sv_2mortal(newSVpvn(fsname,strlen(fsname))));
	    XPUSHs(sv_2mortal(newSViv(flags)));
	    XPUSHs(sv_2mortal(newSViv(filecomplen)));
	    PUTBACK;
	    return;
	}
	EXTEND(SP,1);
	XSRETURN_PV(fsname);
    }
    XSRETURN_EMPTY;
}

static
XS(w32_GetOSVersion)
{
    dXSARGS;
    OSVERSIONINFOA osver;

    if (USING_WIDE()) {
	OSVERSIONINFOW osverw;
	char szCSDVersion[sizeof(osverw.szCSDVersion)];
	osverw.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
	if (!GetVersionExW(&osverw)) {
	    XSRETURN_EMPTY;
	}
	W2AHELPER(osverw.szCSDVersion, szCSDVersion, sizeof(szCSDVersion));
	XPUSHs(newSVpvn(szCSDVersion, strlen(szCSDVersion)));
	osver.dwMajorVersion = osverw.dwMajorVersion;
	osver.dwMinorVersion = osverw.dwMinorVersion;
	osver.dwBuildNumber = osverw.dwBuildNumber;
	osver.dwPlatformId = osverw.dwPlatformId;
    }
    else {
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	if (!GetVersionExA(&osver)) {
	    XSRETURN_EMPTY;
	}
	XPUSHs(newSVpvn(osver.szCSDVersion, strlen(osver.szCSDVersion)));
    }
    XPUSHs(newSViv(osver.dwMajorVersion));
    XPUSHs(newSViv(osver.dwMinorVersion));
    XPUSHs(newSViv(osver.dwBuildNumber));
    XPUSHs(newSViv(osver.dwPlatformId));
    PUTBACK;
}

static
XS(w32_IsWinNT)
{
    dXSARGS;
    EXTEND(SP,1);
    XSRETURN_IV(IsWinNT());
}

static
XS(w32_IsWin95)
{
    dXSARGS;
    EXTEND(SP,1);
    XSRETURN_IV(IsWin95());
}

static
XS(w32_FormatMessage)
{
    dXSARGS;
    DWORD source = 0;
    char msgbuf[ONE_K_BUFSIZE];

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::FormatMessage($errno)");

    if (USING_WIDE()) {
	WCHAR wmsgbuf[ONE_K_BUFSIZE];
	if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM,
			  &source, SvIV(ST(0)), 0,
			  wmsgbuf, ONE_K_BUFSIZE-1, NULL))
	{
	    W2AHELPER(wmsgbuf, msgbuf, sizeof(msgbuf));
	    XSRETURN_PV(msgbuf);
	}
    }
    else {
	if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
			  &source, SvIV(ST(0)), 0,
			  msgbuf, sizeof(msgbuf)-1, NULL))
	    XSRETURN_PV(msgbuf);
    }

    XSRETURN_UNDEF;
}

static
XS(w32_Spawn)
{
    dXSARGS;
    char *cmd, *args;
    PROCESS_INFORMATION stProcInfo;
    STARTUPINFO stStartInfo;
    BOOL bSuccess = FALSE;

    if (items != 3)
	Perl_croak(aTHX_ "usage: Win32::Spawn($cmdName, $args, $PID)");

    cmd = SvPV_nolen(ST(0));
    args = SvPV_nolen(ST(1));

    memset(&stStartInfo, 0, sizeof(stStartInfo));   /* Clear the block */
    stStartInfo.cb = sizeof(stStartInfo);	    /* Set the structure size */
    stStartInfo.dwFlags = STARTF_USESHOWWINDOW;	    /* Enable wShowWindow control */
    stStartInfo.wShowWindow = SW_SHOWMINNOACTIVE;   /* Start min (normal) */

    if (CreateProcess(
		cmd,			/* Image path */
		args,	 		/* Arguments for command line */
		NULL,			/* Default process security */
		NULL,			/* Default thread security */
		FALSE,			/* Must be TRUE to use std handles */
		NORMAL_PRIORITY_CLASS,	/* No special scheduling */
		NULL,			/* Inherit our environment block */
		NULL,			/* Inherit our currrent directory */
		&stStartInfo,		/* -> Startup info */
		&stProcInfo))		/* <- Process info (if OK) */
    {
	int pid = (int)stProcInfo.dwProcessId;
	if (IsWin95() && pid < 0)
	    pid = -pid;
	sv_setiv(ST(2), pid);
	CloseHandle(stProcInfo.hThread);/* library source code does this. */
	bSuccess = TRUE;
    }
    XSRETURN_IV(bSuccess);
}

static
XS(w32_GetTickCount)
{
    dXSARGS;
    DWORD msec = GetTickCount();
    EXTEND(SP,1);
    if ((IV)msec > 0)
	XSRETURN_IV(msec);
    XSRETURN_NV(msec);
}

static
XS(w32_GetShortPathName)
{
    dXSARGS;
    SV *shortpath;
    DWORD len;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::GetShortPathName($longPathName)");

    shortpath = sv_mortalcopy(ST(0));
    SvUPGRADE(shortpath, SVt_PV);
    if (!SvPVX(shortpath) || !SvLEN(shortpath))
        XSRETURN_UNDEF;

    /* src == target is allowed */
    do {
	len = GetShortPathName(SvPVX(shortpath),
			       SvPVX(shortpath),
			       SvLEN(shortpath));
    } while (len >= SvLEN(shortpath) && sv_grow(shortpath,len+1));
    if (len) {
	SvCUR_set(shortpath,len);
	ST(0) = shortpath;
	XSRETURN(1);
    }
    XSRETURN_UNDEF;
}

static
XS(w32_GetFullPathName)
{
    dXSARGS;
    SV *filename;
    SV *fullpath;
    char *filepart;
    DWORD len;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::GetFullPathName($filename)");

    filename = ST(0);
    fullpath = sv_mortalcopy(filename);
    SvUPGRADE(fullpath, SVt_PV);
    if (!SvPVX(fullpath) || !SvLEN(fullpath))
        XSRETURN_UNDEF;

    do {
	len = GetFullPathName(SvPVX(filename),
			      SvLEN(fullpath),
			      SvPVX(fullpath),
			      &filepart);
    } while (len >= SvLEN(fullpath) && sv_grow(fullpath,len+1));
    if (len) {
	if (GIMME_V == G_ARRAY) {
	    EXTEND(SP,1);
	    XST_mPV(1,filepart);
	    len = filepart - SvPVX(fullpath);
	    items = 2;
	}
	SvCUR_set(fullpath,len);
	ST(0) = fullpath;
	XSRETURN(items);
    }
    XSRETURN_EMPTY;
}

static
XS(w32_GetLongPathName)
{
    dXSARGS;
    SV *path;
    char tmpbuf[MAX_PATH+1];
    char *pathstr;
    STRLEN len;

    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::GetLongPathName($pathname)");

    path = ST(0);
    pathstr = SvPV(path,len);
    strcpy(tmpbuf, pathstr);
    pathstr = win32_longpath(tmpbuf);
    if (pathstr) {
	ST(0) = sv_2mortal(newSVpvn(pathstr, strlen(pathstr)));
	XSRETURN(1);
    }
    XSRETURN_EMPTY;
}

static
XS(w32_Sleep)
{
    dXSARGS;
    if (items != 1)
	Perl_croak(aTHX_ "usage: Win32::Sleep($milliseconds)");
    Sleep(SvIV(ST(0)));
    XSRETURN_YES;
}

static
XS(w32_CopyFile)
{
    dXSARGS;
    BOOL bResult;
    if (items != 3)
	Perl_croak(aTHX_ "usage: Win32::CopyFile($from, $to, $overwrite)");
    if (USING_WIDE()) {
	WCHAR wSourceFile[MAX_PATH+1];
	WCHAR wDestFile[MAX_PATH+1];
	A2WHELPER(SvPV_nolen(ST(0)), wSourceFile, sizeof(wSourceFile));
	wcscpy(wSourceFile, PerlDir_mapW(wSourceFile));
	A2WHELPER(SvPV_nolen(ST(1)), wDestFile, sizeof(wDestFile));
	bResult = CopyFileW(wSourceFile, PerlDir_mapW(wDestFile), !SvTRUE(ST(2)));
    }
    else {
	char szSourceFile[MAX_PATH+1];
	strcpy(szSourceFile, PerlDir_mapA(SvPV_nolen(ST(0))));
	bResult = CopyFileA(szSourceFile, PerlDir_mapA(SvPV_nolen(ST(1))), !SvTRUE(ST(2)));
    }

    if (bResult)
	XSRETURN_YES;
    XSRETURN_NO;
}

void
Perl_init_os_extras(void)
{
    dTHXo;
    char *file = __FILE__;
    dXSUB_SYS;

    /* these names are Activeware compatible */
    newXS("Win32::GetCwd", w32_GetCwd, file);
    newXS("Win32::SetCwd", w32_SetCwd, file);
    newXS("Win32::GetNextAvailDrive", w32_GetNextAvailDrive, file);
    newXS("Win32::GetLastError", w32_GetLastError, file);
    newXS("Win32::SetLastError", w32_SetLastError, file);
    newXS("Win32::LoginName", w32_LoginName, file);
    newXS("Win32::NodeName", w32_NodeName, file);
    newXS("Win32::DomainName", w32_DomainName, file);
    newXS("Win32::FsType", w32_FsType, file);
    newXS("Win32::GetOSVersion", w32_GetOSVersion, file);
    newXS("Win32::IsWinNT", w32_IsWinNT, file);
    newXS("Win32::IsWin95", w32_IsWin95, file);
    newXS("Win32::FormatMessage", w32_FormatMessage, file);
    newXS("Win32::Spawn", w32_Spawn, file);
    newXS("Win32::GetTickCount", w32_GetTickCount, file);
    newXS("Win32::GetShortPathName", w32_GetShortPathName, file);
    newXS("Win32::GetFullPathName", w32_GetFullPathName, file);
    newXS("Win32::GetLongPathName", w32_GetLongPathName, file);
    newXS("Win32::CopyFile", w32_CopyFile, file);
    newXS("Win32::Sleep", w32_Sleep, file);

    /* XXX Bloat Alert! The following Activeware preloads really
     * ought to be part of Win32::Sys::*, so they're not included
     * here.
     */
    /* LookupAccountName
     * LookupAccountSID
     * InitiateSystemShutdown
     * AbortSystemShutdown
     * ExpandEnvrironmentStrings
     */
}

void
Perl_win32_init(int *argcp, char ***argvp)
{
    /* Disable floating point errors, Perl will trap the ones we
     * care about.  VC++ RTL defaults to switching these off
     * already, but the Borland RTL doesn't.  Since we don't
     * want to be at the vendor's whim on the default, we set
     * it explicitly here.
     */
#if !defined(_ALPHA_) && !defined(__GNUC__)
    _control87(MCW_EM, MCW_EM);
#endif
    MALLOC_INIT;
}

void
win32_get_child_IO(child_IO_table* ptbl)
{
    ptbl->childStdIn	= GetStdHandle(STD_INPUT_HANDLE);
    ptbl->childStdOut	= GetStdHandle(STD_OUTPUT_HANDLE);
    ptbl->childStdErr	= GetStdHandle(STD_ERROR_HANDLE);
}

#ifdef HAVE_INTERP_INTERN

#  ifdef PERL_OBJECT
#    undef Perl_sys_intern_init
#    define Perl_sys_intern_init CPerlObj::Perl_sys_intern_init
#    undef Perl_sys_intern_dup
#    define Perl_sys_intern_dup CPerlObj::Perl_sys_intern_dup
#    undef Perl_sys_intern_clear
#    define Perl_sys_intern_clear CPerlObj::Perl_sys_intern_clear
#    define pPerl this
#  endif

void
Perl_sys_intern_init(pTHX)
{
    w32_perlshell_tokens	= Nullch;
    w32_perlshell_vec		= (char**)NULL;
    w32_perlshell_items		= 0;
    w32_fdpid			= newAV();
    New(1313, w32_children, 1, child_tab);
    w32_num_children		= 0;
#  ifdef USE_ITHREADS
    w32_pseudo_id		= 0;
    New(1313, w32_pseudo_children, 1, child_tab);
    w32_num_pseudo_children	= 0;
#  endif
    w32_init_socktype		= 0;
}

void
Perl_sys_intern_clear(pTHX)
{
    Safefree(w32_perlshell_tokens);
    Safefree(w32_perlshell_vec);
    /* NOTE: w32_fdpid is freed by sv_clean_all() */
    Safefree(w32_children);
#  ifdef USE_ITHREADS
    Safefree(w32_pseudo_children);
#  endif
}

#  ifdef USE_ITHREADS

void
Perl_sys_intern_dup(pTHX_ struct interp_intern *src, struct interp_intern *dst)
{
    dst->perlshell_tokens	= Nullch;
    dst->perlshell_vec		= (char**)NULL;
    dst->perlshell_items	= 0;
    dst->fdpid			= newAV();
    Newz(1313, dst->children, 1, child_tab);
    dst->pseudo_id		= 0;
    Newz(1313, dst->pseudo_children, 1, child_tab);
    dst->thr_intern.Winit_socktype = 0;
}
#  endif /* USE_ITHREADS */
#endif /* HAVE_INTERP_INTERN */

#ifdef PERL_OBJECT
#  undef this
#  define this pPerl
#endif

static void
win32_free_argvw(pTHXo_ void *ptr)
{
    char** argv = (char**)ptr;
    while(*argv) {
	Safefree(*argv);
	*argv++ = Nullch;
    }
}

void
win32_argv2utf8(int argc, char** argv)
{
    dTHXo;
    char* psz;
    int length, wargc;
    LPWSTR* lpwStr = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if (lpwStr && argc) {
	while (argc--) {
	    length = WideCharToMultiByte(CP_UTF8, 0, lpwStr[--wargc], -1, NULL, 0, NULL, NULL);
	    Newz(0, psz, length, char);
	    WideCharToMultiByte(CP_UTF8, 0, lpwStr[wargc], -1, psz, length, NULL, NULL);
	    argv[argc] = psz;
	}
	call_atexit(win32_free_argvw, argv);
    }
    GlobalFree((HGLOBAL)lpwStr);
}

