/*
 * linux/fs/nfs/nfs4_fs.h
 *
 * Copyright (C) 2005 Trond Myklebust
 *
 * NFSv4-specific filesystem definitions and declarations
 */

#ifndef __LINUX_FS_NFS_NFS4_FS_H
#define __LINUX_FS_NFS_NFS4_FS_H

#ifdef CONFIG_NFS_V4

struct idmap;

/*
 * In a seqid-mutating op, this macro controls which error return
 * values trigger incrementation of the seqid.
 *
 * from rfc 3010:
 * The client MUST monotonically increment the sequence number for the
 * CLOSE, LOCK, LOCKU, OPEN, OPEN_CONFIRM, and OPEN_DOWNGRADE
 * operations.  This is true even in the event that the previous
 * operation that used the sequence number received an error.  The only
 * exception to this rule is if the previous operation received one of
 * the following errors: NFSERR_STALE_CLIENTID, NFSERR_STALE_STATEID,
 * NFSERR_BAD_STATEID, NFSERR_BAD_SEQID, NFSERR_BADXDR,
 * NFSERR_RESOURCE, NFSERR_NOFILEHANDLE.
 *
 */
#define seqid_mutating_err(err)       \
(((err) != NFSERR_STALE_CLIENTID) &&  \
 ((err) != NFSERR_STALE_STATEID)  &&  \
 ((err) != NFSERR_BAD_STATEID)    &&  \
 ((err) != NFSERR_BAD_SEQID)      &&  \
 ((err) != NFSERR_BAD_XDR)        &&  \
 ((err) != NFSERR_RESOURCE)       &&  \
 ((err) != NFSERR_NOFILEHANDLE))

enum nfs4_client_state {
	NFS4CLNT_OK  = 0,
};

/*
 * The nfs4_client identifies our client state to the server.
 */
struct nfs4_client {
	struct list_head	cl_servers;	/* Global list of servers */
	struct in_addr		cl_addr;	/* Server identifier */
	u64			cl_clientid;	/* constant */
	nfs4_verifier		cl_confirm;
	unsigned long		cl_state;

	u32			cl_lockowner_id;

	/*
	 * The following rwsem ensures exclusive access to the server
	 * while we recover the state following a lease expiration.
	 */
	struct rw_semaphore	cl_sem;

	struct list_head	cl_delegations;
	struct list_head	cl_state_owners;
	struct list_head	cl_unused;
	int			cl_nunused;
	spinlock_t		cl_lock;
	atomic_t		cl_count;

	struct rpc_clnt *	cl_rpcclient;
	struct rpc_cred *	cl_cred;

	struct list_head	cl_superblocks;	/* List of nfs_server structs */

	unsigned long		cl_lease_time;
	unsigned long		cl_last_renewal;
	struct work_struct	cl_renewd;
	struct work_struct	cl_recoverd;

	wait_queue_head_t	cl_waitq;
	struct rpc_wait_queue	cl_rpcwaitq;

	/* used for the setclientid verifier */
	struct timespec		cl_boot_time;

	/* idmapper */
	struct idmap *		cl_idmap;

	/* Our own IP address, as a null-terminated string.
	 * This is used to generate the clientid, and the callback address.
	 */
	char			cl_ipaddr[16];
	unsigned char		cl_id_uniquifier;
};

/*
 * NFS4 state_owners and lock_owners are simply labels for ordered
 * sequences of RPC calls. Their sole purpose is to provide once-only
 * semantics by allowing the server to identify replayed requests.
 *
 * The ->so_sema is held during all state_owner seqid-mutating operations:
 * OPEN, OPEN_DOWNGRADE, and CLOSE. Its purpose is to properly serialize
 * so_seqid.
 */
struct nfs4_state_owner {
	struct list_head     so_list;	 /* per-clientid list of state_owners */
	struct nfs4_client   *so_client;
	u32                  so_id;      /* 32-bit identifier, unique */
	struct semaphore     so_sema;
	u32                  so_seqid;   /* protected by so_sema */
	atomic_t	     so_count;

	struct rpc_cred	     *so_cred;	 /* Associated cred */
	struct list_head     so_states;
	struct list_head     so_delegations;
};

/*
 * struct nfs4_state maintains the client-side state for a given
 * (state_owner,inode) tuple (OPEN) or state_owner (LOCK).
 *
 * OPEN:
 * In order to know when to OPEN_DOWNGRADE or CLOSE the state on the server,
 * we need to know how many files are open for reading or writing on a
 * given inode. This information too is stored here.
 *
 * LOCK: one nfs4_state (LOCK) to hold the lock stateid nfs4_state(OPEN)
 */

struct nfs4_lock_state {
	struct list_head	ls_locks;	/* Other lock stateids */
	struct nfs4_state *	ls_state;	/* Pointer to open state */
	fl_owner_t		ls_owner;	/* POSIX lock owner */
#define NFS_LOCK_INITIALIZED 1
	int			ls_flags;
	u32			ls_seqid;
	u32			ls_id;
	nfs4_stateid		ls_stateid;
	atomic_t		ls_count;
};

/* bits for nfs4_state->flags */
enum {
	LK_STATE_IN_USE,
	NFS_DELEGATED_STATE,
};

