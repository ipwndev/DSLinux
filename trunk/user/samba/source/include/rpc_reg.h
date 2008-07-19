/* 
   Unix SMB/CIFS implementation.
   SMB parameters and setup
   Copyright (C) Andrew Tridgell                 1992-1997.
   Copyright (C) Luke Kenneth Casson Leighton    1996-1997.
   Copyright (C) Paul Ashton                          1997.
   Copyright (C) Gerald Carter                        2002.
   
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

#ifndef _RPC_REG_H /* _RPC_REG_H */
#define _RPC_REG_H 


/* winreg pipe defines 
   NOT IMPLEMENTED !!
#define _REG_UNK_01		0x01
#define _REG_UNK_03		0x03
#define REG_CREATE_KEY		0x06
#define REG_DELETE_KEY		0x07
#define REG_DELETE_VALUE	0x08
#define REG_FLUSH_KEY		0x0b
#define REG_GET_KEY_SEC		0x0c
#define	_REG_UNK_0D		0x0d
#define _REG_UNK_0E		0x0e
#define	_REG_UNK_12		0x12
#define _REG_UNK_13		0x13
#define REG_SET_KEY_SEC		0x15
#define REG_CREATE_VALUE	0x16
#define	_REG_UNK_17		0x17
*/

/* Implemented */
#define REG_OPEN_HKCR		0x00
#define REG_OPEN_HKLM		0x02
#define REG_OPEN_HKU		0x04
#define REG_CLOSE		0x05
#define REG_ENUM_KEY		0x09
#define REG_ENUM_VALUE		0x0a
#define REG_OPEN_ENTRY		0x0f
#define REG_QUERY_KEY		0x10
#define REG_INFO		0x11
#define REG_SHUTDOWN		0x18
#define REG_ABORT_SHUTDOWN	0x19
#define	REG_SAVE_KEY		0x14	/* no idea what the real name is */
#define REG_UNKNOWN_1A		0x1a


#define HKEY_CLASSES_ROOT	0x80000000
#define HKEY_CURRENT_USER	0x80000001
#define HKEY_LOCAL_MACHINE 	0x80000002
#define HKEY_USERS         	0x80000003

#define KEY_HKLM	"HKLM"
#define KEY_HKU		"HKU"
#define KEY_HKCR	"HKCR"
#define KEY_PRINTING 	"HKLM\\SYSTEM\\CurrentControlSet\\Control\\Print"
#define KEY_TREE_ROOT	""

/* Registry data types */

#define REG_NONE                       0
#define REG_SZ		               1
#define REG_EXPAND_SZ                  2
#define REG_BINARY 	               3
#define REG_DWORD	               4
#define REG_DWORD_LE	               4	/* DWORD, little endian */
#define REG_DWORD_BE	               5	/* DWORD, big endian */
#define REG_LINK                       6
#define REG_MULTI_SZ  	               7
#define REG_RESOURCE_LIST              8
#define REG_FULL_RESOURCE_DESCRIPTOR   9
#define REG_RESOURCE_REQUIREMENTS_LIST 10

/* structure to contain registry values */

typedef struct {
	fstring		valuename;
	uint16		type;
	uint32		size;	/* in bytes */
	uint8           *data_p;
} REGISTRY_VALUE;

/* container for regostry values */

typedef struct {
	TALLOC_CTX      *ctx;
	uint32          num_values;
	REGISTRY_VALUE	**values;
} REGVAL_CTR;

/* container for registry subkey names */

typedef struct {
	TALLOC_CTX	*ctx;
	uint32          num_subkeys;
	char            **subkeys;
} REGSUBKEY_CTR;


/* 
 * container for function pointers to enumeration routines
 * for vitural registry view 
 */ 
 
typedef struct {
	/* functions for enumerating subkeys and values */	
	int 	(*subkey_fn)( char *key, REGSUBKEY_CTR *subkeys);
	int 	(*value_fn) ( char *key, REGVAL_CTR *val );
	BOOL 	(*store_subkeys_fn)( char *key, REGSUBKEY_CTR *subkeys );
	BOOL 	(*store_values_fn)( char *key, REGVAL_CTR *val );
} REGISTRY_OPS;

typedef struct {
	const char	*keyname;	/* full path to name of key */
	REGISTRY_OPS	*ops;		/* registry function hooks */
} REGISTRY_HOOK;



/* structure to store the registry handles */

typedef struct _RegistryKey {

	struct _RegistryKey *prev, *next;

	POLICY_HND	hnd;
	pstring 	name; 	/* full name of registry key */
	REGISTRY_HOOK	*hook;
	
} REGISTRY_KEY;


