/* copy.c -- core functions for copying files and directories
   Copyright (C) 89, 90, 91, 1995-2006 Free Software Foundation.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/* Extracted from cp.c and librarified by Jim Meyering.  */

#include <config.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>

#if HAVE_HURD_H
# include <hurd.h>
#endif
#if HAVE_PRIV_H
# include <priv.h>
#endif

#include "system.h"
#include "acl.h"
#include "backupfile.h"
#include "buffer-lcm.h"
#include "copy.h"
#include "cp-hash.h"
#include "euidaccess.h"
#include "error.h"
#include "fcntl--.h"
#include "filenamecat.h"
#include "full-write.h"
#include "getpagesize.h"
#include "hash.h"
#include "hash-pjw.h"
#include "lchmod.h"
#include "quote.h"
#include "same.h"
#include "savedir.h"
#include "stat-time.h"
#include "utimecmp.h"
#include "utimens.h"
#include "xreadlink.h"
#include "yesno.h"

#ifndef HAVE_FCHOWN
# define HAVE_FCHOWN false
# define fchown(fd, uid, gid) (-1)
#endif

#define SAME_OWNER(A, B) ((A).st_uid == (B).st_uid)
#define SAME_GROUP(A, B) ((A).st_gid == (B).st_gid)
#define SAME_OWNER_AND_GROUP(A, B) (SAME_OWNER (A, B) && SAME_GROUP (A, B))

#define UNWRITABLE(File_name, File_mode)		\
  ( /* euidaccess is not meaningful for symlinks */	\
    ! S_ISLNK (File_mode)				\
    && euidaccess (File_name, W_OK) != 0)

struct dir_list
{
  struct dir_list *parent;
  ino_t ino;
  dev_t dev;
};

/* Describe a just-created or just-renamed destination file.  */
struct F_triple
{
  char *name;
  ino_t st_ino;
  dev_t st_dev;
};

/* Initial size of the above hash table.  */
#define DEST_INFO_INITIAL_CAPACITY 61

static bool copy_internal (char const *src_name, char const *dst_name,
			   bool new_dst, dev_t device,
			   struct dir_list *ancestors,
			   const struct cp_options *x,
			   bool command_line_arg,
			   bool *copy_into_self,
			   bool *rename_succeeded);

/* Pointers to the file names:  they're used in the diagnostic that is issued
   when we detect the user is trying to copy a directory into itself.  */
static char const *top_level_src_name;
static char const *top_level_dst_name;

/* The invocation name of this program.  */
extern char *program_name;

/* FIXME: describe */
/* FIXME: rewrite this to use a hash table so we avoid the quadratic
   performance hit that's probably noticeable only on trees deeper
   than a few hundred levels.  See use of active_dir_map in remove.c  */

static bool
is_ancestor (const struct stat *sb, const struct dir_list *ancestors)
{
  while (ancestors != 0)
    {
      if (ancestors->ino == sb->st_ino && ancestors->dev == sb->st_dev)
	return true;
      ancestors = ancestors->parent;
    }
  return false;
}

/* Read the contents of the directory SRC_NAME_IN, and recursively
   copy the contents to DST_NAME_IN.  NEW_DST is true if
   DST_NAME_IN is a directory that was created previously in the
   recursion.   SRC_SB and ANCESTORS describe SRC_NAME_IN.
   Set *COPY_INTO_SELF if SRC_NAME_IN is a parent of
   (or the same as) DST_NAME_IN; otherwise, clear it.
   Return true if successful.  */

static bool
copy_dir (char const *src_name_in, char const *dst_name_in, bool new_dst,
	  const struct stat *src_sb, struct dir_list *ancestors,
	  const struct cp_options *x, bool *copy_into_self)
{
  char *name_space;
  char *namep;
  struct cp_options non_command_line_options = *x;
  bool ok = true;

  name_space = savedir (src_name_in);
  if (name_space == NULL)
    {
      /* This diagnostic is a bit vague because savedir can fail in
         several different ways.  */
      error (0, errno, _("cannot access %s"), quote (src_name_in));
      return false;
    }

  /* For cp's -H option, dereference command line arguments, but do not
     dereference symlinks that are found via recursive traversal.  */
  if (x->dereference == DEREF_COMMAND_LINE_ARGUMENTS)
    non_command_line_options.dereference = DEREF_NEVER;

  namep = name_space;
  while (*namep != '\0')
    {
      bool local_copy_into_self;
      char *src_name = file_name_concat (src_name_in, namep, NULL);
      char *dst_name = file_name_concat (dst_name_in, namep, NULL);

      ok &= copy_internal (src_name, dst_name, new_dst, src_sb->st_dev,
			   ancestors, &non_command_line_options, false,
			   &local_copy_into_self, NULL);
      *copy_into_self |= local_copy_into_self;

      free (dst_name);
      free (src_name);

      namep += strlen (namep) + 1;
    }
  free (name_space);
  return ok;
}

/* Set the owner and owning group of DEST_DESC to the st_uid and
   st_gid fields of SRC_SB.  If DEST_DESC is undefined (-1), set
   the owner and owning group of DST_NAME instead.  DEST_DESC must
   refer to the same file as DEST_NAME if defined.
   Return 1 if the syscall succeeds, 0 if it fails but it's OK
   not to preserve ownership, -1 otherwise.  */

static int
set_owner (const struct cp_options *x, char const *dst_name, int dest_desc,
	   uid_t uid, gid_t gid)
{
  if (HAVE_FCHOWN && dest_desc != -1)
    {
      if (fchown (dest_desc, uid, gid) == 0)
	return 1;
    }
  else
    {
      if (chown (dst_name, uid, gid) == 0)
	return 1;
    }

  if (! chown_failure_ok (x))
    {
      error (0, errno, _("failed to preserve ownership for %s"),
	     quote (dst_name));
      if (x->require_preserve)
	return -1;
    }

  return 0;
}

/* Set the st_author field of DEST_DESC to the st_author field of
   SRC_SB. If DEST_DESC is undefined (-1), set the st_author field
   of DST_NAME instead.  DEST_DESC must refer to the same file as
   DEST_NAME if defined.  */

static void
set_author (const char *dst_name, int dest_desc, const struct stat *src_sb)
{
#if HAVE_STRUCT_STAT_ST_AUTHOR
  /* Preserve the st_author field.  */
  file_t file = (dest_desc < 0
		 ? file_name_lookup (dst_name, 0, 0)
		 : getdport (dest_desc));
  if (file == MACH_PORT_NULL)
    error (0, errno, _("failed to lookup file %s"), quote (dst_name));
  else
    {
      error_t err = file_chauthor (file, src_sb->st_author);
      if (err)
	error (0, err, _("failed to preserve authorship for %s"),
	       quote (dst_name));
      mach_port_deallocate (mach_task_self (), file);
    }
#endif
}

/* Change the file mode bits of the file identified by DESC or NAME to MODE.
   Use DESC if DESC is valid and fchmod is available, NAME otherwise.  */

static int
fchmod_or_lchmod (int desc, char const *name, mode_t mode)
{
#if HAVE_FCHMOD
  if (0 <= desc)
    return fchmod (desc, mode);
#endif
  return lchmod (name, mode);
}

/* Copy a regular file from SRC_NAME to DST_NAME.
   If the source file contains holes, copies holes and blocks of zeros
   in the source file as holes in the destination file.
   (Holes are read as zeroes by the `read' system call.)
   When creating the destination, use DST_MODE & ~OMITTED_PERMISSIONS
   as the third argument in the call to open, adding
   OMITTED_PERMISSIONS after copying as needed.
   X provides many option settings.
   Return true if successful.
   *NEW_DST is as in copy_internal.
   SRC_SB is the result of calling XSTAT (aka stat) on SRC_NAME.  */

static bool
copy_reg (char const *src_name, char const *dst_name,
	  const struct cp_options *x,
	  mode_t dst_mode, mode_t omitted_permissions, bool *new_dst,
	  struct stat const *src_sb)
{
  char *buf;
  char *buf_alloc = NULL;
  int dest_desc;
  int source_desc;
  mode_t src_mode = src_sb->st_mode;
  struct stat sb;
  struct stat src_open_sb;
  bool return_val = true;

