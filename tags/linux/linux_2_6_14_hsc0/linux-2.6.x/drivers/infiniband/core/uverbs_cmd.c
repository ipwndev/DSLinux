/*
 * Copyright (c) 2005 Topspin Communications.  All rights reserved.
 * Copyright (c) 2005 Cisco Systems.  All rights reserved.
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

#include <asm/uaccess.h>

#include "uverbs.h"

#define INIT_UDATA(udata, ibuf, obuf, ilen, olen)			\
	do {								\
		(udata)->inbuf  = (void __user *) (ibuf);		\
		(udata)->outbuf = (void __user *) (obuf);		\
		(udata)->inlen  = (ilen);				\
		(udata)->outlen = (olen);				\
	} while (0)

ssize_t ib_uverbs_query_params(struct ib_uverbs_file *file,
			       const char __user *buf,
			       int in_len, int out_len)
{
	struct ib_uverbs_query_params      cmd;
	struct ib_uverbs_query_params_resp resp;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	memset(&resp, 0, sizeof resp);

	resp.num_cq_events = file->device->num_comp;

	if (copy_to_user((void __user *) (unsigned long) cmd.response, &resp, sizeof resp))
	    return -EFAULT;

	return in_len;
}

ssize_t ib_uverbs_get_context(struct ib_uverbs_file *file,
			      const char __user *buf,
			      int in_len, int out_len)
{
	struct ib_uverbs_get_context      cmd;
	struct ib_uverbs_get_context_resp resp;
	struct ib_udata                   udata;
	struct ib_device                 *ibdev = file->device->ib_dev;
	struct ib_ucontext		 *ucontext;
	int i;
	int ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	down(&file->mutex);

	if (file->ucontext) {
		ret = -EINVAL;
		goto err;
	}

	INIT_UDATA(&udata, buf + sizeof cmd,
		   (unsigned long) cmd.response + sizeof resp,
		   in_len - sizeof cmd, out_len - sizeof resp);

	ucontext = ibdev->alloc_ucontext(ibdev, &udata);
	if (IS_ERR(ucontext))
		return PTR_ERR(file->ucontext);

	ucontext->device = ibdev;
	INIT_LIST_HEAD(&ucontext->pd_list);
	INIT_LIST_HEAD(&ucontext->mr_list);
	INIT_LIST_HEAD(&ucontext->mw_list);
	INIT_LIST_HEAD(&ucontext->cq_list);
	INIT_LIST_HEAD(&ucontext->qp_list);
	INIT_LIST_HEAD(&ucontext->srq_list);
	INIT_LIST_HEAD(&ucontext->ah_list);

	resp.async_fd = file->async_file.fd;
	for (i = 0; i < file->device->num_comp; ++i)
		if (copy_to_user((void __user *) (unsigned long) cmd.cq_fd_tab +
				 i * sizeof (__u32),
				 &file->comp_file[i].fd, sizeof (__u32))) {
			ret = -EFAULT;
			goto err_free;
		}

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp)) {
		ret = -EFAULT;
		goto err_free;
	}

	file->ucontext = ucontext;
	up(&file->mutex);

	return in_len;

err_free:
	ibdev->dealloc_ucontext(ucontext);

err:
	up(&file->mutex);
	return ret;
}

ssize_t ib_uverbs_query_device(struct ib_uverbs_file *file,
			       const char __user *buf,
			       int in_len, int out_len)
{
	struct ib_uverbs_query_device      cmd;
	struct ib_uverbs_query_device_resp resp;
	struct ib_device_attr              attr;
	int                                ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	ret = ib_query_device(file->device->ib_dev, &attr);
	if (ret)
		return ret;

	memset(&resp, 0, sizeof resp);

	resp.fw_ver 		       = attr.fw_ver;
	resp.node_guid 		       = attr.node_guid;
	resp.sys_image_guid 	       = attr.sys_image_guid;
	resp.max_mr_size 	       = attr.max_mr_size;
	resp.page_size_cap 	       = attr.page_size_cap;
	resp.vendor_id 		       = attr.vendor_id;
	resp.vendor_part_id 	       = attr.vendor_part_id;
	resp.hw_ver 		       = attr.hw_ver;
	resp.max_qp 		       = attr.max_qp;
	resp.max_qp_wr 		       = attr.max_qp_wr;
	resp.device_cap_flags 	       = attr.device_cap_flags;
	resp.max_sge 		       = attr.max_sge;
	resp.max_sge_rd 	       = attr.max_sge_rd;
	resp.max_cq 		       = attr.max_cq;
	resp.max_cqe 		       = attr.max_cqe;
	resp.max_mr 		       = attr.max_mr;
	resp.max_pd 		       = attr.max_pd;
	resp.max_qp_rd_atom 	       = attr.max_qp_rd_atom;
	resp.max_ee_rd_atom 	       = attr.max_ee_rd_atom;
	resp.max_res_rd_atom 	       = attr.max_res_rd_atom;
	resp.max_qp_init_rd_atom       = attr.max_qp_init_rd_atom;
	resp.max_ee_init_rd_atom       = attr.max_ee_init_rd_atom;
	resp.atomic_cap 	       = attr.atomic_cap;
	resp.max_ee 		       = attr.max_ee;
	resp.max_rdd 		       = attr.max_rdd;
	resp.max_mw 		       = attr.max_mw;
	resp.max_raw_ipv6_qp 	       = attr.max_raw_ipv6_qp;
	resp.max_raw_ethy_qp 	       = attr.max_raw_ethy_qp;
	resp.max_mcast_grp 	       = attr.max_mcast_grp;
	resp.max_mcast_qp_attach       = attr.max_mcast_qp_attach;
	resp.max_total_mcast_qp_attach = attr.max_total_mcast_qp_attach;
	resp.max_ah 		       = attr.max_ah;
	resp.max_fmr 		       = attr.max_fmr;
	resp.max_map_per_fmr 	       = attr.max_map_per_fmr;
	resp.max_srq 		       = attr.max_srq;
	resp.max_srq_wr 	       = attr.max_srq_wr;
	resp.max_srq_sge 	       = attr.max_srq_sge;
	resp.max_pkeys 		       = attr.max_pkeys;
	resp.local_ca_ack_delay        = attr.local_ca_ack_delay;
	resp.phys_port_cnt	       = file->device->ib_dev->phys_port_cnt;

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp))
		return -EFAULT;

	return in_len;
}

ssize_t ib_uverbs_query_port(struct ib_uverbs_file *file,
			     const char __user *buf,
			     int in_len, int out_len)
{
	struct ib_uverbs_query_port      cmd;
	struct ib_uverbs_query_port_resp resp;
	struct ib_port_attr              attr;
	int                              ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	ret = ib_query_port(file->device->ib_dev, cmd.port_num, &attr);
	if (ret)
		return ret;

	memset(&resp, 0, sizeof resp);

	resp.state 	     = attr.state;
	resp.max_mtu 	     = attr.max_mtu;
	resp.active_mtu      = attr.active_mtu;
	resp.gid_tbl_len     = attr.gid_tbl_len;
	resp.port_cap_flags  = attr.port_cap_flags;
	resp.max_msg_sz      = attr.max_msg_sz;
	resp.bad_pkey_cntr   = attr.bad_pkey_cntr;
	resp.qkey_viol_cntr  = attr.qkey_viol_cntr;
	resp.pkey_tbl_len    = attr.pkey_tbl_len;
	resp.lid 	     = attr.lid;
	resp.sm_lid 	     = attr.sm_lid;
	resp.lmc 	     = attr.lmc;
	resp.max_vl_num      = attr.max_vl_num;
	resp.sm_sl 	     = attr.sm_sl;
	resp.subnet_timeout  = attr.subnet_timeout;
	resp.init_type_reply = attr.init_type_reply;
	resp.active_width    = attr.active_width;
	resp.active_speed    = attr.active_speed;
	resp.phys_state      = attr.phys_state;

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp))
		return -EFAULT;

	return in_len;
}

ssize_t ib_uverbs_query_gid(struct ib_uverbs_file *file,
			    const char __user *buf,
			    int in_len, int out_len)
{
	struct ib_uverbs_query_gid      cmd;
	struct ib_uverbs_query_gid_resp resp;
	int                             ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	memset(&resp, 0, sizeof resp);

	ret = ib_query_gid(file->device->ib_dev, cmd.port_num, cmd.index,
			   (union ib_gid *) resp.gid);
	if (ret)
		return ret;

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp))
		return -EFAULT;

	return in_len;
}

ssize_t ib_uverbs_query_pkey(struct ib_uverbs_file *file,
			     const char __user *buf,
			     int in_len, int out_len)
{
	struct ib_uverbs_query_pkey      cmd;
	struct ib_uverbs_query_pkey_resp resp;
	int                              ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	memset(&resp, 0, sizeof resp);

	ret = ib_query_pkey(file->device->ib_dev, cmd.port_num, cmd.index,
			    &resp.pkey);
	if (ret)
		return ret;

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp))
		return -EFAULT;

	return in_len;
}

ssize_t ib_uverbs_alloc_pd(struct ib_uverbs_file *file,
			   const char __user *buf,
			   int in_len, int out_len)
{
	struct ib_uverbs_alloc_pd      cmd;
	struct ib_uverbs_alloc_pd_resp resp;
	struct ib_udata                udata;
	struct ib_uobject             *uobj;
	struct ib_pd                  *pd;
	int                            ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	INIT_UDATA(&udata, buf + sizeof cmd,
		   (unsigned long) cmd.response + sizeof resp,
		   in_len - sizeof cmd, out_len - sizeof resp);

	uobj = kmalloc(sizeof *uobj, GFP_KERNEL);
	if (!uobj)
		return -ENOMEM;

	uobj->context = file->ucontext;

	pd = file->device->ib_dev->alloc_pd(file->device->ib_dev,
					    file->ucontext, &udata);
	if (IS_ERR(pd)) {
		ret = PTR_ERR(pd);
		goto err;
	}

	pd->device  = file->device->ib_dev;
	pd->uobject = uobj;
	atomic_set(&pd->usecnt, 0);

retry:
	if (!idr_pre_get(&ib_uverbs_pd_idr, GFP_KERNEL)) {
		ret = -ENOMEM;
		goto err_pd;
	}

	down(&ib_uverbs_idr_mutex);
	ret = idr_get_new(&ib_uverbs_pd_idr, pd, &uobj->id);
	up(&ib_uverbs_idr_mutex);

	if (ret == -EAGAIN)
		goto retry;
	if (ret)
		goto err_pd;

	down(&file->mutex);
	list_add_tail(&uobj->list, &file->ucontext->pd_list);
	up(&file->mutex);

	memset(&resp, 0, sizeof resp);
	resp.pd_handle = uobj->id;

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp)) {
		ret = -EFAULT;
		goto err_list;
	}

	return in_len;

err_list:
 	down(&file->mutex);
	list_del(&uobj->list);
	up(&file->mutex);

	down(&ib_uverbs_idr_mutex);
	idr_remove(&ib_uverbs_pd_idr, uobj->id);
	up(&ib_uverbs_idr_mutex);

err_pd:
	ib_dealloc_pd(pd);

err:
	kfree(uobj);
	return ret;
}

ssize_t ib_uverbs_dealloc_pd(struct ib_uverbs_file *file,
			     const char __user *buf,
			     int in_len, int out_len)
{
	struct ib_uverbs_dealloc_pd cmd;
	struct ib_pd               *pd;
	struct ib_uobject          *uobj;
	int                         ret = -EINVAL;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	down(&ib_uverbs_idr_mutex);

	pd = idr_find(&ib_uverbs_pd_idr, cmd.pd_handle);
	if (!pd || pd->uobject->context != file->ucontext)
		goto out;

	uobj = pd->uobject;

	ret = ib_dealloc_pd(pd);
	if (ret)
		goto out;

	idr_remove(&ib_uverbs_pd_idr, cmd.pd_handle);

	down(&file->mutex);
	list_del(&uobj->list);
	up(&file->mutex);

	kfree(uobj);

out:
	up(&ib_uverbs_idr_mutex);

	return ret ? ret : in_len;
}

ssize_t ib_uverbs_reg_mr(struct ib_uverbs_file *file,
			 const char __user *buf, int in_len,
			 int out_len)
{
	struct ib_uverbs_reg_mr      cmd;
	struct ib_uverbs_reg_mr_resp resp;
	struct ib_udata              udata;
	struct ib_umem_object       *obj;
	struct ib_pd                *pd;
	struct ib_mr                *mr;
	int                          ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	INIT_UDATA(&udata, buf + sizeof cmd,
		   (unsigned long) cmd.response + sizeof resp,
		   in_len - sizeof cmd, out_len - sizeof resp);

	if ((cmd.start & ~PAGE_MASK) != (cmd.hca_va & ~PAGE_MASK))
		return -EINVAL;

	obj = kmalloc(sizeof *obj, GFP_KERNEL);
	if (!obj)
		return -ENOMEM;

	obj->uobject.context = file->ucontext;

	/*
	 * We ask for writable memory if any access flags other than
	 * "remote read" are set.  "Local write" and "remote write"
	 * obviously require write access.  "Remote atomic" can do
	 * things like fetch and add, which will modify memory, and
	 * "MW bind" can change permissions by binding a window.
	 */
	ret = ib_umem_get(file->device->ib_dev, &obj->umem,
			  (void *) (unsigned long) cmd.start, cmd.length,
			  !!(cmd.access_flags & ~IB_ACCESS_REMOTE_READ));
	if (ret)
		goto err_free;

	obj->umem.virt_base = cmd.hca_va;

	down(&ib_uverbs_idr_mutex);

	pd = idr_find(&ib_uverbs_pd_idr, cmd.pd_handle);
	if (!pd || pd->uobject->context != file->ucontext) {
		ret = -EINVAL;
		goto err_up;
	}

	if (!pd->device->reg_user_mr) {
		ret = -ENOSYS;
		goto err_up;
	}

	mr = pd->device->reg_user_mr(pd, &obj->umem, cmd.access_flags, &udata);
	if (IS_ERR(mr)) {
		ret = PTR_ERR(mr);
		goto err_up;
	}

	mr->device  = pd->device;
	mr->pd      = pd;
	mr->uobject = &obj->uobject;
	atomic_inc(&pd->usecnt);
	atomic_set(&mr->usecnt, 0);

	memset(&resp, 0, sizeof resp);
	resp.lkey = mr->lkey;
	resp.rkey = mr->rkey;

