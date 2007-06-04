/*////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998 Olivier Debon
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
///////////////////////////////////////////////////////////////
//  Author : Olivier Debon  <odebon@club-internet.fr>
*/  

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "flash.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include "vroot.h"

static char *rcsid = "$Id$";

typedef struct {
    FlashDisplay fd;
    Display			*dpy;		/* X11 Display */
    Window			 target;	/* Target window */
    GC		 	 gc;			/* X11 Graphic context */
    Pixmap			 canvas;	/* Graphic buffer */
    long			 xOffset, yOffset;
} X11Context;

X11Context xc1, *xc=&xc1;

int shape_size,shape_nb,shaperecord_size,shaperecord_nb,style_size,style_nb;

Display *dpy;
GC gc;
Window frame,movie,control;
struct FlashInfo fi;
char *filename;

/* memory function for the plugin */

void *flash_malloc(unsigned long size)
{
    return malloc(size);
}

void flash_free(void *p)
{
    free(p);
}

void *flash_realloc(void *p, unsigned long size)
{
    return realloc(p, size);
}

#define FLASH_XEVENT_MASK (ExposureMask|ButtonReleaseMask|ButtonPressMask|PointerMotionMask)

long FlashExecX11(FlashHandle fh, long flag, XEvent *event, struct timeval *wakeDate)
{
    FlashEvent fe;

    if (flag & FLASH_EVENT) {
        /* X to Flash event structure conversion */
        switch (event->type) {
        case ButtonPress:
            fe.type = FeButtonPress;
            break;
        case ButtonRelease:
            fe.type = FeButtonRelease;
            break;
        case MotionNotify:
            fe.type = FeMouseMove;
            fe.x = event->xmotion.x;
            fe.y = event->xmotion.y;
            break;
        case Expose:
            fe.type = FeRefresh;
            break;
        default:
            fe.type = FeNone;
            break;
        }
    }
    
    return FlashExec(fh,flag,&fe,wakeDate);
}

