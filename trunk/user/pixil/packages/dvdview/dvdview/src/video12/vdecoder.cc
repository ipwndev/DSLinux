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

#include "video12/vdecoder.hh"
#include "video12/constants.hh"
#include "video12/vlc.hh"
#include "video12/viddec_mods.hh"
#include "system/system1.hh"
#include "error.hh"

#include "video12/modules/idct_arai.hh" ////////////////////// TODO
#include "video12/modules/mcomp_sgl_simple.hh"

#include "libvideogfx/utility/bitstream/membitsread.hh"
#include "libvideogfx/utility/bitstream/fastbitbuf.hh"

#include <iostream.h>
#include <iomanip.h>
#include <string.h>
#include <strings.h>

#define SHOW_FRAMESIZE 0

#if ENABLE_MMX
#define MMX_DCT 1
#else
#define MMX_DCT 0
#endif

#undef SEND_FULL_BFRAMES

extern "C" void IDCT_mmx(short* in,short* out);
extern "C" void IDCT_16bit8bit(short* in,Pixel* out,int skip);
extern "C" void IDCT_16bit8bit_add(short* in,Pixel* out,int skip);

extern void PrintDataHex(uint8* data,uint32 len);
extern void PrintDataBin(uint8* data,uint32 len);


#include "video12/vlc.cc"
#include "video12/dctblk.cc"

#define NOBZERO 0

inline void DIV2(int& x) { if (x>=0) x/=2; else x=(x-1)/2; }

/*
  implemented syntax elements:   ( - not, o partial, + full )
  - video_sequence: o
  - sequence_header: +
  - extension_and_user_data: o
  - extension_data: o
  - user_data: -
  - sequence_extension: +
  - sequence_display_extension: -
  - sequence_scalable_extension: -
  - group_of_pictures_header: +
  - picture_header: + (excluding extra_information_picture)
  - picture_coding_extension: +
  - quant_matrix_extension: -
  - picture_display_extension: -
  - picture_temporal_scalable_extension: -
  - picture_spatial_scalable_extension: -
  - copyright_extension: -
  - picture_data: +
  - slice: +
  - macroblock: +
  - macroblock_modes: o
  - motion_vectors: +
  - motion_vector: +
  - coded_block_pattern: +
  - block: +
*/


static const int qcode2qscale[2][32] =
{
//  - 1 2 3 4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29  30  31
  { 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58, 60, 62 },
  { 0,1,2,3,4, 5, 6, 7, 8,10,12,14,16,18,20,22,24,28,32,36,40,44,48,52,56,64,72,80,88,96,104,112 }
};


static const int scan[2][64] =
{
  { /* Zig-Zag scan pattern  */
    0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
    12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
    35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
    58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
  },
  { /* Alternate scan pattern */
    0,8,16,24,1,9,2,10,17,25,32,40,48,56,57,49,
    41,33,26,18,3,11,4,12,19,27,34,42,50,58,35,43,
    51,59,20,28,5,13,6,14,21,29,36,44,52,60,37,45,
    53,61,22,30,7,15,23,31,38,46,54,62,39,47,55,63
  }
};


static const int AlternateFromZigZag[64] =
{
// 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
   0, 2, 3, 9, 1, 4, 5, 7, 8,11,10,20,21,35,36,34,
  22,19,18,12, 6,13,14,16,17,24,23,33,37,48,32,38,
  47,49,25,31,15,26,27,29,30,40,39,46,50,57,45,51,
  56,58,41,44,28,42,43,53,52,55,59,62,54,60,61,63
};


VideoSyntaxTrace_Options::VideoSyntaxTrace_Options()
{
  Trace_SeqH =
  Trace_GOPH =
  Trace_PicH =
  Trace_SlcH =
  Trace_MB   =
  Trace_SliceData = false;
  Trace_DCTCoeff  = false;
}


VideoFrameSkip_Options::VideoFrameSkip_Options()
{
  DecodeI = true;
  DecodeP = true;
  DecodeB = true;
}


VideoDecoder::VideoDecoder(class PacketSource& source)
  : d_source(&source),
    d_sink(NULL),
    d_picdata(NULL),
    d_SequenceHeaderRead(false),
    d_next_packet(NULL),
    d_BFrameAvailable(false),
    d_FirstFieldInFrame(true),
    d_next_IsEmpty(true),
    d_skip_this_frame(false),
    d_curr(NULL), d_next(NULL), d_last(NULL),
    d_motcomp(NULL)
{
  d_motcomp = MotionCompensation_SglMB::Create();

#if MMX_DCT
  for (int i=0;i<2;i++)
    for (int n=0;n<64;n++)
      {
	int r=scan[i][n]/8;
	int c=scan[i][n]%8;
	d_scan[i][n]=c*8+r;
      }
#else
  for (int i=0;i<2;i++)
    for (int n=0;n<64;n++)
      d_scan[i][n]=scan[i][n];
#endif
}

VideoDecoder::~VideoDecoder()
{
  if (d_next_packet)
    delete d_next_packet;

  if (d_motcomp) delete d_motcomp;
}

SysPacket_Packet* VideoDecoder::GetNextPacket()
{
  SysPacket_Packet* pck;

  if (d_next_packet)
    {
      pck = d_next_packet;
      d_next_packet=NULL;
    }
  else
    {
      pck = d_source->GetNextPacket();
      if (pck==NULL)
        return NULL;
    }

  return pck;
}

void VideoDecoder::PushbackPacket(SysPacket_Packet* pck)
{
  Assert(d_next_packet==NULL);
  d_next_packet=pck;
}

bool VideoDecoder::DecodeAFrame()
{
  int  LastSliceRead = 0;
  bool BeganPictureDecoding = false;

  for (;;) // Read as much packets as are required to decode a picture.
    {
      // Get next stream packet.

      SysPacket_Packet* pck = GetNextPacket();
      if (pck==NULL)
        {
	  // We reached the end of the stream. Flush the remaining images.

	  // B-picture
	  if (d_BFrameAvailable)
	    {
	      Assert(d_curr);

#if SEND_FULL_BFRAMES
	      d_curr->m_may_modify = true;
	      d_sink->ShowAllMBRows(d_curr);  // Show image
#endif
	      d_sink->FinishedPicture();
	      d_curr->FreePictureData();      // Remove additional information data
	      
	      delete d_curr;
	      d_curr=NULL;
	      d_BFrameAvailable=false;

	      return true;
	    }

	  // Future reference picture
	  if (!d_next_IsEmpty)
	    {
	      d_next->m_may_modify = true;

	      d_sink->BeginPicture(d_next);
	      d_sink->ShowAllMBRows(d_next); // Show last decoded I- or P-image.
	      d_sink->FinishedPicture();
	      d_next->FreePictureData();     // Free the additional information data.
	      
	      delete d_last;
	      delete d_next;
	      d_last = d_next = NULL;

	      d_next_IsEmpty=true;
	    }

          return false;
        }


      // Decode packet data.

      MemBitstreamReader bitstream(pck->data.AskContents() , pck->data.AskLength());


      uint32 startcode = bitstream.PeekBits(32);

      Assert((startcode & 0x00000100) != 0); // TODO: throw excpt

      if (IsMPEG_SLICE_START(startcode&0xFF) && d_SequenceHeaderRead && BeganPictureDecoding)
        {
          if (d_options.Trace_SliceData)
            {
              cout << "Slice Data:\n";
              PrintDataBin(pck->data.AskContents(),pck->data.AskLength());
            }

	  FastBitBuf fastbitbuf(pck->data.AskContents() , pck->data.AskLength());
          DecodeSlice(fastbitbuf);
        }
      else
        {
	  // If we were just decoding a picture and a new picture begins, end decoding of the picture.

	  if ((startcode == STARTCODE_SEQUENCE_HEADER ||
	       startcode == STARTCODE_GROUP_START ||
	       startcode == STARTCODE_PICTURE_START) &&
	      BeganPictureDecoding)
	    {
	      PushbackPacket(pck); // This packet already belongs to the next picture.

	      /* If there's an B-image ready for display, show it.
	       */

	      if (d_BFrameAvailable)
		{
		  Assert(d_curr);
	      
#if SEND_FULL_BFRAMES
		  d_curr->m_may_modify=true;
		  d_sink->ShowAllMBRows(d_curr);  // Show image
#endif
		  d_sink->FinishedPicture();
		  d_curr->FreePictureData();         // Remove additional information data
		  d_BFrameAvailable=false;
		}

	      return true;
	    }


	  switch (startcode)
	    {
	    case STARTCODE_SEQUENCE_HEADER:
	      {
		DecodeSequenceHeader(bitstream,d_seqdata,d_quant_zz);
		d_SequenceHeaderRead=true;

		// Try to read Sequence-Extension.
		
		SysPacket_Packet* pck = GetNextPacket();
		if (pck)
		  {
		    MemBitstreamReader bs(pck->data.AskContents() , pck->data.AskLength());
    
		    uint32 startcode = bs.GetBits(32);
		    if (startcode==STARTCODE_EXTENSION_START)
		      {
			if (bs.GetBits(4)==EXTID_SequenceExtension)
			  DecodeSequenceHeaderExt(bs,d_seqdata);

			delete pck;
		      }
		    else
		      PushbackPacket(pck);
		  }

		// Allocate images and precalculate some values.
		
		PostSequenceHeader();

		if (d_options.Trace_SeqH)
		  TraceSequenceHeader(d_seqdata);
	      }
	      break;

	    case STARTCODE_GROUP_START:
	      {
		GOPHeader gopdata;
		DecodeGOPHeader(bitstream,gopdata);

		if (d_options.Trace_GOPH)
		  TraceGOPHeader(gopdata);
	      }
	      break;

	    case STARTCODE_PICTURE_START:
	      if (d_SequenceHeaderRead)
		{
		  DecodePictureHeader(bitstream,d_pichdr);
		  BeganPictureDecoding=true;

		  // Try to read Picture-Coding-Extension.

		  SysPacket_Packet* pck2 = GetNextPacket();
		  if (pck2)
		    {
		      MemBitstreamReader bs(pck2->data.AskContents() , pck2->data.AskLength());
    
		      uint32 startcode = bs.GetBits(32);
		      if (startcode==STARTCODE_EXTENSION_START)
			{
			  if (bs.GetBits(4)==EXTID_PictureCodingExtension)
			    DecodePictureCodingExt(bs,d_pichdr);

			  delete pck2;
			}
		      else
			PushbackPacket(pck2);
		    }

		  DecodeExtensions(2);

		  // Do some precalculations, pointer settings...

		  PostPictureHeader(pck->timing);

		  if (d_options.Trace_PicH)
		    TracePictureHeader(d_pichdr);
		}
	      break;

#if 0
	    case STARTCODE_EXTENSION_START:
	      bitstream.SkipBits(32);
	      switch (bitstream.GetBits(4))
		{
		case EXTID_SequenceExtension: DecodeSequenceExtension(bitstream); break;

		default:
		  //assert(0);
		  break;
		}
	      break;

	    case STARTCODE_ISO11172_END:
	      Assert(0); /* TODO: check: I think we'll never see this as this is thrown away because
			    it's a syntax stream element.
			 */

              //Append(d_next);
              
	      break;
#endif
	    default:
	      break;
	    }
	}

      delete pck;
    }

  return true;
}