retry:
	if (!idr_pre_get(&ib_uverbs_mr_idr, GFP_KERNEL)) {
		ret = -ENOMEM;
		goto err_unreg;
	}

	ret = idr_get_new(&ib_uverbs_mr_idr, mr, &obj->uobject.id);

	if (ret == -EAGAIN)
		goto retry;
	if (ret)
		goto err_unreg;

	resp.mr_handle = obj->uobject.id;

	down(&file->mutex);
	list_add_tail(&obj->uobject.list, &file->ucontext->mr_list);
	up(&file->mutex);

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp)) {
		ret = -EFAULT;
		goto err_list;
	}

	up(&ib_uverbs_idr_mutex);

	return in_len;

err_list:
	down(&file->mutex);
	list_del(&obj->uobject.list);
	up(&file->mutex);

err_unreg:
	ib_dereg_mr(mr);

err_up:
	up(&ib_uverbs_idr_mutex);

	ib_umem_release(file->device->ib_dev, &obj->umem);

err_free:
	kfree(obj);
	return ret;
}

ssize_t ib_uverbs_dereg_mr(struct ib_uverbs_file *file,
			   const char __user *buf, int in_len,
			   int out_len)
{
	struct ib_uverbs_dereg_mr cmd;
	struct ib_mr             *mr;
	struct ib_umem_object    *memobj;
	int                       ret = -EINVAL;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	down(&ib_uverbs_idr_mutex);

	mr = idr_find(&ib_uverbs_mr_idr, cmd.mr_handle);
	if (!mr || mr->uobject->context != file->ucontext)
		goto out;

	memobj = container_of(mr->uobject, struct ib_umem_object, uobject);

	ret = ib_dereg_mr(mr);
	if (ret)
		goto out;

	idr_remove(&ib_uverbs_mr_idr, cmd.mr_handle);

	down(&file->mutex);
	list_del(&memobj->uobject.list);
	up(&file->mutex);

	ib_umem_release(file->device->ib_dev, &memobj->umem);
	kfree(memobj);

out:
	up(&ib_uverbs_idr_mutex);

	return ret ? ret : in_len;
}

