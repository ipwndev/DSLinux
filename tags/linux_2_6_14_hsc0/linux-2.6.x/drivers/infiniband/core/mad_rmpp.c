/*
 * Copyright (c) 2005 Intel Inc. All rights reserved.
 * Copyright (c) 2005 Voltaire, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * $Id$
 */

#include <linux/dma-mapping.h>

#include "mad_priv.h"
#include "mad_rmpp.h"

enum rmpp_state {
	RMPP_STATE_ACTIVE,
	RMPP_STATE_TIMEOUT,
	RMPP_STATE_COMPLETE
};

struct mad_rmpp_recv {
	struct ib_mad_agent_private *agent;
	struct list_head list;
	struct work_struct timeout_work;
	struct work_struct cleanup_work;
	wait_queue_head_t wait;
	enum rmpp_state state;
	spinlock_t lock;
	atomic_t refcount;

	struct ib_ah *ah;
	struct ib_mad_recv_wc *rmpp_wc;
	struct ib_mad_recv_buf *cur_seg_buf;
	int last_ack;
	int seg_num;
	int newwin;

	__be64 tid;
	u32 src_qp;
	u16 slid;
	u8 mgmt_class;
	u8 class_version;
	u8 method;
};

static void destroy_rmpp_recv(struct mad_rmpp_recv *rmpp_recv)
{
	atomic_dec(&rmpp_recv->refcount);
	wait_event(rmpp_recv->wait, !atomic_read(&rmpp_recv->refcount));
	ib_destroy_ah(rmpp_recv->ah);
	kfree(rmpp_recv);
}

void ib_cancel_rmpp_recvs(struct ib_mad_agent_private *agent)
{
	struct mad_rmpp_recv *rmpp_recv, *temp_rmpp_recv;
	unsigned long flags;

	spin_lock_irqsave(&agent->lock, flags);
	list_for_each_entry(rmpp_recv, &agent->rmpp_list, list) {
		cancel_delayed_work(&rmpp_recv->timeout_work);
		cancel_delayed_work(&rmpp_recv->cleanup_work);
	}
	spin_unlock_irqrestore(&agent->lock, flags);

	flush_workqueue(agent->qp_info->port_priv->wq);

	list_for_each_entry_safe(rmpp_recv, temp_rmpp_recv,
				 &agent->rmpp_list, list) {
		list_del(&rmpp_recv->list);
		if (rmpp_recv->state != RMPP_STATE_COMPLETE)
			ib_free_recv_mad(rmpp_recv->rmpp_wc);
		destroy_rmpp_recv(rmpp_recv);
	}
}

static int data_offset(u8 mgmt_class)
{
	if (mgmt_class == IB_MGMT_CLASS_SUBN_ADM)
		return offsetof(struct ib_sa_mad, data);
	else if ((mgmt_class >= IB_MGMT_CLASS_VENDOR_RANGE2_START) &&
		 (mgmt_class <= IB_MGMT_CLASS_VENDOR_RANGE2_END))
		return offsetof(struct ib_vendor_mad, data);
	else
		return offsetof(struct ib_rmpp_mad, data);
}

static void format_ack(struct ib_rmpp_mad *ack,
		       struct ib_rmpp_mad *data,
		       struct mad_rmpp_recv *rmpp_recv)
{
	unsigned long flags;

	memcpy(&ack->mad_hdr, &data->mad_hdr,
	       data_offset(data->mad_hdr.mgmt_class));

	ack->mad_hdr.method ^= IB_MGMT_METHOD_RESP;
	ack->rmpp_hdr.rmpp_type = IB_MGMT_RMPP_TYPE_ACK;
	ib_set_rmpp_flags(&ack->rmpp_hdr, IB_MGMT_RMPP_FLAG_ACTIVE);

	spin_lock_irqsave(&rmpp_recv->lock, flags);
	rmpp_recv->last_ack = rmpp_recv->seg_num;
	ack->rmpp_hdr.seg_num = cpu_to_be32(rmpp_recv->seg_num);
	ack->rmpp_hdr.paylen_newwin = cpu_to_be32(rmpp_recv->newwin);
	spin_unlock_irqrestore(&rmpp_recv->lock, flags);
}

static void ack_recv(struct mad_rmpp_recv *rmpp_recv,
		     struct ib_mad_recv_wc *recv_wc)
{
	struct ib_mad_send_buf *msg;
	struct ib_send_wr *bad_send_wr;
	int hdr_len, ret;

	hdr_len = sizeof(struct ib_mad_hdr) + sizeof(struct ib_rmpp_hdr);
	msg = ib_create_send_mad(&rmpp_recv->agent->agent, recv_wc->wc->src_qp,
				 recv_wc->wc->pkey_index, rmpp_recv->ah, 1,
				 hdr_len, sizeof(struct ib_rmpp_mad) - hdr_len,
				 GFP_KERNEL);
	if (!msg)
		return;

