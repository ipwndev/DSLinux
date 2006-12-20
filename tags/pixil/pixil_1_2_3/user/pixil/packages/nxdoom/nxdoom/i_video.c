// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log$
// Revision 1.1  2006/10/03 11:26:17  dslinux_amadeus
// adding pristine copy of pixil to HEAD so I can branch from it
//
// Revision 1.2  2003/09/08 22:34:28  jasonk
// Updated files because this fucker won't build for no fucking good reason.
//
// Revision 1.1.1.1  2003/09/04 21:08:12  jasonk
// Initial import
//
// Revision 1.1  2000/12/08 21:07:53  jeffw
// nxdoom initial entry -- No nxdoom/Makefile so it won't build automatically
//
//
// DESCRIPTION:
//	DOOM graphics stuff for Microwindows
//	Ported from SDL Doom by Greg Haerr
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id$";

#include <stdlib.h>
#define MWINCLUDECOLORS
#include <nano-X.h>
#include "m_swap.h"
#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"
#include "doomdef.h"

/* create 16 bit 5/6/5 format pixel from RGB triplet */
#define RGB2PIXEL565(r,g,b)	\
	((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

#define GETPALENTRY(pal,index) ((unsigned long)(pal[index].r |\
				(pal[index].g << 8) | (pal[index].b << 16)))

static GR_WINDOW_ID	win;
static GR_GC_ID		gc;

// Blocky mode,
// replace each 320x200 pixel with multiply*multiply pixels.
// According to Dave Taylor, it still is a bonehead thing
// to use ....
static int	multiply = 1;
static int	flip = 1;
static int	video_w, video_h;
static byte *	finalimage;
static int	pixtype;
static GR_PALETTE pal;

//
//  Translates the key 
//
int
xlatekey(int key)
{
    int rc;

    switch(key) {

      case MWKEY_LEFT:	
	if (flip)
	  rc = KEY_UPARROW;	
	else
	  rc = KEY_LEFTARROW;
	break;

      case MWKEY_RIGHT:	
	if (flip)
	  rc = KEY_DOWNARROW;  
	else
	  rc = KEY_RIGHTARROW;
	break;

      case MWKEY_DOWN:	
	if (flip)
	  rc = KEY_LEFTARROW;  
	else
	  rc = KEY_DOWNARROW;

	break;

    case MWKEY_UP:	
      if (flip)
	rc = KEY_RIGHTARROW;  
      else
	rc = KEY_UPARROW;
      
      break;

    case MWKEY_HOME:rc = KEY_ESCAPE;	break;
      
    case MWKEY_ENTER:
    case MWKEY_KP_ENTER:
    case MWKEY_MENU:	rc = KEY_ENTER;		break;

    case MWKEY_TAB:	rc = KEY_TAB;		break;
    case MWKEY_F1:	rc = KEY_F1;		break;
    case MWKEY_F2:	rc = KEY_F2;		break;
    case MWKEY_F3:	rc = KEY_F3;		break;
    case MWKEY_F4:	rc = KEY_F4;		break;
    case MWKEY_F5:	rc = KEY_F5;		break;
    case MWKEY_F6:	rc = KEY_F6;		break;
    case MWKEY_F7:	rc = KEY_F7;		break;
    case MWKEY_F8:	rc = KEY_F8;		break;
    case MWKEY_F9:	rc = KEY_F9;		break;
    case MWKEY_F10:	rc = KEY_F10;		break;
    case MWKEY_F11:	rc = KEY_F11;		break;
    case MWKEY_F12:	rc = KEY_F12;		break;

    case MWKEY_APP2:    rc = 'y'; break;

    case MWKEY_BACKSPACE:
    case MWKEY_APP1:
      case MWKEY_DELETE:rc = ' '; printf("Got space\n");	break;

      case MWKEY_PAUSE:	rc = KEY_PAUSE;		break;

      case MWKEY_KP_EQUALS:
      case MWKEY_KP_PLUS:
      case '=':		rc = KEY_EQUALS;	break;

      case MWKEY_KP_MINUS:
      case '-':		rc = KEY_MINUS;		break;

      case MWKEY_LSHIFT:
      case MWKEY_RSHIFT:rc = KEY_RSHIFT;	break;

      case MWKEY_LCTRL:
    case MWKEY_RECORD:
      case MWKEY_RCTRL:	rc = KEY_RCTRL;		break;
	
      case MWKEY_LALT:
      case MWKEY_LMETA:
      case MWKEY_RALT:
      case MWKEY_RMETA:	rc = KEY_RALT;		break;
	
      default:		rc = key;		break;
    }
    return rc;
}