ssize_t ib_uverbs_create_cq(struct ib_uverbs_file *file,
			    const char __user *buf, int in_len,
			    int out_len)
{
	struct ib_uverbs_create_cq      cmd;
	struct ib_uverbs_create_cq_resp resp;
	struct ib_udata                 udata;
	struct ib_ucq_object           *uobj;
	struct ib_cq                   *cq;
	int                             ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	INIT_UDATA(&udata, buf + sizeof cmd,
		   (unsigned long) cmd.response + sizeof resp,
		   in_len - sizeof cmd, out_len - sizeof resp);

	if (cmd.event_handler >= file->device->num_comp)
		return -EINVAL;

	uobj = kmalloc(sizeof *uobj, GFP_KERNEL);
	if (!uobj)
		return -ENOMEM;

	uobj->uobject.user_handle   = cmd.user_handle;
	uobj->uobject.context       = file->ucontext;
	uobj->comp_events_reported  = 0;
	uobj->async_events_reported = 0;
	INIT_LIST_HEAD(&uobj->comp_list);
	INIT_LIST_HEAD(&uobj->async_list);

	cq = file->device->ib_dev->create_cq(file->device->ib_dev, cmd.cqe,
					     file->ucontext, &udata);
	if (IS_ERR(cq)) {
		ret = PTR_ERR(cq);
		goto err;
	}

	cq->device        = file->device->ib_dev;
	cq->uobject       = &uobj->uobject;
	cq->comp_handler  = ib_uverbs_comp_handler;
	cq->event_handler = ib_uverbs_cq_event_handler;
	cq->cq_context    = file;
	atomic_set(&cq->usecnt, 0);

retry:
	if (!idr_pre_get(&ib_uverbs_cq_idr, GFP_KERNEL)) {
		ret = -ENOMEM;
		goto err_cq;
	}

	down(&ib_uverbs_idr_mutex);
	ret = idr_get_new(&ib_uverbs_cq_idr, cq, &uobj->uobject.id);
	up(&ib_uverbs_idr_mutex);

	if (ret == -EAGAIN)
		goto retry;
	if (ret)
		goto err_cq;

	down(&file->mutex);
	list_add_tail(&uobj->uobject.list, &file->ucontext->cq_list);
	up(&file->mutex);

	memset(&resp, 0, sizeof resp);
	resp.cq_handle = uobj->uobject.id;
	resp.cqe       = cq->cqe;

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp)) {
		ret = -EFAULT;
		goto err_list;
	}

	return in_len;

