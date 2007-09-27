/*
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.
 *
 * This file is part of the PIXIL Operating Environment
 *
 * The use, copying and distribution of this file is governed by one
 * of two licenses, the PIXIL Commercial License, or the GNU General
 * Public License, version 2.
 *
 * Licensees holding a valid PIXIL Commercial License may use this file
 * in accordance with the PIXIL Commercial License Agreement provided
 * with the Software. Others are governed under the terms of the GNU
 * General Public License version 2.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 2 as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included
 * in the packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * RESTRICTED RIGHTS LEGEND
 *
 * Use, duplication, or disclosure by the government is subject to
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * See http://www.pixil.org/gpl/ for GPL licensing
 * information.
 *
 * See http://www.pixil.org/license.html or
 * email cetsales@centurysoftware.com for information about the PIXIL
 * Commercial License Agreement, or if any conditions of this licensing
 * are not clear to you.
 */

#include <pixil_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include <sys/param.h>

#include <nano-X.h>
#include <nxcolors.h>

#ifdef CONFIG_PAR
#include <par/par.h>
#endif

#include "nxsunclock.h"
#include "city.h"

#define WIDTH 238
#define HEIGHT 120

#define WM_PROPS  (GR_WM_PROPS_NOAUTOMOVE | GR_WM_PROPS_BORDER |\
		   GR_WM_PROPS_CAPTION |\
		   GR_WM_PROPS_CLOSEBOX)

#define IMAGE_FILE "earth.jpg"

/* Pointers to the list of available cities */
static city_t *city_head = 0, *city_tail = 0;

/* The current local and remote city */
static city_t *local_city = 0, *remote_city = 0;

/* A struct that contains a number of settings forthe system */

struct
{
    char path[PATH_MAX];
    char local[64];
    char remote[64];
}
settings;

/* External functions - mainly to do math */
/* See astro.c for all the fun and games */

extern void sunpos(double jd, int apparent, double *ra,
		   double *dec, double *rv, double *slong);

extern double jtime(struct tm *t);
extern double gmst(double jd);

void write_settings(void);

/* Translate latitude and longitude to X and Y positions */

static void
translate(double lat, double lon, int *xpos, int *ypos)
{
    *xpos = (int) ((180.0 + lon) * ((double) WIDTH / 360.0));
    *ypos =
	(int) ((double) HEIGHT - (lat + 90.0) * ((double) HEIGHT / 180.0));
}

/* Copy the map to to the screen */

static void
copy_map(GR_WINDOW_ID wid, GR_WINDOW_ID pixmap, int width, int height)
{
    GR_GC_ID gc = GrNewGC();
    GrCopyArea(wid, gc, 0, 0, width, height, pixmap, 0, 0, MWROP_COPY);
    GrDestroyGC(gc);
}

/* Handle a mouse click on the map */
/* Select the closest city to the click */

static void
handle_map(int ex, int ey)
{

    city_t *city = 0;

    city_t *best_city = 0;
    double best_len = 999.0;

    for (city = city_head; city; city = city->next) {
	int xpos, ypos;
	double dist;

	/* Translate the lat/lon */
	translate(city->lat, city->lon, &xpos, &ypos);

	/* Figure out the distance from the click */

	dist = hypot((double) abs(ex - xpos), (double) (ey - ypos));

	if (dist < best_len) {
	    best_city = city;
	    best_len = dist;
	}
    }

    if (best_city) {
	remote_city = best_city;

	strcpy(settings.remote, remote_city->name);
	write_settings();
    }
}

/* Draw a city on the map */

static void
draw_city(GR_WINDOW_ID wid, GR_GC_ID gc, GR_COLOR color, city_t * city)
{
    int xpos, ypos;

    /* Translate the lat / lon */
    translate(city->lat, city->lon, &xpos, &ypos);

    GrSetGCForeground(gc, color);

    /* Draw a diamond pattern */

    GrPoint(wid, gc, xpos - 1, ypos);
    GrPoint(wid, gc, xpos + 1, ypos);
    GrPoint(wid, gc, xpos, ypos - 1);
    GrPoint(wid, gc, xpos, ypos + 1);
    GrPoint(wid, gc, xpos, ypos);
}


static void
draw_cities(GR_WINDOW_ID wid)
{

    city_t *city = 0;
    GR_GC_ID gc = GrNewGC();

    for (city = city_head; city; city = city->next) {

	/* Don't draw invisible cities, or the local or remote city */

	if (!city->visible)
	    continue;
	if (city == remote_city)
	    continue;
	if (city == local_city)
	    continue;

	draw_city(wid, gc, GR_COLOR_WHITE, city);
    }

    /* Draw the local and remote cities in different colors */

    if (remote_city)
	draw_city(wid, gc, GR_COLOR_RED, remote_city);
    if (local_city)
	draw_city(wid, gc, GR_COLOR_GREEN, local_city);

    GrDestroyGC(gc);
}