  source_desc = open (src_name, O_RDONLY | O_BINARY);
  if (source_desc < 0)
    {
      error (0, errno, _("cannot open %s for reading"), quote (src_name));
      return false;
    }

  if (fstat (source_desc, &src_open_sb) != 0)
    {
      error (0, errno, _("cannot fstat %s"), quote (src_name));
      return_val = false;
      goto close_src_desc;
    }

  /* Compare the source dev/ino from the open file to the incoming,
     saved ones obtained via a previous call to stat.  */
  if (! SAME_INODE (*src_sb, src_open_sb))
    {
      error (0, 0,
	     _("skipping file %s, as it was replaced while being copied"),
	     quote (src_name));
      return_val = false;
      goto close_src_desc;
    }

  /* These semantics are required for cp.
     The if-block will be taken in move_mode.  */
  if (! *new_dst)
    {
      dest_desc = open (dst_name, O_WRONLY | O_TRUNC | O_BINARY);

      if (dest_desc < 0 && x->unlink_dest_after_failed_open)
	{
	  if (unlink (dst_name) != 0)
	    {
	      error (0, errno, _("cannot remove %s"), quote (dst_name));
	      return_val = false;
	      goto close_src_desc;
	    }
	  if (x->verbose)
	    printf (_("removed %s\n"), quote (dst_name));

	  /* Tell caller that the destination file was unlinked.  */
	  *new_dst = true;
	}
    }

  if (*new_dst)
    dest_desc = open (dst_name, O_WRONLY | O_CREAT | O_EXCL | O_BINARY,
		      dst_mode & ~omitted_permissions);
  else
    omitted_permissions = 0;

  if (dest_desc < 0)
    {
      error (0, errno, _("cannot create regular file %s"), quote (dst_name));
      return_val = false;
      goto close_src_desc;
    }

  if (fstat (dest_desc, &sb) != 0)
    {
      error (0, errno, _("cannot fstat %s"), quote (dst_name));
      return_val = false;
      goto close_src_and_dst_desc;
    }

  if (! (S_ISREG (src_open_sb.st_mode) && src_open_sb.st_size == 0))
    {
      typedef uintptr_t word;
      off_t n_read_total = 0;

      /* Choose a suitable buffer size; it may be adjusted later.  */
      size_t buf_alignment = lcm (getpagesize (), sizeof (word));
      size_t buf_alignment_slop = sizeof (word) + buf_alignment - 1;
      size_t buf_size = ST_BLKSIZE (sb);

      /* Deal with sparse files.  */
      bool last_write_made_hole = false;
      bool make_holes = false;

      if (S_ISREG (sb.st_mode))
	{
	  /* Even with --sparse=always, try to create holes only
	     if the destination is a regular file.  */
	  if (x->sparse_mode == SPARSE_ALWAYS)
	    make_holes = true;

#if HAVE_STRUCT_STAT_ST_BLOCKS
	  /* Use a heuristic to determine whether SRC_NAME contains any sparse
	     blocks.  If the file has fewer blocks than would normally be
	     needed for a file of its size, then at least one of the blocks in
	     the file is a hole.  */
	  if (x->sparse_mode == SPARSE_AUTO && S_ISREG (src_open_sb.st_mode)
	      && ST_NBLOCKS (src_open_sb) < src_open_sb.st_size / ST_NBLOCKSIZE)
	    make_holes = true;
#endif
	}

      /* If not making a sparse file, try to use a more-efficient
	 buffer size.  */
      if (! make_holes)
	{
	  /* These days there's no point ever messing with buffers smaller
	     than 8 KiB.  It would be nice to configure SMALL_BUF_SIZE
	     dynamically for this host and pair of files, but there doesn't
	     seem to be a good way to get readahead info portably.  */
	  enum { SMALL_BUF_SIZE = 8 * 1024 };

	  /* Compute the least common multiple of the input and output
	     buffer sizes, adjusting for outlandish values.  */
	  size_t blcm_max = MIN (SIZE_MAX, SSIZE_MAX) - buf_alignment_slop;
	  size_t blcm = buffer_lcm (ST_BLKSIZE (src_open_sb), buf_size,
				    blcm_max);

	  /* Do not use a block size that is too small.  */
	  buf_size = MAX (SMALL_BUF_SIZE, blcm);

	  /* Do not bother with a buffer larger than the input file, plus one
	     byte to make sure the file has not grown while reading it.  */
	  if (S_ISREG (src_open_sb.st_mode) && src_open_sb.st_size < buf_size)
	    buf_size = src_open_sb.st_size + 1;

	  /* However, stick with a block size that is a positive multiple of
	     blcm, overriding the above adjustments.  Watch out for
	     overflow.  */
	  buf_size += blcm - 1;
	  buf_size -= buf_size % blcm;
	  if (buf_size == 0 || blcm_max < buf_size)
	    buf_size = blcm;
	}

      /* Make a buffer with space for a sentinel at the end.  */
      buf_alloc = xmalloc (buf_size + buf_alignment_slop);
      buf = ptr_align (buf_alloc, buf_alignment);

      for (;;)
	{
	  word *wp = NULL;

	  ssize_t n_read = read (source_desc, buf, buf_size);
	  if (n_read < 0)
	    {
#ifdef EINTR
	      if (errno == EINTR)
		continue;
#endif
	      error (0, errno, _("reading %s"), quote (src_name));
	      return_val = false;
	      goto close_src_and_dst_desc;
	    }
	  if (n_read == 0)
	    break;

	  n_read_total += n_read;

	  if (make_holes)
	    {
	      char *cp;

	      buf[n_read] = 1;	/* Sentinel to stop loop.  */

	      /* Find first nonzero *word*, or the word with the sentinel.  */

	      wp = (word *) buf;
	      while (*wp++ == 0)
		continue;

	      /* Find the first nonzero *byte*, or the sentinel.  */

	      cp = (char *) (wp - 1);
	      while (*cp++ == 0)
		continue;

	      if (cp <= buf + n_read)
		/* Clear to indicate that a normal write is needed. */
		wp = NULL;
	      else
		{
		  /* We found the sentinel, so the whole input block was zero.
		     Make a hole.  */
		  if (lseek (dest_desc, n_read, SEEK_CUR) < 0)
		    {
		      error (0, errno, _("cannot lseek %s"), quote (dst_name));
		      return_val = false;
		      goto close_src_and_dst_desc;
		    }
		  last_write_made_hole = true;
		}
	    }

	  if (!wp)
	    {
	      size_t n = n_read;
	      if (full_write (dest_desc, buf, n) != n)
		{
		  error (0, errno, _("writing %s"), quote (dst_name));
		  return_val = false;
		  goto close_src_and_dst_desc;
		}
	      last_write_made_hole = false;

	      /* A short read on a regular file means EOF.  */
	      if (n_read != buf_size && S_ISREG (src_open_sb.st_mode))
		break;
	    }
	}

      /* If the file ends with a `hole', we need to do something to record
	 the length of the file.  On modern systems, calling ftruncate does
	 the job.  On systems without native ftruncate support, we have to
	 write a byte at the ending position.  Otherwise the kernel would
	 truncate the file at the end of the last write operation.  */

      if (last_write_made_hole)
	{
	  if (HAVE_FTRUNCATE
	      ? /* ftruncate sets the file size,
		   so there is no need for a write.  */
	      ftruncate (dest_desc, n_read_total) < 0
	      : /* Seek backwards one character and write a null.  */
	      (lseek (dest_desc, (off_t) -1, SEEK_CUR) < 0L
	       || full_write (dest_desc, "", 1) != 1))
	    {
	      error (0, errno, _("writing %s"), quote (dst_name));
	      return_val = false;
	      goto close_src_and_dst_desc;
	    }
	}
    }