void VideoDecoder::PostSequenceHeader()
{
  d_IsMPEG2 = d_seqdata.m_IsMPEG2;

  d_ChromaFormat = d_seqdata.m_ChromaFormat;
  switch (d_ChromaFormat)
    {
    case CHROMA_420: d_dctblks= 6; d_intra_cbp=0xFC0; mb_chr_w= 8; mb_chr_h= 8; break;
    case CHROMA_422: d_dctblks= 8; d_intra_cbp=0xFF0; mb_chr_w= 8; mb_chr_h=16; break;
    case CHROMA_444: d_dctblks=12; d_intra_cbp=0xFFF; mb_chr_w=16; mb_chr_h=16; break;
    }

  d_MBWidth  = d_seqdata.m_MBWidth =(d_seqdata.m_Width +15)/16;
  d_MBHeight = d_seqdata.m_MBHeight=(d_seqdata.m_Height+15)/16;

  ImageParam param;
  if (d_last) d_last->m_image.Image<Pixel>::GetParam(param);
  if (!d_last ||
      param.width  != d_seqdata.m_Width ||
      param.height != d_seqdata.m_Height)
    {
      specs.width  = d_seqdata.m_Width;
      specs.height = d_seqdata.m_Height;
      specs.halign = 16;
      specs.valign = 16;
      specs.exact_size = true;
      specs.reduced_chroma_size = true;
      switch (d_seqdata.m_ChromaFormat)
        {
        case CHROMA_444: specs.chroma = Chroma444; break;
        case CHROMA_422: specs.chroma = Chroma422; break;
        case CHROMA_420: specs.chroma = Chroma420; break;
        }

      Assert(d_sink != NULL);
      if (d_last) delete d_last;
      if (d_curr) delete d_curr;
      if (d_next) delete d_next;

      d_last=d_curr=d_next=NULL;

      d_last = new DecodedImageData; d_last->m_image.Create(specs);
      d_next = new DecodedImageData; d_next->m_image.Create(specs);
      d_curr = new DecodedImageData; d_curr->m_image.Create(specs);

      d_last->m_width  = d_next->m_width  = d_curr->m_width  = specs.width;
      d_last->m_height = d_next->m_height = d_curr->m_height = specs.height;

      bytesperline_lum = d_last->m_image.AskBitmap(Image<Pixel>::Bitmap_Y ).AskInternalWidth();
      bytesperline_chr = d_last->m_image.AskBitmap(Image<Pixel>::Bitmap_Cr).AskInternalWidth();
    }
}


inline void VideoDecoder::SetSPOffsets(TempRef sp_tempref,int n,  // predictor of which direction
				       TempRef predref,bool field,bool topfield) // physical location
{
  Pixel **spy,**spcr,**spcb;
  if (sp_tempref==LAST) { spy=&sp_last_y[n]; spcr=&sp_last_cr[n]; spcb=&sp_last_cb[n]; }
  else                  { spy=&sp_next_y[n]; spcr=&sp_next_cr[n]; spcb=&sp_next_cb[n]; }

  DecodedImageData* buf;
  if (predref==LAST) { buf=d_last; } else { buf=d_next; }

  if (field)
    {
      *spy  = buf->m_image.AskBitmap(Image<Pixel>::Bitmap_Y ).AskField(topfield)[0];
      *spcr = buf->m_image.AskBitmap(Image<Pixel>::Bitmap_Cr).AskField(topfield)[0];
      *spcb = buf->m_image.AskBitmap(Image<Pixel>::Bitmap_Cb).AskField(topfield)[0];
    }
  else
    {
      *spy  = buf->m_image.AskFrameY()[0];
      *spcr = buf->m_image.AskFrameU()[0];
      *spcb = buf->m_image.AskFrameV()[0];
    }
}

#include <sys/time.h>
#include <unistd.h>

void VideoDecoder::PostPictureHeader(const SystemTimingInformation& timing)
{
  if (0)
  {
    struct timeval tv;
    static struct timeval last_tv;
    gettimeofday(&tv,NULL);

    static int cnt=0;
    long sec_diff = tv.tv_sec - last_tv.tv_sec;
    long udiff = sec_diff * 1000000;
    udiff += tv.tv_usec-last_tv.tv_usec;

    if (cnt>0)
      cout << cnt << " " << udiff << "\n";

    cnt++;
    last_tv=tv;
  }

  // Make some consistency checks to see if input stream is MPEG-2 compliant.

  if (d_IsMPEG2)
    {
      if (1) //options.WarnOnFalseMPEG1Fields)
        {
          if (d_pichdr.m_fullpel_fw != 0)
            {
              //Error(ErrSev_Warning,
	      //"Error in input stream (ignored): fullpel-forward motionvector requested.\n");
            }
          if (d_pichdr.m_fullpel_bw != 0)
            {
              //Error(ErrSev_Warning,
	      //"Error in input stream (ignored): fullpel-backward motionvector requested.\n");
            }
          if (d_pichdr.m_fcode[0][0] != 7)
            {
              //Error(ErrSev_Warning,
	      //"Error in input stream (ignored): forward fcode is not '7'\n");
            }
          if (d_pichdr.m_fcode[1][0] != 7)
            {
              //Error(ErrSev_Warning,
	      //"Error in input stream (ignored): backward fcode is not '7'\n");
            }
        }
    }

  d_dc_pred = (1<<(d_pichdr.m_intra_dc_precision-1));


#ifndef NDEBUG
  if (d_options.Trace_PicH)
    {
      if (timing.HasPTS) cout << "Picture-Header-PTS: " << timing.pts << endl;
    }
#endif


  // Order quantization matrix coefficients

  if (d_pichdr.m_alternate_scan)
    {
      for (int i=0;i<64;i++)
	{
	  d_quant_bs.m_LumIntra[i] = d_quant_zz.m_LumIntra[AlternateFromZigZag[i]];
	  d_quant_bs.m_LumInter[i] = d_quant_zz.m_LumInter[AlternateFromZigZag[i]];
	  d_quant_bs.m_ChrIntra[i] = d_quant_zz.m_ChrIntra[AlternateFromZigZag[i]];
	  d_quant_bs.m_ChrInter[i] = d_quant_zz.m_ChrInter[AlternateFromZigZag[i]];
	}
    }
  else
    {
      memcpy(d_quant_bs.m_LumIntra,d_quant_zz.m_LumIntra,64*sizeof(int));
      memcpy(d_quant_bs.m_LumInter,d_quant_zz.m_LumInter,64*sizeof(int));
      memcpy(d_quant_bs.m_ChrIntra,d_quant_zz.m_ChrIntra,64*sizeof(int));
      memcpy(d_quant_bs.m_ChrInter,d_quant_zz.m_ChrInter,64*sizeof(int));
    }

  d_scalability_mode = None;

  if (d_pichdr.m_picture_structure == PICSTRUCT_BottomField)
    d_field_offset = 1;
  else
    d_field_offset = 0;


  // ------------------- Now the Picture-Header is completely decoded. ---------------------

  // ------------------- Semantic actions follow. ------------------------------------------


  Assert(d_sink);

  const bool IsANewFrame = (d_pichdr.m_picture_structure == PICSTRUCT_FramePicture || d_FirstFieldInFrame);

  /* Do not display images of disabled types.
   */
  if (IsANewFrame)
    {
      d_skip_this_frame =
	(!d_options.DecodeB && d_pichdr.m_picture_coding_type == PICTYPE_B) ||
	(!d_options.DecodeP && d_pichdr.m_picture_coding_type != PICTYPE_I);
	
      // /* ShallISkipThis() */ false;
    }

  if (d_skip_this_frame)
    {
      d_FirstFieldInFrame = !d_FirstFieldInFrame;
      return;
    }
    

  /* If a new I- or P-image starts, show the last decoded I- or P- frame and
     move to d_last-buffer.
  */

  if (IsANewFrame &&
      (d_pichdr.m_picture_coding_type == PICTYPE_I ||
       d_pichdr.m_picture_coding_type == PICTYPE_P))
    {
      if (!d_next_IsEmpty)
        {
	  d_next->m_may_modify=false;
	  d_sink->BeginPicture(d_next);
          d_sink->ShowAllMBRows(d_next); // Show last decoded I- or P-image.
	  d_sink->FinishedPicture();
	  d_next->FreePictureData();     // Free the additional information data.

	  swap(d_last,d_next);    // Old forward prediction image becomes new backward prefiction.
        }
    }


  // Decode a new picture.

  if (d_pichdr.m_picture_structure == PICSTRUCT_FramePicture)
    {
      if (d_pichdr.m_picture_coding_type == PICTYPE_I ||
          d_pichdr.m_picture_coding_type == PICTYPE_P)
        {
	  d_next->m_timing = timing;

          ptr_y  = d_next->m_image.AskFrameY();
          ptr_cr = d_next->m_image.AskFrameU();
          ptr_cb = d_next->m_image.AskFrameV();

	  if (d_pichdr.m_picture_coding_type==PICTYPE_P)
	    {
	      Assert(d_last);
	      SetSPOffsets(LAST,0,       // forward prediction get its data from
			   LAST,false);  // the last frame
	    }

          lineskip_lum = bytesperline_lum;
          lineskip_chr = bytesperline_chr;

          d_next_IsEmpty=false;
        }
      else  // B-picture
        {
	  d_curr->m_timing = timing;

          ptr_y  = d_curr->m_image.AskFrameY();
          ptr_cr = d_curr->m_image.AskFrameU();
          ptr_cb = d_curr->m_image.AskFrameV();

	  SetSPOffsets(LAST,0, LAST,false);  // forward prediction from last frame
	  SetSPOffsets(NEXT,0, NEXT,false);  // backward prediction from next frame

          lineskip_lum = bytesperline_lum;
          lineskip_chr = bytesperline_chr;

          d_BFrameAvailable=true;

	  d_sink->BeginPicture(d_curr);
        }

      d_FirstFieldInFrame=true;
    }

  // ---------------- field pictures ---------------

  else if (d_pichdr.m_picture_coding_type == PICTYPE_I ||
	   d_pichdr.m_picture_coding_type == PICTYPE_P)
    {
      if (d_FirstFieldInFrame)
        {
          bool topfield = (d_pichdr.m_picture_structure==PICSTRUCT_TopField);
          ptr_y  = d_next->m_image.AskBitmap(Image<Pixel>::Bitmap_Y).AskField(topfield);
          ptr_cr = d_next->m_image.AskBitmap(Image<Pixel>::Bitmap_U).AskField(topfield);
          ptr_cb = d_next->m_image.AskBitmap(Image<Pixel>::Bitmap_V).AskField(topfield);

	  Assert(IsANewFrame);
	  d_next->m_timing = timing;

	  if (d_pichdr.m_picture_coding_type == PICTYPE_P)
	    {
	      SetSPOffsets(LAST,0, LAST,true,true);  // backward prediction from last fields
	      SetSPOffsets(LAST,1, LAST,true,false);
	    }

          lineskip_lum = bytesperline_lum*2;
          lineskip_chr = bytesperline_chr*2;

          d_next_IsEmpty=false;
          d_FirstFieldInFrame=false;
        }
      else
        {
          bool topfield = (d_pichdr.m_picture_structure==PICSTRUCT_TopField);
          ptr_y  = d_next->m_image.AskBitmap(Image<Pixel>::Bitmap_Y).AskField(topfield);
          ptr_cr = d_next->m_image.AskBitmap(Image<Pixel>::Bitmap_U).AskField(topfield);
          ptr_cb = d_next->m_image.AskBitmap(Image<Pixel>::Bitmap_V).AskField(topfield);

	  if (d_pichdr.m_picture_coding_type == PICTYPE_P)
	    {
	      if (topfield)
		{
		  SetSPOffsets(LAST,0, LAST,true,true);  // top-field prediction from last frame
		  SetSPOffsets(LAST,1, NEXT,true,false); // bottom-field prediction from current frame
		}
	      else
		{
		  SetSPOffsets(LAST,0, NEXT,true,true);  // top-field prediction from current frame
		  SetSPOffsets(LAST,1, LAST,true,false); // bottom-field prediction from last frame
		}
	    }

          lineskip_lum = bytesperline_lum*2;
          lineskip_chr = bytesperline_chr*2;

          //d_next_IsEmpty=false;
          d_FirstFieldInFrame=true;
        }
    }
  else // B-Field
    {
      bool topfield = (d_pichdr.m_picture_structure==PICSTRUCT_TopField);
      ptr_y  = d_curr->m_image.AskBitmap(Image<Pixel>::Bitmap_Y).AskField(topfield);
      ptr_cr = d_curr->m_image.AskBitmap(Image<Pixel>::Bitmap_U).AskField(topfield);
      ptr_cb = d_curr->m_image.AskBitmap(Image<Pixel>::Bitmap_V).AskField(topfield);

      SetSPOffsets(LAST,0, LAST,true,true);  // forward prediction from last frame
      SetSPOffsets(NEXT,0, NEXT,true,true);
      SetSPOffsets(LAST,1, LAST,true,false); // backward prediction from next frame
      SetSPOffsets(NEXT,1, NEXT,true,false);

      lineskip_lum = bytesperline_lum*2;
      lineskip_chr = bytesperline_chr*2;

      if (d_FirstFieldInFrame)
	{ d_sink->BeginPicture(d_curr); }

      if (!d_FirstFieldInFrame)
        d_BFrameAvailable=true;
  
      d_FirstFieldInFrame = !d_FirstFieldInFrame;
    }


  sp_curr_y  = ptr_y[0];
  sp_curr_cb = ptr_cb[0];
  sp_curr_cr = ptr_cr[0];


  // create PictureData if needed

  if (1) // d_sink->NeedsPictureData(d_pichdr.m_picture_coding_type))
    {
      DecodedImageData* img2decode=NULL;

      if (d_pichdr.m_picture_coding_type == PICTYPE_I ||
	  d_pichdr.m_picture_coding_type == PICTYPE_P)
	{ img2decode=d_next; }
      else
	{ img2decode=d_curr; }


      int mbw=0,mbh=0;
      if (d_sink->NeedsMBData(d_pichdr.m_picture_coding_type))
	{ mbw=d_MBWidth; mbh=d_MBHeight; }

      if (d_pichdr.m_picture_structure == PICSTRUCT_FramePicture ||
	  d_FirstFieldInFrame)
	{
	  Assert(img2decode->m_picdata1==NULL);
	  if (mbw) d_picdata = img2decode->m_picdata1 = PictureData::GetPictureData(mbw,mbh);
	  img2decode->m_pichdr1 = d_pichdr;
	}
      else
	{
	  Assert(img2decode->m_picdata2==NULL);
	  if (mbw) d_picdata = img2decode->m_picdata2 = PictureData::GetPictureData(mbw,mbh);
	  img2decode->m_pichdr2 = d_pichdr;
	}

      // *((PictureHeader*)d_picdata) = d_pichdr;
    }
  else
    { d_picdata=NULL; }

  /*
    cout << "Lineskip Lum: " << lineskip_lum << " Lineskip Chr: " << lineskip_chr << endl;
    cout << "BPL Lum: " << bytesperline_lum << " BPL Chr: " << bytesperline_chr << endl << endl;
  */
}


