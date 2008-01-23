/* 
   Unix SMB/CIFS implementation.
   RPC Pipe client
 
   Copyright (C) Andrew Tridgell              1992-1998,
   Copyright (C) Luke Kenneth Casson Leighton 1996-1998,
   Copyright (C) Paul Ashton                  1997-1998.
   Copyright (C) Jeremy Allison                    1999.
   Copyright (C) Simo Sorce                        2001
   
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

#include "includes.h"

/* Shutdown a server */

WERROR cli_reg_shutdown(struct cli_state * cli, TALLOC_CTX *mem_ctx,
                          const char *msg, uint32 timeout, BOOL do_reboot,
			  BOOL force)
{
	prs_struct qbuf;
	prs_struct rbuf; 
	REG_Q_SHUTDOWN q_s;
	REG_R_SHUTDOWN r_s;
	WERROR result = WERR_GENERAL_FAILURE;

	if (msg == NULL) return WERR_INVALID_PARAM;

	ZERO_STRUCT (q_s);
	ZERO_STRUCT (r_s);

	prs_init(&qbuf , MAX_PDU_FRAG_LEN, mem_ctx, MARSHALL);
	prs_init(&rbuf, 0, mem_ctx, UNMARSHALL);

	/* Marshall data and send request */

	init_reg_q_shutdown(&q_s, msg, timeout, do_reboot, force);

	if (!reg_io_q_shutdown("", &q_s, &qbuf, 0) ||
	    !rpc_api_pipe_req(cli, PI_WINREG, REG_SHUTDOWN, &qbuf, &rbuf))
		goto done;
	
	/* Unmarshall response */
	
	if(reg_io_r_shutdown("", &r_s, &rbuf, 0))
		result = r_s.status;

done:
	prs_mem_free(&rbuf);
	prs_mem_free(&qbuf);

	return result;
}


/* Abort a server shutdown */

WERROR cli_reg_abort_shutdown(struct cli_state * cli, TALLOC_CTX *mem_ctx)
{
	prs_struct rbuf;
	prs_struct qbuf; 
	REG_Q_ABORT_SHUTDOWN q_s;
	REG_R_ABORT_SHUTDOWN r_s;
	WERROR result = WERR_GENERAL_FAILURE;

	ZERO_STRUCT (q_s);
	ZERO_STRUCT (r_s);

	prs_init(&qbuf , MAX_PDU_FRAG_LEN, mem_ctx, MARSHALL);
	prs_init(&rbuf, 0, mem_ctx, UNMARSHALL);
	
	/* Marshall data and send request */

	init_reg_q_abort_shutdown(&q_s);

	if (!reg_io_q_abort_shutdown("", &q_s, &qbuf, 0) ||
	    !rpc_api_pipe_req(cli, PI_WINREG, REG_ABORT_SHUTDOWN, &qbuf, &rbuf))
	    	goto done;
	
		/* Unmarshall response */
	
	if (reg_io_r_abort_shutdown("", &r_s, &rbuf, 0))
		result = r_s.status;

done:
	prs_mem_free(&rbuf);
	prs_mem_free(&qbuf );

	return result;
}
