/* Virtual File System switch code
   Copyright (C) 1995 The Free Software Foundation
   
   Written by: 1995 Miguel de Icaza
               1995 Jakub Jelinek
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>	/* For atol() */
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <malloc.h>
#include <fcntl.h>
#include <signal.h>
#ifdef SCO_FLAVOR
#include <sys/timeb.h>	/* alex: for struct timeb definition */
#endif /* SCO_FLAVOR */
#include <time.h>
#include <sys/time.h>
#include "../src/fs.h"
#include "../src/mad.h"
#include "../src/dir.h"
#include "../src/util.h"
#include "../src/main.h"
#include "../src/panel.h"
#include "../src/key.h"		/* Required for the async alarm handler */
#include "../src/layout.h"	/* For get_panel_widget and get_other_index */
#include "vfs.h"
#include "mcfs.h"
#include "names.h"
#include "extfs.h"
#ifdef USE_NETCODE
#   include "tcputil.h"
#endif

extern int get_other_type (void);

int vfs_timeout = 60; /* VFS timeout in seconds */

extern int cd_symlinks; /* Defined in main.c */

/* They keep track of the current directory */
static vfs *current_vfs = &local_vfs_ops;
char *current_dir = NULL;
char *last_current_dir = NULL;

static int current_mon;
static int current_year;

/* Open files managed by the vfs layer */
#define MAX_VFS_FILES 100
static struct {
    void *fs_info;
    vfs  *operations;
} vfs_file_table [MAX_VFS_FILES];

static int get_bucket ()
{
    int i;

    /* 0, 1, 2 are reserved file descriptors, while (DIR *) 0 means error */
    for (i = 3; i < MAX_VFS_FILES; i++){
	if (!vfs_file_table [i].fs_info)
	    return i;
    }
    fprintf (stderr, "No more virtual file handles\n");
    exit (1);
}

/* This flag is set on each vfs_type call to say if the path was
   absolute (1) or relative (0) to current dir. */

int vfs_type_absolute = 0;

vfs *vfs_type (char *path)
{
    vfs *vfs;

    vfs_type_absolute = 0;
    vfs = current_vfs;
    
    if (*path == PATH_SEP) {
    	vfs = &local_vfs_ops;
    	vfs_type_absolute = 1;
    }
    
    if (strncmp (path, "local:", 6) == 0){
	vfs = &local_vfs_ops;
	vfs_type_absolute = 1;
    }

#ifdef USE_NETCODE
    if (strncmp (path, "mc:", 3) == 0){
	vfs = &mcfs_vfs_ops;
	vfs_type_absolute = 1;
    }
    
    if (strncmp (path, "ftp://", 6) == 0){
        vfs = &ftpfs_vfs_ops;
        vfs_type_absolute = 1;
    }
#endif

#ifdef USE_EXT2FSLIB
    if (strncmp (path, "undel:", 6) == 0){
	vfs = &undelfs_vfs_ops;
	vfs_type_absolute = 1;
    }
#endif
    if (tarfs_is_tar(path)){ // Change. --Hatred
    	vfs = &tarfs_vfs_ops;
	vfs_type_absolute = 1;
    }
    
    if (extfs_prefix_to_type (path) != -1) {
        vfs = &extfs_vfs_ops;
        vfs_type_absolute = 1;
    }
    return vfs;
}

static struct vfs_stamping *stamps;

/* Returns the number of seconds remaining to the vfs timeout
 *
 * FIXME: currently this is set to 10 seconds.  We should compute this.
 */
int vfs_timeouts ()
{
    return stamps ? 10 : 0;
}

void vfs_addstamp (vfs *v, vfsid id, struct vfs_stamping *parent)
{
    if (v != &local_vfs_ops && id != (vfsid)-1) {
        struct vfs_stamping *stamp, *st1;
        
        for (stamp = stamps; stamp != NULL; st1 = stamp, stamp = stamp->next)
            if (stamp->v == v && stamp->id == id) {
		gettimeofday(&(stamp->time), NULL);
                return;
	    }
        stamp = xmalloc (sizeof (struct vfs_stamping), "vfs stamping");
        stamp->v = v;
        stamp->id = id;
	if (parent) {
	    struct vfs_stamping *st = stamp;
	    for ( ; parent; ) {
		st->parent = xmalloc (sizeof (struct vfs_stamping), "vfs stamping");
		*st->parent = *parent;
		parent = parent->parent;
		st = st->parent;
	    }
	    st->parent = 0;
	}
	else
    	    stamp->parent = 0;
        gettimeofday (&(stamp->time), NULL);
        stamp->next = 0;
	if (stamps)
	    st1->next = stamp;
	else
    	    stamps = stamp;
    }
}

void vfs_stamp (vfs *v, vfsid id)
{
    struct vfs_stamping *stamp;
    
    for (stamp = stamps; stamp != NULL; stamp = stamp->next)
        if (stamp->v == v && stamp->id == id) {
            gettimeofday (&(stamp->time), NULL);
            if (stamp->parent != NULL)
                vfs_stamp (stamp->parent->v, stamp->parent->id);
            return;
        }
}

void vfs_rm_parents (struct vfs_stamping *stamp)
{
    struct vfs_stamping *st2, *st3;
    if (stamp) {
	for (st2 = stamp, st3 = st2->parent; st3 != NULL; st2 = st3, st3 = st3->parent)
	    free (st2);
	free (st2);
    }
}

void vfs_rmstamp (vfs *v, vfsid id, int removeparents)
{
    struct vfs_stamping *stamp, *st1;
    
    for (stamp = stamps, st1 = NULL; stamp != NULL; st1 = stamp, stamp = stamp->next)
        if (stamp->v == v && stamp->id == id) {
            if (stamp->parent != NULL) {
                if (removeparents)
                    vfs_rmstamp (stamp->parent->v, stamp->parent->id, 1);
		vfs_rm_parents (stamp->parent);
            }
            if (st1 == NULL) {
                stamps = stamp->next;
            } else {
            	st1->next = stamp->next;
            }
            free (stamp);
            return;
        }
}

int mc_open (char *file, int flags, ...)
{
    int  handle;
    int  mode;
    void *info;
    vfs  *vfs;
    va_list ap;
    
    file = vfs_canon (file);
    vfs = vfs_type (file);

    /* Get the mode flag */
    va_start (ap, flags);
    mode = va_arg (ap, int);
    va_end (ap);
    
    info = (*vfs->open) (file, flags, mode);
    if (!info){
	errno = (*vfs->ferrno)();
	free (file);
	return -1;
    }
    handle = get_bucket ();
    vfs_file_table [handle].fs_info = info;
    vfs_file_table [handle].operations = vfs;
    
    free (file);
    return handle;
}

