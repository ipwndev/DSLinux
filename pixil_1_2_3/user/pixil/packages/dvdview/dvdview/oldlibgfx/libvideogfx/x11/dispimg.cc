/*
 *  dispimg.cc
 */

#include "config.h"

#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include <iostream.h>
#include <iomanip.h>

#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>

extern "C" {
#include <stdio.h>
extern int XShmGetEventBase(Display*);   // TODO: where can this function declaration be found ?
}

#include "libvideogfx/types.hh"

#include "server.hh"
#include "dispimg.hh"


struct DisplayImage_Data
{
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
	  if (d_UseShmExt)
	    {
	      XShmDetach(d_display,&d_ShmSegInfo);
	      XDestroyImage(d_ximg);
	      shmdt(d_ShmSegInfo.shmaddr);
	      shmctl(d_ShmSegInfo.shmid,IPC_RMID,0);
	    }
	  else
	    {
	      d_ximg->data = NULL;
	      XDestroyImage(d_ximg);
	      if (d_data) delete[] d_data;
	    }

	  XFreeGC(d_display, d_gc);
	}
    }

  // user parameters

  bool mayUseMITSHM;

  // internal parameters

  bool        d_initialized;
  Display*    d_display;

  bool        d_UseShmExt;

  Window   d_win;
  GC       d_gc;

  XImage*  d_ximg;
  uint8*   d_data;
  XShmSegmentInfo d_ShmSegInfo;
  bool     d_WaitForCompletion;
  int      d_CompletionType;

  int d_width,d_height;
};


static int shmmajor;

static bool shmfailed;
static int shmhandler(Display* display,XErrorEvent* err)
{
  if (err->request_code == shmmajor &&
      err->minor_code == X_ShmAttach)
    shmfailed=true;

  return 0;
}


DisplayImage_X11::DisplayImage_X11()
{
  d_data = new DisplayImage_Data;
}


void DisplayImage_X11::UseMITSHM(bool flag=true) { d_data->mayUseMITSHM=flag; }

XImage& DisplayImage_X11::AskXImage() { assert(d_data->d_ximg); return *d_data->d_ximg; }