err_list:
 	down(&file->mutex);
	list_del(&uobj->uobject.list);
	up(&file->mutex);

	down(&ib_uverbs_idr_mutex);
	idr_remove(&ib_uverbs_cq_idr, uobj->uobject.id);
	up(&ib_uverbs_idr_mutex);

err_cq:
	ib_destroy_cq(cq);

err:
	kfree(uobj);
	return ret;
}

ssize_t ib_uverbs_destroy_cq(struct ib_uverbs_file *file,
			     const char __user *buf, int in_len,
			     int out_len)
{
	struct ib_uverbs_destroy_cq      cmd;
	struct ib_uverbs_destroy_cq_resp resp;
	struct ib_cq               	*cq;
	struct ib_ucq_object        	*uobj;
	struct ib_uverbs_event		*evt, *tmp;
	u64				 user_handle;
	int                        	 ret = -EINVAL;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	memset(&resp, 0, sizeof resp);

	down(&ib_uverbs_idr_mutex);

	cq = idr_find(&ib_uverbs_cq_idr, cmd.cq_handle);
	if (!cq || cq->uobject->context != file->ucontext)
		goto out;

	user_handle = cq->uobject->user_handle;
	uobj = container_of(cq->uobject, struct ib_ucq_object, uobject);

	ret = ib_destroy_cq(cq);
	if (ret)
		goto out;

	idr_remove(&ib_uverbs_cq_idr, cmd.cq_handle);

	down(&file->mutex);
	list_del(&uobj->uobject.list);
	up(&file->mutex);

	spin_lock_irq(&file->comp_file[0].lock);
	list_for_each_entry_safe(evt, tmp, &uobj->comp_list, obj_list) {
		list_del(&evt->list);
		kfree(evt);
	}
	spin_unlock_irq(&file->comp_file[0].lock);

	spin_lock_irq(&file->async_file.lock);
	list_for_each_entry_safe(evt, tmp, &uobj->async_list, obj_list) {
		list_del(&evt->list);
		kfree(evt);
	}
	spin_unlock_irq(&file->async_file.lock);

	resp.comp_events_reported  = uobj->comp_events_reported;
	resp.async_events_reported = uobj->async_events_reported;

	kfree(uobj);

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp))
		ret = -EFAULT;

