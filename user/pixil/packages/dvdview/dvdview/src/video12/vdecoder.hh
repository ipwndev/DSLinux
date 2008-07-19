/********************************************************************************
  video12/vdecoder.hh
    Video syntax decoder

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   07/set/2000 - Dirk Farin
     - Decoder is now tolerant against bitstream errors.
   05/Sep/2000 - Dirk Farin
     - Output reorganized again. B-pictures can now be sent slice-wise
       to the output device.
     - Major cleanup. Syntax decoding parts have been moved to vidsyntax.cc.
   16/Apr/2000 - Dirk Farin
     - Major architectur change. Output is now PUSH-oriented.
     - PictureData additional information data structure is filled
       out, when data is needed by postprocessors.
     - Bugfix: The two very last pictures of the stream are shown now.
     - Some cleanup, removing old obsolete code.
   22/Oct/1999 - Dirk Farin
     - Robust even when streams start in the middle of a sequence.
   30/Sep/1999 - Dirk Farin
     - integrated into CVS, major code cleanup and rewrite, quantization
       is now done already in the syntax decoder for efficiency reasons
       (coefficients that are 0 do not need to be quantized).
   17/May/1999 - Dirk Farin
     - Correct decoding of bitstreams with concealment MVs.
   31/Jan/1999 - Dirk Farin
     - Bugfix: MPEG-1 macroblock stuffing was not recognized.
   13/Jan/1999 - Dirk Farin
     - Bugfix: Skip USER-DATA startcode-packets. Streams with such
               data could not be read in earlier versions.
   11/Jan/1999 - Dirk Farin
     - Allocation of PictureData is now done using a pool of
       preallocated structures. This results in a huge reduction
       of system time.
   10/Jan/1999 - Dirk Farin
     - Bugfix: Motionvektor decoding was erroneous when fcode was
               not equal to 1.
   09/Jan/1999 - Dirk Farin
     - Bugfix: end of slice now detected, when remaining bits < 2 and
               not < 8, as there may be some very small MBs at the end
               of a slice (for example a MVFwd-only MB in P needs only
               4 bits). The condition is not redundant as the second
               condition (remaining bits!=0 to leave loop) does not work
               when remaining bits==0.
   07/Jan/1999 - Dirk Farin
     - Bugfix: motionvector activation flags were not set
     - Bugfix: motionvector predictors were not updated
   19/Dec/1998 -> 26/Dec/1998 - Dirk Farin
     - first implementation
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

#ifndef DVDVIEW_VIDEO12_VDECODER_HH
#define DVDVIEW_VIDEO12_VDECODER_HH

#include "video12/modules/mcomp_sgl_simple.hh"
#include "types.hh"
#include "video12/output.hh"
#include "video12/vidsyntax.hh"
#include "video12/constants.hh"
#include "error.hh"


struct VideoSyntaxTrace_Options
{
  VideoSyntaxTrace_Options();

  bool Trace_SeqH;
  bool Trace_GOPH;
  bool Trace_PicH;
  bool Trace_SlcH;
  bool Trace_MB;
  bool Trace_SliceData;
  bool Trace_DCTCoeff;
};

struct VideoFrameSkip_Options
{
  VideoFrameSkip_Options();

  bool DecodeI;
  bool DecodeP;
  bool DecodeB;
};

struct VideoDecoder_Options
  : public VideoSyntaxTrace_Options,
    public VideoFrameSkip_Options
{
};


class VideoDecoder
{
public:
   VideoDecoder(class PacketSource&);
  ~VideoDecoder();

  void SetOptions(const VideoDecoder_Options& opts) { d_options=opts; }
  void SetSink(DecodedPictureSink& s) { d_sink=&s; }

  void Reset() { d_SequenceHeaderRead=false; }
  bool DecodeAFrame(); /* Decode a picture. It is NOT guaranteed that the picture will
			  appear at the output of the video decoder. It may be buffered
			  for some time or get completely lost because of jitter
			  reduction or the like.
			  However, it is guaranteed that no more than one picture will
			  appear at the output.
		       */

