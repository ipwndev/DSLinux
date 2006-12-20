/*
 * problem.h --- e2fsck problem error codes
 *
 * Copyright 1996 by Theodore Ts'o
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

typedef __u32 problem_t;

struct problem_context {
	errcode_t	errcode;
	ext2_ino_t ino, ino2, dir;
	struct ext2_inode *inode;
	struct ext2_dir_entry *dirent;
	blk_t	blk, blk2;
	e2_blkcnt_t	blkcount;
	int		group;
	__u64	num;
	const char *str;
};

/*
 * We define a set of "latch groups"; these are problems which are
 * handled as a set.  The user answers once for a particular latch
 * group.
 */
#define PR_LATCH_MASK	0x0ff0  /* Latch mask */
#define PR_LATCH_BLOCK	0x0010	/* Latch for illegal blocks (pass 1) */
#define PR_LATCH_BBLOCK	0x0020	/* Latch for bad block inode blocks (pass 1) */
#define PR_LATCH_IBITMAP 0x0030 /* Latch for pass 5 inode bitmap proc. */
#define PR_LATCH_BBITMAP 0x0040 /* Latch for pass 5 inode bitmap proc. */
#define PR_LATCH_RELOC	0x0050  /* Latch for superblock relocate hint */
#define PR_LATCH_DBLOCK	0x0060	/* Latch for pass 1b dup block headers */
#define PR_LATCH_LOW_DTIME 0x0070 /* Latch for pass1 orphaned list refugees */

#define PR_LATCH(x)	((((x) & PR_LATCH_MASK) >> 4) - 1)

/*
 * Latch group descriptor flags
 */
#define PRL_YES		0x0001	/* Answer yes */
#define PRL_NO		0x0002	/* Answer no */
#define PRL_LATCHED	0x0004	/* The latch group is latched */
#define PRL_SUPPRESS	0x0008	/* Suppress all latch group questions */

#define PRL_VARIABLE	0x000f	/* All the flags that need to be reset */

/*
 * Pre-Pass 1 errors
 */

/* Block bitmap not in group */
#define PR_0_BB_NOT_GROUP	0x000001

/* Inode bitmap not in group */
#define PR_0_IB_NOT_GROUP	0x000002

/* Inode table not in group */
#define PR_0_ITABLE_NOT_GROUP	0x000003

/* Superblock corrupt */
#define PR_0_SB_CORRUPT		0x000004

/* Filesystem size is wrong */
#define PR_0_FS_SIZE_WRONG	0x000005

/* Fragments not supported */
#define PR_0_NO_FRAGMENTS	0x000006

/* Bad blocks_per_group */
#define PR_0_BLOCKS_PER_GROUP	0x000007

/* Bad first_data_block */
#define PR_0_FIRST_DATA_BLOCK	0x000008

/* Adding UUID to filesystem */
#define PR_0_ADD_UUID		0x000009

/* Relocate hint */	
#define PR_0_RELOCATE_HINT	0x00000A

/* Miscellaneous superblock corruption */
#define PR_0_MISC_CORRUPT_SUPER	0x00000B
	
/* Error determing physical device size of filesystem */
#define PR_0_GETSIZE_ERROR	0x00000C

/* Inode count in the superblock incorrect */
#define PR_0_INODE_COUNT_WRONG	0x00000D

/* The Hurd does not support the filetype feature */
#define PR_0_HURD_CLEAR_FILETYPE 0x00000E

/* Journal inode is invalid */
#define PR_0_JOURNAL_BAD_INODE	0x00000F

/* The external journal has multiple filesystems (which we can't handle yet) */
#define PR_0_JOURNAL_UNSUPP_MULTIFS 0x000010

/* Can't find external journal */
#define PR_0_CANT_FIND_JOURNAL	0x000011

/* External journal has bad superblock */
#define PR_0_EXT_JOURNAL_BAD_SUPER 0x000012

/* Superblock has a bad journal UUID */
#define PR_0_JOURNAL_BAD_UUID	0x000013

/* Journal has an unknown superblock type */
#define PR_0_JOURNAL_UNSUPP_SUPER 0x000014

/* Journal superblock is corrupt */
#define PR_0_JOURNAL_BAD_SUPER	0x000015

