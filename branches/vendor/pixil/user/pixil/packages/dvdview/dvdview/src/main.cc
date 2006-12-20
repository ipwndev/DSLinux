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


#include "error.hh"
#include "tests/system1.hh"
#include "tests/memalloc.hh"
#include "tests/vidaccess.hh"
#include "tests/videodec.hh"
#include "libvideogfx/init.hh"
#include "config.h"

#include "input/streamsrc_istr.hh"
#include "input/streamsrc_cin.hh"
#include "input/errors.hh"
#include "system/sysdec1.hh"
#include "system/resync.hh"
#include "system/sysdemux.hh"
#include "system/videoauconv.hh"
#include "system/syncer.hh"
#include "system/ptsvideo.hh"
#include "video12/vdecoder.hh"
#include "vpostproc/buffer.hh"
#if 0
#include "vpostproc/pp_imgtype.hh"
#endif
#include "vpostproc/pp_resize.hh"
#include "vpostproc/pp_filteredresize.hh"
#include "vpostproc/pp_mblks.hh"
#include "vpostproc/pp_mv.hh"
#include "vpostproc/pp_mvcol.hh"
#include "vpostproc/pp_qscale.hh"
#include "vpostproc/pp_fps.hh"
#include "output/out_x11.hh"
#include "output/out_ppm.hh"
#include "output/out_yuv.hh"
#include "version.hh"

#if LINUX
#include "output/out_mgavid.hh"
#include "input/streamsrc_linux_vcd.hh"
#endif

#include "libvideogfx/nanox/imgwin.hh"
#include "libvideogfx/nanox/server.hh"


#include "options.hh"

#include <iostream.h>
#include <fstream.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if 1
#include "libvideogfx/arch/x86/CPUcapabilities.hh"
#endif

class DvdApp 
{
protected:
  // stream sources

   StreamSource_IStream strsrc_istr;
   StreamSource_cin     strsrc_cin;
#if LINUX
   StreamSource_VCD     strsrc_vcd;
#endif
   StreamSource* streamsrc;


  // the system decoder
  
   SystemDecoder_MPEG1 system_dec;
   SystemResyncer      system_resync;
   SystemDemux         system_demux;
  
  // video decoding pipeline
  
   SystemDemuxExtractor video_extractor;
   VideoAUConverter     vidcon;
   PTSVideoCompleter    video_ptsvideo;
   VideoDecoder         video_dec;
  
  // audio decoding pipeline
  
   SystemDemuxExtractor audio_extractor;
   SystemDemuxExtractor ac3_extractor;
   int ac3stream;
  
  // video postprocessors
  
   DecodedPictureSink* lastoutput;
   Resize_Interface*   resizer;
   VideoPostprocessor_MBBoundaries   pp_mblks;
   VideoPostprocessor_MotionVector   pp_mv;
   VideoPostprocessor_MVCol          pp_mvcol;
   VideoPostprocessor_QScale         pp_qscale;
   VideoPostprocessor_FramesPerSec   pp_fps;
   VideoPostprocessor_Resize         pp_resize;
   VideoPostprocessor_FilteredResize pp_filteredresize;
#if 0
   VideoPostprocessor_ImageType    pp_imgtype;
#endif

  VideoOutput* outputobject;  
  VideoSink_YUVFile videosink_yuv;
  VideoSink_PPMFile videosink_ppm;
#if LINUX
  VideoSink_MGA videosink_mga;
#endif
  
  // synchronization
  
   Syncer_Realtime syncer_realtime;
   Syncer* syncer;

  // command line args 
  static   bool imgtypes;
  static   bool qscales;
  static   bool mvs;
  static   bool mvcol;
  static   bool audio;
  static   bool ac3;
  static   bool mvf;
  static   bool mvb;
  static   bool mvcolor;
  static   bool mvpf;
  static   bool mvbf;
  static   bool mv_holdmode;
  static   bool fpsdisplay;
  static   bool x11_output;   // try MGA first
  static   int  vcd;
  static   int  maxframes;
  static   bool resize;

  static   bool rescale;
  static   bool filtered_resize;

  static   bool _initialized;

public:

