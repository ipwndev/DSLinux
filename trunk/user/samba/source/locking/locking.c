/* 
   Unix SMB/CIFS implementation.
   Locking functions
   Copyright (C) Andrew Tridgell 1992-2000
   Copyright (C) Jeremy Allison 1992-2000
   
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
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Revision History:

   12 aug 96: Erik.Devriendt@te6.siemens.be
   added support for shared memory implementation of share mode locking

   May 1997. Jeremy Allison (jallison@whistle.com). Modified share mode
   locking to deal with multiple share modes per open file.

   September 1997. Jeremy Allison (jallison@whistle.com). Added oplock
   support.

   rewrtten completely to use new tdb code. Tridge, Dec '99

   Added POSIX locking support. Jeremy Allison (jeremy@valinux.com), Apr. 2000.
*/

#include "includes.h"
uint16 global_smbpid;

/* the locking database handle */
static TDB_CONTEXT *tdb;

struct locking_data {
        union {
                int num_share_mode_entries;
                share_mode_entry dummy; /* Needed for alignment. */
        } u;
        /* the following two entries are implicit
           share_mode_entry modes[num_share_mode_entries];
           char file_name[];
        */
};

/****************************************************************************
 Debugging aid :-).
****************************************************************************/

static const char *lock_type_name(enum brl_type lock_type)
{
	return (lock_type == READ_LOCK) ? "READ" : "WRITE";
}

/****************************************************************************
 Utility function called to see if a file region is locked.
****************************************************************************/

BOOL is_locked(files_struct *fsp,connection_struct *conn,
	       SMB_BIG_UINT count,SMB_BIG_UINT offset, 
	       enum brl_type lock_type)
{
	int snum = SNUM(conn);
	int strict_locking = lp_strict_locking(snum);
	BOOL ret;
	
	if (count == 0)
		return(False);

	if (!lp_locking(snum) || !strict_locking)
		return(False);

	if (strict_locking == Auto) {
		if  (EXCLUSIVE_OPLOCK_TYPE(fsp->oplock_type) && (lock_type == READ_LOCK || lock_type == WRITE_LOCK)) {
			DEBUG(10,("is_locked: optimisation - exclusive oplock on file %s\n", fsp->fsp_name ));
			ret = 0;
		} else if (LEVEL_II_OPLOCK_TYPE(fsp->oplock_type) && (lock_type == READ_LOCK)) {
			DEBUG(10,("is_locked: optimisation - level II oplock on file %s\n", fsp->fsp_name ));
			ret = 0;
		} else {
			ret = !brl_locktest(fsp->dev, fsp->inode, fsp->fnum,
				     global_smbpid, sys_getpid(), conn->cnum, 
				     offset, count, lock_type);
		}
	} else {
		ret = !brl_locktest(fsp->dev, fsp->inode, fsp->fnum,
				global_smbpid, sys_getpid(), conn->cnum,
				offset, count, lock_type);
	}

	DEBUG(10,("is_locked: brl start=%.0f len=%.0f %s for file %s\n",
			(double)offset, (double)count, ret ? "locked" : "unlocked",
			fsp->fsp_name ));

	/*
	 * There is no lock held by an SMB daemon, check to
	 * see if there is a POSIX lock from a UNIX or NFS process.
	 */

	if(!ret && lp_posix_locking(snum)) {
		ret = is_posix_locked(fsp, offset, count, lock_type);

		DEBUG(10,("is_locked: posix start=%.0f len=%.0f %s for file %s\n",
				(double)offset, (double)count, ret ? "locked" : "unlocked",
				fsp->fsp_name ));
	}

	return ret;
}

/****************************************************************************
 Utility function called by locking requests.
****************************************************************************/

static NTSTATUS do_lock(files_struct *fsp,connection_struct *conn, uint16 lock_pid,
		 SMB_BIG_UINT count,SMB_BIG_UINT offset,enum brl_type lock_type, BOOL *my_lock_ctx)
{
	NTSTATUS status = NT_STATUS_LOCK_NOT_GRANTED;

	if (!lp_locking(SNUM(conn)))
		return NT_STATUS_OK;

	/* NOTE! 0 byte long ranges ARE allowed and should be stored  */

	DEBUG(10,("do_lock: lock type %s start=%.0f len=%.0f requested for file %s\n",
		  lock_type_name(lock_type), (double)offset, (double)count, fsp->fsp_name ));

	if (OPEN_FSP(fsp) && fsp->can_lock && (fsp->conn == conn)) {
		status = brl_lock(fsp->dev, fsp->inode, fsp->fnum,
				  lock_pid, sys_getpid(), conn->cnum, 
				  offset, count, 
				  lock_type, my_lock_ctx);

		if (NT_STATUS_IS_OK(status) && lp_posix_locking(SNUM(conn))) {

			/*
			 * Try and get a POSIX lock on this range.
			 * Note that this is ok if it is a read lock
			 * overlapping on a different fd. JRA.
			 */

			if (!set_posix_lock(fsp, offset, count, lock_type)) {
				if (errno == EACCES || errno == EAGAIN)
					status = NT_STATUS_FILE_LOCK_CONFLICT;
				else
					status = map_nt_error_from_unix(errno);

				/*
				 * We failed to map - we must now remove the brl
				 * lock entry.
				 */
				(void)brl_unlock(fsp->dev, fsp->inode, fsp->fnum,
								lock_pid, sys_getpid(), conn->cnum, 
								offset, count, False,
								NULL, NULL);
			}
		}
	}

	return status;
}

