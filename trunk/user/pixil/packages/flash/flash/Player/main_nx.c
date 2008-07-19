/*///////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998 Olivier Debon
// 
// Ported to Nano-X by Greg Haerr <greg@censoft.com>
// Copyright (c) 2002, 2003 by Greg Haerr
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
//#include <sys/ipc.h>
//#include <sys/shm.h>
#include "flash.h"
#define MWINCLUDECOLORS
#include <nano-X.h>
extern int nxSocket;

static char *rcsid = "$Id$";

typedef struct {
    FlashDisplay fd;
    GR_WINDOW_ID	target;		/* Target window*/
    GR_GC_ID		gc;		/* NX Graphic context*/
    int			pixtype;
    long		xOffset, yOffset;
} NXContext;

NXContext xc1, *xc=&xc1;

GR_GC_ID gc;
GR_WINDOW_ID frame,movie,control;
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

#define FLASH_NXEVENT_MASK (GR_EVENT_MASK_EXPOSURE|GR_EVENT_MASK_BUTTON_DOWN|GR_EVENT_MASK_BUTTON_UP|GR_EVENT_MASK_MOUSE_POSITION|GR_EVENT_MASK_KEY_DOWN)

long FlashExecNX(FlashHandle fh, long flag, GR_EVENT *event, struct timeval *wakeDate)
{
    FlashEvent fe;

    if (flag & FLASH_EVENT) {
        /* NX to Flash event structure conversion*/
        switch (event->type) {
        case GR_EVENT_TYPE_BUTTON_DOWN:
            fe.type = FeButtonPress;
            break;
        case GR_EVENT_TYPE_BUTTON_UP:
            fe.type = FeButtonRelease;
            break;
        case GR_EVENT_TYPE_MOUSE_POSITION:
            fe.type = FeMouseMove;
            fe.x = event->mouse.x;
            fe.y = event->mouse.y;
            break;
        case GR_EVENT_TYPE_EXPOSURE:
            fe.type = FeRefresh;
            break;
        default:
            fe.type = FeNone;
            break;
        }
    }
    
    return FlashExec(fh,flag,&fe,wakeDate);
}

long FlashGraphicInitNX(FlashHandle fh, GR_WINDOW_ID w, int onRoot)
{
    GR_WINDOW_INFO wattr;
    GR_SCREEN_INFO sinfo;
    int nItems;
    int n;
    int			 	 targetWidth;
    int 			 targetHeight;
    long			 bpl;	/* Bytes per line*/
    long			 bpp;	/* Bytes per pixel*/
    long			 pad;	/* Scanline pad in byte*/
    /* Platform dependent members*/
    GR_WINDOW_ID		 target;	/* Target window*/
    GR_GC_ID		 	 gc;		/* NX Graphic context*/
    //GR_WINDOW_ID		 canvas;	/* Graphic buffer
    //struct shmid_ds buf;
    //Cursor		 	 buttonCursor;	/* Window cursor (a hand if over a button)*/
    //XShmSegmentInfo		 segInfo;	/* Shared memory information*/

	target = w;

	/* Get Window dimension*/
	GrGetWindowInfo(target, &wattr);

	/* Get screen info*/
	GrGetScreenInfo(&sinfo);

	bpp = sinfo.bpp/8;	/* bytes per pixel*/
	/* no padding for GrArea (=1)*/
	pad = 1;		/* # bytes pad*/

	gc = GrNewGC();

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

	if (bpp) {
		bpl = (targetWidth*bpp + pad-1)/pad*pad;
	} else {
		bpl = (targetWidth/8 + pad-1)/pad*pad;
	}

	if (!onRoot) {
		GrSelectEvents(target, FLASH_NXEVENT_MASK);
	}

#if 0
	/* Prepare data for Direct Graphics*/
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
	/* When number of attached clients falls down to zero */
	/* the shm is removed. This is convenient when it crashes. */
	if (shmctl(segInfo.shmid, IPC_RMID, &buf) < 0) {
		perror("shmctl");
	}
	//canvas = XShmCreatePixmap(dpy,target,segInfo.shmaddr,&segInfo,targetWidth,targetHeight,DefaultDepth(dpy, DefaultScreen(dpy)));
	//buttonCursor = XCreateFontCursor(dpy, XC_hand2);
#endif

	xc->fd.pixels = (char*)malloc(targetHeight*bpl);
        xc->fd.width = targetWidth;
        xc->fd.height = targetHeight;
        xc->fd.bpl = bpl;
        xc->fd.depth = sinfo.bpp;
        xc->fd.bpp = bpp;
        xc->target = target;
        xc->gc = gc;
	xc->pixtype = sinfo.pixtype;

	GrFlush();
        return FlashGraphicInit(fh, &xc->fd);
}

