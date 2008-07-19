/* 
   Shared library add-on to iptables to add match support for random match.
   
   This file is distributed under the terms of the GNU General Public
   License (GPL). Copies of the GPL can be obtained from:
   ftp://prep.ai.mit.edu/pub/gnu/GPL

   2001-10-14 Fabrice MARIE <fabrice@netfilter.org> : initial development.
   2003-04-30 Maciej Soltysiak <solt@dns.toxicfilms.tv> : IPv6 port.
*/

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <getopt.h>
#include <ip6tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <linux/netfilter_ipv6/ip6t_random.h>

/**
 * The kernel random routing returns numbers between 0 and 255.
 * To ease the task of the user in choosing the probability
 * of matching, we want him to be able to use percentages.
 * Therefore we have to accept numbers in percentage here,
 * turn them into number between 0 and 255 for the kernel module,
 * and turn them back to percentages when we print/save
 * the rule.
 */


/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"random v%s options:\n"
"  [--average]     percent      The probability in percentage of the match\n"
"                               If ommited, a probability of 50%% percent is set.\n"
"                               Percentage must be within : 1 <= percent <= 99.\n\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "average", 1, 0, '1' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ip6t_entry_match *m, unsigned int *nfcache)
{
	struct ip6t_rand_info *randinfo = (struct ip6t_rand_info *)(m)->data;
	*nfcache |= NFC_UNKNOWN;

	/* We assign the average to be 50 which is our default value */
	/* 50 * 2.55 = 128 */
	randinfo->average = 128;
}

#define IP6T_RAND_OPT_AVERAGE	0x01

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ip6t_entry *entry,
      unsigned int *nfcache,
      struct ip6t_entry_match **match)
{
	struct ip6t_rand_info *randinfo = (struct ip6t_rand_info *)(*match)->data;
	unsigned int num;

	switch (c) {
	case '1':
		/* check for common mistakes... */
		if (invert)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify ! --average");
		if (*flags & IP6T_RAND_OPT_AVERAGE)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify --average twice");

		/* Remember, this function will interpret a leading 0 to be 
		   Octal, a leading 0x to be hexdecimal... */
                if (string_to_number(optarg, 1, 99, &num) == -1 || num < 1)
                        exit_error(PARAMETER_PROBLEM,
                                   "bad --average `%s', must be between 1 and 99", optarg);

		/* assign the values */
		randinfo->average = (int)(num * 2.55);
		*flags |= IP6T_RAND_OPT_AVERAGE;
		break;
	default:
		return 0;
	}
	return 1;
}

/* Final check; nothing. */
static void final_check(unsigned int flags)
{
}

/* Prints out the targinfo. */
static void
print(const struct ip6t_ip6 *ip,
      const struct ip6t_entry_match *match,
      int numeric)
{
	const struct ip6t_rand_info *randinfo
		= (const struct ip6t_rand_info *)match->data;
	div_t result = div((randinfo->average*100), 255);
	if (result.rem > 127)  /* round up... */
		++result.quot;

	printf(" random %u%% ", result.quot);
}

/* Saves the union ip6t_targinfo in parsable form to stdout. */
static void
save(const struct ip6t_ip6 *ip, const struct ip6t_entry_match *match)
{
	const struct ip6t_rand_info *randinfo
		= (const struct ip6t_rand_info *)match->data;
	div_t result = div((randinfo->average *100), 255);
	if (result.rem > 127)  /* round up... */
		++result.quot;

	printf("--average %u ", result.quot);
}

struct ip6tables_match rand_match
= { NULL,
    "random",
    IPTABLES_VERSION,
    IP6T_ALIGN(sizeof(struct ip6t_rand_info)),
    IP6T_ALIGN(sizeof(struct ip6t_rand_info)),
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
	register_match6(&rand_match);
}
