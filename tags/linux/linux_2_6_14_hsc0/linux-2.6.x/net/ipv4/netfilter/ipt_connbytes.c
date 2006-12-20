/* Kernel module to match connection tracking byte counter.
 * GPL (C) 2002 Martin Devera (devik@cdi.cz).
 *
 * 2004-07-20 Harald Welte <laforge@netfilter.org>
 * 	- reimplemented to use per-connection accounting counters
 * 	- add functionality to match number of packets
 * 	- add functionality to match average packet size
 * 	- add support to match directions seperately
 *
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_connbytes.h>

#include <asm/div64.h>
#include <asm/bitops.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@netfilter.org>");
MODULE_DESCRIPTION("iptables match for matching number of pkts/bytes per connection");

/* 64bit divisor, dividend and result. dynamic precision */
static u_int64_t div64_64(u_int64_t dividend, u_int64_t divisor)
{
	u_int32_t d = divisor;

	if (divisor > 0xffffffffULL) {
		unsigned int shift = fls(divisor >> 32);

		d = divisor >> shift;
		dividend >>= shift;
	}

	do_div(dividend, d);
	return dividend;
}

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,
      int *hotdrop)
{
	const struct ipt_connbytes_info *sinfo = matchinfo;
	enum ip_conntrack_info ctinfo;
	struct ip_conntrack *ct;
	u_int64_t what = 0;	/* initialize to make gcc happy */

	if (!(ct = ip_conntrack_get((struct sk_buff *)skb, &ctinfo)))
		return 0; /* no match */

	switch (sinfo->what) {
	case IPT_CONNBYTES_PKTS:
		switch (sinfo->direction) {
		case IPT_CONNBYTES_DIR_ORIGINAL:
			what = ct->counters[IP_CT_DIR_ORIGINAL].packets;
			break;
		case IPT_CONNBYTES_DIR_REPLY:
			what = ct->counters[IP_CT_DIR_REPLY].packets;
			break;
		case IPT_CONNBYTES_DIR_BOTH:
			what = ct->counters[IP_CT_DIR_ORIGINAL].packets;
			what += ct->counters[IP_CT_DIR_REPLY].packets;
			break;
		}
		break;
	case IPT_CONNBYTES_BYTES:
		switch (sinfo->direction) {
		case IPT_CONNBYTES_DIR_ORIGINAL:
			what = ct->counters[IP_CT_DIR_ORIGINAL].bytes;
			break;
		case IPT_CONNBYTES_DIR_REPLY:
			what = ct->counters[IP_CT_DIR_REPLY].bytes;
			break;
		case IPT_CONNBYTES_DIR_BOTH:
			what = ct->counters[IP_CT_DIR_ORIGINAL].bytes;
			what += ct->counters[IP_CT_DIR_REPLY].bytes;
			break;
		}
		break;
	case IPT_CONNBYTES_AVGPKT:
		switch (sinfo->direction) {
		case IPT_CONNBYTES_DIR_ORIGINAL:
			what = div64_64(ct->counters[IP_CT_DIR_ORIGINAL].bytes,
					ct->counters[IP_CT_DIR_ORIGINAL].packets);
			break;
		case IPT_CONNBYTES_DIR_REPLY:
			what = div64_64(ct->counters[IP_CT_DIR_REPLY].bytes,
					ct->counters[IP_CT_DIR_REPLY].packets);
			break;
		case IPT_CONNBYTES_DIR_BOTH:
			{
				u_int64_t bytes;
				u_int64_t pkts;
				bytes = ct->counters[IP_CT_DIR_ORIGINAL].bytes +
					ct->counters[IP_CT_DIR_REPLY].bytes;
				pkts = ct->counters[IP_CT_DIR_ORIGINAL].packets+
					ct->counters[IP_CT_DIR_REPLY].packets;

				/* FIXME_THEORETICAL: what to do if sum
				 * overflows ? */

				what = div64_64(bytes, pkts);
			}
			break;
		}
		break;
	}

	if (sinfo->count.to)
		return (what <= sinfo->count.to && what >= sinfo->count.from);
	else
		return (what >= sinfo->count.from);
}

static int check(const char *tablename,
		 const struct ipt_ip *ip,
		 void *matchinfo,
		 unsigned int matchsize,
		 unsigned int hook_mask)
{
	const struct ipt_connbytes_info *sinfo = matchinfo;

	if (matchsize != IPT_ALIGN(sizeof(struct ipt_connbytes_info)))
		return 0;

	if (sinfo->what != IPT_CONNBYTES_PKTS &&
	    sinfo->what != IPT_CONNBYTES_BYTES &&
	    sinfo->what != IPT_CONNBYTES_AVGPKT)
		return 0;

	if (sinfo->direction != IPT_CONNBYTES_DIR_ORIGINAL &&
	    sinfo->direction != IPT_CONNBYTES_DIR_REPLY &&
	    sinfo->direction != IPT_CONNBYTES_DIR_BOTH)
		return 0;

	return 1;
}

static struct ipt_match state_match = {
	.name		= "connbytes",
	.match		= &match,
	.checkentry	= &check,
	.me		= THIS_MODULE
};

static int __init init(void)
{
	return ipt_register_match(&state_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&state_match);
}

module_init(init);
module_exit(fini);