static GR_FONT_ID font = 0;

static void
draw_time(GR_WINDOW_ID id, city_t * city)
{

    int ypos = 35;
    char buffer[128];
    int tw, th, tb;

    GR_WINDOW_INFO wi;

    time_t now;
    struct tm *tm;

    char *prev = getenv("TZ");
    char *saved = 0;

    GR_GC_ID gc = GrNewGC();
    GrGetWindowInfo(id, &wi);

    if (!font)
	font = GrCreateFont(GR_FONT_GUI_VAR, 10, 0);
    GrSetGCFont(gc, font);

    /* Save the previous TZ setting */

    if (prev)
	saved = (char *) strdup(prev);

    if (city)
	setenv("TZ", city->tz, 1);

    /* Get the local time in the TZ */

    now = time(0);
    tm = localtime(&now);

    GrSetGCForeground(gc, GR_COLOR_WHITE);
    GrFillRect(id, gc, 3, 35, wi.width - 6, 18);

    GrSetGCForeground(gc, GR_COLOR_BLACK);
    GrSetGCBackground(gc, GR_COLOR_WHITE);

    strftime(buffer, sizeof(buffer), "%b %d %H:%M", tm);
    GrGetGCTextSize(gc, buffer, -1, GR_TFTOP, &tw, &th, &tb);
    GrText(id, gc, (wi.width - tw) / 2, ypos, buffer, -1, GR_TFTOP);

    setenv("TZ", saved, 1);
    if (saved)
	free(saved);

    GrDestroyGC(gc);
}

static void
draw_timewin(GR_WINDOW_ID id, char *title, city_t * city, int mode)
{

    int ypos = 0;
    int tw, th, tb;

    GR_WINDOW_INFO wi;
    GR_GC_ID gc = GrNewGC();

    GrGetWindowInfo(id, &wi);

    if (!font)
	font = GrCreateFont(GR_FONT_GUI_VAR, 10, 0);
    GrSetGCFont(gc, font);

    GrSetGCForeground(gc, GR_COLOR_WHITE);
    GrFillRect(id, gc, 0, 0, wi.width, 55);

    GrSetGCForeground(gc, MWRGB(0x00, 0x66, 0xCC));
    GrRect(id, gc, 0, 0, wi.width, 55);

  /**** Draw the title ******/

    GrSetGCForeground(gc, MWRGB(0x00, 0x66, 0xCC));
    GrFillRect(id, gc, 0, 0, wi.width, 15);

    GrSetGCBackground(gc, MWRGB(0x00, 0x66, 0xCC));
    GrSetGCForeground(gc, MWRGB(0xFF, 0xFF, 0xFF));

    if (title) {
	GrGetGCTextSize(gc, title, -1, GR_TFTOP, &tw, &th, &tb);
	GrText(id, gc, (wi.width - tw) / 2, ypos, title, -1, GR_TFTOP);
    }

    /* when mode is non zero, draw some navigation arrows */

    if (mode) {
	GrText(id, gc, 0, ypos, "<-", -1, GR_TFTOP);
	GrText(id, gc, wi.width - 10, ypos, "->", -1, GR_TFTOP);
    }

  /**** Draw the city ****/

    ypos += 20;

    GrSetGCBackground(gc, GR_COLOR_WHITE);
    GrSetGCForeground(gc, GR_COLOR_BLACK);

    if (city && city->name) {
	GrGetGCTextSize(gc, city->name, -1, GR_TFTOP, &tw, &th, &tb);
	GrText(id, gc, (wi.width - tw) / 2, ypos, city->name, -1, GR_TFTOP);
    }

    draw_time(id, city);

    GrDestroyGC(gc);
}

/* Handle selecting the next or previous remote city */

static int
handle_remote(GR_WINDOW_ID wid, int xpos, int ypos)
{
    GR_WINDOW_INFO wi;
    GrGetWindowInfo(wid, &wi);

    if (xpos < 10 && ypos > 0 && ypos < 15) {
	remote_city = remote_city->prev ? remote_city->prev : city_tail;
    } else if (xpos > (wi.width - 10) && ypos > 0 && ypos < 15) {
	remote_city = remote_city->next ? remote_city->next : city_head;
    } else
	return 0;

    /* Save the settings in case we shut down */

    strcpy(settings.remote, remote_city->name);
    write_settings();

    return 1;
}

