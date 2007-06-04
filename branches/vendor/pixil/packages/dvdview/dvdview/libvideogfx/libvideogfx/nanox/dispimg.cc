/* Output routines for Microwindows*/
/* Copyright (c) 2001 by Greg Haerr <greg@censoft.com>*/
#include "config.h"

#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include <iostream.h>
#include <iomanip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <nano-X.h>

#include "libvideogfx/types.hh"
#include "server.hh"
#include "dispimg.hh"

extern bool fullscreen;

struct DisplayImage_Data {
  DisplayImage_Data()
    : mayUseMITSHM(true),
      d_initialized(false),
      d_data(NULL)
    {
    }

  ~DisplayImage_Data()
    {
      if (d_initialized)
	{
	      d_ximg->data = NULL;
	      if (d_data) delete[] d_data;
	      GrDestroyGC(d_gc);
	}
    }

  // user parameters

  bool mayUseMITSHM;

  // internal parameters

  bool        d_initialized;
  bool        d_UseShmExt;

  GR_WINDOW_ID   d_win;
  GR_GC_ID       d_gc;

  XImage*  d_ximg;
  uint8*   d_data;
  bool     d_WaitForCompletion;
  int      d_CompletionType;
  int	   d_width,d_height;
};

#define MWPORTRAIT_GRAREA	8	/* no framebuffer, use GrArea instead*/

/* common blitter parameter structure*/
typedef struct {
	void *		dstaddr;	/* dst drawable*/
	MWCOORD		dstx, dsty;	/* dst x,y*/
	MWCOORD		dstw, dsth;	/* dst w,h*/
	int		dstbpp;		/* dst bits per pixel*/
	int		dstbytespp;	/* dst bytes per pixel*/
	int		dstpitch;	/* dst bytes per scanline*/
	int		dstxres, dstyres;/* dst total size, for portrait flipping*/

	void *		srcaddr;	/* src drawable*/
	MWCOORD		srcx, srcy;	/* src x,y*/
	MWCOORD		srcw, srch;	/* src w,h if stretchblit*/
	int		srcbpp;		/* src bits per pixel*/
	int		srcbytespp;	/* src bytes per pixel*/
	int		srcpitch;	/* src bytes per scanline*/

	unsigned long 	rop;		/* raster opcode*/

	GR_WINDOW_ID	wid;		/* window id for GrArea*/
	GR_GC_ID	gc;		/* graphics context for GrArea*/

	//void *		alphachan;	/* alpha chan for MWROP_BLENDCHANNEL*/
	//MWPIXELVAL	fgcolor;	/* fg/bg color for MWROP_BLENDFGBG*/
	//MWPIXELVAL	bgcolor;
	//MWPIXELVAL	transcolor;	/* trans color for MWROP_SRCTRANSCOPY*/
} BLITARGS, *PBLITARGS;

#define SRCX	args->srcx
#define SRCY	args->srcy
#define SRCH	args->srch
#define SRCW	args->srcw
#define DSTX	args->dstx
#define DSTY	args->dsty
#define DSTH	args->dsth
#define DSTW	args->dstw
#define SPITCH	args->srcpitch
#define DPITCH	args->dstpitch
#define DSTXRES	args->dstxres
#define DSTYRES	args->dstyres
#define BYTESPP	args->dstbytespp		/* must be same as srcbpp*/

/*
 * These conversion blits are a major pain.
 * Mess with them at your own risk ;-)
 */