void VideoDecoder::DecodeExtensions(int n)
{
  for (;;)
    {
      SysPacket_Packet* pck = GetNextPacket();
      if (pck==NULL)
        return;

      MemBitstreamReader bs(pck->data.AskContents() , pck->data.AskLength());
      switch (bs.PeekBits(32))
        {
        case STARTCODE_EXTENSION_START:
          // TODO
          delete pck;
          break;
        case STARTCODE_USER_DATA:
          delete pck;
          break;
        default:
          PushbackPacket(pck);
          return;
        }
    }
}




void VideoDecoder::SetReferencePtrsFrm(struct PixPtrs_const& ptrs,const struct MotionVector& mv)
{
  ptrs.y  += ( mv.m_habs   >>1) + ( mv.m_vabs   >>1)*lineskip_lum;

  if (d_ChromaFormat==CHROMA_420)
    {
      ptrs.cr += ((mv.m_habs/2)>>1) + ((mv.m_vabs/2)>>1)*lineskip_chr;
      ptrs.cb += ((mv.m_habs/2)>>1) + ((mv.m_vabs/2)>>1)*lineskip_chr;
    }
  else if (d_ChromaFormat==CHROMA_422)
    {
      ptrs.cr += ((mv.m_habs/2)>>1) + (mv.m_vabs>>1)*lineskip_chr;
      ptrs.cb += ((mv.m_habs/2)>>1) + (mv.m_vabs>>1)*lineskip_chr;
    }
  else
    {
      ptrs.cr += (mv.m_habs>>1) + (mv.m_vabs>>1)*lineskip_chr;
      ptrs.cb += (mv.m_habs>>1) + (mv.m_vabs>>1)*lineskip_chr;
    }
}


void VideoDecoder::SetReferencePtrsFld(struct PixPtrs_const& ptrs,const struct MotionVector& mv)
{
  ptrs.y  += ( mv.m_habs   >>1) + ( mv.m_vabs   >>1)*lineskip_lum*2;

  if (d_ChromaFormat==CHROMA_420)
    {
      ptrs.cr += ((mv.m_habs/2)>>1) + ((mv.m_vabs/2)>>1)*lineskip_chr*2;
      ptrs.cb += ((mv.m_habs/2)>>1) + ((mv.m_vabs/2)>>1)*lineskip_chr*2;
    }
  else if (d_ChromaFormat==CHROMA_422)
    {
      ptrs.cr += ((mv.m_habs/2)>>1) + (mv.m_vabs>>1)*lineskip_chr*2;
      ptrs.cb += ((mv.m_habs/2)>>1) + (mv.m_vabs>>1)*lineskip_chr*2;
    }
  else
    {
      ptrs.cr += (mv.m_habs>>1) + (mv.m_vabs>>1)*lineskip_chr*2;
      ptrs.cb += (mv.m_habs>>1) + (mv.m_vabs>>1)*lineskip_chr*2;
    }
}

#if 0
inline int DequantizeIntra(int value,int qscale,int matrix)
{
#if MMX_DCT
  int deq_value = value*qscale*matrix;
#else
  int deq_value = value*qscale*matrix/16;
#endif

  return deq_value;
}

inline int DequantizeInter(int value,int qscale,int matrix)
{
#if MMX_DCT
  int sign = (value>0 ? 1 : -1);
  int deq_value = (2*value+sign) * matrix * qscale / 2;
#else
  int sign = (value>0 ? 1 : -1);
  int deq_value = (2*value+sign) * matrix * qscale / 32;
#endif

  return deq_value;
}
#endif


void VideoDecoder::SetHalfPelFlags1(struct MotionCompensation_SglMB::MCData& mcdata,
				    const struct MotionVector& mv)
{
  mcdata.LumaHalfFlags    = ( mv.m_vabs   &1);    // set MC_Last_HalfV
  mcdata.LumaHalfFlags   |= ( mv.m_habs   &1)<<1; // set MC_Last_HalfH
  mcdata.ChromaHalfFlags  = ((mv.m_vabs/2)&1);    // set MC_Last_HalfV
  mcdata.ChromaHalfFlags |= ((mv.m_habs/2)&1)<<1; // set MC_Last_HalfH
}

void VideoDecoder::SetHalfPelFlags2(struct MotionCompensation_SglMB::MCData& mcdata,
				    const struct MotionVector& lastmv,
				    const struct MotionVector& nextmv)
{
  mcdata.LumaHalfFlags    = ( lastmv.m_vabs   &1);    // set MC_Last_HalfV
  mcdata.LumaHalfFlags   |= ( lastmv.m_habs   &1)<<1; // set MC_Last_HalfH
  mcdata.ChromaHalfFlags  = ((lastmv.m_vabs/2)&1);    // set MC_Last_HalfV
  mcdata.ChromaHalfFlags |= ((lastmv.m_habs/2)&1)<<1; // set MC_Last_HalfH

  mcdata.LumaHalfFlags   |= ( nextmv.m_vabs   &1)<<2; // set MC_Next_HalfV
  mcdata.LumaHalfFlags   |= ( nextmv.m_habs   &1)<<3; // set MC_Next_HalfH
  mcdata.ChromaHalfFlags |= ((nextmv.m_vabs/2)&1)<<2; // set MC_Next_HalfV
  mcdata.ChromaHalfFlags |= ((nextmv.m_habs/2)&1)<<3; // set MC_Next_HalfH
}