	format_ack((struct ib_rmpp_mad *) msg->mad,
		   (struct ib_rmpp_mad *) recv_wc->recv_buf.mad, rmpp_recv);
	ret = ib_post_send_mad(&rmpp_recv->agent->agent, &msg->send_wr,
			       &bad_send_wr);
	if (ret)
		ib_free_send_mad(msg);
}

static int alloc_response_msg(struct ib_mad_agent *agent,
			      struct ib_mad_recv_wc *recv_wc,
			      struct ib_mad_send_buf **msg)
{
	struct ib_mad_send_buf *m;
	struct ib_ah *ah;
	int hdr_len;

	ah = ib_create_ah_from_wc(agent->qp->pd, recv_wc->wc,
				  recv_wc->recv_buf.grh, agent->port_num);
	if (IS_ERR(ah))
		return PTR_ERR(ah);

	hdr_len = sizeof(struct ib_mad_hdr) + sizeof(struct ib_rmpp_hdr);
	m = ib_create_send_mad(agent, recv_wc->wc->src_qp,
			       recv_wc->wc->pkey_index, ah, 1, hdr_len,
			       sizeof(struct ib_rmpp_mad) - hdr_len,
			       GFP_KERNEL);
	if (IS_ERR(m)) {
		ib_destroy_ah(ah);
		return PTR_ERR(m);
	}
	*msg = m;
	return 0;
}

static void free_msg(struct ib_mad_send_buf *msg)
{
	ib_destroy_ah(msg->send_wr.wr.ud.ah);
	ib_free_send_mad(msg);
}

static void nack_recv(struct ib_mad_agent_private *agent,
		      struct ib_mad_recv_wc *recv_wc, u8 rmpp_status)
{
	struct ib_mad_send_buf *msg;
	struct ib_rmpp_mad *rmpp_mad;
	struct ib_send_wr *bad_send_wr;
	int ret;

	ret = alloc_response_msg(&agent->agent, recv_wc, &msg);
	if (ret)
		return;

	rmpp_mad = (struct ib_rmpp_mad *) msg->mad;
	memcpy(rmpp_mad, recv_wc->recv_buf.mad,
	       data_offset(recv_wc->recv_buf.mad->mad_hdr.mgmt_class));

	rmpp_mad->mad_hdr.method ^= IB_MGMT_METHOD_RESP;
	rmpp_mad->rmpp_hdr.rmpp_version = IB_MGMT_RMPP_VERSION;
	rmpp_mad->rmpp_hdr.rmpp_type = IB_MGMT_RMPP_TYPE_ABORT;
	ib_set_rmpp_flags(&rmpp_mad->rmpp_hdr, IB_MGMT_RMPP_FLAG_ACTIVE);
	rmpp_mad->rmpp_hdr.rmpp_status = rmpp_status;
	rmpp_mad->rmpp_hdr.seg_num = 0;
	rmpp_mad->rmpp_hdr.paylen_newwin = 0;

	ret = ib_post_send_mad(&agent->agent, &msg->send_wr, &bad_send_wr);
	if (ret)
		free_msg(msg);
}

static void recv_timeout_handler(void *data)
{
	struct mad_rmpp_recv *rmpp_recv = data;
	struct ib_mad_recv_wc *rmpp_wc;
	unsigned long flags;

	spin_lock_irqsave(&rmpp_recv->agent->lock, flags);
	if (rmpp_recv->state != RMPP_STATE_ACTIVE) {
		spin_unlock_irqrestore(&rmpp_recv->agent->lock, flags);
		return;
	}
	rmpp_recv->state = RMPP_STATE_TIMEOUT;
	list_del(&rmpp_recv->list);
	spin_unlock_irqrestore(&rmpp_recv->agent->lock, flags);

	rmpp_wc = rmpp_recv->rmpp_wc;
	nack_recv(rmpp_recv->agent, rmpp_wc, IB_MGMT_RMPP_STATUS_T2L);
	destroy_rmpp_recv(rmpp_recv);
	ib_free_recv_mad(rmpp_wc);
}

static void recv_cleanup_handler(void *data)
{
	struct mad_rmpp_recv *rmpp_recv = data;
	unsigned long flags;

	spin_lock_irqsave(&rmpp_recv->agent->lock, flags);
	list_del(&rmpp_recv->list);
	spin_unlock_irqrestore(&rmpp_recv->agent->lock, flags);
	destroy_rmpp_recv(rmpp_recv);
}

static struct mad_rmpp_recv *
create_rmpp_recv(struct ib_mad_agent_private *agent,
		 struct ib_mad_recv_wc *mad_recv_wc)
{
	struct mad_rmpp_recv *rmpp_recv;
	struct ib_mad_hdr *mad_hdr;

	rmpp_recv = kmalloc(sizeof *rmpp_recv, GFP_KERNEL);
	if (!rmpp_recv)
		return NULL;

	rmpp_recv->ah = ib_create_ah_from_wc(agent->agent.qp->pd,
					     mad_recv_wc->wc,
					     mad_recv_wc->recv_buf.grh,
					     agent->agent.port_num);
	if (IS_ERR(rmpp_recv->ah))
		goto error;