/****************************************************************************
 Utility function called by locking requests. This is *DISGUSTING*. It also
 appears to be "What Windows Does" (tm). Andrew, ever wonder why Windows 2000
 is so slow on the locking tests...... ? This is the reason. Much though I hate
 it, we need this. JRA.
****************************************************************************/

NTSTATUS do_lock_spin(files_struct *fsp,connection_struct *conn, uint16 lock_pid,
		 SMB_BIG_UINT count,SMB_BIG_UINT offset,enum brl_type lock_type, BOOL *my_lock_ctx)
{
	int j, maxj = lp_lock_spin_count();
	int sleeptime = lp_lock_sleep_time();
	NTSTATUS status, ret;

	if (maxj <= 0)
		maxj = 1;

	ret = NT_STATUS_OK; /* to keep dumb compilers happy */

	for (j = 0; j < maxj; j++) {
		status = do_lock(fsp, conn, lock_pid, count, offset, lock_type, my_lock_ctx);
		if (!NT_STATUS_EQUAL(status, NT_STATUS_LOCK_NOT_GRANTED) &&
		    !NT_STATUS_EQUAL(status, NT_STATUS_FILE_LOCK_CONFLICT)) {
			return status;
		}
		/* if we do fail then return the first error code we got */
		if (j == 0) {
			ret = status;
			/* Don't spin if we blocked ourselves. */
			if (*my_lock_ctx)
				return ret;
		}
		if (sleeptime)
			sys_usleep(sleeptime);
	}
	return ret;
}

/* Struct passed to brl_unlock. */
struct posix_unlock_data_struct {
	files_struct *fsp;
	SMB_BIG_UINT offset;
	SMB_BIG_UINT count;
};

/****************************************************************************
 Function passed to brl_unlock to allow POSIX unlock to be done first.
****************************************************************************/

static void posix_unlock(void *pre_data)
{
	struct posix_unlock_data_struct *pdata = (struct posix_unlock_data_struct *)pre_data;

	if (lp_posix_locking(SNUM(pdata->fsp->conn)))
		release_posix_lock(pdata->fsp, pdata->offset, pdata->count);
}

/****************************************************************************
 Utility function called by unlocking requests.
****************************************************************************/

NTSTATUS do_unlock(files_struct *fsp,connection_struct *conn, uint16 lock_pid,
		   SMB_BIG_UINT count,SMB_BIG_UINT offset)
{
	BOOL ok = False;
	struct posix_unlock_data_struct posix_data;
	
	if (!lp_locking(SNUM(conn)))
		return NT_STATUS_OK;
	
	if (!OPEN_FSP(fsp) || !fsp->can_lock || (fsp->conn != conn)) {
		return NT_STATUS_INVALID_HANDLE;
	}
	
	DEBUG(10,("do_unlock: unlock start=%.0f len=%.0f requested for file %s\n",
		  (double)offset, (double)count, fsp->fsp_name ));

	/*
	 * Remove the existing lock record from the tdb lockdb
	 * before looking at POSIX locks. If this record doesn't
	 * match then don't bother looking to remove POSIX locks.
	 */

	posix_data.fsp = fsp;
	posix_data.offset = offset;
	posix_data.count = count;

	ok = brl_unlock(fsp->dev, fsp->inode, fsp->fnum,
			lock_pid, sys_getpid(), conn->cnum, offset, count,
			False, posix_unlock, (void *)&posix_data);
   
	if (!ok) {
		DEBUG(10,("do_unlock: returning ERRlock.\n" ));
		return NT_STATUS_RANGE_NOT_LOCKED;
	}
	return NT_STATUS_OK;
}

/****************************************************************************
 Remove any locks on this fd. Called from file_close().
****************************************************************************/

void locking_close_file(files_struct *fsp)
{
	pid_t pid = sys_getpid();

	if (!lp_locking(SNUM(fsp->conn)))
		return;

	/*
	 * Just release all the brl locks, no need to release individually.
	 */

	brl_close(fsp->dev, fsp->inode, pid, fsp->conn->cnum, fsp->fnum);

	if(lp_posix_locking(SNUM(fsp->conn))) {

	 	/* 
		 * Release all the POSIX locks.
		 */
		posix_locking_close_file(fsp);

	}
}

/****************************************************************************
 Initialise the locking functions.
****************************************************************************/

static int open_read_only;

BOOL locking_init(int read_only)
{
	brl_init(read_only);

	if (tdb)
		return True;

	tdb = tdb_open_log(lock_path("locking.tdb"), 
		       0, TDB_DEFAULT|(read_only?0x0:TDB_CLEAR_IF_FIRST), 
		       read_only?O_RDONLY:O_RDWR|O_CREAT,
		       0644);

	if (!tdb) {
		DEBUG(0,("ERROR: Failed to initialise locking database\n"));
		return False;
	}
	
	if (!posix_locking_init(read_only))
		return False;

	open_read_only = read_only;

	return True;
}

