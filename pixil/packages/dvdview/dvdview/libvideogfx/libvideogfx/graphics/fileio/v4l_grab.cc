/*
 *  v4l_grab.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/videodev.h>
#include <string.h>

#include "v4l_grab.hh"


struct GrabData
{
  unsigned char* d_map;
  struct video_mbuf d_vidmbuf;
  struct video_mmap d_vidmmap;
};

V4L_Grabber::V4L_Grabber()
  : d_device("/dev/video"),
    d_fd(-1),
    d_border(0)
{
  d_width  = 384;
  d_height = 288;

  d_grabdata = new GrabData;
}

V4L_Grabber::~V4L_Grabber()
{
  if (d_fd>=0)
    close(d_fd);

  delete d_grabdata;
}

  // initialization

void V4L_Grabber::SetDevice(const char* device)
{
  d_device=device;
}

void V4L_Grabber::SetResolution(int w,int h)
{
  d_width  = w;
  d_height = h;
}

void V4L_Grabber::StartGrabbing(bool greyscale)
{
  d_fd = open(d_device,O_RDWR);
  if (d_fd==-1)
    { perror("open video-device: "); exit(10); }

  if (-1 == ioctl(d_fd,VIDIOCGMBUF,&d_grabdata->d_vidmbuf)) {
    perror("ioctl VIDIOCGMBUF");
  }

  d_greyscale = greyscale;

  d_grabdata->d_map = (unsigned char*)mmap(0,d_grabdata->d_vidmbuf.size,
					   PROT_READ|PROT_WRITE,MAP_SHARED,d_fd,0);
  if ((unsigned char*)-1 == d_grabdata->d_map) {
    perror("mmap on video device");
  }



  // Grabbing starten

  d_grabdata->d_vidmmap.width =d_width;
  d_grabdata->d_vidmmap.height=d_height;
  d_grabdata->d_vidmmap.format = (greyscale ? VIDEO_PALETTE_GREY : VIDEO_PALETTE_YUV422);

  for (int i=0;i<d_grabdata->d_vidmbuf.frames;i++)
    {
      d_grabdata->d_vidmmap.frame =i;
      if (-1 == ioctl(d_fd,VIDIOCMCAPTURE,&d_grabdata->d_vidmmap)) {
        perror("ioctl VIDIOCMCAPTURE");
        exit(10);
      }
    }

  d_nextbuf=0;
}

void V4L_Grabber::Grab(Image_YUV<Pixel>& img)
{
  ImageSpec_YUV spec;
  spec.width  = d_width;
  spec.height = d_height;
  spec.nocolor = (d_greyscale==true);
  spec.halign  = 16;
  spec.chroma  = Chroma420;
  spec.border  = d_border;
  spec.ImageInfo_Alignment::operator=(d_align);

  img.Create(spec);


  if (-1 == ioctl(d_fd,VIDIOCSYNC,&d_nextbuf)) {
    perror("ioctl VIDIOCSYNC");
    exit(10);
  }

  if (d_greyscale)
    {
      Pixel*const* yp=img.AskFrameY();

      unsigned char* mapptr=d_grabdata->d_map + d_grabdata->d_vidmbuf.offsets[d_nextbuf];

      for (int y=0;y<d_height;y++)
	{
	  memcpy(yp[y],mapptr,d_width);
	  mapptr += d_width;
	}
    }
  else
    {
      Pixel*const* yp=img.AskFrameY();
      Pixel*const* up=img.AskFrameU();
      Pixel*const* vp=img.AskFrameV();

      unsigned char* mapptr=d_grabdata->d_map + d_grabdata->d_vidmbuf.offsets[d_nextbuf];

      for (int y=0;y<d_height;y++)
	{
	  for (int x=0;x<d_width;x++)
	    {
	      yp[y   ][x]    = *mapptr++;
	      up[y>>1][x>>1] = *mapptr++;
	      x++;
	      yp[y   ][x]    = *mapptr++;
	      vp[y>>1][x>>1] = *mapptr++;
	    }
	  y++;
	  for (int x=0;x<d_width;x++)
	    {
	      yp[y][x] = *mapptr;
	      mapptr++;
	      mapptr++;
	    }
	}
    }

  d_grabdata->d_vidmmap.frame =d_nextbuf;
  if (-1 == ioctl(d_fd,VIDIOCMCAPTURE,&d_grabdata->d_vidmmap)) {
    perror("ioctl VIDIOCMCAPTURE");
    exit(10);
  }

  d_nextbuf=1-d_nextbuf;
}
