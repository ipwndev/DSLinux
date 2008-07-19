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

#include <string.h>
#include <stdlib.h>
#include <nano-X.h>
#include <nanowm.h>

#include <pixlib/pixlib.h>
#ifdef CONFIG_PAR
#include <par/par.h>
#endif

#include "powerman.h"

static GR_TIMER_ID pm_timer_id = 0;

struct {
  struct {
    int power;
    int bl_timeout;
    int bl_level;
    int bl_set;
    int power_timeout;
  } ac, bat;
} settings;

static int pm_bl_state = 1;

static void pm_do_callback(int type, void *data);

GR_TIMER_ID pm_get_timer_id(void) { return  pm_timer_id; }

#if 0
static void set_pm_timer(int timeout) {
  if (pm_timer_id) return;
  pm_timer_id = GrCreateTimer(1, timeout);
  scrtop_register_timer(pm_timer_id, pm_suspend);
}
#endif

static void stop_bl_timer(void) {
  GrSetScreenSaverTimeout(0);
}

/* We have two levels of timers.  The first level is the backlight,
   triggered after N seconds of inactivity (aka the SCREENSAVER) 

   The second is the power managment timer, which is started after
   the screensaver fires.  The interesting here is if the 
*/

static void pm_first_timer(void) {
  
  int state = pix_pwr_onBattery();
 int timeout = 0;

  if (state == AC_ON) {    
    if (settings.ac.bl_timeout > 0)
      timeout = settings.ac.bl_timeout;
    else if (settings.ac.power && settings.ac.power_timeout > 0)
      timeout = settings.ac.power_timeout;
  }
  else if (state == BAT_ON) {
    if (settings.ac.bl_timeout > 0)
      timeout = settings.bat.bl_timeout;
    else if (settings.bat.power && settings.bat.power_timeout > 0)
      timeout = settings.bat.power_timeout;
  }

  GrSetScreenSaverTimeout(timeout);
}

static void pm_second_timer(void) {
  
  int state = pix_pwr_onBattery();
  int timeout = -1;

  if (pm_timer_id) return;

  if (state == AC_ON) {
    if (settings.ac.power && settings.ac.power_timeout > 0) {
      timeout = settings.ac.power_timeout;
      timeout -= (settings.ac.bl_timeout > 0) ? settings.ac.bl_timeout : 0;
    }
  }
  else if (state == BAT_ON) {
    if (settings.bat.power && settings.bat.power_timeout > 0) {
      timeout = settings.bat.power_timeout;
      timeout -= (settings.bat.bl_timeout > 0) ? settings.bat.bl_timeout : 0;
    }
  }

  /* If no timeout, then don't bother */
  /* If the timeout is zero, then suspend immediately */
  /* Otherwise, set the timer to fire at the appropriate time */

  if (timeout == -1) return;
  else if (timeout == 0) 
    pm_suspend();
  else
    pm_timer_id = GrCreateTimer(1, timeout);
}
    
static void pm_stop_timer(void) {
  if (pm_timer_id) GrDestroyTimer(pm_timer_id);
  scrtop_unregister_timer(pm_timer_id);

  pm_timer_id = 0;

  GrSetScreenSaverTimeout(0);
}

static void
pm_read_settings(void) {

  db_handle *db;
  db = db_openDB(db_getDefaultDB(), PAR_DB_MODE_RDONLY);
  if (!db) return;
 
  memset(&settings, 0, sizeof(settings));

  par_getGlobalPref(db, "power", "ac_off", PAR_BOOL, 
		    &settings.ac.power, sizeof(settings.ac.power));
  par_getGlobalPref(db, "power", "ac_offval", PAR_INT, 
		    &settings.ac.power_timeout, sizeof(settings.ac.power_timeout));

  par_getGlobalPref(db, "backlight", "ac_wakeup", PAR_BOOL, 
		    &settings.ac.bl_set, sizeof(settings.ac.bl_set));

  par_getGlobalPref(db, "backlight", "ac_timeout", PAR_INT, 
		    &settings.ac.bl_timeout, sizeof(settings.ac.bl_timeout));

  par_getGlobalPref(db, "backlight", "ac_level", PAR_INT, 
		    &settings.ac.bl_level, sizeof(settings.ac.bl_level));

  par_getGlobalPref(db, "power", "bat_off", PAR_BOOL, 
		    &settings.bat.power, sizeof(settings.bat.power));
  par_getGlobalPref(db, "power", "bat_offval", PAR_INT, 
		    &settings.bat.power_timeout, sizeof(settings.bat.power_timeout));

  par_getGlobalPref(db, "backlight", "bat_wakeup", PAR_BOOL, 
		    &settings.bat.bl_set, sizeof(settings.bat.bl_set));

  par_getGlobalPref(db, "backlight", "bat_timeout", PAR_INT, 
		    &settings.bat.bl_timeout, sizeof(settings.bat.bl_timeout));
  par_getGlobalPref(db, "backlight", "bat_level", PAR_INT, 
		    &settings.bat.bl_level, sizeof(settings.bat.bl_level));

  db_closeDB(db);
}