  if (x->preserve_timestamps)
    {
      struct timespec timespec[2];
      timespec[0] = get_stat_atime (src_sb);
      timespec[1] = get_stat_mtime (src_sb);

      if (futimens (dest_desc, dst_name, timespec) != 0)
	{
	  error (0, errno, _("preserving times for %s"), quote (dst_name));
	  if (x->require_preserve)
	    {
	      return_val = false;
	      goto close_src_and_dst_desc;
	    }
	}
    }

  if (x->preserve_ownership && ! SAME_OWNER_AND_GROUP (*src_sb, sb))
    {
      switch (set_owner (x, dst_name, dest_desc,
			 src_sb->st_uid, src_sb->st_gid))
	{
	case -1:
	  return_val = false;
	  goto close_src_and_dst_desc;

	case 0:
	  src_mode &= ~ (S_ISUID | S_ISGID | S_ISVTX);
	  break;
	}
    }

  set_author (dst_name, dest_desc, src_sb);

  if (x->preserve_mode || x->move_mode)
    {
      if (copy_acl (src_name, source_desc, dst_name, dest_desc, src_mode) != 0
	  && x->require_preserve)
	return_val = false;
    }
  else if (x->set_mode)
    {
      if (set_acl (dst_name, dest_desc, x->mode) != 0)
	return_val = false;
    }
  else if (omitted_permissions)
    {
      omitted_permissions &= ~ cached_umask ();
      if (omitted_permissions
	  && fchmod_or_lchmod (dest_desc, dst_name, dst_mode) != 0)
	{
	  error (0, errno, _("preserving permissions for %s"),
		 quote (dst_name));
	  if (x->require_preserve)
	    return_val = false;
	}
    }

close_src_and_dst_desc:
  if (close (dest_desc) < 0)
    {
      error (0, errno, _("closing %s"), quote (dst_name));
      return_val = false;
    }
close_src_desc:
  if (close (source_desc) < 0)
    {
      error (0, errno, _("closing %s"), quote (src_name));
      return_val = false;
    }

  free (buf_alloc);
  return return_val;
}

/* Return true if it's ok that the source and destination
   files are the `same' by some measure.  The goal is to avoid
   making the `copy' operation remove both copies of the file
   in that case, while still allowing the user to e.g., move or
   copy a regular file onto a symlink that points to it.
   Try to minimize the cost of this function in the common case.
   Set *RETURN_NOW if we've determined that the caller has no more
   work to do and should return successfully, right away.

   Set *UNLINK_SRC if we've determined that the caller wants to do
   `rename (a, b)' where `a' and `b' are distinct hard links to the same
   file. In that case, the caller should try to unlink `a' and then return
   successfully.  Ideally, we wouldn't have to do that, and we'd be
   able to rely on rename to remove the source file.  However, POSIX
   mistakenly requires that such a rename call do *nothing* and return
   successfully.  */

static bool
same_file_ok (char const *src_name, struct stat const *src_sb,
	      char const *dst_name, struct stat const *dst_sb,
	      const struct cp_options *x, bool *return_now, bool *unlink_src)
{
  const struct stat *src_sb_link;
  const struct stat *dst_sb_link;
  struct stat tmp_dst_sb;
  struct stat tmp_src_sb;

  bool same_link;
  bool same = SAME_INODE (*src_sb, *dst_sb);

  *return_now = false;
  *unlink_src = false;

  /* FIXME: this should (at the very least) be moved into the following
     if-block.  More likely, it should be removed, because it inhibits
     making backups.  But removing it will result in a change in behavior
     that will probably have to be documented -- and tests will have to
     be updated.  */
  if (same && x->hard_link)
    {
      *return_now = true;
      return true;
    }

  if (x->dereference == DEREF_NEVER)
    {
      same_link = same;

      /* If both the source and destination files are symlinks (and we'll
	 know this here IFF preserving symlinks), then it's ok -- as long
	 as they are distinct.  */
      if (S_ISLNK (src_sb->st_mode) && S_ISLNK (dst_sb->st_mode))
	return ! same_name (src_name, dst_name);

      src_sb_link = src_sb;
      dst_sb_link = dst_sb;
    }
  else
    {
      if (!same)
	return true;

      if (lstat (dst_name, &tmp_dst_sb) != 0
	  || lstat (src_name, &tmp_src_sb) != 0)
	return true;

      src_sb_link = &tmp_src_sb;
      dst_sb_link = &tmp_dst_sb;

      same_link = SAME_INODE (*src_sb_link, *dst_sb_link);

      /* If both are symlinks, then it's ok, but only if the destination
	 will be unlinked before being opened.  This is like the test
	 above, but with the addition of the unlink_dest_before_opening
	 conjunct because otherwise, with two symlinks to the same target,
	 we'd end up truncating the source file.  */
      if (S_ISLNK (src_sb_link->st_mode) && S_ISLNK (dst_sb_link->st_mode)
	  && x->unlink_dest_before_opening)
	return true;
    }

  /* The backup code ensures there's a copy, so it's usually ok to
     remove any destination file.  One exception is when both
     source and destination are the same directory entry.  In that
     case, moving the destination file aside (in making the backup)
     would also rename the source file and result in an error.  */
  if (x->backup_type != no_backups)
    {
      if (!same_link)
	{
	  /* In copy mode when dereferencing symlinks, if the source is a
	     symlink and the dest is not, then backing up the destination
	     (moving it aside) would make it a dangling symlink, and the
	     subsequent attempt to open it in copy_reg would fail with
	     a misleading diagnostic.  Avoid that by returning zero in
	     that case so the caller can make cp (or mv when it has to
	     resort to reading the source file) fail now.  */

	  /* FIXME-note: even with the following kludge, we can still provoke
	     the offending diagnostic.  It's just a little harder to do :-)
	     $ rm -f a b c; touch c; ln -s c b; ln -s b a; cp -b a b
	     cp: cannot open `a' for reading: No such file or directory
	     That's misleading, since a subsequent `ls' shows that `a'
	     is still there.
	     One solution would be to open the source file *before* moving
	     aside the destination, but that'd involve a big rewrite. */
	  if ( ! x->move_mode
	       && x->dereference != DEREF_NEVER
	       && S_ISLNK (src_sb_link->st_mode)
	       && ! S_ISLNK (dst_sb_link->st_mode))
	    return false;

	  return true;
	}

      return ! same_name (src_name, dst_name);
    }

#if 0
  /* FIXME: use or remove */

  /* If we're making a backup, we'll detect the problem case in
     copy_reg because SRC_NAME will no longer exist.  Allowing
     the test to be deferred lets cp do some useful things.
     But when creating hardlinks and SRC_NAME is a symlink
     but DST_NAME is not we must test anyway.  */
  if (x->hard_link
      || !S_ISLNK (src_sb_link->st_mode)
      || S_ISLNK (dst_sb_link->st_mode))
    return true;

  if (x->dereference != DEREF_NEVER)
    return true;
#endif

  /* They may refer to the same file if we're in move mode and the
     target is a symlink.  That is ok, since we remove any existing
     destination file before opening it -- via `rename' if they're on
     the same file system, via `unlink (DST_NAME)' otherwise.
     It's also ok if they're distinct hard links to the same file.  */
  if (x->move_mode || x->unlink_dest_before_opening)
    {
      if (S_ISLNK (dst_sb_link->st_mode))
	return true;

      if (same_link
	  && 1 < dst_sb_link->st_nlink
	  && ! same_name (src_name, dst_name))
	{
	  if (x->move_mode)
	    {
	      *unlink_src = true;
	      *return_now = true;
	    }
	  return true;
	}
    }

  /* If neither is a symlink, then it's ok as long as they aren't
     hard links to the same file.  */
  if (!S_ISLNK (src_sb_link->st_mode) && !S_ISLNK (dst_sb_link->st_mode))
    {
      if (!SAME_INODE (*src_sb_link, *dst_sb_link))
	return true;

      /* If they are the same file, it's ok if we're making hard links.  */
      if (x->hard_link)
	{
	  *return_now = true;
	  return true;
	}
    }

  /* It's ok to remove a destination symlink.  But that works only when we
     unlink before opening the destination and when the source and destination
     files are on the same partition.  */
  if (x->unlink_dest_before_opening
      && S_ISLNK (dst_sb_link->st_mode))
    return dst_sb_link->st_dev == src_sb_link->st_dev;

  if (x->dereference == DEREF_NEVER)
    {
      if ( ! S_ISLNK (src_sb_link->st_mode))
	tmp_src_sb = *src_sb_link;
      else if (stat (src_name, &tmp_src_sb) != 0)
	return true;

      if ( ! S_ISLNK (dst_sb_link->st_mode))
	tmp_dst_sb = *dst_sb_link;
      else if (stat (dst_name, &tmp_dst_sb) != 0)
	return true;

      if ( ! SAME_INODE (tmp_src_sb, tmp_dst_sb))
	return true;

      /* FIXME: shouldn't this be testing whether we're making symlinks?  */
      if (x->hard_link)
	{
	  *return_now = true;
	  return true;
	}
    }

  return false;
}

