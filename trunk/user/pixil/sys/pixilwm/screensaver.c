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

#include <nano-X.h>
#include <nxcolors.h>

#include "powerman.h"

static int g_active = 0;
#ifndef CONFIG_PIXILWM_PM
static GR_WINDOW_ID swindow = 0;

static void draw_screensaver(void) {
  GR_SCREEN_INFO si;
  GR_GC_ID gc = GrNewGC();

  GrGetScreenInfo(&si);

  GrSetGCForeground(gc, GR_COLOR_BLACK);
  GrFillRect(swindow, gc, 0, 0, si.cols, si.rows);
  
  GrDestroyGC(gc);
}

static int handle_screensaver(void) {
  GR_EVENT event;
  GrGetNextEvent(&event);

  switch(event.type) {
  case GR_EVENT_TYPE_EXPOSURE:
    if (event.exposure.wid == swindow) draw_screensaver();
    break;
  case GR_EVENT_TYPE_SCREENSAVER:
    if (!event.screensaver.activate) return 1;
  }

  return 0;
}

/* Why make a window and raise it - it was either that or copy off
   the root window, suspend all updates, and blank it.  I just think
   this was an easier way to go about it.
*/

static void do_screensaver(void) {
  GR_SCREEN_INFO si;

  GrGetScreenInfo(&si);
  
  if (!swindow)
    swindow = GrNewWindowEx(GR_WM_PROPS_NODECORATE, 0,
			    GR_ROOT_WINDOW_ID, 0, 0, si.cols, si.rows, 
			    GR_COLOR_BLACK);			
  
  GrSelectEvents(swindow, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_SCREENSAVER);

  /* Turn off the backlight if we can */
  pm_backlight(0);

  GrRaiseWindow(swindow);  /* Raise it to the top */
  GrMapWindow(swindow);

  while(!handle_screensaver());
  GrUnmapWindow(swindow);

  /* Turn back on the backlight */
  pm_backlight(1);
}
#endif

void screensaver_init(void) {
	
#ifdef CONFIG_PIXILWM_PM
  pm_init();   /* Do the power management bit */
#else  
  GrSetScreenSaverTimeout(30); /* Set the screensaver timeout */
#endif
}

int screensaver_active(void) { return g_active; }

void screensaver_enable(void) {

  if (g_active) return;
  
#ifdef CONFIG_PIXILWM_PM
  pm_bltimer_off();
  g_active = 1;
#else
  do_screensaver();
#endif
}

void screensaver_disable(void) {
  
  if (!g_active) return;
  
#ifdef CONFIG_PIXILWM_PM
  pm_bltimer_on();
  g_active = 0;
#endif
}

/* Ugly, I know - but this is the best way to do this, I think */

#ifndef CONFIG_PIXILWM_PM

#include <pixlib/pixlib.h>

void pm_backlight(int mode) {

  /* FIXME:  We need a get current level */

  int max = pix_bl_getmxval();
  if (max == -1) return;

  if (mode) 
    pix_bl_ctrl(mode, 0);
  else
    pix_bl_ctrl(mode, max);
}

#endif
