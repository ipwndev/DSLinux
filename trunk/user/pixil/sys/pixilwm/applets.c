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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>

#include "nanowm.h"
#include "config.h"
#include "applets.h"

#ifdef STATIC_LINK
extern int date_applet_init(int id, int *x, int y, int h);
extern int date_applet_close(void);
extern int backlight_applet_init(int id, int *x, int y, int h);
extern int backlight_applet_close(void);
extern int battery_applet_init(int id, int *x, int y, int h);
extern int battery_applet_close(void);
#define dlclose(x) 0
#endif

static int g_applet_id = 0;
static int g_timer_id = 0;

static int applet_x, applet_y, applet_h;

static applet_t *applet_list = 0;
static applet_events_t *applet_events = 0;
static icon_applet_t *icon_applets = 0;
static applet_timer_t *applet_timers = 0;

void wm_init_applets(void) {
  GR_SCREEN_INFO	si;
  
  GrGetScreenInfo(&si);

  applet_x = 5;  
  applet_y = si.ws_height + 4; 

  applet_h = 13;
}

void wm_applet_handle_event(GR_EVENT *event) {
  GR_WINDOW_ID wid = ((GR_EVENT_GENERAL *) event)->wid;
  applet_events_t *e = applet_events;

  for( ; e; e = e->next) {
    if (e->wid != wid) continue;
    
    if (e->events & GR_EVENTMASK(event->type)) {
      if (e->cb) e->cb(wid, event);
      break;
    }
  }
}

unsigned long wm_applet_handle_timer(unsigned long elapsed) {
  
  applet_timer_t *timer = applet_timers;
  applet_timer_t *prev = 0, *newhead = 0, *newtail = 0;

  if (!timer) return 0;

  while(timer) {
    applet_timer_t *next = timer->next;

    timer->remain -= elapsed;

    if (timer->remain <= 0) {
      if (timer->callback) timer->callback(); /* Fire the callback */
      
      /* Remove this item from the list */
      
      if (!prev) applet_timers = timer->next;
      else prev->next = timer->next;
      
      /* If this item is periodic, then add it to the bottom */
      /* of the new list */

      /* Otherwise, get rid of the record */

      if (timer->type == APPLET_TIMER_PERIODIC) {
	timer->remain = timer->period;
	timer->next = 0;

	if (!newhead) newhead = newtail = timer;
	else {
	  newtail->next = timer;
	  newtail = timer;
	}
      }
      else free(timer);
    } else prev = timer;
    
    timer = next;
  }
  
  if (!applet_timers) applet_timers = newhead;
  else prev->next = newhead;
  
  if (applet_timers) return applet_timers->remain;
  return 0;
}

unsigned long wm_applet_get_timeout(void) {
  return applet_timers ? applet_timers->remain : 0;
}

void wm_applet_del_timer(int applet_id, int timer_id) {
  applet_timer_t *t = applet_timers;
  applet_timer_t *p = 0;

  while(t) {
    applet_timer_t *n = t->next;

    /* Delete the timer, or all the timers for this applet if timer_id is zero */

    if ( (timer_id && t->id == timer_id) || (!timer_id && t->applet_id == applet_id)) {
      if (!p) applet_timers = t->next;
      else p->next = t->next;
      
      free(t);
      if (timer_id) return;
    } else p = t;
	 
    t = n;
  }
}

int wm_applet_add_timer(int applet_id, int type, unsigned long length, applet_timeout_callback cb) {
  
  applet_timer_t *timer = (applet_timer_t *) calloc(1, sizeof(applet_timer_t));

  timer->applet_id = applet_id;
  timer->period = timer->remain = length;
  timer->type = type;
  timer->id = g_timer_id++;
  timer->callback = cb;

  if (!applet_timers) applet_timers = timer;

  else {
    applet_timer_t *t = applet_timers;
    for( ; t->next; t = t->next);
    t->next = timer;
  }

  return timer->id;
}


/* This is a call for a applet to register its window ID plus a list
   of events that it cares about 
*/

int wm_applet_register(int applet_id, GR_WINDOW_ID id, unsigned long events,
		       applet_event_callback cb) {
  
  applet_events_t *e = applet_events;
  applet_events_t *p = 0;

  for( ; e; e = e->next) {
    if (e->wid != id) continue;

    /* if the bit mask is zero, then erase this record */

    if (events == 0) {
      if (!p) applet_events = e->next;
      else p->next = e->next;
      free(e);
      
      return 0;
    }
    
    /* Otherwise, change the events to reflect the new list */
    e->events = events;

    GrSelectEvents(id, events);
    return 0;
  }

  /* Don't let us add a zero event record */
  if (!events) return 0; 

  /* No record was found, so add one */
  e = (applet_events_t *) calloc(1, sizeof(applet_events_t));
  if (!e) return -1;

  e->wid = id;
  e->applet_id = applet_id;
  e->cb = cb;

  e->events = events;

  if (!applet_events) applet_events = e;
  else {
    applet_events_t *t = applet_events;
    for( ; t->next; t = t->next);
    t->next = e;
  }

  GrSelectEvents(id, events);
  return 0;
}