static void
overwrite_prompt (char const *dst_name, struct stat const *dst_sb)
{
  if (euidaccess (dst_name, W_OK) != 0)
    {
      fprintf (stderr,
	       _("%s: overwrite %s, overriding mode %04lo? "),
	       program_name, quote (dst_name),
	       (unsigned long int) (dst_sb->st_mode & CHMOD_MODE_BITS));
    }
  else
    {
      fprintf (stderr, _("%s: overwrite %s? "),
	       program_name, quote (dst_name));
    }
}

/* Hash an F_triple.  */
static size_t
triple_hash (void const *x, size_t table_size)
{
  struct F_triple const *p = x;

  /* Also take the name into account, so that when moving N hard links to the
     same file (all listed on the command line) all into the same directory,
     we don't experience any N^2 behavior.  */
  /* FIXME-maybe: is it worth the overhead of doing this
     just to avoid N^2 in such an unusual case?  N would have
     to be very large to make the N^2 factor noticable, and
     one would probably encounter a limit on the length of
     a command line before it became a problem.  */
  size_t tmp = hash_pjw (p->name, table_size);

  /* Ignoring the device number here should be fine.  */
  return (tmp | p->st_ino) % table_size;
}

/* Hash an F_triple.  */
static size_t
triple_hash_no_name (void const *x, size_t table_size)
{
  struct F_triple const *p = x;

  /* Ignoring the device number here should be fine.  */
  return p->st_ino % table_size;
}

/* Compare two F_triple structs.  */
static bool
triple_compare (void const *x, void const *y)
{
  struct F_triple const *a = x;
  struct F_triple const *b = y;
  return (SAME_INODE (*a, *b) && same_name (a->name, b->name)) ? true : false;
}

/* Free an F_triple.  */
static void
triple_free (void *x)
{
  struct F_triple *a = x;
  free (a->name);
  free (a);
}

/* Initialize the hash table implementing a set of F_triple entries
   corresponding to destination files.  */
extern void
dest_info_init (struct cp_options *x)
{
  x->dest_info
    = hash_initialize (DEST_INFO_INITIAL_CAPACITY,
		       NULL,
		       triple_hash,
		       triple_compare,
		       triple_free);
}

/* Initialize the hash table implementing a set of F_triple entries
   corresponding to source files listed on the command line.  */
extern void
src_info_init (struct cp_options *x)
{

  /* Note that we use triple_hash_no_name here.
     Contrast with the use of triple_hash above.
     That is necessary because a source file may be specified
     in many different ways.  We want to warn about this
       cp a a d/
     as well as this:
       cp a ./a d/
  */
  x->src_info
    = hash_initialize (DEST_INFO_INITIAL_CAPACITY,
		       NULL,
		       triple_hash_no_name,
		       triple_compare,
		       triple_free);
}

/* Return true if there is an entry in hash table, HT,
   for the file described by FILE and STATS.  */
static bool
seen_file (Hash_table const *ht, char const *file,
	   struct stat const *stats)
{
  struct F_triple new_ent;

  if (ht == NULL)
    return false;

  new_ent.name = (char *) file;
  new_ent.st_ino = stats->st_ino;
  new_ent.st_dev = stats->st_dev;

  return !!hash_lookup (ht, &new_ent);
}

/* Record destination file, FILE, and dev/ino from *STATS,
   in the hash table, HT.  If HT is NULL, return immediately.
   If STATS is NULL, call lstat on FILE to get the device
   and inode numbers.  If that lstat fails, simply return.
   If memory allocation fails, exit immediately.  */
static void
record_file (Hash_table *ht, char const *file,
	     struct stat const *stats)
{
  struct F_triple *ent;

  if (ht == NULL)
    return;

  ent = xmalloc (sizeof *ent);
  ent->name = xstrdup (file);
  if (stats)
    {
      ent->st_ino = stats->st_ino;
      ent->st_dev = stats->st_dev;
    }
  else
    {
      struct stat sb;
      if (lstat (file, &sb) != 0)
	return;
      ent->st_ino = sb.st_ino;
      ent->st_dev = sb.st_dev;
    }

  {
    struct F_triple *ent_from_table = hash_insert (ht, ent);
    if (ent_from_table == NULL)
      {
	/* Insertion failed due to lack of memory.  */
	xalloc_die ();
      }

    if (ent_from_table != ent)
      {
	/* There was alread a matching entry in the table, so ENT was
	   not inserted.  Free it.  */
	triple_free (ent);
      }
  }
}

/* When effecting a move (e.g., for mv(1)), and given the name DST_NAME
   of the destination and a corresponding stat buffer, DST_SB, return
   true if the logical `move' operation should _not_ proceed.
   Otherwise, return false.
   Depending on options specified in X, this code may issue an
   interactive prompt asking whether it's ok to overwrite DST_NAME.  */
static bool
abandon_move (const struct cp_options *x,
              char const *dst_name,
              struct stat const *dst_sb)
{
  assert (x->move_mode);
  return (x->interactive == I_ALWAYS_NO
          || ((x->interactive == I_ASK_USER
               || (x->interactive == I_UNSPECIFIED
                   && x->stdin_tty
                   && UNWRITABLE (dst_name, dst_sb->st_mode)))
              && (overwrite_prompt (dst_name, dst_sb), 1)
              && ! yesno ()));
}

/* Print --verbose output on standard output, e.g. `new' -> `old'.
   If BACKUP_DST_NAME is non-NULL, then also indicate that it is
   the name of a backup file.  */
static void
emit_verbose (char const *src, char const *dst, char const *backup_dst_name)
{
  printf ("%s -> %s", quote_n (0, src), quote_n (1, dst));
  if (backup_dst_name)
    printf (_(" (backup: %s)"), quote (backup_dst_name));
  putchar ('\n');
}

/* Copy the file SRC_NAME to the file DST_NAME.  The files may be of
   any type.  NEW_DST should be true if the file DST_NAME cannot
   exist because its parent directory was just created; NEW_DST should
   be false if DST_NAME might already exist.  DEVICE is the device
   number of the parent directory, or 0 if the parent of this file is
   not known.  ANCESTORS points to a linked, null terminated list of
   devices and inodes of parent directories of SRC_NAME.  COMMAND_LINE_ARG
   is true iff SRC_NAME was specified on the command line.
   Set *COPY_INTO_SELF if SRC_NAME is a parent of (or the
   same as) DST_NAME; otherwise, clear it.
   Return true if successful.  */