long FlashGraphicInitX11(FlashHandle fh, Display *d, Window w, int onRoot)
{
    XWindowAttributes wattr;
    XPixmapFormatValues *pf;
    Visual *visual;
    int nItems;
    int n;
    struct shmid_ds buf;
    int			 	 targetWidth;
    int 			 targetHeight;
    long			 bpl;	/* Bytes per line */
    long			 bpp;	/* Bytes per pixel */
    long			 pad;	/* Scanline pad in byte */
    /* Platform dependent members */
    Window			 target;	/* Target window */
    Cursor		 	 buttonCursor;	/* Window cursor (a hand if over a button) */
    Display			*dpy;		/* X11 Display */
    GC		 	 gc;		/* X11 Graphic context */
    Pixmap			 canvas;	/* Graphic buffer */
    XShmSegmentInfo		 segInfo;	/* Shared memory information */

    dpy = d;
    target = w;

    /* Get Window dimension */
    XGetWindowAttributes(dpy, target, &wattr);

    /* Get first visual, don't care about others, really ! */
    visual = wattr.visual;

#define PRINT 0
#if PRINT
	fprintf(stderr,"Id: %x\n", target);
	fprintf(stderr,"VisualId: %x\n", visual->visualid);
	fprintf(stderr,"BitmapPad  = %d\n", BitmapPad(dpy));
	fprintf(stderr,"BitmapUnit = %d\n", BitmapUnit(dpy));
	fprintf(stderr,"Depth      = %d\n", DefaultDepth(dpy,DefaultScreen(dpy)));
	fprintf(stderr,"RedMask    = %x\n", visual->red_mask);
	fprintf(stderr,"GreenMask  = %x\n", visual->green_mask);
	fprintf(stderr,"BlueMask   = %x\n", visual->blue_mask);
	fprintf(stderr,"Bits/RGB   = %d\n", visual->bits_per_rgb);
#endif

	bpp = 0;

	/* Get screen info */

	for(pf=XListPixmapFormats(dpy, &n); n--; pf++) {
		if (pf->depth == DefaultDepth(dpy, DefaultScreen(dpy))) {
			bpp = pf->bits_per_pixel/8;
			pad = pf->scanline_pad/8;
		}
#if PRINT
		fprintf(stderr,"----------------\n");
		fprintf(stderr,"Depth          = %d\n", pf->depth);
		fprintf(stderr,"Bits Per Pixel = %d\n", pf->bits_per_pixel);
		fprintf(stderr,"Scanline Pad   = %d\n", pf->scanline_pad);
#endif
	}

	gc = DefaultGC(dpy, DefaultScreen(dpy));

	if (onRoot) {
		targetWidth = fi.frameWidth/20;
		targetHeight = fi.frameHeight/20;
		xc->xOffset = (wattr.width-targetWidth)/2;
		xc->yOffset = (wattr.height-targetHeight)/2;
	} else {
		targetWidth = wattr.width;
		targetHeight = wattr.height;
		xc->xOffset = 0;
		xc->yOffset = 0;
	}

#if PRINT
	fprintf(stderr,"Target Width  = %d\n", wattr.width);
	fprintf(stderr,"Target Height = %d\n", wattr.height);
#endif

	if (bpp) {
		bpl = (targetWidth*bpp + pad-1)/pad*pad;
	} else {
		bpl = (targetWidth/8 + pad-1)/pad*pad;
	}

	if (!onRoot) {
		XSelectInput(dpy, target, ExposureMask|ButtonReleaseMask|ButtonPressMask|PointerMotionMask);
	}

	/* Prepare data for Direct Graphics */
	segInfo.readOnly = False;
	segInfo.shmid = shmget (IPC_PRIVATE,targetHeight*bpl,IPC_CREAT|0777);
	if (segInfo.shmid <0) {
		perror("shmget");
		fprintf(stderr,"Size = %d x %d\n", targetWidth, targetHeight);
	}
	segInfo.shmaddr = (char*)shmat (segInfo.shmid, 0, 0);
	if ((long)segInfo.shmaddr == -1) {
		perror("shmat");
	}
	XShmAttach(dpy, &segInfo);
	XSync(dpy, False);

	/* When number of attached clients falls down to zero */
	/* the shm is removed. This is convenient when it crashes. */
	if (shmctl(segInfo.shmid, IPC_RMID, &buf) < 0) {
		perror("shmctl");
	}

	xc->fd.pixels = (char*)segInfo.shmaddr;
        xc->fd.width = targetWidth;
        xc->fd.height = targetHeight;
        xc->fd.bpl = bpl;
        xc->fd.depth = DefaultDepth(dpy, DefaultScreen(dpy));
        xc->fd.bpp = bpp;

	canvas = XShmCreatePixmap(dpy,target,segInfo.shmaddr,&segInfo,targetWidth,targetHeight,DefaultDepth(dpy, DefaultScreen(dpy)));
	XSync(dpy, False);

	buttonCursor = XCreateFontCursor(dpy, XC_hand2);
	XFlush(dpy);

        xc->dpy = dpy;
        xc->target = target;
        xc->canvas = canvas;
        xc->gc = gc;

        return FlashGraphicInit(fh, &xc->fd);
}

void FlashCopyX11(int all)
{
    XSetFunction(xc->dpy,xc->gc,GXcopy);
    if (all) {
	    XCopyArea(xc->dpy,xc->canvas,xc->target,xc->gc,
		      0,0,
		      xc->fd.width,xc->fd.height,
		      xc->xOffset, xc->yOffset
		      );
    } else {
	    XCopyArea(xc->dpy,xc->canvas,xc->target,xc->gc,
		      xc->fd.clip_x,xc->fd.clip_y,
		      xc->fd.clip_width,xc->fd.clip_height,
		      xc->fd.clip_x + xc->xOffset, xc->fd.clip_y + xc->yOffset
		      );
    }
    XFlush(xc->dpy);
}

/*
 *	This file is the entry of a very simple Flash Player
 */

int
readFile(char *filename, char **buffer, long *size)
{
	FILE *in;
	char *buf;
	long length;

	in = fopen(filename,"r");
	if (in == 0) {
		perror(filename);
		return -1;
	}
	fseek(in,0,SEEK_END);
	length = ftell(in);
	rewind(in);
	buf = malloc(length);
	fread(buf,length,1,in);
	fclose(in);

	*size = length;
	*buffer = buf;

	return length;
}

