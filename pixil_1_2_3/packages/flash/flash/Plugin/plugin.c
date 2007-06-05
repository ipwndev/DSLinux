/******************************************************************************
 * Copyright (c) 1996 Netscape Communications. All rights reserved.
 ******************************************************************************/

/*
 *	Modifications for the Linux Flash Plugin
 *	by Olivier Debon <odebon@club-internet.fr>
 */

static char *rcsid = "$Id$";

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "npapi.h"

#include "flash.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/Core.h>
#include <X11/CoreP.h>

#define PRINT 0
#define FLASH_XEVENT_MASK (ExposureMask|ButtonReleaseMask|ButtonPressMask|PointerMotionMask|KeyPressMask|KeyReleaseMask)

typedef struct _load {
	char		*url;
	int		 level;
	char		*data;
	long  		 size;

	struct _load	*next;
} LoadCtxt;

typedef struct _PluginInstance
{
	long 		 gInitDone;
	Display 	*dpy;
	GC		 gc;
	Window 		 win;
	Pixmap 		 canvas;
        XShmSegmentInfo	 segInfo;	/* Shared memory information */
	Widget 		 widget;
	XtIntervalId 	 timer;
	struct timeval 	 wd;
	long   		 attributes;
	FlashHandle 	 fh;
	FlashDisplay	 fd;
	int		 cursorOver;
	Cursor		 buttonCursor;
	LoadCtxt	*loading;
} PluginInstance;

static void updateTimer(PluginInstance* This);
static void flashEvent(Widget w, XtPointer client_data, XEvent *event, Boolean *dispatch);
static void flashWakeUp(XtPointer client_data, XtIntervalId *id);
static void getUrl( char * url, char *target, void *client_data);
static void getSwf(char *url, int level, void *client_data);
static void cursorOnOff(int, void *client_data);
static long parseAttributes(int16 n, char *argn[], char *argv[]);

static long FlashExecX11(PluginInstance *, long , XEvent *, struct timeval *);
static long FlashGraphicInitX11(PluginInstance* This);
static void FlashCopyX11(PluginInstance* This);


#ifdef C6R5
/* Ugly but it makes it work if it is compiled on */
/* a libc6 system and running with a libc5-netscape */
int __sigsetjmp()
{
	return 0;
}
#endif

char*
NPP_GetMIMEDescription(void)
{
	return("application/x-shockwave-flash:swf:Flash Plugin;application/x-futuresplash:spl:Future Splash");
}

NPError
NPP_GetValue(void *future, NPPVariable variable, void *value)
{
	NPError err = NPERR_NO_ERROR;

	switch (variable) {
		case NPPVpluginNameString:
			*((char **)value) = PLUGIN_NAME;
			break;
		case NPPVpluginDescriptionString:
			*((char **)value) = "Flash Movie player " FLASH_VERSION_STRING
					    " compatible with Shockwave Flash 4.0 "
					    "<P>Shockwave is a trademark of <A HREF=\"http://www.macromedia.com\">Macromedia&reg;</A>"
					    "<P>Author : <A HREF=mailto:odebon@club-internet.fr>Olivier Debon </A>";
			break;
		default:
			err = NPERR_GENERIC_ERROR;
	}
	return err;
}

NPError
NPP_Initialize(void)
{
    freopen("/dev/tty","w",stdout);
    freopen("/dev/tty","w",stderr);
    return NPERR_NO_ERROR;
}


jref
NPP_GetJavaClass()
{
    return NULL;
}

void
NPP_Shutdown(void)
{
}


NPError 
NPP_New(NPMIMEType pluginType,
	NPP instance,
	uint16 mode,
	int16 argc,
	char* argn[],
	char* argv[],
	NPSavedData* saved)
{
        PluginInstance* This;
	char *audiodev;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
		
	instance->pdata = NPN_MemAlloc(sizeof(PluginInstance));
	
	This = (PluginInstance*) instance->pdata;

	if (This != NULL)
	{
		This->fh = FlashNew();
		This->gInitDone = 0;
		This->dpy = 0;
		This->win = 0;
		This->timer = 0;
		This->attributes = parseAttributes(argc, argn, argv);
		This->loading = 0;

		FlashSetGetUrlMethod(This->fh, getUrl, (void*)instance);

		FlashSetGetSwfMethod(This->fh, getSwf, (void*)instance);

		FlashSetCursorOnOffMethod(This->fh, cursorOnOff, (void*)instance);

		audiodev = getenv("AUDIODEV");
		if (audiodev == NULL) {
#if SUN_SOUND
			audiodev = "/dev/audio";
#else
			audiodev = "/dev/dsp";
#endif
		}
		FlashSoundInit(This->fh, audiodev);

		return NPERR_NO_ERROR;
	}
	else
		return NPERR_OUT_OF_MEMORY_ERROR;
}