private:
  VideoDecoder_Options d_options;

  class PacketSource*       d_source;
  class DecodedPictureSink* d_sink;

  ImageSpec_YUV specs;

  // Values out of last SequenceHeader.
  bool   d_IsMPEG2;            // (redundant)
  uint16 d_MBWidth,d_MBHeight; // (redundant) size of image in macroblock units
  uint2  d_ChromaFormat;       // (redundant)
  int    d_dctblks;         // number of dct blocks in one macroblock
  int    d_intra_cbp;       // CBP for intra blocks
  int    mb_chr_w,mb_chr_h; // Size of chroma components in a macroblock.

  // Values out of last PictureHeader.
  int    d_dc_pred;  // initial DC-prediction value

  enum { None,DataPartitioning,SNR,Spatial,Temporal } d_scalability_mode;


  // Syntax decoder

  PictureHeader  d_pichdr;  // current picture data  (macroblock array not used!)
  PictureData*   d_picdata;

  void PostSequenceHeader();
  void PostPictureHeader(const SystemTimingInformation& timing);

  void DecodeExtensions(int n);
  void DecodeSlice    (class FastBitBuf&);
  void FlushBSlice(int MBRow); /* Send physical MB-row 0 to output at the specified position
				  and reset decoding pointers to the first MB in row 0. */
  void motion_vector  (struct MotionVector&,int s,bool dmv,class FastBitBuf&);


  // pushback buffer

  class SysPacket_Packet* GetNextPacket();
  void PushbackPacket(class SysPacket_Packet*);

  class SysPacket_Packet* d_next_packet;

  QuantMatrices d_quant_zz;  // current quantization matrices in zig-zag-order
  QuantMatrices d_quant_bs;  // current quantization matrices in current bitstream scanning order


  inline void AdvancePtrsMB();
  inline void AdvancePtrsMBRow();


  // -------------- Video Decoder ------------------

  SequenceHeader d_seqdata;
  bool   d_SequenceHeaderRead; // If d_seqdata contains valid data.

  bool d_BFrameAvailable;   /* True iff a complete B-picture (1 frame picture or both fields) has
			       been decoded to d_curr and is ready for display. */
  bool d_FirstFieldInFrame; /* True iff the next picture to be decoded is either the first field
			       or a frame picture. */
  bool d_next_IsEmpty;      /* True iff d_next does not contain an image. */
  bool d_skip_this_frame;

  DecodedImageData* d_last; // The picture that already has been displayed.
  DecodedImageData* d_curr; // The currently decoded B-picture.
  DecodedImageData* d_next; // The I/P-picture to be displayed after the pictures above.
  Pixel*const* ptr_y;   // current bitmaps while picture decoding
  Pixel*const* ptr_cb;  // current bitmaps while picture decoding
  Pixel*const* ptr_cr;  // current bitmaps while picture decoding

  int bytesperline_lum;
  int bytesperline_chr;

  int lineskip_lum;
  int lineskip_chr;

  int d_field_offset; // 0 for topfield, frames / 1 for bottom field

  // --- Pointers to current image data used in the calculations
  // current decoding destination
  Pixel* sp_curr_y;
  Pixel* sp_curr_cr;
  Pixel* sp_curr_cb;
  // forward reference pictures
  Pixel* sp_last_y [2]; // 0 - top field / 1 - bottom field
  Pixel* sp_last_cr[2];
  Pixel* sp_last_cb[2];
  // backward reference pictures
  Pixel* sp_next_y [2];
  Pixel* sp_next_cr[2];
  Pixel* sp_next_cb[2];

  // Pointers to current decoding macroblock position
  Pixel* dp_y;
  Pixel* dp_cr;
  Pixel* dp_cb;

  enum TempRef { LAST,NEXT };
  inline void SetSPOffsets(TempRef sp_tempref,int n,TempRef predref,bool field,bool topfield=true);

  void SetReferencePtrsFrm(struct PixPtrs_const&,const struct MotionVector&);
  void SetReferencePtrsFld(struct PixPtrs_const&,const struct MotionVector&);
  void SetHalfPelFlags1(struct MotionCompensation_SglMB::MCData&,const struct MotionVector&);
  void SetHalfPelFlags2(struct MotionCompensation_SglMB::MCData&,
                        const struct MotionVector& last,
                        const struct MotionVector& next);
  void MC_Frame_FrameBased(const Macroblock& mb,uint16 mb_mode);
  void MC_Frame_FieldBased(const Macroblock& mb,uint16 mb_mode);
  void MC_Field_FieldBased(const Macroblock& mb,uint16 mb_mode);
  void MC_Field_16x8(const Macroblock& mb,uint16 mb_mode);

  // Decoder

  int  d_scan[2][64]; // scan pattern 0 - zigzag / 1 - alternate
  class MotionCompensation_SglMB* d_motcomp;
};

//inline int DequantizeIntra(int value,int qscale,int matrix);
inline int DequantizeInter(int value,int qscale,int matrix);
//inline void Saturate(int& value);

class Excpt_StreamError : public Excpt_Base
{
public:
  Excpt_StreamError(ErrorSeverity sev,const char* text) : Excpt_Base(sev,text) { }
};

class Excpt_Huffman : public Excpt_StreamError
{
public:
  Excpt_Huffman(ErrorSeverity sev,const char* text) : Excpt_StreamError(sev,text) { }
};

#endif