void DisplayImage_X11::Create(int w,int h,Window win,const X11Server* server)
{
  if (d_data->d_initialized)
    return;

  d_data->d_width  = w;
  d_data->d_height = h;
  int roundedwidth  = (w+15); roundedwidth  -= roundedwidth %16;
  //int roundedheight = (h+15); roundedheight -= roundedheight%16;


  // Connect to X11 server

  if (server)
    d_data->d_display = server->AskDisplay();
  else
    d_data->d_display = default_x11server.AskDisplay();

  int screen = DefaultScreen(d_data->d_display);
  d_data->d_win = win;


  // Choose VisualInfo

  XWindowAttributes winattr;
  XGetWindowAttributes(d_data->d_display,win,&winattr);

  VisualID visualid = XVisualIDFromVisual(winattr.visual);
  XVisualInfo vinfo_template;
  vinfo_template.visualid   = visualid;

  XVisualInfo* vinfo;
  int nvinfos;
  vinfo=XGetVisualInfo(d_data->d_display,VisualIDMask,&vinfo_template,&nvinfos);
  assert(vinfo != NULL);
  assert(nvinfos==1);

  //cout << "VISUAL-ID used for image: 0x" << hex << visualid << dec << endl;

  XGCValues gcvals;
  d_data->d_gc = XCreateGC(d_data->d_display,win,0,&gcvals);


  // Set Colormap

  // TODO

  // Create XImage structure

  if (d_data->mayUseMITSHM && XShmQueryExtension(d_data->d_display))
    {
      int dummy;
      int major_version,minor_version,pixmap_flag;

      if (!XShmQueryVersion(d_data->d_display, &major_version, &minor_version,&pixmap_flag)
	  || !XQueryExtension(d_data->d_display, "MIT-SHM", &dummy, &dummy, &dummy))
	{
	  // ShowNote(ErrSev_Note,"X11 shared memory (MITSHM) extension not supported");
	  d_data->d_UseShmExt=false;
        }
      else
	{
	  //char buffer[1000];
	  //sprintf(buffer,"X11 shared memory (MITSHM) extensions version %d.%d detected.",major_version, minor_version);
	  //ShowNote(ErrSev_Note,buffer);
	  d_data->d_UseShmExt = true;
        }
    }
  else
    {
      /*
      if (mayUseMITSHM)
	ShowNote(ErrSev_Note,"X11 shared memory (MITSHM) extension not supported");
      else
	ShowNote(ErrSev_Note,"X11 shared memory (MITSHM) extension disabled");
      */

      d_data->d_UseShmExt=false;
    }

tryagain:
  if (d_data->d_UseShmExt)
    {
      d_data->d_ximg = XShmCreateImage(d_data->d_display,winattr.visual,vinfo->depth,ZPixmap,(char*)0,
				       &d_data->d_ShmSegInfo,roundedwidth,h);
      if (!d_data->d_ximg)
        { 
	  //TODO
	  assert(0);
	  //throw Excpt_Base(ErrSev_Error,"XShmCreateImage failed");
	}

      d_data->d_ShmSegInfo.shmid    = shmget(IPC_PRIVATE,d_data->d_ximg->bytes_per_line*h, IPC_CREAT|0604);
      if (d_data->d_ShmSegInfo.shmid==-1)
	{ perror("shmget failed: "); assert(0); } // throw Excpt_Base(ErrSev_Error,"shmget failed"); }
      d_data->d_ShmSegInfo.shmaddr  = d_data->d_ximg->data = (char*)shmat(d_data->d_ShmSegInfo.shmid,0,0);
      if (d_data->d_ShmSegInfo.shmaddr==((char *)-1))
        { perror("shmat failed: "); assert(0); } // throw Excpt_Base(ErrSev_Error,"shmat failed"); }
      d_data->d_ShmSegInfo.readOnly = True;

      int dummy;
      XQueryExtension(d_data->d_display,"MIT-SHM",&shmmajor,&dummy,&dummy);

      shmfailed=false;
      XSetErrorHandler(shmhandler);

      Status xshma;
      xshma=XShmAttach(d_data->d_display,&d_data->d_ShmSegInfo);
      XSync(d_data->d_display,False);

      XSetErrorHandler(NULL);

      shmctl(d_data->d_ShmSegInfo.shmid, IPC_RMID, 0);

      if (!xshma)
	{ assert(0); } // throw Excpt_Base(ErrSev_Error,"XShmAttach failed");

      if (shmfailed)
	{
	  cout << "MIT-SHM failed, falling back to network mode.\n";
          XDestroyImage(d_data->d_ximg);
          shmdt(d_data->d_ShmSegInfo.shmaddr);
          shmctl(d_data->d_ShmSegInfo.shmid,IPC_RMID,0);
	  d_data->d_UseShmExt = false;
	  goto tryagain;
	}
      
      d_data->d_data = (uint8*)d_data->d_ximg->data;
      d_data->d_CompletionType = XShmGetEventBase(d_data->d_display) + ShmCompletion;
    }
  else
    {
      d_data->d_ximg = XCreateImage(d_data->d_display,vinfo->visual,vinfo->depth,
				    ZPixmap,0,NULL,roundedwidth,h,32,0);
      d_data->d_data = new uint8[d_data->d_ximg->bytes_per_line*h];
      d_data->d_ximg->data = (char*)d_data->d_data;
    }

  d_data->d_WaitForCompletion=false;

  XSync(d_data->d_display,False);

  XFree(vinfo);

  d_data->d_initialized = true; 
}


DisplayImage_X11::~DisplayImage_X11()
{
  delete d_data;
}


void DisplayImage_X11::PutImage(int srcx0,int srcy0,int w,int h, int dstx0,int dsty0)
{
  if (w==0) w=d_data->d_width;
  if (h==0) h=d_data->d_height;

  if (d_data->d_WaitForCompletion)
    while (1)
      {
        XEvent xev;
        
        XNextEvent(d_data->d_display, &xev);
        if (xev.type == d_data->d_CompletionType)
          break;
      }

  if (d_data->d_UseShmExt)
    {
      XShmPutImage(d_data->d_display, d_data->d_win, d_data->d_gc, d_data->d_ximg, srcx0, srcy0, dstx0, dsty0, w,h, True);

      XFlush(d_data->d_display);
      d_data->d_WaitForCompletion=true;
    }
  else
    {
      XPutImage(d_data->d_display, d_data->d_win, d_data->d_gc, d_data->d_ximg, srcx0, srcy0, dstx0, dsty0, w,h);
      XFlush(d_data->d_display);
    }
}