/* Called to reload the settings and restart the timers */

void pm_reload(void) {
  pm_stop_timer();     /* Stop both timers */
  pm_read_settings();
  pm_first_timer();    /* Set up the first tier timer */
}

void 
pm_init(void) {  
  pm_read_settings();
  pm_first_timer();   /* Set up the first tier timer */

  /* Start off in the default state */
  pm_backlight(pm_bl_state);
}

void
pm_suspend(void) {    

  printf("Suspending the screentop - see you on the flip side\n");
  pm_stop_timer();    /* Stop both timers */

  pix_pwr_suspend();   /* At this point the power should be suspended */  
  pm_first_timer();   /* Start the first tier timer again */
}

void pm_backlight(int mode) {

  int state = pix_pwr_onBattery();
  int level, blto, max, val;

  switch(state) {
  case AC_ON:
    level = settings.ac.bl_level;
    blto = settings.ac.bl_timeout;
    break;

  case BAT_ON:
    level = settings.bat.bl_level;
    blto = settings.ac.bl_timeout;
    break;

  default:  /* No APM or error */
    goto do_callback;
  }
    
  max = pix_bl_getmxval();
  val = (((level * max) << 4) / 100) >> 4;
  
  if (mode == BL_ON) { 
    if (blto > 0) pm_first_timer();   
  }
  else if (mode == BL_OFF) {
    stop_bl_timer();
  }

  /* Set the value */
  pix_bl_ctrl(mode, val);

 do_callback:
  pm_bl_state = mode;
  pm_do_callback(PM_CALLBACK_BL, (void *) mode);
}

void pm_bl_toggle(void) {
  pm_backlight(!pm_bl_state);
}

int pm_get_bl_state(void) { return pm_bl_state; }
  
/* Fired when the screen saver timer expires */

void pm_bltimer_off(void) {

  int state = pix_pwr_onBattery();

  if (state == -1) return;

  if (state == AC_ON && settings.ac.bl_timeout <= 0)
      return;

  if (state == BAT_ON && settings.bat.bl_timeout <= 0)
    return;
  
  pm_backlight(0);  /* Turn off the backlight */

  pm_second_timer();   /* Set up the second tier timer */
}

/* Fired when the screen saver timer returns */

void pm_bltimer_on(void) {

  int state = pix_pwr_onBattery();

  /* Stop the second tier timer */

  if (pm_timer_id) {
    GrDestroyTimer(pm_timer_id);
    scrtop_unregister_timer(pm_timer_id);

    pm_timer_id = 0;
  }

  if (state == -1) return;

  if (pm_get_bl_state()) return;  /* Its already on */

  if (state == AC_ON && !settings.ac.bl_set)
    return; 

   if (state == AC_ON && !settings.bat.bl_set)
     return;

   pm_backlight(1);  /* Turn it back on */
}

struct pm_cb_list {
  pm_callback cb;
  struct pm_cb_list *next;
};

struct pm_cb_list *cblist[1] = {0};

/* Set up a function to be called when a particular power management event happens */

void pm_register_callback(int type, pm_callback cb) {
  struct pm_cb_list *l = (struct pm_cb_list *) calloc(1, sizeof(struct pm_cb_list));
  l->cb = cb;

  if (!cblist[type])
    cblist[type] = l;
  else {
    struct pm_cb_list *a = cblist[type];
    for(; a->next; a=a->next);
    a->next = l;
  }
}

void pm_unregister_callback(int type, pm_callback cb) {
  struct pm_cb_list *l = cblist[type];
  struct pm_cb_list *prev = 0;

  for(; l; l = l->next) {
    if (l->cb == cb) {
      if (prev) prev->next = l->next;
      else cblist[type] = l->next;

      free(l);
      return;
    }
    
    prev = l;
  }
}

/* Actually do the callbacks based on the given type and the given data */

static void pm_do_callback(int type, void *data) {
  struct pm_cb_list *l = cblist[type];
  for(; l; l = l->next)
    if (l->cb) l->cb(type, data);
}


    
    

