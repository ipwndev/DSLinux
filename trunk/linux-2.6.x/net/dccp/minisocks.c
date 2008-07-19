/*
 *  net/dccp/minisocks.c
 *
 *  An implementation of the DCCP protocol
 *  Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/config.h>
#include <linux/dccp.h>
#include <linux/skbuff.h>
#include <linux/timer.h>

#include <net/sock.h>
#include <net/xfrm.h>
#include <net/inet_timewait_sock.h>

#include "ackvec.h"
#include "ccid.h"
#include "dccp.h"

struct inet_timewait_death_row dccp_death_row = {
	.sysctl_max_tw_buckets = NR_FILE * 2,
	.period		= DCCP_TIMEWAIT_LEN / INET_TWDR_TWKILL_SLOTS,
	.death_lock	= SPIN_LOCK_UNLOCKED,
	.hashinfo	= &dccp_hashinfo,
	.tw_timer	= TIMER_INITIALIZER(inet_twdr_hangman, 0,
					    (unsigned long)&dccp_death_row),
	.twkill_work	= __WORK_INITIALIZER(dccp_death_row.twkill_work,
					     inet_twdr_twkill_work,
					     &dccp_death_row),
/* Short-time timewait calendar */

	.twcal_hand	= -1,
	.twcal_timer	= TIMER_INITIALIZER(inet_twdr_twcal_tick, 0,
					    (unsigned long)&dccp_death_row),
};

void dccp_time_wait(struct sock *sk, int state, int timeo)
{
	struct inet_timewait_sock *tw = NULL;

	if (dccp_death_row.tw_count < dccp_death_row.sysctl_max_tw_buckets)
		tw = inet_twsk_alloc(sk, state);

	if (tw != NULL) {
		const struct inet_connection_sock *icsk = inet_csk(sk);
		const int rto = (icsk->icsk_rto << 2) - (icsk->icsk_rto >> 1);

		/* Linkage updates. */
		__inet_twsk_hashdance(tw, sk, &dccp_hashinfo);

		/* Get the TIME_WAIT timeout firing. */
		if (timeo < rto)
			timeo = rto;

		tw->tw_timeout = DCCP_TIMEWAIT_LEN;
		if (state == DCCP_TIME_WAIT)
			timeo = DCCP_TIMEWAIT_LEN;

		inet_twsk_schedule(tw, &dccp_death_row, timeo,
				   DCCP_TIMEWAIT_LEN);
		inet_twsk_put(tw);
	} else {
		/* Sorry, if we're out of memory, just CLOSE this
		 * socket up.  We've got bigger problems than
		 * non-graceful socket closings.
		 */
		LIMIT_NETDEBUG(KERN_INFO "DCCP: time wait bucket "
					 "table overflow\n");
	}

	dccp_done(sk);
}

struct sock *dccp_create_openreq_child(struct sock *sk,
				       const struct request_sock *req,
				       const struct sk_buff *skb)
{
	/*
	 * Step 3: Process LISTEN state
	 *
	 * // Generate a new socket and switch to that socket
	 * Set S := new socket for this port pair
	 */
	struct sock *newsk = inet_csk_clone(sk, req, GFP_ATOMIC);

	if (newsk != NULL) {
		const struct dccp_request_sock *dreq = dccp_rsk(req);
		struct inet_connection_sock *newicsk = inet_csk(sk);
		struct dccp_sock *newdp = dccp_sk(newsk);

		newdp->dccps_role	   = DCCP_ROLE_SERVER;
		newdp->dccps_hc_rx_ackvec  = NULL;
		newdp->dccps_service_list  = NULL;
		newdp->dccps_service	   = dreq->dreq_service;
		newicsk->icsk_rto	   = DCCP_TIMEOUT_INIT;
		do_gettimeofday(&newdp->dccps_epoch);

		if (newdp->dccps_options.dccpo_send_ack_vector) {
			newdp->dccps_hc_rx_ackvec =
				dccp_ackvec_alloc(DCCP_MAX_ACKVEC_LEN,
						  GFP_ATOMIC);
			/*
			 * XXX: We're using the same CCIDs set on the parent,
			 * i.e. sk_clone copied the master sock and left the
			 * CCID pointers for this child, that is why we do the
			 * __ccid_get calls.
			 */
			if (unlikely(newdp->dccps_hc_rx_ackvec == NULL))
				goto out_free;
		}

		if (unlikely(ccid_hc_rx_init(newdp->dccps_hc_rx_ccid,
					     newsk) != 0 ||
			     ccid_hc_tx_init(newdp->dccps_hc_tx_ccid,
				     	     newsk) != 0)) {
			dccp_ackvec_free(newdp->dccps_hc_rx_ackvec);
			ccid_hc_rx_exit(newdp->dccps_hc_rx_ccid, newsk);
			ccid_hc_tx_exit(newdp->dccps_hc_tx_ccid, newsk);
out_free:
			/* It is still raw copy of parent, so invalidate
			 * destructor and make plain sk_free() */
			newsk->sk_destruct = NULL;
			sk_free(newsk);
			return NULL;
		}

		__ccid_get(newdp->dccps_hc_rx_ccid);
		__ccid_get(newdp->dccps_hc_tx_ccid);

		/*
		 * Step 3: Process LISTEN state
		 *
		 *	Choose S.ISS (initial seqno) or set from Init Cookie
		 *	Set S.ISR, S.GSR, S.SWL, S.SWH from packet or Init
		 *	Cookie
		 */

		/* See dccp_v4_conn_request */
		newdp->dccps_options.dccpo_sequence_window = req->rcv_wnd;

		newdp->dccps_gar = newdp->dccps_isr = dreq->dreq_isr;
		dccp_update_gsr(newsk, dreq->dreq_isr);

		newdp->dccps_iss = dreq->dreq_iss;
		dccp_update_gss(newsk, dreq->dreq_iss);

		/*
		 * SWL and AWL are initially adjusted so that they are not less than
		 * the initial Sequence Numbers received and sent, respectively:
		 *	SWL := max(GSR + 1 - floor(W/4), ISR),
		 *	AWL := max(GSS - W' + 1, ISS).
		 * These adjustments MUST be applied only at the beginning of the
		 * connection.
		 */
		dccp_set_seqno(&newdp->dccps_swl,
			       max48(newdp->dccps_swl, newdp->dccps_isr));
		dccp_set_seqno(&newdp->dccps_awl,
			       max48(newdp->dccps_awl, newdp->dccps_iss));

		dccp_init_xmit_timers(newsk);

		DCCP_INC_STATS_BH(DCCP_MIB_PASSIVEOPENS);
	}
	return newsk;
}

