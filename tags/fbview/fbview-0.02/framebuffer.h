#ifndef ds_framebuffer_h
#define ds_framebuffer_h

#include "types.h"

// 4MB memery limit
#define MEMSIZE (1024*1024*4)

// byte per pixel for FrameBuffer device
#define FB_BPP (sizeof(fb_pixel)*8)

#pragma pack(push)
#pragma pack(1)

#pragma pack(pop)

typedef Uint8 fb_pixel;

extern fb_pixel * screen;
extern Uint8 pixelsize;
extern Uint32 fb_line_size;
extern Uint16 fb_xres;
extern Uint16 fb_yres;

extern int fb_init();
extern int fb_uninit();
extern void fb_setpixel(fb_pixel * dest, Uint8 R, Uint8 G, Uint8 B);
extern void fb_clear_screen(fb_pixel * screen);

#endif