/*******************************************************************
 Deinitialize the share_mode management.
******************************************************************/

BOOL locking_end(void)
{

	brl_shutdown(open_read_only);
	if (tdb) {

		if (tdb_close(tdb) != 0)
			return False;
	}

	return True;
}

/*******************************************************************
 Form a static locking key for a dev/inode pair.
******************************************************************/

static TDB_DATA locking_key(SMB_DEV_T dev, SMB_INO_T inode)
{
	static struct locking_key key;
	TDB_DATA kbuf;

	memset(&key, '\0', sizeof(key));
	key.dev = dev;
	key.inode = inode;
	kbuf.dptr = (char *)&key;
	kbuf.dsize = sizeof(key);
	return kbuf;
}

static TDB_DATA locking_key_fsp(files_struct *fsp)
{
	return locking_key(fsp->dev, fsp->inode);
}

/*******************************************************************
 Lock a hash bucket entry.
******************************************************************/

BOOL lock_share_entry(connection_struct *conn,
		      SMB_DEV_T dev, SMB_INO_T inode)
{
	return tdb_chainlock(tdb, locking_key(dev, inode)) == 0;
}

/*******************************************************************
 Unlock a hash bucket entry.
******************************************************************/

void unlock_share_entry(connection_struct *conn,
			SMB_DEV_T dev, SMB_INO_T inode)
{
	tdb_chainunlock(tdb, locking_key(dev, inode));
}

/*******************************************************************
 Lock a hash bucket entry. use a fsp for convenience
******************************************************************/

BOOL lock_share_entry_fsp(files_struct *fsp)
{
	return tdb_chainlock(tdb, locking_key(fsp->dev, fsp->inode)) == 0;
}

/*******************************************************************
 Unlock a hash bucket entry.
******************************************************************/

void unlock_share_entry_fsp(files_struct *fsp)
{
	tdb_chainunlock(tdb, locking_key(fsp->dev, fsp->inode));
}

/*******************************************************************
 Print out a share mode.
********************************************************************/

char *share_mode_str(int num, share_mode_entry *e)
{
	static pstring share_str;

	slprintf(share_str, sizeof(share_str)-1, "share_mode_entry[%d]: \
pid = %lu, share_mode = 0x%x, desired_access = 0x%x, port = 0x%x, type= 0x%x, file_id = %lu, dev = 0x%x, inode = %.0f",
	num, (unsigned long)e->pid, e->share_mode, (unsigned int)e->desired_access, e->op_port, e->op_type, e->share_file_id,
	(unsigned int)e->dev, (double)e->inode );

	return share_str;
}

/*******************************************************************
 Print out a share mode table.
********************************************************************/

static void print_share_mode_table(struct locking_data *data)
{
	int num_share_modes = data->u.num_share_mode_entries;
	share_mode_entry *shares = (share_mode_entry *)(data + 1);
	int i;

	for (i = 0; i < num_share_modes; i++) {
		share_mode_entry *entry_p = &shares[i];
		DEBUG(10,("print_share_mode_table: %s\n", share_mode_str(i, entry_p) ));
	}
}

/*******************************************************************
 Get all share mode entries for a dev/inode pair.
********************************************************************/

int get_share_modes(connection_struct *conn, 
		    SMB_DEV_T dev, SMB_INO_T inode, 
		    share_mode_entry **pp_shares)
{
	TDB_DATA dbuf;
	struct locking_data *data;
	int num_share_modes;
	share_mode_entry *shares = NULL;
	TDB_DATA key = locking_key(dev, inode);
	*pp_shares = NULL;

	dbuf = tdb_fetch(tdb, key);
	if (!dbuf.dptr)
		return 0;

	data = (struct locking_data *)dbuf.dptr;
	num_share_modes = data->u.num_share_mode_entries;
	if(num_share_modes) {
		pstring fname;
		int i;
		int del_count = 0;

		shares = (share_mode_entry *)memdup(dbuf.dptr + sizeof(*data),	
						num_share_modes * sizeof(share_mode_entry));

		if (!shares) {
			SAFE_FREE(dbuf.dptr);
			return 0;
		}

		/* Save off the associated filename. */
		pstrcpy(fname, dbuf.dptr + sizeof(*data) + num_share_modes * sizeof(share_mode_entry));

		/*
		 * Ensure that each entry has a real process attached.
		 */

		for (i = 0; i < num_share_modes; ) {
			share_mode_entry *entry_p = &shares[i];
			if (process_exists(entry_p->pid)) {
				DEBUG(10,("get_share_modes: %s\n", share_mode_str(i, entry_p) ));
				i++;
			} else {
				DEBUG(10,("get_share_modes: deleted %s\n", share_mode_str(i, entry_p) ));
				if (num_share_modes - i - 1 > 0) {
					memcpy( &shares[i], &shares[i+1],
						sizeof(share_mode_entry) * (num_share_modes - i - 1));
				}
				num_share_modes--;
				del_count++;
			}
		}

		/* Did we delete any ? If so, re-store in tdb. */
		if (del_count) {
			data->u.num_share_mode_entries = num_share_modes;
			
			if (num_share_modes) {
				memcpy(dbuf.dptr + sizeof(*data), shares,
						num_share_modes * sizeof(share_mode_entry));
				/* Append the filename. */
				pstrcpy(dbuf.dptr + sizeof(*data) + num_share_modes * sizeof(share_mode_entry), fname);
			}

			/* The record has shrunk a bit */
			dbuf.dsize -= del_count * sizeof(share_mode_entry);

			if (data->u.num_share_mode_entries == 0) {
				if (tdb_delete(tdb, key) == -1) {
					SAFE_FREE(shares);
					SAFE_FREE(dbuf.dptr);
					return 0;
				}
			} else {
				if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1) {
					SAFE_FREE(shares);
					SAFE_FREE(dbuf.dptr);
					return 0;
				}
			}
		}
	}

	SAFE_FREE(dbuf.dptr);
	*pp_shares = shares;
	return num_share_modes;
}

