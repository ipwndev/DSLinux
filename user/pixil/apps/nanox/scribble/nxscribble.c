/***********************************************************************

nxscribble.c - NX frontend to the scribble code

Copyright (C) 2003 Century Software

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License. See ../COPYING for
the full agreement.

**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#define MWINCLUDECOLORS
#include <nano-X.h>
#include <wm/nxlib.h>

#include "scrib.h"

#include "widgets.h"
#include <string.h>
#include <unistd.h>

#include <par/par.h>

#define BOUND           64

#define STRMAX 255
#define DIR_STRMAX 200

#define RTN_UP "/rtn_dn.gif"
#define RTN_DN "/rtn_up.gif"
#define SP_UP  "/sp_up.gif"
#define SP_DN  "/sp_dn.gif"
#define BK_UP  "/bk_up.gif"
#define BK_DN  "/bk_dn.gif"

static ScribbleWidget _w[3];

GR_WINDOW_ID g_TopLevel;

#if 0
static GR_BOOL bTextwin = GR_FALSE;
static GR_WINDOW_ID wt = 0;
#endif

static GR_WINDOW_ID wid = 0;

#if 0
static GR_GC_ID gct = 0;
static GR_GC_ID gctb = 0;

static GR_COORD xpos = 0;
static GR_COORD ypos = 0;
static GR_SIZE width;		/* width of character */
static GR_SIZE height;		/* height of character */
static GR_SIZE base;		/* height of baseline */

static void char_out(GR_CHAR ch);
static void char_del(GR_COORD x, GR_COORD y);
#endif

void OnReturn(void *data);
void OnUpper(void *data);
void OnLower(void *data);
void OnNumeral(void *data);
void OnSpecial(void *data);
void OnBkSp(void *data);
void OnSp(void *data);

void do_buttondown(GR_EVENT_BUTTON * bp);
void do_buttonup(GR_EVENT_BUTTON * bp);
void do_motion(GR_EVENT_MOUSE * mp);
void do_focusin(GR_EVENT_GENERAL * gp);
void do_exposure(GR_EVENT_EXPOSURE * ep);

button_t *rtn, *specialChar;

int
main(int ac, char **av)
{

#if 0
    int t = 1;
#endif

    GR_EVENT event;		/* current event */

    db_handle *db;
    char image_path[STRMAX], rtn_up[STRMAX], rtn_dn[STRMAX],
	sp_up[STRMAX], sp_dn[STRMAX], bk_up[STRMAX], bk_dn[STRMAX];

    /* widgets */


    button_t *bkSp, *sp;

#if 0
    while (t < ac) {
	if (!strcmp("-t", av[t])) {
	    bTextwin = GR_TRUE;
	    ++t;
	    continue;
	}
    }
#endif

    if (GrOpen() < 0) {
	fprintf(stderr, "cannot open graphics\n");
	exit(1);
    }


    g_TopLevel = create_top_window();

    /* create scribble input window */
    _w[0] = create_scribble(g_TopLevel, 0);
    _w[1] = create_scribble(g_TopLevel, SCRIB_WRITE_WIDTH);
    _w[2] = create_scribble(g_TopLevel, SCRIB_WRITE_WIDTH * 2);

    FrRecognize(_w[0], CAPS);
    FrRecognize(_w[1], ABC);
    FrRecognize(_w[2], NUM);

    wid = g_TopLevel;

    db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);

    if (db < 0) {
	fprintf(stderr, "Error - Could not open par database %s\n",
		db_getDefaultDB());
    }
    //  memset(image_path, 0, STRMAX);
    par_getScreentopDir(db, "icondir", image_path, STRMAX - 1);

    /* assign image names */
    if (strlen(image_path) >= DIR_STRMAX) {
	fprintf(stderr, "Error - Image path > %d characters. Exiting.\n",
		DIR_STRMAX - 1);
	exit(-1);
    }

    /* reset */
    strncpy(rtn_up, image_path, STRMAX - 1);
    strncat(rtn_up, RTN_UP, 11);

    strncpy(rtn_dn, image_path, STRMAX - 1);
    strncat(rtn_dn, RTN_DN, 11);

    strncpy(sp_up, image_path, STRMAX - 1);
    strncat(sp_up, SP_UP, 10);

    strncpy(sp_dn, image_path, STRMAX - 1);
    strncat(sp_dn, SP_DN, 10);

    strncpy(bk_up, image_path, STRMAX - 1);
    strncat(bk_up, BK_UP, 10);

    strncpy(bk_dn, image_path, STRMAX - 1);
    strncat(bk_dn, BK_DN, 10);

    specialChar = buttonCreate(wid, BUTTON_TOGGLE,
			       SCRIB_BUTTON_X, 0,
			       SCRIB_BUTTON_WIDTH,
			       SCRIB_BUTTON_HEIGHT, "Pnct", WHITE, BLACK);

    rtn = imageButtonCreate(wid, BUTTON_PUSH,
			    SCRIB_BUTTON_X, SCRIB_BUTTON_HEIGHT,
			    SCRIB_BUTTON_WIDTH, SCRIB_BUTTON_HEIGHT,
			    rtn_up, rtn_dn);

    sp = imageButtonCreate(wid, BUTTON_PUSH,
			   SCRIB_BUTTON_X, SCRIB_BUTTON_HEIGHT * 2,
			   SCRIB_BUTTON_WIDTH, SCRIB_BUTTON_HEIGHT,
			   sp_dn, sp_up);

    bkSp = imageButtonCreate(wid, BUTTON_PUSH,
			     SCRIB_BUTTON_X, SCRIB_BUTTON_HEIGHT * 3,
			     SCRIB_BUTTON_WIDTH, SCRIB_BUTTON_HEIGHT,
			     bk_dn, bk_up);


    buttonCallback(specialChar, BUTTON_DOWN_EVENT, &OnSpecial, 0);
    buttonCallback(rtn, BUTTON_DOWN_EVENT, &OnReturn, 0);
    buttonCallback(sp, BUTTON_DOWN_EVENT, &OnSp, 0);
    buttonCallback(bkSp, BUTTON_DOWN_EVENT, &OnBkSp, 0);

    while (1) {
	GrGetNextEvent(&event);
	widgetHandler(&event);

	switch (event.type) {
	case GR_EVENT_TYPE_BUTTON_DOWN:
	    do_buttondown(&event.button);

	    break;

	case GR_EVENT_TYPE_BUTTON_UP:
	    do_buttonup(&event.button);
	    break;

	case GR_EVENT_TYPE_MOUSE_POSITION:
	case GR_EVENT_TYPE_MOUSE_MOTION:
	    do_motion(&event.mouse);
	    break;

	case GR_EVENT_TYPE_FOCUS_IN:
	    do_focusin(&event.general);
	    break;
#if 0
	case GR_EVENT_TYPE_KEY_DOWN:
	    do_keystroke(&event.keystroke);
	    break;

#endif

	case GR_EVENT_TYPE_EXPOSURE:
	    do_exposure(&event.exposure);
	    break;

	case GR_EVENT_TYPE_CLOSE_REQ:
	    GrClose();
	    exit(0);
	}
    }
}

