/*
 * JFFS2 -- Journalling Flash File System, Version 2.
 *
 * Copyright (C) 2001, 2002 Red Hat, Inc.
 *
 * Created by David Woodhouse <dwmw2@infradead.org>
 *
 * For licensing information, see the file 'LICENCE' in this directory.
 *
 * $Id$
 *
 */


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include "nodelist.h"

static void *jffs2_follow_link(struct dentry *dentry, struct nameidata *nd);

struct inode_operations jffs2_symlink_inode_operations =
{	
	.readlink =	generic_readlink,
	.follow_link =	jffs2_follow_link,
	.setattr =	jffs2_setattr
};

static void *jffs2_follow_link(struct dentry *dentry, struct nameidata *nd)
{
	struct jffs2_inode_info *f = JFFS2_INODE_INFO(dentry->d_inode);
	char *p = (char *)f->dents;
	
	/*
	 * We don't acquire the f->sem mutex here since the only data we
	 * use is f->dents which in case of the symlink inode points to the
	 * symlink's target path.
	 *
	 * 1. If we are here the inode has already built and f->dents has
	 * to point to the target path.
	 * 2. Nobody uses f->dents (if the inode is symlink's inode). The
	 * exception is inode freeing function which frees f->dents. But
	 * it can't be called while we are here and before VFS has
	 * stopped using our f->dents string which we provide by means of
	 * nd_set_link() call.
	 */
	
	if (!p) {
		printk(KERN_ERR "jffs2_follow_link(): can't find symlink taerget\n");
		p = ERR_PTR(-EIO);
	} else {
		D1(printk(KERN_DEBUG "jffs2_follow_link(): target path is '%s'\n", (char *) f->dents));
	}

	nd_set_link(nd, p);
	
	/*
	 * We unlock the f->sem mutex but VFS will use the f->dents string. This is safe
	 * since the only way that may cause f->dents to be changed is iput() operation.
	 * But VFS will not use f->dents after iput() has been called.
	 */
	return NULL;
}

