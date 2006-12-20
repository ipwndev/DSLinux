/*
 * swapfs.c --- swap ext2 filesystem data structures
 * 
 * Copyright (C) 1995, 1996 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <time.h>

#include "ext2_fs.h"
#include "ext2fs.h"

#ifdef EXT2FS_ENABLE_SWAPFS
void ext2fs_swap_super(struct ext2_super_block * sb)
{
	sb->s_inodes_count = ext2fs_swab32(sb->s_inodes_count);
	sb->s_blocks_count = ext2fs_swab32(sb->s_blocks_count);
	sb->s_r_blocks_count = ext2fs_swab32(sb->s_r_blocks_count);
	sb->s_free_blocks_count = ext2fs_swab32(sb->s_free_blocks_count);
	sb->s_free_inodes_count = ext2fs_swab32(sb->s_free_inodes_count);
	sb->s_first_data_block = ext2fs_swab32(sb->s_first_data_block);
	sb->s_log_block_size = ext2fs_swab32(sb->s_log_block_size);
	sb->s_log_frag_size = ext2fs_swab32(sb->s_log_frag_size);
	sb->s_blocks_per_group = ext2fs_swab32(sb->s_blocks_per_group);
	sb->s_frags_per_group = ext2fs_swab32(sb->s_frags_per_group);
	sb->s_inodes_per_group = ext2fs_swab32(sb->s_inodes_per_group);
	sb->s_mtime = ext2fs_swab32(sb->s_mtime);
	sb->s_wtime = ext2fs_swab32(sb->s_wtime);
	sb->s_mnt_count = ext2fs_swab16(sb->s_mnt_count);
	sb->s_max_mnt_count = ext2fs_swab16(sb->s_max_mnt_count);
	sb->s_magic = ext2fs_swab16(sb->s_magic);
	sb->s_state = ext2fs_swab16(sb->s_state);
	sb->s_errors = ext2fs_swab16(sb->s_errors);
	sb->s_minor_rev_level = ext2fs_swab16(sb->s_minor_rev_level);
	sb->s_lastcheck = ext2fs_swab32(sb->s_lastcheck);
	sb->s_checkinterval = ext2fs_swab32(sb->s_checkinterval);
	sb->s_creator_os = ext2fs_swab32(sb->s_creator_os);
	sb->s_rev_level = ext2fs_swab32(sb->s_rev_level);
	sb->s_def_resuid = ext2fs_swab16(sb->s_def_resuid);
	sb->s_def_resgid = ext2fs_swab16(sb->s_def_resgid);
	sb->s_first_ino = ext2fs_swab32(sb->s_first_ino);
	sb->s_inode_size = ext2fs_swab16(sb->s_inode_size);
	sb->s_block_group_nr = ext2fs_swab16(sb->s_block_group_nr);
	sb->s_feature_compat = ext2fs_swab32(sb->s_feature_compat);
	sb->s_feature_incompat = ext2fs_swab32(sb->s_feature_incompat);
	sb->s_feature_ro_compat = ext2fs_swab32(sb->s_feature_ro_compat);
	sb->s_algorithm_usage_bitmap = ext2fs_swab32(sb->s_algorithm_usage_bitmap);
	sb->s_journal_inum = ext2fs_swab32(sb->s_journal_inum);
	sb->s_journal_dev = ext2fs_swab32(sb->s_journal_dev);
	sb->s_last_orphan = ext2fs_swab32(sb->s_last_orphan);
}

void ext2fs_swap_group_desc(struct ext2_group_desc *gdp)
{
	gdp->bg_block_bitmap = ext2fs_swab32(gdp->bg_block_bitmap);
	gdp->bg_inode_bitmap = ext2fs_swab32(gdp->bg_inode_bitmap);
	gdp->bg_inode_table = ext2fs_swab32(gdp->bg_inode_table);
	gdp->bg_free_blocks_count = ext2fs_swab16(gdp->bg_free_blocks_count);
	gdp->bg_free_inodes_count = ext2fs_swab16(gdp->bg_free_inodes_count);
	gdp->bg_used_dirs_count = ext2fs_swab16(gdp->bg_used_dirs_count);
}

void ext2fs_swap_inode(ext2_filsys fs, struct ext2_inode *t,
		       struct ext2_inode *f, int hostorder)
{
	unsigned i;
	int islnk = 0;
	
	if (hostorder && LINUX_S_ISLNK(f->i_mode))
		islnk = 1;
	t->i_mode = ext2fs_swab16(f->i_mode);
	if (!hostorder && LINUX_S_ISLNK(t->i_mode))
		islnk = 1;
	t->i_uid = ext2fs_swab16(f->i_uid);
	t->i_size = ext2fs_swab32(f->i_size);
	t->i_atime = ext2fs_swab32(f->i_atime);
	t->i_ctime = ext2fs_swab32(f->i_ctime);
	t->i_mtime = ext2fs_swab32(f->i_mtime);
	t->i_dtime = ext2fs_swab32(f->i_dtime);
	t->i_gid = ext2fs_swab16(f->i_gid);
	t->i_links_count = ext2fs_swab16(f->i_links_count);
	t->i_blocks = ext2fs_swab32(f->i_blocks);
	t->i_flags = ext2fs_swab32(f->i_flags);
	if (!islnk || f->i_blocks) {
		for (i = 0; i < EXT2_N_BLOCKS; i++)
			t->i_block[i] = ext2fs_swab32(f->i_block[i]);
	} else if (t != f) {
		for (i = 0; i < EXT2_N_BLOCKS; i++)
			t->i_block[i] = f->i_block[i];
	}
	t->i_generation = ext2fs_swab32(f->i_generation);
	t->i_file_acl = ext2fs_swab32(f->i_file_acl);
	t->i_dir_acl = ext2fs_swab32(f->i_dir_acl);
	t->i_faddr = ext2fs_swab32(f->i_faddr);

	switch (fs->super->s_creator_os) {
	case EXT2_OS_LINUX:
		t->osd1.linux1.l_i_reserved1 =
			ext2fs_swab32(f->osd1.linux1.l_i_reserved1);
		t->osd2.linux2.l_i_frag = f->osd2.linux2.l_i_frag;
		t->osd2.linux2.l_i_fsize = f->osd2.linux2.l_i_fsize;
		t->osd2.linux2.i_pad1 = ext2fs_swab16(f->osd2.linux2.i_pad1);
		t->osd2.linux2.l_i_uid_high =
		  ext2fs_swab16 (f->osd2.linux2.l_i_uid_high);
		t->osd2.linux2.l_i_gid_high =
		  ext2fs_swab16 (f->osd2.linux2.l_i_gid_high);
		t->osd2.linux2.l_i_reserved2 =
			ext2fs_swab32(f->osd2.linux2.l_i_reserved2);
		break;
	case EXT2_OS_HURD:
		t->osd1.hurd1.h_i_translator =
		  ext2fs_swab32 (f->osd1.hurd1.h_i_translator);
		t->osd2.hurd2.h_i_frag = f->osd2.hurd2.h_i_frag;
		t->osd2.hurd2.h_i_fsize = f->osd2.hurd2.h_i_fsize;
		t->osd2.hurd2.h_i_mode_high =
		  ext2fs_swab16 (f->osd2.hurd2.h_i_mode_high);
		t->osd2.hurd2.h_i_uid_high =
		  ext2fs_swab16 (f->osd2.hurd2.h_i_uid_high);
		t->osd2.hurd2.h_i_gid_high =
		  ext2fs_swab16 (f->osd2.hurd2.h_i_gid_high);
		t->osd2.hurd2.h_i_author =
		  ext2fs_swab32 (f->osd2.hurd2.h_i_author);
		break;
	case EXT2_OS_MASIX:
		t->osd1.masix1.m_i_reserved1 =
			ext2fs_swab32(f->osd1.masix1.m_i_reserved1);
		t->osd2.masix2.m_i_frag = f->osd2.masix2.m_i_frag;
		t->osd2.masix2.m_i_fsize = f->osd2.masix2.m_i_fsize;
		t->osd2.masix2.m_pad1 = ext2fs_swab16(f->osd2.masix2.m_pad1);
		t->osd2.masix2.m_i_reserved2[0] =
			ext2fs_swab32(f->osd2.masix2.m_i_reserved2[0]);
		t->osd2.masix2.m_i_reserved2[1] =
			ext2fs_swab32(f->osd2.masix2.m_i_reserved2[1]);
		break;
	}
}
#endif