#define vfs_op(handle) vfs_file_table [handle].operations 
#define vfs_info(handle) vfs_file_table [handle].fs_info
#define vfs_free_bucket(handle) vfs_info(handle) = 0;

int mc_read (int handle, char *buffer, int count)
{
    vfs *vfs;
    int result;

    if (handle == -1)
	return -1;
	    
    vfs = vfs_op (handle);
    result = (*vfs->read)(vfs_info (handle), buffer, count);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_ctl (int handle, int ctlop, int arg)
{
    vfs *vfs;
    int result;

    vfs = vfs_op (handle);
    if (vfs->ctl == NULL)
        return 0;
    result = (*vfs->ctl)(vfs_info (handle), ctlop, arg);
    return result;
}

int mc_setctl (char *path, int ctlop, char *arg)
{
    vfs *vfs;
    int result;

    path = vfs_canon (path);
    vfs = vfs_type (path);    
    if (vfs->setctl == NULL) {
        free (path);
        return 0;
    }
    result = (*vfs->setctl)(path, ctlop, arg);
    free (path);
    return result;
}

int mc_close (int handle)
{
    vfs *vfs;
    int result;

    if (handle == -1 || !vfs_info (handle))
	return -1;
    
    vfs = vfs_op (handle);
    if (handle < 3)
	return close (handle);

    result = (*vfs->close)(vfs_info (handle));
    vfs_free_bucket (handle);
    if (result == -1)
	errno = (*vfs->ferrno)();
    
    return result;
}

DIR *mc_opendir (char *dirname)
{
    int  handle, *handlep;
    void *info;
    vfs  *vfs;
    char *p = NULL;
    int i = strlen (dirname);

    if (dirname [i - 1] != PATH_SEP) { 
    /* We should make possible reading of the root directory in a tar file */
        p = xmalloc (i + 2, "slash");
        strcpy (p, dirname);
        strcpy (p + i, PATH_SEP_STR);
        dirname = p;
    }
    dirname = vfs_canon (dirname);
    vfs = vfs_type (dirname);

    info = (*vfs->opendir)(dirname);
    if (!info){
	errno = (*vfs->ferrno)();
	free (dirname);
	if (p)
	    free (p);
	return NULL;
    }
    handle = get_bucket ();
    vfs_file_table [handle].fs_info = info;
    vfs_file_table [handle].operations = vfs;

    free (dirname);
    if (p)
        free (p);
    
    handlep = (int *) xmalloc (sizeof (int), "opendir handle");
    *handlep = handle;
    return (DIR *) handlep;
}

/* This should strip the non needed part of a path name */
#define vfs_name(x) x

struct dirent *mc_readdir(DIR *dirp)
{
    int handle;
    vfs *vfs;

    if (!dirp){
#ifdef EBADF
	errno = EBADF;
#else
	errno = 1;
#endif
	return NULL;
    }
    handle = *(int *) dirp;
    vfs = vfs_op (handle);
    return (*vfs->readdir)(vfs_info (handle));
}

int mc_closedir (DIR *dirp)
{
    int handle = *(int *) dirp;
    vfs *vfs = vfs_op (handle);
    int result;

    result = (*vfs->closedir)(vfs_info (handle));
    vfs_free_bucket (handle);
    free (dirp);
    return result; 
}

int mc_stat (char *path, struct stat *buf)
{
    vfs *vfs;
    int result;

    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->stat)(vfs_name (path), buf);
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_lstat (char *path, struct stat *buf)
{
    vfs *vfs;
    int result;

    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->lstat)(vfs_name (path), buf);
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_fstat (int handle, struct stat *buf)
{
    vfs *vfs;
    int result;

    if (handle == -1)
	return -1;
    
    vfs = vfs_op (handle);
    result = (*vfs->fstat)(vfs_info (handle), buf);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

/* FIXME: We should return the buffer, not set it, since it could
   overflow */

/*
char *mc_get_current_wd (char *buffer, int size)
{
    char *p;
    struct stat my_stat, my_stat2;
    
    if (current_vfs == &local_vfs_ops){
// - check it more carefully !!! --Olegarch
//#ifdef HAVE_GETWD
//	p = (char *) getwd (buffer);
//#else
	p = getcwd (buffer, size);
//#endif
	if (!p) {
	    strcpy (buffer, current_dir);
	    return buffer;
	}

	mc_stat (buffer, &my_stat);
	mc_stat (current_dir, &my_stat2);
	if (my_stat.st_ino != my_stat2.st_ino ||
	    my_stat.st_dev != my_stat2.st_dev ||
	    !cd_symlinks) {
	    if (last_current_dir != current_dir)
		free (last_current_dir);
	    free (current_dir);
	    current_dir = strdup (p);
	    last_current_dir = current_dir;
	    return p;
	}
    } 
    strcpy (buffer, current_dir);
    return buffer;
}
*/

char *get_current_dir (void)
{
    char *buffer;
    char *dir;

    buffer = (char *) xmalloc (sizeof(char)* MC_MAXPATHLEN, "Extfs: buffer");
    *buffer = 0;
  
    /* We don't use getcwd(3) on SUNOS, because, it does a popen("pwd")
    * and, if that wasn't bad enough, hangs in doing so.
    */
#if	defined (sun) && !defined (__SVR4)
    dir = getwd (buffer);
#else	/* !sun */
//    dir = getcwd (buffer, G_PATH_LENGTH - 1);
    dir = getcwd (buffer, MC_MAXPATHLEN - 1);
#endif	/* !sun */
  
    if (!dir || !*buffer)
    {
	/* hm, should we g_error() out here?
	* this can happen if e.g. "./" has mode \0000
	*/
      buffer[0] = PATH_SEP;
//      buffer[0] = G_DIR_SEPARATOR;
      buffer[1] = 0;
    }

    dir = strdup (buffer);
    free (buffer);
  
    return dir;
}

/* FIXME: We should return the buffer, not set it, since it could
   overflow */

const char *mc_return_cwd ()
{
    char *p;
    struct stat my_stat, my_stat2;
    
    if (!vfs_canon (current_dir)){
//    if (!vfs_rosplit (current_dir)){
	p = get_current_dir ();
	if (!p)  /* One of the directories in the path is not readable */
	    return current_dir;

	/* Otherwise check if it is O.K. to use the current_dir */
	if (!cd_symlinks ||
	    mc_stat (p, &my_stat) || 
	    mc_stat (current_dir, &my_stat2) ||
	    my_stat.st_ino != my_stat2.st_ino ||
	    my_stat.st_dev != my_stat2.st_dev){
	    free (current_dir);
	    current_dir = p;

	    return p;
	} /* Otherwise we return current_dir below */
	free (p);
    } 
    return current_dir;
}


char *mc_get_current_wd (char *buffer, int size)
 {
    const char *cwd = (const char *)mc_return_cwd();

    strncpy (buffer, cwd, size - 1);
    buffer [size - 1] = 0;
    return buffer;
 }



int mc_chmod (char *path, int mode)
{
    vfs *vfs;
    int result;
    
    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->chmod)(vfs_name (path), mode);
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_chown (char *path, int owner, int group)
{
    vfs *vfs;
    int result;
    
    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->chown)(vfs_name (path), owner, group);
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_utime (char *path, struct utimbuf *times)
{
    vfs *vfs;
    int result;
    
    path = vfs_canon (path);
    vfs = vfs_type (path);    
    if (!vfs->utime) {
	free (path);
	return 0;
    }
    result = (*vfs->utime)(vfs_name (path), times);
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_readlink(char *path, char *buf, int bufsiz)
{
    vfs *vfs;
    int result;
    
    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->readlink)(vfs_name (path), buf, bufsiz);
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}
/*
int mc_unlink (char *path)
{
    vfs *vfs;
    int result;
    
    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->unlink)(vfs_name (path));
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}
*/
/* 
   int mc_unlink (char *pathName)
   For Windows 95 and NT, files should be able to be deleted even
   if they don't have write-protection. We should build a question box
   like: Delete anyway? Yes <No> All
*/
int mc_unlink (char *path)
{
    vfs *vfs;
    int result;
    
    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->unlink)(vfs_name (path));
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}


int mc_symlink (char *name1, char *name2)
{
    vfs *vfs2 = 0;
    int result;

    name2 = vfs_canon (name2);
    vfs2 = vfs_type (name2);    
    
    result = (*vfs2->symlink)(vfs_name (name1), vfs_name (name2));
    free (name2);
    if (result == -1){
	errno = (*vfs2->ferrno)();
	return -1;
    }
    return result;
}

int mc_link (char *name1, char *name2)
{
    vfs *vfs1;
    vfs *vfs2;
    int result;

    name1 = vfs_canon (name1);
    vfs1 = vfs_type (name1);    
    name2 = vfs_canon (name2);
    vfs2 = vfs_type (name2);    
    
    if (vfs1 != vfs2){
    	errno = EXDEV;
    	free (name1);
    	free (name2);
	return -1;
    }

    result = (*vfs1->link)(vfs_name (name1), vfs_name (name2));
    free (name1);
    free (name2);
    if (result == -1){
	errno = (*vfs1->ferrno)();
	return -1;
    }
    return result;
}

int mc_write (int fd, char *buf, int nbyte)
{
    vfs *vfs; 
    int result;

    if (fd == -1)
	return -1;
    
    vfs = vfs_op (fd);
    result = (*vfs->write)(vfs_info (fd), buf, nbyte);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_rename (char *path1, char *path2)
{
    vfs *vfs1;
    vfs *vfs2;
    int result;    

    path1 = vfs_canon (path1);
    vfs1 = vfs_type (path1);    
    path2 = vfs_canon (path2);
    vfs2 = vfs_type (path2);    
    
    if (vfs1 != vfs2){
	errno = EXDEV;
	free (path1);
	free (path2);
	return -1;
    }

    result = (*vfs1->rename)(vfs_name (path1), vfs_name (path2));
    free (path1);
    free (path2);
    if (result == -1){
	errno = (*vfs1->ferrno)();
	return -1;
    }
    return result;
}

off_t mc_lseek (int fd, off_t offset, int whence)
{
    vfs *vfs;

    if (fd == -1)
	return -1;

    vfs = vfs_op (fd);
    return (*vfs->lseek)(vfs_info (fd), offset, whence);
}

int is_special_prefix (char *path)
{
    if (strncmp (path, "mc:", 3) == 0)
	return 1;
    if (strncmp (path, "local:", 6) == 0)
	return 1;
    if (tarfs_is_tar(path)) // Change. --Hatred
	return 1;
    if (extfs_prefix_to_type (path) != -1)
        return 1;
    if (strncmp (path, "ftp://", 6) == 0)
        return 1;
    if (strncmp (path, "undel:", 6) == 0)
	return 1;
    return 0;
}

char *vfs_canon (char *path)
{
    vfs *vfs;

    vfs = current_vfs;
    
    if (strncmp (path, "local:", 7) == 0) {
	return vfs_canon (path+6);
	
#ifdef USE_NETCODE
    } else if (strncmp (path, "mc:", 3) == 0) {
	vfs = &mcfs_vfs_ops;
    } else if (strncmp (path, "ftp://", 6) == 0) {
    	vfs = &ftpfs_vfs_ops;
#endif
#ifdef USE_EXT2FSLIB
    } else if (strncmp (path, "undel:", 6) == 0){
	vfs = &undelfs_vfs_ops;
#endif
    } else if (tarfs_is_tar(path)) { // Change. --Hatred
    	vfs = &tarfs_vfs_ops;
    } else if (extfs_prefix_to_type (path) != -1) {
        vfs = &extfs_vfs_ops;
    } else { 
	switch (*path) {
	    case PATH_SEP: { /* Absolute local */
    		vfs = &local_vfs_ops;
		break;
	    }
	    case '~': { /* Tilde expansion */
    		char *local, *result;

    		local = tilde_expand (path);
		if (local){
		    result = vfs_canon (local);
		    free (local);
		    return result;
		} 
	    }
	    default: { /* Relative to current directory */
    		char *local, *result;

		if (current_dir [strlen (current_dir) - 1] == PATH_SEP)
		    local = copy_strings (current_dir, path, NULL);
		else
		    local = copy_strings (current_dir, PATH_SEP_STR, path, NULL);

		result = vfs_canon (local);
		free (local);
		return result;
	    }
	} /* switch (*path) */
    }

    if (vfs == &local_vfs_ops)
/* NOTE: No local:/ is necessary at the moment, we are handling absolute paths
         at the moment, even it is impossible, since it doesn't work then at all. */    
	return strdup (canonicalize_pathname (path));
#ifdef USE_EXT2FSLIB
    if (vfs == &undelfs_vfs_ops) {
	int i = strlen (path);
	if ((path[i - 1] == '.' && path[i - 2] == '.') ||
	(path[i - 1] == PATH_SEP && path[i - 2] == '.' && path[i - 3] == '.'))
	    return strdup (PATH_SEP_STR); /* we want to go out of the undelfs ->
	                            go to the root directory */
	else
	    return strdup (path);
    }
#endif
#ifdef USE_NETCODE
    if (vfs == &ftpfs_vfs_ops || vfs == &mcfs_vfs_ops) {
	char *q, *p, *r, *s;
	int prefixsh, isftp;
	
	if (vfs == &ftpfs_vfs_ops) {
	    prefixsh = 6;
	    isftp = 1;
	} else {
	    if (path[3] == PATH_SEP && path[4] == PATH_SEP)
		prefixsh = 5;
	    else
		prefixsh = 3;
	    isftp = 0;
	}
	
	q = strdup (path);
	p = strchr (q + prefixsh, PATH_SEP);
	
	if (p != NULL) {
	    if (p [0] && p [1] == '~') { /* Tilde expansion */
	    	*p = 0;
	    	if (isftp) {
	            s = ftpfs_gethome (q);
	            if (s == NULL)
	                s = PATH_SEP_STR;
	        } else {
	            s = mcfs_gethome (q);
	            if (s == NULL)
	                s = strdup (PATH_SEP_STR);
	        }
	        if (p [2] == PATH_SEP)
	            p += 3;
	        else
	            p += 2;
	        r = copy_strings (q, s, p, NULL);
	        if (!isftp)
	            free (s);
	        free (q);
	        q = r;
	        p = strchr (q + prefixsh, PATH_SEP);
	    }
	    p++;
	    if (p [0])
		canonicalize_pathname (p);
	    else
		p = ".";
	    
	    if (!strncmp (p, "..", 2) && (p [2] == PATH_SEP || !p [2])) {
	    	*p = 0;
	        r = isftp ? ftpfs_getupdir (q) : mcfs_getupdir (q);
	        if (r == NULL)
	            r = PATH_SEP_STR;
	        p += 2;
	        if (*p == PATH_SEP)
	            p++;
	        r = copy_strings (r, p, NULL);
	        free (q);
	        q = vfs_canon (r);
	        free (r);
	    }
	}
	return q;
    }
#endif
    if (vfs == &tarfs_vfs_ops || vfs == &extfs_vfs_ops) {
        char *local_path, *arc_name, *p, *q, *result, *extfs_prefix = NULL;
        int istar, extfs_type;
        
        /* There has to be a leading tar: or extfs prefix in the path */

	if (vfs == &tarfs_vfs_ops)
	    istar = 1;
	else
	    istar = 0;
	
    /* The original code does not compile right in NeXTStep:
	 * istar = (vfs == &tarfs_vfs_ops);
	 */
        if (istar)
            local_path = tarfs_analysis (path, &arc_name, 0);
        else {
            local_path = extfs_analysis (path, &arc_name, &extfs_type, 0);
            extfs_prefix = extfs_get_prefix (extfs_type);
        }
        if (local_path == NULL) {
            return strdup (PATH_SEP_STR); /* For error cases cd to the root dir */
        }
        p = (*local_path == PATH_SEP) ? local_path + 1 : local_path;
        if (*p)
            canonicalize_pathname (p);
        if (!strcmp (p, ".")) /* This is an incorrect result of canonicalization,
                                 if we have p = somedir/.. */
	    *p = 0;                                 
        if (p [0] != '.' || p [1] != '.' || (p [2] && p [2] != PATH_SEP)) { 
            /* I.e. p doesn't start with the .. directory */
            if (istar)
	        //result = copy_strings ("tar:", arc_name, "/", p, 0);
			result = copy_strings (arc_name, "#tar", PATH_SEP_STR, p, 0);
	    else {
		/* Two chances: the extfs requires an archive to
		 * use or it does not need it:
		 */

		/* If it does need an archive... */
		if (arc_name && *arc_name)
		{
			result = copy_strings(arc_name, "#", extfs_prefix, PATH_SEP_STR, p, 0);
		}
		else 
		    result = canonicalize_pathname (strdup(path));
	    }
	    free (local_path);
	    if (arc_name)
    	        free (arc_name);
	    return result;
	} else {
	    /* We want to move outside of the archive */
	    if (!istar && (!arc_name || !*arc_name)) {
	        free (local_path);
	        if (arc_name) free (arc_name);
	        return strdup (PATH_SEP_STR);
	    }
	    q = strrchr (arc_name, PATH_SEP);
	    if (q == NULL)
	    	q = arc_name; /* Should not happen */
	    else
	    	q++; 
	    *q = 0;
	    if (p [2] == PATH_SEP)
	    	p += 3;
	    else
	    	p += 2;
	    result = copy_strings (arc_name, p, 0);
	    free (local_path);
	    free (arc_name);
	    local_path = vfs_canon (result);
	    free (result);
	    return local_path;
	}
    }

    fprintf (stderr, "Could not happend\n");
    return 0;
}

vfsid vfs_ncs_getid (vfs *nvfs, char *dir, struct vfs_stamping **par)
{
    vfsid nvfsid;
    int freeit = 0;

    if (dir [strlen (dir) - 1] != PATH_SEP) {
        dir = copy_strings (dir, PATH_SEP_STR, NULL);    
        freeit = 1;
    }
    nvfsid = (*nvfs->getid)(dir, par);
    if (freeit)
        free (dir);
    return nvfsid;
}

static int is_parent (vfs * nvfs, vfsid nvfsid, struct vfs_stamping *parent)
{
    struct vfs_stamping *stamp;
    for (stamp = parent; stamp; stamp = stamp->parent)
	if (stamp->v == nvfs && stamp->id == nvfsid)
	    break;
    return (stamp ? 1 : 0);
}

void vfs_add_noncurrent_stamps (vfs * oldvfs, vfsid oldvfsid, struct vfs_stamping *parent)
{
    vfs *nvfs, *n2vfs, *n3vfs;
    vfsid nvfsid, n2vfsid, n3vfsid;
    struct vfs_stamping *par, *stamp;
    int f;

    /* FIXME: As soon as we convert to multiple panels, this stuff
       has to change. It works like this: We do not time out the
       vfs's which are current in any panel and on the other
       side we add the old directory with all its parents which
       are not in any panel (if we find such one, we stop adding
       parents to the time-outing structure. */

    /* There are three directories we have to take care of: current_dir,
       cpanel->cwd and opanel->cwd. Athough most of the time either
       current_dir and cpanel->cwd or current_dir and opanel->cwd are the
       same, it's possible that all three are different -- Norbert */
       
    if (!cpanel)
	return;

    nvfs = vfs_type (current_dir);
    nvfsid = vfs_ncs_getid (nvfs, current_dir, &par);
    vfs_rmstamp (nvfs, nvfsid, 1);

    f = is_parent (oldvfs, oldvfsid, par);
    vfs_rm_parents (par);
    if ((nvfs == oldvfs && nvfs == oldvfsid) || oldvfsid == (vfsid *)-1 || f) {
	return;
    }

    if (get_current_type () == view_listing) {
	n2vfs = vfs_type (cpanel->cwd);
	n2vfsid = vfs_ncs_getid (n2vfs, cpanel->cwd, &par);
        f = is_parent (oldvfs, oldvfsid, par);
	vfs_rm_parents (par);
	if ((n2vfs == oldvfs && n2vfsid == oldvfsid) || f) 
	    return;
    } else {
	n2vfs = (vfs *) -1;
	n2vfsid = (vfs *) -1;
    }
    
    if (get_other_type () == view_listing) {
	n3vfs = vfs_type (opanel->cwd);
	n3vfsid = vfs_ncs_getid (n3vfs, opanel->cwd, &par);
        f = is_parent (oldvfs, oldvfsid, par);
	vfs_rm_parents (par);
	if ((n3vfs == oldvfs && n3vfsid == oldvfsid) || f)
	    return;
    } else {
	n3vfs = (vfs *)-1;
	n3vfsid = (vfs *)-1;
    }
    
    if ((*oldvfs->nothingisopen) (oldvfsid)) {
	if (oldvfs == &extfs_vfs_ops && ((struct extfs_archive *) oldvfsid)->name == 0) {
	    /* Free the resources immediatly when we leave a mtools fs
	       ('cd a:') instead of waiting for the vfs-timeout */
	    (oldvfs->free) (oldvfsid);
	} else
	    vfs_addstamp (oldvfs, oldvfsid, parent);
	for (stamp = parent; stamp != NULL; stamp = stamp->parent) {
	    if ((stamp->v == nvfs && stamp->id == nvfsid) ||
		(stamp->v == n2vfs && stamp->id == n2vfsid) ||
		(stamp->v == n3vfs && stamp->id == n3vfsid) ||
		stamp->id == (vfsid) - 1 ||
		!(*stamp->v->nothingisopen) (stamp->id))
		break;
	    if (stamp->v == &extfs_vfs_ops && ((struct extfs_archive *) stamp->id)->name == 0) {
		(stamp->v->free) (stamp->id);
		vfs_rmstamp (stamp->v, stamp->id, 0);
	    } else
		vfs_addstamp (stamp->v, stamp->id, stamp->parent);
	}
    }
}


static void vfs_stamp_path (char *path)
{
    vfs *vfs;
    vfsid id;
    struct vfs_stamping *par, *stamp;
    
    vfs = vfs_type (path);
    id  = vfs_ncs_getid (vfs, path, &par);
    vfs_addstamp (vfs, id, par);
    
    for (stamp = par; stamp != NULL; stamp = stamp->parent) 
	vfs_addstamp (stamp->v, stamp->id, stamp->parent);
    vfs_rm_parents (par);
}

void vfs_add_current_stamps (void)
{
    vfs_stamp_path (current_dir);
    if (get_current_type () == view_listing)
        vfs_stamp_path (cpanel->cwd);
    if (get_other_type () == view_listing)
	vfs_stamp_path (opanel->cwd);
}

/* This function is really broken */
int mc_chdir (char *path)
{
    char *a;
    int result;
    char *p = NULL;
    int i = strlen (path);
    vfs *oldvfs;
    vfsid oldvfsid;
    struct vfs_stamping *parent;

    if (path [i - 1] != PATH_SEP) {
    /* We should make possible reading of the root directory in a tar archive */
        p = xmalloc (i + 2, "slash");
        strcpy (p, path);
        strcpy (p + i, PATH_SEP_STR);
        path = p;
    }
    
    if (!current_dir) {
	current_dir = strdup (PATH_SEP_STR); /* Note: we should set current_dir
	                               on startup to a reasonable pwd.
	                               It cannot be relative, i.e. no
	                               . or anything else except
	                               /, tar:, mcfs:, local:, ftp:, 
	                               $extfs_prefix .
	                               This is done by the vfs_setup_wd call
	                               in main.c */
	last_current_dir = current_dir;
    }	                               

    a = current_dir; /* Save a copy for case of failure */
    if (last_current_dir != current_dir)
	free (last_current_dir);
    last_current_dir = a;
    current_dir = vfs_canon (path);
    current_vfs = vfs_type (current_dir);
    result = (*current_vfs->chdir)(vfs_name (current_dir));
    if (result == -1) {
	errno = (*current_vfs->ferrno)();
    	free (current_dir);
    	current_vfs = vfs_type (a);
    	current_dir = a;
    } else {
    	last_current_dir = current_dir;
        oldvfs = vfs_type (a);
	oldvfsid = vfs_ncs_getid (oldvfs, a, &parent);
	free (a);
        vfs_add_noncurrent_stamps (oldvfs, oldvfsid, parent);
	vfs_rm_parents (parent);
    }
    if (p) {
        free (p);
    }
    if (*current_dir) {
	p = strchr (current_dir, 0) - 1;
	if (*p == PATH_SEP && p > current_dir)
	    *p = 0; /* Sometimes we assume no trailing slash on cwd */
    }
    return result;
}

int vfs_current_is_local (void)
{
    return current_vfs == &local_vfs_ops;
}

int vfs_current_is_extfs (void)
{
    return current_vfs == &extfs_vfs_ops;
}

int vfs_current_is_tarfs (void)
{
    return current_vfs == &tarfs_vfs_ops;
}

int vfs_file_is_local (char *filename)
{
    vfs *vfs;
    
    filename = vfs_canon (filename);
    vfs = vfs_type (filename);
    free (filename);
    return vfs == &local_vfs_ops;
}

int vfs_file_is_ftp (char *filename)
{
#ifdef USE_NETCODE
    vfs *vfs;
    
    filename = vfs_canon (filename);
    vfs = vfs_type (filename);
    free (filename);
    return vfs == &ftpfs_vfs_ops;
#else
    return 0;
#endif
}

char *vfs_get_current_dir (void)
{
    return current_dir;
}

static void vfs_setup_wd (void)
{
    current_dir = xmalloc (MC_MAXPATHLEN, "Current directory");
    if (!get_current_wd (current_dir, MC_MAXPATHLEN))
	if (errno != 0)
	     current_dir [0] = 0;
    current_dir [MC_MAXPATHLEN - 1] = 0;
    last_current_dir = current_dir;
    return;
}

int mc_mkdir (char *path, mode_t mode)
{
    vfs *vfs;
    int result;

    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->mkdir)(vfs_name (path), mode);
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_rmdir (char *path)
{
    vfs *vfs;
    int result;

    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->rmdir)(vfs_name (path));
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

int mc_mknod (char *path, int mode, int dev)
{
    vfs *vfs;
    int result;

    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->mknod)(vfs_name (path), mode, dev);
    free (path);
    if (result == -1){
	errno = (*vfs->ferrno)();
	return -1;
    }
    return result;
}

#ifdef HAVE_MMAP
struct mc_mmapping {
    caddr_t addr;
    void *vfs_info;
    vfs *vfs;
    struct mc_mmapping *next;
} *mc_mmaparray = NULL;

caddr_t mc_mmap (caddr_t addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    vfs *vfs;
    caddr_t result;
    struct mc_mmapping *mcm;

    if (fd == -1)
	return (caddr_t) -1;
    
    vfs = vfs_op (fd);
    result = (*vfs->mmap)(addr, len, prot, flags, vfs_info (fd), offset);
    if (result == (caddr_t)-1){
	errno = (*vfs->ferrno)();
	return (caddr_t)-1;
    }
    mcm = (struct mc_mmapping *) xmalloc (sizeof (struct mc_mmapping), "vfs: mmap handling");
    mcm->addr = result;
    mcm->vfs_info = vfs_info (fd);
    mcm->vfs = vfs;
    mcm->next = mc_mmaparray;
    mc_mmaparray = mcm;
    return result;
}

int mc_munmap (caddr_t addr, size_t len)
{
    struct mc_mmapping *mcm, *mcm2 = NULL;
    
    for (mcm = mc_mmaparray; mcm != NULL; mcm2 = mcm, mcm = mcm->next) {
        if (mcm->addr == addr) {
            if (mcm2 == NULL)
            	mc_mmaparray = mcm->next;
            else
            	mcm2->next = mcm->next;
            (*mcm->vfs->munmap)(addr, len, mcm->vfs_info);
            free (mcm);
            return 0;
        }
    }
    return -1;
}

#endif

char *mc_def_getlocalcopy (char *filename)
{
    char *tmp;
    int fdin, fdout, i;
    char buffer[8192];
    struct stat mystat;

    fdin = mc_open (filename, O_RDONLY);
    if (fdin == -1)
        return NULL;
    fdout = mc_mkstemp(&tmp, "vfs-", NULL);
    if (fdout == -1) {
        mc_close (fdin);
        return NULL;
    }
    tmp = strdup (tmp);
    while ((i = mc_read (fdin, buffer, sizeof (buffer))) > 0) {
        write (fdout, buffer, i);
    }
    mc_close (fdin);
    close (fdout);
    if (mc_stat (filename, &mystat) != -1) {
        chmod (tmp, mystat.st_mode);
    }
    return tmp;
}

char *mc_getlocalcopy (char *path)
{
    vfs *vfs;
    char *result;

    path = vfs_canon (path);
    vfs = vfs_type (path);    
    result = (*vfs->getlocalcopy)(vfs_name (path));
    free (path);
    if (result == NULL){
	errno = (*vfs->ferrno)();
	return NULL;
    }
    return result;
}

void mc_def_ungetlocalcopy (char *filename, char *local, int has_changed)
{
    if (has_changed) {
        int fdin, fdout, i;
        char buffer[8192];
    
        fdin = open (local, O_RDONLY);
        if (fdin == -1) {
            unlink (local);
            free (local);
            return;
        }
        fdout = mc_open (filename, O_WRONLY | O_TRUNC);
        if (fdout == -1) {
            close (fdin);
            unlink (local);
            free (local);
            return;
        }
        while ((i = read (fdin, buffer, sizeof (buffer))) > 0) {
            mc_write (fdout, buffer, i);
        }
        close (fdin);
        mc_close (fdout);
    }
    unlink (local);
    free (local);
}

void mc_ungetlocalcopy (char *path, char *local, int has_changed)
{
    vfs *vfs;

    path = vfs_canon (path);
    vfs = vfs_type (path);    
    (*vfs->ungetlocalcopy)(vfs_name (path), local, has_changed);
    free (path);
}

inline int timeoutcmp (struct timeval *t1, struct timeval *t2)
{
    return ((t1->tv_sec < t2->tv_sec)
	    || ((t1->tv_sec == t2->tv_sec) && (t1->tv_usec <= t2->tv_usec)));
}

void vfs_expire (int now)
{
    static int locked = 0;
    struct timeval time;
    struct vfs_stamping *stamp, *st;

    /* Avoid recursive invocation, e.g. when one of the free functions
       calls message_1s */
    if (locked)
	return;
    locked = 1;

    gettimeofday (&time, NULL);
    time.tv_sec -= vfs_timeout;

    for (stamp = stamps; stamp != NULL;){
        if (now || (timeoutcmp (&stamp->time, &time))){
            st = stamp->next;
            (*stamp->v->free) (stamp->id);
	    vfs_rmstamp (stamp->v, stamp->id, 0);
	    stamp = st;
        } else
            stamp = stamp->next;
    }
    locked = 0;
}


void vfs_timeout_handler ()
{
vfs_expire (0);

/*
    struct timeval time;
    struct vfs_stamping *stamp, *st;

    gettimeofday (&time, NULL);
    time.tv_sec -= vfs_timeout;
    for (stamp = stamps; stamp != NULL;) {
        if (timeoutcmp (&stamp->time, &time)) {
            st = stamp->next;
            (*stamp->v->free) (stamp->id);
	    vfs_rmstamp (stamp->v, stamp->id, 0);
	    stamp = st;
        } else
            stamp = stamp->next;
    }
*/
}

void vfs_init (void)
{
    time_t current_time;
    struct tm *t;
    
    current_time = time (NULL);
    t = localtime (&current_time);
    current_mon = t->tm_mon;
    current_year = t->tm_year;
    
#ifdef USE_NETCODE
    tcp_init();
    ftpfs_init();
#endif
    extfs_init ();
    vfs_setup_wd ();
}

void vfs_free_resources (char *path)
{
    vfs *vfs;
    vfsid vid;
    struct vfs_stamping *parent;
    
    vfs = vfs_type (path);
    vid = vfs_ncs_getid (vfs, path, &parent);
    if (vid != (vfsid) -1)
	(*vfs->free)(vid);
    vfs_rm_parents (parent);
}

#if 0
/* Shutdown a vfs given a path name */
void vfs_shut_path (char *p)
{
    vfs *the_vfs;
    struct vfs_stamping *par;

    the_vfs = vfs_type (p);
    vfs_ncs_getid (the_vfs, p, &par);
    (*par->v->free)(par->id);
    vfs_rm_parents (par);
}
#endif

void vfs_shut (void)
{
    struct vfs_stamping *stamp, *st;

    for ( stamp = stamps, stamps = 0; stamp != NULL; ) {
	(*stamp->v->free)(stamp->id);
	st = stamp->next;
	free (stamp);
	stamp = st;
    }

    if (stamps)
	vfs_rmstamp (stamps->v, stamps->id, 1);
    
    if (last_current_dir && last_current_dir != current_dir)
	free (last_current_dir);
    if (current_dir)
	free (current_dir);

    extfs_done ();

#ifdef USE_NETCODE
    ftpfs_done();
#endif

}

/* These ones grab information from the VFS
 *  and handles them to an upper layer
 */
void vfs_fill_names (void (*func)(char *))
{
#ifdef USE_NETCODE
    mcfs_fill_names (func);
    ftpfs_fill_names (func);
#endif
    tarfs_fill_names (func);
    extfs_fill_names (func);
}

/* Following stuff (parse_ls_lga) is used by ftpfs and extfs */
#define MAXCOLS 30

static char *columns [MAXCOLS];	/* Points to the string in column n */
static int   column_ptr [MAXCOLS]; /* Index from 0 to the starting positions of the columns */

static int split_text (char *p)
{
    char *original = p;
    int  numcols;


    for (numcols = 0; *p && numcols < MAXCOLS; numcols++){
	while (*p == ' ' || *p == '\r' || *p == '\n'){
	    *p = 0;
	    p++;
	}
	columns [numcols] = p;
	column_ptr [numcols] = p - original;
	while (*p && *p != ' ' && *p != '\r' && *p != '\n')
	    p++;
    }
    return numcols;
}

static int is_num (int idx)
{
    if (columns [idx][0] < '0' || columns [idx][0] > '9')
	return 0;
    return 1;
}

#define free_and_return(x) {free (p_copy); return (x); }
int parse_ls_lga (char *p, struct stat *s, char **filename, char **linkname)
{
    int idx, idx2, num_cols, isconc = 0;
    int i;
    long l;
    struct tm tim;
    int extfs_format_date = 0;
    int year_supplied = 0;
    char *p_copy;
    
    s->st_mode = 0;
    if (strncmp (p, "total", 5) == 0){
        return 0;
    }
    switch (*(p++)) {
        case 'd': s->st_mode |= S_IFDIR; break;
        case 'b': s->st_mode |= S_IFBLK; break;
        case 'c': s->st_mode |= S_IFCHR; break;
        case 'm': s->st_mode |= S_IFREG; break; /* Don't know what it is :-) */
        case 'n': s->st_mode |= S_IFREG; break; /* and this as well */
        case 'l': s->st_mode |= S_IFLNK; break;
#ifdef IS_IFSOCK        
        case 's': s->st_mode |= S_IFSOCK; break;
#endif
	case 'D': /* Solaris door */
#ifdef S_IFDOOR
		s->st_mode |= S_IFDOOR; break;
#else
		s->st_mode |= S_IFIFO; break;
#endif
        case 'p': s->st_mode |= S_IFIFO; break;
        case '-':
        case '?': s->st_mode |= S_IFREG; break;
        default: return 0;
    }
    if (*p == '[') {
	if (strlen (p) <= 8 || p [8] != ']')
	    return 0;
	/* Should parse here the Notwell permissions :) */
	if (S_ISDIR (s->st_mode))
	    s->st_mode |= (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IXUSR | S_IXGRP | S_IXOTH);
	else
	    s->st_mode |= (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR);
	p += 9;
    } else {
	switch (*(p++)) {
	 case 'r': s->st_mode |= 0400; break;
	 case '-': break;
	 default: return 0;
	}
	switch (*(p++)) {
	 case 'w': s->st_mode |= 0200; break;
	 case '-': break;
	 default: return 0;
	}
	switch (*(p++)) {
	 case 'x': s->st_mode |= 0100; break;
	 case 's': s->st_mode |= 0100 | S_ISUID; break;
	 case 'S': s->st_mode |= S_ISUID; break;
	 case '-': break;
	 default: return 0;
	}
	switch (*(p++)) {
	 case 'r': s->st_mode |= 0040; break;
	 case '-': break;
	 default: return 0;
	}
	switch (*(p++)) {
	 case 'w': s->st_mode |= 0020; break;
	 case '-': break;
	 default: return 0;
	}
	switch (*(p++)) {
	 case 'x': s->st_mode |= 0010; break;
	 case 's': s->st_mode |= 0010 | S_ISGID; break;
	 case 'S': s->st_mode |= S_ISGID; break;
	 case '-': break;
	 default: return 0;
	}
	switch (*(p++)) {
	 case 'r': s->st_mode |= 0004; break;
	 case '-': break;
	 default: return 0;
	}
	switch (*(p++)) {
	 case 'w': s->st_mode |= 0002; break;
	 case '-': break;
	 default: return 0;
	}
	switch (*(p++)) {
	 case 'x': s->st_mode |= 0001; break;
	 case 't': s->st_mode |= 0001 | S_ISVTX; break;
	 case 'T': s->st_mode |= S_ISVTX; break;
	 case '-': break;
	 default: return 0;
	}
    }
	
    p_copy = strdup (p);
    num_cols = split_text (p);

    s->st_nlink = atol (columns [0]);
    if (s->st_nlink <= 0)
        free_and_return (0);

    if (!is_num (1))
	s->st_uid = finduid (columns [1]);
    else
        s->st_uid = (uid_t) atol (columns [1]);

    /* Mhm, the ls -lg did not produce a group field */
    for (idx = 3; idx <= 5; idx++) 
        if ((*columns [idx] >= 'A' && *columns [idx] <= 'S' &&
            strlen (columns[idx]) == 3) || (strlen (columns[idx])==8 &&
            columns [idx][2] == '-' && columns [idx][5] == '-'))
            break;
    if (idx == 6 || (idx == 5 && !S_ISCHR (s->st_mode) && !S_ISBLK (s->st_mode)))
        free_and_return (0);
    if (idx < 5) {
        char *p = strchr(columns [idx - 1], ',');
        if (p && p[1] >= '0' && p[1] <= '9')
            isconc = 1;
    }
    if (idx == 3 || (idx == 4 && !isconc && (S_ISCHR(s->st_mode) || S_ISBLK (s->st_mode))))
        idx2 = 2;
    else {
	if (is_num (2))
	    s->st_gid = (gid_t) atol (columns [2]);
	else
	    s->st_gid = findgid (columns [2]);
	idx2 = 3;
    }

    if (S_ISCHR (s->st_mode) || S_ISBLK (s->st_mode)) {
        char *p;
	if (!is_num (idx2))
	    free_and_return (0);
#ifdef HAVE_ST_RDEV
	s->st_rdev = (atol (columns [idx2]) & 0xff) << 8;
#endif
	if (isconc) {
	    p = strchr (columns [idx2], ',');
	    if (!p || p [1] < '0' || p [1] > '9')
	        free_and_return (0);
	    p++;
	} else {
	    p = columns [idx2 + 1];
	    if (!is_num (idx2+1))
	        free_and_return (0);
	}
	
#ifdef HAVE_ST_RDEV
	s->st_rdev |= (atol (p) & 0xff);
#endif
	s->st_size = 0;
    } else {
	if (!is_num (idx2))
	    free_and_return (0);
	
	s->st_size = (size_t) atol (columns [idx2]);
#ifdef HAVE_ST_RDEV
	s->st_rdev = 0;
#endif
    }
    
    p = columns [idx++];
    
    if (!strcmp (p, "Jan"))
        tim.tm_mon = 0; 
    else if (!strcmp (p, "Feb"))
        tim.tm_mon = 1; 
    else if (!strcmp (p, "Mar"))
        tim.tm_mon = 2; 
    else if (!strcmp (p, "Apr"))
        tim.tm_mon = 3; 
    else if (!strcmp (p, "May"))
        tim.tm_mon = 4; 
    else if (!strcmp (p, "Jun"))
        tim.tm_mon = 5; 
    else if (!strcmp (p, "Jul"))
        tim.tm_mon = 6; 
    else if (!strcmp (p, "Aug"))
        tim.tm_mon = 7; 
    else if (!strcmp (p, "Sep"))
        tim.tm_mon = 8; 
    else if (!strcmp (p, "Oct"))
        tim.tm_mon = 9; 
    else if (!strcmp (p, "Nov"))
        tim.tm_mon = 10; 
    else if (!strcmp (p, "Dec"))
        tim.tm_mon = 11; 
    else {
        /* This case should not normaly happen, but in extfs we allow these
           date formats:
           Mon DD hh:mm
           Mon DD YYYY
           Mon DD YYYY hh:mm
           MM-DD-YY hh:mm
           where Mon is Jan-Dec, DD, MM, YY two digit day, month, year,
           YYYY four digit year, hh, mm two digit hour and minute. */
        if (strlen (p) == 8 && p [2] == '-' && p [5] == '-') {
            p [2] = 0;
            p [5] = 0;
            tim.tm_mon = (int) atol (p);
            if (!tim.tm_mon)
                free_and_return (0)
            else
                tim.tm_mon--;
            tim.tm_mday = (int) atol (p + 3);
            tim.tm_year = (int) atol (p + 6);
            if (tim.tm_year < 70)
                tim.tm_year += 70;
            extfs_format_date = 1;
        } else
            free_and_return (0);
    }

    if (!extfs_format_date) {
        if (!is_num (idx))
	    free_and_return (0);
        tim.tm_mday = (int)atol (columns [idx++]);
    }

    /* Microsoft ftp server may send a non padded digit */
    if (columns [idx][2] != ':' && columns [idx][1] != ':'){
	/* There is a year */
        l = atol (columns [idx++]);
        if (l < 1900 || l > 3000)
            free_and_return (0);
        tim.tm_year = (int) (l - 1900);
        tim.tm_hour = 0;
        tim.tm_min = 0;
        tim.tm_sec = 0;
        year_supplied = 1;
    }
    if (columns [idx][2] == ':' || columns [idx][1] == ':'){
        if (sscanf (columns [idx],
		    "%2d:%2d", &tim.tm_hour, &tim.tm_min) != 2) {
	    if (!year_supplied)
	        free_and_return (0);
	    tim.tm_hour = 0;
	    tim.tm_min = 0;
	    tim.tm_sec = 0;
        } else {
            idx++;
            tim.tm_sec = 0;
            if (!extfs_format_date && !year_supplied) {
                tim.tm_year = current_year;
                if (tim.tm_mon > current_mon)
                    tim.tm_year--;
            }
        }
    } else if (!year_supplied)
        free_and_return (0);
    tim.tm_isdst = 0;
    s->st_mtime = mktime (&tim);
    if (s->st_mtime == -1)
        s->st_mtime = 0;
    s->st_atime = s->st_mtime;
    s->st_ctime = s->st_mtime;
    s->st_dev = 0;
    s->st_ino = 0;
#ifdef HAVE_ST_BLKSIZE
    s->st_blksize = 512;
#endif
#ifdef HAVE_ST_BLOCKS
    s->st_blocks = (s->st_size + 511) / 512;
#endif

    for (i = idx + 1, idx2 = 0; i < num_cols; i++ ) 
	if (strcmp (columns [i], "->") == 0) {
	    idx2 = i;
	    break;
	}
    
    if (((S_ISLNK (s->st_mode) || 
        (num_cols == idx + 3 && s->st_nlink > 1))) /* Maybe a hardlink? (in extfs) */
        && idx2){
	int p;
	char *s;
	    
	if (filename) {
	    p = column_ptr [idx2] - column_ptr [idx];
	    s = xmalloc (p, "filename");
	    strncpy (s, p_copy + column_ptr [idx], p - 1);
	    s[p - 1] = '\0';
	    *filename = s;
	}
	if (linkname) {
	    s = strdup (p_copy + column_ptr [idx2+1]);
	    p = strlen (s);
	    if (s [p-1] == '\r' || s [p-1] == '\n')
		s [p-1] = 0;
	    if (s [p-2] == '\r' || s [p-2] == '\n')
		s [p-2] = 0;
		
	    *linkname = s;
	}
    } else {
	/* Extract the filename from the string copy, not from the columns
	 * this way we have a chance of entering hidden directories like ". ."
	 */
	if (filename){
	    /* 
	    *filename = strdup (columns [idx++]);
	    */
	    int p;
	    char *s;
	    
	    s = strdup (p_copy + column_ptr [idx++]);
	    p = strlen (s);
	    if (s [p-1] == '\r' || s [p-1] == '\n')
		s [p-1] = 0;
	    if (s [p-2] == '\r' || s [p-2] == '\n')
		s [p-2] = 0;
	    
	    *filename = s;
	}
	if (linkname)
	    *linkname = NULL;
    }
    free_and_return (1);
}

void
vfs_force_expire (char *pathname)
{
    vfs *vfs;
    
    pathname = vfs_canon (pathname);
    vfs = vfs_type (pathname);
    if (vfs->forget_about)
	(*vfs->forget_about) (pathname);
    free (pathname);
}