void I_ShutdownGraphics(void)
{
  GrClose();
}



//
// I_StartFrame
//
void I_StartFrame (void)
{
}

/* This processes Nano-X events */
void I_GetEvent(GR_EVENT *Event)
{
    int 		ch;
    unsigned int	buttonstate;
    event_t		event;
    int			xpos, ypos, xrel, yrel;
    int			factor;
    static int		lastx = 0;
    static int		lasty = 0;

    switch (Event->type)
    {

	/* Coming soon to a Nano-X server near you:  Joystick support */
#ifdef NOTUSED

    case GR_EVENT_TYPE_JOYSTICK_MOVE:
    case GR_EVENT_TYPE_JOYSTICK_BUTTON:

      event.type = ev_joystick;
      
      /* We can support more than 4 buttons, we might */
      /* as well grab as many as we can               */

      event.data1 = ((GR_EVENT_JOYSTICK *)Event)->button_value;
      event.data2 = (((GR_EVENT_JOYSTICK *)Event)->axis[GR_JOYSTICK_XAXIS]);
      event.data3 = -((GR_EVENT_JOYSTICK *)Event)->axis[GR_JOYSTICK_YAXIS];
      D_PostEvent(&event);
      break;
#endif

    case GR_EVENT_TYPE_KEY_DOWN:
	event.type = ev_keydown;
	ch = ((GR_EVENT_KEYSTROKE *)Event)->ch;
	event.data1 = xlatekey(ch);
	D_PostEvent(&event);
        break;

      case GR_EVENT_TYPE_KEY_UP:
	event.type = ev_keyup;
	ch = ((GR_EVENT_KEYSTROKE *)Event)->ch;
	event.data1 = xlatekey(ch);
	D_PostEvent(&event);
	break;

      case GR_EVENT_TYPE_BUTTON_DOWN:
      case GR_EVENT_TYPE_BUTTON_UP:
	buttonstate = ((GR_EVENT_MOUSE *)Event)->buttons;
	event.type = ev_mouse;
	event.data1 = 0
	    | (buttonstate & GR_BUTTON_L ? 1 : 0)
	    | (buttonstate & GR_BUTTON_M ? 2 : 0)
	    | (buttonstate & GR_BUTTON_R ? 4 : 0);
	event.data2 = event.data3 = 0;
	D_PostEvent(&event);
	break;

      case GR_EVENT_TYPE_MOUSE_MOTION:
	    buttonstate = ((GR_EVENT_MOUSE *)Event)->buttons;
	    xpos = ((GR_EVENT_MOUSE *)Event)->x;
	    ypos = ((GR_EVENT_MOUSE *)Event)->y;
	    
	    /* ignore warped mouse events*/
	    if (xpos == video_w/2 && ypos == video_h/2)
		    break;
	    xrel = xpos - lastx;
	    yrel = ypos - lasty;
	    lastx = xpos;
	    lasty = ypos;

	    event.type = ev_mouse;
	    event.data1 = 0
	        | (buttonstate & GR_BUTTON_L ? 1 : 0)
	        | (buttonstate & GR_BUTTON_M ? 2 : 0)
	        | (buttonstate & GR_BUTTON_R ? 4 : 0);
	    factor = 3;
	    if (xpos > video_w || xpos < video_w/2-50)
		    ++factor;
	    event.data2 = xrel << factor;
	    if (ypos > video_h/2+50 || ypos < video_h/2-50)
		    ++factor;
	    event.data3 = -yrel << factor;
	    D_PostEvent(&event);

#define MINVALUE	10
	    if (xpos < MINVALUE || xpos > video_w-MINVALUE ||
		ypos < MINVALUE || ypos > video_h-MINVALUE) {
		    GrInjectPointerEvent(video_w/2, video_h/2, 0, 0);
		    lastx = video_w/2;
		    lasty = video_h/2;
	    }
	break;

      case GR_EVENT_TYPE_CLOSE_REQ:
	I_Quit();
	break;
    }

}

//
// I_StartTic
//
void I_StartTic (void)
{
    GR_EVENT Event;

    do {
    	GrCheckNextEvent(&Event);
	I_GetEvent(&Event);
    } while (Event.type != GR_EVENT_TYPE_NONE);
}