void FlashCopyNX(int all)
{
    //GrSetMode(xc->gc, GR_MODE_COPY);
    //if (all) {
	    GrArea(xc->target, xc->gc,
	    		xc->xOffset, xc->yOffset, xc->fd.width, xc->fd.height,
			xc->fd.pixels, xc->pixtype);
	    //XCopyArea(xc->dpy,xc->canvas,xc->target,xc->gc,
		      //0,0,
		      //xc->fd.width,xc->fd.height,
		      //xc->xOffset, xc->yOffset
		      //);
    //} else {
	    //XCopyArea(xc->dpy,xc->canvas,xc->target,xc->gc,
		      //xc->fd.clip_x,xc->fd.clip_y,
		      //xc->fd.clip_width,xc->fd.clip_height,
		      //xc->fd.clip_x + xc->xOffset, xc->fd.clip_y + xc->yOffset
		      //);
    //}
    GrFlush();
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
	char msg[256];

	sprintf(msg,"%s (Flash %d)  - Frames = %d  - Rate = %d fps",
			filename,fi.version,fi.frameCount,fi.frameRate);

	GrSetGCForeground(gc, WHITE);
	GrText(control,gc,10,15,msg, strlen(msg), GR_TFASCII|GR_TFBASELINE);

	sprintf(msg, "  (Q)uit (R)eplay (P)ause (C)ontinue (+) ZoomIn (-) ZoomOut");
	GrText(control,gc,10,35,msg, strlen(msg), GR_TFASCII|GR_TFBASELINE);
	GrFlush();
}