/* Journal superblock is corrupt */
#define PR_0_JOURNAL_HAS_JOURNAL 0x000016

/* Superblock has recovery flag set but no journal */
#define PR_0_JOURNAL_RECOVER_SET 0x000017

/* Warning message about leaving data in the journal */
#define PR_0_JOURNAL_RESET_JOURNAL 0x000018

/* Superblock recovery flag clear - journal needs to be reset */
#define PR_0_JOURNAL_RESET_PROMPT 0x000019

/* Filesystem revision is 0, but feature flags are set */
#define PR_0_FS_REV_LEVEL	0x00001A

/* Clearing orphan inode */
#define PR_0_ORPHAN_CLEAR_INODE			0x000020
	
/* Illegal block found in orphaned inode */
#define PR_0_ORPHAN_ILLEGAL_BLOCK_NUM		0x000021

/* Already cleared block found in orphaned inode */
#define PR_0_ORPHAN_ALREADY_CLEARED_BLOCK	0x000022
	
/* Illegal orphan inode in superblock */
#define PR_0_ORPHAN_ILLEGAL_HEAD_INODE		0x000023

/* Illegal inode in orphaned inode list */
#define PR_0_ORPHAN_ILLEGAL_INODE 		0x000024

/* Journal has unsupported read-only feature - abort */
#define PR_0_JOURNAL_UNSUPP_ROCOMPAT		0x000025

/* Journal has unsupported incompatible feature - abort */
#define PR_0_JOURNAL_UNSUPP_INCOMPAT		0x000026

/* Journal has unsupported version number */
#define PR_0_JOURNAL_UNSUPP_VERSION		0x000027

/*
 * Pass 1 errors
 */

/* Pass 1: Checking inodes, blocks, and sizes */
#define PR_1_PASS_HEADER		0x010000

/* Root directory is not an inode */
#define PR_1_ROOT_NO_DIR		0x010001

/* Root directory has dtime set */
#define PR_1_ROOT_DTIME			0x010002

/* Reserved inode has bad mode */
#define PR_1_RESERVED_BAD_MODE		0x010003

/* Deleted inode has zero dtime */
#define PR_1_ZERO_DTIME			0x010004

/* Inode in use, but dtime set */
#define PR_1_SET_DTIME			0x010005

/* Zero-length directory */
#define PR_1_ZERO_LENGTH_DIR		0x010006

/* Block bitmap conflicts with some other fs block */
#define PR_1_BB_CONFLICT		0x010007

/* Inode bitmap conflicts with some other fs block */
#define PR_1_IB_CONFLICT		0x010008

/* Inode table conflicts with some other fs block */
#define PR_1_ITABLE_CONFLICT		0x010009

/* Block bitmap is on a bad block */
#define PR_1_BB_BAD_BLOCK		0x01000A

/* Inode bitmap is on a bad block */
#define PR_1_IB_BAD_BLOCK		0x01000B

/* Inode has incorrect i_size */
#define PR_1_BAD_I_SIZE			0x01000C

/* Inode has incorrect i_blocks */
#define PR_1_BAD_I_BLOCKS		0x01000D

/* Illegal block number in inode */
#define PR_1_ILLEGAL_BLOCK_NUM		0x01000E

/* Block number overlaps fs metadata */
#define PR_1_BLOCK_OVERLAPS_METADATA	0x01000F

/* Inode has illegal blocks (latch question) */
#define PR_1_INODE_BLOCK_LATCH		0x010010

/* Too many bad blocks in inode */
#define	PR_1_TOO_MANY_BAD_BLOCKS 	0x010011
	
/* Illegal block number in bad block inode */
#define PR_1_BB_ILLEGAL_BLOCK_NUM 	0x010012

/* Bad block inode has illegal blocks (latch question) */
#define PR_1_INODE_BBLOCK_LATCH		0x010013

/* Duplicate or bad blocks in use! */
#define PR_1_DUP_BLOCKS_PREENSTOP	0x010014
	
/* Bad block used as bad block indirect block */	  
#define PR_1_BBINODE_BAD_METABLOCK	0x010015

/* Inconsistency can't be fixed prompt */
#define PR_1_BBINODE_BAD_METABLOCK_PROMPT 0x010016
	
/* Bad primary block */
#define PR_1_BAD_PRIMARY_BLOCK		0x010017
		  
