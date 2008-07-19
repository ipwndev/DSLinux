#ifndef APPLET_H_
#define APPLET_H_

#include <nano-X.h>
#include "config.h"

typedef void (*applet_event_callback)(GR_WINDOW_ID, GR_EVENT *);
typedef void (*applet_timeout_callback)(void);

#define APPLET_TIMER_ONESHOT  0
#define APPLET_TIMER_PERIODIC 1

typedef struct applet_timer {
  int id;

  int applet_id;
  signed long remain;
  unsigned long period;
  applet_timeout_callback callback;

  int type;

  struct applet_timer *next;
} applet_timer_t;

typedef struct applet_event {
  int applet_id;
  GR_WINDOW_ID wid;

  unsigned long events;
  applet_event_callback cb;
  
  struct applet_event *next;
} applet_events_t;

typedef struct icon_applet {
  unsigned long applet_id;

  GR_WINDOW_ID wid;
  GR_IMAGE_ID image;

  void (*callback) (void);
  struct icon_applet *next;
} icon_applet_t;

typedef struct applet {
  int applet_id;
  void *handle;
  
  int (*init)(int, int *, int, int);
  int (*close)(void);

  struct applet *next;
} applet_t;

int wm_applet_add_timer(int applet_id, int type, unsigned long length, applet_timeout_callback cb);

void wm_init_applets(void);
unsigned long wm_applet_get_timeout(void);
void wm_applet_handle_event(GR_EVENT *event);
unsigned long wm_applet_handle_timer(unsigned long elapsed);
int wm_applet_load(char *filename);

int wm_applet_register(int applet_id, GR_WINDOW_ID id, unsigned long events,
		       applet_event_callback cb);

void wm_applet_del_timer(int applet_id, int timer_id);

#endif