static void
ConvBlit16(PBLITARGS args)
{
	unsigned short *ilineptr;
	unsigned short *olineptr;
	unsigned int x, y;

	if (args->rop == MWPORTRAIT_RIGHT) {
		/* right portrait mode conversion*/
		unsigned int linerem = SPITCH/2 - SRCW;
		ilineptr = (unsigned short *)(((char *)args->srcaddr) +
			SRCY*SPITCH + SRCX*BYTESPP);
		olineptr = (unsigned short *)args->dstaddr;
		for (y=0; y<DSTH; ++y) {
			unsigned int ox =
				(DSTH-1-y + DSTXRES-DSTH - (DSTY+SRCY)) * BYTESPP;
			for (x=0; x<DSTW; ++x) {
				unsigned int oy = (x + DSTX + SRCX) * DPITCH;
				olineptr[(oy + ox)/2] = *ilineptr++;
			}
			ilineptr += linerem;
		}
	} 
	else if (args->rop == MWPORTRAIT_LEFT) {
		/* left portrait mode conversion*/
		unsigned int linerem = SPITCH/2 - SRCW;
		ilineptr = (unsigned short *)((char *)args->srcaddr + SRCY*SPITCH + SRCX*BYTESPP);
		olineptr = (unsigned short *)args->dstaddr;
		for (y=0; y<DSTH; ++y) {
			unsigned int ox = (y + DSTY + SRCY) * BYTESPP;
			for (x=0; x<DSTW; ++x) {
				unsigned int oy =
				    (DSTW-1-x + DSTYRES-DSTW - (DSTX+SRCX)) * DPITCH;
				olineptr[(oy + ox)/2] = *ilineptr++;
			}
			ilineptr += linerem;
		}
	}
	else if (args->rop == MWPORTRAIT_NONE) {
		/* straight blit, no conversion*/
		unsigned int srclinerem = SPITCH/2;
		unsigned int dstlinerem = DPITCH/2;
		unsigned int srcoffset = SRCY*SPITCH + SRCX*BYTESPP;
		unsigned int dstoffset = DSTY*DPITCH + DSTX*BYTESPP;
		ilineptr = (unsigned short *)((char *)args->srcaddr + srcoffset);
		olineptr = (unsigned short *)((char *)args->dstaddr + dstoffset);
		for (y=0; y<DSTH; ++y) {
			memcpy(olineptr, ilineptr, DSTW<<1);
			olineptr += dstlinerem;
			ilineptr += srclinerem;
		}
	}
	else {
		unsigned char *buf;

		/* check pitch/bytespp same, no malloc*/
		if (SRCY || SRCX || (SPITCH != DPITCH)) {
			/* straight blit, convert pitch, don't draw on framebuffer*/
			unsigned int linerem = SPITCH/2;
			unsigned int srcoffset = SRCY*SPITCH + SRCX*BYTESPP;
			ilineptr = (unsigned short *)((char *)args->srcaddr + srcoffset);
			buf = (unsigned char *)malloc(DSTW * DSTH * 2);
			olineptr = (unsigned short *)buf;
			for (y=0; y<DSTH; ++y) {
				memcpy(olineptr, ilineptr, DSTW<<1);
				olineptr += DSTW;
				ilineptr += linerem;
			}
		} else buf = (unsigned char *)args->srcaddr;

		GrArea(args->wid, args->gc, DSTX, DSTY, DSTW, DSTH, buf, MWPF_TRUECOLOR565);
		GrFlush();

		if (buf != args->srcaddr)
			free(buf);
	}
}