void VideoDecoder::MC_Frame_FrameBased(const Macroblock& mb,uint16 mb_mode)
{
  struct MotionCompensation_SglMB::MCData mcdata;
  MotionCompensation_SglMB::MCompFunc*const* mclum_func;
  MotionCompensation_SglMB::MCompFunc*const* mcchr_func;

  mcdata.bytesperline_lum = lineskip_lum;
  mcdata.bytesperline_chr = lineskip_chr;
  mcdata.blkheight=16;
  mcdata.blkheight_chr=mb_chr_h;
                    
  mcdata.currimg.y  = dp_y;
  mcdata.currimg.cr = dp_cr;
  mcdata.currimg.cb = dp_cb;

  int offs_y  = dp_y  - sp_curr_y;
  int offs_cr = dp_cr - sp_curr_cr;
  int offs_cb = dp_cb - sp_curr_cb;
  Assert(offs_cr==offs_cb);

  if ((mb_mode & (MBMODE_MVFWD|MBMODE_MVBKW))==(MBMODE_MVFWD|MBMODE_MVBKW))
    {
      mcdata.lastimg.y  = offs_y  + sp_last_y[0];
      mcdata.lastimg.cr = offs_cr + sp_last_cr[0];
      mcdata.lastimg.cb = offs_cb + sp_last_cb[0];
                        
      mcdata.nextimg.y  = offs_y  + sp_next_y[0];
      mcdata.nextimg.cr = offs_cr + sp_next_cr[0];
      mcdata.nextimg.cb = offs_cb + sp_next_cb[0];

      SetReferencePtrsFrm(mcdata.lastimg,mb.m_forward1);
      SetReferencePtrsFrm(mcdata.nextimg,mb.m_backward1);

      SetHalfPelFlags2(mcdata,mb.m_forward1,mb.m_backward1);

      mclum_func = d_motcomp->AskMCompFunc_Dbl_Luma();
      mcchr_func = d_motcomp->AskMCompFunc_Dbl_Chroma(CHROMA_420);
    }
  else
    {
      const MotionVector* mv;
      if (mb.m_HasMotionForward)
	{
	  mv = &mb.m_forward1;

	  mcdata.lastimg.y  = offs_y  + sp_last_y[0];
	  mcdata.lastimg.cr = offs_cr + sp_last_cr[0];
	  mcdata.lastimg.cb = offs_cb + sp_last_cb[0];
	}
      else
	{
	  mv = &mb.m_backward1;

	  mcdata.lastimg.y  = offs_y  + sp_next_y[0];
	  mcdata.lastimg.cr = offs_cr + sp_next_cr[0];
	  mcdata.lastimg.cb = offs_cb + sp_next_cb[0];
	}
                        
      mcdata.nextimg.y  = mcdata.nextimg.cr = mcdata.nextimg.cb = NULL;

      SetReferencePtrsFrm(mcdata.lastimg,*mv);
      SetHalfPelFlags1(mcdata,*mv);

      mclum_func = d_motcomp->AskMCompFunc_Sgl_Luma();
      mcchr_func = d_motcomp->AskMCompFunc_Sgl_Chroma(CHROMA_420);
    }

  (*(mclum_func[mcdata.LumaHalfFlags  ]))(&mcdata);
  (*(mcchr_func[mcdata.ChromaHalfFlags]))(&mcdata);
}


void VideoDecoder::MC_Frame_FieldBased(const Macroblock& mb,uint16 mb_mode)
{
  struct MotionCompensation_SglMB::MCData mcdata;
  MotionCompensation_SglMB::MCompFunc*const* mclum_func;
  MotionCompensation_SglMB::MCompFunc*const* mcchr_func;

  int offs_y  = dp_y  - sp_curr_y;
  int offs_cr = dp_cr - sp_curr_cr;
  int offs_cb = dp_cb - sp_curr_cb;
  Assert(offs_cr==offs_cb);

  for (int i=0;i<2;i++)
    {
      const MotionVector* mvfwd;
      const MotionVector* mvbkw;
      if (i==0)
	{
	  mcdata.currimg.y  = dp_y;
	  mcdata.currimg.cr = dp_cr;
	  mcdata.currimg.cb = dp_cb;
	  mvfwd = &mb.m_forward1;
	  mvbkw = &mb.m_backward1;
	}
      else
	{
	  mcdata.currimg.y  = dp_y +lineskip_lum;
	  mcdata.currimg.cr = dp_cr+lineskip_chr;
	  mcdata.currimg.cb = dp_cb+lineskip_chr;
	  mvfwd = &mb.m_forward2;
	  mvbkw = &mb.m_backward2;
	}

      if ((mb_mode & (MBMODE_MVFWD|MBMODE_MVBKW))==(MBMODE_MVFWD|MBMODE_MVBKW))
	{
	  if (mvfwd->m_vertical_field_select)
	    {
	      mcdata.lastimg.y  = offs_y  + sp_last_y[0]  + lineskip_lum;
	      mcdata.lastimg.cr = offs_cr + sp_last_cr[0] + lineskip_chr;
	      mcdata.lastimg.cb = offs_cb + sp_last_cb[0] + lineskip_chr;
	    }
	  else
	    {
	      mcdata.lastimg.y  = offs_y  + sp_last_y[0];
	      mcdata.lastimg.cr = offs_cr + sp_last_cr[0];
	      mcdata.lastimg.cb = offs_cb + sp_last_cb[0];
	    }

	  if (mvbkw->m_vertical_field_select)
	    {
	      mcdata.nextimg.y  = offs_y  + sp_next_y[0]  + lineskip_lum;
	      mcdata.nextimg.cr = offs_cr + sp_next_cr[0] + lineskip_chr;
	      mcdata.nextimg.cb = offs_cb + sp_next_cb[0] + lineskip_chr;
	    }
	  else
	    {
	      mcdata.nextimg.y  = offs_y  + sp_next_y[0];
	      mcdata.nextimg.cr = offs_cr + sp_next_cr[0];
	      mcdata.nextimg.cb = offs_cb + sp_next_cb[0];
	    }

	  SetReferencePtrsFld(mcdata.lastimg,*mvfwd);
	  SetReferencePtrsFld(mcdata.nextimg,*mvbkw);

	  SetHalfPelFlags2(mcdata,*mvfwd,*mvbkw);
                            
	  mclum_func = d_motcomp->AskMCompFunc_Dbl_Luma();
	  mcchr_func = d_motcomp->AskMCompFunc_Dbl_Chroma(CHROMA_420);
	}
      else
	{
	  const MotionVector* mv;
	  if (mb.m_HasMotionForward)
	    {
	      mv = mvfwd;
                                
	      if (mvfwd->m_vertical_field_select)
		{
		  mcdata.lastimg.y  = offs_y  + sp_last_y[0]  + lineskip_lum;
		  mcdata.lastimg.cr = offs_cr + sp_last_cr[0] + lineskip_chr;
		  mcdata.lastimg.cb = offs_cb + sp_last_cb[0] + lineskip_chr;
		}
	      else
		{
		  mcdata.lastimg.y  = offs_y  + sp_last_y[0];
		  mcdata.lastimg.cr = offs_cr + sp_last_cr[0];
		  mcdata.lastimg.cb = offs_cb + sp_last_cb[0];
		}
	    }
	  else
	    {
	      mv = mvbkw;
                                
	      if (mvbkw->m_vertical_field_select)
		{
		  mcdata.lastimg.y  = offs_y  + sp_next_y[0]  + lineskip_lum;
		  mcdata.lastimg.cr = offs_cr + sp_next_cr[0] + lineskip_chr;
		  mcdata.lastimg.cb = offs_cb + sp_next_cb[0] + lineskip_chr;
		}
	      else
		{
		  mcdata.lastimg.y  = offs_y  + sp_next_y[0];
		  mcdata.lastimg.cr = offs_cr + sp_next_cr[0];
		  mcdata.lastimg.cb = offs_cb + sp_next_cb[0];
		}
	    }
                            
	  SetReferencePtrsFld(mcdata.lastimg,*mv);
	  SetHalfPelFlags1(mcdata,*mv);
                            
	  mclum_func = d_motcomp->AskMCompFunc_Sgl_Luma();
	  mcchr_func = d_motcomp->AskMCompFunc_Sgl_Chroma(CHROMA_420);
	}

      mcdata.bytesperline_lum = lineskip_lum*2;
      mcdata.bytesperline_chr = lineskip_chr*2;
      mcdata.blkheight=8;
      mcdata.blkheight_chr=mb_chr_h/2;

      (*(mclum_func[mcdata.LumaHalfFlags]))(&mcdata);
      (*(mcchr_func[mcdata.ChromaHalfFlags]))(&mcdata);
    }
}


void VideoDecoder::MC_Field_FieldBased(const Macroblock& mb,uint16 mb_mode)
{
  struct MotionCompensation_SglMB::MCData mcdata;
  MotionCompensation_SglMB::MCompFunc*const* mclum_func;
  MotionCompensation_SglMB::MCompFunc*const* mcchr_func;

  mcdata.currimg.y  = dp_y;
  mcdata.currimg.cr = dp_cr;
  mcdata.currimg.cb = dp_cb;

  mcdata.bytesperline_lum = lineskip_lum;
  mcdata.bytesperline_chr = lineskip_chr;
  mcdata.blkheight=16;
  mcdata.blkheight_chr=mb_chr_h;

  int offs_y  = dp_y  - sp_curr_y;
  int offs_cr = dp_cr - sp_curr_cr;
  int offs_cb = dp_cb - sp_curr_cb;
  Assert(offs_cr==offs_cb);

  if ((mb_mode & (MBMODE_MVFWD|MBMODE_MVBKW))==(MBMODE_MVFWD|MBMODE_MVBKW))
    {
      mcdata.lastimg.y  = offs_y  + sp_last_y [mb.m_forward1.m_vertical_field_select];
      mcdata.lastimg.cr = offs_cr + sp_last_cr[mb.m_forward1.m_vertical_field_select];
      mcdata.lastimg.cb = offs_cb + sp_last_cb[mb.m_forward1.m_vertical_field_select];
                        
      mcdata.nextimg.y  = offs_y  + sp_next_y [mb.m_backward1.m_vertical_field_select];
      mcdata.nextimg.cr = offs_cr + sp_next_cr[mb.m_backward1.m_vertical_field_select];
      mcdata.nextimg.cb = offs_cb + sp_next_cb[mb.m_backward1.m_vertical_field_select];
                        
      SetReferencePtrsFrm(mcdata.lastimg,mb.m_forward1);
      SetReferencePtrsFrm(mcdata.nextimg,mb.m_backward1);
                        
      SetHalfPelFlags2(mcdata,mb.m_forward1,mb.m_backward1);
                        
      mclum_func = d_motcomp->AskMCompFunc_Dbl_Luma();
      mcchr_func = d_motcomp->AskMCompFunc_Dbl_Chroma(CHROMA_420);
    }
  else
    {
      const MotionVector* mv;
      if (mb.m_HasMotionForward)
	{
	  mv = &mb.m_forward1;
                            
	  mcdata.lastimg.y  = offs_y +sp_last_y [mb.m_forward1.m_vertical_field_select];
	  mcdata.lastimg.cr = offs_cr+sp_last_cr[mb.m_forward1.m_vertical_field_select];
	  mcdata.lastimg.cb = offs_cb+sp_last_cb[mb.m_forward1.m_vertical_field_select];
	}
      else
	{
	  mv = &mb.m_backward1;
                            
	  mcdata.lastimg.y  =offs_y +sp_next_y [mb.m_backward1.m_vertical_field_select];
	  mcdata.lastimg.cr =offs_cr+sp_next_cr[mb.m_backward1.m_vertical_field_select];
	  mcdata.lastimg.cb =offs_cb+sp_next_cb[mb.m_backward1.m_vertical_field_select];
	}
                        
      SetReferencePtrsFrm(mcdata.lastimg,*mv);
      SetHalfPelFlags1(mcdata,*mv);
                        
      mclum_func = d_motcomp->AskMCompFunc_Sgl_Luma();
      mcchr_func = d_motcomp->AskMCompFunc_Sgl_Chroma(CHROMA_420);
    }
                    
  (*(mclum_func[mcdata.LumaHalfFlags]))(&mcdata);
  (*(mcchr_func[mcdata.ChromaHalfFlags]))(&mcdata);
}

