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

#include "tests/videodec.hh"
#include "input/streamsrc_istr.hh"
#include "input/errors.hh"
#include "system/sysdec1.hh"
#include "system/resync.hh"
#include "system/sysdemux.hh"
#include "system/videoauconv.hh"
#include "system/syncer.hh"
//#include "video12/vidsyndec.hh"
//#include "video12/viddec_std.hh"
#include "video12/vdecoder.hh"
#include "vpostproc/buffer.hh"

#include "vpostproc/pp_mblks.hh"

#include "error.hh"
#include "libvideogfx/nanox/imgwin.hh"

#include <iostream.h>
#include <iomanip.h>


void ShowVideoSyntax(istream& istr)
{
  StreamSource_IStream strsrc;
  strsrc.SetIStream(istr);

  ShowVideoSyntax(strsrc);
}


void ShowVideoSyntax(class StreamSource& strsrc)
{
#if 0
  SystemDecoder_MPEG1 sysdec;
  sysdec.SetSource(strsrc);

  SystemResyncer resync;
  resync.SetSource(&sysdec);

  SystemDemux demux(resync);
  demux.EnableChannel(StreamIDBase_Video|0);

  SystemDemuxExtractor extract;
  extract.SetSource(&demux);
  extract.SetChannel(StreamIDBase_Video|0);

  VideoAUConverter vidcon(extract);

  VideoDecoder_Log   dec_log;
  VideoSyntaxDecoder vidsyndec(vidcon,dec_log);


  while (vidsyndec.DecodeAFrame())
    {
    }
#endif
}






#if 0
void DecodeVideo(class istream& istr)
{
  StreamSource_IStream strsrc;
  strsrc.SetIStream(istr);

  DecodeVideo(strsrc);
}


void DecodeVideo(class StreamSource& strsrc)
{
  SystemDecoder_MPEG1 sysdec;
  sysdec.SetSource(strsrc);

  SystemResyncer resync;
  resync.SetSource(&sysdec);

  SystemDemux demux(resync);
  demux.EnableChannel(StreamIDBase_Video|0);

  SystemDemuxExtractor extract;
  extract.SetSource(&demux);
  extract.SetChannel(StreamIDBase_Video|0);

  VideoAUConverter vidcon(extract);

  VideoDecoder_Standard   dec_std;
  VideoSyntaxDecoder vidsyndec(vidcon,dec_std);


  ImageWindow_Autorefresh_X11 imgwin;
  bool first=true;

  int w,h;

  int nframes=0;
  while (1) //nframes>0)
    {
      if (dec_std.BufferEmpty())
	{
	  bool more = vidsyndec.DecodeAFrame();
	  if (!more)
	    break;
	}
      else
	{
	  DecodedImageData dimg;
	  dimg = dec_std.GetNextImage();

#if 1
	  if (first)
	    {
	      first=false;

	      ImageParam_YUV param;
	      dimg.m_image.GetParam(param);
	      imgwin.Create(w=param.width,h=param.height,
			    "DVDview DecodeVideo-TEST  (c) Dirk Farin");
	    }

	  imgwin.Display(dimg.m_image);
#endif
          nframes++;
	}
    }

  cout << "Decoded " << nframes << " frames.\n";
}
#endif






void DecodeVideo(class istream& istr)
{
  StreamSource_IStream strsrc;
  strsrc.SetIStream(istr);

  DecodeVideo(strsrc);
}

#include <fstream.h>

#define USEDGA 0

#if USEDGA
#include "output/xf86dga.hh"
#endif

void DecodeVideo(class StreamSource& strsrc)
{
  SystemDecoder_MPEG1 sysdec;
  sysdec.SetSource(strsrc);

  SystemResyncer resync;
  resync.SetSource(&sysdec);

  SystemDemux demux(resync);
  demux.EnableChannel(StreamIDBase_Video|0);

  SystemDemuxExtractor extract;
  extract.SetSource(&demux);
  extract.SetChannel(StreamIDBase_Video|0);


  VideoAUConverter vidcon(extract);

  VideoDecoder  dec_fast(vidcon);
  Syncer_Realtime syncer;

  //dec_fast.SetSyncer(&syncer);
  VideoPostprocessor_MBBoundaries pp_mblks;

  VideoImageBuffer bufprov;
  pp_mblks.SetNext(&bufprov);
  dec_fast.SetBufferProvider(pp_mblks);

  //pp_mblks.SetBigMarks();
  pp_mblks.SetStaticMode();

#if USEDGA
  xf86out_init();
#else
  ImageWindow_Autorefresh_X11 imgwin;
#endif

  bool first=true;

  int w,h;

  int nframes=0;
  try
    {
      while (1) //nframes>0)
	{
#if 1
	  if (bufprov.BufferEmpty())
	    {
	      //cout << "DECODE\n";
	      bool more = dec_fast.DecodeAFrame();
	      if (!more)
		break;
	    }
	  else
#endif
	    {
	      DecodedImageData* dimg;
	      dimg = bufprov.GetNextImage();

	      //cout << "SHOW\n";

	      if (first)
		{
		  first=false;

		  ImageParam_YUV param;
		  dimg->m_image.GetParam(param);
		  cout << "w,h: " << param.width << "," << param.height << endl;
#if !USEDGA
		  imgwin.Create(w=param.width,h=param.height,
				"DVDview DecodeVideo-TEST");
#endif
		}

#if USEDGA
	      xf86out_display(dimg->m_image);
#else
	      imgwin.Display(dimg->m_image);
#endif

	      bufprov.FreeImage(dimg);

	      //getchar();

	      nframes++;
	    }
	}
    }
  catch(...)
    {
      cout << "Unexpected end. Decoded up to frame: " << nframes << endl;
      throw;
    }

  cout << "Frame: " << nframes << endl;

#if USEDGA
  xf86out_close();
#endif
  cout << "Decoded " << nframes << " frames.\n";
}