	rmpp_recv->agent = agent;
	init_waitqueue_head(&rmpp_recv->wait);
	INIT_WORK(&rmpp_recv->timeout_work, recv_timeout_handler, rmpp_recv);
	INIT_WORK(&rmpp_recv->cleanup_work, recv_cleanup_handler, rmpp_recv);
	spin_lock_init(&rmpp_recv->lock);
	rmpp_recv->state = RMPP_STATE_ACTIVE;
	atomic_set(&rmpp_recv->refcount, 1);

	rmpp_recv->rmpp_wc = mad_recv_wc;
	rmpp_recv->cur_seg_buf = &mad_recv_wc->recv_buf;
	rmpp_recv->newwin = 1;
	rmpp_recv->seg_num = 1;
	rmpp_recv->last_ack = 0;

	mad_hdr = &mad_recv_wc->recv_buf.mad->mad_hdr;
	rmpp_recv->tid = mad_hdr->tid;
	rmpp_recv->src_qp = mad_recv_wc->wc->src_qp;
	rmpp_recv->slid = mad_recv_wc->wc->slid;
	rmpp_recv->mgmt_class = mad_hdr->mgmt_class;
	rmpp_recv->class_version = mad_hdr->class_version;
	rmpp_recv->method  = mad_hdr->method;
	return rmpp_recv;

error:	kfree(rmpp_recv);
	return NULL;
}

static inline void deref_rmpp_recv(struct mad_rmpp_recv *rmpp_recv)
{
	if (atomic_dec_and_test(&rmpp_recv->refcount))
		wake_up(&rmpp_recv->wait);
}

static struct mad_rmpp_recv *
find_rmpp_recv(struct ib_mad_agent_private *agent,
	       struct ib_mad_recv_wc *mad_recv_wc)
{
	struct mad_rmpp_recv *rmpp_recv;
	struct ib_mad_hdr *mad_hdr = &mad_recv_wc->recv_buf.mad->mad_hdr;

	list_for_each_entry(rmpp_recv, &agent->rmpp_list, list) {
		if (rmpp_recv->tid == mad_hdr->tid &&
		    rmpp_recv->src_qp == mad_recv_wc->wc->src_qp &&
		    rmpp_recv->slid == mad_recv_wc->wc->slid &&
		    rmpp_recv->mgmt_class == mad_hdr->mgmt_class &&
		    rmpp_recv->class_version == mad_hdr->class_version &&
		    rmpp_recv->method == mad_hdr->method)
			return rmpp_recv;
	}
	return NULL;
}

static struct mad_rmpp_recv *
acquire_rmpp_recv(struct ib_mad_agent_private *agent,
		  struct ib_mad_recv_wc *mad_recv_wc)
{
	struct mad_rmpp_recv *rmpp_recv;
	unsigned long flags;

	spin_lock_irqsave(&agent->lock, flags);
	rmpp_recv = find_rmpp_recv(agent, mad_recv_wc);
	if (rmpp_recv)
		atomic_inc(&rmpp_recv->refcount);
	spin_unlock_irqrestore(&agent->lock, flags);
	return rmpp_recv;
}

static struct mad_rmpp_recv *
insert_rmpp_recv(struct ib_mad_agent_private *agent,
		 struct mad_rmpp_recv *rmpp_recv)
{
	struct mad_rmpp_recv *cur_rmpp_recv;

	cur_rmpp_recv = find_rmpp_recv(agent, rmpp_recv->rmpp_wc);
	if (!cur_rmpp_recv)
		list_add_tail(&rmpp_recv->list, &agent->rmpp_list);

	return cur_rmpp_recv;
}

static inline int get_last_flag(struct ib_mad_recv_buf *seg)
{
	struct ib_rmpp_mad *rmpp_mad;

	rmpp_mad = (struct ib_rmpp_mad *) seg->mad;
	return ib_get_rmpp_flags(&rmpp_mad->rmpp_hdr) & IB_MGMT_RMPP_FLAG_LAST;
}

static inline int get_seg_num(struct ib_mad_recv_buf *seg)
{
	struct ib_rmpp_mad *rmpp_mad;

	rmpp_mad = (struct ib_rmpp_mad *) seg->mad;
	return be32_to_cpu(rmpp_mad->rmpp_hdr.seg_num);
}

static inline struct ib_mad_recv_buf * get_next_seg(struct list_head *rmpp_list,
						    struct ib_mad_recv_buf *seg)
{
	if (seg->list.next == rmpp_list)
		return NULL;

	return container_of(seg->list.next, struct ib_mad_recv_buf, list);
}

static inline int window_size(struct ib_mad_agent_private *agent)
{
	return max(agent->qp_info->recv_queue.max_active >> 3, 1);
}

static struct ib_mad_recv_buf * find_seg_location(struct list_head *rmpp_list,
						  int seg_num)
{
        struct ib_mad_recv_buf *seg_buf;
	int cur_seg_num;

	list_for_each_entry_reverse(seg_buf, rmpp_list, list) {
		cur_seg_num = get_seg_num(seg_buf);
		if (seg_num > cur_seg_num)
			return seg_buf;
		if (seg_num == cur_seg_num)
			break;
	}
	return NULL;
}

