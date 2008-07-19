/* 
 *  Unix SMB/CIFS implementation.
 *  RPC Pipe client / server routines for Dfs
 *  Copyright (C) Andrew Tridgell              1992-1997,
 *  Copyright (C) Luke Kenneth Casson Leighton 1996-1997,
 *  Copyright (C) Shirish Kalele               2000.
 *  Copyright (C) Jeremy Allison				2001.
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* This is the implementation of the dfs pipe. */

#include "includes.h"
#include "nterr.h"

#undef DBGC_CLASS
#define DBGC_CLASS DBGC_RPC_SRV

#define MAX_MSDFS_JUNCTIONS 256

/* This function does not return a WERROR or NTSTATUS code but rather 1 if
   dfs exists, or 0 otherwise. */

uint32 _dfs_exist(pipes_struct *p, DFS_Q_DFS_EXIST *q_u, DFS_R_DFS_EXIST *r_u)
{
	if(lp_host_msdfs()) 
		return 1;
	else
		return 0;
}

WERROR _dfs_add(pipes_struct *p, DFS_Q_DFS_ADD* q_u, DFS_R_DFS_ADD *r_u)
{
	struct current_user user;
	struct junction_map jn;
	struct referral* old_referral_list = NULL;
	BOOL exists = False;

	pstring dfspath, servername, sharename;
	pstring altpath;

	get_current_user(&user,p);

	if (user.uid != 0) {
		DEBUG(10,("_dfs_add: uid != 0. Access denied.\n"));
		return WERR_ACCESS_DENIED;
	}

	unistr2_to_ascii(dfspath, &q_u->DfsEntryPath, sizeof(dfspath)-1);
	unistr2_to_ascii(servername, &q_u->ServerName, sizeof(servername)-1);
	unistr2_to_ascii(sharename, &q_u->ShareName, sizeof(sharename)-1);

	DEBUG(5,("init_reply_dfs_add: Request to add %s -> %s\\%s.\n",
		dfspath, servername, sharename));

	pstrcpy(altpath, servername);
	pstrcat(altpath, "\\");
	pstrcat(altpath, sharename);

	/* The following call can change the cwd. */
	if(get_referred_path(dfspath, &jn, NULL, NULL)) {
		exists = True;
		jn.referral_count += 1;
		old_referral_list = jn.referral_list;
	} else {
		jn.referral_count = 1;
	}

	vfs_ChDir(p->conn,p->conn->connectpath);

	jn.referral_list = TALLOC_ARRAY(p->mem_ctx, struct referral, jn.referral_count);
	if(jn.referral_list == NULL) {
		DEBUG(0,("init_reply_dfs_add: talloc failed for referral list!\n"));
		return WERR_DFS_INTERNAL_ERROR;
	}

	if(old_referral_list) {
		memcpy(jn.referral_list, old_referral_list, sizeof(struct referral)*jn.referral_count-1);
		SAFE_FREE(old_referral_list);
	}
  
	jn.referral_list[jn.referral_count-1].proximity = 0;
	jn.referral_list[jn.referral_count-1].ttl = REFERRAL_TTL;

	pstrcpy(jn.referral_list[jn.referral_count-1].alternate_path, altpath);
  
	if(!create_msdfs_link(&jn, exists)) {
		vfs_ChDir(p->conn,p->conn->connectpath);
		return WERR_DFS_CANT_CREATE_JUNCT;
	}
	vfs_ChDir(p->conn,p->conn->connectpath);

	return WERR_OK;
}

