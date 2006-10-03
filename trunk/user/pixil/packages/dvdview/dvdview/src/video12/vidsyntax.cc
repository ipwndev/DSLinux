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

#include "video12/vidsyntax.hh"
#include "video12/constants.hh"
#include "libvideogfx/utility/bitstream/membitsread.hh"

#include <iostream.h>
#include <iomanip.h>


const int PoolSize = 10;

static PictureData* pdpool[PoolSize];
static int pdpoolsize=0;


PictureData* PictureData::GetPictureData(int w,int h)
{
  if (pdpoolsize==0)
    {
      PictureData* picdata = new PictureData;

      if (w>0)
	picdata->m_codedimage.Create(w,h);

      return picdata;
    }
  else
    {
      pdpoolsize--;
      PictureData* picdata = pdpool[pdpoolsize];

      if (picdata->m_codedimage.AskWidth()  != w ||
	  picdata->m_codedimage.AskHeight() != h)
	{
	  if (w!=0)
	    picdata->m_codedimage.Create(w,h);
	}

      return picdata;
    }
}


void PictureData::FreePictureData(PictureData* p)
{
  if (pdpoolsize==PoolSize)
    ::delete p;
  else
    pdpool[pdpoolsize++] = p;
}



static const int frameratecode2framerate[16] =
{
   0, 24000*10000/1001, 24*10000,25*10000, 30000*10000/1001, 30*10000,50*10000,60000*10000/1001,
  60*10000, 0,0,0,0,0,0,0
};

#if 0
static const uint8 default_intra_matrix[8][8] = {
  {  8, 16, 19, 22, 26, 27, 29, 34 },
  { 16, 16, 22, 24, 27, 29, 34, 37 },
  { 19, 22, 26, 27, 29, 34, 34, 38 },
  { 22, 22, 26, 27, 29, 34, 37, 40 },
  { 22, 26, 27, 29, 32, 35, 40, 48 },
  { 26, 27, 29, 32, 35, 40, 48, 58 },
  { 26, 27, 29, 34, 38, 46, 56, 69 },
  { 27, 29, 35, 38, 46, 56, 69, 83 }
};
#endif

static const uint8 default_intra_matrix_zigzag[64] = {
   8,16,16,19,16,19,22,22,22,22,22,22,26,24,26,27,27,27,26,26,26,26,27,27,29,29,29,34,34,
  34,37,34,34,32,32,29,29,35,34,35,35,37,38,40,40,40,38,38,46,46,48,48,58,56,56,69,69,83
};


void DecodeSequenceHeader(class MemBitstreamReader& bs,SequenceHeader& sh,QuantMatrices& qmat)
{
  SequenceHeader* seqh = &sh;


  /**** Read sequence header ****/

  Assert(bs.PeekBits(32)==STARTCODE_SEQUENCE_HEADER);
  bs.SkipBits(32);

  seqh->m_IsMPEG2 = false; /* This will be set to "true" if there is a Sequence-Extension following */
  seqh->m_Width  = bs.GetBits(12);
  seqh->m_Height = bs.GetBits(12);
  seqh->m_AspectRatio = bs.GetBits(4);
  seqh->m_Framerate = frameratecode2framerate[bs.GetBits(4)];

  /* TODO
     if (seqh->m_Framerate == 0.0)
     ShowNote(ErrSev_Warning,"MPEG stream error: framerate set to invalid value !");
  */

  seqh->m_Bitrate = bs.GetBits(18);
  bs.SkipBits(1);
  seqh->m_VBVBufferSize = bs.GetBits(10);
  seqh->m_Constrained = bs.GetBits(1);


  // Reset qmatrices or set them to new values.

  // Set Intra-Matrix
  if (bs.GetBits(1))
    {
      for (int i=0;i<64;i++)
	qmat.m_LumIntra[i] = qmat.m_ChrIntra[i] = bs.GetBits(8);
    }
  else
    {
      for (int i=0;i<64;i++)
	qmat.m_LumIntra[i] = qmat.m_ChrIntra[i] = default_intra_matrix_zigzag[i];
    }

  // Set Inter-Matrix
  if (bs.GetBits(1))
    {
      for (int i=0;i<64;i++)
	qmat.m_LumInter[i] = qmat.m_ChrInter[i] = bs.GetBits(8);
    }
  else
    {
      for (int i=0;i<64;i++)
	qmat.m_LumInter[i] = qmat.m_ChrInter[i] = 16;
    }

  // fill MPEG-2 entries to match MPEG-1 semantics

  seqh->m_ProgressiveSequence = false;
  seqh->m_ChromaFormat = CHROMA_420;
  seqh->m_LowDelay = false;
}


