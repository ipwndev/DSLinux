/********************************************************************************
    Copyright (C) 1999  Dirk Farin

    This program is distributed under GNU Public License (GPL) as
    outlined in the COPYING file that comes with the source distribution.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************************************/

#include "output/out_mgavid.hh"
#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>


VideoSink_MGA::VideoSink_MGA()
  : first(true), scaled(false), width(0), height(0), pic_available(false)
{
  fileh = open("/dev/mga_vid",O_RDWR);
}


VideoSink_MGA::~VideoSink_MGA()
{
  if (fileh>0) close(fileh);
}


#include <libvideogfx/graphics/fileio/write_yuv.hh>

void VideoSink_MGA::ShowMBRows(DecodedImageData* dimg)
{
  const Image_YUV<Pixel>& img = dimg->m_image;

  nextpts = dimg->m_timing.pts;

  if (dimg->m_width*dimg->m_height > 1024*680)
    {
      /* TODO: how can we detect if MGA will work? */
      MessageDisplay::Show(ErrSev_Warning,"image size could be too large for MGA device");
    }

  /*
  cout << "MGA-Show lines from " << dimg->m_src_y_start << " to "
       << dimg->m_src_y_end << " at " << dimg->m_dst_y_start
       << (dimg->m_field_lines ? " as field" : " as frame") << endl;
  */

  assert(fileh>0);

  ImageParam_YUV param;
  img.GetParam(param);

  if (first)
    {
      first=false;

      assert(param.chroma==Chroma420 || param.chroma==Chroma422);
      if (param.chroma==Chroma422)
	{
	  MessageDisplay::Show(ErrSev_Warning,"displaying 4:2:2 video on 4:2:0 MGA device");
	}

      config.src_width   = param.width;
      config.src_height  = param.height;
      if (scaled)
	{
	  config.dest_width  = width;
	  config.dest_height = height;
	}
      else
	{
	  config.dest_width  = param.width;
	  config.dest_height = param.height;
	}

      config.x_org = 0;
      config.y_org = 0;
      config.colkey_on = 0;

      if (ioctl(fileh,MGA_VID_CONFIG,&config))
	{
	  perror("error in MGA_VID_CONFIG.\n");
	}

      if (ioctl(fileh,MGA_VID_ON,0))
	{
	  perror("error in MGA_VID_ON.\n");
	}
      base = (uint_8*)mmap(0,256*4096,PROT_WRITE,MAP_SHARED,fileh,0);
      if (base==MAP_FAILED)
	{
	  perror("error in mmap of MGA memory.\n");
	}
    }

  const Pixel*const* yp = img.AskFrameY_const();
  const Pixel*const* up = img.AskFrameU_const();
  const Pixel*const* vp = img.AskFrameV_const();

  int lineskip = (dimg->m_field_lines ? 2 : 1);

  int chroma_vfact = 2/ChromaSubV(param.chroma);

  if (config.card_type == MGA_G400)
    {
      int bes_pitch = (config.src_width+31) & ~31;
      uint_8* dest = base;
      int hoffs;

      dest += dimg->m_dst_y_start*bes_pitch;

      for (int h=dimg->m_src_y_start;h<=dimg->m_src_y_end;h+=lineskip)
	{
	  __builtin_memcpy(dest,yp[h],config.src_width);
	  dest += bes_pitch*lineskip;
	}

      dest = base+bes_pitch*config.src_height;
      dest += (dimg->m_dst_y_start+1)/2*bes_pitch/2;

      for (int h=(dimg->m_src_y_start+1)/2;h<=dimg->m_src_y_end/2;h+=lineskip)
	{
	  __builtin_memcpy(dest,up[h*chroma_vfact],config.src_width/2);
	  dest += bes_pitch/2*lineskip;
	}

      dest = base+bes_pitch*5*config.src_height/4;
      dest += (dimg->m_dst_y_start+1)/2*bes_pitch/2;

      for (int h=(dimg->m_src_y_start+1)/2;h<=dimg->m_src_y_end/2;h+=lineskip)
	{
	  __builtin_memcpy(dest,vp[h*chroma_vfact],config.src_width/2);
	  dest += bes_pitch/2*lineskip;
	}
    }
  else
    {
      // No G400 so we have a g200 don't we  ?

      assert(config.card_type == MGA_G200);  // Just to be sure :)  G450?

      int bespitch = (config.src_width + 31) & ~31;
      uint_8* dest = base + dimg->m_dst_y_start*bespitch;

      for(int h=dimg->m_src_y_start; h <= dimg->m_src_y_end; h+=lineskip)
	{
	  __builtin_memcpy(dest, yp[h],config.src_width);
	  dest += bespitch*lineskip;
	}

      dest = base+bespitch*config.src_height;
      dest += (dimg->m_dst_y_start+1)/2*bespitch;

      for (int h=(dimg->m_src_y_start+1)/2;h<=dimg->m_src_y_end/2;h+=lineskip)
        {
#if ENABLE_MMX
	  const Pixel*  cb = up[h*chroma_vfact];
	  const Pixel*  cr = vp[h*chroma_vfact];
	  uint_8* mem = dest;

	  for(int w=0; w < config.src_width/2/8; w++)
	    {
	      asm volatile (
			    "movq       (%1),%%mm0\n\t"
			    "movq       %%mm0,%%mm1\n\t"
			    "punpcklbw  (%2),%%mm0\n\t" 
			    "punpckhbw  (%2),%%mm1\n\t" 
			    "movq       %%mm0,(%0)\n\t"
			    "movq       %%mm1,8(%0)\n\t"
			    : : "r" (mem), "r" (cb),"r" (cr)
#if PGCC_COMPILER
			    : "mm0","mm1"
#endif
			    );
	      cb += 8;
	      cr += 8;
	      mem += 16;
	    }

	  dest += bespitch*lineskip;
#else
	  uint_8* mem = dest;

	  for(int w=0; w < config.src_width/2; w++)
	    {
	      *mem++ = up[h*chroma_vfact][w];
	      *mem++ = vp[h*chroma_vfact][w];
	    }

	  dest += bespitch*lineskip;
#endif
	}
    }
}