WERROR _dfs_remove(pipes_struct *p, DFS_Q_DFS_REMOVE *q_u, 
                   DFS_R_DFS_REMOVE *r_u)
{
	struct current_user user;
	struct junction_map jn;
	BOOL found = False;

	pstring dfspath, servername, sharename;
	pstring altpath;

	get_current_user(&user,p);

	if (user.uid != 0) {
		DEBUG(10,("_dfs_remove: uid != 0. Access denied.\n"));
		return WERR_ACCESS_DENIED;
	}

	unistr2_to_ascii(dfspath, &q_u->DfsEntryPath, sizeof(dfspath)-1);
	if(q_u->ptr_ServerName) {
		unistr2_to_ascii(servername, &q_u->ServerName, sizeof(servername)-1);
	}

	if(q_u->ptr_ShareName) {
		unistr2_to_ascii(sharename, &q_u->ShareName, sizeof(sharename)-1);
	}

	if(q_u->ptr_ServerName && q_u->ptr_ShareName) {
		pstrcpy(altpath, servername);
		pstrcat(altpath, "\\");
		pstrcat(altpath, sharename);
		strlower_m(altpath);
	}

	DEBUG(5,("init_reply_dfs_remove: Request to remove %s -> %s\\%s.\n",
		dfspath, servername, sharename));

	if(!get_referred_path(dfspath, &jn, NULL, NULL)) {
		return WERR_DFS_NO_SUCH_VOL;
	}

	/* if no server-share pair given, remove the msdfs link completely */
	if(!q_u->ptr_ServerName && !q_u->ptr_ShareName) {
		if(!remove_msdfs_link(&jn)) {
			vfs_ChDir(p->conn,p->conn->connectpath);
			return WERR_DFS_NO_SUCH_VOL;
		}
		vfs_ChDir(p->conn,p->conn->connectpath);
	} else {
		int i=0;
		/* compare each referral in the list with the one to remove */
		DEBUG(10,("altpath: .%s. refcnt: %d\n", altpath, jn.referral_count));
		for(i=0;i<jn.referral_count;i++) {
			pstring refpath;
			pstrcpy(refpath,jn.referral_list[i].alternate_path);
			trim_char(refpath, '\\', '\\');
			DEBUG(10,("_dfs_remove:  refpath: .%s.\n", refpath));
			if(strequal(refpath, altpath)) {
				*(jn.referral_list[i].alternate_path)='\0';
				DEBUG(10,("_dfs_remove: Removal request matches referral %s\n",
					refpath));
				found = True;
			}
		}

		if(!found) {
			return WERR_DFS_NO_SUCH_SHARE;
		}

		/* Only one referral, remove it */
		if(jn.referral_count == 1) {
			if(!remove_msdfs_link(&jn)) {
				vfs_ChDir(p->conn,p->conn->connectpath);
				return WERR_DFS_NO_SUCH_VOL;
			}
		} else {
			if(!create_msdfs_link(&jn, True)) { 
				vfs_ChDir(p->conn,p->conn->connectpath);
				return WERR_DFS_CANT_CREATE_JUNCT;
			}
		}
		vfs_ChDir(p->conn,p->conn->connectpath);
	}

	return WERR_OK;
}

static BOOL init_reply_dfs_info_1(struct junction_map* j, DFS_INFO_1* dfs1, int num_j)
{
	int i=0;
	for(i=0;i<num_j;i++) {
		pstring str;
		dfs1[i].ptr_entrypath = 1;
		slprintf(str, sizeof(pstring)-1, "\\\\%s\\%s\\%s", global_myname(), 
			j[i].service_name, j[i].volume_name);
		DEBUG(5,("init_reply_dfs_info_1: %d) initing entrypath: %s\n",i,str));
		init_unistr2(&dfs1[i].entrypath,str,UNI_STR_TERMINATE);
	}
	return True;
}

static BOOL init_reply_dfs_info_2(struct junction_map* j, DFS_INFO_2* dfs2, int num_j)
{
	int i=0;
	for(i=0;i<num_j;i++) {
		pstring str;
		dfs2[i].ptr_entrypath = 1;
		slprintf(str, sizeof(pstring)-1, "\\\\%s\\%s\\%s", global_myname(),
			j[i].service_name, j[i].volume_name);
		init_unistr2(&dfs2[i].entrypath, str, UNI_STR_TERMINATE);
		dfs2[i].ptr_comment = 0;
		dfs2[i].state = 1; /* set up state of dfs junction as OK */
		dfs2[i].num_storages = j[i].referral_count;
	}
	return True;
}

static BOOL init_reply_dfs_info_3(TALLOC_CTX *ctx, struct junction_map* j, DFS_INFO_3* dfs3, int num_j)
{
	int i=0,ii=0;
	for(i=0;i<num_j;i++) {
		pstring str;
		dfs3[i].ptr_entrypath = 1;
		if (j[i].volume_name[0] == '\0')
			slprintf(str, sizeof(pstring)-1, "\\\\%s\\%s",
				global_myname(), j[i].service_name);
		else
			slprintf(str, sizeof(pstring)-1, "\\\\%s\\%s\\%s", global_myname(),
				j[i].service_name, j[i].volume_name);

		init_unistr2(&dfs3[i].entrypath, str, UNI_STR_TERMINATE);
		dfs3[i].ptr_comment = 1;
		init_unistr2(&dfs3[i].comment, "", UNI_STR_TERMINATE);
		dfs3[i].state = 1;
		dfs3[i].num_storages = dfs3[i].num_storage_infos = j[i].referral_count;
		dfs3[i].ptr_storages = 1;
     
		/* also enumerate the storages */
		dfs3[i].storages = TALLOC_ARRAY(ctx, DFS_STORAGE_INFO, j[i].referral_count);
		if (!dfs3[i].storages)
			return False;

		memset(dfs3[i].storages, '\0', j[i].referral_count * sizeof(DFS_STORAGE_INFO));

		for(ii=0;ii<j[i].referral_count;ii++) {
			char* p; 
			pstring path;
			DFS_STORAGE_INFO* stor = &(dfs3[i].storages[ii]);
			struct referral* ref = &(j[i].referral_list[ii]);
	  
			pstrcpy(path, ref->alternate_path);
			trim_char(path,'\\','\0');
			p = strrchr_m(path,'\\');
			if(p==NULL) {
				DEBUG(4,("init_reply_dfs_info_3: invalid path: no \\ found in %s\n",path));
				continue;
			}
			*p = '\0';
			DEBUG(5,("storage %d: %s.%s\n",ii,path,p+1));
			stor->state = 2; /* set all storages as ONLINE */
			init_unistr2(&stor->servername, path, UNI_STR_TERMINATE);
			init_unistr2(&stor->sharename,  p+1, UNI_STR_TERMINATE);
			stor->ptr_servername = stor->ptr_sharename = 1;
		}
	}
	return True;
}

