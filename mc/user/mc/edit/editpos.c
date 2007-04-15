
/* file where positions are stored */
#define MC_EDITPOS ".mc/edit.pos"
/* temporary file */
#define MC_EDITPOS_TMP ".mc/editpos.tmp"
/* maximum entries in MC_EDITPOS */
#define MC_MAX_EDITPOS_ENTRIES 1024


/* Read and restore position the given filename */

static void
load_file_position (char *filename, long *line, long *column)
{
    char *fn;
    FILE *f;
    char buf[MC_MAXPATHLEN + 20];
    int len;

    /* defaults */
    *line = 1;
    *column = 0;

    /* open file with positions */
    fn = concat_dir_and_file (home_dir, MC_EDITPOS);
    f = fopen (fn, "r");
    free (fn);
    if (!f)
	return;

    len = strlen (filename);

    while (fgets (buf, sizeof (buf), f)) {
	char *p;

	/* check if the filename matches the beginning of string */
	if (strncmp (buf, filename, len) != 0)
	    continue;

	/* followed by single space */
	if (buf[len] != ' ')
	    continue;

	/* and string without spaces */
	p = &buf[len + 1];
	if (strchr (p, ' '))
	    continue;

	*line = atol (p);
	p = strchr (buf, ';');
	*column = atol (&p[1]);
    }
    fclose (f);
}

static void
save_file_position (char *filename, long line, long column)
{
    char *tmp, *fn;
    FILE *f, *t;
    char buf[MC_MAXPATHLEN + 20];
    int i = 1;
    int len;

    len = strlen (filename);

    tmp = concat_dir_and_file (home_dir, MC_EDITPOS_TMP);
    fn = concat_dir_and_file (home_dir, MC_EDITPOS);

    /* open temporary file */
    t = fopen (tmp, "w");
    if (!t) {
	free (tmp);
	free (fn);
	return;
    }

    /* put the new record */
    fprintf (t, "%s %ld;%ld\n", filename, line, column);

    /* copy records from the old file */
    f = fopen (fn, "r");
    if (f) {
	while (fgets (buf, sizeof (buf), f)) {
	    /* Skip entries for the current filename */
	    if (strncmp (buf, filename, len) == 0 && buf[len] == ' '
		&& !strchr (&buf[len + 1], ' '))
		continue;

	    fprintf (t, "%s", buf);
	    if (++i > MC_MAX_EDITPOS_ENTRIES)
		break;
	}
	fclose (f);
    }

    fclose (t);
    rename (tmp, fn);
    free (tmp);
    free (fn);
}
