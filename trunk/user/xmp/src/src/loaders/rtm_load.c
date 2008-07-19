/* Extended Module Player
 * Copyright (C) 1996-2007 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * $Id: rtm_load.c,v 1.19 2007/11/30 17:30:22 cmatsuoka Exp $
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU General Public License. See doc/COPYING
 * for more information.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "load.h"
#include "period.h"
#include "rtm.h"


static int rtm_test(FILE *, char *, const int);
static int rtm_load (struct xmp_context *, FILE *, const int);

struct xmp_loader_info rtm_loader = {
	"RTM",
	"Real Tracker",
	rtm_test,
	rtm_load
};

static int rtm_test(FILE *f, char *t, const int start)
{
	char buf[4];

	fread(buf, 4, 1, f);
	if (memcmp(buf, "RTMM", 4))
		return -1;

	if (read8(f) != 0x20)
		return -1;

	read_title(f, t, 32);

	return 0;
}


#define MAX_SAMP 1024

static int read_object_header(FILE *f, struct ObjectHeader *h)
{
	fread(&h->id, 4, 1, f);
	_D(_D_WARN "object id: %02x %02x %02x %02x", h->id[0],
					h->id[1], h->id[2], h->id[3]);
	h->rc = read8(f);
	if (h->rc != 0x20)
		return -1;
	fread(&h->name, 32, 1, f);
	h->eof = read8(f);
	h->version = read16l(f);
	h->headerSize = read16l(f);
	_D(_D_INFO "object %-4.4s (%d)", h->id, h->headerSize);
	
	return 0;
}


static int rtm_load(struct xmp_context *ctx, FILE *f, const int start)
{
	struct xmp_player_context *p = &ctx->p;
	struct xmp_mod_context *m = &p->m;
	int i, j, r;
	struct xxm_event *event;
	struct ObjectHeader oh;
	struct RTMMHeader rh;
	struct RTNDHeader rp;
	struct RTINHeader ri;
	struct RTSMHeader rs;
	int offset, smpnum;

	LOAD_INIT();

	read_object_header(f, &oh);

	fread(&rh.software, 1, 20, f);
	fread(&rh.composer, 1, 32, f);
	rh.flags = read16l(f);	/* bit 0: linear table, bit 1: track names */
	rh.ntrack = read8(f);
	rh.ninstr = read8(f);
	rh.nposition = read16l(f);
	rh.npattern = read16l(f);
	rh.speed = read8(f);
	rh.tempo = read8(f);
	fread(&rh.panning, 32, 1, f);
	rh.extraDataSize = read32l(f);
	for (i = 0; i < rh.nposition; i++)
		m->xxo[i] = read16l(f);
	
	strncpy(m->name, oh.name, 20);
	snprintf(m->type, XMP_NAMESIZE, "RTMM %d.%02d (%s)",
			oh.version >> 8, oh.version & 0xff, rh.software);

	m->xxh->len = rh.nposition;
	m->xxh->pat = rh.npattern;
	m->xxh->chn = rh.ntrack;
	m->xxh->trk = m->xxh->chn * m->xxh->pat + 1;
	m->xxh->ins = rh.ninstr;
	m->xxh->tpo = rh.speed;
	m->xxh->bpm = rh.tempo;
	m->xxh->flg = rh.flags & 0x01 ? XXM_FLG_LINEAR : 0;

	MODULE_INFO();

	for (i = 0; i < m->xxh->chn; i++)
		m->xxc[i].pan = rh.panning[i] & 0xff;

	PATTERN_INIT();

	if (V(0))
		report("Stored patterns: %d ", m->xxh->pat);

	offset = 42 + oh.headerSize + rh.extraDataSize;

	for (i = 0; i < m->xxh->pat; i++) {
		uint8 c;

		fseek(f, start + offset, SEEK_SET);

		read_object_header(f, &oh);
	
		rp.flags = read16l(f);
		rp.ntrack = read8(f);
		rp.nrows = read16l(f);
		rp.datasize = read32l(f);

		offset += 42 + oh.headerSize + rp.datasize;

		PATTERN_ALLOC(i);
		m->xxp[i]->rows = rp.nrows;
		TRACK_ALLOC(i);

		for (r = 0; r < rp.nrows; r++) {
			for (j = 0; j < rp.ntrack; j++) {
				event = &EVENT(i, j, r);
				c = read8(f);
				if (c == 0)		/* next row */
					break;
				if (c & 0x01) {		/* set track */
					j = read8(f);
					event = &EVENT(i, j, r);
				}
				if (c & 0x02) {		/* read note */
					event->note = read8(f);
					if (event->note == 0x61)
						event->note = XMP_KEY_OFF;
				}
				if (c & 0x04)		/* read instrument */
					event->ins = read8(f);
				if (c & 0x08)		/* read effect */
					event->fxt = read8(f);
				if (c & 0x10)		/* read parameter */
					event->fxp = read8(f);
				if (c & 0x20)		/* read effect 2 */
					event->f2t = read8(f);
				if (c & 0x40)		/* read parameter 2 */
					event->f2p = read8(f);
					
			}
		}

		if (V(0))
		report(".");
	}

	if (V(0))
		report("\n");

	/*
	 * load instruments
	 */

	if (V(0))
		report("Instruments    : %d ", m->xxh->ins);
	if (V(1))
		report("\n");

	fseek(f, start + offset, SEEK_SET);

	/* ESTIMATED value! We don't know the actual value at this point */
	m->xxh->smp = MAX_SAMP;

	INSTRUMENT_INIT();

	for (smpnum = i = 0; i < m->xxh->ins; i++) {
		read_object_header(f, &oh);

		copy_adjust(m->xxih[i].name, (uint8 *)&oh.name, 32);

		if (oh.headerSize == 0) {
			if (V(1) && strlen((char *)m->xxih[i].name)) {
				report("[%2X] %-26.26s %2d ", i, m->xxih[i].name,
								m->xxih[i].nsm);
			}
			ri.nsample = 0;
			if (V(1) && (strlen((char *)m->xxih[i].name) || m->xxih[i].nsm))
				report("\n");
			continue;
		}

		ri.nsample = read8(f);
		ri.flags = read16l(f);	/* bit 0 : default panning enabled */
		fread(&ri.table, 120, 1, f);

		ri.volumeEnv.npoint = read8(f);
		for (j = 0; j < 12; j++) {
			ri.volumeEnv.point[j].x = read32l(f);
			ri.volumeEnv.point[j].y = read32l(f);
		}
		ri.volumeEnv.sustain = read8(f);
		ri.volumeEnv.loopstart = read8(f);
		ri.volumeEnv.loopend = read8(f);
		ri.volumeEnv.flags = read16l(f); /* bit 0:enable 1:sus 2:loop */
		
		ri.panningEnv.npoint = read8(f);
		for (j = 0; j < 12; j++) {
			ri.panningEnv.point[j].x = read32l(f);
			ri.panningEnv.point[j].y = read32l(f);
		}
		ri.panningEnv.sustain = read8(f);
		ri.panningEnv.loopstart = read8(f);
		ri.panningEnv.loopend = read8(f);
		ri.panningEnv.flags = read16l(f);

		ri.vibflg = read8(f);
		ri.vibsweep = read8(f);
		ri.vibdepth = read8(f);
		ri.vibrate = read8(f);
		ri.volfade = read16l(f);

		/* I don't see these inside the module */
		//ri.midiPort = read8(f);
		//ri.midiChannel = read8(f);
		//ri.midiProgram = read8(f);
		//ri.midiEnable = read8(f);

		m->xxih[i].nsm = ri.nsample;
		if (V(1) && (strlen((char *)m->xxih[i].name) || ri.nsample)) {
			report("[%2X] %-26.26s %2d ", i, m->xxih[i].name,
							m->xxih[i].nsm);
		}
		if (m->xxih[i].nsm > 16)
			m->xxih[i].nsm = 16;
		m->xxi[i] = calloc(sizeof (struct xxm_instrument), m->xxih[i].nsm);

		for (j = 0; j < 96; j++)
			m->xxim->ins[j] = ri.table[j + 12];

		/* Envelope */
		m->xxih[i].rls = ri.volfade;
		m->xxih[i].aei.npt = ri.volumeEnv.npoint;
		m->xxih[i].aei.sus = ri.volumeEnv.sustain;
		m->xxih[i].aei.lps = ri.volumeEnv.loopstart;
		m->xxih[i].aei.lpe = ri.volumeEnv.loopend;
		m->xxih[i].aei.flg = ri.volumeEnv.flags;
		m->xxih[i].pei.npt = ri.panningEnv.npoint;
		m->xxih[i].pei.sus = ri.panningEnv.sustain;
		m->xxih[i].pei.lps = ri.panningEnv.loopstart;
		m->xxih[i].pei.lpe = ri.panningEnv.loopend;
		m->xxih[i].pei.flg = ri.panningEnv.flags;
		if (m->xxih[i].aei.npt)
			m->xxae[i] = calloc(4, m->xxih[i].aei.npt);
		else
			m->xxih[i].aei.flg &= ~XXM_ENV_ON;
		if (m->xxih[i].pei.npt)
			m->xxpe[i] = calloc(4, m->xxih[i].pei.npt);
		else
			m->xxih[i].pei.flg &= ~XXM_ENV_ON;

		for (j = 0; j < m->xxih[i].aei.npt; j++) {
			m->xxae[i][j * 2 + 0] = ri.volumeEnv.point[j].x;
			m->xxae[i][j * 2 + 1] = ri.volumeEnv.point[j].y / 2;
		}
		for (j = 0; j < m->xxih[i].pei.npt; j++) {
			m->xxpe[i][j * 2 + 0] = ri.panningEnv.point[j].x;
			m->xxpe[i][j * 2 + 1] = 32 + ri.panningEnv.point[j].y / 2;
		}

		/* For each sample */
		for (j = 0; j < m->xxih[i].nsm; j++, smpnum++) {
			read_object_header(f, &oh);

			rs.flags = read16l(f);
			rs.basevolume = read8(f);
			rs.defaultvolume = read8(f);
			rs.length = read32l(f);
			rs.loop = read32l(f);
			rs.loopbegin = read32l(f);
			rs.loopend = read32l(f);
			rs.basefreq = read32l(f);
			rs.basenote = read8(f);
			rs.panning = read8(f);

			c2spd_to_note(rs.basefreq,
					&m->xxi[i][0].xpo, &m->xxi[i][0].fin);
			m->xxi[i][j].xpo += 48 - rs.basenote;

			m->xxi[i][j].vol = rs.defaultvolume * rs.basevolume / 0x40;
			m->xxi[i][j].pan = 0x80 + rs.panning * 2;
			m->xxi[i][j].vwf = ri.vibflg;
			m->xxi[i][j].vde = ri.vibdepth;
			m->xxi[i][j].vra = ri.vibrate;
			m->xxi[i][j].vsw = ri.vibsweep;
			m->xxi[i][j].sid = smpnum;

			if (smpnum >= MAX_SAMP) {
				fseek(f, rs.length, SEEK_CUR);
				continue;
			}

			copy_adjust(m->xxs[smpnum].name, (uint8 *)oh.name, 32);

			m->xxs[smpnum].len = rs.length;
			m->xxs[smpnum].lps = rs.loopbegin;
			m->xxs[smpnum].lpe = rs.loopend;
			m->xxs[smpnum].flg = rs.flags & 0x02 ?  WAVE_16_BITS : 0;
			m->xxs[smpnum].flg |= rs.loop & 0x03 ?  WAVE_LOOPING : 0;
			m->xxs[smpnum].flg |= rs.loop == 2 ? WAVE_BIDIR_LOOP : 0;

			if ((V(1)) && rs.length) {
				report ("%s[%1x] %05x%c%05x %05x %c "
						"V%02x F%+04d P%02x R%+03d",
					j ? "\n\t\t\t\t    " : " ", j,
					m->xxs[m->xxi[i][j].sid].len,
					m->xxs[m->xxi[i][j].sid].flg & WAVE_16_BITS ? '+' : ' ',
					m->xxs[m->xxi[i][j].sid].lps,
					m->xxs[m->xxi[i][j].sid].lpe,
					m->xxs[m->xxi[i][j].sid].flg & WAVE_BIDIR_LOOP ? 'B' :
					m->xxs[m->xxi[i][j].sid].flg & WAVE_LOOPING ? 'L' : ' ',
					m->xxi[i][j].vol, m->xxi[i][j].fin,
					m->xxi[i][j].pan, m->xxi[i][j].xpo);

			}

			xmp_drv_loadpatch(ctx, f, m->xxi[i][j].sid, m->c4rate,
				XMP_SMP_DIFF, &m->xxs[m->xxi[i][j].sid], NULL);
		}
		if (m->verbosity == 1)
			report (".");

		if ((V(1)) && (strlen((char *) m->xxih[i].name) || ri.nsample))
			report ("\n");
	}
	if (m->verbosity == 1)
		report ("\n");

	m->xxh->smp = smpnum;
	m->xxs = realloc(m->xxs, sizeof (struct xxm_sample) * m->xxh->smp);

	m->fetch |= XMP_MODE_FT2;

	return 0;
}