static WERROR init_reply_dfs_ctr(TALLOC_CTX *ctx, uint32 level, 
                                   DFS_INFO_CTR* ctr, struct junction_map* jn,
                                   int num_jn)
{
	/* do the levels */
	switch(level) {
	case 1:
		{
		DFS_INFO_1* dfs1;
		dfs1 = TALLOC_ARRAY(ctx, DFS_INFO_1, num_jn);
		if (!dfs1)
			return WERR_NOMEM;
		init_reply_dfs_info_1(jn, dfs1, num_jn);
		ctr->dfs.info1 = dfs1;
		break;
		}
	case 2:
		{
		DFS_INFO_2* dfs2;
		dfs2 = TALLOC_ARRAY(ctx, DFS_INFO_2, num_jn);
		if (!dfs2)
			return WERR_NOMEM;
		init_reply_dfs_info_2(jn, dfs2, num_jn);
		ctr->dfs.info2 = dfs2;
		break;
		}
	case 3:
		{
		DFS_INFO_3* dfs3;
		dfs3 = TALLOC_ARRAY(ctx, DFS_INFO_3, num_jn);
		if (!dfs3)
			return WERR_NOMEM;
		init_reply_dfs_info_3(ctx, jn, dfs3, num_jn);
		ctr->dfs.info3 = dfs3;
		break;
		}
	default:
		return WERR_INVALID_PARAM;
	}
	return WERR_OK;
}
      
WERROR _dfs_enum(pipes_struct *p, DFS_Q_DFS_ENUM *q_u, DFS_R_DFS_ENUM *r_u)
{
	uint32 level = q_u->level;
	struct junction_map jn[MAX_MSDFS_JUNCTIONS];
	int num_jn = 0;

	num_jn = enum_msdfs_links(jn);
	vfs_ChDir(p->conn,p->conn->connectpath);
    
	DEBUG(5,("make_reply_dfs_enum: %d junctions found in Dfs, doing level %d\n", num_jn, level));

	r_u->ptr_buffer = level;
	r_u->level = r_u->level2 = level;
	r_u->ptr_num_entries = r_u->ptr_num_entries2 = 1;
	r_u->num_entries = r_u->num_entries2 = num_jn;
	r_u->reshnd.ptr_hnd = 1;
	r_u->reshnd.handle = num_jn;
  
	r_u->ctr = TALLOC_P(p->mem_ctx, DFS_INFO_CTR);
	if (!r_u->ctr)
		return WERR_NOMEM;
	ZERO_STRUCTP(r_u->ctr);
	r_u->ctr->switch_value = level;
	r_u->ctr->num_entries = num_jn;
	r_u->ctr->ptr_dfs_ctr = 1;
  
	r_u->status = init_reply_dfs_ctr(p->mem_ctx, level, r_u->ctr, jn, num_jn);

	return r_u->status;
}
      
WERROR _dfs_get_info(pipes_struct *p, DFS_Q_DFS_GET_INFO *q_u, 
                     DFS_R_DFS_GET_INFO *r_u)
{
	UNISTR2* uni_path = &q_u->uni_path;
	uint32 level = q_u->level;
	int consumedcnt = sizeof(pstring);
	pstring path;
	struct junction_map jn;

	unistr2_to_ascii(path, uni_path, sizeof(path)-1);
	if(!create_junction(path, &jn))
		return WERR_DFS_NO_SUCH_SERVER;
  
	/* The following call can change the cwd. */
	if(!get_referred_path(path, &jn, &consumedcnt, NULL) || consumedcnt < strlen(path)) {
		vfs_ChDir(p->conn,p->conn->connectpath);
		return WERR_DFS_NO_SUCH_VOL;
	}

	vfs_ChDir(p->conn,p->conn->connectpath);
	r_u->level = level;
	r_u->ptr_ctr = 1;
	r_u->status = init_reply_dfs_ctr(p->mem_ctx, level, &r_u->ctr, &jn, 1);
  
	return r_u->status;
}