static void update_seg_num(struct mad_rmpp_recv *rmpp_recv,
			   struct ib_mad_recv_buf *new_buf)
{
	struct list_head *rmpp_list = &rmpp_recv->rmpp_wc->rmpp_list;

	while (new_buf && (get_seg_num(new_buf) == rmpp_recv->seg_num + 1)) {
		rmpp_recv->cur_seg_buf = new_buf;
		rmpp_recv->seg_num++;
		new_buf = get_next_seg(rmpp_list, new_buf);
	}
}

static inline int get_mad_len(struct mad_rmpp_recv *rmpp_recv)
{
	struct ib_rmpp_mad *rmpp_mad;
	int hdr_size, data_size, pad;

	rmpp_mad = (struct ib_rmpp_mad *)rmpp_recv->cur_seg_buf->mad;

	hdr_size = data_offset(rmpp_mad->mad_hdr.mgmt_class);
	data_size = sizeof(struct ib_rmpp_mad) - hdr_size;
	pad = IB_MGMT_RMPP_DATA - be32_to_cpu(rmpp_mad->rmpp_hdr.paylen_newwin);
	if (pad > IB_MGMT_RMPP_DATA || pad < 0)
		pad = 0;

	return hdr_size + rmpp_recv->seg_num * data_size - pad;
}

static struct ib_mad_recv_wc * complete_rmpp(struct mad_rmpp_recv *rmpp_recv)
{
	struct ib_mad_recv_wc *rmpp_wc;

	ack_recv(rmpp_recv, rmpp_recv->rmpp_wc);
	if (rmpp_recv->seg_num > 1)
		cancel_delayed_work(&rmpp_recv->timeout_work);

	rmpp_wc = rmpp_recv->rmpp_wc;
	rmpp_wc->mad_len = get_mad_len(rmpp_recv);
	/* 10 seconds until we can find the packet lifetime */
	queue_delayed_work(rmpp_recv->agent->qp_info->port_priv->wq,
			   &rmpp_recv->cleanup_work, msecs_to_jiffies(10000));
	return rmpp_wc;
}

void ib_coalesce_recv_mad(struct ib_mad_recv_wc *mad_recv_wc, void *buf)
{
	struct ib_mad_recv_buf *seg_buf;
	struct ib_rmpp_mad *rmpp_mad;
	void *data;
	int size, len, offset;
	u8 flags;

	len = mad_recv_wc->mad_len;
	if (len <= sizeof(struct ib_mad)) {
		memcpy(buf, mad_recv_wc->recv_buf.mad, len);
		return;
	}

	offset = data_offset(mad_recv_wc->recv_buf.mad->mad_hdr.mgmt_class);

	list_for_each_entry(seg_buf, &mad_recv_wc->rmpp_list, list) {
		rmpp_mad = (struct ib_rmpp_mad *)seg_buf->mad;
		flags = ib_get_rmpp_flags(&rmpp_mad->rmpp_hdr);

		if (flags & IB_MGMT_RMPP_FLAG_FIRST) {
			data = rmpp_mad;
			size = sizeof(*rmpp_mad);
		} else {
			data = (void *) rmpp_mad + offset;
			if (flags & IB_MGMT_RMPP_FLAG_LAST)
				size = len;
			else
				size = sizeof(*rmpp_mad) - offset;
		}

		memcpy(buf, data, size);
		len -= size;
		buf += size;
	}
}
EXPORT_SYMBOL(ib_coalesce_recv_mad);

static struct ib_mad_recv_wc *
continue_rmpp(struct ib_mad_agent_private *agent,
	      struct ib_mad_recv_wc *mad_recv_wc)
{
	struct mad_rmpp_recv *rmpp_recv;
	struct ib_mad_recv_buf *prev_buf;
	struct ib_mad_recv_wc *done_wc;
	int seg_num;
	unsigned long flags;

	rmpp_recv = acquire_rmpp_recv(agent, mad_recv_wc);
	if (!rmpp_recv)
		goto drop1;

	seg_num = get_seg_num(&mad_recv_wc->recv_buf);

	spin_lock_irqsave(&rmpp_recv->lock, flags);
	if ((rmpp_recv->state == RMPP_STATE_TIMEOUT) ||
	    (seg_num > rmpp_recv->newwin))
		goto drop3;

	if ((seg_num <= rmpp_recv->last_ack) ||
	    (rmpp_recv->state == RMPP_STATE_COMPLETE)) {
		spin_unlock_irqrestore(&rmpp_recv->lock, flags);
		ack_recv(rmpp_recv, mad_recv_wc);
		goto drop2;
	}

	prev_buf = find_seg_location(&rmpp_recv->rmpp_wc->rmpp_list, seg_num);
	if (!prev_buf)
		goto drop3;

