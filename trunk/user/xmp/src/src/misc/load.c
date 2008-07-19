/* Extended Module Player
 * Copyright (C) 1996-2007 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * $Id: load.c,v 1.69 2007/12/06 20:22:56 cmatsuoka Exp $
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU General Public License. See doc/COPYING
 * for more information.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMX__
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include "driver.h"
#include "convert.h"
#include "loader.h"

#include "list.h"
#include "../prowizard/prowiz.h"

extern struct list_head loader_list;
extern struct list_head *checked_format;

char *global_filename;

int decrunch_arc	(FILE *, FILE *);
int decrunch_arcfs	(FILE *, FILE *);
int decrunch_sqsh	(FILE *, FILE *);
int decrunch_pp		(FILE *, FILE *);
int decrunch_mmcmp	(FILE *, FILE *);
int decrunch_oxm	(FILE *, FILE *);
int decrunch_xfd	(FILE *, FILE *);
int decrunch_pw		(FILE *, FILE *);
int test_oxm		(FILE *);
char *test_xfd		(unsigned char *, int);
int pw_check		(unsigned char *, int);

#define BUILTIN_PP	0x01
#define BUILTIN_SQSH	0x02
#define BUILTIN_MMCMP	0x03
#define BUILTIN_PW	0x04
#define BUILTIN_ARC	0x05
#define BUILTIN_ARCFS	0x06
#define BUILTIN_S404	0x07
#define BUILTIN_OXM	0x08
#define BUILTIN_XFD	0x09


#define TMP_SIZE 512

#if defined __EMX__ || defined WIN32
#define REDIR_STDERR "2>NUL"
#elif defined unix
#define REDIR_STDERR "2>/dev/null"
#else
#define REDIR_STDERR
#endif

static int decrunch(struct xmp_context *ctx, FILE **f, char **s)
{
    struct xmp_options *o = &ctx->o;
    unsigned char *b;
    char *cmd;
    FILE *t;
    int fd, builtin, res;
    char *packer, *temp, *temp2, tmp[TMP_SIZE];

    packer = cmd = NULL;
    builtin = res = 0;
    get_temp_dir(tmp, TMP_SIZE);
    strncat(tmp, "xmp_XXXXXX", TMP_SIZE);
    temp = strdup(tmp);

    fseek(*f, 0, SEEK_SET);
    b = calloc(1, PW_TEST_CHUNK);
    fread(b, 1, PW_TEST_CHUNK, *f);

#if defined __AMIGA__ && !defined __AROS__
    if (packer = test_xfd(b,PW_TEST_CHUNK)) {
	builtin = BUILTIN_XFD;
    } else
#endif
    if (b[0] == 'P' && b[1] == 'K') {
	packer = "Zip";
	cmd = "unzip -pqqC \"%s\" -x readme '*.diz' '*.nfo' '*.txt' "
		"'*.exe' '*.com' " REDIR_STDERR;
    } else if (b[2] == '-' && b[3] == 'l' && b[4] == 'h') {
	packer = "LHa";
#if defined __EMX__
	fprintf( stderr, "LHA for OS/2 does NOT support output to stdout.\n" );
#elif defined __AMIGA__
	cmd = "lha p -q \"%s\"";
#endif
	cmd = "lha -pq \"%s\"";
    } else if (b[0] == 31 && b[1] == 139) {
	packer = "gzip";
	cmd = "gzip -dc \"%s\"";
    } else if (b[0] == 'B' && b[1] == 'Z' && b[2] == 'h') {
	packer = "bzip2";
	cmd = "bzip2 -dc \"%s\"";
    } else if (b[0] == 0x5d && b[1] == 0 && b[2] == 0 && b[3] == 0x80) {
	packer = "lzma";
	cmd = "lzma -dc \"%s\"";
    } else if (b[0] == 'Z' && b[1] == 'O' && b[2] == 'O' && b[3] == ' ') {
	packer = "zoo";
	cmd = "zoo xpq \"%s\"";
    } else if (b[0] == 'M' && b[1] == 'O' && b[2] == '3') {
	packer = "MO3";
	cmd = "unmo3 -s \"%s\" STDOUT";
    } else if (b[0] == 31 && b[1] == 157) {
	packer = "compress";
#ifdef __EMX__
	fprintf( stderr, "I was unable to find a OS/2 version of UnCompress...\n" );
	fprintf( stderr, "I hope that the default command will work if a OS/2 version is found/created!\n" );
#endif
	cmd = "uncompress -c \"%s\"";
    } else if (b[0] == 'P' && b[1] == 'P' && b[2] == '2' && b[3] == '0') {
	packer = "PowerPack";
	builtin = BUILTIN_PP;
    } else if (b[0] == 'X' && b[1] == 'P' && b[2] == 'K' && b[3] == 'F' &&
	b[8] == 'S' && b[9] == 'Q' && b[10] == 'S' && b[11] == 'H') {
	packer = "SQSH";
	builtin = BUILTIN_SQSH;
    } else if (!memcmp(b, "Archive\0", 8)) {
	packer = "ArcFS";
	builtin = BUILTIN_ARCFS;
    } else if (b[0] == 'z' && b[1] == 'i' && b[2] == 'R' && b[3] == 'C' &&
		b[4] == 'O' && b[5] == 'N' && b[6] == 'i' && b[7] == 'a') {
	packer = "MMCMP";
	builtin = BUILTIN_MMCMP;
    } else if (b[0] == 'R' && b[1] == 'a' && b[2] == 'r') {
	packer = "rar";
	cmd = "unrar p -inul -xreadme -x*.diz -x*.nfo -x*.txt "
	    "-x*.exe -x*.com \"%s\"";
#if !defined WIN32 && !defined __AMIGA__
    } else if (test_oxm(*f) == 0) {
	packer = "oggmod";
	builtin = BUILTIN_OXM;
#endif
    } else {
	int extra;
	int s = PW_TEST_CHUNK;

	while ((extra = pw_check(b, s)) > 0) {
	    b = realloc(b, s + extra);
	    fread(b + s, extra, 1, *f);
	    s += extra;
	}

	if (extra == 0) {
	    struct pw_format *format;

	    format = list_entry(checked_format, struct pw_format, list);

	    if (format->enable) {
	        packer = format->name;
	        builtin = BUILTIN_PW;

		xmp_enable_format("MOD", 1);
	    }
	}
    }

    /* Test Arc after prowizard to prevent misidentification */
    if (packer == NULL && b[0] == 0x1a) {
	int x = b[1] & 0x7f;
	if (x >= 1 && x <= 9 && x != 7) {
	    packer = "Arc";
	    builtin = BUILTIN_ARC;
	} else if (x == 0x7f) {
	    packer = "!Spark";
	    builtin = BUILTIN_ARC;
	}
    }

    free(b);

    fseek(*f, 0, SEEK_SET);

    if (packer == NULL) {
	free(temp);
	return 0;
    }

    /* TODO: move pw to loaders -- add better Arc test above */
    if (builtin != BUILTIN_PW)
       reportv(ctx, 0, "Depacking %s file... ", packer);

    if ((fd = mkstemp(temp)) < 0) {
	if (o->verbosity > 0)
	    report("failed\n");
	goto err;
    }

    if ((t = fdopen(fd, "w+b")) == NULL) {
	reportv(ctx, 0, "failed\n");
	goto err;
    }

    if (cmd) {
	int n, lsize;
	char *line, *buf;
	FILE *p;

	lsize = strlen(cmd) + strlen(*s) + 16;
	line = malloc(lsize);
	snprintf(line, lsize, cmd, *s);

#ifdef WIN32
	if ((p = popen(line, "rb")) == NULL) {
#else
	if ((p = popen(line, "r")) == NULL) {
#endif
	    reportv(ctx, 0, "failed\n");
	    fclose(t);
	    free(line);
	    goto err2;
	}
	free (line);
#define BSIZE 0x4000
	if ((buf = malloc (BSIZE)) == NULL) {
	    reportv(ctx, 0, "failed\n");
	    pclose (p);
	    fclose (t);
	    goto err2;
	}
	while ((n = fread(buf, 1, BSIZE, p)) > 0) {
	    fwrite(buf, 1, n, t);
	} 
	free(buf);
	pclose (p);
    } else {
	switch (builtin) {
	case BUILTIN_PP:    
	    res = decrunch_pp(*f, t);
	    break;
	case BUILTIN_ARC:
	    res = decrunch_arc(*f, t);
	    break;
	case BUILTIN_ARCFS:
	    res = decrunch_arcfs(*f, t);
	    break;
	case BUILTIN_SQSH:    
	    res = decrunch_sqsh(*f, t);
	    break;
	case BUILTIN_MMCMP:    
	    res = decrunch_mmcmp(*f, t);
	    break;
#if !defined __MINGW32__ && !defined __AMIGA__
	case BUILTIN_OXM:
	    res = decrunch_oxm(*f, t);
	    break;
#endif
#ifdef AMIGA
	case BUILTIN_XFD:
	    res = decrunch_xfd(*f, t);
	    break;
#endif
	case BUILTIN_PW:
	    res = decrunch_pw(*f, t);
	    break;
	}
    }

    if (res < 0) {
	reportv(ctx, 0, "failed\n");
	goto err2;
    }

    reportv(ctx, 0, "done\n");

    fclose(*f);
    *f = t;
 
    temp2 = strdup(temp);
    decrunch(ctx, f, &temp);
    unlink(temp2);
    free(temp2);
    free(temp);

    return 1;

err2:
    unlink(temp);
err:
    free(temp);
    return -1;
}