#if LATER
static void
ConvBlit24(PBLITARGS args)
{
	unsigned long *ilineptr;
	unsigned long *olineptr;
	unsigned int x, y;

	if (args->rop == MWPORTRAIT_RIGHT) {
		/* right portrait mode conversion*/
		unsigned int linerem = SPITCH - SRCW*3;
		ilineptr = (unsigned long *)(((char *)args->srcaddr) +
			SRCY*SPITCH + SRCX*BYTESPP);
		olineptr = (unsigned long *)args->dstaddr;
		for (y=0; y<DSTH; ++y) {
			unsigned int ox =
				(DSTH-1-y + DSTXRES-DSTH - (DSTY+SRCY)) * BYTESPP;
			for (x=0; x<DSTW; ++x) {
				unsigned int oy = (x + DSTX + SRCX) * DPITCH;
				olineptr[(oy + ox)] = *ilineptr++;
				olineptr[(oy + ox + 1)] = *ilineptr++;
				olineptr[(oy + ox + 2)] = *ilineptr++;
			}
			ilineptr += linerem;
		}
	} 
	else if (args->rop == MWPORTRAIT_LEFT) {
		/* left portrait mode conversion*/
		unsigned int linerem = SPITCH - SRCW*3;
		ilineptr = (unsigned long *)((char *)args->srcaddr + SRCY*SPITCH + SRCX*BYTESPP);
		olineptr = (unsigned long *)args->dstaddr;
		for (y=0; y<DSTH; ++y) {
			unsigned int ox = (y + DSTY + SRCY) * BYTESPP;
			for (x=0; x<DSTW; ++x) {
				unsigned int oy =
				    (DSTW-1-x + DSTYRES-DSTW - (DSTX+SRCX)) * DPITCH;
				olineptr[(oy + ox)] = *ilineptr++;
				olineptr[(oy + ox + 1)] = *ilineptr++;
				olineptr[(oy + ox + 2)] = *ilineptr++;
			}
			ilineptr += linerem;
		}
	}
	else if (args->rop == MWPORTRAIT_NONE) {
		/* straight blit, no conversion*/
		unsigned int srclinerem = SPITCH;
		unsigned int dstlinerem = DPITCH;
		unsigned int srcoffset = SRCY*SPITCH + SRCX*BYTESPP;
		unsigned int dstoffset = DSTY*DPITCH + DSTX*BYTESPP;
		ilineptr = (unsigned long *)((char *)args->srcaddr + srcoffset);
		olineptr = (unsigned long *)((char *)args->dstaddr + dstoffset);
		for (y=0; y<DSTH; ++y) {
			memcpy(olineptr, ilineptr, DSTW*3);
			olineptr += dstlinerem;
			ilineptr += srclinerem;
		}
	}
	else {
		unsigned char *buf;

		/* check pitch/bytespp same, no malloc*/
		if (SRCY || SRCX || (SPITCH != DPITCH)) {
			/* straight blit, convert pitch, don't draw on framebuffer*/
			unsigned int linerem = SPITCH;
			unsigned int dstw3 = DSTW*3;
			unsigned int srcoffset = SRCY*SPITCH + SRCX*BYTESPP;
			ilineptr = (unsigned long *)((char *)args->srcaddr + srcoffset);
			buf = (unsigned char *)malloc(DSTW * DSTH * 3);
			olineptr = (unsigned long *)buf;
			for (y=0; y<DSTH; ++y) {
				memcpy(olineptr, ilineptr, dstw3);
				olineptr += dstw3;
				ilineptr += linerem;
			}
		} else buf = (unsigned char *)args->srcaddr;

		GrArea(args->wid, args->gc, DSTX, DSTY, DSTW, DSTH, buf, MWPF_TRUECOLOR888);
		GrFlush();

		if (buf != args->srcaddr)
			free(buf);
	}
}
#endif /* LATER*/

