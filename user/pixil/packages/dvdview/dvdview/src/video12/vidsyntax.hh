/*********************************************************************
  video12/vidsyntax.hh
    Main video decoder

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   07/set/2000 - Dirk Farin
     - Changed type of Framerate field in SequenceHeader from double to int
       to overcome MMX problems.
   05/Sep/2000 - Dirk Farin
     - Syntax decoding functions extracted from vdecoder.cc.
   30/Sep/1999 - Dirk Farin
     - Integrated code into CVS.
   05/May/1999 - Dirk Farin
     - Took structure definitions out of viddec.hh
 ********************************************************************************
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

#ifndef DVDVIEW_VIDEO12_VIDSYNTAX_HH
#define DVDVIEW_VIDEO12_VIDSYNTAX_HH

#include "types.hh"
#include "video12/macroblock.hh"
#include "libvideogfx/containers/array2.hh"


struct VideoSyntaxData
{
  virtual ~VideoSyntaxData() { }
};


struct SequenceHeader : public VideoSyntaxData
{
  uint32 m_Width,m_Height;
  uint4  m_AspectRatio;
  int    m_Framerate;     // *10000
  uint32 m_Bitrate;
  uint32 m_VBVBufferSize;
  bool   m_Constrained;


  // MPEG-2 only data

  uint4  m_Profile;
  uint4  m_Level;
  bool   m_ProgressiveSequence;
  uint2  m_ChromaFormat;
  bool   m_LowDelay;

  // deduced data

  bool   m_IsMPEG2;
  uint32 m_MBWidth,m_MBHeight; // width and height in macroblocks
};


struct QuantMatrices : public VideoSyntaxData
{
  int  m_LumIntra[64];
  int  m_LumInter[64];
  int  m_ChrIntra[64];
  int  m_ChrInter[64];
};

struct GOPHeader : public VideoSyntaxData
{
  bool  m_TimeCode_DropFrameFlag;
  uint5 m_TimeCode_Hours;
  uint6 m_TimeCode_Minutes;
  uint6 m_TimeCode_Seconds;
  uint6 m_TimeCode_Pictures;

  bool m_ClosedGOP;
  bool m_BrokenLink;
};


struct PictureHeader : public VideoSyntaxData
{
  uint16 m_temporal_reference;
  uint3  m_picture_coding_type;
  uint16 m_vbv_delay;
  bool   m_fullpel_fw;
  bool   m_fullpel_bw;
  uint3  m_fcode[2][2];  // [ 0 fw, 1 bw ]  [ 0 h, 1 v ]

  // MPEG2 only fields follow

  bool   m_IsMPEG2;
  
  uint4  m_intra_dc_precision;
  uint2  m_picture_structure;
  bool   m_top_field_first;
  bool   m_frame_pred_frame_dct;
  bool   m_concealment_motion_vectors;
  uint1  m_q_scale_type;
  uint1  m_intra_vlc_format;
  bool   m_alternate_scan;
  bool   m_repeat_first_field;
  uint1  m_chroma420type;
  bool   m_progressive_frame;

  bool   m_composite_display_flag;
  uint1  m_v_axis;
  uint3  m_field_sequence;
  uint1  m_sub_carrier;
  uint7  m_burst_amplitude;
  uint8  m_sub_carrier_phase;


  // precalculated values

  int m_intra_dc_precision_shift;


  // extra data

  int m_picturedata_length;
};


struct PictureData
{
  Array2<Macroblock> m_codedimage;


  // efficient allocation

  static PictureData* GetPictureData(int mb_w,int mb_h);
  static void FreePictureData(PictureData*);

  void operator delete(void*) { Assert(0); /* Use FreePictureData() to delete PictureData objects.
					      Calling ::delete is safe but very inefficient. */ }
};



struct EndOfVideoStream : public VideoSyntaxData
{
};



void DecodeSequenceHeader(class MemBitstreamReader& bs,SequenceHeader& sh,QuantMatrices& qmat);
void DecodeSequenceHeaderExt(class MemBitstreamReader& bs2,SequenceHeader& sh);
void TraceSequenceHeader(const SequenceHeader& sh);

void DecodeGOPHeader(class MemBitstreamReader&,GOPHeader&);
void TraceGOPHeader(const GOPHeader&);

void DecodePictureHeader(class MemBitstreamReader&,PictureHeader&);
void DecodePictureCodingExt(class MemBitstreamReader&,PictureHeader&);
void TracePictureHeader(const PictureHeader&);

#endif
