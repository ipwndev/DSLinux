/*
 * Output command-line help to stdout.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct optdesc_s {
	char *optshort;
	char *optlong;
	char *param;
	char *description;
};


/*
 * Display command-line help.
 */
void display_help(void)
{
	struct optdesc_s optlist[] = {
		{"-p", "--progress", 0,
		 _("show progress bar")},
		{"-t", "--timer", 0,
		 _("show elapsed time")},
		{"-e", "--eta", 0,
		 _("show estimated time of arrival (completion)")},
		{"-r", "--rate", 0,
		 _("show data transfer rate counter")},
		{"-b", "--bytes", 0,
		 _("show number of bytes transferred")},
		{"-f", "--force", 0,
		 _("output even if standard error is not a terminal")},
		{"-n", "--numeric", 0,
		 _("output percentages, not visual information")},
		{"-q", "--quiet", 0,
		 _("do not output any transfer information at all")},
		{"-c", "--cursor", 0,
		 _("use cursor positioning escape sequences")},
		{"-W", "--wait", 0,
		 _("display nothing until first byte transferred")},
		{"-s", "--size", _("SIZE"),
		 _("set estimated data size to SIZE bytes")},
		{"-l", "--line-mode", 0,
		 _("count lines instead of bytes")},
		{"-i", "--interval", _("SEC"),
		 _("update every SEC seconds")},
		{"-w", "--width", _("WIDTH"),
		 _("assume terminal is WIDTH characters wide")},
		{"-H", "--height", _("HEIGHT"),
		 _("assume terminal is HEIGHT rows high")},
		{"-N", "--name", _("NAME"),
		 _("prefix visual information with NAME")},
		{"", 0, 0, 0},
		{"-L", "--rate-limit", _("RATE"),
		 _("limit transfer to RATE bytes per second")},
		{"-B", "--buffer-size", _("BYTES"),
		 _("use a buffer size of BYTES")},
		{"-R", "--remote", _("PID"),
		 _("update settings of process PID")},
		{"", 0, 0, 0},
		{"-h", "--help", 0,
		 _("show this help and exit")},
		{"-V", "--version", 0,
		 _("show version information and exit")},
		{0, 0, 0, 0}
	};
	int i, col1max = 0, tw = 77;
	char *optbuf;

	printf(_("Usage: %s [OPTION] [FILE]..."),	/* RATS: ignore */
	       PROGRAM_NAME);
	printf("\n%s\n\n",
	       _
	       ("Concatenate FILE(s), or standard input, to standard output,\n"
		"with monitoring."));

	for (i = 0; optlist[i].optshort; i++) {
		int width = 0;

		width = 2 + strlen(optlist[i].optshort);	/* RATS: ignore */
#ifdef HAVE_GETOPT_LONG
		if (optlist[i].optlong)
			width += 2 + strlen(optlist[i].optlong);	/* RATS: ignore */
#endif
		if (optlist[i].param)
			width += 1 + strlen(optlist[i].param);	/* RATS: ignore */

		if (width > col1max)
			col1max = width;
	}

	col1max++;

	optbuf = malloc(col1max + 16);
	if (optbuf == NULL) {
		fprintf(stderr, "%s: %s\n", PROGRAM_NAME, strerror(errno));
		exit(1);
	}

	for (i = 0; optlist[i].optshort; i++) {
		char *start;
		char *end;

		if (optlist[i].optshort[0] == 0) {
			printf("\n");
			continue;
		}

		sprintf(optbuf, "%s%s%s%s%s",	/* RATS: ignore (checked) */
			optlist[i].optshort,
#ifdef HAVE_GETOPT_LONG
			optlist[i].optlong ? ", " : "",
			optlist[i].optlong ? optlist[i].optlong : "",
#else
			"", "",
#endif
			optlist[i].param ? " " : "",
			optlist[i].param ? optlist[i].param : "");

		printf("  %-*s ", col1max - 2, optbuf);

		if (optlist[i].description == NULL) {
			printf("\n");
			continue;
		}

		start = optlist[i].description;

		while (strlen(start) /* RATS: ignore */ >tw - col1max) {
			end = start + tw - col1max;
			while ((end > start) && (end[0] != ' '))
				end--;
			if (end == start) {
				end = start + tw - col1max;
			} else {
				end++;
			}
			printf("%.*s\n%*s ", (int) (end - start), start,
			       col1max, "");
			if (end == start)
				end++;
			start = end;
		}

		printf("%s\n", start);
	}

	printf("\n");
	printf(_("Please report any bugs to %s."),	/* RATS: ignore */
	       BUG_REPORTS_TO);
	printf("\n");
}

/* EOF */