	done_wc = NULL;
	list_add(&mad_recv_wc->recv_buf.list, &prev_buf->list);
	if (rmpp_recv->cur_seg_buf == prev_buf) {
		update_seg_num(rmpp_recv, &mad_recv_wc->recv_buf);
		if (get_last_flag(rmpp_recv->cur_seg_buf)) {
			rmpp_recv->state = RMPP_STATE_COMPLETE;
			spin_unlock_irqrestore(&rmpp_recv->lock, flags);
			done_wc = complete_rmpp(rmpp_recv);
			goto out;
		} else if (rmpp_recv->seg_num == rmpp_recv->newwin) {
			rmpp_recv->newwin += window_size(agent);
			spin_unlock_irqrestore(&rmpp_recv->lock, flags);
			ack_recv(rmpp_recv, mad_recv_wc);
			goto out;
		}
	}
	spin_unlock_irqrestore(&rmpp_recv->lock, flags);
out:
	deref_rmpp_recv(rmpp_recv);
	return done_wc;

drop3:	spin_unlock_irqrestore(&rmpp_recv->lock, flags);
drop2:	deref_rmpp_recv(rmpp_recv);
drop1:	ib_free_recv_mad(mad_recv_wc);
	return NULL;
}

static struct ib_mad_recv_wc *
start_rmpp(struct ib_mad_agent_private *agent,
	   struct ib_mad_recv_wc *mad_recv_wc)
{
	struct mad_rmpp_recv *rmpp_recv;
	unsigned long flags;

	rmpp_recv = create_rmpp_recv(agent, mad_recv_wc);
	if (!rmpp_recv) {
		ib_free_recv_mad(mad_recv_wc);
		return NULL;
	}

	spin_lock_irqsave(&agent->lock, flags);
	if (insert_rmpp_recv(agent, rmpp_recv)) {
		spin_unlock_irqrestore(&agent->lock, flags);
		/* duplicate first MAD */
		destroy_rmpp_recv(rmpp_recv);
		return continue_rmpp(agent, mad_recv_wc);
	}
	atomic_inc(&rmpp_recv->refcount);

	if (get_last_flag(&mad_recv_wc->recv_buf)) {
		rmpp_recv->state = RMPP_STATE_COMPLETE;
		spin_unlock_irqrestore(&agent->lock, flags);
		complete_rmpp(rmpp_recv);
	} else {
		spin_unlock_irqrestore(&agent->lock, flags);
		/* 40 seconds until we can find the packet lifetimes */
		queue_delayed_work(agent->qp_info->port_priv->wq,
				   &rmpp_recv->timeout_work,
				   msecs_to_jiffies(40000));
		rmpp_recv->newwin += window_size(agent);
		ack_recv(rmpp_recv, mad_recv_wc);
		mad_recv_wc = NULL;
	}
	deref_rmpp_recv(rmpp_recv);
	return mad_recv_wc;
}

static inline u64 get_seg_addr(struct ib_mad_send_wr_private *mad_send_wr)
{
	return mad_send_wr->sg_list[0].addr + mad_send_wr->data_offset +
	       (sizeof(struct ib_rmpp_mad) - mad_send_wr->data_offset) *
	       (mad_send_wr->seg_num - 1);
}

static int send_next_seg(struct ib_mad_send_wr_private *mad_send_wr)
{
	struct ib_rmpp_mad *rmpp_mad;
	int timeout;
	u32 paylen;

	rmpp_mad = (struct ib_rmpp_mad *)mad_send_wr->send_wr.wr.ud.mad_hdr;
	ib_set_rmpp_flags(&rmpp_mad->rmpp_hdr, IB_MGMT_RMPP_FLAG_ACTIVE);
	rmpp_mad->rmpp_hdr.seg_num = cpu_to_be32(mad_send_wr->seg_num);

	if (mad_send_wr->seg_num == 1) {
		rmpp_mad->rmpp_hdr.rmpp_rtime_flags |= IB_MGMT_RMPP_FLAG_FIRST;
		paylen = mad_send_wr->total_seg * IB_MGMT_RMPP_DATA -
			 mad_send_wr->pad;
		rmpp_mad->rmpp_hdr.paylen_newwin = cpu_to_be32(paylen);
		mad_send_wr->sg_list[0].length = sizeof(struct ib_rmpp_mad);
	} else {
		mad_send_wr->send_wr.num_sge = 2;
		mad_send_wr->sg_list[0].length = mad_send_wr->data_offset;
		mad_send_wr->sg_list[1].addr = get_seg_addr(mad_send_wr);
		mad_send_wr->sg_list[1].length = sizeof(struct ib_rmpp_mad) -
						 mad_send_wr->data_offset;
		mad_send_wr->sg_list[1].lkey = mad_send_wr->sg_list[0].lkey;
		rmpp_mad->rmpp_hdr.paylen_newwin = 0;
	}

	if (mad_send_wr->seg_num == mad_send_wr->total_seg) {
		rmpp_mad->rmpp_hdr.rmpp_rtime_flags |= IB_MGMT_RMPP_FLAG_LAST;
		paylen = IB_MGMT_RMPP_DATA - mad_send_wr->pad;
		rmpp_mad->rmpp_hdr.paylen_newwin = cpu_to_be32(paylen);
	}

	/* 2 seconds for an ACK until we can find the packet lifetime */
	timeout = mad_send_wr->send_wr.wr.ud.timeout_ms;
	if (!timeout || timeout > 2000)
		mad_send_wr->timeout = msecs_to_jiffies(2000);
	mad_send_wr->seg_num++;
	return ib_send_mad(mad_send_wr);
}