static void
ConvBlit32(PBLITARGS args)
{
	unsigned long *ilineptr;
	unsigned long *olineptr;
	unsigned int x, y;

	if (args->rop == MWPORTRAIT_RIGHT) {
		/* right portrait mode conversion*/
		unsigned int linerem = SPITCH/4 - SRCW;
		ilineptr = (unsigned long *)(((char *)args->srcaddr) +
			SRCY*SPITCH + SRCX*BYTESPP);
		olineptr = (unsigned long *)args->dstaddr;
		for (y=0; y<DSTH; ++y) {
			unsigned int ox =
				(DSTH-1-y + DSTXRES-DSTH - (DSTY+SRCY)) * BYTESPP;
			for (x=0; x<DSTW; ++x) {
				unsigned int oy = (x + DSTX + SRCX) * DPITCH;
				olineptr[(oy + ox)/4] = *ilineptr++;
			}
			ilineptr += linerem;
		}
	} 
	else if (args->rop == MWPORTRAIT_LEFT) {
		/* left portrait mode conversion*/
		unsigned int linerem = SPITCH/4 - SRCW;
		ilineptr = (unsigned long *)((char *)args->srcaddr + SRCY*SPITCH + SRCX*BYTESPP);
		olineptr = (unsigned long *)args->dstaddr;
		for (y=0; y<DSTH; ++y) {
			unsigned int ox = (y + DSTY + SRCY) * BYTESPP;
			for (x=0; x<DSTW; ++x) {
				unsigned int oy =
				    (DSTW-1-x + DSTYRES-DSTW - (DSTX+SRCX)) * DPITCH;
				olineptr[(oy + ox)/4] = *ilineptr++;
			}
			ilineptr += linerem;
		}
	}
	else if (args->rop == MWPORTRAIT_NONE) {
		/* straight blit, no conversion*/
		unsigned int srclinerem = SPITCH/4;
		unsigned int dstlinerem = DPITCH/4;
		unsigned int srcoffset = SRCY*SPITCH + SRCX*BYTESPP;
		unsigned int dstoffset = DSTY*DPITCH + DSTX*BYTESPP;
		ilineptr = (unsigned long *)((char *)args->srcaddr + srcoffset);
		olineptr = (unsigned long *)((char *)args->dstaddr + dstoffset);
		for (y=0; y<DSTH; ++y) {
			memcpy(olineptr, ilineptr, DSTW<<2);
			olineptr += dstlinerem;
			ilineptr += srclinerem;
		}
	}
	else {
		unsigned char *buf;

		/* check pitch/bytespp same, no malloc*/
		if (SRCY || SRCX || (SPITCH != DPITCH)) {
			/* straight blit, convert pitch, don't draw on framebuffer*/
			unsigned int linerem = SPITCH/4;
			unsigned int srcoffset = SRCY*SPITCH + SRCX*BYTESPP;
			ilineptr = (unsigned long *)((char *)args->srcaddr + srcoffset);
			buf = (unsigned char *)malloc(DSTW * DSTH * 4);
			olineptr = (unsigned long *)buf;
			for (y=0; y<DSTH; ++y) {
				memcpy(olineptr, ilineptr, DSTW<<2);
				olineptr += DSTW;
				ilineptr += linerem;
			}
		} else buf = (unsigned char *)args->srcaddr;

		GrArea(args->wid, args->gc, DSTX, DSTY, DSTW, DSTH, buf, MWPF_TRUECOLOR0888);
		GrFlush();

		if (buf != args->srcaddr)
			free(buf);
	}
}

static void
ConvBlit(PBLITARGS args)
{
	assert(args->dstbpp == args->srcbpp);
	assert(args->dstbytespp == args->srcbytespp);

	switch (args->dstbpp) {
	case 16:
		ConvBlit16(args);
		break;
#if LATER
	case 24:
		ConvBlit32(args);
		break;
#endif
	case 32:
		ConvBlit32(args);
		break;
	default:
		printf("No conversion blitter for %d bpp\n", args->dstbpp);
		break;
	}
}
//--------------------------------------------

DisplayImage_X11::DisplayImage_X11()
{
  d_data = new DisplayImage_Data;
}


void DisplayImage_X11::UseMITSHM(bool flag=true) { d_data->mayUseMITSHM=flag; }

XImage& DisplayImage_X11::AskXImage() { assert(d_data->d_ximg); return *d_data->d_ximg; }


static XImage ximage;

void DisplayImage_X11::Create(int w,int h,GR_WINDOW_ID win,const X11Server* server)
{
  int bpp = 0;
  int red_mask, green_mask, blue_mask;
  GR_SCREEN_INFO sinfo;

  if (d_data->d_initialized)
    return;

  d_data->d_width  = w;
  d_data->d_height = h;
  int roundedwidth  = (w+15); roundedwidth  -= roundedwidth %16;

  d_data->d_win = win;
  d_data->d_gc = GrNewGC();
  d_data->d_UseShmExt=false;
  d_data->d_WaitForCompletion=false;

  // Create XImage structure
  // try 16,15,24,32, then 8
	GrGetScreenInfo(&sinfo);
	d_data->d_ximg = &ximage;
	d_data->d_ximg->width = roundedwidth;
	d_data->d_ximg->height = h;
	switch (sinfo.pixtype) {
	case MWPF_TRUECOLOR0888:
	case MWPF_TRUECOLOR565:
	case MWPF_TRUECOLOR555:
		bpp = sinfo.bpp;
		red_mask = sinfo.rmask;
		green_mask = sinfo.gmask;
		blue_mask = sinfo.bmask;
		break;
	default:
		printf("Unsupported screen bits per pixel\n");
		break;
	}
	d_data->d_ximg->bits_per_pixel = bpp;
	d_data->d_ximg->red_mask = red_mask;
	d_data->d_ximg->green_mask = green_mask;
	d_data->d_ximg->blue_mask = blue_mask;
	// note: must be dword right-padded for later blit?
	d_data->d_ximg->bytes_per_line = roundedwidth * bpp/8;
	d_data->d_ximg->byte_order = LSBFirst;		// FIXME, add to SCREENINFO
	d_data->d_data = new uint8[d_data->d_ximg->bytes_per_line*h];
	d_data->d_ximg->data = (char*)d_data->d_data;

	if (GrOpenClientFramebuffer() == NULL)
		printf("Can't directly map framebuffer, using GrArea\n");

  d_data->d_initialized = true; 
}