void VideoDecoder::MC_Field_16x8(const Macroblock& mb,uint16 mb_mode)
{
  struct MotionCompensation_SglMB::MCData mcdata;
  MotionCompensation_SglMB::MCompFunc*const* mclum_func;
  MotionCompensation_SglMB::MCompFunc*const* mcchr_func;

  mcdata.currimg.y  = dp_y;
  mcdata.currimg.cr = dp_cr;
  mcdata.currimg.cb = dp_cb;

  mcdata.bytesperline_lum = lineskip_lum;
  mcdata.bytesperline_chr = lineskip_chr;
  mcdata.blkheight=8;
  mcdata.blkheight_chr=mb_chr_h/2;

  int offs_y  = dp_y  - sp_curr_y;
  int offs_cr = dp_cr - sp_curr_cr;
  int offs_cb = dp_cb - sp_curr_cb;
  Assert(offs_cr==offs_cb);

  for (int v=0;v<2;v++)
    {
      if ((mb_mode & (MBMODE_MVFWD|MBMODE_MVBKW))==(MBMODE_MVFWD|MBMODE_MVBKW))
	{
	  mcdata.lastimg.y  = offs_y  + sp_last_y [mb.m_vector[v][0].m_vertical_field_select];
	  mcdata.lastimg.cr = offs_cr + sp_last_cr[mb.m_vector[v][0].m_vertical_field_select];
	  mcdata.lastimg.cb = offs_cb + sp_last_cb[mb.m_vector[v][0].m_vertical_field_select];
                        
	  mcdata.nextimg.y  = offs_y  + sp_next_y [mb.m_vector[v][1].m_vertical_field_select];
	  mcdata.nextimg.cr = offs_cr + sp_next_cr[mb.m_vector[v][1].m_vertical_field_select];
	  mcdata.nextimg.cb = offs_cb + sp_next_cb[mb.m_vector[v][1].m_vertical_field_select];
                        
	  SetReferencePtrsFrm(mcdata.lastimg,mb.m_vector[v][0]);
	  SetReferencePtrsFrm(mcdata.nextimg,mb.m_vector[v][1]);
                        
	  SetHalfPelFlags2(mcdata,mb.m_vector[v][0],mb.m_vector[v][1]);
                        
	  mclum_func = d_motcomp->AskMCompFunc_Dbl_Luma();
	  mcchr_func = d_motcomp->AskMCompFunc_Dbl_Chroma(CHROMA_420);
	}
      else
	{
	  const MotionVector* mv;
	  if (mb.m_HasMotionForward)
	    {
	      mv = &mb.m_vector[v][0];
                            
	      mcdata.lastimg.y  = offs_y +sp_last_y [mb.m_vector[v][0].m_vertical_field_select];
	      mcdata.lastimg.cr = offs_cr+sp_last_cr[mb.m_vector[v][0].m_vertical_field_select];
	      mcdata.lastimg.cb = offs_cb+sp_last_cb[mb.m_vector[v][0].m_vertical_field_select];
	    }
	  else
	    {
	      mv = &mb.m_vector[v][1];
                            
	      mcdata.lastimg.y  =offs_y +sp_next_y [mb.m_vector[v][1].m_vertical_field_select];
	      mcdata.lastimg.cr =offs_cr+sp_next_cr[mb.m_vector[v][1].m_vertical_field_select];
	      mcdata.lastimg.cb =offs_cb+sp_next_cb[mb.m_vector[v][1].m_vertical_field_select];
	    }
                        
	  SetReferencePtrsFrm(mcdata.lastimg,*mv);
	  SetHalfPelFlags1(mcdata,*mv);
                        
	  mclum_func = d_motcomp->AskMCompFunc_Sgl_Luma();
	  mcchr_func = d_motcomp->AskMCompFunc_Sgl_Chroma(CHROMA_420);
	}
                    
      (*(mclum_func[mcdata.LumaHalfFlags]))(&mcdata);
      (*(mcchr_func[mcdata.ChromaHalfFlags]))(&mcdata);

      mcdata.currimg.y  += lineskip_lum*8;
      mcdata.currimg.cb += lineskip_chr*mb_chr_h/2;
      mcdata.currimg.cr += lineskip_chr*mb_chr_h/2;

      offs_y  += lineskip_lum*8;
      offs_cr += lineskip_chr*mb_chr_h/2;
      offs_cb += lineskip_chr*mb_chr_h/2;
    }
}

static bool cbblks[12] =
{ false,false,false,false,
  true, false,true, false,
  true, false,true, false
};


static int blk_xoffs[12] = { 0,8,0,8, 0,0,0,0, 8,8,8,8 };
static int blk_yoffs[12] = { 0,0,8,8, 0,0,8,8, 0,0,8,8 };

static int blk_xoffs_field[12] = { 0,8,0,8, 0,0,0,0, 8,8,8,8 };
static int blk_yoffs_field[12] = { 0,0,1,1, 0,0,1,1, 0,0,1,1 };


inline void VideoDecoder::AdvancePtrsMB()
{
  dp_y  += 16;
  dp_cr +=  mb_chr_w;
  dp_cb += mb_chr_w;
}

inline void VideoDecoder::AdvancePtrsMBRow()
{
  dp_y  += 15*lineskip_lum;
  int cskip = 7*lineskip_chr + bytesperline_chr-d_MBWidth*mb_chr_w;
  dp_cr += cskip;
  dp_cb += cskip;
}


void VideoDecoder::FlushBSlice(int mbrow)
{
#ifndef SEND_FULL_BFRAMES
  d_curr->m_may_modify = true;
  d_curr->m_src_y_start = d_field_offset;

  if (d_pichdr.m_picture_structure == PICSTRUCT_FramePicture)
    { d_curr->m_src_y_end   = 15;                d_curr->m_field_lines=false; }
  else 
    { d_curr->m_src_y_end   = 30+d_field_offset; d_curr->m_field_lines=true; }

  switch (d_pichdr.m_picture_structure)
    {
    case PICSTRUCT_FramePicture: d_curr->m_dst_y_start = mbrow*16;   break;
    case PICSTRUCT_TopField:     d_curr->m_dst_y_start = mbrow*32;   break;
    case PICSTRUCT_BottomField:  d_curr->m_dst_y_start = mbrow*32+1; break;
    }

  int end_line = d_curr->m_dst_y_start + (d_curr->m_src_y_end-d_curr->m_src_y_start);
  if (end_line >= d_seqdata.m_Height)
    {
      end_line = d_seqdata.m_Height-1;
      d_curr->m_src_y_end = end_line  + d_curr->m_src_y_start - d_curr->m_dst_y_start;
    }
      
  d_sink->ShowMBRows(d_curr);  // Show image

  const int nRows = ((d_pichdr.m_picture_structure == PICSTRUCT_FramePicture) ? 1 : 2);

  for (int n=0;n<2;n++)
    {
      sp_last_y[n]  += nRows*bytesperline_lum*16;
      sp_last_cr[n] += nRows*bytesperline_chr*mb_chr_h;
      sp_last_cb[n] += nRows*bytesperline_chr*mb_chr_h;

      sp_next_y[n]  += nRows*bytesperline_lum*16;
      sp_next_cr[n] += nRows*bytesperline_chr*mb_chr_h;
      sp_next_cb[n] += nRows*bytesperline_chr*mb_chr_h;
    }      

  dp_y  = ptr_y [0];
  dp_cr = ptr_cr[0];
  dp_cb = ptr_cb[0];
#endif
}