NPError 
NPP_Destroy(NPP instance, NPSavedData** save)
{
	PluginInstance* This;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	This = (PluginInstance*) instance->pdata;

	if (This != NULL) {
		LoadCtxt *l,*prev;

		if (This->timer) {
			XtRemoveTimeOut(This->timer);
			This->timer = 0;
		}
		if (This->fh) {
			XShmDetach(This->dpy, &This->segInfo);
			XSync(This->dpy,False);
			XFreePixmap(This->dpy, This->canvas);
			shmdt(This->segInfo.shmaddr);

			FlashClose(This->fh);
			This->fh = 0;
		}
		XtRemoveEventHandler(This->widget, FLASH_XEVENT_MASK,
				  True, (XtEventHandler) flashEvent, (XtPointer)This);

		prev = 0;
		for(l = This->loading; l; prev = l, l = l->next) {
			free(l->data);
			free(l->url);
			free(prev);
		}
		free(prev);

		NPN_MemFree(instance->pdata);
		instance->pdata = NULL;
	}

	return NPERR_NO_ERROR;
}

static void
updateTimer(PluginInstance* This)
{
	XtAppContext ctxt;
	struct timeval now;
	long delay;

	if (This->timer) {
		XtRemoveTimeOut(This->timer);
	}

	gettimeofday(&now,0);
	delay = (This->wd.tv_sec-now.tv_sec)*1000 + (This->wd.tv_usec-now.tv_usec)/1000;

	/*fprintf(stderr,"Wakeup in %d ms\n", delay); */

	if (delay < 20) {
	    	/* OVERRUN !!! */
	    	delay = 20;	/* Leave 20 ms */
	}
	ctxt = XtWidgetToApplicationContext(This->widget);
	This->timer = XtAppAddTimeOut(ctxt, delay,
			    (XtTimerCallbackProc) flashWakeUp,
			    (XtPointer) This);
}

static void
flashEvent(Widget w, XtPointer client_data, XEvent *event, Boolean *dispatch)
{
	PluginInstance* This;
	long cmd;
	long wakeUp;

	This = (PluginInstance*)client_data;

	if (This->fh) {
		cmd = FLASH_EVENT;
		wakeUp = FlashExecX11(This, cmd, event, &This->wd);
		if (This->fd.flash_refresh) {
			FlashCopyX11(This);
		}
		if (wakeUp) {
			updateTimer(This);
		}
	}
}

static void
flashWakeUp(XtPointer client_data, XtIntervalId *id)
{
	PluginInstance* This;
	long cmd;
	long wakeUp;

	This = (PluginInstance*)client_data;

	if (This->fh) {
		cmd = FLASH_WAKEUP;
		wakeUp = FlashExecX11(This, cmd, 0, &This->wd);
		if (This->fd.flash_refresh) {
			FlashCopyX11(This);
		}

		/* If have to wake up next time */
		if (wakeUp) {
			updateTimer(This);
		} else {
			if (This->timer) {
				XtRemoveTimeOut(This->timer);
			}
			This->timer = 0;
		}
	}
}

NPError 
NPP_SetWindow(NPP instance, NPWindow* window)
{
	PluginInstance* This;
	NPSetWindowCallbackStruct *ws;
	Window frame;
	XWindowAttributes xwa;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	if (window == NULL)
		return NPERR_NO_ERROR;

	This = (PluginInstance*) instance->pdata;

	This->win = (Window) window->window;
	ws = (NPSetWindowCallbackStruct *)window->ws_info;
	This->dpy = ws->display;
	This->gc = DefaultGC(This->dpy, DefaultScreen(This->dpy));
	This->widget = XtWindowToWidget(This->dpy,This->win);

	XGetWindowAttributes(This->dpy, This->win, &xwa);

	return NPERR_NO_ERROR;
}