void drawInfo()
{
	char msg[1024];

	sprintf(msg,"%s (Flash %d)  - Frames = %d  - Rate = %d fps",
			filename,fi.version,fi.frameCount,fi.frameRate);

	XSetForeground(dpy,gc,WhitePixel(dpy, DefaultScreen(dpy)));
	XDrawString(dpy,control,gc,10,15,msg, strlen(msg));

	sprintf(msg, "  (Q)uit (R)eplay (P)ause (C)ontinue");
	XDrawString(dpy,control,gc,10,35,msg, strlen(msg));
	XFlush(dpy);
}

void playMovie(FlashHandle flashHandle, Display *dpy, Window movie, int onRoot)
{
	struct timeval wd,de,now;
	XEvent event;
	long evMask, cmd;
	fd_set fdset;
	int status;
	long delay = 0;
	long wakeUp;
	long z = 1;
	long x = 0;
	long y = 0;
        FlashEvent fe;

	cmd = FLASH_WAKEUP;
	wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
	if (!onRoot) {
		XSelectInput(dpy, movie, FLASH_XEVENT_MASK|KeyPressMask);
	}
	XSync(dpy,False);

	while(1) {
		FD_ZERO(&fdset);
		FD_SET(ConnectionNumber(dpy),&fdset);

		/*printf("WakeUp = %d  Delay = %d\n", wakeUp, delay); */
		if (delay < 0) {
			delay = 20;
		}

                if (xc->fd.flash_refresh) {
                    FlashCopyX11(0);
                    xc->fd.flash_refresh = 0;
                }
		if (wakeUp) {
			de.tv_sec = delay/1000;
			de.tv_usec = (delay%1000)*1000;
			status = select(ConnectionNumber(dpy)+1, &fdset, 0, 0, &de);
		} else {
			status = select(ConnectionNumber(dpy)+1, &fdset, 0, 0, 0);
		}

		if (status == 0) {
			cmd = FLASH_WAKEUP;
			wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
		} else {
			XNextEvent(dpy, &event);
			/*printf("Event %d (%d)\n",event.type,event.xany.serial); */
			if (event.xany.window == movie) {
				int keycode;
				KeySym keysym;

				switch (event.type) {
				case KeyPress:
					keycode = event.xkey.keycode;
					keysym = XLookupKeysym((XKeyEvent*)&event, 0);
                                        fe.type = FeKeyPress;
                                        fe.key = 0;
					switch (keysym) {
						case XK_Up:
                                                    fe.key = FeKeyUp;
                                                    break;
						case XK_Down:
                                                    fe.key = FeKeyDown;
                                                    break;
						case XK_Left:
                                                    fe.key = FeKeyLeft;
                                                    break;
						case XK_Right:
                                                    fe.key = FeKeyRight;
                                                    break;
						case XK_Return:
                                                    fe.key = FeKeyEnter;
                                                    break;
						case XK_Tab:
                                                    fe.key = FeKeyNext;
                                                    break;
                                        }
                                        if (fe.key != 0) {
                                            cmd = FLASH_EVENT;
                                            if (FlashExec(flashHandle, cmd, &fe, &wd)) {
                                                wakeUp = 1;
                                            }
                                        } else {
                                            switch (keysym) {
#if 0
						case XK_Up:
							y -= 10;
							FlashOffset(flashHandle,x,y);
							break;
						case XK_Down:
							y += 10;
							FlashOffset(flashHandle,x,y);
							break;
						case XK_Left:
							x -= 10;
							FlashOffset(flashHandle,x,y);
							break;
						case XK_Right:
							x += 10;
							FlashOffset(flashHandle,x,y);
							break;
#endif
						case XK_KP_Add:
							FlashZoom(flashHandle,++z);
							break;
						case XK_KP_Subtract:
							FlashZoom(flashHandle,--z);
							break;
						case XK_q:
							return;
							break;
						case XK_c:
							cmd = FLASH_CONT;
							wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
							break;
						case XK_p:
							cmd = FLASH_STOP;
							wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
							break;
						case XK_r:
							cmd = FLASH_REWIND;
							FlashExec(flashHandle, cmd, 0, &wd);
							cmd = FLASH_CONT;
							wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
							break;
                                            }
                                        }
					break;
				case Expose:
				        FlashCopyX11(1);
					break;
				case NoExpose:
					break;
				default:
					cmd = FLASH_EVENT;
					if (FlashExecX11(flashHandle, cmd, &event, &wd)) {
                                            wakeUp = 1;
                                        }
					break;
				}
			}
			if (!onRoot && event.xany.window == control) {
				if (event.type == Expose) {
					drawInfo();
				}
			}
		}

		/* Recompute delay */
		gettimeofday(&now,0);
		delay = (wd.tv_sec-now.tv_sec)*1000 + (wd.tv_usec-now.tv_usec)/1000;
	}
}