/* Bad primary block prompt */
#define PR_1_BAD_PRIMARY_BLOCK_PROMPT	0x010018

/* Bad primary superblock */
#define PR_1_BAD_PRIMARY_SUPERBLOCK	0x010019

/* Bad primary block group descriptors */
#define PR_1_BAD_PRIMARY_GROUP_DESCRIPTOR 0x01001A

/* Bad superblock in group */
#define PR_1_BAD_SUPERBLOCK		0x01001B

/* Bad block group descriptors in group */
#define PR_1_BAD_GROUP_DESCRIPTORS	0x01001C

/* Block claimed for no reason */	  
#define PR_1_PROGERR_CLAIMED_BLOCK	0x01001D

/* Error allocating blocks for relocating metadata */
#define PR_1_RELOC_BLOCK_ALLOCATE	0x01001E
		
/* Error allocating block buffer during relocation process */
#define PR_1_RELOC_MEMORY_ALLOCATE	0x01001F
		
/* Relocating metadata group information from X to Y */	
#define PR_1_RELOC_FROM_TO		0x010020
		
/* Relocating metatdata group information to X */
#define PR_1_RELOC_TO			0x010021
		
/* Block read error during relocation process */
#define PR_1_RELOC_READ_ERR		0x010022
		
/* Block write error during relocation process */
#define PR_1_RELOC_WRITE_ERR		0x010023

/* Error allocating inode bitmap */
#define PR_1_ALLOCATE_IBITMAP_ERROR	0x010024

/* Error allocating block bitmap */
#define PR_1_ALLOCATE_BBITMAP_ERROR	0x010025

/* Error allocating icount structure */
#define PR_1_ALLOCATE_ICOUNT		0x010026
	
/* Error allocating dbcount */
#define PR_1_ALLOCATE_DBCOUNT		0x010027

/* Error while scanning inodes */
#define PR_1_ISCAN_ERROR		0x010028

/* Error while iterating over blocks */
#define PR_1_BLOCK_ITERATE		0x010029

/* Error while storing inode count information */	  
#define PR_1_ICOUNT_STORE		0x01002A

/* Error while storing directory block information */	  
#define PR_1_ADD_DBLOCK			0x01002B

/* Error while reading inode (for clearing) */
#define PR_1_READ_INODE			0x01002C

/* Suppress messages prompt */
#define PR_1_SUPPRESS_MESSAGES		0x01002D

/* Imagic flag set on an inode when filesystem doesn't support it */
#define PR_1_SET_IMAGIC			0x01002F

/* Immutable flag set on a device or socket inode */
#define PR_1_SET_IMMUTABLE		0x010030

/* Compression flag set on a non-compressed filesystem */
#define PR_1_COMPR_SET			0x010031

/* Non-zero size on on device, fifo or socket inode */
#define PR_1_SET_NONZSIZE		0x010032

/* Filesystem revision is 0, but feature flags are set */
#define PR_1_FS_REV_LEVEL		0x010033

/* Journal inode not in use, needs clearing */
#define PR_1_JOURNAL_INODE_NOT_CLEAR	0x010034

/* Journal inode has wrong mode */
#define PR_1_JOURNAL_BAD_MODE		0x010035

/* Inode that was part of orphan linked list */
#define PR_1_LOW_DTIME			0x010036

/* Latch question which asks how to deal with low dtime inodes */
#define PR_1_ORPHAN_LIST_REFUGEES	0x010037

/* Error allocating refcount structure */
#define PR_1_ALLOCATE_REFCOUNT		0x010038
	
/* Error reading Extended Attribute block */
#define PR_1_READ_EA_BLOCK		0x010039

/* Invalid Extended Attribute block */
#define PR_1_BAD_EA_BLOCK		0x01003A

/* Error reading Extended Attribute block while fixing refcount -- abort */
#define PR_1_EXTATTR_READ_ABORT		0x01003B

/* Extended attribute reference count incorrect */
#define PR_1_EXTATTR_REFCOUNT		0x01003C

/* Error writing Extended Attribute block while fixing refcount */ 
#define PR_1_EXTATTR_WRITE		0x01003D

/* Multiple EA blocks not supported */
#define PR_1_EA_MULTI_BLOCK		0x01003E