DisplayImage_X11::~DisplayImage_X11()
{
  GrCloseClientFramebuffer();
  delete d_data;
}

void DisplayImage_X11::PutImage(int srcx0,int srcy0,int w,int h, int dstx0,int dsty0)
{
#if 0
  if (d_data->d_WaitForCompletion)
    while (1)
      {
        XEvent xev;
        
        XNextEvent(d_data->d_display, &xev);
        if (xev.type == d_data->d_CompletionType)
          break;
      }
#endif

      if (w==0) w=d_data->d_width;
      if (h==0) h=d_data->d_height;

#if 0
      // FIXME must deal with srcx0, srcy0
      if (srcx0 || srcy0) cout << "srcx0/srcy0 NOT 0!!\n";
      int type = (d_data->d_ximg->bits_per_pixel == 32)?
      	MWPF_TRUECOLOR0888: MWPF_TRUECOLOR565;
      GrArea(d_data->d_win, d_data->d_gc, dstx0, dsty0, w, h,
      	d_data->d_ximg->data, type);
      GrFlush();
#else
	GR_WINDOW_FB_INFO fbinfo;
	BLITARGS	  args;
	GrGetWindowFBInfo(d_data->d_win, &fbinfo);

	if (::fullscreen)
		args.rop = fbinfo.physpixels? MWPORTRAIT_NONE: MWPORTRAIT_GRAREA;
	else args.rop = fbinfo.physpixels? fbinfo.portrait_mode: MWPORTRAIT_GRAREA;

	args.dstx = dstx0;
	args.dsty = dsty0;
	args.dstaddr = fbinfo.physpixels;	/* can't use winpixels for portrait conversion*/
	if (args.rop != MWPORTRAIT_GRAREA) {
		/* dstx is framebuffer relative for non-GrArea*/
		args.dstx += fbinfo.x;
		args.dsty += fbinfo.y;
	}
	args.dstw = w;
	args.dsth = h;
	args.dstbpp = fbinfo.bpp;
	args.dstbytespp = fbinfo.bytespp;
	if (args.rop == MWPORTRAIT_GRAREA)
		args.dstpitch = d_data->d_ximg->bytes_per_line;
	else args.dstpitch = fbinfo.pitch;
	args.dstxres = fbinfo.xres;
	args.dstyres = fbinfo.yres;

	args.srcaddr = d_data->d_ximg->data;
	args.srcx = srcx0;
	args.srcy = srcy0;
	args.srcw = d_data->d_ximg->width;
	args.srch = d_data->d_ximg->height;
	args.srcbpp = d_data->d_ximg->bits_per_pixel;
	args.srcbytespp = (d_data->d_ximg->bits_per_pixel+7)/8;
	args.srcpitch = d_data->d_ximg->bytes_per_line;

	args.wid = d_data->d_win;
	args.gc = d_data->d_gc;

// FIXME REMOVE
if (args.dsth != args.srch) printf("src != dst h\n");
if (args.dstw != args.srcw) printf("src != dst w\n");

	ConvBlit(&args);
#endif
}