/* REG_Q_OPEN_HKCR   */
typedef struct q_reg_open_hkcr_info
{
	uint32 ptr;
	uint16 unknown_0; /* 0x5428      - 16 bit unknown */
	uint16 unknown_1; /* random.  changes */
	uint32 level;     /* 0x0000 0002 - 32 bit unknown */

} REG_Q_OPEN_HKCR  ;

/* REG_R_OPEN_HKCR   */
typedef struct r_reg_open_hkcr_info
{
	POLICY_HND pol;       /* policy handle */
	WERROR status;         /* return status */

} REG_R_OPEN_HKCR;


/* REG_Q_OPEN_HKLM   */
typedef struct q_reg_open_hklm_info
{
	uint32 ptr;
	uint16 unknown_0;	/* 0xE084      - 16 bit unknown */
	uint16 unknown_1;	/* random.  changes */
	uint32 access_mask;

}
REG_Q_OPEN_HKLM;

/* REG_R_OPEN_HKLM   */
typedef struct r_reg_open_hklm_info
{
	POLICY_HND pol;		/* policy handle */
	WERROR status;		/* return status */

}
REG_R_OPEN_HKLM;


/* REG_Q_OPEN_HKU */
typedef struct q_reg_open_hku_info
{
	uint32 ptr;
	uint16 unknown_0; 
	uint16 unknown_1; 
	uint32 access_mask;    

} REG_Q_OPEN_HKU;

/* REG_R_OPEN_HKU */
typedef struct r_reg_open_hku_info
{
	POLICY_HND pol;      /* policy handle */
	WERROR status;     /* return status */

} REG_R_OPEN_HKU;


/* REG_Q_FLUSH_KEY */
typedef struct q_reg_open_flush_key_info
{
	POLICY_HND pol;       /* policy handle */

} REG_Q_FLUSH_KEY;

/* REG_R_FLUSH_KEY */
typedef struct r_reg_open_flush_key_info
{
	WERROR status;         /* return status */

} REG_R_FLUSH_KEY;


/* REG_Q_SET_KEY_SEC */
typedef struct q_reg_set_key_sec_info
{
	POLICY_HND pol;         /* policy handle */

	uint32 sec_info;       /* xxxx_SECURITY_INFORMATION */

	uint32 ptr;       /* pointer */
	BUFHDR hdr_sec;    /* header for security data */
	SEC_DESC_BUF *data;    /* security data */
	
} REG_Q_SET_KEY_SEC;

/* REG_R_SET_KEY_SEC */
typedef struct r_reg_set_key_sec_info
{
	WERROR status;
	
} REG_R_SET_KEY_SEC;


/* REG_Q_GET_KEY_SEC */
typedef struct q_reg_get_key_sec_info
{
	POLICY_HND pol;         /* policy handle */

	uint32 sec_info;       /* xxxx_SECURITY_INFORMATION */

	uint32 ptr;       /* pointer */
	BUFHDR hdr_sec;    /* header for security data */
	SEC_DESC_BUF *data;    /* security data */
	
} REG_Q_GET_KEY_SEC;

/* REG_R_GET_KEY_SEC */
typedef struct r_reg_get_key_sec_info
{
	uint32 sec_info;       /* xxxx_SECURITY_INFORMATION */

	uint32 ptr;       /* pointer */
	BUFHDR hdr_sec;    /* header for security data */
	SEC_DESC_BUF *data;    /* security data */

	WERROR status;
	
} REG_R_GET_KEY_SEC;

/* REG_Q_CREATE_VALUE */
typedef struct q_reg_create_value_info
{
	POLICY_HND pol;    /* policy handle */

	UNIHDR hdr_name;   /* name of value */
	UNISTR2 uni_name;

	uint32 type;       /* 1 = UNISTR, 3 = BYTES, 4 = DWORD, 7 = MULTI_UNISTR */

	BUFFER3 *buf_value; /* value, in byte buffer */

} REG_Q_CREATE_VALUE;

/* REG_R_CREATE_VALUE */
typedef struct r_reg_create_value_info
{ 
	WERROR status;         /* return status */

} REG_R_CREATE_VALUE;

/* REG_Q_ENUM_VALUE */
typedef struct q_reg_query_value_info
{
	POLICY_HND pol;    /* policy handle */

	uint32 val_index;  /* index */

	UNIHDR hdr_name;   /* name of value */
	UNISTR2 uni_name;

	uint32 ptr_type;   /* pointer */
	uint32 type;       /* 1 = UNISTR, 3 = BYTES, 4 = DWORD, 7 = MULTI_UNISTR */

	uint32 ptr_value;  /* pointer */
	BUFFER2 buf_value; /* value, in byte buffer */

	uint32 ptr1;       /* pointer */
	uint32 len_value1; /* */

	uint32 ptr2;       /* pointer */
	uint32 len_value2; /* */


} REG_Q_ENUM_VALUE;

