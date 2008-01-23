/* 
   Unix SMB/CIFS implementation.
   SMB parameters and setup
   Copyright (C) Andrew Tridgell 1992-1997
   Copyright (C) Luke Kenneth Casson Leighton 1996-1997
   Copyright (C) Paul Ashton 1997
   Copyright (C) Jeremy Allison 2000-2004
   
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
*/

#ifndef _NT_DOMAIN_H /* _NT_DOMAIN_H */
#define _NT_DOMAIN_H 

struct uuid {
	uint32 time_low;
	uint16 time_mid;
	uint16 time_hi_and_version;
	uint8  clock_seq[2];
	uint8  node[6];
};
#define UUID_SIZE 16

#define UUID_FLAT_SIZE 16
typedef struct uuid_flat {
	uint8 info[UUID_FLAT_SIZE];
} UUID_FLAT;

/* dce/rpc support */
#include "rpc_dce.h"

/* miscellaneous structures / defines */
#include "rpc_misc.h"

#include "rpc_creds.h"

#include "talloc.h"

/*
 * A bunch of stuff that was put into smb.h
 * in the NTDOM branch - it didn't belong there.
 */
 
typedef struct _prs_struct {
	BOOL io; /* parsing in or out of data stream */
	/* 
	 * If the (incoming) data is big-endian. On output we are
	 * always little-endian.
	 */ 
	BOOL bigendian_data;
	uint8 align; /* data alignment */
	BOOL is_dynamic; /* Do we own this memory or not ? */
	uint32 data_offset; /* Current working offset into data. */
	uint32 buffer_size; /* Current allocated size of the buffer. */
	uint32 grow_size; /* size requested via prs_grow() calls */
	char *data_p; /* The buffer itself. */
	TALLOC_CTX *mem_ctx; /* When unmarshalling, use this.... */
} prs_struct;

/*
 * Defines for io member of prs_struct.
 */

#define MARSHALL 0
#define UNMARSHALL 1

#define MARSHALLING(ps) (!(ps)->io)
#define UNMARSHALLING(ps) ((ps)->io)

#define RPC_BIG_ENDIAN 		1
#define RPC_LITTLE_ENDIAN	0

#define RPC_PARSE_ALIGN 4

typedef struct _output_data {
	/*
	 * Raw RPC output data. This does not include RPC headers or footers.
	 */
	prs_struct rdata;

	/* The amount of data sent from the current rdata struct. */
	uint32 data_sent_length;

	/*
	 * The current PDU being returned. This inclues
	 * headers, data and authentication footer.
	 */
	unsigned char current_pdu[MAX_PDU_FRAG_LEN];

	/* The amount of data in the current_pdu buffer. */
	uint32 current_pdu_len;

	/* The amount of data sent from the current PDU. */
	uint32 current_pdu_sent;
} output_data;

typedef struct _input_data {
	/*
	 * This is the current incoming pdu. The data here
	 * is collected via multiple writes until a complete
	 * pdu is seen, then the data is copied into the in_data
	 * structure. The maximum size of this is 0x1630 (MAX_PDU_FRAG_LEN).
	 */
	unsigned char current_in_pdu[MAX_PDU_FRAG_LEN];

	/*
	 * The amount of data needed to complete the in_pdu.
	 * If this is zero, then we are at the start of a new
	 * pdu.
	 */
	uint32 pdu_needed_len;

	/*
	 * The amount of data received so far in the in_pdu.
	 * If this is zero, then we are at the start of a new
	 * pdu.
	 */
	uint32 pdu_received_len;

	/*
	 * This is the collection of input data with all
	 * the rpc headers and auth footers removed.
	 * The maximum length of this (1Mb) is strictly enforced.
	 */
	prs_struct data;
} input_data;

/*
 * Handle database - stored per pipe.
 */

struct policy {
	struct policy *next, *prev;

	POLICY_HND pol_hnd;

	void *data_ptr;
	void (*free_fn)(void *);
};

struct handle_list {
	struct policy *Policy; 	/* List of policies. */
	size_t count;			/* Current number of handles. */
	size_t pipe_ref_count;	/* Number of pipe handles referring to this list. */
};

/* Domain controller authentication protocol info */
struct dcinfo {
	DOM_CHAL clnt_chal; /* Initial challenge received from client */
	DOM_CHAL srv_chal;  /* Initial server challenge */
	DOM_CRED clnt_cred; /* Last client credential */
	DOM_CRED srv_cred;  /* Last server credential */
 
	uchar  sess_key[8]; /* Session key */
	uchar  md4pw[16];   /* md4(machine password) */

	fstring mach_acct;  /* Machine name we've authenticated. */

	fstring remote_machine;  /* Machine name we've authenticated. */

	BOOL challenge_sent;
	BOOL got_session_key;
	BOOL authenticated;

};

typedef struct pipe_rpc_fns {

	struct pipe_rpc_fns *next, *prev;
	
	/* RPC function table associated with the current rpc_bind (associated by context) */
	
	struct api_struct *cmds;
	int n_cmds;
	uint32 context_id;
	
} PIPE_RPC_FNS;

/*
 * DCE/RPC-specific samba-internal-specific handling of data on
 * NamedPipes.
 */