static void abort_send(struct ib_mad_agent_private *agent, __be64 tid,
		       u8 rmpp_status)
{
	struct ib_mad_send_wr_private *mad_send_wr;
	struct ib_mad_send_wc wc;
	unsigned long flags;

	spin_lock_irqsave(&agent->lock, flags);
	mad_send_wr = ib_find_send_mad(agent, tid);
	if (!mad_send_wr)
		goto out;	/* Unmatched send */

	if ((mad_send_wr->last_ack == mad_send_wr->total_seg) ||
	    (!mad_send_wr->timeout) || (mad_send_wr->status != IB_WC_SUCCESS))
		goto out;	/* Send is already done */

	ib_mark_mad_done(mad_send_wr);
	spin_unlock_irqrestore(&agent->lock, flags);

	wc.status = IB_WC_REM_ABORT_ERR;
	wc.vendor_err = rmpp_status;
	wc.wr_id = mad_send_wr->wr_id;
	ib_mad_complete_send_wr(mad_send_wr, &wc);
	return;
out:
	spin_unlock_irqrestore(&agent->lock, flags);
}

static void process_rmpp_ack(struct ib_mad_agent_private *agent,
			     struct ib_mad_recv_wc *mad_recv_wc)
{
	struct ib_mad_send_wr_private *mad_send_wr;
	struct ib_rmpp_mad *rmpp_mad;
	unsigned long flags;
	int seg_num, newwin, ret;

	rmpp_mad = (struct ib_rmpp_mad *)mad_recv_wc->recv_buf.mad;
	if (rmpp_mad->rmpp_hdr.rmpp_status) {
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   IB_MGMT_RMPP_STATUS_BAD_STATUS);
		nack_recv(agent, mad_recv_wc, IB_MGMT_RMPP_STATUS_BAD_STATUS);
		return;
	}

	seg_num = be32_to_cpu(rmpp_mad->rmpp_hdr.seg_num);
	newwin = be32_to_cpu(rmpp_mad->rmpp_hdr.paylen_newwin);
	if (newwin < seg_num) {
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   IB_MGMT_RMPP_STATUS_W2S);
		nack_recv(agent, mad_recv_wc, IB_MGMT_RMPP_STATUS_W2S);
		return;
	}

	spin_lock_irqsave(&agent->lock, flags);
	mad_send_wr = ib_find_send_mad(agent, rmpp_mad->mad_hdr.tid);
	if (!mad_send_wr)
		goto out;	/* Unmatched ACK */

	if ((mad_send_wr->last_ack == mad_send_wr->total_seg) ||
	    (!mad_send_wr->timeout) || (mad_send_wr->status != IB_WC_SUCCESS))
		goto out;	/* Send is already done */

	if (seg_num > mad_send_wr->total_seg || seg_num > mad_send_wr->newwin) {
		spin_unlock_irqrestore(&agent->lock, flags);
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   IB_MGMT_RMPP_STATUS_S2B);
		nack_recv(agent, mad_recv_wc, IB_MGMT_RMPP_STATUS_S2B);
		return;
	}

	if (newwin < mad_send_wr->newwin || seg_num < mad_send_wr->last_ack)
		goto out;	/* Old ACK */

	if (seg_num > mad_send_wr->last_ack) {
		mad_send_wr->last_ack = seg_num;
		mad_send_wr->retries = mad_send_wr->send_wr.wr.ud.retries;
	}
	mad_send_wr->newwin = newwin;
	if (mad_send_wr->last_ack == mad_send_wr->total_seg) {
		/* If no response is expected, the ACK completes the send */
		if (!mad_send_wr->send_wr.wr.ud.timeout_ms) {
			struct ib_mad_send_wc wc;

			ib_mark_mad_done(mad_send_wr);
			spin_unlock_irqrestore(&agent->lock, flags);

			wc.status = IB_WC_SUCCESS;
			wc.vendor_err = 0;
			wc.wr_id = mad_send_wr->wr_id;
			ib_mad_complete_send_wr(mad_send_wr, &wc);
			return;
		}
		if (mad_send_wr->refcount == 1)
			ib_reset_mad_timeout(mad_send_wr, mad_send_wr->
					     send_wr.wr.ud.timeout_ms);
	} else if (mad_send_wr->refcount == 1 &&
		   mad_send_wr->seg_num < mad_send_wr->newwin &&
		   mad_send_wr->seg_num <= mad_send_wr->total_seg) {
		/* Send failure will just result in a timeout/retry */
		ret = send_next_seg(mad_send_wr);
		if (ret)
			goto out;

		mad_send_wr->refcount++;
		list_del(&mad_send_wr->agent_list);
		list_add_tail(&mad_send_wr->agent_list,
			      &mad_send_wr->mad_agent_priv->send_list);
	}
