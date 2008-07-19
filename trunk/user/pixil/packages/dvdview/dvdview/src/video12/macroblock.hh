/*********************************************************************
  video12/macroblock.hh
    Macroblock syntax.

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   30/Sep/1999 - Dirk Farin
     - Integrated code into CVS.
   05/May/99 - Dirk Farin
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

#ifndef DVDVIEW_VIDEO12_MACROBLOCK_HH
#define DVDVIEW_VIDEO12_MACROBLOCK_HH

#include "types.hh"


struct DCTBlock
{
  short m_coeff[64]; // in custom order, implementation dependent
  bool  m_hasdata; // true iff m_coeff[][]!=0 exists
  int   m_nCoeffs;
};

struct MotionVector
{
  int   m_habs, m_vabs;

  uint1 m_vertical_field_select;  // 0 ist TopField.
   int2 m_dmvector[2];
};

struct Macroblock
{
  uint7  m_quantiser_scale_code; /* 1 bis 31  */

  bool   m_IsIntra;
  DCTBlock m_blks[12];

  // Fuer Frame-Pictures mit 'frame_pred_frame_dct==0' und vorhandenen DCT-Koeff.
  bool m_FieldDCT;

  enum FrameMotionType { FRMT_FieldBased=1, FRMT_FrameBased=2, FRMT_DualPrime=3 };
  enum FieldMotionType { FIMT_FieldBased=1, FIMT_16x8MC=2,     FIMT_DualPrime=3 };
  union {
    // MotionComp.Type fuer Pictures mit 'picture_structure==frame' und
    // 'frame_pred_frame_dct==0'. Ist 'frame_pred_frame_dct==1', so wird FRMT_FrameBased angenommen,
    // m_frame_motion_type aber nicht in den Bitstream codiert.
    enum FrameMotionType m_frame_motion_type;
    
    // MotionComp.Type fuer Pictures mit 'picture_structure!=frame'.
    enum FieldMotionType m_field_motion_type;
  };
  uint2 m_motion_vector_count;
  enum { MVFormat_Field, MVFormat_Frame } m_mv_format;
  bool  m_dmv;

  inline void SetFrameMotionType(FrameMotionType,int temporal_weight_class=0);
  inline void SetFieldMotionType(FieldMotionType);

  MotionVector m_vector[4 /*r*/][2 /*s*/]; // t in der Struktur
  bool m_HasMotionForward;  // <- wird auch als Consealment-Vektor-Flag ( vector[0][0] ) benutzt
  bool m_HasMotionBackward;

#define m_forward1  m_vector[0][0]
#define m_forward2  m_vector[1][0]
#define m_backward1 m_vector[0][1]
#define m_backward2 m_vector[1][1]
};

#include "error.hh"

inline void Macroblock::SetFrameMotionType(FrameMotionType t,int temporal_weight_class)
{
#if 0
  m_motion_vector_count = 1;
  m_mv_format = MVFormat_Frame;

#else

  m_frame_motion_type = t;
  m_mv_format = MVFormat_Frame;

  if (t==FRMT_FrameBased)
    {
      m_motion_vector_count = 1;
      m_dmv = 0;
    }
  else if (t==FRMT_FieldBased)
    {
      if (temporal_weight_class <= 1)
        m_motion_vector_count = 2;
      else
        m_motion_vector_count = 1;
      m_dmv = 0;
    }
  else if (t==FRMT_DualPrime)
    {
      m_motion_vector_count = 1;
      m_dmv = 1;
    }
  else
    {
      // TODO Error(ErrSev_Warning,"Invalid stream: undefined \"frame motion type\". Using \"frame-based\"\n");
      m_motion_vector_count = 1;
      m_dmv = 0;
    }
#endif
}

#include <iostream.h>

inline void Macroblock::SetFieldMotionType(FieldMotionType t)
{
#if 0
  m_motion_vector_count = 1;
#else
  m_field_motion_type = t;
  m_mv_format = MVFormat_Field;

  switch (t)
    {
    case FIMT_FieldBased:
      m_motion_vector_count = 1;
      m_dmv = 0;
      break;
    case FIMT_16x8MC:
      //cout << "16x8\n";
      m_motion_vector_count = 2;
      m_dmv = 0;
      break;
    case FIMT_DualPrime:
      m_motion_vector_count = 1;
      m_dmv = 1;
      break;
    default:
      Assert(0);
      break;
    }
#endif
}

#endif