out:
	up(&ib_uverbs_idr_mutex);

	return ret ? ret : in_len;
}

ssize_t ib_uverbs_create_qp(struct ib_uverbs_file *file,
			    const char __user *buf, int in_len,
			    int out_len)
{
	struct ib_uverbs_create_qp      cmd;
	struct ib_uverbs_create_qp_resp resp;
	struct ib_udata                 udata;
	struct ib_uevent_object        *uobj;
	struct ib_pd                   *pd;
	struct ib_cq                   *scq, *rcq;
	struct ib_srq                  *srq;
	struct ib_qp                   *qp;
	struct ib_qp_init_attr          attr;
	int ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	INIT_UDATA(&udata, buf + sizeof cmd,
		   (unsigned long) cmd.response + sizeof resp,
		   in_len - sizeof cmd, out_len - sizeof resp);

	uobj = kmalloc(sizeof *uobj, GFP_KERNEL);
	if (!uobj)
		return -ENOMEM;

	down(&ib_uverbs_idr_mutex);

	pd  = idr_find(&ib_uverbs_pd_idr, cmd.pd_handle);
	scq = idr_find(&ib_uverbs_cq_idr, cmd.send_cq_handle);
	rcq = idr_find(&ib_uverbs_cq_idr, cmd.recv_cq_handle);
	srq = cmd.is_srq ? idr_find(&ib_uverbs_srq_idr, cmd.srq_handle) : NULL;

	if (!pd  || pd->uobject->context  != file->ucontext ||
	    !scq || scq->uobject->context != file->ucontext ||
	    !rcq || rcq->uobject->context != file->ucontext ||
	    (cmd.is_srq && (!srq || srq->uobject->context != file->ucontext))) {
		ret = -EINVAL;
		goto err_up;
	}

	attr.event_handler = ib_uverbs_qp_event_handler;
	attr.qp_context    = file;
	attr.send_cq       = scq;
	attr.recv_cq       = rcq;
	attr.srq           = srq;
	attr.sq_sig_type   = cmd.sq_sig_all ? IB_SIGNAL_ALL_WR : IB_SIGNAL_REQ_WR;
	attr.qp_type       = cmd.qp_type;

	attr.cap.max_send_wr     = cmd.max_send_wr;
	attr.cap.max_recv_wr     = cmd.max_recv_wr;
	attr.cap.max_send_sge    = cmd.max_send_sge;
	attr.cap.max_recv_sge    = cmd.max_recv_sge;
	attr.cap.max_inline_data = cmd.max_inline_data;

	uobj->uobject.user_handle = cmd.user_handle;
	uobj->uobject.context     = file->ucontext;
	uobj->events_reported     = 0;
	INIT_LIST_HEAD(&uobj->event_list);

	qp = pd->device->create_qp(pd, &attr, &udata);
	if (IS_ERR(qp)) {
		ret = PTR_ERR(qp);
		goto err_up;
	}

	qp->device     	  = pd->device;
	qp->pd         	  = pd;
	qp->send_cq    	  = attr.send_cq;
	qp->recv_cq    	  = attr.recv_cq;
	qp->srq	       	  = attr.srq;
	qp->uobject       = &uobj->uobject;
	qp->event_handler = attr.event_handler;
	qp->qp_context    = attr.qp_context;
	qp->qp_type	  = attr.qp_type;
	atomic_inc(&pd->usecnt);
	atomic_inc(&attr.send_cq->usecnt);
	atomic_inc(&attr.recv_cq->usecnt);
	if (attr.srq)
		atomic_inc(&attr.srq->usecnt);

	memset(&resp, 0, sizeof resp);
	resp.qpn = qp->qp_num;

retry:
	if (!idr_pre_get(&ib_uverbs_qp_idr, GFP_KERNEL)) {
		ret = -ENOMEM;
		goto err_destroy;
	}

	ret = idr_get_new(&ib_uverbs_qp_idr, qp, &uobj->uobject.id);

	if (ret == -EAGAIN)
		goto retry;
	if (ret)
		goto err_destroy;

	resp.qp_handle = uobj->uobject.id;

	down(&file->mutex);
	list_add_tail(&uobj->uobject.list, &file->ucontext->qp_list);
	up(&file->mutex);

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp)) {
		ret = -EFAULT;
		goto err_list;
	}

	up(&ib_uverbs_idr_mutex);

	return in_len;