//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{

    // draws little dots on the bottom of the screen
    if (devparm) {
        int		tics;
        int		i;
        static int	lasttic;

	i = I_GetTime();
	tics = i - lasttic;
	lasttic = i;
	if (tics > 20) tics = 20;

	for (i=0 ; i<tics*2 ; i+=2)
	    screens[0][ (video_h-1)*video_w + i] = 0xff;
	for ( ; i<20*2 ; i+=2)
	    screens[0][ (video_h-1)*video_w + i] = 0x0;
    }

    if (multiply == 1) {
	/*
	 * Yep, it's ugly.
	 */
	if (pixtype == MWPF_PALETTE) {
		if (flip) {
		    unsigned char *ilineptr = (unsigned char *)screens[0];
		    int y, x;

		    for (x=0; x<SCREENHEIGHT; ++x) {
			for (y=SCREENWIDTH-1; y>=0; --y) {
			    finalimage[y*SCREENHEIGHT + x] = *ilineptr++;
			}
		    }
		    //		    printf("GrArea #1\n");
		    GrArea(win, gc, 0, 0, SCREENHEIGHT, SCREENWIDTH, finalimage,
			MWPF_PALETTE);

		} else {
		  //		  printf("GrArea #2\n");
		GrArea(win, gc, 0, 0, video_w, video_h, screens[0],
		       MWPF_PALETTE);
		}

	} else {
		unsigned char *ilineptr = (unsigned char *)screens[0];
		unsigned short *olineptr = (unsigned short *)finalimage;
		int y, x;
		unsigned char index;

		if (flip) {
		    for (x=0; x<SCREENHEIGHT; ++x) {
			for (y=SCREENWIDTH-1; y>=0; --y) {
			    index = *ilineptr++;
			    olineptr[y*SCREENHEIGHT + x] = 
				RGB2PIXEL565(pal.palette[index].r,
				pal.palette[index].g, pal.palette[index].b);
			}
		    }
		    //		    printf("GrArea #3\n");
		    GrArea(win, gc, 0, 0, SCREENHEIGHT, SCREENWIDTH, finalimage,
			MWPF_TRUECOLOR565);
		} else {
		    for (y=0; y<SCREENHEIGHT; ++y) {
			for (x=0; x<SCREENWIDTH; ++x) {
				index = *ilineptr++;
				*olineptr++ = RGB2PIXEL565(pal.palette[index].r,
				    pal.palette[index].g, pal.palette[index].b);
			}
		    }
		    //		    printf("GrArea #4 %d/%d\n",video_w,video_h);
		    GrArea(win, gc, 0, 0, video_w, video_h, finalimage,
			   MWPF_TRUECOLOR565);

		}
	}

    } else if (multiply == 2) {
	unsigned int *olineptrs[2];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int twoopixels;
	unsigned int twomoreopixels;
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0; i<2 ; i++)
	    olineptrs[i] = (unsigned int *)
		    &((unsigned char *)finalimage)[i*video_w];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		twoopixels =	(fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xffff00)
		    |	((fouripixels>>16) & 0xff);
		twomoreopixels =	((fouripixels<<16) & 0xff000000)
		    |	((fouripixels<<8) & 0xffff00)
		    |	(fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
#else
		*olineptrs[0]++ = twomoreopixels;
		*olineptrs[1]++ = twomoreopixels;
		*olineptrs[0]++ = twoopixels;
		*olineptrs[1]++ = twoopixels;
#endif
	    } while (x-=4);
	    olineptrs[0] += video_w/4;
	    olineptrs[1] += video_w/4;
	}
	//	printf("GrArea #5\n");
    	GrArea(win, gc, 0, 0, video_w, video_h, finalimage, MWPF_PALETTE);

    } else if (multiply == 3) {
	unsigned int *olineptrs[3];
	unsigned int *ilineptr;
	int x, y, i;
	unsigned int fouropixels[3];
	unsigned int fouripixels;

	ilineptr = (unsigned int *) (screens[0]);
	for (i=0 ; i<3 ; i++)
	    olineptrs[i] = (unsigned int *)
		    &((unsigned char *)finalimage)[i*video_w];

	y = SCREENHEIGHT;
	while (y--)
	{
	    x = SCREENWIDTH;
	    do
	    {
		fouripixels = *ilineptr++;
		fouropixels[0] = (fouripixels & 0xff000000)
		    |	((fouripixels>>8) & 0xff0000)
		    |	((fouripixels>>16) & 0xffff);
		fouropixels[1] = ((fouripixels<<8) & 0xff000000)
		    |	(fouripixels & 0xffff00)
		    |	((fouripixels>>8) & 0xff);
		fouropixels[2] = ((fouripixels<<16) & 0xffff0000)
		    |	((fouripixels<<8) & 0xff00)
		    |	(fouripixels & 0xff);
#ifdef __BIG_ENDIAN__
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
#else
		*olineptrs[0]++ = fouropixels[2];
		*olineptrs[1]++ = fouropixels[2];
		*olineptrs[2]++ = fouropixels[2];
		*olineptrs[0]++ = fouropixels[1];
		*olineptrs[1]++ = fouropixels[1];
		*olineptrs[2]++ = fouropixels[1];
		*olineptrs[0]++ = fouropixels[0];
		*olineptrs[1]++ = fouropixels[0];
		*olineptrs[2]++ = fouropixels[0];
#endif
	    } while (x-=4);
	    olineptrs[0] += 2*video_w/4;
	    olineptrs[1] += 2*video_w/4;
	    olineptrs[2] += 2*video_w/4;
	}
	//	printf("GrArea #6\n");
    	GrArea(win, gc, 0, 0, video_w, video_h, finalimage, MWPF_PALETTE);

    }
}