void DecodeSequenceHeaderExt(class MemBitstreamReader& bs2,SequenceHeader& sh)
{
  SequenceHeader* seqh = &sh;

  /**** Decode Sequence-Extension ****/
    
  seqh->m_IsMPEG2 = true;
    
  seqh->m_Profile = bs2.GetBits(4);
  seqh->m_Level   = bs2.GetBits(4);
  seqh->m_ProgressiveSequence = bs2.GetBits(1);
  seqh->m_ChromaFormat = bs2.GetBits(2);
  seqh->m_Width  |= bs2.GetBits(2)<<12;
  seqh->m_Height |= bs2.GetBits(2)<<12;
  seqh->m_Bitrate |= bs2.GetBits(12)<<18;
  bs2.SkipBits(1);
  seqh->m_VBVBufferSize |= bs2.GetBits(8)<<10;
  seqh->m_LowDelay = bs2.GetBits(1);
  seqh->m_Framerate *= bs2.GetBits(2)+1;
  seqh->m_Framerate /= bs2.GetBits(5)+1;
}


void TraceSequenceHeader(const SequenceHeader& sh)
{
#ifndef NDEBUG
  const SequenceHeader* seqh = &sh;

    {
      cout << "SequenceHeader:\n"
           << "  MPEG-2:       " << (seqh->m_IsMPEG2?"true":"false") << endl
           << "  width:        " << seqh->m_Width << endl
           << "  height:       " << seqh->m_Height << endl
           << "  aspect code:  " << ((int)seqh->m_AspectRatio) << endl
           << "  framerate:    " << seqh->m_Framerate/10000 << '.' << seqh->m_Framerate%10000 << endl
           << "  bitrate:      " << seqh->m_Bitrate << endl
           << "  VBV buf.size: " << seqh->m_VBVBufferSize << endl
           << "  constrained:  " << (seqh->m_Constrained?"true":"false") << endl
	;
      //           << "  load intra qm:" << (!seqh->m_qmatrices.m_Hint_Std_Intra_Lum?"true":"false") << endl
      //           << "  load inter qm:" << (!seqh->m_qmatrices.m_Hint_Std_Inter_Lum?"true":"false") << endl;

      if (seqh->m_IsMPEG2)
        {
          cout << "  profile:      ";
          switch (seqh->m_Profile)
            {
            case PROFILE_Simple: cout << "Simple\n"; break;
            case PROFILE_Main:   cout << "Main\n";   break;
            case PROFILE_SNRScalable: cout << "SNR\n"; break;
            case PROFILE_SpatiallyScalable: cout << "Spatial\n"; break;
            case PROFILE_High:   cout << "High\n"; break;
            default: cout << "UNKNOWN !\n"; break;
            }

          cout << "  level:        ";
          switch (seqh->m_Level)
            {
            case LEVEL_Low:    cout << "Low\n"; break;
            case LEVEL_Main:   cout << "Main\n"; break;
            case LEVEL_High1440: cout << "High1440\n"; break;
            case LEVEL_High:   cout << "High\n"; break;
            default: cout << "UNKNOWN !\n"; break;
            }

          cout << "  progr. seq.:  " << (seqh->m_ProgressiveSequence?"true":"false") << endl;
          cout << "  chroma format:";
          switch (seqh->m_ChromaFormat)
            {
            case CHROMA_420: cout << "4:2:0\n"; break;
            case CHROMA_422: cout << "4:2:2\n"; break;
            case CHROMA_444: cout << "4:4:4\n"; break;
            default: cout << "UNKNOWN !\n"; break;
            }

          cout << "  low delay:    " << (seqh->m_LowDelay?"true":"false") << endl;
        }
    }
#endif
}