err_list:
	down(&file->mutex);
	list_del(&uobj->uobject.list);
	up(&file->mutex);

err_destroy:
	ib_destroy_qp(qp);

err_up:
	up(&ib_uverbs_idr_mutex);

	kfree(uobj);
	return ret;
}

ssize_t ib_uverbs_modify_qp(struct ib_uverbs_file *file,
			    const char __user *buf, int in_len,
			    int out_len)
{
	struct ib_uverbs_modify_qp cmd;
	struct ib_qp              *qp;
	struct ib_qp_attr         *attr;
	int                        ret;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	attr = kmalloc(sizeof *attr, GFP_KERNEL);
	if (!attr)
		return -ENOMEM;

	down(&ib_uverbs_idr_mutex);

	qp = idr_find(&ib_uverbs_qp_idr, cmd.qp_handle);
	if (!qp || qp->uobject->context != file->ucontext) {
		ret = -EINVAL;
		goto out;
	}

	attr->qp_state 		  = cmd.qp_state;
	attr->cur_qp_state 	  = cmd.cur_qp_state;
	attr->path_mtu 		  = cmd.path_mtu;
	attr->path_mig_state 	  = cmd.path_mig_state;
	attr->qkey 		  = cmd.qkey;
	attr->rq_psn 		  = cmd.rq_psn;
	attr->sq_psn 		  = cmd.sq_psn;
	attr->dest_qp_num 	  = cmd.dest_qp_num;
	attr->qp_access_flags 	  = cmd.qp_access_flags;
	attr->pkey_index 	  = cmd.pkey_index;
	attr->alt_pkey_index 	  = cmd.pkey_index;
	attr->en_sqd_async_notify = cmd.en_sqd_async_notify;
	attr->max_rd_atomic 	  = cmd.max_rd_atomic;
	attr->max_dest_rd_atomic  = cmd.max_dest_rd_atomic;
	attr->min_rnr_timer 	  = cmd.min_rnr_timer;
	attr->port_num 		  = cmd.port_num;
	attr->timeout 		  = cmd.timeout;
	attr->retry_cnt 	  = cmd.retry_cnt;
	attr->rnr_retry 	  = cmd.rnr_retry;
	attr->alt_port_num 	  = cmd.alt_port_num;
	attr->alt_timeout 	  = cmd.alt_timeout;

	memcpy(attr->ah_attr.grh.dgid.raw, cmd.dest.dgid, 16);
	attr->ah_attr.grh.flow_label        = cmd.dest.flow_label;
	attr->ah_attr.grh.sgid_index        = cmd.dest.sgid_index;
	attr->ah_attr.grh.hop_limit         = cmd.dest.hop_limit;
	attr->ah_attr.grh.traffic_class     = cmd.dest.traffic_class;
	attr->ah_attr.dlid 	    	    = cmd.dest.dlid;
	attr->ah_attr.sl   	    	    = cmd.dest.sl;
	attr->ah_attr.src_path_bits 	    = cmd.dest.src_path_bits;
	attr->ah_attr.static_rate   	    = cmd.dest.static_rate;
	attr->ah_attr.ah_flags 	    	    = cmd.dest.is_global ? IB_AH_GRH : 0;
	attr->ah_attr.port_num 	    	    = cmd.dest.port_num;

	memcpy(attr->alt_ah_attr.grh.dgid.raw, cmd.alt_dest.dgid, 16);
	attr->alt_ah_attr.grh.flow_label    = cmd.alt_dest.flow_label;
	attr->alt_ah_attr.grh.sgid_index    = cmd.alt_dest.sgid_index;
	attr->alt_ah_attr.grh.hop_limit     = cmd.alt_dest.hop_limit;
	attr->alt_ah_attr.grh.traffic_class = cmd.alt_dest.traffic_class;
	attr->alt_ah_attr.dlid 	    	    = cmd.alt_dest.dlid;
	attr->alt_ah_attr.sl   	    	    = cmd.alt_dest.sl;
	attr->alt_ah_attr.src_path_bits     = cmd.alt_dest.src_path_bits;
	attr->alt_ah_attr.static_rate       = cmd.alt_dest.static_rate;
	attr->alt_ah_attr.ah_flags 	    = cmd.alt_dest.is_global ? IB_AH_GRH : 0;
	attr->alt_ah_attr.port_num 	    = cmd.alt_dest.port_num;

	ret = ib_modify_qp(qp, attr, cmd.attr_mask);
	if (ret)
		goto out;

	ret = in_len;

out:
	up(&ib_uverbs_idr_mutex);
	kfree(attr);

	return ret;
}

