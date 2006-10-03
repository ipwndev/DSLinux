/********************************************************************************
  video12/vlc.hh

  purpose:
   Utility functions to decode Variable Length Codes

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   28/Dec/1998 - Dirk Farin
     - Added support for table B15 (intra_vlc_format==1)
   26/Dec/1998 - Dirk Farin
     - Added GetMotionCode()
   15/Nov/1998 - Dirk Farin
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

#ifndef DVDVIEW_VIDEO12_VLC_HH
#define DVDVIEW_VIDEO12_VLC_HH

#include "types.hh"

int GetMBAddrIncr (class FastBitBuf&);
int GetCBP        (class FastBitBuf&);
int GetDClumSize  (class FastBitBuf&);
int GetDCchromSize(class FastBitBuf&);
int GetMotionCode (class FastBitBuf&);

bool GetRunLen(FastBitBuf& bs,int& run,int& value ,bool B15,bool first,bool MPEG1);

const uint16 MBMODE_QUANT   = 0x10;
const uint16 MBMODE_MVFWD   = 0x08;
const uint16 MBMODE_MVBKW   = 0x04;
const uint16 MBMODE_PATTERN = 0x02;
const uint16 MBMODE_INTRA   = 0x01;

// extern int (* GetMBMode[5])(class FastBitBuf&);  // use picture_coding_type to index into array

#endif
