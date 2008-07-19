/*
 * xrick/src/e_sbonus.c
 *
 * Copyright (C) 1998-2002 BigOrno (bigorno@bigorno.net). All rights reserved.
 *
 * The use and distribution terms for this software are contained in the file
 * named README, which can be found in the root of this distribution. By
 * using this software in any fashion, you are agreeing to be bound by the
 * terms of this license.
 *
 * You must not remove this notice, or any other, from this software.
 */

#include "system.h"
#include "game.h"
#include "ents.h"
#include "e_sbonus.h"

#include "util.h"
#include "maps.h"
#include "e_rick.h"


/*
 * public vars
 */
U8 e_sbonus_counting = FALSE;
U8 e_sbonus_counter = 0;
U16 e_sbonus_bonus = 0;


/*
 * Entity action / start counting
 *
 * ASM 2182
 */
void
e_sbonus_start(U8 e)
{
	ent_ents[e].sprite = 0; /* invisible */
	if (u_trigbox(e, ENT_XRICK.x + 0x0C, ENT_XRICK.y + 0x0A)) {
		/* rick is within trigger box */
		ent_ents[e].n = 0;
		e_sbonus_counting = TRUE;  /* 6DD5 */
		e_sbonus_counter = 0x1e;  /* 6DDB */
		e_sbonus_bonus = 2000;    /* 291A-291D */
#ifdef ENABLE_SOUND
		syssnd_play(WAV_SBONUS1, 1);
#endif
	}
}


/*
 * Entity action / stop counting
 *
 * ASM 2143
 */
void
e_sbonus_stop(U8 e)
{
	ent_ents[e].sprite = 0; /* invisible */

	if (!e_sbonus_counting)
		return;

	if (u_trigbox(e, ENT_XRICK.x + 0x0C, ENT_XRICK.y + 0x0A)) {
		/* rick is within trigger box */
		e_sbonus_counting = FALSE;  /* stop counting */
		ent_ents[e].n = 0;  /* deactivate entity */
		game_score += e_sbonus_bonus;  /* add bonus to score */
#ifdef ENABLE_SOUND
		syssnd_play(WAV_SBONUS2, 1);
#endif
		/* make sure the entity won't be activated again */
		map_marks[ent_ents[e].mark].ent |= MAP_MARK_NACT;
	}
	else {
		/* keep counting */
		if (--e_sbonus_counter == 0) {
			e_sbonus_counter = 0x1e;
			if (e_sbonus_bonus) e_sbonus_bonus--;
		}
	}
}

/* eof */