void
showUrl(char *url, char *target, void *client_data)
{
	printf("GetURL : %s\n", url);
}

void
getSwf(char *url, int level, void *client_data)
{
	FlashHandle flashHandle;
	char *buffer;
	long size;

	flashHandle = (FlashHandle) client_data;
	printf("LoadMovie: %s @ %d\n", url, level);
	if (readFile(url, &buffer, &size) > 0) {
		FlashParse(flashHandle, level, buffer, size);
	}
}

main(int argc, char **argv)
{
	char *buffer;
	long size;
	char *audiodev;
	FlashHandle flashHandle;
	int status;
	int onRoot = 0;
	Screen *screen;

	if (argc < 2) {
		fprintf(stderr,"Usage : %s [ -root ] <file.swf>\n", argv[0]);
		exit(1);
	}
	if (argc == 3) {
		if (!strcmp(argv[1],"-root") || !strcmp(argv[1],"-inroot")) {
			onRoot = 1;
		}
		argc--;
		argv++;
	}

	dpy = XOpenDisplay(getenv("DISPLAY"));
	if (dpy == 0) {
		fprintf(stderr,"Can't open X display\n");
		exit(1);
	}

	screen = DefaultScreenOfDisplay(dpy);

	gc = DefaultGC(dpy, DefaultScreen(dpy));

	filename = argv[1];
	if (readFile(filename, &buffer, &size) < 0) {
		exit(2);
	}

	flashHandle = FlashNew();

	if (flashHandle == 0) {
		exit(1);
	}

	/* Load level 0 movie */
	do {
		status = FlashParse(flashHandle, 0, buffer, size);
	} while (status & FLASH_PARSE_NEED_DATA);

	free(buffer);

	FlashGetInfo(flashHandle, &fi);

	if (onRoot == 0) {
		frame = XCreateSimpleWindow(dpy, RootWindow(dpy, DefaultScreen(dpy)),
					    0, 0,
					    fi.frameWidth/20, fi.frameHeight/20+40,
					    0, WhitePixel(dpy, DefaultScreen(dpy)), BlackPixel(dpy, DefaultScreen(dpy))
					    );

		XMapWindow(dpy, frame);

		movie = XCreateSimpleWindow(dpy, frame, 0, 0, fi.frameWidth/20,fi.frameHeight/20,
					    0, WhitePixel(dpy, DefaultScreen(dpy)), BlackPixel(dpy, DefaultScreen(dpy))
					    );

		XMapWindow(dpy, movie);

		control = XCreateSimpleWindow(dpy, frame, 0, fi.frameHeight/20, fi.frameWidth/20,40,
					    0, BlackPixel(dpy, DefaultScreen(dpy)), BlackPixel(dpy, DefaultScreen(dpy))
					    );

		XMapWindow(dpy, control);
		XSelectInput(dpy, control, ExposureMask);
		drawInfo();
	} else {
		movie = VirtualRootWindowOfScreen(screen);
		/* Auto loop when onRoot */
		FlashSettings(flashHandle, PLAYER_LOOP);
	}

	XFlush(dpy);

        FlashGraphicInitX11(flashHandle, dpy, movie, onRoot);

	audiodev = getenv("AUDIODEV");
	if (audiodev == NULL) {
#if SUN_SOUND
	    	audiodev = "/dev/audio";
#else
	    	audiodev = "/dev/dsp";
#endif
	}
	FlashSoundInit(flashHandle, audiodev);

	FlashSetGetUrlMethod(flashHandle, showUrl, 0);

	FlashSetGetSwfMethod(flashHandle, getSwf, (void*)flashHandle);

	playMovie(flashHandle, dpy, movie, onRoot);

	FlashClose(flashHandle);

	exit(0);
}