ssize_t ib_uverbs_destroy_qp(struct ib_uverbs_file *file,
			     const char __user *buf, int in_len,
			     int out_len)
{
	struct ib_uverbs_destroy_qp      cmd;
	struct ib_uverbs_destroy_qp_resp resp;
	struct ib_qp               	*qp;
	struct ib_uevent_object        	*uobj;
	struct ib_uverbs_event		*evt, *tmp;
	int                        	 ret = -EINVAL;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	memset(&resp, 0, sizeof resp);

	down(&ib_uverbs_idr_mutex);

	qp = idr_find(&ib_uverbs_qp_idr, cmd.qp_handle);
	if (!qp || qp->uobject->context != file->ucontext)
		goto out;

	uobj = container_of(qp->uobject, struct ib_uevent_object, uobject);

	ret = ib_destroy_qp(qp);
	if (ret)
		goto out;

	idr_remove(&ib_uverbs_qp_idr, cmd.qp_handle);

	down(&file->mutex);
	list_del(&uobj->uobject.list);
	up(&file->mutex);

	spin_lock_irq(&file->async_file.lock);
	list_for_each_entry_safe(evt, tmp, &uobj->event_list, obj_list) {
		list_del(&evt->list);
		kfree(evt);
	}
	spin_unlock_irq(&file->async_file.lock);

	resp.events_reported = uobj->events_reported;

	kfree(uobj);

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp))
		ret = -EFAULT;

out:
	up(&ib_uverbs_idr_mutex);

	return ret ? ret : in_len;
}

ssize_t ib_uverbs_attach_mcast(struct ib_uverbs_file *file,
			       const char __user *buf, int in_len,
			       int out_len)
{
	struct ib_uverbs_attach_mcast cmd;
	struct ib_qp                 *qp;
	int                           ret = -EINVAL;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	down(&ib_uverbs_idr_mutex);

	qp = idr_find(&ib_uverbs_qp_idr, cmd.qp_handle);
	if (qp && qp->uobject->context == file->ucontext)
		ret = ib_attach_mcast(qp, (union ib_gid *) cmd.gid, cmd.mlid);

	up(&ib_uverbs_idr_mutex);

	return ret ? ret : in_len;
}

ssize_t ib_uverbs_detach_mcast(struct ib_uverbs_file *file,
			       const char __user *buf, int in_len,
			       int out_len)
{
	struct ib_uverbs_detach_mcast cmd;
	struct ib_qp                 *qp;
	int                           ret = -EINVAL;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	down(&ib_uverbs_idr_mutex);

	qp = idr_find(&ib_uverbs_qp_idr, cmd.qp_handle);
	if (qp && qp->uobject->context == file->ucontext)
		ret = ib_detach_mcast(qp, (union ib_gid *) cmd.gid, cmd.mlid);

	up(&ib_uverbs_idr_mutex);

	return ret ? ret : in_len;
}