static void get_smp_size(struct xmp_player_context *p, int awe, int *a, int *b)
{
    int i, len, smp_size, smp_4kb;

    for (smp_4kb = smp_size = i = 0; i < p->m.xxh->smp; i++) {
	len = p->m.xxs[i].len;

	/* AWE cards work with 16 bit samples only and expand bidirectional
	 * loops.
	 */
	if (awe) {
	    if (p->m.xxs[i].flg & WAVE_BIDIR_LOOP)
		len += p->m.xxs[i].lpe - p->m.xxs[i].lps;
	    if (awe && (~p->m.xxs[i].flg & WAVE_16_BITS))
		len <<= 1;
	}

	smp_size += (len += sizeof (int)); 
	if (len < 0x1000)
	    smp_4kb += len;
    }

    *a = smp_size;
    *b = smp_4kb;
}


static int crunch_ratio(struct xmp_context *ctx, int awe)
{
    struct xmp_player_context *p = &ctx->p;
    struct xmp_driver_context *d = &ctx->d;
    struct xmp_options *o = &ctx->o;
    int memavl, smp_size, ratio, smp_4kb;

    ratio = 0x10000;
    if (!(memavl = d->memavl))
	return ratio;

    memavl = memavl * 100 / (100 + o->crunch);

    get_smp_size(p, awe, &smp_size, &smp_4kb);

    if (smp_size > memavl) {
	if (!awe)
	    xmp_cvt_to8bit(ctx);
	get_smp_size(p, awe, &smp_size, &smp_4kb);
    }

    if (smp_size > memavl) {
	ratio = (int)
	    (((long long)(memavl - smp_4kb) << 16) / (smp_size - smp_4kb));
	if (o->verbosity)
	    report ("Crunch ratio   : %d%% [Mem:%.3fMb Smp:%.3fMb]\n",
		100 - 100 * ratio / 0x10000, .000001 * d->memavl,
		.000001 * smp_size);
    }
	
    return ratio;
}


