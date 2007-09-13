#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <nano-X.h>

#include <applets.h>

static GR_WINDOW_ID g_wid = 0;
static int g_height = 0;
static int g_applet = 0;

static int loadavg;

struct {
  unsigned long user;
  unsigned long nice;
  unsigned long sys;
  unsigned long idle;
  unsigned char valid;
} cpudata;

static void draw_applet(void);

static int get_load(void)
{
  unsigned long user, nice, sys, idle;
  float total = 0, busy = 0;
  
  char str[BUFSIZ];
  char *c;
  char dummy[3];

  /* Very tricky.  We read the first line from /proc/stat */
  /* and parse it up */
  
  lseek((int) loadavg, 0, SEEK_SET);
  read((int) loadavg, str, BUFSIZ - 1);

  /* Now skip over "cpu" */
  for (c = str; *c != ' '; c++)
    continue;
  c++;

  /* Get the new values */
  
  user = strtoul(c, &c, 0);
  nice = strtoul(c, &c, 0);
  sys = strtoul(c, &c, 0);
  idle = strtoul(c, &c, 0);
  
  /* Get the delta with the old values */
  if (cpudata.valid) {
    unsigned long duser, dnice, dsys, didle;
    
    duser = abs(user - cpudata.user);
    dnice = abs(nice - cpudata.nice);
    dsys = abs(sys - cpudata.sys);
    didle = abs(idle - cpudata.idle);
    
    busy = (float) duser + dnice + dsys;
    total = (float) busy + didle;
  } else
    total = 0;
  
  /* And fill up the struct with the new values */
  cpudata.user = user;
  cpudata.nice = nice;
  cpudata.sys = sys;
  cpudata.idle = idle;
  cpudata.valid = 1;
  
  if (total == 0)
    return (0);
  
  /* Return the % of cpu use */
  return (busy * 100) / total;
}

static int vals[30];
static int ptr = 0;

static void timeout_callback(void) {
  vals[ptr] = get_load();
  ptr = (ptr + 1 == 30) ? 0 : ptr + 1;
  draw_applet();
}

#define START_RED 170
#define START_GREEN 170
#define START_BLUE 170

#define END_RED 0
#define END_GREEN 0x33
#define END_BLUE  0x80
 
static void draw_applet(void) {
  GR_GC_ID gc=GrNewGC();

  int start = (ptr + 1 == 30) ? 0 : ptr + 1;
  int x = 1;
  int r = START_RED, g = START_GREEN, b = START_BLUE;

  while(start != ptr) {
    int i = (vals[start] * g_height) / 100;
	
    GrSetGCForeground(gc, MWRGB(r, g, b));

    if (i)
      GrLine(g_wid, gc, x, g_height - i, x, g_height);
    
    GrSetGCForeground(gc, MWRGB(0xFF, 0xFF, 0xFF));
    GrLine(g_wid, gc, x, 0, x, (g_height - i));
    
    x += 1;
    if (x < 15) {
      r -= (START_RED - END_RED) / 15;  
      g -= (START_GREEN - END_GREEN) / 15; 
      b -= (START_BLUE - END_BLUE) / 15; 
    }
    else {
      r += (START_RED - END_RED) / 15;
      g += (START_GREEN - END_GREEN) / 15;
      b += (START_BLUE - END_BLUE) / 15;
    }

    start = (start + 1 == 30) ? 0 : start + 1;
  }

  GrDestroyGC(gc);
}

static void event_callback(GR_WINDOW_ID wid, GR_EVENT *event) {
  draw_applet();
}

int applet_init(int id, int *x, int y, int h) {

  int ret;  
  g_applet = id;

  loadavg = open("/proc/stat", O_RDONLY);
  if (loadavg == -1) {
    printf("oops\n");
    return -1;
  }

  g_wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, 0, GR_ROOT_WINDOW_ID,
			*x, y, 31, h, 0xFFFFFF);
 
  wm_applet_register(id, g_wid, GR_EVENT_MASK_EXPOSURE, event_callback);

  wm_applet_add_timer(id, APPLET_TIMER_PERIODIC, 1000, timeout_callback);

  GrMapWindow(g_wid);

  *x += 30;

  if (ret) *x += 13;
  g_height = h - 2;

  return ret ? 0 : -1;
}

int applet_close(void) {
  wm_applet_del_timer(g_applet, 0);
}