//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, screens[0], SCREENWIDTH*SCREENHEIGHT);
}


//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
    int i;

    pal.count = 256;
    for ( i=0; i<256; ++i ) {
	pal.palette[i].r = gammatable[usegamma][*palette++];
	pal.palette[i].g = gammatable[usegamma][*palette++];
	pal.palette[i].b = gammatable[usegamma][*palette++];
    }
    GrSetSystemPalette(0, &pal);
}


void I_InitGraphics(void)
{
    static int	firsttime=1;
    int	bytesperpixel;
    GR_SCREEN_INFO info;

    if (!firsttime)
	return;
    firsttime = 0;

    //if (!!M_CheckParm("-fullscreen"))
        //video_flags |= SDL_FULLSCREEN;

    if (M_CheckParm("-2"))
	multiply = 2;

    if (M_CheckParm("-3"))
	multiply = 3;

    if (M_CheckParm("-flip"))
	flip = 0;

    GrGetScreenInfo(&info);
    pixtype = info.pixtype;
    switch (pixtype) {
    case MWPF_PALETTE:
    case MWPF_TRUECOLOR332:
	    bytesperpixel = 1;
	    break;
    case MWPF_TRUECOLOR565:
	    bytesperpixel = 2;
	    break;
    default:
	    I_Error("Pixel depth not supported\n");
    }

    if (flip) {
	    video_h = SCREENWIDTH * multiply;
	    video_w = SCREENHEIGHT * multiply;
    } else {
	    video_w = SCREENWIDTH * multiply;
	    video_h = SCREENHEIGHT * multiply;
    }

#ifdef NOTUSED
    GrOpenJoystick();
#endif

    win = GrNewWindowEx(GR_WM_PROPS_NOAUTOMOVE | GR_WM_PROPS_CLOSEBOX, "Microwindows Doom! v1.10",
		GR_ROOT_WINDOW_ID, 0, 0, info.cols, info.rows, BLACK);
    GrSelectEvents(win, GR_EVENT_MASK_CLOSE_REQ | GR_EVENT_MASK_MOUSE_MOTION | 
		GR_EVENT_MASK_BUTTON_DOWN | GR_EVENT_MASK_BUTTON_UP | 
		GR_EVENT_MASK_KEY_DOWN | GR_EVENT_MASK_KEY_UP |
		GR_EVENT_MASK_EXPOSURE 
#ifdef NOTUSED
| GR_EVENT_MASK_JOYSTICK_BUTTON | GR_EVENT_MASK_JOYSTICK_MOVE
#endif
);
    gc = GrNewGC();
    GrMapWindow(win);

    //SDL_ShowCursor(0);

    /* Set up the screen displays */
    screens[0] = (unsigned char *) malloc (SCREENWIDTH * SCREENHEIGHT);
    if (screens[0] == NULL)
            I_Error("Couldn't allocate screen memory");
    if (multiply > 1 || flip || bytesperpixel != 1) {
	    finalimage = (unsigned char *)
		    malloc (video_w * video_h * bytesperpixel);
	    if (finalimage == NULL)
		    I_Error("Couldn't allocate screen memory");
    }
}