/* REG_R_ENUM_VALUE */
typedef struct r_reg_enum_value_info
{ 
	UNIHDR hdr_name;        /* name of value */
	UNISTR2 uni_name;

	uint32 ptr_type;            /* pointer */
	uint32 type;        /* 1 = UNISTR, 3 = BYTES, 4 = DWORD, 7 = MULTI_UNISTR */

	uint32 ptr_value;       /* pointer */
	BUFFER2 buf_value;    /* value, in byte buffer */

	uint32 ptr1;            /* pointer */
	uint32 len_value1;       /* */

	uint32 ptr2;            /* pointer */
	uint32 len_value2;       /* */

	WERROR status;         /* return status */

} REG_R_ENUM_VALUE;

/* REG_Q_CREATE_KEY */
typedef struct q_reg_create_key_info
{
	POLICY_HND pnt_pol;       /* parent key policy handle */

	UNIHDR hdr_name;
	UNISTR2 uni_name;

	UNIHDR hdr_class;
	UNISTR2 uni_class;

	uint32 reserved; /* 0x0000 0000 */
	SEC_ACCESS sam_access; /* access rights flags, see rpc_secdes.h */

	uint32 ptr1;
	uint32 sec_info; /* xxxx_SECURITY_INFORMATION */

	uint32 ptr2;       /* pointer */
	BUFHDR hdr_sec;    /* header for security data */
	uint32 ptr3;       /* pointer */
	SEC_DESC_BUF *data;

	uint32 unknown_2; /* 0x0000 0000 */

} REG_Q_CREATE_KEY;

/* REG_R_CREATE_KEY */
typedef struct r_reg_create_key_info
{
	POLICY_HND key_pol;       /* policy handle */
	uint32 unknown; /* 0x0000 0000 */

	WERROR status;         /* return status */

} REG_R_CREATE_KEY;

/* REG_Q_DELETE_KEY */
typedef struct q_reg_delete_key_info
{
	POLICY_HND pnt_pol;       /* parent key policy handle */

	UNIHDR hdr_name;
	UNISTR2 uni_name;
} REG_Q_DELETE_KEY;

/* REG_R_DELETE_KEY */
typedef struct r_reg_delete_key_info
{
	POLICY_HND key_pol;       /* policy handle */

	WERROR status;         /* return status */

} REG_R_DELETE_KEY;

/* REG_Q_DELETE_VALUE */
typedef struct q_reg_delete_val_info
{
	POLICY_HND pnt_pol;       /* parent key policy handle */

	UNIHDR hdr_name;
	UNISTR2 uni_name;

} REG_Q_DELETE_VALUE;

/* REG_R_DELETE_VALUE */
typedef struct r_reg_delete_val_info
{
	POLICY_HND key_pol;       /* policy handle */

	WERROR status;         /* return status */

} REG_R_DELETE_VALUE;

/* REG_Q_QUERY_KEY */
typedef struct q_reg_query_info
{
	POLICY_HND pol;       /* policy handle */
	UNIHDR hdr_class;
	UNISTR2 uni_class;

} REG_Q_QUERY_KEY;

/* REG_R_QUERY_KEY */
typedef struct r_reg_query_key_info
{
	UNIHDR hdr_class;
	UNISTR2 uni_class;

	uint32 num_subkeys;
	uint32 max_subkeylen;
	uint32 reserved; /* 0x0000 0000 - according to MSDN (max_subkeysize?) */
	uint32 num_values;
	uint32 max_valnamelen;
	uint32 max_valbufsize; 
	uint32 sec_desc; /* 0x0000 0078 */
	NTTIME mod_time;  /* modified time */

	WERROR status;         /* return status */

} REG_R_QUERY_KEY;


/* REG_Q_UNKNOWN_1A */
typedef struct q_reg_unk_1a_info
{
	POLICY_HND pol;       /* policy handle */

} REG_Q_UNKNOWN_1A;

/* REG_R_UNKNOWN_1A */
typedef struct r_reg_unk_1a_info
{
	uint32 unknown;         /* 0x0500 0000 */
	WERROR status;         /* return status */

} REG_R_UNKNOWN_1A;


/* REG_Q_UNKNOWN_1A */
typedef struct q_reg_unknown_14
{
	POLICY_HND pol;       /* policy handle */
	
	UNIHDR  hdr_file;	/* unicode product type header */
	UNISTR2 uni_file;	/* local filename to save key as from regedt32.exe */
				/* e.g. "c:\temp\test.dat" */
	
	uint32 unknown;		/* 0x0000 0000 */

} REG_Q_SAVE_KEY;


/* REG_R_UNKNOWN_1A */
typedef struct r_reg_unknown_14
{
	WERROR status;         /* return status */

} REG_R_SAVE_KEY;