/* Error allocating EA region allocation structure */
#define PR_1_EA_ALLOC_REGION		0x01003F
	
/* Error EA allocation collision */
#define PR_1_EA_ALLOC_COLLISION		0x010040
	
/* Bad extended attribute name */
#define PR_1_EA_BAD_NAME		0x010041

/* Bad extended attribute value */
#define PR_1_EA_BAD_VALUE		0x0100423
	
/*
 * Pass 1b errors
 */

/* Pass 1B: Rescan for duplicate/bad blocks */
#define PR_1B_PASS_HEADER	0x011000

/* Duplicate/bad block(s) header */
#define PR_1B_DUP_BLOCK_HEADER	0x011001

/* Duplicate/bad block(s) in inode */
#define PR_1B_DUP_BLOCK		0x011002

/* Duplicate/bad block(s) end */
#define PR_1B_DUP_BLOCK_END	0x011003
	
/* Error while scanning inodes */
#define PR_1B_ISCAN_ERROR	0x011004

/* Error allocating inode bitmap */
#define PR_1B_ALLOCATE_IBITMAP_ERROR 0x011005

/* Error while iterating over blocks */
#define PR_1B_BLOCK_ITERATE	0x0110006

	
/* Pass 1C: Scan directories for inodes with dup blocks. */
#define PR_1C_PASS_HEADER	0x012000


/* Pass 1D: Reconciling duplicate blocks */
#define PR_1D_PASS_HEADER	0x013000

/* File has duplicate blocks */
#define PR_1D_DUP_FILE		0x013001

/* List of files sharing duplicate blocks */	
#define PR_1D_DUP_FILE_LIST	0x013002

/* File sharing blocks with filesystem metadata  */	
#define PR_1D_SHARE_METADATA	0x013003

/* Report of how many duplicate/bad inodes */	
#define PR_1D_NUM_DUP_INODES	0x013004

/* Duplicated blocks already reassigned or cloned. */
#define PR_1D_DUP_BLOCKS_DEALT	0x013005

/* Clone duplicate/bad blocks? */
#define PR_1D_CLONE_QUESTION	0x013006

/* Delete file? */
#define PR_1D_DELETE_QUESTION	0x013007

/* Couldn't clone file (error) */
#define PR_1D_CLONE_ERROR	0x013008
		
/*
 * Pass 2 errors
 */

/* Pass 2: Checking directory structure */
#define PR_2_PASS_HEADER	0x020000

/* Bad inode number for '.' */
#define PR_2_BAD_INODE_DOT	0x020001

/* Directory entry has bad inode number */
#define PR_2_BAD_INO		0x020002

/* Directory entry has deleted or unused inode */
#define PR_2_UNUSED_INODE	0x020003

/* Directry entry is link to '.' */
#define PR_2_LINK_DOT		0x020004

/* Directory entry points to inode now located in a bad block */
#define PR_2_BB_INODE		0x020005

/* Directory entry contains a link to a directory */
#define PR_2_LINK_DIR		0x020006

/* Directory entry contains a link to the root directry */
#define PR_2_LINK_ROOT		0x020007

/* Directory entry has illegal characters in its name */
#define PR_2_BAD_NAME		0x020008

/* Missing '.' in directory inode */	  
#define PR_2_MISSING_DOT	0x020009

/* Missing '..' in directory inode */	  
#define PR_2_MISSING_DOT_DOT	0x02000A

/* First entry in directory inode doesn't contain '.' */
#define PR_2_1ST_NOT_DOT	0x02000B

/* Second entry in directory inode doesn't contain '..' */
#define PR_2_2ND_NOT_DOT_DOT	0x02000C

/* i_faddr should be zero */
#define PR_2_FADDR_ZERO		0x02000D

/* i_file_acl should be zero */
#define PR_2_FILE_ACL_ZERO	0x02000E

/* i_dir_acl should be zero */
#define PR_2_DIR_ACL_ZERO	0x02000F

/* i_frag should be zero */
#define PR_2_FRAG_ZERO		0x020010

/* i_fsize should be zero */
#define PR_2_FSIZE_ZERO		0x020011
		  
/* inode has bad mode */
#define PR_2_BAD_MODE		0x020012