void wm_applet_unregister(int applet_id, GR_WINDOW_ID gid) {
  wm_applet_register(applet_id, gid, 0, 0);
}

int wm_applet_load(char *filename) {
  
  applet_t *applet = 0;
  if (!filename) return -1;
  
  applet = (applet_t *) calloc(1, sizeof(applet_t));
  if (!applet) return -1;

#ifndef STATIC_LINK
  applet->handle = dlopen(filename, RTLD_LAZY | RTLD_GLOBAL);

  if (!applet->handle) {
    printf("Unable to load applet [%s] [%s]\n", filename, dlerror());
    goto load_error;
  }
#endif

  /* Get the hooks for the applet */
  
#ifdef STATIC_LINK
  /* This is an ugly hack */
  if (strcmp(filename, "date") == 0) {
	applet->init = date_applet_init;
	applet->close = date_applet_close;
  }
  if (strcmp(filename, "backlight") == 0) {
	applet->init = backlight_applet_init;
	applet->close = backlight_applet_close;
  }
  if (strcmp(filename, "battery") == 0) {
	applet->init = battery_applet_init;
	applet->close = battery_applet_close;
  }
#else
  applet->init = (int (*)(int, int *, int, int)) 
    dlsym(applet->handle, "applet_init");

  applet->close = (int (*)(void)) 
    dlsym(applet->handle, "applet_close");
#endif

  /* Set the applet id */
  applet->applet_id = g_applet_id++;

  /* Initalize the applet */
  if (!applet->init) 
    goto load_error;
  if (applet->init(applet->applet_id, &applet_x, applet_y, applet_h))
    goto load_error;

  /* Insert some padding */
  applet_x += 5;

  /* Finally, insert it into the list of applets */

  if (!applet_list) applet_list = applet;

  else {
    applet_t *a = applet_list;
    for( ; a->next; a = a->next);
    a->next = applet;
  }

  return 0;
  
 load_error:
  printf("Error loading the plugin [%s], bailing!\n", filename);

  if (applet->handle) dlclose(applet->handle);
  free(applet);

  return -1;
}

/*** Utility functions ***/

/* These are common functions that can be defined once and used many times */

/* This creates a icon applet (ie, a picture that gets pressed) */
/* and calls a callback when the icon is pressed */

static void icon_applet_draw(icon_applet_t *icon) {

  GR_GC_ID gc = GrNewGC();
  
  GrDrawImageToFit(icon->wid, gc, 0, 0, -1, -1, icon->image);
  GrDestroyGC(gc);
}

 void icon_applet_callback(GR_WINDOW_ID wid, GR_EVENT *event) {
  
  icon_applet_t *i = icon_applets;

  for( ; i; i = i->next) {
    if (i->wid != wid) continue;
    
    if (event->type == GR_EVENT_TYPE_BUTTON_DOWN) {
      if (i->callback) i->callback();
    }
    else if (event->type == GR_EVENT_TYPE_EXPOSURE) {
      icon_applet_draw(i);
    }
    return;
  }
}
  
GR_WINDOW_ID wm_create_icon_applet(int id, int x, int y, int w, int h, char *filename, 
				   void (*callback)(void)) {
  
  icon_applet_t *icon = (icon_applet_t *) calloc(1, sizeof(icon_applet_t));
  if (!icon) return 0;

  if (!icon->image) icon->image = loadIconImage(filename, 0, 0);
  if (!icon->image) goto exit_icon;

  icon->applet_id = id;
  icon->callback = callback;

  icon->wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, 0, GR_ROOT_WINDOW_ID,
			    x, y, w, h, 0xFFFFFF);

  if (!icon->wid) goto exit_icon;

  if (!icon_applets) icon_applets = icon;

  else {
    icon_applet_t *i = icon_applets;
    for( ; i->next; i = i->next);
    i->next = icon;
  }

  wm_applet_register(id, icon->wid, GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_EXPOSURE,
		     icon_applet_callback);

  GrMapWindow(icon->wid);
  return(icon->wid);

 exit_icon:
  if (icon->image) GrFreeImage(icon->image);
  free(icon);

  return 0;
}

void wm_close_icon_applet(int id) {

  icon_applet_t *i = icon_applets;
  icon_applet_t *p = 0;

  while(i) {
    icon_applet_t *n = i->next;
    if (i->applet_id == id) {

      if (!p) icon_applets = i->next;
      else p->next = i->next;

      if (i->image) GrFreeImage(i->image);
      if (i->wid) GrDestroyWindow(i->wid);

      free(i);
      
    }
    else p = i;
    i = n;
  }
}
