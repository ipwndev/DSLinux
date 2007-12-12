/* Extended Module Player
 * Copyright (C) 1996-2007 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * $Id: control.c,v 1.35 2007/11/20 23:38:19 cmatsuoka Exp $
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
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "driver.h"
#include "mixer.h"

static int drv_parm = 0;
extern struct xmp_drv_info drv_callback;


int pw_init(void);

void *xmp_create_context()
{
	struct xmp_context *ctx;
	struct xmp_options *o;
	uint16 w;

	ctx = calloc(1, sizeof(struct xmp_context));

	if (ctx == NULL)
		return NULL;

	o = &ctx->o;

	w = 0x00ff;
	o->big_endian = (*(char *)&w == 0x00);

	/* Set defaults */
	o->freq = 44100;
	o->mix = 80;
	o->resol = 16;
	o->flags = XMP_CTL_DYNPAN | XMP_CTL_FILTER | XMP_CTL_ITPT;

	/* Set max number of voices per channel */
	o->maxvoc = 16;

	return ctx;
}

void xmp_free_context(xmp_context ctx)
{
	free(ctx);
}

struct xmp_options *xmp_get_options(xmp_context ctx)
{
	return &((struct xmp_context *)ctx)->o;
}

void xmp_init_callback(xmp_context ctx, void (*callback) (void *, int))
{
	struct xmp_options *o = &((struct xmp_context *)ctx)->o;

	xmp_drv_register(&drv_callback);
	xmp_init_formats(ctx);
	pw_init();

	xmp_register_driver_callback(ctx, callback);

	o->drv_id = "callback";
}

void xmp_init(xmp_context ctx, int argc, char **argv)
{
	int num;

	xmp_init_drivers();
	xmp_init_formats(ctx);
	pw_init();

	xmp_event_callback = NULL;

	/* must be parsed before loading the rc file. */
	for (num = 1; num < argc; num++) {
		if (!strcmp(argv[num], "--norc"))
			break;
	}
	if (num >= argc)
		xmpi_read_rc((struct xmp_context *)ctx);

	xmpi_tell_wait();
}

inline int xmp_open_audio(xmp_context ctx)
{
	return xmp_drv_open((struct xmp_context *)ctx);
}

inline void xmp_close_audio(xmp_context ctx)
{
	xmp_drv_close((struct xmp_context *)ctx);
}

void xmp_set_driver_parameter(struct xmp_options *o, char *s)
{
	o->parm[drv_parm] = s;
	while (isspace(*o->parm[drv_parm]))
		o->parm[drv_parm]++;
	drv_parm++;
}

inline void xmp_register_event_callback(void (*cb) (unsigned long))
{
	xmp_event_callback = cb;
}

void xmp_channel_mute(xmp_context ctx, int from, int num, int on)
{
	from += num - 1;

	if (num > 0) {
		while (num--)
			xmp_drv_mute((struct xmp_context *)ctx, from - num, on);
	}
}

int xmp_player_ctl(xmp_context ctx, int cmd, int arg)
{
	struct xmp_player_context *p = &((struct xmp_context *)ctx)->p;
	struct xmp_mod_context *m = &p->m;

	switch (cmd) {
	case XMP_ORD_PREV:
		if (p->pos > 0)
			p->pos--;
		return p->pos;
	case XMP_ORD_NEXT:
		if (p->pos < m->xxh->len)
			p->pos++;
		return p->pos;
	case XMP_ORD_SET:
		if (arg < m->xxh->len && arg >= 0)
			p->pos = arg;
		return p->pos;
	case XMP_MOD_STOP:
		p->pos = -2;
		break;
	case XMP_MOD_PAUSE:
		p->pause ^= 1;
		return p->pause;
	case XMP_MOD_RESTART:
		p->pos = -1;
		break;
	case XMP_GVOL_DEC:
		if (m->volume > 0)
			m->volume--;
		return m->volume;
	case XMP_GVOL_INC:
		if (m->volume < 64)
			m->volume++;
		return m->volume;
	case XMP_TIMER_STOP:
		xmp_drv_stoptimer((struct xmp_context *)ctx);
		break;
	case XMP_TIMER_RESTART:
		xmp_drv_starttimer((struct xmp_context *)ctx);
		break;
	}

	return XMP_OK;
}

int xmp_play_module(xmp_context ctx)
{
	struct xmp_options *o = &((struct xmp_context *)ctx)->o;
	time_t t0, t1;
	int t;

	time(&t0);
	xmpi_player_start((struct xmp_context *)ctx);
	time(&t1);
	t = difftime(t1, t0);

	o->start = 0;

	return t;
}

void xmp_release_module(xmp_context ctx)
{
	struct xmp_player_context *p = &((struct xmp_context *)ctx)->p;
	struct xmp_mod_context *m = &p->m;
	int i;

	_D(_D_INFO "Freeing memory");

	if (m->med_vol_table) {
		for (i = 0; i < m->xxh->ins; i++)
			if (m->med_vol_table[i])
				free(m->med_vol_table[i]);
		free(m->med_vol_table);
	}

	if (m->med_wav_table) {
		for (i = 0; i < m->xxh->ins; i++)
			if (m->med_wav_table[i])
				free(m->med_wav_table[i]);
		free(m->med_wav_table);
	}

	for (i = 0; i < m->xxh->trk; i++)
		free(m->xxt[i]);
	for (i = 0; i < m->xxh->pat; i++)
		free(m->xxp[i]);
	for (i = 0; i < m->xxh->ins; i++) {
		free(m->xxfe[i]);
		free(m->xxpe[i]);
		free(m->xxae[i]);
		free(m->xxi[i]);
	}
	free(m->xxt);
	free(m->xxp);
	free(m->xxi);
	if (m->xxh->smp > 0)
		free(m->xxs);
	free(m->xxim);
	free(m->xxih);
	free(m->xxfe);
	free(m->xxpe);
	free(m->xxae);
	free(m->xxh);

	if (m->dirname)
		free(m->dirname);
	if (m->basename)
		free(m->basename);
}

void xmp_get_driver_cfg(xmp_context ctx, int *srate, int *res, int *chn,
			int *itpt)
{
	struct xmp_driver_context *d = &((struct xmp_context *)ctx)->d;
	struct xmp_options *o = &((struct xmp_context *)ctx)->o;

	*srate = d->memavl ? 0 : o->freq;
	*res = o->resol ? o->resol : 8 /* U_LAW */ ;
	*chn = o->outfmt & XMP_FMT_MONO ? 1 : 2;
	*itpt = !!(o->flags & XMP_CTL_ITPT);
}

int xmp_verbosity_level(xmp_context ctx, int i)
{
	struct xmp_options *o = &((struct xmp_context *)ctx)->o;
	int tmp;

	tmp = o->verbosity;
	o->verbosity = i;

	return tmp;
}

int xmp_seek_time(xmp_context ctx, int time)
{
	struct xmp_player_context *p = &((struct xmp_context *)ctx)->p;
	int i, t;
	/* _D("seek to %d, total %d", time, xmp_cfg.time); */

	time *= 1000;
	for (i = 0; i < p->m.xxh->len; i++) {
		t = p->m.xxo_info[i].time;

		_D("%2d: %d %d", i, time, t);

		if (t > time) {
			if (i > 0)
				i--;
			xmp_ord_set(ctx, i);
			return XMP_OK;
		}
	}
	return -1;
}