/* Actually draw the map in the pximap */

static void
draw_map(GR_IMAGE_ID image, GR_WINDOW_ID pixmap, int width, int height)
{

    static GR_WINDOW_ID copy_pixmap = 0;

    /* Drawing variables */

    GR_REGION_ID region = GrNewRegion();
    GR_GC_ID gc;

    double daywave[WIDTH];

    /* Math variables */
    double sunra, sundec, sunrv, sunlon;
    double quot, cd, sd, shiftp;
    double f2, f3, spos, jgmt;
    int hemi, i;

    time_t now;
    struct tm *gmt;
    double julian;

    /* Get the current UTC time */

    now = time(0);
    gmt = gmtime(&now);

    /* Convert it to julian time */
    julian = jtime(gmt);

    /* Get the position of the sun */
    sunpos(julian, 0, &sunra, &sundec, &sunrv, &sunlon);

    jgmt = (double) gmst(julian);
    sunlon = fixangle(180.0 + (sunra - (jgmt * 15)));

    /* Convert the sun x position on the map */
    spos = sunlon * (width / 360.0);
    sunlon -= 180;

    /* Now, start to figure the y position of the sun */
    quot = dtr(sundec);

    /* Figure out what hemisphere the sun is in */
    hemi = (quot > 0) ? 1 : -1;

    /* The sine and coside components of the sun */
    cd = cos(quot);
    sd = sin(quot);

    /* This is the radius of the sun */
    quot = (2.0 * M_PI) / (double) width;

    shiftp = 0.5 * (height + 1);

    f2 = ((double) height) / M_PI;
    f3 = 1E-8 * f2;

    for (i = 0; i < width; i++) {
	double val = (double) i;
	daywave[i] = cos(quot * (val - spos));
    }

    /* now, move across the map and add in the shading */
    /* This is done by adding a small sliver of region */
    /* for all the areas that must be shaded           */

    for (i = 0; i < width; i++) {
	GR_RECT rect;
	int ypos;

	rect.x = i;
	rect.width = 1;

	if (fabs(sd) > f3)
	    ypos = (int) (shiftp + f2 * atan(daywave[i] * (cd / sd)));
	else
	    ypos = 0;

	if (ypos < 0)
	    ypos = 0;

	if (hemi == 1) {
	    rect.y = ypos;
	    rect.height = height - ypos;
	} else {
	    rect.y = 0;
	    rect.height = ypos;
	}

	GrUnionRectWithRegion(region, &rect);
    }

    /* Create a black pixmap to copy on the map */

    if (!copy_pixmap) {
	GR_GC_ID cgc = GrNewGC();

	copy_pixmap = GrNewPixmap(width, height, 0);
	GrSetGCForeground(cgc, MWRGB(0, 0, 0));
	GrFillRect(copy_pixmap, cgc, 0, 0, width, height);
	GrDestroyGC(cgc);
    }

    gc = GrNewGC();

    /* Draw the image */
    GrDrawImageToFit(pixmap, gc, 0, 0, width, height, image);

    /* Set the region - this will ensure only the desired area gets affected */
    GrSetGCRegion(gc, region);

    /* Alpha blend the shading */
    GrCopyArea(pixmap, gc, 0, 0, width, height, copy_pixmap, 0, 0,
	       MWROP_BLENDCONSTANT | 128);

    GrDestroyGC(gc);
    GrDestroyRegion(region);
}

/* Load the image from the data directory */

GR_IMAGE_ID
load_image(char *base, char *image)
{
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", base, image);
    return GrLoadImageFromFile(path, 0);
}

/* Load the settings from PAR */

void
get_settings(void)
{

#ifdef CONFIG_PAR
    db_handle *db;
#endif

    bzero(&settings, sizeof(settings));
    getcwd(settings.path, sizeof(settings.path) - 1);

    strcpy(settings.local, "Salt Lake City");
    strcpy(settings.remote, "New York");

#ifdef CONFIG_PAR
    db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);

    if (!db) {
	fprintf(stderr, "PAR database open returned error %d\n", pardb_errno);
	return;
    }

    par_getAppPref(db, "worldclock", "settings", "datapath",
		   settings.path, sizeof(settings.path));

    par_getAppPref(db, "worldclock", "settings", "local_city",
		   settings.local, sizeof(settings.local));

    par_getAppPref(db, "worldclock", "settings", "remote_city",
		   settings.remote, sizeof(settings.remote));

    db_closeDB(db);
#endif
}

