/********************************************************************************
  video12/constants.hh
    MPEG stream constants.

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   04/Oct/1999 - Dirk Farin
     - integrated code to CVS
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

#ifndef DVDVIEW_VIDEO12_CONSTANTS_HH
#define DVDVIEW_VIDEO12_CONSTANTS_HH

#include "types.hh"

// -------------------------------- Startcodes -------------------------------------

// Video Startcodes

#define STARTCODE_PICTURE_START   0x00000100
#define STARTCODE_USER_DATA       0x000001B2
#define STARTCODE_SEQUENCE_HEADER 0x000001B3
#define STARTCODE_SEQUENCE_ERROR  0x000001B4
#define STARTCODE_EXTENSION_START 0x000001B5
#define STARTCODE_SEQUENCE_END    0x000001B7
#define STARTCODE_GROUP_START     0x000001B8

// System Startcodes

#define STARTCODE_ISO11172_END    0x000001B9
#define STARTCODE_PACK_START      0x000001BA
#define STARTCODE_SYSTEM_HEADER_START 0x000001BB

// Packet Startcodes

#define STARTCODE_VIDEO0     0x000001E0
#define STARTCODE_VIDEO1     0x000001E1
#define STARTCODE_VIDEO2     0x000001E2
#define STARTCODE_AUDIO0     0x000001C0
#define STARTCODE_AUDIO1     0x000001C1
#define STARTCODE_AUDIO2     0x000001C2

inline bool IsMPEG_SLICE_START (uint8 code) { return code>=0x01 && code<=0xAF; }
inline bool IsMPEG_VIDEO_STREAM(uint8 code) { return code>=0xE0 && code<=0xEF; }
inline bool IsMPEG_AUDIO_STREAM(uint8 code) { return code>=0xC0 && code<=0xDF; }
inline bool IsMPEG_PACKETCODE  (uint8 code) { return code>=0xC0 && code<=0xEF; }


// ---------- extension_start_code_identifier -----------

const uint4 EXTID_SequenceExtension         = 0x1;
const uint4 EXTID_SequenceDisplayExtension  = 0x2;
const uint4 EXTID_QuantMatrixExtension      = 0x3;
const uint4 EXTID_CopyrightExtension        = 0x4;
const uint4 EXTID_SequenceScalableExtension = 0x5;
const uint4 EXTID_PictureDisplayExtension   = 0x7;
const uint4 EXTID_PictureCodingExtension    = 0x8;
const uint4 EXTID_PictureSpatialScalableExtension  = 0x9;
const uint4 EXTID_PictureTemporalScalableExtension = 0xA;


// ---------- aspect_ratio_information ------------

const uint4 SAR_Square = 0x01;     // Sample Aspect Ratio = 1.0  (square pixels)
const uint4 DAR_3_4    = 0x02;     // Display Aspect Ratio = 3/4
const uint4 DAR_9_16   = 0x03;     // Display Aspect Ratio = 9/16
const uint4 DAR_2_2_21 = 0x04;     // Display Aspect Ratio = 2/2.21


// ---------- frame_rate_code ------------

const uint4 FRAMERATE_23_976 = 0x01;  // 23.976 fps
const uint4 FRAMERATE_24     = 0x02;  // 24     fps
const uint4 FRAMERATE_25     = 0x03;  // 25     fps
const uint4 FRAMERATE_29_97  = 0x04;  // 29.97  fps
const uint4 FRAMERATE_30     = 0x05;  // 30     fps
const uint4 FRAMERATE_50     = 0x06;  // 50     fps
const uint4 FRAMERATE_59_94  = 0x07;  // 59.94  fps
const uint4 FRAMERATE_60     = 0x08;  // 60     fps


// --------- profile_identification ----------

const uint3 PROFILE_Simple      = 0x5;
const uint3 PROFILE_Main        = 0x4;
const uint3 PROFILE_SNRScalable = 0x3;
const uint3 PROFILE_SpatiallyScalable = 0x2;
const uint3 PROFILE_High        = 0x1;


// --------- level_identification ------------

const uint4 LEVEL_Low      = 0xA;
const uint4 LEVEL_Main     = 0x8;
const uint4 LEVEL_High1440 = 0x6;
const uint4 LEVEL_High     = 0x4;


// --------- chroma_format ----------

const uint2 CHROMA_420 = 1;
const uint2 CHROMA_422 = 2;
const uint2 CHROMA_444 = 3;


// --------- picture_coding_type --------

const uint3 PICTYPE_I = 1;
const uint3 PICTYPE_P = 2;
const uint3 PICTYPE_B = 3;
const uint3 PICTYPE_D = 4;


// -------- picture_structure ---------

const uint2 PICSTRUCT_TopField     = 1;
const uint2 PICSTRUCT_BottomField  = 2;
const uint2 PICSTRUCT_FramePicture = 3;

#endif