NPError 
NPP_NewStream(NPP instance,
			  NPMIMEType type,
			  NPStream *stream, 
			  NPBool seekable,
			  uint16 *stype)
{
	NPByteRange range;
	PluginInstance* This;
	LoadCtxt *l;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	This = (PluginInstance*) instance->pdata;

	/* This is only for the very first movie */
	if (This->loading == 0) {
		l = (LoadCtxt *)malloc(sizeof(LoadCtxt));
		This->loading = l;
		l->next = 0;
		l->level = 0;
		l->data = 0;
		l->size = 0;
		l->url = strdup(stream->url);
	}

	return NPERR_NO_ERROR;
}


#define BUFFERSIZE (16*1024)

int32 
NPP_WriteReady(NPP instance, NPStream *stream)
{
	PluginInstance* This;

	if (instance != NULL)
	{
		return BUFFERSIZE;
	}
	return 0;
}


int32 
NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
	PluginInstance* This;

	if (instance != NULL)
	{
		int status;
		LoadCtxt *l;

		This = (PluginInstance*) instance->pdata;

		for(l = This->loading; l != 0; l = l->next) {
			if (l->url && strstr(stream->url, l->url)) {
				break;
			}
		}

		if (l == 0) return 0;	/* Should not happen */

		if (l->data == 0) {
			l->data = (char *) malloc(len);
		} else {
			l->data = (char *) realloc(l->data, l->size+len);
		}

		memcpy(&l->data[offset], buffer, len);
		l->size += len;
		status = FlashParse(This->fh, l->level, l->data, l->size);

		if (status == FLASH_PARSE_ERROR) {
			free(l->data); l->data = 0;
			free(l->url); l->url = 0;
			return 0;
		}
		if (status & FLASH_PARSE_START) {
			if (!This->gInitDone && This->dpy) {
				FlashGraphicInitX11(This);
				XtAddEventHandler(This->widget, FLASH_XEVENT_MASK,
						  True, (XtEventHandler) flashEvent, (XtPointer)This);
				This->gInitDone = 1;
			}
		}
		if (status & FLASH_PARSE_WAKEUP) {
			flashWakeUp((XtPointer)This, 0);
		}
		if (status & FLASH_PARSE_EOM) {
			/*l->url = "";*/
		}
	}

	return len;		/* The number of bytes accepted */
}

static void
getUrl( char * url, char *target, void *client_data)
{
	NPP instance;

	instance = (NPP)client_data;
	NPN_GetURL(instance, url, target );
}

static void
getSwf(char *url, int level, void *client_data)
{
	PluginInstance* This;
	NPP instance;
	LoadCtxt *l;

	instance = (NPP)client_data;
	if (instance == 0) return;

	This = (PluginInstance*) instance->pdata;
	if (This == 0) return;

	l = (LoadCtxt *)malloc(sizeof(LoadCtxt));
	l->next = This->loading;
	This->loading = l;
	l->level = level;
	l->data = 0;
	l->size = 0;
	l->url = strdup(url);

	NPN_GetURL(instance, url, 0);
} 

static void
cursorOnOff( int on, void *client_data)
{
	PluginInstance* This;
	NPP instance;

	instance = (NPP)client_data;
	if (instance == 0) return;

	This = (PluginInstance*) instance->pdata;
	if (This == 0) return;

	if (on && !This->cursorOver) {
		XDefineCursor(This->dpy, This->win, This->buttonCursor);
		This->cursorOver = 1;
	}
	if (!on && This->cursorOver) {
		XUndefineCursor(This->dpy, This->win);
		This->cursorOver = 0;
	}
	XFlush(This->dpy);
}

NPError 
NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
	PluginInstance* This;
	LoadCtxt *l,*prev;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	if (reason != NPERR_NO_ERROR)
		return NPERR_INVALID_INSTANCE_ERROR;

	This = (PluginInstance*) instance->pdata;

	if (This == 0) return NPERR_INVALID_INSTANCE_ERROR;

	if (This->fh == 0) {
		return NPERR_INVALID_INSTANCE_ERROR;
	}

	for(l = This->loading; l; l = l->next) {
		if (l->url && strstr(stream->url, l->url)) {
			free(l->data);
			l->data = 0;
			free(l->url);
			l->url = 0;
			break;
		}
	}

	if (!This->gInitDone && This->dpy) {
		FlashGraphicInitX11(This);
		XtAddEventHandler(This->widget, FLASH_XEVENT_MASK,
				  True, (XtEventHandler) flashEvent, (XtPointer)This);
		This->gInitDone = 1;

		flashWakeUp((XtPointer)This, 0);
	}

	return NPERR_NO_ERROR;
}