struct nfs4_state {
	struct list_head open_states;	/* List of states for the same state_owner */
	struct list_head inode_states;	/* List of states for the same inode */
	struct list_head lock_states;	/* List of subservient lock stateids */

	struct nfs4_state_owner *owner;	/* Pointer to the open owner */
	struct inode *inode;		/* Pointer to the inode */

	unsigned long flags;		/* Do we hold any locks? */
	struct semaphore lock_sema;	/* Serializes file locking operations */
	spinlock_t state_lock;		/* Protects the lock_states list */

	nfs4_stateid stateid;

	unsigned int nreaders;
	unsigned int nwriters;
	int state;			/* State on the server (R,W, or RW) */
	atomic_t count;
};


struct nfs4_exception {
	long timeout;
	int retry;
};

struct nfs4_state_recovery_ops {
	int (*recover_open)(struct nfs4_state_owner *, struct nfs4_state *);
	int (*recover_lock)(struct nfs4_state *, struct file_lock *);
};

extern struct dentry_operations nfs4_dentry_operations;
extern struct inode_operations nfs4_dir_inode_operations;

/* inode.c */
extern ssize_t nfs4_getxattr(struct dentry *, const char *, void *, size_t);
extern int nfs4_setxattr(struct dentry *, const char *, const void *, size_t, int);
extern ssize_t nfs4_listxattr(struct dentry *, char *, size_t);


/* nfs4proc.c */
extern int nfs4_map_errors(int err);
extern int nfs4_proc_setclientid(struct nfs4_client *, u32, unsigned short);
extern int nfs4_proc_setclientid_confirm(struct nfs4_client *);
extern int nfs4_proc_async_renew(struct nfs4_client *);
extern int nfs4_proc_renew(struct nfs4_client *);
extern int nfs4_do_close(struct inode *inode, struct nfs4_state *state, mode_t mode);
extern struct inode *nfs4_atomic_open(struct inode *, struct dentry *, struct nameidata *);
extern int nfs4_open_revalidate(struct inode *, struct dentry *, int);

extern struct nfs4_state_recovery_ops nfs4_reboot_recovery_ops;
extern struct nfs4_state_recovery_ops nfs4_network_partition_recovery_ops;

extern const u32 nfs4_fattr_bitmap[2];
extern const u32 nfs4_statfs_bitmap[2];
extern const u32 nfs4_pathconf_bitmap[2];
extern const u32 nfs4_fsinfo_bitmap[2];

/* nfs4renewd.c */
extern void nfs4_schedule_state_renewal(struct nfs4_client *);
extern void nfs4_renewd_prepare_shutdown(struct nfs_server *);
extern void nfs4_kill_renewd(struct nfs4_client *);
extern void nfs4_renew_state(void *);

/* nfs4state.c */
extern void init_nfsv4_state(struct nfs_server *);
extern void destroy_nfsv4_state(struct nfs_server *);
extern struct nfs4_client *nfs4_get_client(struct in_addr *);
extern void nfs4_put_client(struct nfs4_client *clp);
extern int nfs4_init_client(struct nfs4_client *clp);
extern struct nfs4_client *nfs4_find_client(struct in_addr *);
extern u32 nfs4_alloc_lockowner_id(struct nfs4_client *);

extern struct nfs4_state_owner * nfs4_get_state_owner(struct nfs_server *, struct rpc_cred *);
extern void nfs4_put_state_owner(struct nfs4_state_owner *);
extern void nfs4_drop_state_owner(struct nfs4_state_owner *);
extern struct nfs4_state * nfs4_get_open_state(struct inode *, struct nfs4_state_owner *);
extern void nfs4_put_open_state(struct nfs4_state *);
extern void nfs4_close_state(struct nfs4_state *, mode_t);
extern struct nfs4_state *nfs4_find_state(struct inode *, struct rpc_cred *, mode_t mode);
extern void nfs4_increment_seqid(int status, struct nfs4_state_owner *sp);
extern void nfs4_schedule_state_recovery(struct nfs4_client *);
extern int nfs4_set_lock_state(struct nfs4_state *state, struct file_lock *fl);
extern void nfs4_increment_lock_seqid(int status, struct nfs4_lock_state *ls);
extern void nfs4_copy_stateid(nfs4_stateid *, struct nfs4_state *, fl_owner_t);

extern const nfs4_stateid zero_stateid;

/* nfs4xdr.c */
extern uint32_t *nfs4_decode_dirent(uint32_t *p, struct nfs_entry *entry, int plus);
extern struct rpc_procinfo nfs4_procedures[];

struct nfs4_mount_data;

/* callback_xdr.c */
extern struct svc_version nfs4_callback_version1;

#else

#define init_nfsv4_state(server)  do { } while (0)
#define destroy_nfsv4_state(server)       do { } while (0)
#define nfs4_put_state_owner(inode, owner) do { } while (0)
#define nfs4_put_open_state(state) do { } while (0)
#define nfs4_close_state(a, b) do { } while (0)

#endif /* CONFIG_NFS_V4 */
#endif /* __LINUX_FS_NFS_NFS4_FS.H */
