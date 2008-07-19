#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include <nano-X.h>

#include "nanowm.h"
#include "applets.h"

#ifdef STATIC_LINK
#define applet_init date_applet_init
#define applet_close date_applet_close
#endif

static GR_WINDOW_ID wid;
static GR_FONT_ID fontid;

static int g_w = 0, g_h = 0;
static int dtoggle = 0;
static int applet_id;
static int lastminute = -1;

static void draw_date(int periodic) {
  char buffer[256];
  time_t t = time(0);
  struct tm *tv = localtime(&t);

  if (periodic && (tv->tm_min == lastminute))
	return;
  lastminute = tv->tm_min;

  GR_GC_ID gc = GrNewGC();
  if (!fontid) fontid = GrCreateFont(GR_FONT_GUI_VAR, 0, NULL);

  GrSetGCForeground(gc, wm_getColor(WM_TASKBAR));
  GrFillRect(wid, gc, 0, 0, g_w, g_h);

  if (dtoggle)
    strftime(buffer, sizeof(buffer) - 1, "%m/%d/%y", tv);
  else
    strftime(buffer, sizeof(buffer) - 1, "%I:%M %p\n", tv);

  GrSetGCBackground(gc, wm_getColor(WM_TASKBAR));
  GrSetGCForeground(gc, wm_getColor(WM_BGCOLOR));
  GrSetGCFont(gc, fontid);
  
  GrText(wid, gc, 0, 0, buffer, -1, GR_TFTOP);
  GrDestroyGC(gc);
}

static void event_callback(GR_WINDOW_ID wid, GR_EVENT *event) {

  switch(event->type) {
  case GR_EVENT_TYPE_BUTTON_DOWN:
    dtoggle = 1;
    break;

  case GR_EVENT_TYPE_BUTTON_UP:
    dtoggle = 0;
    break;
  }

  draw_date(0);
}

static void timeout_callback(void) {
  draw_date(1);
}

int applet_init(int id, int *x, int y, int h) {

  int tid;

  applet_id = id;

  wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, 0, GR_ROOT_WINDOW_ID,
		      *x, y, 50, h, wm_getColor(WM_TASKBAR));

  if (!wid) return -1;

  wm_applet_register(id, wid, 
		     GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP | 
		     GR_EVENT_MASK_EXPOSURE, event_callback);

  tid = wm_applet_add_timer(id, APPLET_TIMER_PERIODIC, 1000, 
			    timeout_callback);
  
  GrMapWindow(wid);

  g_w = 50;
  g_h = h;
  
  *x += 50;

  return 0;
}

int applet_close(void) {
  wm_applet_del_timer(applet_id, 0);
  return 0;
}
  