void 
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
	PluginInstance* This;
	if (instance != NULL)
		This = (PluginInstance*) instance->pdata;
}


void 
NPP_Print(NPP instance, NPPrint* printInfo)
{
	if(printInfo == NULL)
		return;

	if (instance != NULL) {
		PluginInstance* This = (PluginInstance*) instance->pdata;
	
		if (printInfo->mode == NP_FULL) {
		    /*
		     * PLUGIN DEVELOPERS:
		     *	If your plugin would like to take over
		     *	printing completely when it is in full-screen mode,
		     *	set printInfo->pluginPrinted to TRUE and print your
		     *	plugin as you see fit.  If your plugin wants Netscape
		     *	to handle printing in this case, set
		     *	printInfo->pluginPrinted to FALSE (the default) and
		     *	do nothing.  If you do want to handle printing
		     *	yourself, printOne is true if the print button
		     *	(as opposed to the print menu) was clicked.
		     *	On the Macintosh, platformPrint is a THPrint; on
		     *	Windows, platformPrint is a structure
		     *	(defined in npapi.h) containing the printer name, port,
		     *	etc.
		     */

			void* platformPrint =
				printInfo->print.fullPrint.platformPrint;
			NPBool printOne =
				printInfo->print.fullPrint.printOne;
			
			/* Do the default*/
			printInfo->print.fullPrint.pluginPrinted = FALSE;
		}
		else {	/* If not fullscreen, we must be embedded */
		    /*
		     * PLUGIN DEVELOPERS:
		     *	If your plugin is embedded, or is full-screen
		     *	but you returned false in pluginPrinted above, NPP_Print
		     *	will be called with mode == NP_EMBED.  The NPWindow
		     *	in the printInfo gives the location and dimensions of
		     *	the embedded plugin on the printed page.  On the
		     *	Macintosh, platformPrint is the printer port; on
		     *	Windows, platformPrint is the handle to the printing
		     *	device context.
		     */

			NPWindow* printWindow =
				&(printInfo->print.embedPrint.window);
			void* platformPrint =
				printInfo->print.embedPrint.platformPrint;
		}
	}
}

static
long parseAttributes(int16 n, char *argn[], char *argv[])
{
	int16 i;
	int c;
	long attributes;

	attributes = 0;
	for(i=0; i<n; i++)
	{
		if (!strcasecmp(argn[i],"loop")) {
			if (!strcasecmp(argv[i],"true")) {
				attributes |= PLAYER_LOOP;
			}
		}
		if (!strcasecmp(argn[i],"menu")) {
			if (!strcasecmp(argv[i],"true")) {
				attributes |= PLAYER_MENU;
			}
		}
		if (!strcasecmp(argn[i],"quality")) {
			if (!strcasecmp(argv[i],"high")
			 || !strcasecmp(argv[i],"autohigh")) {
				attributes |= PLAYER_QUALITY;
			}
		}
	}
	return attributes;
}

static long
FlashExecX11(PluginInstance *This, long flag, XEvent *event, struct timeval *wakeDate)
{
    FlashEvent fe;
    long wu;

    if (flag & FLASH_EVENT) {
	int keycode;
	KeySym keysym;

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
	    XSetInputFocus(This->dpy,This->win, RevertToParent, CurrentTime);
            break;
        case Expose:
            fe.type = FeRefresh;
	    if (!This->gInitDone) return 0;
	    XSetFunction(This->dpy,This->gc,GXcopy);
	    XCopyArea(This->dpy,This->canvas,This->win,This->gc,
		      0,0, This->fd.width,This->fd.height, 0,0);
	    XFlush(This->dpy);
	    return 0;
            break;
	case KeyPress:
		keycode = event->xkey.keycode;
		keysym = XLookupKeysym((XKeyEvent*)event, 0);
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
		}
	    break;
	default:
            fe.type = FeNone;
	    return 0;
            break;
        }
    }

    return FlashExec(This->fh,flag,&fe,wakeDate);
}