/*******************************************************************
 Fill a share mode entry.
********************************************************************/

static void fill_share_mode(char *p, files_struct *fsp, uint16 port, uint16 op_type)
{
	share_mode_entry *e = (share_mode_entry *)p;
	void *x = &e->time; /* Needed to force alignment. p may not be aligned.... */

	memset(e, '\0', sizeof(share_mode_entry));
	e->pid = sys_getpid();
	e->share_mode = fsp->share_mode;
	e->desired_access = fsp->desired_access;
	e->op_port = port;
	e->op_type = op_type;
	memcpy(x, &fsp->open_time, sizeof(struct timeval));
	e->share_file_id = fsp->file_id;
	e->dev = fsp->dev;
	e->inode = fsp->inode;
}

/*******************************************************************
 Check if two share mode entries are identical, ignoring oplock 
 and port info and desired_access.
********************************************************************/

BOOL share_modes_identical( share_mode_entry *e1, share_mode_entry *e2)
{
#if 1 /* JRA PARANOIA TEST - REMOVE LATER */
	if (e1->pid == e2->pid &&
		e1->share_file_id == e2->share_file_id &&
		e1->dev == e2->dev &&
		e1->inode == e2->inode &&
		(e1->share_mode & ~DELETE_ON_CLOSE_FLAG) != (e2->share_mode & ~DELETE_ON_CLOSE_FLAG)) {
			DEBUG(0,("PANIC: share_modes_identical: share_mode missmatch (e1 = %u, e2 = %u). Logic error.\n",
				(unsigned int)(e1->share_mode & ~DELETE_ON_CLOSE_FLAG),
				(unsigned int)(e2->share_mode & ~DELETE_ON_CLOSE_FLAG) ));
		smb_panic("PANIC: share_modes_identical logic error.\n");
	}
#endif

	return (e1->pid == e2->pid &&
		(e1->share_mode & ~DELETE_ON_CLOSE_FLAG) == (e2->share_mode & ~DELETE_ON_CLOSE_FLAG) &&
		e1->dev == e2->dev &&
		e1->inode == e2->inode &&
		e1->share_file_id == e2->share_file_id );
}

/*******************************************************************
 Delete a specific share mode. Return the number
 of entries left, and a memdup'ed copy of the entry deleted (if required).
 Ignore if no entry deleted.
********************************************************************/

ssize_t del_share_entry( SMB_DEV_T dev, SMB_INO_T inode,
			share_mode_entry *entry, share_mode_entry **ppse)
{
	TDB_DATA dbuf;
	struct locking_data *data;
	int i, del_count=0;
	share_mode_entry *shares;
	ssize_t count = 0;
	TDB_DATA key = locking_key(dev, inode);

	if (ppse)
		*ppse = NULL;

	/* read in the existing share modes */
	dbuf = tdb_fetch(tdb, key);
	if (!dbuf.dptr)
		return -1;

	data = (struct locking_data *)dbuf.dptr;
	shares = (share_mode_entry *)(dbuf.dptr + sizeof(*data));

	/*
	 * Find any with this pid and delete it
	 * by overwriting with the rest of the data 
	 * from the record.
	 */

	DEBUG(10,("del_share_entry: num_share_modes = %d\n", data->u.num_share_mode_entries ));

	for (i=0;i<data->u.num_share_mode_entries;) {
		if (share_modes_identical(&shares[i], entry)) {
			DEBUG(10,("del_share_entry: deleted %s\n",
				share_mode_str(i, &shares[i]) ));
			if (ppse)
				*ppse = memdup(&shares[i], sizeof(*shares));
			data->u.num_share_mode_entries--;
			if ((dbuf.dsize - (sizeof(*data) + (i+1)*sizeof(*shares))) > 0) {
				memmove(&shares[i], &shares[i+1], 
					dbuf.dsize - (sizeof(*data) + (i+1)*sizeof(*shares)));
			}
			del_count++;

			DEBUG(10,("del_share_entry: deleting entry %d\n", i ));

		} else {
			i++;
		}
	}

	if (del_count) {
		/* the record may have shrunk a bit */
		dbuf.dsize -= del_count * sizeof(*shares);

		count = (ssize_t)data->u.num_share_mode_entries;

		/* store it back in the database */
		if (data->u.num_share_mode_entries == 0) {
			if (tdb_delete(tdb, key) == -1)
				count = -1;
		} else {
			if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1)
				count = -1;
		}
	}
	DEBUG(10,("del_share_entry: Remaining table.\n"));
	print_share_mode_table((struct locking_data *)dbuf.dptr);
	SAFE_FREE(dbuf.dptr);
	return count;
}