/*
 * Return button response
 */

static ScribbleWidget
GetWidget(GR_WINDOW_ID wid)
{
    if (_w[0]->win == wid)
	return _w[0];
    if (_w[1]->win == wid)
	return _w[1];
    if (_w[2]->win == wid)
	return _w[2];

    return 0;
}

void
OnReturn(void *data)
{
    FrRecognize(_w[0], RETURN);
}

/*
 * Special characters radio button response
 */
void
OnSpecial(void *data)
{
    if (specialChar->state) {
	FrRecognize(_w[2], SHIFT);
	strcpy(specialChar->text, "123");
    } else {
	FrRecognize(_w[2], NUM);
	strcpy(specialChar->text, "Pnct");
    }
}

/*
 * Back space button response
 */
void
OnBkSp(void *data)
{
    FrRecognize(_w[0], BKSP);
}

/*
 * Spacebar button response
 */
void
OnSp(void *data)
{
    FrRecognize(_w[0], SP);
}

/*
 * Here when a button is pressed.
 */
void
do_buttondown(GR_EVENT_BUTTON * bp)
{
    //  if ( (bp->x >= BOUND) || (bp->wid != w->win) )
    //    return;

    if (GetWidget(bp->wid))
	ActionStart(GetWidget(bp->wid), bp->x, bp->y);
}


/*
 * Here when a button is released.
 */
void
do_buttonup(GR_EVENT_BUTTON * bp)
{
    //  if ( (bp->x >= BOUND) || (bp->wid != w->win) )
    //    return;

    if (GetWidget(bp->wid))
	ActionEnd(GetWidget(bp->wid), bp->x, bp->y);
}


/*
 * Here when the mouse has a motion event.
 */
void
do_motion(GR_EVENT_MOUSE * mp)
{
    //   if (mp->x >= BOUND)
    //    return;
    if (GetWidget(mp->wid))
	ActionMove(GetWidget(mp->wid), mp->x, mp->y);
}


/*
 * Here when our window gets focus
 */
void
do_focusin(GR_EVENT_GENERAL * gp)
{
#if 0
    /* if the window receiving focus is scribble, remember last window */
    if (gp->wid == w->win && gp->wid != 1)
	w->lastfocusid = gp->otherid;
#endif
}


/*
 * Here when an exposure event occurs.
 */
void
do_exposure(GR_EVENT_EXPOSURE * ep)
{
    if (GetWidget(ep->wid))
	Redisplay(GetWidget(ep->wid));
}