static long
FlashGraphicInitX11(PluginInstance *This)
{
    XWindowAttributes 		 wattr;
    XPixmapFormatValues 	*pf;
    Visual 			*visual;
    int				 nItems;
    int				 n;
    struct shmid_ds		 buf;
    int				 targetWidth;
    int 			 targetHeight;
    long			 bpl;	/* Bytes per line */
    long			 bpp;	/* Bytes per pixel */
    long			 pad;	/* Scanline pad in byte */

    /* Platform dependent members */
    Window			 target;	/* Target window */
    Cursor		 	 buttonCursor;	/* Window cursor (a hand if over a button) */
    Display			*dpy;		/* X11 Display */
    GC		 		 gc;		/* X11 Graphic context */
    Pixmap			 canvas;	/* Graphic buffer */

    dpy = This->dpy;
    target = This->win;

    /* Get Window dimension */
    XGetWindowAttributes(dpy, target, &wattr);

    /* Get first visual, don't care about others, really ! */
    visual = wattr.visual;

#if PRINT
	printf("BitmapPad  = %d\n", BitmapPad(dpy));
	printf("BitmapUnit = %d\n", BitmapUnit(dpy));
	printf("Depth      = %d\n", DefaultDepth(dpy,DefaultScreen(dpy)));
	printf("RedMask    = %x\n", visual->red_mask);
	printf("GreenMask  = %x\n", visual->green_mask);
	printf("BlueMask   = %x\n", visual->blue_mask);
	printf("Bits/RGB   = %d\n", visual->bits_per_rgb);
#endif

	/* Get screen info */

	for(pf=XListPixmapFormats(dpy, &n); n--; pf++) {
		if (pf->depth == DefaultDepth(dpy, DefaultScreen(dpy))) {
			bpp = pf->bits_per_pixel/8;
			pad = pf->scanline_pad/8;
		}
#if PRINT
		printf("----------------\n");
		printf("Depth          = %d\n", pf->depth);
		printf("Bits Per Pixel = %d\n", pf->bits_per_pixel);
		printf("Scanline Pad   = %d\n", pf->scanline_pad);
#endif
	}

	targetWidth = wattr.width;
	targetHeight = wattr.height;

#if PRINT
	printf("Target Width  = %d\n", targetWidth);
	printf("Target Height = %d\n", targetHeight);
#endif

	if (bpp) {
		bpl = (targetWidth*bpp + pad-1)/pad*pad;
	} else {
		bpl = (targetWidth/8 + pad-1)/pad*pad;
	}

	XSelectInput(dpy, target, FLASH_XEVENT_MASK);

	/* Prepare data for Direct Graphics */
	This->segInfo.readOnly = False;
	This->segInfo.shmid = shmget (IPC_PRIVATE,targetHeight*bpl,IPC_CREAT|0777);
	if (This->segInfo.shmid <0) {
		perror("shmget");
		fprintf(stderr,"Size = %d x %d\n", targetWidth, targetHeight);
	}
	This->segInfo.shmaddr = (char*)shmat (This->segInfo.shmid, 0, 0);
	if ((long)This->segInfo.shmaddr == -1) {
		perror("shmat");
	}
	XShmAttach(dpy, &This->segInfo);
	XSync(dpy, False);

	/* When number of attached clients falls down to zero */
	/* the shm is removed. This is convenient when it crashes. */
	if (shmctl(This->segInfo.shmid, IPC_RMID, &buf) < 0) {
		perror("shmctl");
	}

	This->fd.pixels = (char*)This->segInfo.shmaddr;
        This->fd.width = targetWidth;
        This->fd.height = targetHeight;
        This->fd.bpl = bpl;
        This->fd.bpp = bpp;
        This->fd.depth = DefaultDepth(dpy, DefaultScreen(dpy));

	canvas = XShmCreatePixmap(dpy,target,This->segInfo.shmaddr,&This->segInfo,targetWidth,targetHeight,DefaultDepth(dpy, DefaultScreen(dpy)));
	XSync(dpy, False);

	This->canvas = canvas;

	This->buttonCursor = XCreateFontCursor(dpy, XC_hand2);
	XFlush(dpy);

	This->cursorOver = 0;

        return FlashGraphicInit(This->fh, &This->fd);
}

static void
FlashCopyX11(PluginInstance *This)
{
    if (!This->gInitDone) return;
    XSetFunction(This->dpy,This->gc,GXcopy);
    XCopyArea(This->dpy,This->canvas,This->win,This->gc,
              This->fd.clip_x,This->fd.clip_y,
	      This->fd.clip_width,This->fd.clip_height,
              This->fd.clip_x,This->fd.clip_y
	      );
    XFlush(This->dpy);
    This->fd.flash_refresh = 0;
}