/*******************************************************************
 Del the share mode of a file for this process. Return the number
 of entries left, and a memdup'ed copy of the entry deleted.
********************************************************************/

ssize_t del_share_mode(files_struct *fsp, share_mode_entry **ppse)
{
	share_mode_entry entry;

	/*
	 * Fake up a share_mode_entry for comparisons.
	 */

	fill_share_mode((char *)&entry, fsp, 0, 0);
	return del_share_entry(fsp->dev, fsp->inode, &entry, ppse);
}

/*******************************************************************
 Set the share mode of a file. Return False on fail, True on success.
********************************************************************/

BOOL set_share_mode(files_struct *fsp, uint16 port, uint16 op_type)
{
	TDB_DATA dbuf;
	struct locking_data *data;
	char *p=NULL;
	int size;
	TDB_DATA key = locking_key_fsp(fsp);
	BOOL ret = True;
		
	/* read in the existing share modes if any */
	dbuf = tdb_fetch(tdb, key);
	if (!dbuf.dptr) {
		size_t offset;
		/* we'll need to create a new record */
		pstring fname;

		pstrcpy(fname, fsp->conn->connectpath);
		pstrcat(fname, "/");
		pstrcat(fname, fsp->fsp_name);

		size = sizeof(*data) + sizeof(share_mode_entry) + strlen(fname) + 1;
		p = (char *)SMB_MALLOC(size);
		if (!p)
			return False;
		data = (struct locking_data *)p;
		data->u.num_share_mode_entries = 1;
	
		DEBUG(10,("set_share_mode: creating entry for file %s. num_share_modes = 1\n",
			fsp->fsp_name ));

		offset = sizeof(*data) + sizeof(share_mode_entry);
		safe_strcpy(p + offset, fname, size - offset - 1);
		fill_share_mode(p + sizeof(*data), fsp, port, op_type);
		dbuf.dptr = p;
		dbuf.dsize = size;
		if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1)
			ret = False;

		print_share_mode_table((struct locking_data *)p);

		SAFE_FREE(p);
		return ret;
	}

	/* we're adding to an existing entry - this is a bit fiddly */
	data = (struct locking_data *)dbuf.dptr;

	data->u.num_share_mode_entries++;
	
	DEBUG(10,("set_share_mode: adding entry for file %s. new num_share_modes = %d\n",
		fsp->fsp_name, data->u.num_share_mode_entries ));

	size = dbuf.dsize + sizeof(share_mode_entry);
	p = SMB_MALLOC(size);
	if (!p) {
		SAFE_FREE(dbuf.dptr);
		return False;
	}
	memcpy(p, dbuf.dptr, sizeof(*data));
	fill_share_mode(p + sizeof(*data), fsp, port, op_type);
	memcpy(p + sizeof(*data) + sizeof(share_mode_entry), dbuf.dptr + sizeof(*data),
	       dbuf.dsize - sizeof(*data));
	SAFE_FREE(dbuf.dptr);
	dbuf.dptr = p;
	dbuf.dsize = size;
	if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1)
		ret = False;
	print_share_mode_table((struct locking_data *)p);
	SAFE_FREE(p);
	return ret;
}

/*******************************************************************
 A generic in-place modification call for share mode entries.
********************************************************************/

static BOOL mod_share_mode( SMB_DEV_T dev, SMB_INO_T inode, share_mode_entry *entry,
			   void (*mod_fn)(share_mode_entry *, SMB_DEV_T, SMB_INO_T, void *),
			   void *param)
{
	TDB_DATA dbuf;
	struct locking_data *data;
	int i;
	share_mode_entry *shares;
	BOOL need_store=False;
	BOOL ret = True;
	TDB_DATA key = locking_key(dev, inode);

	/* read in the existing share modes */
	dbuf = tdb_fetch(tdb, key);
	if (!dbuf.dptr)
		return False;

	data = (struct locking_data *)dbuf.dptr;
	shares = (share_mode_entry *)(dbuf.dptr + sizeof(*data));

	/* find any with our pid and call the supplied function */
	for (i=0;i<data->u.num_share_mode_entries;i++) {
		if (share_modes_identical(entry, &shares[i])) {
			mod_fn(&shares[i], dev, inode, param);
			need_store=True;
		}
	}

	/* if the mod fn was called then store it back */
	if (need_store) {
		if (data->u.num_share_mode_entries == 0) {
			if (tdb_delete(tdb, key) == -1)
				ret = False;
		} else {
			if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1)
				ret = False;
		}
	}

	SAFE_FREE(dbuf.dptr);
	return ret;
}

/*******************************************************************
 Static function that actually does the work for the generic function
 below.
********************************************************************/

static void remove_share_oplock_fn(share_mode_entry *entry, SMB_DEV_T dev, SMB_INO_T inode, 
                                   void *param)
{
	DEBUG(10,("remove_share_oplock_fn: removing oplock info for entry dev=%x ino=%.0f\n",
		  (unsigned int)dev, (double)inode ));
	/* Delete the oplock info. */
	entry->op_port = 0;
	entry->op_type = NO_OPLOCK;
}

