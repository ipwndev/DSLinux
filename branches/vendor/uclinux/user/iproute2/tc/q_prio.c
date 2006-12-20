/*
 * q_prio.c		PRIO.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 * Changes:
 *
 * Ole Husgaard <sparre@login.dknet.dk>: 990513: prio2band map was always reset.
 * J Hadi Salim <hadi@cyberus.ca>: 990609: priomap fix.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr, "Usage: ... prio bands NUMBER priomap P1 P2...\n");
}

#define usage() return(-1)

static int prio_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int ok=0;
	int pmap_mode = 0;
	int idx = 0;
	struct tc_prio_qopt opt={3,{ 1, 2, 2, 2, 1, 2, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 }};

	while (argc > 0) {
		if (strcmp(*argv, "bands") == 0) {
			if (pmap_mode)
				explain();
			NEXT_ARG();
			if (get_integer(&opt.bands, *argv, 10)) {
				fprintf(stderr, "Illegal \"bands\"\n");
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "priomap") == 0) {
			if (pmap_mode) {
				fprintf(stderr, "Error: duplicate priomap\n");
				return -1;
			}
			pmap_mode = 1;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			unsigned band;
			if (!pmap_mode) {
				fprintf(stderr, "What is \"%s\"?\n", *argv);
				explain();
				return -1;
			}
			if (get_unsigned(&band, *argv, 10)) {
				fprintf(stderr, "Illegal \"priomap\" element\n");
				return -1;
			}
			if (band > opt.bands) {
				fprintf(stderr, "\"priomap\" element is out of bands\n");
				return -1;
			}
			if (idx > TC_PRIO_MAX) {
				fprintf(stderr, "\"priomap\" index > TC_PRIO_MAX=%u\n", TC_PRIO_MAX);
				return -1;
			}
			opt.priomap[idx++] = band;
		}
		argc--; argv++;
	}

/*
	if (pmap_mode) {
		for (; idx < TC_PRIO_MAX; idx++)
			opt.priomap[idx] = opt.priomap[TC_PRIO_BESTEFFORT];
	}
*/
	addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
	return 0;
}

static int prio_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	int i;
	struct tc_prio_qopt *qopt;

	if (opt == NULL)
		return 0;

	if (RTA_PAYLOAD(opt)  < sizeof(*qopt))
		return -1;
	qopt = RTA_DATA(opt);
	fprintf(f, "bands %u priomap ", qopt->bands);
	for (i=0; i<=TC_PRIO_MAX; i++)
		fprintf(f, " %d", qopt->priomap[i]);
	return 0;
}

static int prio_print_xstats(struct qdisc_util *qu, FILE *f, struct rtattr *xstats)
{
	return 0;
}


struct qdisc_util prio_util = {
	NULL,
	"prio",
	prio_parse_opt,
	prio_print_opt,
	prio_print_xstats,
};