ssize_t ib_uverbs_create_srq(struct ib_uverbs_file *file,
			     const char __user *buf, int in_len,
			     int out_len)
{
	struct ib_uverbs_create_srq      cmd;
	struct ib_uverbs_create_srq_resp resp;
	struct ib_udata                  udata;
	struct ib_uevent_object         *uobj;
	struct ib_pd                    *pd;
	struct ib_srq                   *srq;
	struct ib_srq_init_attr          attr;
	int ret;

	if (out_len < sizeof resp)
		return -ENOSPC;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	INIT_UDATA(&udata, buf + sizeof cmd,
		   (unsigned long) cmd.response + sizeof resp,
		   in_len - sizeof cmd, out_len - sizeof resp);

	uobj = kmalloc(sizeof *uobj, GFP_KERNEL);
	if (!uobj)
		return -ENOMEM;

	down(&ib_uverbs_idr_mutex);

	pd  = idr_find(&ib_uverbs_pd_idr, cmd.pd_handle);

	if (!pd || pd->uobject->context != file->ucontext) {
		ret = -EINVAL;
		goto err_up;
	}

	attr.event_handler  = ib_uverbs_srq_event_handler;
	attr.srq_context    = file;
	attr.attr.max_wr    = cmd.max_wr;
	attr.attr.max_sge   = cmd.max_sge;
	attr.attr.srq_limit = cmd.srq_limit;

	uobj->uobject.user_handle = cmd.user_handle;
	uobj->uobject.context     = file->ucontext;
	uobj->events_reported     = 0;
	INIT_LIST_HEAD(&uobj->event_list);

	srq = pd->device->create_srq(pd, &attr, &udata);
	if (IS_ERR(srq)) {
		ret = PTR_ERR(srq);
		goto err_up;
	}

	srq->device    	   = pd->device;
	srq->pd        	   = pd;
	srq->uobject       = &uobj->uobject;
	srq->event_handler = attr.event_handler;
	srq->srq_context   = attr.srq_context;
	atomic_inc(&pd->usecnt);
	atomic_set(&srq->usecnt, 0);

	memset(&resp, 0, sizeof resp);

retry:
	if (!idr_pre_get(&ib_uverbs_srq_idr, GFP_KERNEL)) {
		ret = -ENOMEM;
		goto err_destroy;
	}

	ret = idr_get_new(&ib_uverbs_srq_idr, srq, &uobj->uobject.id);

	if (ret == -EAGAIN)
		goto retry;
	if (ret)
		goto err_destroy;

	resp.srq_handle = uobj->uobject.id;

	down(&file->mutex);
	list_add_tail(&uobj->uobject.list, &file->ucontext->srq_list);
	up(&file->mutex);

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp)) {
		ret = -EFAULT;
		goto err_list;
	}

	up(&ib_uverbs_idr_mutex);

	return in_len;

err_list:
	down(&file->mutex);
	list_del(&uobj->uobject.list);
	up(&file->mutex);

err_destroy:
	ib_destroy_srq(srq);

err_up:
	up(&ib_uverbs_idr_mutex);

	kfree(uobj);
	return ret;
}

ssize_t ib_uverbs_modify_srq(struct ib_uverbs_file *file,
			     const char __user *buf, int in_len,
			     int out_len)
{
	struct ib_uverbs_modify_srq cmd;
	struct ib_srq              *srq;
	struct ib_srq_attr          attr;
	int                         ret;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	down(&ib_uverbs_idr_mutex);

	srq = idr_find(&ib_uverbs_srq_idr, cmd.srq_handle);
	if (!srq || srq->uobject->context != file->ucontext) {
		ret = -EINVAL;
		goto out;
	}

	attr.max_wr    = cmd.max_wr;
	attr.max_sge   = cmd.max_sge;
	attr.srq_limit = cmd.srq_limit;

	ret = ib_modify_srq(srq, &attr, cmd.attr_mask);

out:
	up(&ib_uverbs_idr_mutex);

	return ret ? ret : in_len;
}

ssize_t ib_uverbs_destroy_srq(struct ib_uverbs_file *file,
			      const char __user *buf, int in_len,
			      int out_len)
{
	struct ib_uverbs_destroy_srq      cmd;
	struct ib_uverbs_destroy_srq_resp resp;
	struct ib_srq               	 *srq;
	struct ib_uevent_object        	 *uobj;
	struct ib_uverbs_event		 *evt, *tmp;
	int                         	  ret = -EINVAL;

	if (copy_from_user(&cmd, buf, sizeof cmd))
		return -EFAULT;

	down(&ib_uverbs_idr_mutex);

	memset(&resp, 0, sizeof resp);

	srq = idr_find(&ib_uverbs_srq_idr, cmd.srq_handle);
	if (!srq || srq->uobject->context != file->ucontext)
		goto out;

	uobj = container_of(srq->uobject, struct ib_uevent_object, uobject);

	ret = ib_destroy_srq(srq);
	if (ret)
		goto out;

	idr_remove(&ib_uverbs_srq_idr, cmd.srq_handle);

	down(&file->mutex);
	list_del(&uobj->uobject.list);
	up(&file->mutex);

	spin_lock_irq(&file->async_file.lock);
	list_for_each_entry_safe(evt, tmp, &uobj->event_list, obj_list) {
		list_del(&evt->list);
		kfree(evt);
	}
	spin_unlock_irq(&file->async_file.lock);

	resp.events_reported = uobj->events_reported;

	kfree(uobj);

	if (copy_to_user((void __user *) (unsigned long) cmd.response,
			 &resp, sizeof resp))
		ret = -EFAULT;

out:
	up(&ib_uverbs_idr_mutex);

	return ret ? ret : in_len;
}