/* REG_Q_CLOSE */
typedef struct reg_q_close_info
{
	POLICY_HND pol; /* policy handle */

} REG_Q_CLOSE;

/* REG_R_CLOSE */
typedef struct reg_r_close_info
{
	POLICY_HND pol; /* policy handle.  should be all zeros. */

	WERROR status; /* return code */

} REG_R_CLOSE;


/* REG_Q_ENUM_KEY */
typedef struct q_reg_enum_value_info
{
	POLICY_HND pol;         /* policy handle */

	uint32 key_index;       

	uint16 key_name_len;    /* 0x0000 */
	uint16 unknown_1;       /* 0x0414 */

	uint32 ptr1;            /* pointer */
	uint32 unknown_2;       /* 0x0000 020A */
	uint8  pad1[8];         /* padding - zeros */

	uint32 ptr2;            /* pointer */
	uint8  pad2[8];         /* padding - zeros */

	uint32 ptr3;            /* pointer */
	NTTIME time;            /* current time? */

} REG_Q_ENUM_KEY;

/* REG_R_ENUM_KEY */
typedef struct r_reg_enum_key_info
{ 
	uint16 key_name_len;    /* number of bytes in key name */
	uint16 unknown_1;       /* 0x0414 - matches with query unknown_1 */

	uint32 ptr1;            /* pointer */
	uint32 unknown_2;       /* 0x0000 020A */
	uint32 unknown_3;       /* 0x0000 0000 */

	UNISTR3 key_name;

	uint32 ptr2;            /* pointer */
	uint8  pad2[8];         /* padding - zeros */

	uint32 ptr3;            /* pointer */
	NTTIME time;            /* current time? */

	WERROR status;         /* return status */

} REG_R_ENUM_KEY;


/* REG_Q_INFO */
typedef struct q_reg_info_info
{
	POLICY_HND pol;		/* policy handle */

	UNIHDR  hdr_type;	/* unicode product type header */
	UNISTR2 uni_type;	/* unicode product type - "ProductType" */

	uint32 ptr_reserved;	/* pointer */
  
	uint32 ptr_buf;		/* the next three fields follow if ptr_buf != 0 */
	uint32 ptr_bufsize;
	uint32 bufsize;
	uint32 buf_unk;

	uint32 unk1;
	uint32 ptr_buflen;
	uint32 buflen;
  
	uint32 ptr_buflen2;
	uint32 buflen2;

} REG_Q_INFO;

/* REG_R_INFO */
typedef struct r_reg_info_info
{ 
	uint32 ptr_type;	/* key type pointer */
	uint32 type;		/* key datatype  */

	uint32 ptr_uni_val;	/* key value pointer */
	BUFFER2 uni_val;	/* key value */

	uint32 ptr_max_len;
	uint32 buf_max_len;

	uint32 ptr_len;
	uint32 buf_len;
  
	WERROR status;	/* return status */

} REG_R_INFO;


/* REG_Q_OPEN_ENTRY */
typedef struct q_reg_open_entry_info
{
	POLICY_HND pol;        /* policy handle */

	UNIHDR  hdr_name;       /* unicode registry string header */
	UNISTR2 uni_name;       /* unicode registry string name */

	uint32 unknown_0;       /* 32 bit unknown - 0x0000 0000 */
	uint32 access_desired; 

} REG_Q_OPEN_ENTRY;



/* REG_R_OPEN_ENTRY */
typedef struct r_reg_open_entry_info
{
	POLICY_HND pol;       /* policy handle */
	WERROR status;         /* return status */

} REG_R_OPEN_ENTRY;

/* REG_Q_SHUTDOWN */
typedef struct q_reg_shutdown_info
{
	uint32 ptr_0;
	uint32 ptr_1;
	uint32 ptr_2;
	UNIHDR hdr_msg;		/* shutdown message */
	UNISTR2 uni_msg;	/* seconds */
	uint32 timeout;		/* seconds */
	uint8 force;		/* boolean: force shutdown */
	uint8 reboot;		/* boolean: reboot on shutdown */
		
} REG_Q_SHUTDOWN;

/* REG_R_SHUTDOWN */
typedef struct r_reg_shutdown_info
{
	WERROR status;		/* return status */

} REG_R_SHUTDOWN;

/* REG_Q_ABORT_SHUTDOWN */
typedef struct q_reg_abort_shutdown_info
{
	uint32 ptr_server;
	uint16 server;

} REG_Q_ABORT_SHUTDOWN;

/* REG_R_ABORT_SHUTDOWN */
typedef struct r_reg_abort_shutdown_info
{ 
	WERROR status; /* return status */

} REG_R_ABORT_SHUTDOWN;


#endif /* _RPC_REG_H */

