/* -*- linux-c -*-
 * sysctl_net.c: sysctl interface to net subsystem.
 *
 * Begun April 1, 1996, Mike Shaver.
 * Added /proc/sys/net directories for each protocol family. [MS]
 *
 * $Log$
 * Revision 1.2  2006/02/20 17:04:46  stsp2
 * First step in merging 2.6.14 to HEAD. This commit still lacks patches to
 * actually make things work with 2.6.14. There may also be more files we
 * need in user/, lib/, or the buildsystem.
 *
 * Note that this commit includes the *whole* linux kernel.
 * We might want to hand-pick bits we do not need out later to keep
 * working copies at a reasonable size. But for now the focus is on
 * getting things working with 2.6.14.
 *
 * 2.6.9 is gone onto the dslinux_2_6_9_branch branch.
 *
 * Revision 1.1.2.1  2006/02/18 14:07:18  stsp2
 * adding linux-2.6.14-hsc0
 *
 * Revision 1.2  1996/05/08  20:24:40  shaver
 * Added bits for NET_BRIDGE and the NET_IPV4_ARP stuff and
 * NET_IPV4_IP_FORWARD.
 *
 *
 */

#include <linux/config.h>
#include <linux/mm.h>
#include <linux/sysctl.h>

#include <net/sock.h>

#ifdef CONFIG_INET
#include <net/ip.h>
#endif

#ifdef CONFIG_NET
#include <linux/if_ether.h>
#endif

#ifdef CONFIG_TR
#include <linux/if_tr.h>
#endif

struct ctl_table net_table[] = {
	{
		.ctl_name	= NET_CORE,
		.procname	= "core",
		.mode		= 0555,
		.child		= core_table,
	},
#ifdef CONFIG_NET
	{
		.ctl_name	= NET_ETHER,
		.procname	= "ethernet",
		.mode		= 0555,
		.child		= ether_table,
	},
#endif
#ifdef CONFIG_INET
	{
		.ctl_name	= NET_IPV4,
		.procname	= "ipv4",
		.mode		= 0555,
		.child		= ipv4_table
	},
#endif
#ifdef CONFIG_TR
	{
		.ctl_name	= NET_TR,
		.procname	= "token-ring",
		.mode		= 0555,
		.child		= tr_table,
	},
#endif
	{ 0 },
};