void
write_settings(void)
{

#ifdef CONFIG_PAR
    db_handle *db;

    db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RW);

    if (!db) {
	fprintf(stderr, "PAR database open returned error %d\n", pardb_errno);
	return;
    }

    par_addAppPref(db, "worldclock", "settings", "datapath",
		   settings.path, strlen(settings.path), PAR_TEXT);

    par_addAppPref(db, "worldclock", "settings", "local_city",
		   settings.local, strlen(settings.local), PAR_TEXT);

    par_addAppPref(db, "worldclock", "settings", "remote_city",
		   settings.remote, strlen(settings.remote), PAR_TEXT);

    db_closeDB(db);
#endif

}

int
main(int argc, char **argv)
{

    int seconds = 0;

    GR_WINDOW_ID g_wid, g_map, g_pixmap;
    GR_IMAGE_ID g_image, g_local, g_remote;

    get_settings();

    if (load_cities(settings.path, "city.list", &city_head, &city_tail) <= 0)
	exit(-1);

    /* Find the default cities from the database */

    local_city = find_city(settings.local, city_head);
    remote_city = find_city(settings.remote, city_head);

    if (!GrOpen())
	exit(-1);

    /* Load the image */
    g_image = load_image(settings.path, IMAGE_FILE);

    if (!g_image) {
	printf("Error - couldn't make the image\n");
	return -1;
    }

    /* Make a pixmap to hold the temporary map */
    g_pixmap = GrNewPixmap(WIDTH, HEIGHT, 0);

    /* Make the main window - this will provide us with the map in the upper top */
    /* FIXME:  The colors should closely mirror the rest of Pixil */

    g_wid = GrNewWindowEx(WM_PROPS, "World Clock",
			  GR_ROOT_WINDOW_ID, 0, 0, WIDTH, HEIGHT + 55,
			  0xFFFFFFFF);

    GrSelectEvents(g_wid, GR_EVENT_MASK_CLOSE_REQ);

    g_local = GrNewWindow(g_wid, 0, HEIGHT, 120, 55,
			  0, GR_COLOR_WHITE, GR_COLOR_WHITE);

    GrSelectEvents(g_local, GR_EVENT_MASK_EXPOSURE);

    g_map = GrNewWindow(g_wid, 0, 0, WIDTH, HEIGHT,
			0, GR_COLOR_BLACK, GR_COLOR_BLACK);

    GrSelectEvents(g_map, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);

    g_remote = GrNewWindow(g_wid, 120, HEIGHT, 120, 55,
			   0, GR_COLOR_WHITE, GR_COLOR_WHITE);

    GrSelectEvents(g_remote,
		   GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);

    GrMapWindow(g_map);
    GrMapWindow(g_remote);
    GrMapWindow(g_local);

    GrMapWindow(g_wid);

    while (1) {
	GR_EVENT event;

	/* Update every second, but only update the map every minute */
	GrGetNextEventTimeout(&event, 1000L);

	switch (event.type) {
	case GR_EVENT_TYPE_EXPOSURE:

	    if (event.exposure.wid == g_map) {
		draw_map(g_image, g_pixmap, WIDTH, HEIGHT);
		copy_map(g_map, g_pixmap, WIDTH, HEIGHT);
		draw_cities(g_map);
	    }

	    if (event.exposure.wid == g_local)
		draw_timewin(g_local, "Home", local_city, 0);

	    if (event.exposure.wid == g_remote)
		draw_timewin(g_remote, "Away", remote_city, 1);

	    break;

	case GR_EVENT_TYPE_BUTTON_DOWN:
	    if (event.button.wid == g_map) {

		handle_map(event.button.x, event.button.y);

		draw_timewin(g_remote, "Away", remote_city, 1);
		draw_cities(g_map);
	    }

	    if (event.button.wid == g_remote) {
		if (handle_remote(g_remote, event.button.x, event.button.y)) {
		    draw_timewin(g_remote, "Away", remote_city, 1);
		    draw_cities(g_map);
		}
	    }
	    break;

	case GR_EVENT_TYPE_CLOSE_REQ:
	    GrClose();
	    exit(0);
	    break;

	case GR_EVENT_TYPE_TIMEOUT:

	    draw_time(g_local, local_city);
	    draw_time(g_remote, remote_city);

	    if (seconds++ > 60) {
		draw_map(g_image, g_pixmap, WIDTH, HEIGHT);
		copy_map(g_map, g_pixmap, WIDTH, HEIGHT);
		draw_cities(g_map);
		seconds = 0;
	    }

	    break;
	}
    }
}