int xmp_test_module(xmp_context ctx, char *s, char *n)
{
    FILE *f;
    struct xmp_loader_info *li;
    struct list_head *head;
    struct stat st;

    if ((f = fopen(s, "rb")) == NULL)
	return -3;

    if (fstat(fileno(f), &st) < 0)
	goto err;

    if (S_ISDIR(st.st_mode))
	goto err;

    if (decrunch((struct xmp_context *)ctx, &f, &s) < 0)
	goto err;

    if (fstat(fileno(f), &st) < 0)	/* get size after decrunch */
	goto err;

    list_for_each(head, &loader_list) {
	li = list_entry(head, struct xmp_loader_info, list);
	if (li->enable) {
	    fseek(f, 0, SEEK_SET);
	    if (li->test(f, n, 0) == 0) {
	        fclose(f);
	        return 0;
	    }
	}
    }

err:
    fclose (f);
    return -1;
}


static void split_name(char *s, char **d, char **b)
{
	char tmp, *div;

	if ((div = strrchr(s, '/'))) {
		tmp = *(div + 1);
		*(div + 1) = 0;
		*d = strdup(s);
		*(div + 1) = tmp;
		*b = strdup(div + 1);
	} else {
		*d = strdup("");
		*b = strdup(s);
	}
}


int xmp_load_module(xmp_context ctx, char *s)
{
    FILE *f;
    int i, t;
    struct xmp_loader_info *li;
    struct list_head *head;
    struct stat st;
    unsigned int crc;
    struct xmp_player_context *p = &((struct xmp_context *)ctx)->p;
    struct xmp_driver_context *d = &((struct xmp_context *)ctx)->d;
    struct xmp_mod_context *m = &p->m;
    struct xmp_options *o = &((struct xmp_context *)ctx)->o;

    _D(_D_WARN "s = %s", s);

    if ((f = fopen(s, "rb")) == NULL)
	return -3;

    if (fstat(fileno (f), &st) < 0)
	goto err;

    if (S_ISDIR(st.st_mode))
	goto err;

    _D(_D_INFO "decrunch");
    if ((t = decrunch((struct xmp_context *)ctx, &f, &s)) < 0)
	goto err;

    if (fstat(fileno(f), &st) < 0)	/* get size after decrunch */
	goto err;

    split_name(s, &m->dirname, &m->basename);

    crc = cksum(f);

    _D(_D_INFO "clear mem");
    xmp_drv_clearmem((struct xmp_context *)ctx);

    /* Reset variables */
    memset(m->name, 0, XMP_NAMESIZE);
    memset(m->type, 0, XMP_NAMESIZE);
    memset(m->author, 0, XMP_NAMESIZE);
    m->filename = s;		/* For ALM, SSMT, etc */
    m->size = st.st_size;
    m->rrate = PAL_RATE;
    m->c4rate = C4_PAL_RATE;
    m->volbase = 0x40;
    m->volume = 0x40;
    m->vol_xlat = NULL;
    /* Reset control for next module */
    m->fetch = o->flags & ~XMP_CTL_FILTER;

    _D(_D_INFO "read modconf");
    xmpi_read_modconf((struct xmp_context *)ctx, crc, st.st_size);

    m->xxh = calloc(sizeof (struct xxm_header), 1);
    /* Set defaults */
    m->xxh->tpo = 6;
    m->xxh->bpm = 125;
    m->xxh->chn = 4;

    for (i = 0; i < 64; i++) {
	m->xxc[i].pan = (((i + 1) / 2) % 2) * 0xff;
	m->xxc[i].cho = o->chorus;
	m->xxc[i].rvb = o->reverb;
	m->xxc[i].vol = 0x40;
	m->xxc[i].flg = 0;
    }

    m->verbosity = o->verbosity;

    _D(_D_WARN "load");
    list_for_each(head, &loader_list) {
	li = list_entry(head, struct xmp_loader_info, list);

        _D(_D_INFO "check exclusion");
	if (li->enable == 0)
	    continue;
        _D(_D_INFO "not excluded");
	
	if (o->verbosity > 3)
	    report("Test format: %s (%s)\n", li->id, li->name);
	fseek(f, 0, SEEK_SET);
   	if ((i = li->test(f, NULL, 0)) == 0) {
	    if (o->verbosity > 3)
		report("Identified as %s\n", li->id);
	    fseek(f, 0, SEEK_SET);
	    _D(_D_WARN "load format: %s (%s)", li->id, li->name);
	    if ((i = li->loader((struct xmp_context *)ctx, f, 0) == 0))
		break;
	}
    }

    fclose(f);

    if (i < 0)
	return i;

    if (d->description && (i = (strstr(d->description, " [AWE") != NULL))) {
	xmp_cvt_to16bit((struct xmp_context *)ctx);
	xmp_cvt_bid2und((struct xmp_context *)ctx);
    }

    xmp_drv_flushpatch((struct xmp_context *)ctx, crunch_ratio((struct xmp_context *)ctx, i));

    /* Fix cases where the restart value is invalid e.g. kc_fall8.xm
     * from http://aminet.net/mods/mvp/mvp_0002.lha (reported by
     * Ralf Hoffmann <ralf@boomerangsworld.de>)
     */
    if (m->xxh->rst >= m->xxh->len)
	m->xxh->rst = 0;

    /* Disable filter if --nofilter is specified */
    m->fetch &= ~(~o->flags & XMP_CTL_FILTER);

    str_adj(m->name);
    if (!*m->name)
	strncpy(m->name, m->basename, XMP_NAMESIZE);

    if (o->verbosity > 1) {
	report("Module looping : %s\n",
	    m->fetch & XMP_CTL_LOOP ? "yes" : "no");
	report("Period mode    : %s\n",
	    m->xxh->flg & XXM_FLG_LINEAR ? "linear" : "Amiga");
    }

    if (o->verbosity > 2) {
	report("Amiga range    : %s\n", m->xxh->flg & XXM_FLG_MODRNG ?
		"yes" : "no");
	report("Restart pos    : %d\n", m->xxh->rst);
	report("Base volume    : %d\n", m->volbase);
	report("C4 replay rate : %d\n", m->c4rate);
	report("Channel mixing : %d%% (dynamic pan %s)\n",
		m->fetch & XMP_CTL_REVERSE ? -o->mix : o->mix,
		m->fetch & XMP_CTL_DYNPAN ? "enabled" : "disabled");
    }

    if (o->verbosity) {
	report("Channels       : %d [ ", m->xxh->chn);
	for (i = 0; i < m->xxh->chn; i++) {
	    if (m->xxc[i].flg & XXM_CHANNEL_FM)
		report("F ");
	    else
	        report("%x ", m->xxc[i].pan >> 4);
	}
	report("]\n");
    }

    t = xmpi_scan_module((struct xmp_context *)ctx);

    if (o->verbosity) {
	if (m->fetch & XMP_CTL_LOOP)
	    report ("One loop time  : %dmin%02ds\n",
		(t + 500) / 60000, ((t + 500) / 1000) % 60);
	else
	    report ("Estimated time : %dmin%02ds\n",
		(t + 500) / 60000, ((t + 500) / 1000) % 60);
    }

    m->time = t;

    return t;

err:
    fclose(f);
    return -1;
}


int xmp_enable_format(char *id, int enable)
{
    struct list_head *head;
    struct xmp_loader_info *li;

    list_for_each(head, &loader_list) {
	li = list_entry(head, struct xmp_loader_info, list);
	if (!strcasecmp(id, li->id)) {
	    li->enable = enable;
	    return 0;
        }
    }

    return pw_enable(id, enable);
}