void VideoDecoder::DecodeSlice(class FastBitBuf& bs)
{
  try
    {
      if (d_skip_this_frame)
	return;

      const bool IsBPicture = (d_pichdr.m_picture_coding_type == PICTYPE_B);

      int startcode = bs.GetBits(24);
      Assert(startcode==0x000001);

      struct
      {
	uint16 vpos;
	uint8  priority_breakpoint;
	uint5  quantiser_scale_code;
	bool   intra_slice;
      } slicehdr; 

      slicehdr.intra_slice=false;


      // *** decode slice header ***
      slicehdr.vpos = bs.GetBits(8)-1;


      struct MotionCompensation_SglMB::MCData mcdata;
      MotionCompensation_SglMB::MCompFunc*const* mclum_func;
      MotionCompensation_SglMB::MCompFunc*const* mcchr_func;


      int PMV [2 /* f/s */] [2 /* fw/bw */] [2 /* h/v */];

#ifndef SEND_FULL_BFRAMES
      if (IsBPicture)
	{
	  // reuse the same lines every time

	  dp_y  = ptr_y [0]-16;
	  dp_cr = ptr_cr[0]-mb_chr_w;
	  dp_cb = ptr_cb[0]-mb_chr_w;
	}
      else
#endif
	{
	  dp_y  = ptr_y [slicehdr.vpos*16]-16;
	  dp_cr = ptr_cr[slicehdr.vpos*mb_chr_h]-mb_chr_w;
	  dp_cb = ptr_cb[slicehdr.vpos*mb_chr_h]-mb_chr_w;
	}

      if (d_MBHeight > 175) // <==>  height > 2800
	slicehdr.vpos += bs.GetBits(3)<<7;

      if (d_scalability_mode == DataPartitioning)
	slicehdr.priority_breakpoint = bs.GetBits(7);

      slicehdr.quantiser_scale_code = bs.GetBits(5);
      if (bs.GetBits(1)==1)
	{
	  slicehdr.intra_slice = bs.GetBits(1);
	  bs.SkipBits(7);

	  while (bs.GetBits(1)==1)
	    bs.SkipBits(8);
	}

      if (d_options.Trace_SlcH)
	{
	  cout << "SliceHeader:\n"
	       << "  vertical pos:         " << slicehdr.vpos << endl
	       << "  quantiser scale code: " << ((int)slicehdr.quantiser_scale_code) << endl
	       << "  intra slice:          " << (slicehdr.intra_slice?"true":"false") << endl;
	}


      // *** decode all macroblocks in slice ***

      int dcpred_y  = d_dc_pred;
      int dcpred_cb = d_dc_pred;
      int dcpred_cr = d_dc_pred;

      bzero(&PMV[0][0][0],2*2*2*sizeof(int)); // Reset MV-predictions.

      const Macroblock* lastmb = NULL;
      uint16 last_mb_mode=0;

      int mb_addr=-1;
      int mb_row =slicehdr.vpos;
      int qcode  =slicehdr.quantiser_scale_code;

      while (bs.AskBitsLeft() >= 2 && bs.PeekBits(min(bs.AskBitsLeft(),23UL)) != 0)
	{
	  if (!d_IsMPEG2)
	    {
	      // Macroblock-Stuffing

	      while (bs.PeekBits(11)==0x00F) bs.SkipBits(11);
	    }

	  int incr = GetMBAddrIncr(bs);

#ifndef NDEBUG
	  if (d_options.Trace_MB)
	    {
	      cout << "--- MB --- " << slicehdr.vpos << " " << mb_addr+1 << endl;
	      cout << "last MBAddr: " << mb_addr << " increment: " << incr << endl;
	    }
#endif

	  // Handle skipped macroblocks. Note: the first MBincr in a slice does not define skipped MBs.
	  if (incr>1 && mb_addr>=0)
	    {
	      dcpred_y  = dcpred_cb = dcpred_cr = d_dc_pred;

	      if (d_pichdr.m_picture_coding_type == PICTYPE_B)
		{
		  for (int i=incr;i>0;i--)
		    {
		      AdvancePtrsMB();
		      mb_addr++;

		      if (mb_addr==d_MBWidth)
			{
			  AdvancePtrsMBRow();
			  if (IsBPicture) FlushBSlice(mb_row);
			  mb_row++;
			  mb_addr=0;
			}

		      if (i>1)
			{
			  if (d_pichdr.m_picture_structure==PICSTRUCT_FramePicture)
			    MC_Frame_FrameBased(*lastmb,last_mb_mode);
			  else
			    MC_Field_FieldBased(*lastmb,last_mb_mode);
			}
		    }
		}
	      else if (d_pichdr.m_picture_coding_type == PICTYPE_P)
		{
		  bzero(&PMV[0][0][0],2*2*2*sizeof(int)); // Reset PMVs

		  incr--;
		  mb_addr++;
		  AdvancePtrsMB();
		  if (mb_addr==d_MBWidth)
		    {
		      AdvancePtrsMBRow();
		      if (IsBPicture) FlushBSlice(mb_row);
		      mb_row++;
		      mb_addr=0;
		    }

		  while (incr)
		    {
		      int len;
		      if (mb_addr+incr >= d_MBWidth) len = d_MBWidth-mb_addr;
		      else                           len = incr;

		      int y0=mb_row*8;
		      int x0=mb_addr*8;
		      int plen = len*8;

		      const Pixel* sp = dp_y-sp_curr_y+sp_last_y[d_field_offset];
		      Pixel* dp = dp_y;

		      int h = 16;
                        
		      for (int y=0;y<h;y++)
			{
			  memcpy(dp,sp,plen+plen);
			  sp += lineskip_lum;
			  dp += lineskip_lum;
			}

		      sp = dp_cr-sp_curr_cr+sp_last_cr[d_field_offset];
		      dp = dp_cr;

		      for (int y=0;y<h/2;y++)
			{
			  memcpy(dp,sp,plen);
			  sp += lineskip_chr;
			  dp += lineskip_chr;
			}

		      sp = dp_cb-sp_curr_cb+sp_last_cb[d_field_offset];
		      dp = dp_cb;

		      for (int y=0;y<h/2;y++)
			{
			  memcpy(dp,sp,plen);
			  sp += lineskip_chr;
			  dp += lineskip_chr;
			}

		      mb_addr += len;
		      for (int i=0;i<len;i++) AdvancePtrsMB();
		      if (mb_addr==d_MBWidth)
			{
			  AdvancePtrsMBRow();
			  if (IsBPicture) FlushBSlice(mb_row);
			  mb_addr=0;
			  mb_row++;
			}
		      else
			{
			  Assert(mb_addr < d_MBWidth);
			}

		      incr -= len;
		    }
		}
	      else if (d_pichdr.m_picture_coding_type == PICTYPE_I)
		{
		  /* It is legal for the slice not to start in the first
		     column. But it is illegal to skip any macroblocks in I pictures. */
#if 1
		  throw Excpt_StreamError(ErrSev_Warning,
					  "Invalid MPEG stream. "
					  "Macroblock increment not 1 in I picture");
#endif
		}
	      else
		{ NotImplemented  /* TODO: D-pictures ? */ }
	    }
	  else
	    {
	      /* TODO: Should we do something if incr>=d_MBWidth ??? */

	      mb_addr += incr;
	      for (int i=0;i<incr;i++)
		AdvancePtrsMB();
	    }

	  static Macroblock s_mb;
	  Macroblock& mb = ((d_picdata && d_picdata->m_codedimage.IsInitialized()) ?
			    d_picdata->m_codedimage.Ask(mb_row,mb_addr) :
			    s_mb);


	  // process around-end-of-line-wrap in MPEG-1
	  Assert(mb_addr>=0);
	  if ((uint32)mb_addr >= d_MBWidth)
	    {
	      if (d_IsMPEG2)
		throw Excpt_StreamError(ErrSev_Warning, /* TODO: Put this into a separate
							   Exceptionclass for recovering */
					"Invalid MPEG-2 input stream. "
					"Slice is wider than image width.");

	      int nlines = mb_addr/d_MBWidth;

	      for (int i=0;i<nlines;i++)
		AdvancePtrsMBRow();

	      if (IsBPicture) FlushBSlice(mb_row);

	      mb_row  += nlines;
	      mb_addr %= d_MBWidth;
	    }



	  // -------- macroblock_modes() ----------

	  if (d_pichdr.m_picture_coding_type>4)
	    throw Excpt_StreamError(ErrSev_Error,
				    "Invalid MPEG stream, picture coding type >4 read.");

	  uint16 mb_mode = GetMBMode[d_pichdr.m_picture_coding_type](bs);



	  /* TODO: Spatial_Temporal_Weight_Code missing */

	  if (mb_mode & (MBMODE_MVFWD|MBMODE_MVBKW))
	    {
	      if (d_pichdr.m_picture_structure == PICSTRUCT_FramePicture)
		{
		  if (d_pichdr.m_frame_pred_frame_dct)
		    {
		      mb.SetFrameMotionType(Macroblock::FRMT_FrameBased);
		    }
		  else
		    {
		      mb.SetFrameMotionType((enum Macroblock::FrameMotionType)bs.GetBits(2));
		    }
		}
	      else
		{
		  mb.SetFieldMotionType((enum Macroblock::FieldMotionType)bs.GetBits(2));
		}
	    }
	  else if ((mb_mode & MBMODE_INTRA) && d_pichdr.m_concealment_motion_vectors)
	    {
	      if (d_pichdr.m_picture_structure == PICSTRUCT_FramePicture)
		{
		  mb.SetFrameMotionType(Macroblock::FRMT_FrameBased);
		}
	      else
		{
		  mb.SetFieldMotionType(Macroblock::FIMT_FieldBased);
		}
	    }

	  mb.m_FieldDCT=false;
	  if (d_IsMPEG2 &&
	      d_pichdr.m_picture_structure == PICSTRUCT_FramePicture &&
	      d_pichdr.m_frame_pred_frame_dct == false &&
	      (mb_mode & (MBMODE_INTRA|MBMODE_PATTERN)))
	    {
	      mb.m_FieldDCT = bs.GetBits(1);
	    }

	  // -------- end of macroblock_modes() -----------

	  if (!(mb_mode & MBMODE_INTRA))
	    {
	      dcpred_y  = 
		dcpred_cb = 
		dcpred_cr = d_dc_pred;
	    }

	  // Reset motionvektor predictions
	  if (((mb_mode & MBMODE_INTRA) && !d_pichdr.m_concealment_motion_vectors) ||
	      (d_pichdr.m_picture_coding_type == PICTYPE_P && !(mb_mode & MBMODE_MVFWD) && !(mb_mode & MBMODE_INTRA)))
	    {
	      bzero(&PMV[0][0][0],2*2*2*sizeof(int));
	    }

	  if (mb_mode & MBMODE_QUANT)
	    qcode = bs.GetBits(5);

	  mb.m_quantiser_scale_code = qcode;


	  bool FieldMVInFramePic = (mb.m_mv_format == Macroblock::MVFormat_Frame &&
				    mb.m_frame_motion_type == Macroblock::FRMT_FieldBased);

	  if (mb.m_mv_format == Macroblock::MVFormat_Frame &&
	      mb.m_frame_motion_type == Macroblock::FRMT_DualPrime)
	    FieldMVInFramePic = true;

	  if ((mb_mode & MBMODE_MVFWD) ||
	      ((mb_mode & MBMODE_INTRA) && d_pichdr.m_concealment_motion_vectors))
	    {
	      // --- motion_vectors(0) ---

	      if (mb.m_motion_vector_count == 1)
		{
		  if (FieldMVInFramePic)
		    {
		      DIV2(PMV[0][0][1]);
		    }

		  mb.m_vector[0][0].m_habs = PMV[0][0][0];
		  mb.m_vector[0][0].m_vabs = PMV[0][0][1];

		  if (mb.m_mv_format == Macroblock::MVFormat_Field && mb.m_dmv==false)
		    { mb.m_vector[0][0].m_vertical_field_select = bs.GetBits(1); }
		  motion_vector(mb.m_vector[0][0],0,mb.m_dmv,bs);

		  PMV[0][0][0] = mb.m_vector[0][0].m_habs;
		  PMV[0][0][1] = mb.m_vector[0][0].m_vabs;

		  if (FieldMVInFramePic)
		    {
		      PMV[0][0][1] *= 2;
		    }

		  PMV[1][0][0] = PMV[0][0][0];    // Nach Table 7-9
		  PMV[1][0][1] = PMV[0][0][1];
		}
	      else
		{
		  if (FieldMVInFramePic)
		    {
		      DIV2(PMV[0][0][1]);
		      DIV2(PMV[1][0][1]);
		    }

		  mb.m_vector[0][0].m_habs = PMV[0][0][0];
		  mb.m_vector[0][0].m_vabs = PMV[0][0][1];
		  mb.m_vector[1][0].m_habs = PMV[1][0][0];
		  mb.m_vector[1][0].m_vabs = PMV[1][0][1];

		  mb.m_vector[0][0].m_vertical_field_select = bs.GetBits(1);
		  motion_vector(mb.m_vector[0][0],0,mb.m_dmv,bs);
		  mb.m_vector[1][0].m_vertical_field_select = bs.GetBits(1);
		  motion_vector(mb.m_vector[1][0],0,mb.m_dmv,bs);

		  PMV[0][0][0] = mb.m_vector[0][0].m_habs;
		  PMV[0][0][1] = mb.m_vector[0][0].m_vabs;
		  PMV[1][0][0] = mb.m_vector[1][0].m_habs;
		  PMV[1][0][1] = mb.m_vector[1][0].m_vabs;

		  if (FieldMVInFramePic)
		    {
		      PMV[0][0][1] *= 2;
		      PMV[1][0][1] *= 2;
		    }
		}
	    }

	  if (mb_mode & MBMODE_MVBKW)
	    {
	      // --- motion_vectors(1) ---

	      if (mb.m_motion_vector_count == 1)
		{
		  if (FieldMVInFramePic)
		    {
		      DIV2(PMV[0][1][1]);
		    }

		  mb.m_vector[0][1].m_habs = PMV[0][1][0];
		  mb.m_vector[0][1].m_vabs = PMV[0][1][1];

		  if (mb.m_mv_format == Macroblock::MVFormat_Field && mb.m_dmv==false)
		    { mb.m_vector[0][1].m_vertical_field_select = bs.GetBits(1); }
		  motion_vector(mb.m_vector[0][1],1,mb.m_dmv,bs);

		  PMV[0][1][0] = mb.m_vector[0][1].m_habs;
		  PMV[0][1][1] = mb.m_vector[0][1].m_vabs;

		  if (FieldMVInFramePic)
		    {
		      PMV[0][1][1] *= 2;
		    }

		  PMV[1][1][0] = PMV[0][1][0];  // Nach Table 7-9
		  PMV[1][1][1] = PMV[0][1][1];
		}
	      else
		{
		  if (FieldMVInFramePic)
		    {
		      DIV2(PMV[0][1][1]);
		      DIV2(PMV[1][1][1]);
		    }

		  mb.m_vector[0][1].m_habs = PMV[0][1][0];
		  mb.m_vector[0][1].m_vabs = PMV[0][1][1];
		  mb.m_vector[1][1].m_habs = PMV[1][1][0];
		  mb.m_vector[1][1].m_vabs = PMV[1][1][1];

		  mb.m_vector[0][1].m_vertical_field_select = bs.GetBits(1);
		  motion_vector(mb.m_vector[0][1],1,mb.m_dmv,bs);
		  mb.m_vector[1][1].m_vertical_field_select = bs.GetBits(1);
		  motion_vector(mb.m_vector[1][1],1,mb.m_dmv,bs);

		  PMV[0][1][0] = mb.m_vector[0][1].m_habs;
		  PMV[0][1][1] = mb.m_vector[0][1].m_vabs;
		  PMV[1][1][0] = mb.m_vector[1][1].m_habs;
		  PMV[1][1][1] = mb.m_vector[1][1].m_vabs;

		  if (FieldMVInFramePic)
		    {
		      PMV[0][1][1] *= 2;
		      PMV[1][1][1] *= 2;
		    }
		}
	    }

#ifndef NDEBUG
	  if (d_options.Trace_MB)
	    {
	      cout << "PMV[0][0][]: " << PMV[0][0][0] << ',' << PMV[0][0][1] << endl;
	      cout << "PMV[0][1][]: " << PMV[0][1][0] << ',' << PMV[0][1][1] << endl;
	      cout << "PMV[1][0][]: " << PMV[1][0][0] << ',' << PMV[1][0][1] << endl;
	      cout << "PMV[1][1][]: " << PMV[1][1][0] << ',' << PMV[1][1][1] << endl;
	    }
#endif

	  if ((mb_mode & MBMODE_INTRA) && d_pichdr.m_concealment_motion_vectors)
	    {
	      bs.SkipBits(1);
	    }

	  if (d_pichdr.m_picture_coding_type == PICTYPE_P && !(mb_mode & (MBMODE_MVFWD|MBMODE_INTRA)))
	    {
	      mb_mode |= MBMODE_MVFWD;
	      mb.m_forward1.m_habs=0;
	      mb.m_forward1.m_vabs=0;
	      if (d_pichdr.m_picture_structure==PICSTRUCT_FramePicture)
		{
		  mb.SetFrameMotionType(Macroblock::FRMT_FrameBased);
		}
	      else if (d_pichdr.m_picture_structure == PICSTRUCT_TopField)
		{
		  mb.SetFieldMotionType(Macroblock::FIMT_FieldBased);
		  mb.m_forward1.m_vertical_field_select = 0;
		}
	      else
		{
		  mb.SetFieldMotionType(Macroblock::FIMT_FieldBased);
		  mb.m_forward1.m_vertical_field_select = 1;
		}
	    }

	  if (mb_mode & MBMODE_INTRA) mb.m_IsIntra=true;
	  else                        mb.m_IsIntra=false;

	  mb.m_HasMotionForward  = (mb_mode & MBMODE_MVFWD);
	  mb.m_HasMotionBackward = (mb_mode & MBMODE_MVBKW);


	  // --- Motion-Compensation ---

	  if (mb_mode & (MBMODE_MVFWD|MBMODE_MVBKW))
	    {
	      if (mb.m_mv_format==Macroblock::MVFormat_Frame)
		{
		  switch (mb.m_frame_motion_type)
		    {
		    case Macroblock::FRMT_FrameBased:
		      MC_Frame_FrameBased(mb,mb_mode);
		      break;

		    case Macroblock::FRMT_FieldBased:
		      MC_Frame_FieldBased(mb,mb_mode);
		      break;

		    case Macroblock::FRMT_DualPrime:
		      cout << "Frame: DualPrime\n";
		      break;

		    default:
		      cout << "UNIMPLEMENTED FRAME MOTION\n";
		      break;
		    }
		}
	      else  // MV-Format: Field
		{
		  switch (mb.m_field_motion_type)
		    {
		    case Macroblock::FIMT_FieldBased:
		      MC_Field_FieldBased(mb,mb_mode);
		      break;

		    case Macroblock::FIMT_16x8MC:
		      MC_Field_16x8(mb,mb_mode);
		      //cout << "Fld 16x8\n";
		      break;
                  
		    case Macroblock::FIMT_DualPrime:
		      cout << "Field: DualPrime\n";
		      break;

		    default:
		      cout << "UNIMPLEMENTED FIELD MOTION\n";
		      break;
		    }
		}
	    }

	  // ------- begin of coded_block_pattern() -----------

	  uint16 cbp = 0;

	  if (mb_mode & MBMODE_PATTERN)
	    {
	      cbp=GetCBP(bs)<<6;

	      if (d_ChromaFormat==CHROMA_422)
		{ cbp |= bs.GetBits(2)<<4; }
	      else if (d_ChromaFormat==CHROMA_444)
		{ cbp |= bs.GetBits(6);    }
	    }
	  else if (mb_mode & MBMODE_INTRA)
	    {
	      cbp = d_intra_cbp;
	    }

	  // ------- end of coded_block_pattern() -----------


#ifndef NDEBUG
	  if (d_options.Trace_MB)
	    {
	      cout << "MBHdr (pos: " << mb_row << ',' << mb_addr << '=' << mb_row*d_MBWidth+mb_addr << ")\n"
		   << "  modes:  ";
	      if (mb_mode & MBMODE_QUANT)   cout << "Quant "; else cout << "..... ";
	      if (mb_mode & MBMODE_MVFWD)   cout << "MVFwd "; else cout << "..... ";
	      if (mb_mode & MBMODE_MVBKW)   cout << "MVBkw "; else cout << "..... ";
	      if (mb_mode & MBMODE_PATTERN) cout << "Pattern "; else cout << "....... ";
	      if (mb_mode & MBMODE_INTRA)   cout << "Intra\n"; else cout << ".....\n";

	      if (mb_mode & MBMODE_QUANT)   cout << "  qcode:  " << qcode << endl;
	      cout << "  nBlks:  " << d_dctblks << endl;
	      cout << "  CBP:    0x" << hex << cbp << dec << endl;

	      if (!(mb_mode & MBMODE_INTRA))
		{
		  cout << "  Prediction: ";
		  if (mb.m_mv_format==Macroblock::MVFormat_Frame)
		    {
		      switch (mb.m_frame_motion_type)
			{
			case Macroblock::FRMT_FieldBased: cout << " frame, field based\n"; break;
			case Macroblock::FRMT_FrameBased: cout << " frame, frame based\n"; break;
			case Macroblock::FRMT_DualPrime:  cout << " frame, dual prime\n"; break;
			}
		    }
		  else
		    {
		      switch (mb.m_field_motion_type)
			{
			case Macroblock::FIMT_FieldBased: cout << " field, field based\n"; break;
			case Macroblock::FIMT_16x8MC:     cout << " field, 16x8MC\n";      break;
			case Macroblock::FIMT_DualPrime:  cout << " field, dual prime\n";  break;
			}
		    }

		  cout << "fwd1 field select: " << (mb.m_forward1.m_vertical_field_select==0 ? "t\n" : "b\n");
		  cout << "fwd2 field select: " << (mb.m_forward2.m_vertical_field_select==0 ? "t\n" : "b\n");
		  cout << "bkw1 field select: " << (mb.m_backward1.m_vertical_field_select==0 ? "t\n" : "b\n");
		  cout << "bkw2 field select: " << (mb.m_backward2.m_vertical_field_select==0 ? "t\n" : "b\n");
		}

	      if (mb_mode & (MBMODE_INTRA|MBMODE_PATTERN))
		{ cout << "DCT: " << (mb.m_FieldDCT ? "field":"frame") << endl; }
	    }
#endif

	  // Decode DCT blocks.

	  int cbp_bit=0x800;

	  int qscale = qcode2qscale[d_pichdr.m_q_scale_type][mb.m_quantiser_scale_code];
	  int scanid = (d_pichdr.m_alternate_scan ? 1 : 0);

	  bool b15 = (d_pichdr.m_intra_vlc_format && (mb_mode & MBMODE_INTRA));

	  for (int b=0 ; b<d_dctblks ; b++,cbp_bit>>=1)
	    {
	      int* matrix;
	      int  mismatch_sum; // Sum of all coefficients to calculate mismatch.

	      if (mb_mode & MBMODE_INTRA)
		{
		  if (b<4) matrix=d_quant_bs.m_LumIntra;
		  else     matrix=d_quant_bs.m_ChrIntra;	
		}
	      else
		{
		  if (b<4) matrix=d_quant_bs.m_LumInter;
		  else     matrix=d_quant_bs.m_ChrInter;
		}

	      int nextcoeff;

	      if ((cbp & cbp_bit)==0)
		{
#ifndef NDEBUG
		  if (d_options.Trace_DCTCoeff)
		    cout << "all DCT-Coeff. = 0\n";
#endif
		  continue;
		}

#ifndef NDEBUG
	      if (d_options.Trace_DCTCoeff)
		cout << "DCT (" << b << "): ";
#endif

	      bzero(&mb.m_blks[b].m_coeff[0],64*sizeof(short));

	      int run,value;


	      // Decode first coefficient.

	      if (mb_mode & MBMODE_INTRA)
		{
		  if (b<4)
		    {
		      int diff  = get_luma_dc_dct_diff(bs);

		      dcpred_y += diff;
		      mb.m_blks[b].m_coeff[0] = mismatch_sum
			= (dcpred_y << d_pichdr.m_intra_dc_precision_shift);
		    }
		  else
		    {
		      int diff =get_chroma_dc_dct_diff(bs);

		      int val;
		      if (cbblks[b]) { dcpred_cb+=diff; val = dcpred_cb; }
		      else           { dcpred_cr+=diff; val = dcpred_cr; }

		      mb.m_blks[b].m_coeff[0] = mismatch_sum
			= (val << d_pichdr.m_intra_dc_precision_shift);
		    }

		  nextcoeff=1;
		}

	      if (mb_mode & MBMODE_INTRA)
		{
		  if (b15)
		    {
		      get_intra_block_B15(bs,&mb.m_blks[b].m_coeff[0],d_scan[scanid],qscale,matrix);
		    }
		  else
		    {
		      if (d_IsMPEG2)
			{
			  get_intra_block_B14(bs,&mb.m_blks[b].m_coeff[0],d_scan[scanid],qscale,matrix);
			}
		      else
			{
			  get_mpeg1_intra_block(bs,&mb.m_blks[b].m_coeff[0],d_scan[scanid],qscale,matrix);
			}
		    }
		}
	      else
		{
		  if (d_IsMPEG2)
		    {
		      get_non_intra_block(bs,&mb.m_blks[b].m_coeff[0],d_scan[scanid],qscale,matrix);
		    }
		  else
		    {
		      get_mpeg1_non_intra_block(bs,&mb.m_blks[b].m_coeff[0],d_scan[scanid],qscale,matrix);
		    }
		}


	      // Apply iDCT

	      bool add_dct = !mb.m_IsIntra;

	      if (b<4)
		{
		  int lineskip;
		  Pixel* blkstart;
		  if (mb.m_FieldDCT)
		    {
		      blkstart = &dp_y[blk_yoffs_field[b]*lineskip_lum+blk_xoffs_field[b]];
		      lineskip=lineskip_lum*2;
		    }
		  else
		    {
		      blkstart = &dp_y[blk_yoffs[b]*lineskip_lum+blk_xoffs[b]];
		      lineskip=lineskip_lum;
		    }

#if MMX_DCT
		  if (add_dct)
		    {
		      IDCT_mmx(&mb.m_blks[b].m_coeff[0],
			       &mb.m_blks[b].m_coeff[0]);

		      IDCT_16bit8bit_add(&mb.m_blks[b].m_coeff[0],blkstart,lineskip);
		    }
		  else
		    {
		      mb.m_blks[b].m_coeff[0] <<= 4;
		      mb.m_blks[b].m_coeff[0] |= 0x8;
                      
		      IDCT_mmx(&mb.m_blks[b].m_coeff[0],
			       &mb.m_blks[b].m_coeff[0]);

		      IDCT_16bit8bit(&mb.m_blks[b].m_coeff[0],blkstart,lineskip);
		    }
#else
		  Pixel* op[8];
                  
		  op[0] = blkstart;
		  for (int i=1;i<8;i++)
		    op[i]=op[i-1]+lineskip;

		  IDCT_Int2b(&mb.m_blks[b].m_coeff[0], op, add_dct);
#endif
		}
	      else
		{
		  Pixel* p;
		  if (cbblks[b]) p=dp_cr; else p=dp_cb;
		  int lineskip;

		  if (mb.m_FieldDCT && d_ChromaFormat!=CHROMA_420)
		    {
		      p += blk_yoffs_field[b]*lineskip_chr+blk_xoffs_field[b];
		      lineskip = lineskip_chr*2;
		    }
		  else
		    {
		      p += blk_yoffs[b]*lineskip_chr+blk_xoffs[b];
		      lineskip = lineskip_chr;
		    }

		  // if (mb.m_blks[b].m_hasdata)
		  {
#if MMX_DCT
		    Pixel* blkstart = p;

		    if (add_dct)
		      {
			IDCT_mmx(&mb.m_blks[b].m_coeff[0],
				 &mb.m_blks[b].m_coeff[0]);

			IDCT_16bit8bit_add(&mb.m_blks[b].m_coeff[0],blkstart,lineskip);
		      }
		    else
		      {
			mb.m_blks[b].m_coeff[0] <<= 4;
			mb.m_blks[b].m_coeff[0] |= 0x8;
                      
			IDCT_mmx(&mb.m_blks[b].m_coeff[0],
				 &mb.m_blks[b].m_coeff[0]);

			IDCT_16bit8bit(&mb.m_blks[b].m_coeff[0],blkstart,lineskip);
		      }
#else
		    Pixel* op[8];

		    op[0] = p;
		    for (int i=1;i<8;i++)
		      op[i]=op[i-1]+lineskip;

		    IDCT_Int2b(&mb.m_blks[b].m_coeff[0], op, add_dct);
#endif
		  }
		}
	    }


	  if (d_pichdr.m_picture_coding_type == PICTYPE_D)
	    {
	      // D-picture requires final '1' bit (end of macroblock)
	  
	      int n = bs.GetBits(1);
	      if (n != 1)
		{
		  throw Excpt_StreamError(ErrSev_Error,
					  "Invalid MPEG stream, 'end of macroblock'-bit missing in D-picture.");
		}
	    }
	  lastmb = &mb;
	  last_mb_mode = mb_mode;
	}

      if (IsBPicture && mb_addr==d_MBWidth-1)
	{ FlushBSlice(mb_row); }
    }
  catch(Excpt_StreamError& err)
    {
      MessageDisplay::Show(err);
      MessageDisplay::Show(ErrSev_Note,"continuing anyway...");
    }
}