void DecodeGOPHeader(class MemBitstreamReader& bs,GOPHeader& gopdata)
{
  /**** Read GOP-Header ****/

  Assert(bs.PeekBits(32)==STARTCODE_GROUP_START);
  bs.SkipBits(32);

  GOPHeader* goph = &gopdata;

  goph->m_TimeCode_DropFrameFlag = bs.GetBits(1);
  goph->m_TimeCode_Hours         = bs.GetBits(5);
  goph->m_TimeCode_Minutes       = bs.GetBits(6);
  bs.SkipBits(1);
  goph->m_TimeCode_Seconds       = bs.GetBits(6);
  goph->m_TimeCode_Pictures      = bs.GetBits(6);

  goph->m_ClosedGOP = bs.GetBits(1);
  goph->m_BrokenLink= bs.GetBits(1);
}


void TraceGOPHeader(const GOPHeader& gopdata)
{
  const GOPHeader* goph = &gopdata;

#ifndef NDEBUG
  cout << "GOP-Header:\n"
       << "  Timecode:    " << ((int)goph->m_TimeCode_Hours)
       << ':' << setfill('0') << setw(2) << ((int)goph->m_TimeCode_Minutes)
       << ':' << setw(2) << ((int)goph->m_TimeCode_Seconds)
       << '.' << setw(2) << ((int)goph->m_TimeCode_Pictures);
  if (goph->m_TimeCode_DropFrameFlag) cout << " drop-frame-flag set";
  cout << "\n  closed GOP:  " << (goph->m_ClosedGOP?"true":"false") << endl
       << "  broken link: " << (goph->m_BrokenLink?"true":"false") << endl;
#endif
}


void DecodePictureHeader(class MemBitstreamReader& bs,PictureHeader& pichdr)
{
  Assert(bs.PeekBits(32)==STARTCODE_PICTURE_START);
  bs.SkipBits(32);

  pichdr.m_temporal_reference  = bs.GetBits(10);
  pichdr.m_picture_coding_type = bs.GetBits(3);
  pichdr.m_vbv_delay           = bs.GetBits(16);

  pichdr.m_fullpel_fw = false;
  pichdr.m_fullpel_bw = false;
  pichdr.m_fcode[0][0] = 7;
  pichdr.m_fcode[0][1] = 7;
  pichdr.m_fcode[1][0] = 7;
  pichdr.m_fcode[1][1] = 7;

  if (pichdr.m_picture_coding_type == 2 ||
      pichdr.m_picture_coding_type == 3)
    {
      pichdr.m_fullpel_fw = bs.GetBits(1);
      pichdr.m_fcode[0][0] =
	pichdr.m_fcode[0][1] = bs.GetBits(3);
    }

  if (pichdr.m_picture_coding_type == 3)
    {
      pichdr.m_fullpel_bw = bs.GetBits(1);
      pichdr.m_fcode[1][0] =
	pichdr.m_fcode[1][1] = bs.GetBits(3);
    }

  while (bs.GetBits(1)==1)
    {
      bs.SkipBits(8);
    }


  // Set fields that are variable in MPEG-2 but constant in MPEG-1.
  // These will be overwritten again, when decoding MPEG-2 Picture Coding Extension.

  pichdr.m_IsMPEG2 = false;
  pichdr.m_intra_dc_precision = 8;
  pichdr.m_picture_structure  = PICSTRUCT_FramePicture;
  pichdr.m_concealment_motion_vectors = false;
  pichdr.m_progressive_frame  = true;
  pichdr.m_q_scale_type       = 0;
  pichdr.m_intra_vlc_format   = 0;
  pichdr.m_alternate_scan     = false;
  pichdr.m_progressive_frame  = true;
  pichdr.m_composite_display_flag = false;
  pichdr.m_frame_pred_frame_dct = true;

  pichdr.m_intra_dc_precision_shift = 11-pichdr.m_intra_dc_precision;
}


