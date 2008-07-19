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
 * Use, duplication, or disc1losure by the government is subject to      
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
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <nano-X.h>

#include "nanowm.h"
#include "apps.h"
#include "categories.h"
#include "applets.h"

#ifdef CONFIG_PIXILWM_MENUS
#include "sys_menu.h"
#endif

#ifdef CONFIG_PIXILWM_PM
#include "powerman.h"
#endif

#include "screensaver.h"

static void scrtop_fire_timer(GR_TIMER_ID id);
static void mainloop(void);

GR_TIMER_ID pwr_timer_id = 0;
char *path = "/tmp/nxscrtop.pid";

/*
 * This the universal, get-outta here function that closes everything down and bails.  
 */

void
close_nxscrtop(int id)
{
    win *window;

    warning("The screentop has been closed.  Freeing resources....\n");

    /* Step 1.  Kill off any outstanding applications that are still running */
    kill_running_processes();

    /* Step 2.  Remove all the windows */
    window = find_window(GR_ROOT_WINDOW_ID);
    remove_window_and_children(window);

    /* Step 3.  Free all the outstanding memory */
    apps_free_memory();
    root_free_memory();
    nxFreeCategories();

    unlink(path);

    exit(0);
}

int
main(int argc, char *argv[])
{

    pid_t pid = 0;
    int pathfd_;
    char buf[512];
    int ret;

    extern void application_handler(int id);

    /* These are death signals, that indicate that we should close up shop */

    signal(SIGINT, close_nxscrtop);
    signal(SIGTERM, close_nxscrtop);
    signal(SIGHUP, close_nxscrtop);

    memset(buf, 0, sizeof(buf));
    pid = getpid();

    if (!access(path, F_OK)) {
	printf("Warnning - found a stale lockfile.  Deleting it...\n");
	unlink(path);
    }

    pathfd_ = open(path, O_RDWR | O_TRUNC | O_CREAT);
    if (pathfd_ == -1) {
	perror("open(): /tmp/nxscrtop.pid");
	exit(errno);
    }
    sprintf(buf, "%d", pid);
    ret = write(pathfd_, buf, strlen(buf));
    if (-1 == ret) {
	perror("write(): pid");
	exit(errno);
    }
    close(pathfd_);

    /* This is the sigchild handler, useful for when our children die */

    signal(SIGCHLD, application_handler);

    if (GrOpen() < 0) {
	error("Couldn't connect to Nano-X server!\n");
	unlink(path);
	exit(-1);
    }

    GrReqShmCmds(65536L);

    startIPC();
    wm_init_applets();

    nxLoadConfig();

    /* pass errors through main loop, don't exit */
    GrSetErrorHandler(NULL);

    root_create();

    /* Start the screen saver / power management */
    screensaver_init();

    mainloop();

    unlink(path);
    return 0;
}