void VideoDecoder::motion_vector(struct MotionVector& mv,int s,bool dmv,
				 class FastBitBuf& bs)
{
  // horizontal component

  {
    const int motcode = GetMotionCode(bs);
    const int fcode   = d_pichdr.m_fcode[s][0];
    const int rsize   = fcode-1;
    int delta;
    int res;

    if (motcode!=0 && fcode !=1)
      {
        res = bs.GetBits(rsize);
        if (motcode>0)
          delta =   (( motcode-1)<<rsize) + res+1;
        else
          delta = -(((-motcode-1)<<rsize) + res+1);
      }
    else
      delta = motcode;
    
    mv.m_habs += delta;
    
    if      (mv.m_habs < (-16 << rsize))
      mv.m_habs += 32<<rsize;
    else if (mv.m_habs > ( 16 << rsize)-1)
      mv.m_habs -= 32<<rsize;

#ifndef NDEBUG
    if (d_options.Trace_MB)
      cout << "H (" << motcode << " " << res << ") -> " << delta << "  " << mv.m_habs << endl;
#endif
  }

  if (dmv)
    {
      int bits = bs.PeekBits(2);
      switch (bits)
        {
        case 0:
        case 1: mv.m_dmvector[0]= 0; bs.SkipBits(1); break;
        case 2: mv.m_dmvector[0]= 1; bs.SkipBits(2); break;
        case 3: mv.m_dmvector[0]=-1; bs.SkipBits(2); break;
        }
    }



