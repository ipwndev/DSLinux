/*
 * Functions for opening and closing files.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#define _GNU_SOURCE 1
#include <limits.h>

#include <stdio.h>
#include "options.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * Try to work out the total size of all data by adding up the sizes of all
 * input files. If any of the input files are of indeterminate size (i.e.
 * they are a pipe), the total size is set to zero.
 *
 * Any files that cannot be stat()ed or that access() says we can't read
 * will cause a warning to be output and will be removed from the list.
 *
 * In line mode, any files that pass the above checks will then be read to
 * determine how many lines they contain, and the total size will be set to
 * the total line count. Only regular files will be read.
 */
void pv_calc_total_size(opts_t opts)
{
	struct stat64 sb;
	int rc, i, j, fd;

	opts->size = 0;
	rc = 0;

	if (opts->argc < 1) {
		if (fstat64(STDIN_FILENO, &sb) == 0)
			opts->size = sb.st_size;
		return;
	}

	for (i = 0; i < opts->argc; i++) {
		if (strcmp(opts->argv[i], "-") == 0) {
			rc = fstat64(STDIN_FILENO, &sb);
			if (rc != 0) {
				opts->size = 0;
				return;
			}
		} else {
			rc = stat64(opts->argv[i], &sb);
			if (rc == 0)
				rc = access(opts->argv[i], R_OK);
		}

		if (rc != 0) {
			fprintf(stderr, "%s: %s: %s\n", opts->program_name,
				opts->argv[i], strerror(errno));
			for (j = i; j < opts->argc - 1; j++) {
				opts->argv[j] = opts->argv[j + 1];
			}
			opts->argc--;
			i--;
			continue;
		}

		if (S_ISBLK(sb.st_mode)) {
			/*
			 * Get the size of block devices by opening
			 * them and seeking to the end.
			 */
			if (strcmp(opts->argv[i], "-") == 0) {
				fd = open64("/dev/stdin", O_RDONLY);
			} else {
				fd = open64(opts->argv[i], O_RDONLY);
			}
			if (fd >= 0) {
				opts->size += lseek64(fd, 0, SEEK_END);
				close(fd);
			}
		} else if (S_ISREG(sb.st_mode)) {
			opts->size += sb.st_size;
		} else {
			opts->size = 0;
		}
	}

	if (!opts->linemode)
		return;

	opts->size = 0;

	for (i = 0; i < opts->argc; i++) {
		fd = -1;

		if (strcmp(opts->argv[i], "-") == 0) {
			rc = fstat64(STDIN_FILENO, &sb);
			if ((rc != 0) || (!S_ISREG(sb.st_mode))) {
				opts->size = 0;
				return;
			}
			fd = dup(STDIN_FILENO);
		} else {
			rc = stat64(opts->argv[i], &sb);
			if ((rc != 0) || (!S_ISREG(sb.st_mode))) {
				opts->size = 0;
				return;
			}
			fd = open64(opts->argv[i], O_RDONLY);
		}

		if (fd < 0) {
			fprintf(stderr, "%s: %s: %s\n", opts->program_name,
				opts->argv[i], strerror(errno));
			opts->size = 0;
			return;
		}

		while (1) {
			unsigned char scanbuf[1024];	/* RATS: ignore (OK) */
			int numread, i;

			numread = read(fd, /* RATS: ignore (OK) */ scanbuf,
				       sizeof(scanbuf));
			if (numread <= 0)
				break;
			for (i = 0; i < numread; i++) {
				if (scanbuf[i] == '\n')
					opts->size++;
			}
		}

		lseek64(fd, 0, SEEK_SET);
		close(fd);
	}
}


/*
 * Close the given file descriptor and open the next one, whose number in
 * the list is "filenum", returning the new file descriptor (or negative on
 * error). It is an error if the next input file is the same as the file
 * stdout is pointing to.
 */
int pv_next_file(opts_t opts, int filenum, int oldfd)
{
	struct stat64 isb;
	struct stat64 osb;
	int fd;

	if (oldfd > 0) {
		if (close(oldfd)) {
			fprintf(stderr, "%s: %s: %s\n",
				opts->program_name,
				_("failed to close file"),
				strerror(errno));
			return -1;
		}
	}

	if (filenum >= opts->argc)
		return -1;

	if (filenum < 0)
		return -1;

	if (strcmp(opts->argv[filenum], "-") == 0) {
		fd = STDIN_FILENO;
	} else {
		fd = open64(opts->argv[filenum], O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, "%s: %s: %s: %s\n",
				opts->program_name,
				_("failed to read file"),
				opts->argv[filenum], strerror(errno));
			return -1;
		}
	}

	if (fstat64(fd, &isb)) {
		fprintf(stderr, "%s: %s: %s: %s\n",
			opts->program_name,
			_("failed to stat file"),
			opts->argv[filenum], strerror(errno));
		close(fd);
		return -1;
	}

	if (fstat64(STDOUT_FILENO, &osb)) {
		fprintf(stderr, "%s: %s: %s\n",
			opts->program_name,
			_("failed to stat output file"), strerror(errno));
		close(fd);
		return -1;
	}

	/*
	 * Check that this new input file is not the same as stdout's
	 * destination. This restriction is ignored for anything other
	 * than a regular file or block device.
	 */
	if (isb.st_dev != osb.st_dev)
		return fd;
	if (isb.st_ino != osb.st_ino)
		return fd;
	if (isatty(fd))
		return fd;
	if ((!S_ISREG(isb.st_mode)) && (!S_ISBLK(isb.st_mode)))
		return fd;

	fprintf(stderr, "%s: %s: %s\n",
		opts->program_name,
		_("input file is output file"), opts->argv[filenum]);
	close(fd);
	return -1;
}

/* EOF */