static void
mainloop(void)
{
    unsigned long timeout = wm_applet_get_timeout();

    GR_EVENT event;
    win *window;

    while (1) {
        unsigned long elapsed = 0;
        struct timeval b, a;
    
        gettimeofday(&b, 0);
        GrGetNextEventTimeout(&event, timeout);
        gettimeofday(&a, 0);

	switch (event.type) {

	    /* all these events share their wid member location */
	case GR_EVENT_TYPE_BUTTON_DOWN:
	case GR_EVENT_TYPE_BUTTON_UP:
	case GR_EVENT_TYPE_KEY_DOWN:
	case GR_EVENT_TYPE_KEY_UP:
	case GR_EVENT_TYPE_EXPOSURE:
	case GR_EVENT_TYPE_UPDATE:
	case GR_EVENT_TYPE_MOUSE_POSITION:
	case GR_EVENT_TYPE_MOUSE_ENTER:
	case GR_EVENT_TYPE_MOUSE_EXIT:
	case GR_EVENT_TYPE_FOCUS_IN:
	case GR_EVENT_TYPE_FOCUS_OUT:
#ifdef MICROWIN_PRE8
	case GR_EVENT_TYPE_BUTTON_LONG_CLICK:
#endif

	    window = find_window(((GR_EVENT_GENERAL *) & event)->wid);

#ifdef CONFIG_PIXILWM_MENUS
	    if (system_menu_active() == GR_TRUE) {
		if (handle_system_menu(&event) == 1)
		    break;
	    }
#endif

	    if (window && window->proc)
		window->proc(window, &event);

	    wm_applet_handle_event(&event);
	    break;

	case GR_EVENT_TYPE_FDINPUT:

	    /* We got a FD input, thats probably a colosseum thing */

	    handleIPC(&event.fdinput);
	    break;

	case GR_EVENT_TYPE_CHLD_UPDATE:
	    window = find_window(((GR_EVENT_UPDATE *) & event)->subwid);

	    /* 
	     * Add unknown newly mapped window to window manager
	     * internal data structures.  This will also create
	     * a new parent container window.
	     */
	    if (!window && event.update.utype == GR_UPDATE_MAP) {
		client_updatemap_new(((GR_EVENT_UPDATE *) & event)->subwid);
	    }

	    if (window && window->proc)
		window->proc(window, &event);

	    break;

	case GR_EVENT_TYPE_TIMER:
	  if (event.timer.tid) scrtop_fire_timer(event.timer.tid);
	  break;

	case GR_EVENT_TYPE_TIMEOUT:
	  /* Handled elsewhere */
	  break;
	  
	case GR_EVENT_TYPE_SCREENSAVER:

	    /* Backlight */
	    if (event.screensaver.activate == GR_TRUE)
	      screensaver_enable();
	    else
	      screensaver_disable();
	    
	    break;
	    
	case GR_EVENT_TYPE_ERROR:
	    error("GrError (%s)", event.error.name);
	    error(nxErrorStrings[event.error.code], event.error.id);
	    break;

	default:
	    warning("Got unexpected event %d\n", event.type);
	    break;
	}


	/* Get the number of elapsed milliseconds for the timer */	
	elapsed = 0;
	
	if (b.tv_usec > a.tv_usec) {
	  if ((a.tv_sec - b.tv_sec) > 0) 
	    elapsed = ((a.tv_sec - b.tv_sec) - 1) * 1000;
	  
	  elapsed += ((1000000 - b.tv_usec) + a.tv_usec) / 1000;
	}
	else {
	  elapsed = (a.tv_sec - b.tv_sec) * 1000;
	  elapsed += (a.tv_usec - b.tv_usec) / 1000;
	}
	
	timeout = wm_applet_handle_timer(elapsed);	
    }
}

struct timer_list {
  GR_TIMER_ID id;
  scrtop_timer_cb callback;
  struct timer_list *next;
};

static struct timer_list *scrtop_timer_list = 0;

void scrtop_register_timer(GR_TIMER_ID id, scrtop_timer_cb callback) {
  struct timer_list *a = 0, *p = 0, *i = 0;

  for(a = scrtop_timer_list; a; p = a, a = a->next) 
    if (a->id == id) return;

  i = (struct timer_list *) calloc(1, sizeof(struct timer_list));
  i->id = id;
  i->callback = callback;

  if (!p) scrtop_timer_list = i;
  else p->next = i;

  return;
}

void scrtop_unregister_timer(GR_TIMER_ID id) {
  struct timer_list *a = 0, *p = 0;

  for(a = scrtop_timer_list; a; p = a, a = a->next) 
    if (a->id == id) {
      if (p) p->next = a->next;
      else scrtop_timer_list = a->next;
      free(a);

      return;
    }
}

static void scrtop_fire_timer(GR_TIMER_ID id) {
  struct timer_list *a = 0, *p = 0;
  
  for(a = scrtop_timer_list; a; p = a, a = a->next)
    if (a->id == id) {
      scrtop_timer_cb ptr = a->callback;
      
      if (p) p->next = a->next;
      else scrtop_timer_list = a->next;
      free(a);
      
      if (ptr) ptr();
      return;
    }
}
  
static unsigned char cur_log_level = LOG_WARNING;

void
scrtop_log_message(int level, char *fmt, ...)
{

    va_list ap;

    if (level > cur_log_level)
	return;

    switch (level) {

    case LOG_DEBUG:
	printf("debug: ");
	break;

    case LOG_WARNING:
	printf("Warning: ");
	break;

    case LOG_ERROR:
	printf("Error: ");
	break;
    }

    printf("nxscrtop -- ");

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}