  // vertical component

  {
    const int motcode = GetMotionCode(bs);
    const int fcode   = d_pichdr.m_fcode[s][1];
    const int rsize   = fcode-1;
    int delta;
    int res;

    if (motcode!=0 && fcode !=1)
      {
        res = bs.GetBits(rsize);
        if (motcode>0)
          delta =   (( motcode-1)<<rsize) + res+1;
        else
          delta = -(((-motcode-1)<<rsize) + res+1);
      }
    else
      delta = motcode;
    
    mv.m_vabs += delta;
    
    if      (mv.m_vabs < (-16 << rsize))
      mv.m_vabs += 32<<rsize;
    else if (mv.m_vabs > ( 16 << rsize)-1)
      mv.m_vabs -= 32<<rsize;

#ifndef NDEBUG
    if (d_options.Trace_MB)
      cout << "V (" << motcode << " " << res << ") -> " << delta << "  " << mv.m_vabs << endl;
#endif
  }

  if (dmv)
    {
      int bits = bs.PeekBits(2);
      switch (bits)
        {
        case 0:
        case 1: mv.m_dmvector[0]= 0; bs.SkipBits(1); break;
        case 2: mv.m_dmvector[0]= 1; bs.SkipBits(2); break;
        case 3: mv.m_dmvector[0]=-1; bs.SkipBits(2); break;
        }
    }


#ifndef NDEBUG
  if (d_options.Trace_MB)
        cout << "MV: " << mv.m_habs << " " << mv.m_vabs << endl;
#endif
}

#if 0
  {
  ImageParam_YUV param; d_outbuf[d_outbuf_len-1].m_image.GetParam(param);
  Pixel*const* p = ((Image_YUV<Pixel>*)&d_outbuf[d_outbuf_len-1].m_image)->AskFrameY();
  for (int y=0;y<param.height;y+=16)
    for (int x=0;x<param.width;x++)
      p[y][x]= (((y%160)==0) ? 255 : (((y%80)==0) ? 200 : 150));

  for (int y=0;y<param.height;y++)
    for (int x=0;x<param.width;x+=16)
      p[y][x]= (((x%160)==0) ? 255 : (((x%80)==0) ? 200 : 150));
  }
#endif

#if 0
  {
  ImageParam_YUV param; d_outbuf[d_outbuf_len-1].m_image.GetParam(param);
  Pixel*const* p = ((Image_YUV<Pixel>*)&d_outbuf[d_outbuf_len-1].m_image)->AskFrameU();
  for (int y=0;y<param.height/2;y++)
    for (int x=0;x<param.width/2;x++)
      p[y][x]=128;
  }
  {
  ImageParam_YUV param; d_outbuf[d_outbuf_len-1].m_image.GetParam(param);
  Pixel*const* p = ((Image_YUV<Pixel>*)&d_outbuf[d_outbuf_len-1].m_image)->AskFrameV();
  for (int y=0;y<param.height/2;y++)
    for (int x=0;x<param.width/2;x++)
      p[y][x]=128;
  }
#endif

