/*
 * tc.c		"tc" utility frontend.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 * Fixes:
 *
 * Petri Mattila <petri@prihateam.fi> 990308: wrong memset's resulted in faults
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#ifndef NO_DL
#include <dlfcn.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include "SNAPSHOT.h"
#include "utils.h"
#include "tc_util.h"
#include "tc_common.h"

int show_stats = 0;
int show_details = 0;
int show_raw = 0;
int resolve_hosts = 0;

void *BODY;
static struct qdisc_util * qdisc_list;
static struct filter_util * filter_list;

static int print_noqopt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	if (opt && RTA_PAYLOAD(opt))
		fprintf(f, "[Unknown qdisc, optlen=%u] ", RTA_PAYLOAD(opt));
	return 0;
}

static int parse_noqopt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	if (argc) {
		fprintf(stderr, "Unknown qdisc \"%s\", hence option \"%s\" is unparsable\n", qu->id, *argv);
		return -1;
	}
	return 0;
}

static int print_nofopt(struct filter_util *qu, FILE *f, struct rtattr *opt, __u32 fhandle)
{
	if (opt && RTA_PAYLOAD(opt))
		fprintf(f, "fh %08x [Unknown filter, optlen=%u] ", fhandle, RTA_PAYLOAD(opt));
	else if (fhandle)
		fprintf(f, "fh %08x ", fhandle);
	return 0;
}

static int parse_nofopt(struct filter_util *qu, char *fhandle, int argc, char **argv, struct nlmsghdr *n)
{
	__u32 handle;

	if (argc) {
		fprintf(stderr, "Unknown filter \"%s\", hence option \"%s\" is unparsable\n", qu->id, *argv);
		return -1;
	}
	if (fhandle) {
		struct tcmsg *t = NLMSG_DATA(n);
		if (get_u32(&handle, fhandle, 16)) {
			fprintf(stderr, "Unparsable filter ID \"%s\"\n", fhandle);
			return -1;
		}
		t->tcm_handle = handle;
	}
	return 0;
}

#if 0
/* Builtin filter types */

static int f_parse_noopt(struct filter_util *qu, char *fhandle, int argc, char **argv, struct nlmsghdr *n)
{
	if (argc || fhandle) {
		fprintf(stderr, "Filter \"%s\" has no options.\n", qu->id);
		return -1;
	}
	return 0;
}
#endif

#ifdef NO_DL
#ifdef TC_CONFIG_ATM
extern struct qdisc_util atm_util;
#endif
#ifdef TC_CONFIG_DIFFSERV
extern struct qdisc_util dsmark_util;
extern struct qdisc_util gred_util;
extern struct qdisc_util ingress_util;
#endif
extern struct qdisc_util cbq_util;
extern struct qdisc_util htb_util;
extern struct qdisc_util pfifo_util;
extern struct qdisc_util bfifo_util;
#if 0
extern struct qdisc_util csz_util;
extern struct qdisc_util hfsc_util;
extern struct qdisc_util hpfq_util;
#endif
extern struct qdisc_util prio_util;
extern struct qdisc_util red_util;
extern struct qdisc_util wrr_util;
extern struct qdisc_util sfq_util;
extern struct qdisc_util tbf_util;
static struct qdisc_util *qdisc[] = {
#ifdef TC_CONFIG_ATM
	&atm_util,
#endif
#ifdef TC_CONFIG_DIFFSERV
	&dsmark_util,
	&gred_util,
	&ingress_util,
#endif
	&cbq_util,
	&htb_util,
	&pfifo_util,
	&bfifo_util,
#if 0
	&csz_util,
	&hfsc_util,
	&hpfq_util,
#endif
	&prio_util,
	&red_util,
	&wrr_util,
	&sfq_util,
	&tbf_util,
	NULL
};
#endif

struct qdisc_util *get_qdisc_kind(char *str)
{
	void *dlh;
	char buf[256];
	struct qdisc_util *q;

	for (q = qdisc_list; q; q = q->next)
		if (strcmp(q->id, str) == 0)
			return q;

#ifdef NO_DL
{
	int i;
	for (i = 0; qdisc[i]; i++)
		if (strcmp(qdisc[i]->id, str) == 0) {
			q = qdisc[i];
			goto reg;
		}
	goto noexist;
		
}
#else
	snprintf(buf, sizeof(buf), "q_%s.so", str);
	dlh = dlopen(buf, RTLD_LAZY);
	if (dlh == NULL) {
		dlh = BODY;
		if (dlh == NULL) {
			dlh = BODY = dlopen(NULL, RTLD_LAZY);
			if (dlh == NULL)
				goto noexist;
		}
	}

	snprintf(buf, sizeof(buf), "%s_util", str);
	q = dlsym(dlh, buf);
	if (q == NULL)
		goto noexist;
#endif

reg:
	q->next = qdisc_list;
	qdisc_list = q;
	return q;

noexist:
	q = malloc(sizeof(*q));
	if (q) {
		memset(q, 0, sizeof(*q));
		strncpy(q->id, str, 15);
		q->parse_qopt = parse_noqopt;
		q->print_qopt = print_noqopt;
		goto reg;
	}
	return q;
}


#ifdef NO_DL
extern struct filter_util fw_util;
extern struct filter_util route_util;
extern struct filter_util rsvp_util;
#ifdef TC_CONFIG_DIFFSERV
extern struct filter_util tcindex_util;
#endif
extern struct filter_util u32_util;
static struct filter_util *filter[] = {
	&fw_util,
	&route_util,
	&rsvp_util,
#ifdef TC_CONFIG_DIFFSERV
	&tcindex_util,
#endif
	&u32_util,
	NULL
};
#endif