out:
	spin_unlock_irqrestore(&agent->lock, flags);
}

static struct ib_mad_recv_wc *
process_rmpp_data(struct ib_mad_agent_private *agent,
		  struct ib_mad_recv_wc *mad_recv_wc)
{
	struct ib_rmpp_hdr *rmpp_hdr;
	u8 rmpp_status;

	rmpp_hdr = &((struct ib_rmpp_mad *)mad_recv_wc->recv_buf.mad)->rmpp_hdr;

	if (rmpp_hdr->rmpp_status) {
		rmpp_status = IB_MGMT_RMPP_STATUS_BAD_STATUS;
		goto bad;
	}

	if (rmpp_hdr->seg_num == __constant_htonl(1)) {
		if (!(ib_get_rmpp_flags(rmpp_hdr) & IB_MGMT_RMPP_FLAG_FIRST)) {
			rmpp_status = IB_MGMT_RMPP_STATUS_BAD_SEG;
			goto bad;
		}
		return start_rmpp(agent, mad_recv_wc);
	} else {
		if (ib_get_rmpp_flags(rmpp_hdr) & IB_MGMT_RMPP_FLAG_FIRST) {
			rmpp_status = IB_MGMT_RMPP_STATUS_BAD_SEG;
			goto bad;
		}
		return continue_rmpp(agent, mad_recv_wc);
	}
bad:
	nack_recv(agent, mad_recv_wc, rmpp_status);
	ib_free_recv_mad(mad_recv_wc);
	return NULL;
}

static void process_rmpp_stop(struct ib_mad_agent_private *agent,
			      struct ib_mad_recv_wc *mad_recv_wc)
{
	struct ib_rmpp_mad *rmpp_mad;

	rmpp_mad = (struct ib_rmpp_mad *)mad_recv_wc->recv_buf.mad;

	if (rmpp_mad->rmpp_hdr.rmpp_status != IB_MGMT_RMPP_STATUS_RESX) {
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   IB_MGMT_RMPP_STATUS_BAD_STATUS);
		nack_recv(agent, mad_recv_wc, IB_MGMT_RMPP_STATUS_BAD_STATUS);
	} else
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   rmpp_mad->rmpp_hdr.rmpp_status);
}

static void process_rmpp_abort(struct ib_mad_agent_private *agent,
			       struct ib_mad_recv_wc *mad_recv_wc)
{
	struct ib_rmpp_mad *rmpp_mad;

	rmpp_mad = (struct ib_rmpp_mad *)mad_recv_wc->recv_buf.mad;

	if (rmpp_mad->rmpp_hdr.rmpp_status < IB_MGMT_RMPP_STATUS_ABORT_MIN ||
	    rmpp_mad->rmpp_hdr.rmpp_status > IB_MGMT_RMPP_STATUS_ABORT_MAX) {
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   IB_MGMT_RMPP_STATUS_BAD_STATUS);
		nack_recv(agent, mad_recv_wc, IB_MGMT_RMPP_STATUS_BAD_STATUS);
	} else
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   rmpp_mad->rmpp_hdr.rmpp_status);
}

struct ib_mad_recv_wc *
ib_process_rmpp_recv_wc(struct ib_mad_agent_private *agent,
			struct ib_mad_recv_wc *mad_recv_wc)
{
	struct ib_rmpp_mad *rmpp_mad;

	rmpp_mad = (struct ib_rmpp_mad *)mad_recv_wc->recv_buf.mad;
	if (!(rmpp_mad->rmpp_hdr.rmpp_rtime_flags & IB_MGMT_RMPP_FLAG_ACTIVE))
		return mad_recv_wc;

	if (rmpp_mad->rmpp_hdr.rmpp_version != IB_MGMT_RMPP_VERSION) {
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   IB_MGMT_RMPP_STATUS_UNV);
		nack_recv(agent, mad_recv_wc, IB_MGMT_RMPP_STATUS_UNV);
		goto out;
	}

	switch (rmpp_mad->rmpp_hdr.rmpp_type) {
	case IB_MGMT_RMPP_TYPE_DATA:
		return process_rmpp_data(agent, mad_recv_wc);
	case IB_MGMT_RMPP_TYPE_ACK:
		process_rmpp_ack(agent, mad_recv_wc);
		break;
	case IB_MGMT_RMPP_TYPE_STOP:
		process_rmpp_stop(agent, mad_recv_wc);
		break;
	case IB_MGMT_RMPP_TYPE_ABORT:
		process_rmpp_abort(agent, mad_recv_wc);
		break;
	default:
		abort_send(agent, rmpp_mad->mad_hdr.tid,
			   IB_MGMT_RMPP_STATUS_BADT);
		nack_recv(agent, mad_recv_wc, IB_MGMT_RMPP_STATUS_BADT);
		break;
	}
out:
	ib_free_recv_mad(mad_recv_wc);
	return NULL;
}