  void InitStreamSource(ifstream& istr);
  void InitVideoDecoder1();
  void InitStreamSource_cin();
  void InitStreamSourceVCD(int track);
  void InitAudioDecoder();
  void InitAC3Decoder();
  void InsertPostprocessor(VideoPostprocessor& pp);
  void InitVideoDecoder2();
  void ViewMPEG(bool audio,bool ac3);
  DvdApp(int argc,char** argv);

  void InitOptions();
};

bool DvdApp::imgtypes;
bool DvdApp::qscales;
bool DvdApp::mvs;
bool DvdApp::mvcol;
bool DvdApp::audio;
bool DvdApp::ac3;
bool DvdApp::mvf;
bool DvdApp::mvb;
bool DvdApp::mvcolor;
bool DvdApp::mvpf;
bool DvdApp::mvbf;
bool DvdApp::mv_holdmode;
bool DvdApp::fpsdisplay;
bool DvdApp::x11_output;
int  DvdApp::vcd;
int  DvdApp::maxframes;
bool DvdApp::resize;
bool DvdApp::rescale;
bool DvdApp::filtered_resize;
bool DvdApp::_initialized;
static   bool loop;
extern bool fullscreen;

// video output
static   VideoSink_X11 videosink_x11;

void DvdApp::InitStreamSource(ifstream& istr)
{
  strsrc_istr.SetIStream(istr);
  streamsrc=&strsrc_istr;
}

void DvdApp::InitStreamSource_cin()
{
  streamsrc=&strsrc_cin;
}


void DvdApp::InitStreamSourceVCD(int track)
{
#if LINUX
  strsrc_vcd.Init();
  if (track<0 || track>strsrc_vcd.AskNTracks())
    throw "VCD track out of range";

  strsrc_vcd.SkipToTrack(track);
  streamsrc=&strsrc_vcd;
#endif
}


void DvdApp::InitVideoDecoder1()
{
  system_dec.SetSource(*streamsrc);
  system_resync.SetSource(&system_dec);

  system_demux.EnableChannel(StreamIDBase_Video|0);

  video_extractor.SetSource(&system_demux);
  video_extractor.SetChannel(StreamIDBase_Video|0);
}


void DvdApp::InitAudioDecoder()
{
  system_demux.EnableChannel(StreamIDBase_Audio|0);
  audio_extractor.SetSource(&system_demux);
  audio_extractor.SetChannel(StreamIDBase_Audio|0);
}


void DvdApp::InitAC3Decoder()
{
  system_demux.EnableChannel(0xBD);
  ac3_extractor.SetSource(&system_demux);
  ac3_extractor.SetChannel(0xBD);
}


void DvdApp::InsertPostprocessor(VideoPostprocessor& pp)
{
  pp.SetNext(lastoutput);
  lastoutput = &pp;
}


void DvdApp::InitVideoDecoder2()
{
  video_dec.SetSink(*lastoutput);
}

void DvdApp::ViewMPEG(bool audio,bool ac3)
{
  int w,h;

  int nframes=0;
  try
    {
      while (1)
	{
	  while (audio && audio_extractor.PacketPending())
	    {
	      SysPacket_Packet* pck = audio_extractor.GetNextPacket();
	      fwrite(pck->data.AskContents(),pck->data.AskLength(),1,stdout);
	      delete pck;
	    }

	  while (ac3 && ac3_extractor.PacketPending())
	    {
	      SysPacket_Packet* pck = ac3_extractor.GetNextPacket();
	      if (pck->data.AskContents()[0] == ac3stream)
		{
		  fwrite(pck->data.AskContents()+4,
			 pck->data.AskLength()-4,1,stdout);
		}

	      delete pck;
	    }

	  if (outputobject->PictureAvailable())
	    {
	      //cout << "Wait until: " << outputobject->AskPTSOfNextToBeDisplayed() << endl;
	      if (outputobject->AskPTSOfNextToBeDisplayed())
		syncer->WaitUntil(outputobject->AskPTSOfNextToBeDisplayed());
	      outputobject->ShowPicture();
	      nframes++;

	      if (maxframes && nframes>=maxframes)
		break;
	    }
	  else
	    {
	      bool more = video_dec.DecodeAFrame();
	      if (!more)
		break;
	    }
	}
    }
  catch(...)
    {
      cout << "Unexpected end. Decoded up to frame: " << nframes << endl;
      throw;
    }

  cout << "Decoded " << nframes << " frames.\n";
}