struct filter_util *get_filter_kind(char *str)
{
	void *dlh;
	char buf[256];
	struct filter_util *q;

	for (q = filter_list; q; q = q->next)
		if (strcmp(q->id, str) == 0)
			return q;

#ifdef NO_DL
{
	int i;
	for (i = 0; filter[i]; i++)
		if (strcmp(filter[i]->id, str) == 0) {
			q = filter[i];
			goto reg;
		}
	goto noexist;
		
}
#else
	snprintf(buf, sizeof(buf), "f_%s.so", str);
	dlh = dlopen(buf, RTLD_LAZY);
	if (dlh == NULL) {
		dlh = BODY;
		if (dlh == NULL) {
			dlh = BODY = dlopen(NULL, RTLD_LAZY);
			if (dlh == NULL)
				goto noexist;
		}
	}

	snprintf(buf, sizeof(buf), "%s_util", str);
	q = dlsym(dlh, buf);
	if (q == NULL)
		goto noexist;
#endif

reg:
	q->next = filter_list;
	filter_list = q;
	return q;

noexist:
	q = malloc(sizeof(*q));
	if (q) {
		memset(q, 0, sizeof(*q));
		strncpy(q->id, str, 15);
		q->parse_fopt = parse_nofopt;
		q->print_fopt = print_nofopt;
		goto reg;
	}
	return q;
}

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, "Usage: tc [ OPTIONS ] OBJECT { COMMAND | help }\n"
	                "where  OBJECT := { qdisc | class | filter }\n"
	                "       OPTIONS := { -s[tatistics] | -d[etails] | -r[aw] | -b[atch] file }\n");
	exit(-1);
}



int main(int argc, char **argv)
{
	char *basename;

	basename = strrchr(argv[0], '/');
	if (basename == NULL)
		basename = argv[0];
	else
		basename++;
	

	/* batch mode */
	if (argc > 1 && matches(argv[1], "-batch") == 0) {
		FILE *batch;
		char line[400];
		char *largv[100];
		int largc, ret=0;
#define	BMAXARG	(sizeof(largv)/sizeof(char *)-2)

		if (argc != 3) {
			fprintf(stderr, "Wrong number of arguments in batch mode\n");
			exit(-1);
		}
		if (matches(argv[2], "-") != 0) {
			if ((batch = fopen(argv[2], "r")) == NULL) {
				fprintf(stderr, "Cannot open file \"%s\" for reading: %s=n", argv[2], strerror(errno));
				exit(-1);
			}
		} else {
			if ((batch = fdopen(0, "r")) == NULL) {
				fprintf(stderr, "Cannot open stdin for reading: %s=n", strerror(errno));
				exit(-1);
			}
		}

		tc_core_init();

		while (fgets(line, sizeof(line)-1, batch)) {
			if (line[strlen(line)-1]=='\n') {
				line[strlen(line)-1] = '\0';
			} else {
				fprintf(stderr, "No newline at the end of line, looks like to long (%d chars or more)\n", strlen(line));
				exit(-1);
			}
			largc = 0;
			largv[largc]=strtok(line, " ");
			while ((largv[++largc]=strtok(NULL, " ")) != NULL) {
				if (largc > BMAXARG) {
					fprintf(stderr, "Over %d arguments in batch mode, enough!\n", BMAXARG);
					exit(-1);
				}
			}

			if (matches(largv[0], "qdisc") == 0) {
				ret += do_qdisc(largc-1, largv+1);
			} else if (matches(largv[0], "class") == 0) {
				ret += do_class(largc-1, largv+1);
			} else if (matches(largv[0], "filter") == 0) {
				ret += do_filter(largc-1, largv+1);
			} else if (matches(largv[0], "help") == 0) {
				usage();	/* note that usage() doesn't return */
			} else {
				fprintf(stderr, "Object \"%s\" is unknown, try \"tc help\".\n", largv[1]);
				exit(-1);
			}
		}
		fclose(batch);
		exit(0); /* end of batch, that's all */
	}

	while (argc > 1) {
		if (argv[1][0] != '-')
			break;
		if (matches(argv[1], "-stats") == 0 ||
		    matches(argv[1], "-statistics") == 0) {
			++show_stats;
		} else if (matches(argv[1], "-details") == 0) {
			++show_details;
		} else if (matches(argv[1], "-raw") == 0) {
			++show_raw;
		} else if (matches(argv[1], "-Version") == 0) {
			printf("tc utility, iproute2-ss%s\n", SNAPSHOT);
			exit(0);
		} else if (matches(argv[1], "-help") == 0) {
			usage();
		} else {
			fprintf(stderr, "Option \"%s\" is unknown, try \"tc -help\".\n", argv[1]);
			exit(-1);
		}
		argc--;	argv++;
	}

	tc_core_init();

	if (argc > 1) {
		if (matches(argv[1], "qdisc") == 0)
			return do_qdisc(argc-2, argv+2);
		if (matches(argv[1], "class") == 0)
			return do_class(argc-2, argv+2);
		if (matches(argv[1], "filter") == 0)
			return do_filter(argc-2, argv+2);
		if (matches(argv[1], "help") == 0)
			usage();
		fprintf(stderr, "Object \"%s\" is unknown, try \"tc help\".\n", argv[1]);
		exit(-1);
	}

	usage();
}