int ib_send_rmpp_mad(struct ib_mad_send_wr_private *mad_send_wr)
{
	struct ib_rmpp_mad *rmpp_mad;
	int i, total_len, ret;

	rmpp_mad = (struct ib_rmpp_mad *)mad_send_wr->send_wr.wr.ud.mad_hdr;
	if (!(ib_get_rmpp_flags(&rmpp_mad->rmpp_hdr) &
	      IB_MGMT_RMPP_FLAG_ACTIVE))
		return IB_RMPP_RESULT_UNHANDLED;

	if (rmpp_mad->rmpp_hdr.rmpp_type != IB_MGMT_RMPP_TYPE_DATA)
		return IB_RMPP_RESULT_INTERNAL;

	if (mad_send_wr->send_wr.num_sge > 1)
		return -EINVAL;		/* TODO: support num_sge > 1 */

	mad_send_wr->seg_num = 1;
	mad_send_wr->newwin = 1;
	mad_send_wr->data_offset = data_offset(rmpp_mad->mad_hdr.mgmt_class);

	total_len = 0;
	for (i = 0; i < mad_send_wr->send_wr.num_sge; i++)
		total_len += mad_send_wr->send_wr.sg_list[i].length;

        mad_send_wr->total_seg = (total_len - mad_send_wr->data_offset) /
			(sizeof(struct ib_rmpp_mad) - mad_send_wr->data_offset);
	mad_send_wr->pad = total_len - offsetof(struct ib_rmpp_mad, data) -
			   be32_to_cpu(rmpp_mad->rmpp_hdr.paylen_newwin);

	/* We need to wait for the final ACK even if there isn't a response */
	mad_send_wr->refcount += (mad_send_wr->timeout == 0);
	ret = send_next_seg(mad_send_wr);
	if (!ret)
		return IB_RMPP_RESULT_CONSUMED;
	return ret;
}

int ib_process_rmpp_send_wc(struct ib_mad_send_wr_private *mad_send_wr,
			    struct ib_mad_send_wc *mad_send_wc)
{
	struct ib_rmpp_mad *rmpp_mad;
	struct ib_mad_send_buf *msg;
	int ret;

	rmpp_mad = (struct ib_rmpp_mad *)mad_send_wr->send_wr.wr.ud.mad_hdr;
	if (!(ib_get_rmpp_flags(&rmpp_mad->rmpp_hdr) &
	      IB_MGMT_RMPP_FLAG_ACTIVE))
		return IB_RMPP_RESULT_UNHANDLED; /* RMPP not active */

	if (rmpp_mad->rmpp_hdr.rmpp_type != IB_MGMT_RMPP_TYPE_DATA) {
		msg = (struct ib_mad_send_buf *) (unsigned long)
		      mad_send_wc->wr_id;
		if (rmpp_mad->rmpp_hdr.rmpp_type == IB_MGMT_RMPP_TYPE_ACK)
			ib_free_send_mad(msg);
		else
			free_msg(msg);
		return IB_RMPP_RESULT_INTERNAL;	 /* ACK, STOP, or ABORT */
	}

	if (mad_send_wc->status != IB_WC_SUCCESS ||
	    mad_send_wr->status != IB_WC_SUCCESS)
		return IB_RMPP_RESULT_PROCESSED; /* Canceled or send error */

	if (!mad_send_wr->timeout)
		return IB_RMPP_RESULT_PROCESSED; /* Response received */

	if (mad_send_wr->last_ack == mad_send_wr->total_seg) {
		mad_send_wr->timeout =
			msecs_to_jiffies(mad_send_wr->send_wr.wr.ud.timeout_ms);
		return IB_RMPP_RESULT_PROCESSED; /* Send done */
	}

	if (mad_send_wr->seg_num > mad_send_wr->newwin ||
	    mad_send_wr->seg_num > mad_send_wr->total_seg)
		return IB_RMPP_RESULT_PROCESSED; /* Wait for ACK */

	ret = send_next_seg(mad_send_wr);
	if (ret) {
		mad_send_wc->status = IB_WC_GENERAL_ERR;
		return IB_RMPP_RESULT_PROCESSED;
	}
	return IB_RMPP_RESULT_CONSUMED;
}

int ib_retry_rmpp(struct ib_mad_send_wr_private *mad_send_wr)
{
	struct ib_rmpp_mad *rmpp_mad;
	int ret;

	rmpp_mad = (struct ib_rmpp_mad *)mad_send_wr->send_wr.wr.ud.mad_hdr;
	if (!(ib_get_rmpp_flags(&rmpp_mad->rmpp_hdr) &
	      IB_MGMT_RMPP_FLAG_ACTIVE))
		return IB_RMPP_RESULT_UNHANDLED; /* RMPP not active */

	if (mad_send_wr->last_ack == mad_send_wr->total_seg)
		return IB_RMPP_RESULT_PROCESSED;

	mad_send_wr->seg_num = mad_send_wr->last_ack + 1;
	ret = send_next_seg(mad_send_wr);
	if (ret)
		return IB_RMPP_RESULT_PROCESSED;

	return IB_RMPP_RESULT_CONSUMED;
}