void playMovie(FlashHandle flashHandle, GR_WINDOW_ID movie, int onRoot)
{
	struct timeval wd,de,now;
	GR_EVENT event;
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
		GrSelectEvents(movie, FLASH_NXEVENT_MASK);
	}

	while(1) {
		FD_ZERO(&fdset);
		FD_SET(nxSocket,&fdset);

		if (GrPeekEvent(&event))
			goto have_event;

		/*printf("WakeUp = %d  Delay = %d\n", wakeUp, delay);*/
		if (delay < 0) {
			delay = 20;
		}

                if (xc->fd.flash_refresh) {
                    FlashCopyNX(0);
                    xc->fd.flash_refresh = 0;
                }
		if (wakeUp) {
			de.tv_sec = delay/1000;
			de.tv_usec = (delay%1000)*1000;
			status = select(nxSocket+1, &fdset, 0, 0, &de);
		} else {
			de.tv_sec = 0;
			de.tv_usec = 0;
			status = select(nxSocket+1, &fdset, 0, 0, &de);
		}

		if (status == 0) {
			cmd = FLASH_WAKEUP;
			wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
		} else {
have_event:
			GrGetNextEvent(&event);

			if (event.type == GR_EVENT_TYPE_CLOSE_REQ) {
				GrClose();
				exit(0);
			}

			if (event.exposure.wid == movie) {

				switch (event.type) {
				case GR_EVENT_TYPE_KEY_DOWN:
                                        fe.type = FeKeyPress;
                                        fe.key = 0;
					switch (event.keystroke.ch) {
						case MWKEY_UP:
                                                    fe.key = FeKeyUp;
                                                    break;
						case MWKEY_DOWN:
                                                    fe.key = FeKeyDown;
                                                    break;
						case MWKEY_LEFT:
                                                    fe.key = FeKeyLeft;
                                                    break;
						case MWKEY_RIGHT:
                                                    fe.key = FeKeyRight;
                                                    break;
						case MWKEY_ENTER:
                                                    fe.key = FeKeyEnter;
                                                    break;
						case MWKEY_TAB:
                                                    fe.key = FeKeyNext;
                                                    break;
                                        }
                                        if (fe.key != 0) {
                                            cmd = FLASH_EVENT;
                                            if (FlashExec(flashHandle, cmd, &fe, &wd)) {
                                                wakeUp = 1;
                                            }
                                        } else {
                                            switch (event.keystroke.ch) {
#if 1
						case MWKEY_UP:
							y -= 10;
							FlashOffset(flashHandle,x,y);
							break;
						case MWKEY_DOWN:
							y += 10;
							FlashOffset(flashHandle,x,y);
							break;
						case MWKEY_LEFT:
							x -= 10;
							FlashOffset(flashHandle,x,y);
							break;
						case MWKEY_RIGHT:
							x += 10;
							FlashOffset(flashHandle,x,y);
							break;
#endif
						case MWKEY_KP_PLUS:
						case '+':
						case '=':
							FlashZoom(flashHandle,++z);
							break;
						case MWKEY_KP_MINUS:
						case '-':
							FlashZoom(flashHandle,--z);
							break;
						case 'q':
							return;
							break;
						case 'c':
							cmd = FLASH_CONT;
							wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
							break;
						case 'p':
							cmd = FLASH_STOP;
							wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
							break;
						case 'r':
							cmd = FLASH_REWIND;
							FlashExec(flashHandle, cmd, 0, &wd);
							cmd = FLASH_CONT;
							wakeUp = FlashExec(flashHandle, cmd, 0, &wd);
							break;
                                            }
                                        }
					break;
				case GR_EVENT_TYPE_EXPOSURE:
				        FlashCopyNX(1);
					break;
				default:
					cmd = FLASH_EVENT;
					if (FlashExecNX(flashHandle, cmd, &event, &wd)) {
                                            wakeUp = 1;
                                        }
					break;
				}
			}
			if (!onRoot && event.exposure.wid == control) {
				if (event.type == GR_EVENT_TYPE_EXPOSURE) {
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

	if (GrOpen() < 0) {
		fprintf(stderr,"Can't open graphics\n");
		exit(1);
	}

	gc = GrNewGC();
	GrSetGCUseBackground(gc, GR_FALSE);

	filename = argv[1];
	if (readFile(filename, &buffer, &size) < 0) {
		exit(2);
	}

	flashHandle = FlashNew();

	if (flashHandle == 0) {
		exit(1);
	}

	/* Load level 0 movie*/
	do {
		status = FlashParse(flashHandle, 0, buffer, size);
	} while (status & FLASH_PARSE_NEED_DATA);

	if (status == FLASH_PARSE_ERROR) {
		fprintf(stderr, "Error - Unable to parse the flash file\n");
		exit(-1);
	}

	free(buffer);

	FlashGetInfo(flashHandle, &fi);

	if (onRoot == 0) {
		frame = GrNewWindowEx(GR_WM_PROPS_APPWINDOW, "Microwindows Flash Player",
					    GR_ROOT_WINDOW_ID,
					    0, 0, fi.frameWidth/20, fi.frameHeight/20+40,
					    BLACK);

		GrSelectEvents(frame, GR_EVENT_MASK_CLOSE_REQ);
		GrMapWindow(frame);

		movie = GrNewWindow(frame,
					    0, 0, fi.frameWidth/20,fi.frameHeight/20,
					    0, BLACK, BLACK);

		GrMapWindow(movie);

		control = GrNewWindow(frame,
					    0, fi.frameHeight/20, fi.frameWidth/20, 40,
					    0, GREEN, BLACK);

		GrMapWindow(control);
		GrSelectEvents(control, GR_EVENT_MASK_EXPOSURE);
		drawInfo();
	} else {
		movie = GR_ROOT_WINDOW_ID;
		control = GR_ROOT_WINDOW_ID;
		/* Auto loop when onRoot*/
		FlashSettings(flashHandle, PLAYER_LOOP);
	}

	GrFlush();

        FlashGraphicInitNX(flashHandle, movie, onRoot);

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

	playMovie(flashHandle, movie, onRoot);

	FlashClose(flashHandle);

	exit(0);
}