static bool
copy_internal (char const *src_name, char const *dst_name,
	       bool new_dst,
	       dev_t device,
	       struct dir_list *ancestors,
	       const struct cp_options *x,
	       bool command_line_arg,
	       bool *copy_into_self,
	       bool *rename_succeeded)
{
  struct stat src_sb;
  struct stat dst_sb;
  mode_t src_mode;
  mode_t dst_mode IF_LINT (= 0);
  mode_t omitted_permissions;
  bool restore_dst_mode = false;
  char *earlier_file = NULL;
  char *dst_backup = NULL;
  bool backup_succeeded = false;
  bool delayed_ok;
  bool copied_as_regular = false;
  bool preserve_metadata;

  if (x->move_mode && rename_succeeded)
    *rename_succeeded = false;

  *copy_into_self = false;

  if (XSTAT (x, src_name, &src_sb) != 0)
    {
      error (0, errno, _("cannot stat %s"), quote (src_name));
      return false;
    }

  src_mode = src_sb.st_mode;

  if (S_ISDIR (src_mode) && !x->recursive)
    {
      error (0, 0, _("omitting directory %s"), quote (src_name));
      return false;
    }

  /* Detect the case in which the same source file appears more than
     once on the command line and no backup option has been selected.
     If so, simply warn and don't copy it the second time.
     This check is enabled only if x->src_info is non-NULL.  */
  if (command_line_arg)
    {
      if ( ! S_ISDIR (src_sb.st_mode)
	   && x->backup_type == no_backups
	   && seen_file (x->src_info, src_name, &src_sb))
	{
	  error (0, 0, _("warning: source file %s specified more than once"),
		 quote (src_name));
	  return true;
	}

      record_file (x->src_info, src_name, &src_sb);
    }

  if (!new_dst)
    {
      if (XSTAT (x, dst_name, &dst_sb) != 0)
	{
	  if (errno != ENOENT)
	    {
	      error (0, errno, _("cannot stat %s"), quote (dst_name));
	      return false;
	    }
	  else
	    {
	      new_dst = true;
	    }
	}
      else
	{ /* Here, we know that dst_name exists, at least to the point
	     that it is XSTAT'able.  */
	  bool return_now;
	  bool unlink_src;

	  if (! same_file_ok (src_name, &src_sb, dst_name, &dst_sb,
			      x, &return_now, &unlink_src))
	    {
	      error (0, 0, _("%s and %s are the same file"),
		     quote_n (0, src_name), quote_n (1, dst_name));
	      return false;
	    }

	  /* When there is an existing destination file, we may end up
	     returning early, and hence not copying/moving the file.
	     This may be due to an interactive `negative' reply to the
	     prompt about the existing file.  It may also be due to the
	     use of the --reply=no option.

	     cp and mv treat -i and -f differently.  */
	  if (x->move_mode)
	    {
	      if (abandon_move (x, dst_name, &dst_sb)
		  || (unlink_src && unlink (src_name) == 0))
		{
		  /* Pretend the rename succeeded, so the caller (mv)
		     doesn't end up removing the source file.  */
		  if (rename_succeeded)
		    *rename_succeeded = true;
		  if (unlink_src && x->verbose)
		    printf (_("removed %s\n"), quote (src_name));
		  return true;
		}
	      if (unlink_src)
		{
		  error (0, errno, _("cannot remove %s"), quote (src_name));
		  return false;
		}
	    }
	  else
	    {
	      if (! S_ISDIR (src_mode)
		  && (x->interactive == I_ALWAYS_NO
		      || (x->interactive == I_ASK_USER
			  && (overwrite_prompt (dst_name, &dst_sb), 1)
			  && ! yesno ())))
		return true;
	    }

	  if (return_now)
	    return true;

	  if (!S_ISDIR (dst_sb.st_mode))
	    {
	      if (S_ISDIR (src_mode))
		{
		  if (x->move_mode && x->backup_type != no_backups)
		    {
		      /* Moving a directory onto an existing
			 non-directory is ok only with --backup.  */
		    }
		  else
		    {
		      error (0, 0,
		       _("cannot overwrite non-directory %s with directory %s"),
			     quote_n (0, dst_name), quote_n (1, src_name));
		      return false;
		    }
		}

	      /* Don't let the user destroy their data, even if they try hard:
		 This mv command must fail (likewise for cp):
		   rm -rf a b c; mkdir a b c; touch a/f b/f; mv a/f b/f c
		 Otherwise, the contents of b/f would be lost.
		 In the case of `cp', b/f would be lost if the user simulated
		 a move using cp and rm.
		 Note that it works fine if you use --backup=numbered.  */
	      if (command_line_arg
		  && x->backup_type != numbered_backups
		  && seen_file (x->dest_info, dst_name, &dst_sb))
		{
		  error (0, 0,
			 _("will not overwrite just-created %s with %s"),
			 quote_n (0, dst_name), quote_n (1, src_name));
		  return false;
		}
	    }

	  if (!S_ISDIR (src_mode))
	    {
	      if (S_ISDIR (dst_sb.st_mode))
		{
		  if (x->move_mode && x->backup_type != no_backups)
		    {
		      /* Moving a non-directory onto an existing
			 directory is ok only with --backup.  */
		    }
		  else
		    {
		      error (0, 0,
			 _("cannot overwrite directory %s with non-directory"),
			     quote (dst_name));
		      return false;
		    }
		}

	      if (x->update)
		{
		  /* When preserving time stamps (but not moving within a file
		     system), don't worry if the destination time stamp is
		     less than the source merely because of time stamp
		     truncation.  */
		  int options = ((x->preserve_timestamps
				  && ! (x->move_mode
					&& dst_sb.st_dev == src_sb.st_dev))
				 ? UTIMECMP_TRUNCATE_SOURCE
				 : 0);

		  if (0 <= utimecmp (dst_name, &dst_sb, &src_sb, options))
		    {
		      /* We're using --update and the destination is not older
			 than the source, so do not copy or move.  Pretend the
			 rename succeeded, so the caller (if it's mv) doesn't
			 end up removing the source file.  */
		      if (rename_succeeded)
			*rename_succeeded = true;
		      return true;
		    }
		}
	    }

	  if (x->move_mode)
	    {
	      /* Don't allow user to move a directory onto a non-directory.  */
	      if (S_ISDIR (src_sb.st_mode) && !S_ISDIR (dst_sb.st_mode)
		  && x->backup_type == no_backups)
		{
		  error (0, 0,
		       _("cannot move directory onto non-directory: %s -> %s"),
			 quote_n (0, src_name), quote_n (0, dst_name));
		  return false;
		}
	    }

	  if (x->backup_type != no_backups
	      /* Don't try to back up a destination if the last
		 component of src_name is "." or "..".  */
	      && ! dot_or_dotdot (last_component (src_name))
	      /* Create a backup of each destination directory in move mode,
		 but not in copy mode.  FIXME: it might make sense to add an
		 option to suppress backup creation also for move mode.
		 That would let one use mv to merge new content into an
		 existing hierarchy.  */
	      && (x->move_mode || ! S_ISDIR (dst_sb.st_mode)))
	    {
	      char *tmp_backup = find_backup_file_name (dst_name,
							x->backup_type);

	      /* Detect (and fail) when creating the backup file would
		 destroy the source file.  Before, running the commands
		 cd /tmp; rm -f a a~; : > a; echo A > a~; cp --b=simple a~ a
		 would leave two zero-length files: a and a~.  */
	      /* FIXME: but simply change e.g., the final a~ to `./a~'
		 and the source will still be destroyed.  */
	      if (STREQ (tmp_backup, src_name))
		{
		  const char *fmt;
		  fmt = (x->move_mode
		 ? _("backing up %s would destroy source;  %s not moved")
		 : _("backing up %s would destroy source;  %s not copied"));
		  error (0, 0, fmt,
			 quote_n (0, dst_name),
			 quote_n (1, src_name));
		  free (tmp_backup);
		  return false;
		}

	      /* FIXME: use fts:
		 Using alloca for a file name that may be arbitrarily
		 long is not recommended.  In fact, even forming such a name
		 should be discouraged.  Eventually, this code will be rewritten
		 to use fts, so using alloca here will be less of a problem.  */
	      ASSIGN_STRDUPA (dst_backup, tmp_backup);
	      free (tmp_backup);
	      if (rename (dst_name, dst_backup) != 0)
		{
		  if (errno != ENOENT)
		    {
		      error (0, errno, _("cannot backup %s"), quote (dst_name));
		      return false;
		    }
		  else
		    {
		      dst_backup = NULL;
		    }
		}
	      else
		{
		  backup_succeeded = true;
		}
	      new_dst = true;
	    }
	  else if (! S_ISDIR (dst_sb.st_mode)
		   && (x->unlink_dest_before_opening
		       || (x->preserve_links && 1 < dst_sb.st_nlink)
		       || (!x->move_mode
			   && x->dereference == DEREF_NEVER
			   && S_ISLNK (src_sb.st_mode))
		       ))
	    {
	      if (unlink (dst_name) != 0 && errno != ENOENT)
		{
		  error (0, errno, _("cannot remove %s"), quote (dst_name));
		  return false;
		}
	      new_dst = true;
	      if (x->verbose)
		printf (_("removed %s\n"), quote (dst_name));
	    }
	}
    }

  /* If the source is a directory, we don't always create the destination
     directory.  So --verbose should not announce anything until we're
     sure we'll create a directory. */
  if (x->verbose && !S_ISDIR (src_mode))
    emit_verbose (src_name, dst_name, backup_succeeded ? dst_backup : NULL);

  /* Associate the destination file name with the source device and inode
     so that if we encounter a matching dev/ino pair in the source tree
     we can arrange to create a hard link between the corresponding names
     in the destination tree.

     Sometimes, when preserving links, we have to record dev/ino even
     though st_nlink == 1:
     - when in move_mode, since we may be moving a group of N hard-linked
	files (via two or more command line arguments) to a different
	partition; the links may be distributed among the command line
	arguments (possibly hierarchies) so that the link count of
	the final, once-linked source file is reduced to 1 when it is
	considered below.  But in this case (for mv) we don't need to
	incur the expense of recording the dev/ino => name mapping; all we
	really need is a lookup, to see if the dev/ino pair has already
	been copied.
     - when using -H and processing a command line argument;
	that command line argument could be a symlink pointing to another
	command line argument.  With `cp -H --preserve=link', we hard-link
	those two destination files.
     - likewise for -L except that it applies to all files, not just
	command line arguments.

     Also record directory dev/ino when using --recursive.  We'll use that
     info to detect this problem: cp -R dir dir.  FIXME-maybe: ideally,
     directory info would be recorded in a separate hash table, since
     such entries are useful only while a single command line hierarchy
     is being copied -- so that separate table could be cleared between
     command line args.  Using the same hash table to preserve hard
     links means that it may not be cleared.  */

  if (x->move_mode && src_sb.st_nlink == 1)
    {
	earlier_file = src_to_dest_lookup (src_sb.st_ino, src_sb.st_dev);
    }
  else if ((x->preserve_links
	    && (1 < src_sb.st_nlink
		|| (command_line_arg
		    && x->dereference == DEREF_COMMAND_LINE_ARGUMENTS)
		|| x->dereference == DEREF_ALWAYS))
	   || (x->recursive && S_ISDIR (src_mode)))
    {
      earlier_file = remember_copied (dst_name, src_sb.st_ino, src_sb.st_dev);
    }

  /* Did we copy this inode somewhere else (in this command line argument)
     and therefore this is a second hard link to the inode?  */

  if (earlier_file)
    {
      /* Avoid damaging the destination file system by refusing to preserve
	 hard-linked directories (which are found at least in Netapp snapshot
	 directories).  */
      if (S_ISDIR (src_mode))
	{
	  /* If src_name and earlier_file refer to the same directory entry,
	     then warn about copying a directory into itself.  */
	  if (same_name (src_name, earlier_file))
	    {
	      error (0, 0, _("cannot copy a directory, %s, into itself, %s"),
		     quote_n (0, top_level_src_name),
		     quote_n (1, top_level_dst_name));
	      *copy_into_self = true;
	      goto un_backup;
	    }
	  else if (x->dereference == DEREF_ALWAYS)
	    {
	      /* This happens when e.g., encountering a directory for the
		 second or subsequent time via symlinks when cp is invoked
		 with -R and -L.  E.g.,
		 rm -rf a b c d; mkdir a b c d; ln -s ../c a; ln -s ../c b;
		 cp -RL a b d
	      */
	    }
	  else
	    {
	      error (0, 0, _("will not create hard link %s to directory %s"),
		     quote_n (0, dst_name), quote_n (1, earlier_file));
	      goto un_backup;
	    }
	}
      else
	{
	  bool link_failed = (link (earlier_file, dst_name) != 0);

	  /* If the link failed because of an existing destination,
	     remove that file and then call link again.  */
	  if (link_failed && errno == EEXIST)
	    {
	      if (unlink (dst_name) != 0)
		{
		  error (0, errno, _("cannot remove %s"), quote (dst_name));
		  goto un_backup;
		}
	      if (x->verbose)
		printf (_("removed %s\n"), quote (dst_name));
	      link_failed = (link (earlier_file, dst_name) != 0);
	    }

	  if (link_failed)
	    {
	      error (0, errno, _("cannot create hard link %s to %s"),
		     quote_n (0, dst_name), quote_n (1, earlier_file));
	      goto un_backup;
	    }

	  return true;
	}
    }

  if (x->move_mode)
    {
      if (rename (src_name, dst_name) == 0)
	{
	  if (x->verbose && S_ISDIR (src_mode))
	    emit_verbose (src_name, dst_name,
			  backup_succeeded ? dst_backup : NULL);

	  if (rename_succeeded)
	    *rename_succeeded = true;

	  if (command_line_arg)
	    {
	      /* Record destination dev/ino/name, so that if we are asked
		 to overwrite that file again, we can detect it and fail.  */
	      /* It's fine to use the _source_ stat buffer (src_sb) to get the
	         _destination_ dev/ino, since the rename above can't have
		 changed those, and `mv' always uses lstat.
		 We could limit it further by operating
		 only on non-directories.  */
	      record_file (x->dest_info, dst_name, &src_sb);
	    }

	  return true;
	}

      /* FIXME: someday, consider what to do when moving a directory into
	 itself but when source and destination are on different devices.  */

      /* This happens when attempting to rename a directory to a
	 subdirectory of itself.  */
      if (errno == EINVAL)
	{
	  /* FIXME: this is a little fragile in that it relies on rename(2)
	     failing with a specific errno value.  Expect problems on
	     non-POSIX systems.  */
	  error (0, 0, _("cannot move %s to a subdirectory of itself, %s"),
		 quote_n (0, top_level_src_name),
		 quote_n (1, top_level_dst_name));

	  /* Note that there is no need to call forget_created here,
	     (compare with the other calls in this file) since the
	     destination directory didn't exist before.  */

	  *copy_into_self = true;
	  /* FIXME-cleanup: Don't return true here; adjust mv.c accordingly.
	     The only caller that uses this code (mv.c) ends up setting its
	     exit status to nonzero when copy_into_self is nonzero.  */
	  return true;
	}

      /* WARNING: there probably exist systems for which an inter-device
	 rename fails with a value of errno not handled here.
	 If/as those are reported, add them to the condition below.
	 If this happens to you, please do the following and send the output
	 to the bug-reporting address (e.g., in the output of cp --help):
	   touch k; perl -e 'rename "k","/tmp/k" or print "$!(",$!+0,")\n"'
	 where your current directory is on one partion and /tmp is the other.
	 Also, please try to find the E* errno macro name corresponding to
	 the diagnostic and parenthesized integer, and include that in your
	 e-mail.  One way to do that is to run a command like this
	   find /usr/include/. -type f \
	     | xargs grep 'define.*\<E[A-Z]*\>.*\<18\>' /dev/null
	 where you'd replace `18' with the integer in parentheses that
	 was output from the perl one-liner above.
	 If necessary, of course, change `/tmp' to some other directory.  */
      if (errno != EXDEV)
	{
	  /* There are many ways this can happen due to a race condition.
	     When something happens between the initial XSTAT and the
	     subsequent rename, we can get many different types of errors.
	     For example, if the destination is initially a non-directory
	     or non-existent, but it is created as a directory, the rename
	     fails.  If two `mv' commands try to rename the same file at
	     about the same time, one will succeed and the other will fail.
	     If the permissions on the directory containing the source or
	     destination file are made too restrictive, the rename will
	     fail.  Etc.  */
	  error (0, errno,
		 _("cannot move %s to %s"),
		 quote_n (0, src_name), quote_n (1, dst_name));
	  forget_created (src_sb.st_ino, src_sb.st_dev);
	  return false;
	}

      /* The rename attempt has failed.  Remove any existing destination
	 file so that a cross-device `mv' acts as if it were really using
	 the rename syscall.  */
      if (unlink (dst_name) != 0 && errno != ENOENT)
	{
	  error (0, errno,
	     _("inter-device move failed: %s to %s; unable to remove target"),
		 quote_n (0, src_name), quote_n (1, dst_name));
	  forget_created (src_sb.st_ino, src_sb.st_dev);
	  return false;
	}

      new_dst = true;
    }

  /* If the ownership might change, omit some permissions at first, so
     unauthorized users cannot nip in before the file has the right
     ownership.  */
  omitted_permissions =
    (x->preserve_ownership
     ? (x->set_mode ? x->mode : src_mode) & (S_IRWXG | S_IRWXO)
     : 0);

  delayed_ok = true;

  /* In certain modes (cp's --symbolic-link), and for certain file types
     (symlinks and hard links) it doesn't make sense to preserve metadata,
     or it's possible to preserve only some of it.
     In such cases, set this variable to zero.  */
  preserve_metadata = true;

  if (S_ISDIR (src_mode))
    {
      struct dir_list *dir;

      /* If this directory has been copied before during the
         recursion, there is a symbolic link to an ancestor
         directory of the symbolic link.  It is impossible to
         continue to copy this, unless we've got an infinite disk.  */

      if (is_ancestor (&src_sb, ancestors))
	{
	  error (0, 0, _("cannot copy cyclic symbolic link %s"),
		 quote (src_name));
	  goto un_backup;
	}

      /* Insert the current directory in the list of parents.  */

      dir = alloca (sizeof *dir);
      dir->parent = ancestors;
      dir->ino = src_sb.st_ino;
      dir->dev = src_sb.st_dev;

      if (new_dst || !S_ISDIR (dst_sb.st_mode))
	{
	  /* POSIX says mkdir's behavior is implementation-defined when
	     (src_mode & ~S_IRWXUGO) != 0.  However, common practice is
	     to ask mkdir to copy all the CHMOD_MODE_BITS, letting mkdir
	     decide what to do with S_ISUID | S_ISGID | S_ISVTX.  */
	  mode_t mkdir_mode =
	    ((x->set_mode ? x->mode : src_mode)
	     & CHMOD_MODE_BITS & ~omitted_permissions);
	  if (mkdir (dst_name, mkdir_mode) != 0)
	    {
	      error (0, errno, _("cannot create directory %s"),
		     quote (dst_name));
	      goto un_backup;
	    }

	  /* We need search and write permissions to the new directory
	     for writing the directory's contents. Check if these
	     permissions are there.  */

	  if (lstat (dst_name, &dst_sb) != 0)
	    {
	      error (0, errno, _("cannot stat %s"), quote (dst_name));
	      goto un_backup;
	    }
	  else if ((dst_sb.st_mode & S_IRWXU) != S_IRWXU)
	    {
	      /* Make the new directory searchable and writable.  */

	      dst_mode = dst_sb.st_mode;
	      restore_dst_mode = true;

	      if (lchmod (dst_name, dst_mode | S_IRWXU) != 0)
		{
		  error (0, errno, _("setting permissions for %s"),
			 quote (dst_name));
		  goto un_backup;
		}
	    }

	  /* Insert the created directory's inode and device
             numbers into the search structure, so that we can
             avoid copying it again.  */

	  remember_copied (dst_name, dst_sb.st_ino, dst_sb.st_dev);

	  if (x->verbose)
	    emit_verbose (src_name, dst_name, NULL);
	}

      /* Are we crossing a file system boundary?  */
      if (x->one_file_system && device != 0 && device != src_sb.st_dev)
	return true;

      /* Copy the contents of the directory.  */

      if (! copy_dir (src_name, dst_name, new_dst, &src_sb, dir, x,
		      copy_into_self))
	{
	  /* Don't just return here -- otherwise, the failure to read a
	     single file in a source directory would cause the containing
	     destination directory not to have owner/perms set properly.  */
	  delayed_ok = false;
	}
    }
  else if (x->symbolic_link)
    {
      preserve_metadata = false;

      if (*src_name != '/')
	{
	  /* Check that DST_NAME denotes a file in the current directory.  */
	  struct stat dot_sb;
	  struct stat dst_parent_sb;
	  char *dst_parent;
	  bool in_current_dir;

	  dst_parent = dir_name (dst_name);

	  in_current_dir = (STREQ (".", dst_parent)
			    /* If either stat call fails, it's ok not to report
			       the failure and say dst_name is in the current
			       directory.  Other things will fail later.  */
			    || stat (".", &dot_sb) != 0
			    || stat (dst_parent, &dst_parent_sb) != 0
			    || SAME_INODE (dot_sb, dst_parent_sb));
	  free (dst_parent);

	  if (! in_current_dir)
	    {
	      error (0, 0,
	   _("%s: can make relative symbolic links only in current directory"),
		     quote (dst_name));
	      goto un_backup;
	    }
	}
      if (symlink (src_name, dst_name) != 0)
	{
	  error (0, errno, _("cannot create symbolic link %s to %s"),
		 quote_n (0, dst_name), quote_n (1, src_name));
	  goto un_backup;
	}
    }

  else if (x->hard_link
#ifdef LINK_FOLLOWS_SYMLINKS
  /* A POSIX-conforming link syscall dereferences a symlink, yet cp,
     invoked with `--link --no-dereference', should not.  Thus, with
     a POSIX-conforming link system call, we can't use link() here,
     since that would create a hard link to the referent (effectively
     dereferencing the symlink), rather than to the symlink itself.
     We can approximate the desired behavior by skipping this hard-link
     creating block and instead copying the symlink, via the `S_ISLNK'-
     copying code below.
     When link operates on the symlinks themselves, we use this block
     and just call link().  */
	   && !(S_ISLNK (src_mode) && x->dereference == DEREF_NEVER)
#endif
	   )
    {
      preserve_metadata = false;
      if (link (src_name, dst_name))
	{
	  error (0, errno, _("cannot create link %s"), quote (dst_name));
	  goto un_backup;
	}
    }
  else if (S_ISREG (src_mode)
	   || (x->copy_as_regular && !S_ISLNK (src_mode)))
    {
      copied_as_regular = true;
      /* POSIX says the permission bits of the source file must be
	 used as the 3rd argument in the open call.  Historical
	 practice passed all the source mode bits to 'open', but the extra
	 bits were ignored, so it should be the same either way.  */
      if (! copy_reg (src_name, dst_name, x, src_mode & S_IRWXUGO,
		      omitted_permissions, &new_dst, &src_sb))
	goto un_backup;
    }
  else if (S_ISFIFO (src_mode))
    {
      /* Use mknod, rather than mkfifo, because the former preserves
	 the special mode bits of a fifo on Solaris 10, while mkfifo
	 does not.  */
      if (mknod (dst_name, src_mode & ~omitted_permissions, 0) != 0)
	{
	  error (0, errno, _("cannot create fifo %s"), quote (dst_name));
	  goto un_backup;
	}
    }
  else if (S_ISBLK (src_mode) || S_ISCHR (src_mode) || S_ISSOCK (src_mode))
    {
      if (mknod (dst_name, src_mode & ~omitted_permissions, src_sb.st_rdev)
	  != 0)
	{
	  error (0, errno, _("cannot create special file %s"),
		 quote (dst_name));
	  goto un_backup;
	}
    }
  else if (S_ISLNK (src_mode))
    {
      char *src_link_val = xreadlink (src_name, src_sb.st_size);
      if (src_link_val == NULL)
	{
	  error (0, errno, _("cannot read symbolic link %s"), quote (src_name));
	  goto un_backup;
	}

      if (symlink (src_link_val, dst_name) == 0)
	free (src_link_val);
      else
	{
	  int saved_errno = errno;
	  bool same_link = false;
	  if (x->update && !new_dst && S_ISLNK (dst_sb.st_mode)
	      && dst_sb.st_size == strlen (src_link_val))
	    {
	      /* See if the destination is already the desired symlink.
		 FIXME: This behavior isn't documented, and seems wrong
		 in some cases, e.g., if the destination symlink has the
		 wrong ownership, permissions, or time stamps.  */
	      char *dest_link_val = xreadlink (dst_name, dst_sb.st_size);
	      if (STREQ (dest_link_val, src_link_val))
		same_link = true;
	      free (dest_link_val);
	    }
	  free (src_link_val);

	  if (! same_link)
	    {
	      error (0, saved_errno, _("cannot create symbolic link %s"),
		     quote (dst_name));
	      goto un_backup;
	    }
	}

      /* There's no need to preserve timestamps or permissions.  */
      preserve_metadata = false;

      if (x->preserve_ownership)
	{
	  /* Preserve the owner and group of the just-`copied'
	     symbolic link, if possible.  */
#if HAVE_LCHOWN
	  if (lchown (dst_name, src_sb.st_uid, src_sb.st_gid) != 0
	      && ! chown_failure_ok (x))
	    {
	      error (0, errno, _("failed to preserve ownership for %s"),
		     dst_name);
	      goto un_backup;
	    }
#else
	  /* Can't preserve ownership of symlinks.
	     FIXME: maybe give a warning or even error for symlinks
	     in directories with the sticky bit set -- there, not
	     preserving owner/group is a potential security problem.  */
#endif
	}
    }
  else
    {
      error (0, 0, _("%s has unknown file type"), quote (src_name));
      goto un_backup;
    }

  if (command_line_arg)
    record_file (x->dest_info, dst_name, NULL);

  if ( ! preserve_metadata)
    return true;

  if (copied_as_regular)
    return delayed_ok;

  /* POSIX says that `cp -p' must restore the following:
     - permission bits
     - setuid, setgid bits
     - owner and group
     If it fails to restore any of those, we may give a warning but
     the destination must not be removed.
     FIXME: implement the above. */

  /* Adjust the times (and if possible, ownership) for the copy.
     chown turns off set[ug]id bits for non-root,
     so do the chmod last.  */

  if (x->preserve_timestamps)
    {
      struct timespec timespec[2];
      timespec[0] = get_stat_atime (&src_sb);
      timespec[1] = get_stat_mtime (&src_sb);

      if (utimens (dst_name, timespec) != 0)
	{
	  error (0, errno, _("preserving times for %s"), quote (dst_name));
	  if (x->require_preserve)
	    return false;
	}
    }

  /* Avoid calling chown if we know it's not necessary.  */
  if (x->preserve_ownership
      && (new_dst || !SAME_OWNER_AND_GROUP (src_sb, dst_sb)))
    {
      switch (set_owner (x, dst_name, -1, src_sb.st_uid, src_sb.st_gid))
	{
	case -1:
	  return false;

	case 0:
	  src_mode &= ~ (S_ISUID | S_ISGID | S_ISVTX);
	  break;
	}
    }

  set_author (dst_name, -1, &src_sb);

  if (x->preserve_mode || x->move_mode)
    {
      if (copy_acl (src_name, -1, dst_name, -1, src_mode) != 0
	  && x->require_preserve)
	return false;
    }
  else if (x->set_mode)
    {
      if (set_acl (dst_name, -1, x->mode) != 0)
	return false;
    }
  else
    {
      if (omitted_permissions)
	{
	  omitted_permissions &= ~ cached_umask ();

	  if (omitted_permissions && !restore_dst_mode)
	    {
	      /* Permissions were deliberately omitted when the file
		 was created due to security concerns.  See whether
		 they need to be re-added now.  It'd be faster to omit
		 the lstat, but deducing the current destination mode
		 is tricky in the presence of implementation-defined
		 rules for special mode bits.  */
	      if (new_dst && lstat (dst_name, &dst_sb) != 0)
		{
		  error (0, errno, _("cannot stat %s"), quote (dst_name));
		  return false;
		}
	      dst_mode = dst_sb.st_mode;
	      if (omitted_permissions & ~dst_mode)
		restore_dst_mode = true;
	    }
	}

      if (restore_dst_mode)
	{
	  if (lchmod (dst_name, dst_mode | omitted_permissions) != 0)
	    {
	      error (0, errno, _("preserving permissions for %s"),
		     quote (dst_name));
	      if (x->require_preserve)
		return false;
	    }
	}
    }

  return delayed_ok;

un_backup:

  /* We have failed to create the destination file.
     If we've just added a dev/ino entry via the remember_copied
     call above (i.e., unless we've just failed to create a hard link),
     remove the entry associating the source dev/ino with the
     destination file name, so we don't try to `preserve' a link
     to a file we didn't create.  */
  if (earlier_file == NULL)
    forget_created (src_sb.st_ino, src_sb.st_dev);

  if (dst_backup)
    {
      if (rename (dst_backup, dst_name) != 0)
	error (0, errno, _("cannot un-backup %s"), quote (dst_name));
      else
	{
	  if (x->verbose)
	    printf (_("%s -> %s (unbackup)\n"),
		    quote_n (0, dst_backup), quote_n (1, dst_name));
	}
    }
  return false;
}

