/* A version applet */

#include <stdio.h>

#include <nano-X.h>
#include <applets.h>

#define TERMICON "term.gif"

static int myid = 0;

void draw_version(GR_WINDOW_ID wid) {
  GR_WINDOW_INFO info;

  GR_GC_ID gc = GrNewGC();  
  GR_FONT_ID font = GrCreateFont(GR_FONT_GUI_VAR, 0, NULL);
 
  GrSetGCFont(gc, font);

  GrGetWindowInfo(wid, &info);

  GrSetGCForeground(gc, MWRGB(0x00, 0x66, 0xcc));
  GrSetGCBackground(gc, MWRGB(0x00, 0x66, 0xcc));

  GrFillRect(wid, gc, 0, 0, info.width, info.height);
  
  GrSetGCForeground(gc, MWRGB(255,255,255));
  GrRect(wid, gc, 2, 2, info.width - 4, info.height - 4);


  GrText(wid, gc, 5, 5, "Pixil Window Manager", -1, GR_TFASCII | GR_TFTOP);
  GrText(wid, gc, 5, 20, "Version 0.1", -1, GR_TFASCII | GR_TFTOP);

  GrText(wid, gc, 5, 35, "Press button to close", -1, GR_TFASCII | GR_TFTOP);

  GrDestroyFont(font);
  GrDestroyGC(gc);
}

static void show_version(void) {
  
  GR_WINDOW_ID    wid;
  GR_SCREEN_INFO  si;

  GrGetScreenInfo(&si);

  wid = GrNewWindowEx(GR_WM_PROPS_NODECORATE, NULL, GR_ROOT_WINDOW_ID,
		      (si.cols - 200) / 2 , (si.rows - 55) / 2, 
		      200, 55, 0xFFFFFF);
  
  if (!wid) return;

  GrSelectEvents(wid, GR_EVENT_MASK_EXPOSURE | GR_EVENT_MASK_BUTTON_DOWN);
  GrMapWindow(wid);

  while(1) {
    GR_EVENT event;
    GrGetNextEvent(&event);

    if (((GR_EVENT_GENERAL *) &event)->wid != wid) continue;
    
    if (event.type == GR_EVENT_TYPE_EXPOSURE) draw_version(wid);
    else break;
  }

  GrDestroyWindow(wid);
}

/* Entry points */

int applet_init(int id, int *x, int y, int h) {

  int ret;
  
  myid = id;
  
  ret = wm_create_icon_applet(id, *x, y, 13, h, TERMICON, show_version);

  if (ret) *x += 13;

  return ret ? 0 : -1;
}

int applet_close(void) {
  wm_close_icon_applet(myid);
}

  