static void usage()
{
  cerr <<
    " DVDview " DVDVIEW_VERSION "    " DVDVIEW_DATE " (c) Dirk Farin\n"
    "------------------------------------------------\n"
    "usage: dvdview <options> filename.mpg\n"
    "\n"
    "Options:\n"
    "  -B    Skip B-frames\n"
    "  -P    Skip P-frames (and B-frames)\n"
    "  -T #  Set speed (100 for realtime, >100 for faster)\n"
    "  -S #  Set output size.\n"
    "  -N #  Number of frames to decode\n"
    "  -L    Loop continuously\n"
    "  -U    Use fullscreen\n"
    "  -v #  Enable logging level (0 <= # <= 7)\n"
    "         0 - off\n"
    "         1 - sequence headers\n"
    "         2 - gop headers\n"
    "         3 - picture headers\n"
    "         4 - slice headers\n"
    "         5 - macroblocks\n"
    "         6 - DCT coefficients\n"
    "         7 - binary slice data\n"
    "  -F    Write PPM sequence rather than display stream contents.\n"
    "  -Y    Write single YUV file rather than display stream contents.\n"
    "  -M    Show macroblock boundaries (twice for bigger marks).\n"
    "  -d      Show boundary as dark pixels.\n"
    "  -l      Show boundary as bright pixels.\n"    
    "  -Q    Show QScale of macroblocks.\n"
    "  -V    Show motionvectors of macroblocks (vectors).\n"
    "  -C    Show motionvectors of macroblocks (colors).\n"
    "  -p      Enable vectors in P-frames.\n"
    "  -b      Enable vectors in B-frames.\n"
    "  -c      Show colored motionvectors.\n"
    "  -f      Enable forward vectors.\n"
    "  -r      Enable backward vectors.\n"
    "  -O      hold mode\n"
    "  -s    Show decoder speed in fps.\n"
    "  -A    Extract audio-stream to stdout. Pipe this into mpg123.\n"
    "  -3    Extract AC3-stream to stdout. Pipe this into ac3dec.\n"
    "  -a #    select AC3-stream (0-7) default is 0.\n"
#if 0
    "  -W    Set time to wait after each frame (in 1/100s).\n"
    "  -S    Disable X11 shared memory extension.\n"
    "  -F    DisableSomething=true (development only).\n"
    "  -m    Mark special MBs (development only).\n"
#endif
    "  -h    Show this usage information\n"

#if LINUX
    " - - - - - -  Linux only features  - - - - - -\n"
    "  -D #  Play VideoCD track rather that MPEG file (omit filename)\n"
    "  -X    Use X11 output even when MGA_VID is available.\n"
#endif
    ;
}


extern void GetLinesForDrawing(DecodedImageData* dimg,int first,int last);

#if 0
int main(int argc,char** argv)
{
  InitULib(argc,argv);

  DecodedImageData dimg;
  dimg.m_may_modify=false;
  dimg.m_field_lines=true;
  dimg.m_src_y_start=10;
  dimg.m_src_y_end  =40;
  dimg.m_dst_y_start=1;
  GetLinesForDrawing(&dimg,10,20);
}
#endif

int main(int argc, char ** argv)
{
  do {
    DvdApp * m  = new DvdApp(argc,argv);
    delete m;
    videosink_x11.reset();
  } while (loop);
}

void DvdApp::InitOptions()
{
  if(!_initialized) {
    imgtypes=false;
    qscales=false;
    mvs=false;
    mvcol=false;
    audio=false;
    ac3=false;
    mvf=false;
    mvb=false;
    mvcolor=false;
    mvpf=false;
    mvbf=false;
    mv_holdmode=false;
    fpsdisplay=false;
    x11_output=false;   // try MGA first
    vcd=0;
    resize=false;
    rescale=false;
    filtered_resize=false;
  }
}