/* directory corrupted */
#define PR_2_DIR_CORRUPTED	0x020013
		  
/* filename too long */
#define PR_2_FILENAME_LONG	0x020014
		  
/* Directory inode has a missing block (hole) */
#define PR_2_DIRECTORY_HOLE	0x020015

/* '.' is not NULL terminated */
#define PR_2_DOT_NULL_TERM	0x020016

/* '..' is not NULL terminated */
#define PR_2_DOT_DOT_NULL_TERM	0x020017

/* Illegal character device in inode */
#define PR_2_BAD_CHAR_DEV	0x020018

/* Illegal block device in inode */
#define PR_2_BAD_BLOCK_DEV	0x020019

/* Duplicate '.' entry */
#define PR_2_DUP_DOT		0x02001A

/* Duplicate '..' entry */
#define PR_2_DUP_DOT_DOT	0x02001B
	
/* Internal error: couldn't find dir_info */
#define PR_2_NO_DIRINFO		0x02001C

/* Final rec_len is wrong */
#define PR_2_FINAL_RECLEN	0x02001D

/* Error allocating icount structure */
#define PR_2_ALLOCATE_ICOUNT	0x02001E

/* Error iterating over directory blocks */
#define PR_2_DBLIST_ITERATE	0x02001F

/* Error reading directory block */
#define PR_2_READ_DIRBLOCK	0x020020

/* Error writing directory block */
#define PR_2_WRITE_DIRBLOCK	0x020021

/* Error allocating new directory block */
#define PR_2_ALLOC_DIRBOCK	0x020022

/* Error deallocating inode */
#define PR_2_DEALLOC_INODE	0x020023

/* Directory entry for '.' is big.  Split? */
#define PR_2_SPLIT_DOT		0x020024

/* Illegal FIFO */
#define PR_2_BAD_FIFO		0x020025

/* Illegal socket */
#define PR_2_BAD_SOCKET		0x020026

/* Directory filetype not set */
#define PR_2_SET_FILETYPE	0x020027

/* Directory filetype incorrect */
#define PR_2_BAD_FILETYPE	0x020028

/* Directory filetype set when it shouldn't be */
#define PR_2_CLEAR_FILETYPE	0x020029

/* Directory filename can't be zero-length  */
#define PR_2_NULL_NAME		0x020030

/* Invalid fast symlink size */
#define PR_2_SYMLINK_SIZE	0x020031

/* i_file_acl (extended attribute) is bad */
#define PR_2_FILE_ACL_BAD	0x020032

/* Filesystem contains large files, but has no such flag in sb */
#define PR_2_FEATURE_LARGE_FILES 0x020033

/*
 * Pass 3 errors
 */

/* Pass 3: Checking directory connectivity */
#define PR_3_PASS_HEADER		0x030000

/* Root inode not allocated */
#define PR_3_NO_ROOT_INODE		0x030001

/* No room in lost+found */
#define PR_3_EXPAND_LF_DIR		0x030002

/* Unconnected directory inode */
#define PR_3_UNCONNECTED_DIR		0x030003

/* /lost+found not found */
#define PR_3_NO_LF_DIR			0x030004

/* .. entry is incorrect */
#define PR_3_BAD_DOT_DOT		0x030005

/* Bad or non-existent /lost+found.  Cannot reconnect */
#define PR_3_NO_LPF			0x030006

/* Could not expand /lost+found */
#define PR_3_CANT_EXPAND_LPF		0x030007

/* Could not reconnect inode */
#define PR_3_CANT_RECONNECT		0x030008

/* Error while trying to find /lost+found */
#define PR_3_ERR_FIND_LPF		0x030009

/* Error in ext2fs_new_block while creating /lost+found */
#define PR_3_ERR_LPF_NEW_BLOCK		0x03000A

/* Error in ext2fs_new_inode while creating /lost+found */
#define PR_3_ERR_LPF_NEW_INODE		0x03000B

/* Error in ext2fs_new_dir_block while creating /lost+found */	  
#define PR_3_ERR_LPF_NEW_DIR_BLOCK	0x03000C
		  
/* Error while writing directory block for /lost+found */
#define PR_3_ERR_LPF_WRITE_BLOCK	0x03000D

/* Error while adjusting inode count */
#define PR_3_ADJUST_INODE		0x03000E