/*******************************************************************
 Remove an oplock port and mode entry from a share mode.
********************************************************************/

BOOL remove_share_oplock(files_struct *fsp)
{
	share_mode_entry entry;
	/*
	 * Fake up an entry for comparisons...
	 */
	fill_share_mode((char *)&entry, fsp, 0, 0);
	return mod_share_mode(fsp->dev, fsp->inode, &entry, remove_share_oplock_fn, NULL);
}

/*******************************************************************
 Static function that actually does the work for the generic function
 below.
********************************************************************/

static void downgrade_share_oplock_fn(share_mode_entry *entry, SMB_DEV_T dev, SMB_INO_T inode, 
                                   void *param)
{
	DEBUG(10,("downgrade_share_oplock_fn: downgrading oplock info for entry dev=%x ino=%.0f\n",
		  (unsigned int)dev, (double)inode ));
	entry->op_type = LEVEL_II_OPLOCK;
}

/*******************************************************************
 Downgrade a oplock type from exclusive to level II.
********************************************************************/

BOOL downgrade_share_oplock(files_struct *fsp)
{
	share_mode_entry entry;
	/*
	 * Fake up an entry for comparisons...
	 */
	fill_share_mode((char *)&entry, fsp, 0, 0);
	return mod_share_mode(fsp->dev, fsp->inode, &entry, downgrade_share_oplock_fn, NULL);
}

/*******************************************************************
 Get/Set the delete on close flag in a set of share modes.
 Return False on fail, True on success.
********************************************************************/

BOOL modify_delete_flag( SMB_DEV_T dev, SMB_INO_T inode, BOOL delete_on_close)
{
	TDB_DATA dbuf;
	struct locking_data *data;
	int i;
	share_mode_entry *shares;
	TDB_DATA key = locking_key(dev, inode);

	/* read in the existing share modes */
	dbuf = tdb_fetch(tdb, key);
	if (!dbuf.dptr)
		return False;

	data = (struct locking_data *)dbuf.dptr;
	shares = (share_mode_entry *)(dbuf.dptr + sizeof(*data));

	/* Set/Unset the delete on close element. */
	for (i=0;i<data->u.num_share_mode_entries;i++,shares++) {
		shares->share_mode = (delete_on_close ?
                            (shares->share_mode | DELETE_ON_CLOSE_FLAG) :
                            (shares->share_mode & ~DELETE_ON_CLOSE_FLAG) );
	}

	/* store it back */
	if (data->u.num_share_mode_entries) {
		if (tdb_store(tdb, key, dbuf, TDB_REPLACE)==-1) {
			SAFE_FREE(dbuf.dptr);
			return False;
		}
	}

	SAFE_FREE(dbuf.dptr);
	return True;
}

/*******************************************************************
 Print out a deferred open entry.
********************************************************************/

char *deferred_open_str(int num, deferred_open_entry *e)
{
	static pstring de_str;

	slprintf(de_str, sizeof(de_str)-1, "deferred_open_entry[%d]: \
pid = %lu, mid = %u, dev = 0x%x, inode = %.0f, port = %u, time = [%u.%06u]",
		num, (unsigned long)e->pid, (unsigned int)e->mid, (unsigned int)e->dev, (double)e->inode,
		(unsigned int)e->port,
		(unsigned int)e->time.tv_sec, (unsigned int)e->time.tv_usec );

	return de_str;
}

/* Internal data structures for deferred opens... */

struct de_locking_key {
	char name[4];
	SMB_DEV_T dev;
	SMB_INO_T inode;
};

struct deferred_open_data {
        union {
                int num_deferred_open_entries;
                deferred_open_entry dummy; /* Needed for alignment. */
        } u;
        /* the following two entries are implicit
           deferred_open_entry de_entries[num_deferred_open_entries];
           char file_name[];
        */
};

/*******************************************************************
 Print out a deferred open table.
********************************************************************/

static void print_deferred_open_table(struct deferred_open_data *data)
{
	int num_de_entries = data->u.num_deferred_open_entries;
	deferred_open_entry *de_entries = (deferred_open_entry *)(data + 1);
	int i;

	for (i = 0; i < num_de_entries; i++) {
		deferred_open_entry *entry_p = &de_entries[i];
		DEBUG(10,("print_deferred_open_table: %s\n", deferred_open_str(i, entry_p) ));
	}
}


/*******************************************************************
 Form a static deferred open locking key for a dev/inode pair.
******************************************************************/

static TDB_DATA deferred_open_locking_key(SMB_DEV_T dev, SMB_INO_T inode)
{
	static struct de_locking_key key;
	TDB_DATA kbuf;

	memset(&key, '\0', sizeof(key));
	memcpy(&key.name[0], "DOE", 4);
	key.dev = dev;
	key.inode = inode;
	kbuf.dptr = (char *)&key;
	kbuf.dsize = sizeof(key);
	return kbuf;
}

/*******************************************************************
 Get all deferred open entries for a dev/inode pair.
********************************************************************/

