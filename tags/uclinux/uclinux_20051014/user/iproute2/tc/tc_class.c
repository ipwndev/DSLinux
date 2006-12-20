/*
 * tc_class.c		"tc class".
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
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
#include <math.h>

#include "utils.h"
#include "tc_util.h"
#include "tc_common.h"

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, "Usage: tc class [ add | del | change | get ] dev STRING\n");
	fprintf(stderr, "       [ classid CLASSID ] [ root | parent CLASSID ]\n");
	fprintf(stderr, "       [ [ QDISC_KIND ] [ help | OPTIONS ] ]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "       tc class show [ dev STRING ] [ root | parent CLASSID ]\n");
	fprintf(stderr, "Where:\n");
	fprintf(stderr, "QDISC_KIND := { prio | cbq | etc. }\n");
	fprintf(stderr, "OPTIONS := ... try tc class add <desired QDISC_KIND> help\n");
	exit(-1);
}

int tc_class_modify(int cmd, unsigned flags, int argc, char **argv)
{
	struct rtnl_handle rth;
	static struct {
		struct nlmsghdr 	n;
		struct tcmsg 		t;
		char   			buf[4096];
	} req;
	struct qdisc_util *q = NULL;
	struct tc_estimator est;
	char  d[16];
	char  k[16];

	memset(&req, 0, sizeof(req));
	memset(&est, 0, sizeof(est));
	memset(d, 0, sizeof(d));
	memset(k, 0, sizeof(k));

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct tcmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST|flags;
	req.n.nlmsg_type = cmd;
	req.t.tcm_family = AF_UNSPEC;

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (d[0])
				duparg("dev", *argv);
			strncpy(d, *argv, sizeof(d)-1);
		} else if (strcmp(*argv, "classid") == 0) {
			__u32 handle;
			NEXT_ARG();
			if (req.t.tcm_handle)
				duparg("classid", *argv);
			if (get_tc_classid(&handle, *argv))
				invarg(*argv, "invalid class ID");
			req.t.tcm_handle = handle;
		} else if (strcmp(*argv, "root") == 0) {
			if (req.t.tcm_parent) {
				fprintf(stderr, "Error: \"root\" is duplicate parent ID.\n");
				exit(-1);
			}
			req.t.tcm_parent = TC_H_ROOT;
		} else if (strcmp(*argv, "parent") == 0) {
			__u32 handle;
			NEXT_ARG();
			if (req.t.tcm_parent)
				duparg("parent", *argv);
			if (get_tc_classid(&handle, *argv))
				invarg(*argv, "invalid parent ID");
			req.t.tcm_parent = handle;
		} else if (matches(*argv, "estimator") == 0) {
			if (parse_estimator(&argc, &argv, &est))
				return -1;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			strncpy(k, *argv, sizeof(k)-1);

			q = get_qdisc_kind(k);
			argc--; argv++;
			break;
		}
		argc--; argv++;
	}

	if (k[0])
		addattr_l(&req.n, sizeof(req), TCA_KIND, k, strlen(k)+1);
	if (est.ewma_log)
		addattr_l(&req.n, sizeof(req), TCA_RATE, &est, sizeof(est));

	if (q) {
		if (q->parse_copt == NULL) {
			fprintf(stderr, "Error: Qdisc \"%s\" is classless.\n", k);
			exit(1);
		}
		if (q->parse_copt(q, argc, argv, &req.n))
			exit(1);
	} else {
		if (argc) {
			if (matches(*argv, "help") == 0)
				usage();
			fprintf(stderr, "Garbage instead of arguments \"%s ...\". Try \"tc class help\".", *argv);
			exit(-1);
		}
	}

	if (rtnl_open(&rth, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		exit(1);
	}

	if (d[0])  {
		ll_init_map(&rth);

		if ((req.t.tcm_ifindex = ll_name_to_index(d)) == 0) {
			fprintf(stderr, "Cannot find device \"%s\"\n", d);
			exit(1);
		}
	}

	if (rtnl_talk(&rth, &req.n, 0, 0, NULL, NULL, NULL) < 0)
		exit(2);

	rtnl_close(&rth);
	return 0;
}

void print_class_tcstats(FILE *fp, struct tc_stats *st)
{
	SPRINT_BUF(b1);

	fprintf(fp, " Sent %llu bytes %u pkts (dropped %u, overlimits %u) ",
		(unsigned long long)st->bytes, st->packets, st->drops, st->overlimits);
	if (st->bps || st->pps || st->qlen || st->backlog) {
		fprintf(fp, "\n ");
		if (st->bps || st->pps) {
			fprintf(fp, "rate ");
			if (st->bps)
				fprintf(fp, "%s ", sprint_rate(st->bps, b1));
			if (st->pps)
				fprintf(fp, "%upps ", st->pps);
		}
		if (st->qlen || st->backlog) {
			fprintf(fp, "backlog ");
			if (st->backlog)
				fprintf(fp, "%s ", sprint_size(st->backlog, b1));
			if (st->qlen)
				fprintf(fp, "%up ", st->qlen);
		}
	}
}

int filter_ifindex;
__u32 filter_qdisc;

int print_class(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE*)arg;
	struct tcmsg *t = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	struct rtattr * tb[TCA_MAX+1];
	struct qdisc_util *q;
	static char abuf[256];

	if (n->nlmsg_type != RTM_NEWTCLASS && n->nlmsg_type != RTM_DELTCLASS) {
		fprintf(stderr, "Not a class\n");
		return 0;
	}
	len -= NLMSG_LENGTH(sizeof(*t));
	if (len < 0) {
		fprintf(stderr, "Wrong len %d\n", len);
		return -1;
	}
	if (filter_qdisc && TC_H_MAJ(t->tcm_handle^filter_qdisc))
		return 0;

	memset(tb, 0, sizeof(tb));
	parse_rtattr(tb, TCA_MAX, TCA_RTA(t), len);

	if (tb[TCA_KIND] == NULL) {
		fprintf(stderr, "NULL kind\n");
		return -1;
	}

	if (n->nlmsg_type == RTM_DELTCLASS)
		fprintf(fp, "deleted ");

	abuf[0] = 0;
	if (t->tcm_handle) {
		if (filter_qdisc)
			print_tc_classid(abuf, sizeof(abuf), TC_H_MIN(t->tcm_handle));
		else
			print_tc_classid(abuf, sizeof(abuf), t->tcm_handle);
	}
	fprintf(fp, "class %s %s ", (char*)RTA_DATA(tb[TCA_KIND]), abuf);

	if (filter_ifindex == 0)
		fprintf(fp, "dev %s ", ll_index_to_name(t->tcm_ifindex));

	if (t->tcm_parent == TC_H_ROOT)
		fprintf(fp, "root ");
	else {
		if (filter_qdisc)
			print_tc_classid(abuf, sizeof(abuf), TC_H_MIN(t->tcm_parent));
		else
			print_tc_classid(abuf, sizeof(abuf), t->tcm_parent);
		fprintf(fp, "parent %s ", abuf);
	}
	if (t->tcm_info)
		fprintf(fp, "leaf %x: ", t->tcm_info>>16);
	q = get_qdisc_kind(RTA_DATA(tb[TCA_KIND]));
	if (q && q->print_copt)
		q->print_copt(q, fp, tb[TCA_OPTIONS]);
	else
		fprintf(fp, "[UNKNOWN]");
	fprintf(fp, "\n");
	if (show_stats) {
		if (tb[TCA_STATS]) {
			if (RTA_PAYLOAD(tb[TCA_STATS]) < sizeof(struct tc_stats))
				fprintf(fp, "statistics truncated");
			else {
				struct tc_stats st;
				memcpy(&st, RTA_DATA(tb[TCA_STATS]), sizeof(st));
				print_class_tcstats(fp, &st);
				fprintf(fp, "\n");
			}
		}
		if (q && tb[TCA_XSTATS]) {
			q->print_xstats(q, fp, tb[TCA_XSTATS]);
			fprintf(fp, "\n");
		}
	}
	fflush(fp);
	return 0;
}


int tc_class_list(int argc, char **argv)
{
	struct tcmsg t;
	struct rtnl_handle rth;
	char d[16];

	memset(&t, 0, sizeof(t));
	t.tcm_family = AF_UNSPEC;
	memset(d, 0, sizeof(d));

	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			NEXT_ARG();
			if (d[0])
				duparg("dev", *argv);
			strncpy(d, *argv, sizeof(d)-1);
		} else if (strcmp(*argv, "qdisc") == 0) {
			NEXT_ARG();
			if (filter_qdisc)
				duparg("qdisc", *argv);
			if (get_qdisc_handle(&filter_qdisc, *argv))
				invarg(*argv, "invalid qdisc ID");
		} else if (strcmp(*argv, "root") == 0) {
			if (t.tcm_parent) {
				fprintf(stderr, "Error: \"root\" is duplicate parent ID\n");
				exit(-1);
			}
			t.tcm_parent = TC_H_ROOT;
		} else if (strcmp(*argv, "parent") == 0) {
			__u32 handle;
			if (t.tcm_parent)
				duparg("parent", *argv);
			NEXT_ARG();
			if (get_tc_classid(&handle, *argv))
				invarg(*argv, "invalid parent ID");
			t.tcm_parent = handle;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			fprintf(stderr, "What is \"%s\"? Try \"tc class help\".\n", *argv);
			exit(-1);
		}

		argc--; argv++;
	}

	if (rtnl_open(&rth, 0) < 0) {
		fprintf(stderr, "Cannot open rtnetlink\n");
		exit(1);
	}

	ll_init_map(&rth);

	if (d[0]) {
		if ((t.tcm_ifindex = ll_name_to_index(d)) == 0) {
			fprintf(stderr, "Cannot find device \"%s\"\n", d);
			exit(1);
		}
		filter_ifindex = t.tcm_ifindex;
	}

	if (rtnl_dump_request(&rth, RTM_GETTCLASS, &t, sizeof(t)) < 0) {
		perror("Cannot send dump request");
		exit(1);
	}

	if (rtnl_dump_filter(&rth, print_class, stdout, NULL, NULL) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}

	rtnl_close(&rth);
	return 0;
}

int do_class(int argc, char **argv)
{
	if (argc < 1)
		return tc_class_list(0, NULL);
	if (matches(*argv, "add") == 0)
		return tc_class_modify(RTM_NEWTCLASS, NLM_F_EXCL|NLM_F_CREATE, argc-1, argv+1);
	if (matches(*argv, "change") == 0)
		return tc_class_modify(RTM_NEWTCLASS, 0, argc-1, argv+1);
	if (matches(*argv, "replace") == 0)
		return tc_class_modify(RTM_NEWTCLASS, NLM_F_CREATE, argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return tc_class_modify(RTM_DELTCLASS, 0,  argc-1, argv+1);
#if 0
	if (matches(*argv, "get") == 0)
		return tc_class_get(RTM_GETTCLASS, 0,  argc-1, argv+1);
#endif
	if (matches(*argv, "list") == 0 || matches(*argv, "show") == 0
	    || matches(*argv, "lst") == 0)
		return tc_class_list(argc-1, argv+1);
	if (matches(*argv, "help") == 0)
		usage();
	fprintf(stderr, "Command \"%s\" is unknown, try \"tc class help\".\n", *argv);
	return -1;
}