/* Couldn't fix parent directory -- error */
#define PR_3_FIX_PARENT_ERR		0x03000F

/* Couldn't fix parent directory -- couldn't find it */	  
#define PR_3_FIX_PARENT_NOFIND		0x030010
	
/* Error allocating inode bitmap */
#define PR_3_ALLOCATE_IBITMAP_ERROR	0x030011
		  
/* Error creating root directory */
#define PR_3_CREATE_ROOT_ERROR		0x030012

/* Error creating lost and found directory */
#define PR_3_CREATE_LPF_ERROR		0x030013

/* Root inode is not directory; aborting */
#define PR_3_ROOT_NOT_DIR_ABORT		0x030014

/* Cannot proceed without a root inode. */
#define PR_3_NO_ROOT_INODE_ABORT	0x030015

/* Internal error: couldn't find dir_info */
#define PR_3_NO_DIRINFO			0x030016

/* Lost+found is not a directory */
#define PR_3_LPF_NOTDIR			0x030017

/*
 * Pass 4 errors
 */

/* Pass 4: Checking reference counts */
#define PR_4_PASS_HEADER	0x040000

/* Unattached zero-length inode */
#define PR_4_ZERO_LEN_INODE	0x040001

/* Unattached inode */
#define PR_4_UNATTACHED_INODE	0x040002

/* Inode ref count wrong */
#define PR_4_BAD_REF_COUNT	0x040003

/* Inconsistent inode count information cached */
#define PR_4_INCONSISTENT_COUNT	0x040004

/*
 * Pass 5 errors
 */

/* Pass 5: Checking group summary information */
#define PR_5_PASS_HEADER		0x050000
	
/* Padding at end of inode bitmap is not set. */
#define PR_5_INODE_BMAP_PADDING		0x050001

/* Padding at end of block bitmap is not set. */
#define PR_5_BLOCK_BMAP_PADDING		0x050002

/* Block bitmap differences header */
#define PR_5_BLOCK_BITMAP_HEADER 	0x050003

/* Block not used, but marked in bitmap */
#define PR_5_UNUSED_BLOCK		0x050004
	  
/* Block used, but not marked used in bitmap */
#define PR_5_BLOCK_USED			0x050005

/* Block bitmap differences end */	  
#define PR_5_BLOCK_BITMAP_END		0x050006

/* Inode bitmap differences header */
#define PR_5_INODE_BITMAP_HEADER	0x050007

/* Inode not used, but marked in bitmap */
#define PR_5_UNUSED_INODE		0x050008
	  
/* Inode used, but not marked used in bitmap */
#define PR_5_INODE_USED			0x050009

/* Inode bitmap differences end */	  
#define PR_5_INODE_BITMAP_END		0x05000A

/* Free inodes count for group wrong */
#define PR_5_FREE_INODE_COUNT_GROUP	0x05000B

/* Directories count for group wrong */
#define PR_5_FREE_DIR_COUNT_GROUP	0x05000C

/* Free inodes count wrong */
#define PR_5_FREE_INODE_COUNT	0x05000D

/* Free blocks count for group wrong */
#define PR_5_FREE_BLOCK_COUNT_GROUP	0x05000E

/* Free blocks count wrong */
#define PR_5_FREE_BLOCK_COUNT		0x05000F

/* Programming error: bitmap endpoints don't match */
#define PR_5_BMAP_ENDPOINTS		0x050010

/* Internal error: fudging end of bitmap */
#define PR_5_FUDGE_BITMAP_ERROR		0x050011

/* Error copying in replacement inode bitmap */
#define PR_5_COPY_IBITMAP_ERROR		0x050012

/* Error copying in replacement block bitmap */
#define PR_5_COPY_BBITMAP_ERROR		0x050013

/*
 * Function declarations
 */
int fix_problem(e2fsck_t ctx, problem_t code, struct problem_context *pctx);
int end_problem_latch(e2fsck_t ctx, int mask);
int set_latch_flags(int mask, int setflags, int clearflags);
int get_latch_flags(int mask, int *value);
void clear_problem_context(struct problem_context *ctx);

/* message.c */
void print_e2fsck_message(e2fsck_t ctx, const char *msg,
			  struct problem_context *pctx, int first);