DvdApp::DvdApp(int argc,char** argv) :
  system_demux(system_resync),
  vidcon(video_extractor),
  video_ptsvideo(vidcon),
  video_dec(video_ptsvideo),
  ac3stream(0x08),
  syncer(&syncer_realtime)

{
  int scaled_width,scaled_height;

  VideoDecoder_Options vdec_options;

  // possibly initialize user options to default values
  InitOptions();

#if 0 && ENABLE_MMX
  cout << "CPU capabilities detected: ";
  if (cpucapabilities.HasMMX()) cout << "MMX ";
  if (cpucapabilities.HasKNI()) cout << "KNI ";
  if (cpucapabilities.Has3dNow()) cout << "3dNow! ";
  if (cpucapabilities.HasMTRR()) cout << "MTRR ";
  if (cpucapabilities.HasCMOV()) cout << "CMOV ";
  if (cpucapabilities.HasFPU()) cout << "FPU ";
  cout << endl;
#endif

#if LINUX
  // Try to use MGA if available.

  bool mga_available = videosink_mga.MGA_Available();
  if (mga_available)
    {
      outputobject = &videosink_mga;
      resizer = &videosink_mga;
    }
  else
    {
      outputobject = &videosink_x11;
      resizer = &pp_resize;

      if (!default_x11server.AskDisplay())
	{ cerr << "Cannot open display.\n"; throw "no display"; }
    }
#else
  // Use X11 output by default

  outputobject = &videosink_x11;
  resizer = &pp_resize;

  if (!default_x11server.AskDisplay())
    { cerr << "Cannot open display.\n"; throw "no display"; }
#endif

  try
    {
#if 1
      int c;

      // only parse the command line args if we haven't before (from _initialized)

      while (!_initialized && (c=getopt(argc,argv,"f3rlcdv:wpuT:S:BbPHMQVW:FYOCdmAa:hXxD:sN:L")) != EOF)
	{
	  switch (c)
	    {
	    case 'v':
	      {
#ifndef NDEBUG
		int level=atoi(optarg);
		if (level>7) level=7;
		if (level<0) level=0;
		switch(level)
		  {
		  case 7: vdec_options.Trace_SliceData = true;
		  case 6: vdec_options.Trace_DCTCoeff  = true;
		  case 5: vdec_options.Trace_MB   = true;
		  case 4: vdec_options.Trace_SlcH = true;
		  case 3: vdec_options.Trace_PicH = true;
		  case 2: vdec_options.Trace_GOPH = true;
		  case 1: vdec_options.Trace_SeqH = true;
		  case 0: break;
		  }
#else
		cerr << "Ignoring option -v#: Verbose output was not compiled in.\n";
#endif
	      }
	      break;
	      
	    case 'M':
	      if (!options.Postproc_ShowMBs)
		options.Postproc_ShowMBs = true;
	      else
		{ pp_mblks.SetBigMarks(); }
	      break;

	    case 'd':
	      pp_mblks.SetStaticMode(0);
	      break;

	    case 'l':
	      pp_mblks.SetStaticMode(255);
	      break;

	    case 'P':
	      vdec_options.DecodeP = vdec_options.DecodeB = false;
	      break;
	      
	    case 'B':
	      vdec_options.DecodeB = false;
	      break;

	    case 'x':
	      imgtypes=true;
	      break;
	      
	    case 'Q':
	      qscales=true;
	      break;
	      
	    case 'V':
	      mvs=true;
	      break;

	    case 'C':
	      mvcol=true;
	      break;

	    case 'O':
	      mv_holdmode=true;
	      break;

	    case 'c': mvcolor=true; break;
	    case 'f': mvf=true; break;
	    case 'r': mvb=true; break;
	    case 'p': mvpf=true; break;
	    case 'b': mvbf=true; break;
	      
	    case 'A':
	      audio=true;
	      break;
	      
	    case '3':
	      ac3=true;
	      break;

	    case 'a':
	      ac3stream = atoi(optarg) + 0x80;
	      break;

	    case 'N':
	      maxframes = atoi(optarg);
	      break;

	    case 'F':
	      outputobject = &videosink_ppm;
	      resizer = &pp_resize;
	      break;

	    case 'Y':
	      outputobject = &videosink_yuv;
	      resizer = &pp_resize;
	      break;

	    case 's':
	      fpsdisplay=true;
	      break;

	    case 'h':
	      usage();
              exit(10);
	      break;

	    case 'T':
	      {
		int speed=atoi(optarg);
		if (speed<=0)
		  {
		    cerr << "Speed must be greater than 0%, using 1%\n";
		    speed=1;
		  }
		syncer_realtime.SetSpeed(speed);
	      }
	      break;

	    case 'S':
	      {
		char* w = strtok(optarg,"xX");
		char* h = strtok(NULL,"xX");
		if (!w || !h)
		  { cerr << "invalid size argument (1)\n"; exit(5); }
		
		scaled_width  = atoi(w);
		scaled_height = atoi(h);

		rescale=true;
	      }
	      break;

	    case 'H':
	      filtered_resize = true;
	      break;

	    case 'X':
	      x11_output=true;
	      break;

	    case 'L':
	      loop=true;
	      break;

	    case 'U':
	      fullscreen=true;
	      break;

#if LINUX
	    case 'D':
	      vcd=atoi(optarg);
	      break;
#endif
	    }
	}

      // make sure we don't re-initialize from the command line args 
      _initialized = true;

      if (x11_output)
	{
	  outputobject = &videosink_x11;
	  resizer = &pp_resize;

  	  if (!default_x11server.AskDisplay())
	    { cerr << "Cannot open display.\n"; throw "no display"; }
	}

      if (filtered_resize && resizer==&pp_resize) { cout << "bla\n"; resizer=&pp_filteredresize; }

      lastoutput = outputobject;


      if (optind==argc && !vcd)
	{ usage(); exit(5); }

      video_dec.SetOptions(vdec_options);

      if (rescale) resizer->SetScaledSize(scaled_width,scaled_height);



      ifstream str;

      if (!vcd)
	{
	  if (strcmp(argv[optind],"-")==0)
	    InitStreamSource_cin();
	  else
	    {
	      str.open(argv[optind]);
	      InitStreamSource(str);
	    }
	}
      else
	{
	  InitStreamSourceVCD(vcd);
	}

      InitVideoDecoder1();
      if (audio) InitAudioDecoder();
      if (ac3)   InitAC3Decoder();

      if (filtered_resize) InsertPostprocessor(pp_filteredresize); else InsertPostprocessor(pp_resize);
      if (qscales)  InsertPostprocessor(pp_qscale);
      if (options.Postproc_ShowMBs) InsertPostprocessor(pp_mblks);
      if (fpsdisplay) InsertPostprocessor(pp_fps);
      if (mvs)
	{
	  if (!mvf && !mvb) { mvf=mvb=true; }
	  pp_mv.SelectMVs(mvf,mvb);
	  pp_mv.ColoredVectors(mvcolor);
	  pp_mv.SelectFrametypes(mvpf,mvbf);
	  InsertPostprocessor(pp_mv);
	}
#if 0
      if (imgtypes) InsertPostprocessor(pp_imgtype);
#endif
      if (mvcol)
	{
	  pp_mvcol.SetHoldMode(mv_holdmode);
	  pp_mvcol.SelectFrametypes(mvpf,mvbf);
	  InsertPostprocessor(pp_mvcol);
	}
      InitVideoDecoder2();

      //str.seekg(200000,ios::beg);


      ViewMPEG(audio,ac3);

#endif

#if 0
      ifstream str(argv[1]);
      ShowSystemPackets(str);
#endif

#if 0
      ifstream str(argv[1]);
      //ShowVideoAccessUnitPackets(str);
      //ShowVideoSyntax(str);
      DecodeVideo(str);
#endif

#if 0
      CheckMemAlloc();
#endif

#if 0
      if (!ProcessOptions(argc,argv, options))
	return 0;
#endif
    }
  catch (Excpt_Base& e)
    {
      MessageDisplay::Show(e.m_severity , e.m_text);
    }
  catch (const char* txt)
    {
      cerr << "error: " << txt << endl;
    }
  catch (...)
    {
      cerr << "error occurred... quitting...\n";
    }
}
