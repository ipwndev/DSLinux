/* Shared library add-on to iptables to add CLUSTERIP target support. 
 * (C) 2003 by Harald Welte <laforge@gnumonks.org>
 *
 * Development of this code was funded by SuSE AG, http://www.suse.com/
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif

#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_CLUSTERIP.h>

static void
help(void)
{
	printf(
"CLUSTERIP target v%s options:\n"
"  --new			 Create a new ClusterIP\n"
"  --hashmode <mode>		 Specify hashing mode\n"
"					sourceip\n"
"					sourceip-sourceport\n"
"					sourceip-sourceport-destport\n"
"  --clustermac <mac>		 Set clusterIP MAC address\n"
"  --total-nodes <num>		 Set number of total nodes in cluster\n"
"  --local-node <num>		 Set the local node number\n"
"  --hash-init\n"
"\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "new", 0, 0, '1' },
	{ "hashmode", 1, 0, '2' },
	{ "clustermac", 1, 0, '3' },
	{ "total-nodes", 1, 0, '4' },
	{ "local-node", 1, 0, '5' },
	{ 0 }
};

static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
}

static void
parse_mac(const char *mac, char *macbuf)
{
	unsigned int i = 0;

	if (strlen(mac) != ETH_ALEN*3-1)
		exit_error(PARAMETER_PROBLEM, "Bad mac address `%s'", mac);

	for (i = 0; i < ETH_ALEN; i++) {
		long number;
		char *end;

		number = strtol(mac + i*3, &end, 16);

		if (end == mac + i*3 + 2
		    && number >= 0
		    && number <= 255)
			macbuf[i] = number;
		else
			exit_error(PARAMETER_PROBLEM,
				   "Bad mac address `%s'", mac);
	}
}

#define	PARAM_NEW	0x0001
#define PARAM_HMODE	0x0002
#define PARAM_MAC	0x0004
#define PARAM_TOTALNODE	0x0008
#define PARAM_LOCALNODE	0x0010

static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_clusterip_tgt_info *cipinfo
		= (struct ipt_clusterip_tgt_info *)(*target)->data;

	switch (c) {
		unsigned int num;
	case '1':
		cipinfo->flags |= CLUSTERIP_FLAG_NEW;
		if (*flags & PARAM_NEW)
			exit_error(PARAMETER_PROBLEM, "Can only specify `--new' once\n");
		*flags |= PARAM_NEW;
		break;
	case '2':
		if (!(*flags & PARAM_NEW))
			exit_error(PARAMETER_PROBLEM, "Can only specify hashmode combined with `--new'\n");
		if (*flags & PARAM_HMODE)
			exit_error(PARAMETER_PROBLEM, "Can only specify hashmode once\n");
		if (!strcmp(optarg, "sourceip"))
			cipinfo->hash_mode = CLUSTERIP_HASHMODE_SIP;
		else if (!strcmp(optarg, "sourceip-sourceport"))
			cipinfo->hash_mode = CLUSTERIP_HASHMODE_SIP_SPT;
		else if (!strcmp(optarg, "sourceip-sourceport-destport"))
			cipinfo->hash_mode = CLUSTERIP_HASHMODE_SIP_SPT_DPT;
		else
			exit_error(PARAMETER_PROBLEM, "Unknown hashmode `%s'\n",
				   optarg);
		*flags |= PARAM_HMODE;
		break;
	case '3':
		if (!(*flags & PARAM_NEW))
			exit_error(PARAMETER_PROBLEM, "Can only specify MAC combined with `--new'\n");
		if (*flags & PARAM_MAC)
			exit_error(PARAMETER_PROBLEM, "Can only specify MAC once\n");
		parse_mac(optarg, cipinfo->clustermac);
		if (!(cipinfo->clustermac[0] & 0x01))
			exit_error(PARAMETER_PROBLEM, "MAC has to be a multicast ethernet address\n");
		*flags |= PARAM_MAC;
		break;
	case '4':
		if (!(*flags & PARAM_NEW))
			exit_error(PARAMETER_PROBLEM, "Can only specify node number combined with `--new'\n");
		if (*flags & PARAM_TOTALNODE)
			exit_error(PARAMETER_PROBLEM, "Can only specify total node number once\n");
		if (string_to_number(optarg, 1, CLUSTERIP_MAX_NODES, &num) < 0)
			exit_error(PARAMETER_PROBLEM, "Unable to parse `%s'\n", optarg);
		cipinfo->num_total_nodes = (u_int16_t)num;
		*flags |= PARAM_TOTALNODE;
		break;
	case '5':
		if (!(*flags & PARAM_NEW))
			exit_error(PARAMETER_PROBLEM, "Can only specify node number combined with `--new'\n");
		if (*flags & PARAM_LOCALNODE)
			exit_error(PARAMETER_PROBLEM, "Can only specify local node number once\n");
		if (string_to_number(optarg, 1, CLUSTERIP_MAX_NODES, &num) < 0)
			exit_error(PARAMETER_PROBLEM, "Unable to parse `%s'\n", optarg);
		cipinfo->num_local_nodes = 1;
		cipinfo->local_nodes[0] = (u_int16_t)num;
		*flags |= PARAM_LOCALNODE;
		break;
	default:
		return 0;
	}

	return 1;
}

static void
final_check(unsigned int flags)
{
	if (flags == 0)
		return;

	if (flags == (PARAM_NEW|PARAM_HMODE|PARAM_MAC|PARAM_TOTALNODE|PARAM_LOCALNODE))
		return;

	exit_error(PARAMETER_PROBLEM, "CLUSTERIP target: Invalid parameter combination\n");
}

static char *hashmode2str(enum clusterip_hashmode mode)
{
	char *retstr;
	switch (mode) {
		case CLUSTERIP_HASHMODE_SIP:
			retstr = "sourceip";
			break;
		case CLUSTERIP_HASHMODE_SIP_SPT:
			retstr = "sourceip-sourceport";
			break;
		case CLUSTERIP_HASHMODE_SIP_SPT_DPT:
			retstr = "sourceip-sourceport-destport";
			break;
		default:
			retstr = "unknown-error";
			break;
	}
	return retstr;
}

static char *mac2str(u_int8_t mac[ETH_ALEN])
{
	static char buf[ETH_ALEN*3];
	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return buf;
}
			

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	const struct ipt_clusterip_tgt_info *cipinfo =
		(const struct ipt_clusterip_tgt_info *)target->data;
	
	if (!cipinfo->flags & CLUSTERIP_FLAG_NEW) {
		printf("CLUSTERIP");
		return;
	}

	printf("CLUSTERIP hashmode=%s clustermac=%s total_nodes=%u local_node=%u ", 
		hashmode2str(cipinfo->hash_mode),
		mac2str(cipinfo->clustermac),
		cipinfo->num_total_nodes,
		cipinfo->local_nodes[0]);
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	/*
	const struct ipt_connmark_target_info *markinfo =
		(const struct ipt_connmark_target_info *)target->data;

	switch (markinfo->mode) {
	case IPT_CONNMARK_SET:
	    printf("--set-mark 0x%lx ", markinfo->mark);
	    break;
	case IPT_CONNMARK_SAVE:
	    printf("--save-mark ");
	    break;
	case IPT_CONNMARK_RESTORE:
	    printf("--restore-mark ");
	    break;
	default:
	    printf("ERROR: UNKNOWN CONNMARK MODE ");
	    break;
	}
	*/
}

static
struct iptables_target clusterip
= { NULL,
    "CLUSTERIP",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_clusterip_tgt_info)),
    IPT_ALIGN(sizeof(struct ipt_clusterip_tgt_info)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    &save,
    opts
};

void _init(void)
{
	register_target(&clusterip);
}