typedef struct pipes_struct {
	struct pipes_struct *next, *prev;

	connection_struct *conn;
	uint16 vuid; /* points to the unauthenticated user that opened this pipe. */

	fstring name;
	fstring pipe_srv_name;
	
	/* linked list of rpc dispatch tables associated 
	   with the open rpc contexts */
	   
	PIPE_RPC_FNS *contexts;
	
	RPC_HDR hdr; /* Incoming RPC header. */
	RPC_HDR_REQ hdr_req; /* Incoming request header. */

	uint32 ntlmssp_chal_flags; /* Client challenge flags. */
	BOOL ntlmssp_auth_requested; /* If the client wanted authenticated rpc. */
	BOOL ntlmssp_auth_validated; /* If the client *got* authenticated rpc. */
	unsigned char challenge[8];
	unsigned char ntlmssp_hash[258];
	uint32 ntlmssp_seq_num;
	struct dcinfo dc; /* Keeps the creds data. */

	 /* Hmm. In my understanding the authentication happens
	    implicitly later, so there are no two stages for
	    schannel. */

	BOOL netsec_auth_validated;
	struct netsec_auth_struct netsec_auth;

	/*
	 * Windows user info.
	 */
	fstring user_name;
	fstring domain;
	fstring wks;

	/*
	 * Unix user name and credentials.
	 */

	fstring pipe_user_name;
	struct current_user pipe_user;

	DATA_BLOB session_key;

	/*
	 * Set to true when an RPC bind has been done on this pipe.
	 */
	
	BOOL pipe_bound;
	
	/*
	 * Set to true when we should return fault PDU's for everything.
	 */
	
	BOOL fault_state;

	/*
	 * Set to true when we should return fault PDU's for a bad handle.
	 */

	BOOL bad_handle_fault_state;
	
	/*
	 * Set to RPC_BIG_ENDIAN when dealing with big-endian PDU's
	 */
	
	BOOL endian;
	
	/*
	 * Struct to deal with multiple pdu inputs.
	 */

	input_data in_data;

	/*
	 * Struct to deal with multiple pdu outputs.
	 */

	output_data out_data;

	/* talloc context to use when allocating memory on this pipe. */
	TALLOC_CTX *mem_ctx;

	/* handle database to use on this pipe. */
	struct handle_list *pipe_handles;

} pipes_struct;

typedef struct smb_np_struct {
	struct smb_np_struct *next, *prev;
	int pnum;
	connection_struct *conn;
	uint16 vuid; /* points to the unauthenticated user that opened this pipe. */
	BOOL open; /* open connection */
	uint16 device_state;
	uint16 priority;
	fstring name;

	/* When replying to an SMBtrans, this is the maximum amount of
           data that can be sent in the initial reply. */
	int max_trans_reply;

	/*
	 * NamedPipe state information.
	 *
	 * (e.g. typecast a np_struct, above).
	 */
	void *np_state;

	/*
	 * NamedPipe functions, to be called to perform
	 * Named Pipe transactions on request from an
	 * SMB client.
	 */

	/* call to create a named pipe connection.
	 * returns: state information representing the connection.
	 *          is stored in np_state, above.
	 */
	void *   (*namedpipe_create)(char *pipe_name, 
					  connection_struct *conn, uint16 vuid);

	/* call to perform a write / read namedpipe transaction.
	 * TransactNamedPipe is weird: it returns whether there
	 * is more data outstanding to be read, and the
	 * caller is expected to take note and follow up with
	 * read requests.
	 */
	ssize_t  (*namedpipe_transact)(void *np_state,
	                               char *data, int len,
	                               char *rdata, int rlen,
	                               BOOL *pipe_outstanding);

	/* call to perform a write namedpipe operation
	 */
	ssize_t  (*namedpipe_write)(void * np_state,
	                            char *data, size_t n);

	/* call to perform a read namedpipe operation.
	 *
	 * NOTE: the only reason that the pipe_outstanding
	 * argument is here is because samba does not use
	 * the namedpipe_transact function yet: instead,
	 * it performs the same as what namedpipe_transact
	 * does - a write, followed by a read.
	 *
	 * when samba is modified to use namedpipe_transact,
	 * the pipe_outstanding argument may be removed.
	 */
	ssize_t  (*namedpipe_read)(void * np_state,
	                           char *data, size_t max_len,
	                           BOOL *pipe_outstanding);

	/* call to close a namedpipe.
	 * function is expected to perform all cleanups
	 * necessary, free all memory etc.
	 *
	 * returns True if cleanup was successful (not that
	 * we particularly care).
	 */
	BOOL     (*namedpipe_close)(void * np_state);

} smb_np_struct;

struct api_struct {  
	const char *name;
	uint8 opnum;
	BOOL (*fn) (pipes_struct *);
};

typedef struct {  
	uint32 rid;
	const char *name;
} rid_name;

/*
 * higher order functions for use with msrpc client code
 */

#define PRINT_INFO_FN(fn)\
        void (*fn)(const char*, uint32, uint32, void  *const *const)
#define JOB_INFO_FN(fn)\
        void (*fn)(const char*, const char*, uint32, uint32, void *const *const)

/* end higher order functions */


/* security descriptor structures */
#include "rpc_secdes.h"

/* pac */
#include "authdata.h"

/* different dce/rpc pipes */
#include "rpc_lsa.h"
#include "rpc_netlogon.h"
#include "rpc_reg.h"
#include "rpc_samr.h"
#include "rpc_srvsvc.h"
#include "rpc_wkssvc.h"
#include "rpc_spoolss.h"
#include "rpc_dfs.h"
#include "rpc_ds.h"
#include "rpc_echo.h"
#include "rpc_shutdown.h"

#endif /* _NT_DOMAIN_H */