/* 
 * Process an incoming packet for RESPOND sockets represented
 * as an request_sock.
 */
struct sock *dccp_check_req(struct sock *sk, struct sk_buff *skb,
			    struct request_sock *req,
			    struct request_sock **prev)
{
	struct sock *child = NULL;

	/* Check for retransmitted REQUEST */
	if (dccp_hdr(skb)->dccph_type == DCCP_PKT_REQUEST) {
		if (after48(DCCP_SKB_CB(skb)->dccpd_seq,
			    dccp_rsk(req)->dreq_isr)) {
			struct dccp_request_sock *dreq = dccp_rsk(req);

			dccp_pr_debug("Retransmitted REQUEST\n");
			/* Send another RESPONSE packet */
			dccp_set_seqno(&dreq->dreq_iss, dreq->dreq_iss + 1);
			dccp_set_seqno(&dreq->dreq_isr,
				       DCCP_SKB_CB(skb)->dccpd_seq);
			req->rsk_ops->rtx_syn_ack(sk, req, NULL);
		}
		/* Network Duplicate, discard packet */
		return NULL;
	}

	DCCP_SKB_CB(skb)->dccpd_reset_code = DCCP_RESET_CODE_PACKET_ERROR;

	if (dccp_hdr(skb)->dccph_type != DCCP_PKT_ACK &&
	    dccp_hdr(skb)->dccph_type != DCCP_PKT_DATAACK)
		goto drop;

	/* Invalid ACK */
	if (DCCP_SKB_CB(skb)->dccpd_ack_seq != dccp_rsk(req)->dreq_iss) {
		dccp_pr_debug("Invalid ACK number: ack_seq=%llu, "
			      "dreq_iss=%llu\n",
			      (unsigned long long)
			      DCCP_SKB_CB(skb)->dccpd_ack_seq,
			      (unsigned long long)
			      dccp_rsk(req)->dreq_iss);
		goto drop;
	}

	child = dccp_v4_request_recv_sock(sk, skb, req, NULL);
	if (child == NULL)
		goto listen_overflow;

	/* FIXME: deal with options */

	inet_csk_reqsk_queue_unlink(sk, req, prev);
	inet_csk_reqsk_queue_removed(sk, req);
	inet_csk_reqsk_queue_add(sk, req, child);
out:
	return child;
listen_overflow:
	dccp_pr_debug("listen_overflow!\n");
	DCCP_SKB_CB(skb)->dccpd_reset_code = DCCP_RESET_CODE_TOO_BUSY;
drop:
	if (dccp_hdr(skb)->dccph_type != DCCP_PKT_RESET)
		req->rsk_ops->send_reset(skb);

	inet_csk_reqsk_queue_drop(sk, req, prev);
	goto out;
}

/*
 *  Queue segment on the new socket if the new socket is active,
 *  otherwise we just shortcircuit this and continue with
 *  the new socket.
 */
int dccp_child_process(struct sock *parent, struct sock *child,
		       struct sk_buff *skb)
{
	int ret = 0;
	const int state = child->sk_state;

	if (!sock_owned_by_user(child)) {
		ret = dccp_rcv_state_process(child, skb, dccp_hdr(skb),
					     skb->len);

		/* Wakeup parent, send SIGIO */
		if (state == DCCP_RESPOND && child->sk_state != state)
			parent->sk_data_ready(parent, 0);
	} else {
		/* Alas, it is possible again, because we do lookup
		 * in main socket hash table and lock on listening
		 * socket does not protect us more.
		 */
		sk_add_backlog(child, skb);
	}

	bh_unlock_sock(child);
	sock_put(child);
	return ret;
}