int get_deferred_opens(connection_struct *conn, 
		    SMB_DEV_T dev, SMB_INO_T inode, 
		    deferred_open_entry **pp_de_entries)
{
	TDB_DATA dbuf;
	struct deferred_open_data *data;
	int num_de_entries;
	deferred_open_entry *de_entries = NULL;
	TDB_DATA key = deferred_open_locking_key(dev, inode);

	*pp_de_entries = NULL;

	dbuf = tdb_fetch(tdb, key);
	if (!dbuf.dptr)
		return 0;

	data = (struct deferred_open_data *)dbuf.dptr;
	num_de_entries = data->u.num_deferred_open_entries;
	if(num_de_entries) {
		pstring fname;
		int i;
		int del_count = 0;

		de_entries = (deferred_open_entry *)memdup(dbuf.dptr + sizeof(*data),	
						num_de_entries * sizeof(deferred_open_entry));

		if (!de_entries) {
			SAFE_FREE(dbuf.dptr);
			return 0;
		}

		/* Save off the associated filename. */
		pstrcpy(fname, dbuf.dptr + sizeof(*data) + num_de_entries * sizeof(deferred_open_entry));

		/*
		 * Ensure that each entry has a real process attached.
		 */

		for (i = 0; i < num_de_entries; ) {
			deferred_open_entry *entry_p = &de_entries[i];
			if (process_exists(entry_p->pid)) {
				DEBUG(10,("get_deferred_opens: %s\n", deferred_open_str(i, entry_p) ));
				i++;
			} else {
				DEBUG(10,("get_deferred_opens: deleted %s\n", deferred_open_str(i, entry_p) ));
				if (num_de_entries - i - 1 > 0) {
					memcpy( &de_entries[i], &de_entries[i+1],
						sizeof(deferred_open_entry) * (num_de_entries - i - 1));
				}
				num_de_entries--;
				del_count++;
			}
		}

		/* Did we delete any ? If so, re-store in tdb. */
		if (del_count) {
			data->u.num_deferred_open_entries = num_de_entries;
			
			if (num_de_entries) {
				memcpy(dbuf.dptr + sizeof(*data), de_entries,
						num_de_entries * sizeof(deferred_open_entry));
				/* Append the filename. */
				pstrcpy(dbuf.dptr + sizeof(*data) + num_de_entries * sizeof(deferred_open_entry), fname);
			}

			/* The record has shrunk a bit */
			dbuf.dsize -= del_count * sizeof(deferred_open_entry);

			if (data->u.num_deferred_open_entries == 0) {
				if (tdb_delete(tdb, key) == -1) {
					SAFE_FREE(de_entries);
					SAFE_FREE(dbuf.dptr);
					return 0;
				}
			} else {
				if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1) {
					SAFE_FREE(de_entries);
					SAFE_FREE(dbuf.dptr);
					return 0;
				}
			}
		}
	}

	SAFE_FREE(dbuf.dptr);
	*pp_de_entries = de_entries;
	return num_de_entries;
}

/*******************************************************************
 Check if two deferred open entries are identical.
********************************************************************/

static BOOL deferred_open_entries_identical( deferred_open_entry *e1, deferred_open_entry *e2)
{
	return (e1->pid == e2->pid &&
		e1->mid == e2->mid &&
		e1->port == e2->port &&
		e1->dev == e2->dev &&
		e1->inode == e2->inode &&
		e1->time.tv_sec == e2->time.tv_sec &&
		e1->time.tv_usec == e2->time.tv_usec);
}

/*******************************************************************
 Delete a specific deferred open entry.
 Ignore if no entry deleted.
********************************************************************/

BOOL delete_deferred_open_entry(deferred_open_entry *entry)
{
	TDB_DATA dbuf;
	struct deferred_open_data *data;
	int i, del_count=0;
	deferred_open_entry *de_entries;
	BOOL ret = True;
	TDB_DATA key = deferred_open_locking_key(entry->dev, entry->inode);

	/* read in the existing share modes */
	dbuf = tdb_fetch(tdb, key);
	if (!dbuf.dptr)
		return -1;

	data = (struct deferred_open_data *)dbuf.dptr;
	de_entries = (deferred_open_entry *)(dbuf.dptr + sizeof(*data));

	/*
	 * Find any with this pid and delete it
	 * by overwriting with the rest of the data 
	 * from the record.
	 */

	DEBUG(10,("delete_deferred_open_entry: num_deferred_open_entries = %d\n",
		data->u.num_deferred_open_entries ));

	for (i=0;i<data->u.num_deferred_open_entries;) {
		if (deferred_open_entries_identical(&de_entries[i], entry)) {
			DEBUG(10,("delete_deferred_open_entry: deleted %s\n",
				deferred_open_str(i, &de_entries[i]) ));

			data->u.num_deferred_open_entries--;
			if ((dbuf.dsize - (sizeof(*data) + (i+1)*sizeof(*de_entries))) > 0) {
				memmove(&de_entries[i], &de_entries[i+1], 
					dbuf.dsize - (sizeof(*data) + (i+1)*sizeof(*de_entries)));
			}
			del_count++;

			DEBUG(10,("delete_deferred_open_entry: deleting entry %d\n", i ));

		} else {
			i++;
		}
	}

	SMB_ASSERT(del_count == 0 || del_count == 1);

	if (del_count) {
		/* the record may have shrunk a bit */
		dbuf.dsize -= del_count * sizeof(*de_entries);

		/* store it back in the database */
		if (data->u.num_deferred_open_entries == 0) {
			if (tdb_delete(tdb, key) == -1)
				ret = False;
		} else {
			if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1)
				ret = False;
		}
	}
	DEBUG(10,("delete_deferred_open_entry: Remaining table.\n"));
	print_deferred_open_table((struct deferred_open_data*)dbuf.dptr);
	SAFE_FREE(dbuf.dptr);
	return ret;
}