void DecodePictureCodingExt(class MemBitstreamReader& bs2,PictureHeader& pichdr)
{
  pichdr.m_IsMPEG2 = true;

  pichdr.m_fullpel_fw = false;
  pichdr.m_fullpel_bw = false;
  pichdr.m_fcode[0][0] = bs2.GetBits(4);
  pichdr.m_fcode[0][1] = bs2.GetBits(4);
  pichdr.m_fcode[1][0] = bs2.GetBits(4);
  pichdr.m_fcode[1][1] = bs2.GetBits(4);

  pichdr.m_intra_dc_precision   = bs2.GetBits(2)+8;
  pichdr.m_picture_structure    = bs2.GetBits(2);
  pichdr.m_top_field_first      = bs2.GetBits(1);
  pichdr.m_frame_pred_frame_dct = bs2.GetBits(1);
  pichdr.m_concealment_motion_vectors = bs2.GetBits(1);
  pichdr.m_q_scale_type         = bs2.GetBits(1);
  pichdr.m_intra_vlc_format     = bs2.GetBits(1);
  pichdr.m_alternate_scan       = bs2.GetBits(1);
  pichdr.m_repeat_first_field   = bs2.GetBits(1);
  pichdr.m_chroma420type        = bs2.GetBits(1);
  pichdr.m_progressive_frame    = bs2.GetBits(1);

  pichdr.m_composite_display_flag = bs2.GetBits(1);
  if (pichdr.m_composite_display_flag)
    {
      pichdr.m_v_axis           = bs2.GetBits(1);
      pichdr.m_field_sequence   = bs2.GetBits(3);
      pichdr.m_sub_carrier      = bs2.GetBits(1);
      pichdr.m_burst_amplitude  = bs2.GetBits(7);
      pichdr.m_sub_carrier_phase= bs2.GetBits(8);
    }

  pichdr.m_intra_dc_precision_shift = 11-pichdr.m_intra_dc_precision;
}


void TracePictureHeader(const PictureHeader& pichdr)
{
  cout << "PictureHeader:\n"
       << "  temporal reference:    " << pichdr.m_temporal_reference << endl
       << "  picture_coding_type:   ";
  switch (pichdr.m_picture_coding_type)
    {
    case PICTYPE_I: cout << 'I'; break;
    case PICTYPE_P: cout << 'P'; break;
    case PICTYPE_B: cout << 'B'; break;
    case PICTYPE_D: cout << 'D'; break;
    default: cout << "UNKNOWN !"; break;
    }

  cout << "\n  vbv_delay:             " << pichdr.m_vbv_delay << endl
       << "  fullpel forward mv:    " << (pichdr.m_fullpel_fw?"true":"false") << endl
       << "  fullpel backward mv:   " << (pichdr.m_fullpel_bw?"true":"false") << endl;
  for (int i=0;i<2;i++)
    for (int j=0;j<2;j++)
      {
	cout << "  fcode[" << i << "][" << j << "] ("
	     << (i?"bw/":"fw/") << (j?'v':'h')
	     << "):    " << ((int)pichdr.m_fcode[i][j]) << endl;
      }

  if (pichdr.m_IsMPEG2)
    {
      cout << "  intra dc precision:    " << ((int)pichdr.m_intra_dc_precision) << endl
	   << "  picture structure:     ";
      switch (pichdr.m_picture_structure)
	{
	case PICSTRUCT_TopField: cout << "top field\n"; break;
	case PICSTRUCT_BottomField: cout << "bottom field\n"; break;
	case PICSTRUCT_FramePicture: cout << "frame picture\n"; break;
	}
 
      cout << "  top field first:       " << (pichdr.m_top_field_first?"true":"false") << endl
	   << "  frame pred frame dct:  " << (pichdr.m_frame_pred_frame_dct?"true":"false") << endl
	   << "  concealment mvs:       " << (pichdr.m_concealment_motion_vectors?"true":"false") << endl
	   << "  q-scale type:          " << ((int)pichdr.m_q_scale_type) << endl
	   << "  intra vlc format:      " << ((int)pichdr.m_intra_vlc_format) << endl
	   << "  alternate scan:        " << (pichdr.m_alternate_scan?"true":"false") << endl
	   << "  repeat first field:    " << (pichdr.m_repeat_first_field?"true":"false") << endl
	   << "  chroma420type(obsolet):" << ((int)pichdr.m_chroma420type) << endl
	   << "  progressive frame:     " << (pichdr.m_progressive_frame?"true":"false") << endl;
          
      if (pichdr.m_composite_display_flag)
	{
	  cout << "  v-axis:                " << ((int)pichdr.m_v_axis) << endl
	       << "  field sequence:        " << ((int)pichdr.m_field_sequence) << endl
	       << "  sub carrier:           " << ((int)pichdr.m_sub_carrier) << endl
	       << "  burst amplitude:       " << ((int)pichdr.m_burst_amplitude) << endl
	       << "  sub carrier phase:     " << ((int)pichdr.m_sub_carrier_phase) << endl;
	}
    }
}


#include "libvideogfx/containers/array2.cc"

template class Array2<Macroblock>;
