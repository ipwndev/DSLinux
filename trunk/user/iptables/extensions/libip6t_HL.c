/*
 * IPv6 Hop Limit Target module
 * Maciej Soltysiak <solt@dns.toxicfilms.tv>
 * Based on HW's ttl target
 * This program is distributed under the terms of GNU GPL
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <ip6tables.h>

#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter_ipv6/ip6t_HL.h>

#define IP6T_HL_USED	1

static void init(struct ip6t_entry_target *t, unsigned int *nfcache) 
{
}

static void help(void) 
{
	printf(
"HL target v%s options\n"
"  --hl-set value		Set HL to <value>\n"
"  --hl-dec value		Decrement HL by <value>\n"
"  --hl-inc value		Increment HL by <value>\n"
, IPTABLES_VERSION);
}

static int parse(int c, char **argv, int invert, unsigned int *flags,
		const struct ip6t_entry *entry,
		struct ip6t_entry_target **target)
{
	struct ip6t_HL_info *info = (struct ip6t_HL_info *) (*target)->data;
	u_int8_t value;

	if (*flags & IP6T_HL_USED) {
		exit_error(PARAMETER_PROBLEM, 
				"Can't specify HL option twice");
	}

	if (!optarg) 
		exit_error(PARAMETER_PROBLEM, 
				"HL: You must specify a value");

	if (check_inverse(optarg, &invert, NULL, 0))
		exit_error(PARAMETER_PROBLEM,
				"HL: unexpected `!'");
	
	value = atoi(optarg);

	switch (c) {

		case '1':
			info->mode = IP6T_HL_SET;
			break;

		case '2':
			if (value == 0) {
				exit_error(PARAMETER_PROBLEM,
					"HL: decreasing by 0?");
			}

			info->mode = IP6T_HL_DEC;
			break;

		case '3':
			if (value == 0) {
				exit_error(PARAMETER_PROBLEM,
					"HL: increasing by 0?");
			}

			info->mode = IP6T_HL_INC;
			break;

		default:
			return 0;

	}
	
	info->hop_limit = value;
	*flags |= IP6T_HL_USED;

	return 1;
}

static void final_check(unsigned int flags)
{
	if (!(flags & IP6T_HL_USED))
		exit_error(PARAMETER_PROBLEM,
				"HL: You must specify an action");
}

static void save(const struct ip6t_ip6 *ip,
		const struct ip6t_entry_target *target)
{
	const struct ip6t_HL_info *info = 
		(struct ip6t_HL_info *) target->data;

	switch (info->mode) {
		case IP6T_HL_SET:
			printf("--hl-set ");
			break;
		case IP6T_HL_DEC:
			printf("--hl-dec ");
			break;

		case IP6T_HL_INC:
			printf("--hl-inc ");
			break;
	}
	printf("%u ", info->hop_limit);
}

static void print(const struct ip6t_ip6 *ip,
		const struct ip6t_entry_target *target, int numeric)
{
	const struct ip6t_HL_info *info =
		(struct ip6t_HL_info *) target->data;

	printf("HL ");
	switch (info->mode) {
		case IP6T_HL_SET:
			printf("set to ");
			break;
		case IP6T_HL_DEC:
			printf("decrement by ");
			break;
		case IP6T_HL_INC:
			printf("increment by ");
			break;
	}
	printf("%u ", info->hop_limit);
}

static struct option opts[] = {
	{ "hl-set", 1, 0, '1' },
	{ "hl-dec", 1, 0, '2' },
	{ "hl-inc", 1, 0, '3' },
	{ 0 }
};

static
struct ip6tables_target HL = { NULL, 
	"HL",
	IPTABLES_VERSION,
	IP6T_ALIGN(sizeof(struct ip6t_HL_info)),
	IP6T_ALIGN(sizeof(struct ip6t_HL_info)),
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
	register_target6(&HL);
}