/*******************************************************************
 Fill a deferred open entry.
********************************************************************/

static void fill_deferred_open(char *p, uint16 mid, struct timeval *ptv, SMB_DEV_T dev, SMB_INO_T inode, uint16 port)
{
	deferred_open_entry *e = (deferred_open_entry *)p;
	void *x = &e->time; /* Needed to force alignment. p may not be aligned.... */

	memset(e, '\0', sizeof(deferred_open_entry));
	e->mid = mid;
	e->pid = sys_getpid();
	memcpy(x, ptv, sizeof(struct timeval));
	e->dev = dev;
	e->inode = inode;
	e->port = port;
}

/*******************************************************************
 Add a deferred open record. Return False on fail, True on success.
********************************************************************/

BOOL add_deferred_open(uint16 mid, struct timeval *ptv, SMB_DEV_T dev, SMB_INO_T inode, uint16 port, const char *fname)
{
	TDB_DATA dbuf;
	struct deferred_open_data *data;
	char *p=NULL;
	int size;
	TDB_DATA key = deferred_open_locking_key(dev, inode);
	BOOL ret = True;
		
	/* read in the existing deferred open records if any */
	dbuf = tdb_fetch(tdb, key);
	if (!dbuf.dptr) {
		size_t offset;
		/* we'll need to create a new record */

		size = sizeof(*data) + sizeof(deferred_open_entry) + strlen(fname) + 1;
		p = (char *)SMB_MALLOC(size);
		if (!p)
			return False;
		data = (struct deferred_open_data *)p;
		data->u.num_deferred_open_entries = 1;
	
		DEBUG(10,("add_deferred_open: creating entry for file %s. num_deferred_open_entries = 1\n",
			fname ));

		offset = sizeof(*data) + sizeof(deferred_open_entry);
		safe_strcpy(p + offset, fname, size - offset - 1);
		fill_deferred_open(p + sizeof(*data), mid, ptv, dev, inode, port);
		dbuf.dptr = p;
		dbuf.dsize = size;
		if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1)
			ret = False;

		print_deferred_open_table((struct deferred_open_data *)p);

		SAFE_FREE(p);
		return ret;
	}

	/* we're adding to an existing entry - this is a bit fiddly */
	data = (struct deferred_open_data *)dbuf.dptr;

	data->u.num_deferred_open_entries++;
	
	DEBUG(10,("add_deferred_open: adding entry for file %s. new num_deferred_open_entries = %d\n",
		fname, data->u.num_deferred_open_entries ));

	size = dbuf.dsize + sizeof(deferred_open_entry);
	p = SMB_MALLOC(size);
	if (!p) {
		SAFE_FREE(dbuf.dptr);
		return False;
	}
	memcpy(p, dbuf.dptr, sizeof(*data));
	fill_deferred_open(p + sizeof(*data), mid, ptv, dev, inode, port);
	memcpy(p + sizeof(*data) + sizeof(deferred_open_entry), dbuf.dptr + sizeof(*data),
	       dbuf.dsize - sizeof(*data));
	SAFE_FREE(dbuf.dptr);
	dbuf.dptr = p;
	dbuf.dsize = size;
	if (tdb_store(tdb, key, dbuf, TDB_REPLACE) == -1)
		ret = False;
	print_deferred_open_table((struct deferred_open_data *)p);
	SAFE_FREE(p);
	return ret;
}

/****************************************************************************
 Traverse the whole database with this function, calling traverse_callback
 on each share mode
****************************************************************************/

static int traverse_fn(TDB_CONTEXT *the_tdb, TDB_DATA kbuf, TDB_DATA dbuf, 
                       void* state)
{
	struct locking_data *data;
	share_mode_entry *shares;
	char *name;
	int i;

	SHAREMODE_FN(traverse_callback) = (SHAREMODE_FN_CAST())state;

	/* Ensure this is a locking_key record. */
	if (kbuf.dsize != sizeof(struct locking_key))
		return 0;

	data = (struct locking_data *)dbuf.dptr;
	shares = (share_mode_entry *)(dbuf.dptr + sizeof(*data));
	name = dbuf.dptr + sizeof(*data) + data->u.num_share_mode_entries*sizeof(*shares);

	for (i=0;i<data->u.num_share_mode_entries;i++) {
		traverse_callback(&shares[i], name);
	}
	return 0;
}

/*******************************************************************
 Call the specified function on each entry under management by the
 share mode system.
********************************************************************/

int share_mode_forall(SHAREMODE_FN(fn))
{
	if (!tdb)
		return 0;
	return tdb_traverse(tdb, traverse_fn, (void*)fn);
}