static bool
valid_options (const struct cp_options *co)
{
  assert (co != NULL);
  assert (VALID_BACKUP_TYPE (co->backup_type));
  assert (VALID_SPARSE_MODE (co->sparse_mode));
  assert (!(co->hard_link && co->symbolic_link));
  return true;
}

/* Copy the file SRC_NAME to the file DST_NAME.  The files may be of
   any type.  NONEXISTENT_DST should be true if the file DST_NAME
   is known not to exist (e.g., because its parent directory was just
   created);  NONEXISTENT_DST should be false if DST_NAME might already
   exist.  OPTIONS is ... FIXME-describe
   Set *COPY_INTO_SELF if SRC_NAME is a parent of (or the
   same as) DST_NAME; otherwise, set clear it.
   Return true if successful.  */

extern bool
copy (char const *src_name, char const *dst_name,
      bool nonexistent_dst, const struct cp_options *options,
      bool *copy_into_self, bool *rename_succeeded)
{
  assert (valid_options (options));

  /* Record the file names: they're used in case of error, when copying
     a directory into itself.  I don't like to make these tools do *any*
     extra work in the common case when that work is solely to handle
     exceptional cases, but in this case, I don't see a way to derive the
     top level source and destination directory names where they're used.
     An alternative is to use COPY_INTO_SELF and print the diagnostic
     from every caller -- but I don't want to do that.  */
  top_level_src_name = src_name;
  top_level_dst_name = dst_name;

  return copy_internal (src_name, dst_name, nonexistent_dst, 0, NULL,
			options, true, copy_into_self, rename_succeeded);
}

/* Return true if this process has appropriate privileges to chown a
   file whose owner is not the effective user ID.  */

extern bool
chown_privileges (void)
{
#ifdef PRIV_FILE_CHOWN
  bool result;
  priv_set_t *pset = priv_allocset ();
  if (!pset)
    xalloc_die ();
  result = (getppriv (PRIV_EFFECTIVE, pset) == 0
	    && priv_ismember (pset, PRIV_FILE_CHOWN));
  priv_freeset (pset);
  return result;
#else
  return (geteuid () == 0);
#endif
}

/* Return true if it's OK for chown to fail, where errno is
   the error number that chown failed with and X is the copying
   option set.  */

extern bool
chown_failure_ok (struct cp_options const *x)
{
  /* If non-root uses -p, it's ok if we can't preserve ownership.
     But root probably wants to know, e.g. if NFS disallows it,
     or if the target system doesn't support file ownership.  */

  return ((errno == EPERM || errno == EINVAL) && !x->chown_privileges);
}

/* Return the user's umask, caching the result.  */

extern mode_t
cached_umask (void)
{
  static mode_t mask = -1;
  if (mask == (mode_t) -1)
    {
      mask = umask (0);
      umask (mask);
    }
  return mask;
}
