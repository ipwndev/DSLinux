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

/* NOTE: do not compile this file. It is included inline into vdecoder.cc. */

#include "libvideogfx/utility/bitstream/fastbitbuf.hh"
#include "video12/vlc.hh"
#include "types.hh"
#include "error.hh"



#define maybeinline inline

struct DirectVLCTableEntry
{
  short bits; int nBits;
};

struct DirectVLC
{
  int    value;
  uint16 bits;
};




/************************************* MB-INCR ********************************/



static DirectVLCTableEntry vlc_mbaddrinc_tab[] =
{
  {0x08,11}, // ESCAPE-CODE

  /* table taken from MSSG-encoder */
  {0x01,1},  {0x03,3},  {0x02,3},  {0x03,4},
  {0x02,4},  {0x03,5},  {0x02,5},  {0x07,7},
  {0x06,7},  {0x0b,8},  {0x0a,8},  {0x09,8},
  {0x08,8},  {0x07,8},  {0x06,8},  {0x17,10},
  {0x16,10}, {0x15,10}, {0x14,10}, {0x13,10},
   {0x12,10}, {0x23,11}, {0x22,11}, {0x21,11},
  {0x20,11}, {0x1f,11}, {0x1e,11}, {0x1d,11},
  {0x1c,11}, {0x1b,11}, {0x1a,11}, {0x19,11},
  {0x18,11}, {0,-1}
};


#include <iostream.h>
#include <iomanip.h>

static DirectVLC vlc_mbaddr[2048 /*2^11*/];

struct MBAtab
{
  uint8 mba,len;
};

static MBAtab MBA_5 [] = {
                    {7, 5}, {6, 5}, {5, 4}, {5, 4}, {4, 4}, {4, 4},
    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {2, 3}, {2, 3}, {2, 3}, {2, 3},
    {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1},
    {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}, {1, 1}
};

static MBAtab MBA_11 [] = {
    {33, 11}, {32, 11}, {31, 11}, {30, 11},
    {29, 11}, {28, 11}, {27, 11}, {26, 11},
    {25, 11}, {24, 11}, {23, 11}, {22, 11},
    {21, 10}, {21, 10}, {20, 10}, {20, 10},
    {19, 10}, {19, 10}, {18, 10}, {18, 10},
    {17, 10}, {17, 10}, {16, 10}, {16, 10},
    {15,  8}, {15,  8}, {15,  8}, {15,  8},
    {15,  8}, {15,  8}, {15,  8}, {15,  8},
    {14,  8}, {14,  8}, {14,  8}, {14,  8},
    {14,  8}, {14,  8}, {14,  8}, {14,  8},
    {13,  8}, {13,  8}, {13,  8}, {13,  8},
    {13,  8}, {13,  8}, {13,  8}, {13,  8},
    {12,  8}, {12,  8}, {12,  8}, {12,  8},
    {12,  8}, {12,  8}, {12,  8}, {12,  8},
    {11,  8}, {11,  8}, {11,  8}, {11,  8},
    {11,  8}, {11,  8}, {11,  8}, {11,  8},
    {10,  8}, {10,  8}, {10,  8}, {10,  8},
    {10,  8}, {10,  8}, {10,  8}, {10,  8},
    { 9,  7}, { 9,  7}, { 9,  7}, { 9,  7},
    { 9,  7}, { 9,  7}, { 9,  7}, { 9,  7},
    { 9,  7}, { 9,  7}, { 9,  7}, { 9,  7},
    { 9,  7}, { 9,  7}, { 9,  7}, { 9,  7},
    { 8,  7}, { 8,  7}, { 8,  7}, { 8,  7},
    { 8,  7}, { 8,  7}, { 8,  7}, { 8,  7},
    { 8,  7}, { 8,  7}, { 8,  7}, { 8,  7},
    { 8,  7}, { 8,  7}, { 8,  7}, { 8,  7}
};


maybeinline int GetMBAddrIncr(class FastBitBuf& bs)
{
#if 1
  MBAtab * tab;
  int mba;

  mba = 0;

  bs.Fill16Bits();

  while (1) {
    if (bs.Peek16BitsMSB() >= 0x10000000) {
      tab = MBA_5 - 2 + bs.PeekBitsFast(5);
      bs.SkipBitsFast(tab->len);
      return mba + tab->mba;
    } else if (bs.Peek16BitsMSB() >= 0x03000000) {
      tab = MBA_11 - 24 + bs.PeekBitsFast(11);
      bs.SkipBitsFast(tab->len);
      return mba + tab->mba;
    } else
      {
	switch (bs.PeekBits(11)) {
	case 8:         // macroblock_escape
	  mba += 33;
	  // no break here on purpose
	case 15:        // macroblock_stuffing (MPEG1 only)
	  bs.SkipBitsFast(11);
	  bs.Fill16Bits();
	  break;
	default:        // end of slice, or error
	  cout << "GetMBAddrIncr-ERROR\n";
	  throw Excpt_Huffman(ErrSev_Warning,"Invalid MPEG stream, not existing MB-addr-incr. read. (1)");
	  return 0;
	}
      }
  }
#else
  uint16 code = bs.PeekBits(11);
  int val = vlc_mbaddr[code].value;

  bs.SkipBits(vlc_mbaddr[code].bits);

  if (val<0)
    throw Excpt_Huffman(ErrSev_Warning,"Invalid MPEG stream, not existing MB-addr-incr. read. (1)");

  if (val!=0) return val;

  int val1=33;
 loop:
  code = bs.PeekBits(11);
  val = vlc_mbaddr[code].value;
  bs.SkipBits(vlc_mbaddr[code].bits);

  if (val<0)
    throw Excpt_Huffman(ErrSev_Warning,"Invalid MPEG stream, not existing MB-addr-incr. read. (2)");

  if (val!=0) return val+val1;
  else        { val1 += 33; goto loop; }
#endif
}





/************************************* MB-MODE ********************************/





static DirectVLCTableEntry vlc_mbmode_P_tab[] =
{
  {0,0}, {3,5}, {1,2}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {1,3}, {0,0}, {1,1}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {1,6}, {1,5}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {2,5}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,-1}
};

static DirectVLCTableEntry vlc_mbmode_B_tab[] =
{
  {0,0}, {3,5}, {0,0}, {0,0}, {2,3}, {0,0}, {3,3}, {0,0},
  {2,4}, {0,0}, {3,4}, {0,0}, {2,2}, {0,0}, {3,2}, {0,0},
  {0,0}, {1,6}, {0,0}, {0,0}, {0,0}, {0,0}, {2,6}, {0,0},
  {0,0}, {0,0}, {3,6}, {0,0}, {0,0}, {0,0}, {2,5}, {0,0}, {0,-1}
};


static DirectVLC vlc_mbmode_P[64 /*2^6*/];
static DirectVLC vlc_mbmode_B[64 /*2^6*/];


static int GetMBMode_I(class FastBitBuf& bs)
{
  int code = bs.PeekBits(2);
  if (code & 2)
    {
      bs.SkipBitsFast(1);
      return 1;
    }
  else
    {
      if (code!=1)
        throw Excpt_Huffman(ErrSev_Warning,"Invalid MPEG stream, not existing MB-class (I-picture) read.");

      bs.SkipBitsFast(2);
      return 17;
    }
}

static int GetMBMode_P(class FastBitBuf& bs)
{
  uint16 code = bs.PeekBits(6);
  bs.SkipBits(vlc_mbmode_P[code].bits);

  if (vlc_mbmode_P[code].value==0)
    throw Excpt_Huffman(ErrSev_Warning,"Invalid MPEG stream, not existing MB-class (P-picture) read.");

  return vlc_mbmode_P[code].value;
}

static int GetMBMode_B(class FastBitBuf& bs)
{
  uint16 code = bs.PeekBits(6);
  bs.SkipBitsFast(vlc_mbmode_B[code].bits);

  if (vlc_mbmode_B[code].value==0)
    {
      //cerr << "CODEVAL: $" << hex << code << " 6 bits " << endl;
      throw Excpt_Huffman(ErrSev_Warning,"Invalid MPEG stream, not existing MB-class (B-picture) read.");
    }

  return vlc_mbmode_B[code].value;
}

static int GetMBMode_D(class FastBitBuf& bs)
{
  int c = bs.GetBits(1);
  if (c != 1)
    {
      throw Excpt_Huffman(ErrSev_Error,"Invalid MPEG stream, invalid MB-mode in D-picture read.");
    }

  return MBMODE_INTRA;
}

static int (* GetMBMode[5])(class FastBitBuf&) =
{
  NULL,
  GetMBMode_I,
  GetMBMode_P,
  GetMBMode_B,
  GetMBMode_D
};



/************************************* MotionCode ********************************/

static DirectVLCTableEntry vlc_mcode_tab[]=
{
  /* table taken from MSSG-encoder */

  {0x01,1},  {0x01,2},  {0x01,3},  {0x01,4},
  {0x03,6},  {0x05,7},  {0x04,7},  {0x03,7},
  {0x0b,9},  {0x0a,9},  {0x09,9},  {0x11,10},
  {0x10,10}, {0x0f,10}, {0x0e,10}, {0x0d,10},
  {0x0c,10}, {0,-1}
};

static DirectVLC vlc_mc[1024 /*2^10*/];

maybeinline int GetMotionCode(class FastBitBuf& bs)
{
  uint16 code = bs.PeekBits(10);
  bs.SkipBitsFast(vlc_mc[code].bits);

  if (vlc_mc[code].value==0)
    return 0;

  if (bs.GetBits(1)==0)
    return  vlc_mc[code].value;
  else
    return -vlc_mc[code].value;
}


/************************************* CBP ********************************/

static DirectVLCTableEntry vlc_cbp_tab[]=
{
  /* table taken from MSSG-encoder */

  {0x01,9}, {0x0b,5}, {0x09,5}, {0x0d,6}, 
  {0x0d,4}, {0x17,7}, {0x13,7}, {0x1f,8}, 
  {0x0c,4}, {0x16,7}, {0x12,7}, {0x1e,8}, 
  {0x13,5}, {0x1b,8}, {0x17,8}, {0x13,8}, 
  {0x0b,4}, {0x15,7}, {0x11,7}, {0x1d,8}, 
  {0x11,5}, {0x19,8}, {0x15,8}, {0x11,8}, 
  {0x0f,6}, {0x0f,8}, {0x0d,8}, {0x03,9}, 
  {0x0f,5}, {0x0b,8}, {0x07,8}, {0x07,9}, 
  {0x0a,4}, {0x14,7}, {0x10,7}, {0x1c,8}, 
  {0x0e,6}, {0x0e,8}, {0x0c,8}, {0x02,9}, 
  {0x10,5}, {0x18,8}, {0x14,8}, {0x10,8}, 
  {0x0e,5}, {0x0a,8}, {0x06,8}, {0x06,9}, 
  {0x12,5}, {0x1a,8}, {0x16,8}, {0x12,8}, 
  {0x0d,5}, {0x09,8}, {0x05,8}, {0x05,9}, 
  {0x0c,5}, {0x08,8}, {0x04,8}, {0x04,9},
  {0x07,3}, {0x0a,5}, {0x08,5}, {0x0c,6}, {0,-1}
};

static DirectVLC vlc_cbp[512 /*2^9*/];


maybeinline int GetCBP(class FastBitBuf& bs)
{
  uint16 code = bs.PeekBits(9);
  bs.SkipBitsFast(vlc_cbp[code].bits);
  return vlc_cbp[code].value;
}


/************************************* Initialization ********************************/

static DirectVLCTableEntry vlc_DClum_tab[]=
{
  /* table taken from MSSG-encoder */
  {0x0004,3}, {0x0000,2}, {0x0001,2}, {0x0005,3}, {0x0006,3}, {0x000e,4},
  {0x001e,5}, {0x003e,6}, {0x007e,7}, {0x00fe,8}, {0x01fe,9}, {0x01ff,9}, {0,-1}
};

static DirectVLCTableEntry vlc_DCchrom_tab[]=
{
  /* table taken from MSSG-encoder */
  {0x0000,2}, {0x0001,2}, {0x0002,2}, {0x0006,3}, {0x000e,4}, {0x001e,5},
  {0x003e,6}, {0x007e,7}, {0x00fe,8}, {0x01fe,9}, {0x03fe,10},{0x03ff,10}, {0,-1}
};

static DirectVLC vlc_DClum  [1<< 9];
static DirectVLC vlc_DCchrom[1<<10];

maybeinline int GetDClumSize(class FastBitBuf& bs)
{
  uint16 code = bs.PeekBits(9);
  bs.SkipBitsFast(vlc_DClum[code].bits);
  return vlc_DClum[code].value;
}

maybeinline int GetDCchromSize(class FastBitBuf& bs)
{
  uint16 code = bs.PeekBits(10);
  bs.SkipBitsFast(vlc_DCchrom[code].bits);
  return vlc_DCchrom[code].value;
}



typedef struct {
    uint8 size;
    uint8 len;
} DCtab;

static DCtab DC_lum_5 [] = {
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
    {4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5}
};

static DCtab DC_long [] = {
    {6, 5}, {6, 5}, {6, 5}, {6, 5}, {6, 5}, {6, 5}, { 6, 5}, { 6, 5},
    {6, 5}, {6, 5}, {6, 5}, {6, 5}, {6, 5}, {6, 5}, { 6, 5}, { 6, 5},
    {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, { 7, 6}, { 7, 6},
    {8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10, 9}, {11, 9}
};


static DCtab DC_lum_7 [] = {
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},

    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},

    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},
    {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3}, {0, 3},

    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},

    {4, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3},
    {4, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3}, {4, 3},
    {5, 4}, {5, 4}, {5, 4}, {5, 4}, {5, 4}, {5, 4}, {5, 4}, {5, 4},
    {6, 5}, {6, 5}, {6, 5}, {6, 5}, {7, 6}, {7, 6}, {8, 7}
};


#if 0
static inline int get_luma_dc_dct_diff (FastBitBuf& bs)
{
  bs.Fill16Bits();

#define bit_buf bs.d_buffer
#define bits bs.d_freebits
  DCtab * tab;
  int size;
  int dc_diff;

  if (bit_buf < 0xf8000000) {
    /* Max. codelength = 5 + max. size = 6  <= 16 , no refill needed. */

    tab = DC_lum_5 + UBITS (bit_buf, 5);
    size = tab->size;
    if (size) {
      bits += tab->len + size;
      bit_buf <<= tab->len;
      dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
      bit_buf <<= size;
      return dc_diff;
    } else {
      DUMPBITS (bit_buf, bits, 3);
      return 0;
    }
  } else {
    tab = DC_long - 0x1e0 + UBITS (bit_buf, 9);
    size = tab->size;
    if (size>=9)
      {
	DUMPBITS(bit_buf,bits,tab->len);
	bs.Fill16Bits();
	dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
	DUMPBITS(bit_buf,bits,size);
      }
    else
      {
	bits += tab->len+size;
	bit_buf <<= tab->len;
	dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
	bit_buf <<= size;
      }
    return dc_diff;
  }
#undef bit_buf
#undef bits
}
#endif

#if 1
static inline int get_luma_dc_dct_diff (FastBitBuf& bs)
{
  bs.Fill16Bits();

#define bit_buf bs.d_buffer
#define bits bs.d_freebits
  DCtab * tab;
  int size;
  int dc_diff;

  if (bit_buf < 0xfe000000) {
    /* Max. codelength = 7 + max. size = 8  <= 15 , no refill needed. */

    tab = DC_lum_7 + UBITS (bit_buf, 7);
    size = tab->size;
    if (size) {
      bits += tab->len + size;
      bit_buf <<= tab->len;
      dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
      bit_buf <<= size;
      return dc_diff;
    } else {
      DUMPBITS (bit_buf, bits, 3);
      return 0;
    }
  } else {
    tab = DC_long - 0x1e0 + UBITS (bit_buf, 9);
    size = tab->size;
    DUMPBITS(bit_buf,bits,tab->len);
    bs.Fill16Bits();
    dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
    DUMPBITS(bit_buf,bits,size);
    return dc_diff;
  }
#undef bit_buf
#undef bits
}
#endif

#if 0
static DCtab DC_small [] = {
    {1, 2}, {1, 2}, {2, 2}, {2, 2}, {0, 3}, {3, 3}
};

static inline int get_luma_dc_dct_diff(FastBitBuf& bs)
{
  bs.Fill16Bits();

#define bit_buf bs.d_buffer
#define bits bs.d_freebits
  DCtab * tab;
  int size;
  int dc_diff;

  int cnt;

  __asm__
    (
     "bsrl %1,%0\n\t"
     : "=r" (cnt) : "m" (~bit_buf)
     );

  if (cnt>=30)
    {
      tab = DC_small + UBITS(bit_buf, 3);
      size = tab->size;
      if (size) {
	bits += tab->len + size;
	bit_buf <<= tab->len;
	dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
	bit_buf <<= size;
	return dc_diff;
      } else {
	DUMPBITS (bit_buf, bits, 3);
	return 0;
      }
    } else if (cnt>=25)	{
      int len  = 32-cnt;
      size = len+1;

      bits += len+size;
      bit_buf <<= len;
      dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
      bit_buf <<= size;
      return dc_diff;
    } else {
      int len  = 32-cnt;
      size = len+1;
      DUMPBITS(bit_buf,bits,len);
      bs.Fill16Bits();
      dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
      DUMPBITS(bit_buf,bits,size);
      return dc_diff;
    }
#undef bit_buf
#undef bits
}
#endif


// -------------- chroma

static DCtab DC_chrom_5 [] = {
    {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2},
    {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
    {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
    {3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5}
};


static inline int get_chroma_dc_dct_diff (FastBitBuf& bs)
{
  bs.Fill16Bits();

#define bit_buf bs.d_buffer
#define bits bs.d_freebits
  DCtab * tab;
  int size;
  int dc_diff;

  if (bit_buf < 0xf8000000) {
    /* Max. codelength = 5 + max. size = 5  <= 16 , no refill needed. */

    tab = DC_chrom_5 + UBITS (bit_buf, 5);
    size = tab->size;
    if (size) {
      bits += tab->len + size;
      bit_buf <<= tab->len;
      dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
      bit_buf <<= size;
      return dc_diff;
    } else {
      DUMPBITS (bit_buf, bits, 2);
      return 0;
    }
  } else {
    tab = DC_long - 0x3e0 + UBITS (bit_buf, 10);
    size = tab->size;
    int len = tab->len+1;
    if (size>=8)
      {
	DUMPBITS(bit_buf,bits,len);
	bs.Fill16Bits();
	dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
	DUMPBITS(bit_buf,bits,size);
      }
    else
      {
	bits += len+size;
	bit_buf <<= len;
	dc_diff = UBITS (bit_buf, size) - UBITS (SBITS (~bit_buf, 1), size);
	bit_buf <<= size;
      }
    return dc_diff;
  }
#undef bit_buf
#undef bits
}



/************************************* Run/Level pairs ********************************/

struct DCTtab {
  uint8 run, level, len;
};


/* Table B-14, DCT coefficients table zero,
 * codes 0100 ... 1xxx (used for first (DC) coefficient)
 */
static DCTtab DCTtabfirst[12] =
{
  {0,2,4}, {2,1,4}, {1,1,3}, {1,1,3},
  {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1},
  {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}
};

/* Table B-14, DCT coefficients table zero,
 * codes 0100 ... 1xxx (used for all other coefficients)
 */
static DCTtab DCTtabnext[12] =
{
  {0,2,4},  {2,1,4},  {1,1,3},  {1,1,3},
  {64,0,2}, {64,0,2}, {64,0,2}, {64,0,2}, /* EOB */
  {0,1,2},  {0,1,2},  {0,1,2},  {0,1,2}
};

/* Table B-14, DCT coefficients table zero,
 * codes 000001xx ... 00111xxx
 */
static DCTtab DCTtab0[60] =
{
  {65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
  {2,2,7}, {2,2,7}, {9,1,7}, {9,1,7},
  {0,4,7}, {0,4,7}, {8,1,7}, {8,1,7},
  {7,1,6}, {7,1,6}, {7,1,6}, {7,1,6},
  {6,1,6}, {6,1,6}, {6,1,6}, {6,1,6},
  {1,2,6}, {1,2,6}, {1,2,6}, {1,2,6},
  {5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
  {13,1,8}, {0,6,8}, {12,1,8}, {11,1,8},
  {3,2,8}, {1,3,8}, {0,5,8}, {10,1,8},
  {0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
  {0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
  {4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
  {4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5}
};

/* Table B-15, DCT coefficients table one,
 * codes 000001xx ... 11111111
*/
static DCTtab DCTtab0a[252] =
{
  {65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
  {7,1,7}, {7,1,7}, {8,1,7}, {8,1,7},
  {6,1,7}, {6,1,7}, {2,2,7}, {2,2,7},
  {0,7,6}, {0,7,6}, {0,7,6}, {0,7,6},
  {0,6,6}, {0,6,6}, {0,6,6}, {0,6,6},
  {4,1,6}, {4,1,6}, {4,1,6}, {4,1,6},
  {5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
  {1,5,8}, {11,1,8}, {0,11,8}, {0,10,8},
  {13,1,8}, {12,1,8}, {3,2,8}, {1,4,8},
  {2,1,5}, {2,1,5}, {2,1,5}, {2,1,5},
  {2,1,5}, {2,1,5}, {2,1,5}, {2,1,5},
  {1,2,5}, {1,2,5}, {1,2,5}, {1,2,5},
  {1,2,5}, {1,2,5}, {1,2,5}, {1,2,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4}, /* EOB */
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,4,5}, {0,4,5}, {0,4,5}, {0,4,5},
  {0,4,5}, {0,4,5}, {0,4,5}, {0,4,5},
  {0,5,5}, {0,5,5}, {0,5,5}, {0,5,5},
  {0,5,5}, {0,5,5}, {0,5,5}, {0,5,5},
  {9,1,7}, {9,1,7}, {1,3,7}, {1,3,7},
  {10,1,7}, {10,1,7}, {0,8,7}, {0,8,7},
  {0,9,7}, {0,9,7}, {0,12,8}, {0,13,8},
  {2,3,8}, {4,2,8}, {0,14,8}, {0,15,8}
};

/* Table B-14, DCT coefficients table zero,
 * codes 0000001000 ... 0000001111
 */
static DCTtab DCTtab1[8] =
{
  {16,1,10}, {5,2,10}, {0,7,10}, {2,3,10},
  {1,4,10}, {15,1,10}, {14,1,10}, {4,2,10}
};

/* Table B-15, DCT coefficients table one,
 * codes 000000100x ... 000000111x
 */
static DCTtab DCTtab1a[8] =
{
  {5,2,9}, {5,2,9}, {14,1,9}, {14,1,9},
  {2,4,10}, {16,1,10}, {15,1,9}, {15,1,9}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 000000010000 ... 000000011111
 */
static DCTtab DCTtab2[16] =
{
  {0,11,12}, {8,2,12}, {4,3,12}, {0,10,12},
  {2,4,12}, {7,2,12}, {21,1,12}, {20,1,12},
  {0,9,12}, {19,1,12}, {18,1,12}, {1,5,12},
  {3,3,12}, {0,8,12}, {6,2,12}, {17,1,12}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 0000000010000 ... 0000000011111
 */
static DCTtab DCTtab3[16] =
{
  {10,2,13}, {9,2,13}, {5,3,13}, {3,4,13},
  {2,5,13}, {1,7,13}, {1,6,13}, {0,15,13},
  {0,14,13}, {0,13,13}, {0,12,13}, {26,1,13},
  {25,1,13}, {24,1,13}, {23,1,13}, {22,1,13}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 00000000010000 ... 00000000011111
 */
static DCTtab DCTtab4[16] =
{
  {0,31,14}, {0,30,14}, {0,29,14}, {0,28,14},
  {0,27,14}, {0,26,14}, {0,25,14}, {0,24,14},
  {0,23,14}, {0,22,14}, {0,21,14}, {0,20,14},
  {0,19,14}, {0,18,14}, {0,17,14}, {0,16,14}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 000000000010000 ... 000000000011111
 */
static DCTtab DCTtab5[16] =
{
  {0,40,15}, {0,39,15}, {0,38,15}, {0,37,15},
  {0,36,15}, {0,35,15}, {0,34,15}, {0,33,15},
  {0,32,15}, {1,14,15}, {1,13,15}, {1,12,15},
  {1,11,15}, {1,10,15}, {1,9,15}, {1,8,15}
};

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 0000000000010000 ... 0000000000011111
 */
static DCTtab DCTtab6[16] =
{
  {1,18,16}, {1,17,16}, {1,16,16}, {1,15,16},
  {6,3,16}, {16,2,16}, {15,2,16}, {14,2,16},
  {13,2,16}, {12,2,16}, {11,2,16}, {31,1,16},
  {30,1,16}, {29,1,16}, {28,1,16}, {27,1,16}
};


// r=0,l=0 -> EOB
// r=1,l=0 -> ESCAPE

struct CoeffTab
{
  uint8 run;
  int16 level;
  uint8 nbits;
};

static CoeffTab coefftab_first[1<<17];
static CoeffTab coefftab_nonfirst[1<<17];
static CoeffTab coefftab_B15[1<<17];

struct CoeffTabInit
{
  uint8  run;
  int16 level;
  uint8 nbits;
  uint32 bits;
};

struct CoeffTabInit2
{
  uint8  run;
  int16 level;
  char* bits;
};

CoeffTabInit coeff[] =
{
  { 0, 2,  5, 0x0008 },
  { 0, 3,  6, 0x000A },
  { 0, 4,  8, 0x000C },
  { 0, 5,  9, 0x004C },
  { 0, 6,  9, 0x0042 },
  { 0, 7, 11, 0x0014 },
  { 0, 8, 13, 0x003A },
  { 0, 9, 13, 0x0030 },
  { 0,10, 13, 0x0026 },
  { 0,11, 13, 0x0020 },
  { 0,12, 14, 0x0034 },
  { 0,13, 14, 0x0032 },
  { 0,14, 14, 0x00C0>>2 },
  { 0,15, 14, 0x00B8>>2 },
  { 0,16, 15, 0x007C>>1 },
  { 0,17, 15, 0x0078>>1 },
  { 0,18, 15, 0x0074>>1 },
  { 0,19, 15, 0x0070>>1 },
  { 0,20, 15, 0x006C>>1 },
  { 0,21, 15, 0x0068>>1 },
  { 0,22, 15, 0x0064>>1 },
  { 0,23, 15, 0x0060>>1 },
  { 0,24, 15, 0x005C>>1 },
  { 0,25, 15, 0x0058>>1 },
  { 0,26, 15, 0x0054>>1 },
  { 0,27, 15, 0x0050>>1 },
  { 0,28, 15, 0x004C>>1 },
  { 0,29, 15, 0x0048>>1 },
  { 0,30, 15, 0x0044>>1 },
  { 0,31, 15, 0x0040>>1 },
  { 0,32, 16, 0x0030 },
  { 0,33, 16, 0x002E },
  { 0,34, 16, 0x002C },
  { 0,35, 16, 0x002A },
  { 0,36, 16, 0x0028 },
  { 0,37, 16, 0x0026 },
  { 0,38, 16, 0x0024 },
  { 0,39, 16, 0x0022 },
  { 0,40, 16, 0x0020 },
  { 1, 1,  4, 0x0006 },
  { 1, 2,  7, 0x0018>>1 },
  { 1, 3,  9, 0x0025<<1 },
  { 1, 4, 11, 0x0030>>1 },
  { 1, 5, 13, 0x001B<<1 },
  { 1, 6, 14, 0x000B<<2 },
  { 1, 7, 14, 0x00A8>>2 },
  { 1, 8, 16, 0x003E },
  { 1, 9, 16, 0x003C },
  { 1,10, 16, 0x003A },
  { 1,11, 16, 0x0038 },
  { 1,12, 16, 0x0036 },
  { 1,13, 16, 0x0034 },
  { 1,14, 16, 0x0032 },
  { 1,15, 17, 0x0013<<1 },
  { 1,16, 17, 0x0012<<1 },
  { 1,17, 17, 0x0011<<1 },
  { 1,18, 17, 0x0010<<1 },
  { 2, 1,  5, 0x0005<<1 },
  { 2, 2,  8, 0x0008 },
  { 2, 3, 11, 0x002C>>1 },
  { 2, 4, 13, 0x0014<<1 },
  { 2, 5, 14, 0x00A0>>2 },
  { 3, 1,  6, 0x0038>>2 },
  { 3, 2,  9, 0x0024<<1 },
  { 3, 3, 13, 0x001C<<1 },
  { 3, 4, 14, 0x0098>>2 },
  { 4, 1,  6, 0x0030>>2 },
  { 4, 2, 11, 0x003C>>1 },
  { 4, 3, 13, 0x0012<<1 },
  { 5, 1,  7, 0x001C>>1 },
  { 5, 2, 11, 0x0024>>1 },
  { 5, 3, 14, 0x0090>>2 },
  { 6, 1,  7, 0x0014>>1 },
  { 6, 2, 13, 0x001E<<1 },
  { 6, 3, 17, 0x0014<<1 },
  { 7, 1,  7, 0x0010>>1 },
  { 7, 2, 13, 0x0015<<1 },
  { 8, 1,  8, 0x000E },
  { 8, 2, 13, 0x0011<<1 },
  { 9, 1,  8, 0x000A },
  { 9, 2, 14, 0x0088>>2 },
  {10, 1,  9, 0x0027<<1 },
  {10, 2, 14, 0x0080>>2 },
  {11, 1,  9, 0x0023<<1 },
  {11, 2, 17, 0x001A<<1 },
  {12, 1,  9, 0x0022<<1 },
  {12, 2, 17, 0x0019<<1 },
  {13, 1,  9, 0x0020<<1 },
  {13, 2, 17, 0x0018<<1 },
  {14, 1, 11, 0x0038>>1 },
  {14, 2, 17, 0x0017<<1 },
  {15, 1, 11, 0x0034>>1 },
  {15, 2, 17, 0x0016<<1 },
  {16, 1, 11, 0x0020>>1 },
  {16, 2, 17, 0x0015<<1 },
  {17, 1, 13, 0x001F<<1 },
  {18, 1, 13, 0x001A<<1 },
  {19, 1, 13, 0x0019<<1 },
  {20, 1, 13, 0x0017<<1 },
  {21, 1, 13, 0x0016<<1 },
  {22, 1, 14, 0x00F8>>2 },
  {23, 1, 14, 0x00F0>>2 },
  {24, 1, 14, 0x00E8>>2 },
  {25, 1, 14, 0x00E0>>2 },
  {26, 1, 14, 0x00D8>>2 },
  {27, 1, 17, 0x001F<<1 },
  {28, 1, 17, 0x001E<<1 },
  {29, 1, 17, 0x001D<<1 },
  {30, 1, 17, 0x001C<<1 },
  {31, 1, 17, 0x001B<<1 },
  
  { 0,0,0,0 }
};


CoeffTabInit2 coeff_B15[] =
{
  { 0, 1,  "10s" },
  { 1, 1,  "010s" },
  { 0, 2,  "110s" },
  { 2, 1,  "00101s" },
  { 0, 3,  "0111s" },
  { 3, 1,  "00111s" },
  { 4, 1,  "000110s" },
  { 1, 2,  "00110s" },
  { 5, 1,  "000111s" },
  { 6, 1,  "0000110s" },
  { 7, 1,  "0000100s" },
  { 0, 4,  "11100s" },
  { 2, 2,  "0000111s" },
  { 8, 1,  "0000101s" },
  { 9, 1,  "1111000s" },
  { 0, 5,  "11101s" },
  { 0, 6,  "000101s" },
  { 1, 3,  "1111001s" },
  { 3, 2,  "00100110s" },
  {10, 1,  "1111010s" },
  {11, 1,  "00100001s" },
  {12, 1,  "00100101s" },
  {13, 1,  "00100100s" },
  { 0, 7,  "000100s" },
  { 1, 4,  "00100111s" },
  { 2, 3,  "11111100s" },
  { 4, 2,  "11111101s" },
  { 5, 2,  "000000100s" },
  {14, 1,  "000000101s" },
  {15, 1,  "000000111s" },
  {16, 1,  "0000001101s" },
  { 0, 8,  "1111011s" },
  { 0, 9,  "1111100s" },
  { 0,10,  "00100011s" },
  { 0,11,  "00100010s" },
  { 1, 5,  "00100000s" },
  { 2, 4,  "0000001100s" },
  { 3, 3,  "000000011100s" },
  { 4, 3,  "000000010010s" },
  { 6, 2,  "000000011110s" },
  { 7, 2,  "000000010101s" },
  { 8, 2,  "000000010001s" },
  {17, 1,  "000000011111s" },
  {18, 1,  "000000011010s" },
  {19, 1,  "000000011001s" },
  {20, 1,  "000000010111s" },
  {21, 1,  "000000010110s" },
  { 0,12,  "11111010s" },
  { 0,13,  "11111011s" },
  { 0,14,  "11111110s" },
  { 0,15,  "11111111s" },
  { 1, 6,  "0000000010110s" },
  { 1, 7,  "0000000010101s" },
  { 2, 5,  "0000000010100s" },
  { 3, 4,  "0000000010011s" },
  { 5, 3,  "0000000010010s" },
  { 9, 2,  "0000000010001s" },
  {10, 2,  "0000000010000s" },
  {22, 1,  "0000000011111s" },
  {23, 1,  "0000000011110s" },
  {24, 1,  "0000000011101s" },
  {25, 1,  "0000000011100s" },
  {26, 1,  "0000000011011s" },
  { 0,16,  "00000000011111s" },
  { 0,17,  "00000000011110s" },
  { 0,18,  "00000000011101s" },
  { 0,19,  "00000000011100s" },
  { 0,20,  "00000000011011s" },
  { 0,21,  "00000000011010s" },
  { 0,22,  "00000000011001s" },
  { 0,23,  "00000000011000s" },
  { 0,24,  "00000000010111s" },
  { 0,25,  "00000000010110s" },
  { 0,26,  "00000000010101s" },
  { 0,27,  "00000000010100s" },
  { 0,28,  "00000000010011s" },
  { 0,29,  "00000000010010s" },
  { 0,30,  "00000000010001s" },
  { 0,31,  "00000000010000s" },
  { 0,32,  "000000000011000s" },
  { 0,33,  "000000000010111s" },
  { 0,34,  "000000000010110s" },
  { 0,35,  "000000000010101s" },
  { 0,36,  "000000000010100s" },
  { 0,37,  "000000000010011s" },
  { 0,38,  "000000000010010s" },
  { 0,39,  "000000000010001s" },
  { 0,40,  "000000000010000s" },
  { 1, 8,  "000000000011111s" },
  { 1, 9,  "000000000011110s" },
  { 1,10,  "000000000011101s" },
  { 1,11,  "000000000011100s" },
  { 1,12,  "000000000011011s" },
  { 1,13,  "000000000011010s" },
  { 1,14,  "000000000011001s" },
  { 1,15,  "0000000000010011s" },
  { 1,16,  "0000000000010010s" },
  { 1,17,  "0000000000010001s" },
  { 1,18,  "0000000000010000s" },
  { 6, 3,  "0000000000010100s" },
  {11, 2,  "0000000000011010s" },
  {12, 2,  "0000000000011001s" },
  {13, 2,  "0000000000011000s" },
  {14, 2,  "0000000000010111s" },
  {15, 2,  "0000000000010110s" },
  {16, 2,  "0000000000010101s" },
  {27, 1,  "0000000000011111s" },
  {28, 1,  "0000000000011110s" },
  {29, 1,  "0000000000011101s" },
  {30, 1,  "0000000000011100s" },
  {31, 1,  "0000000000011011s" },
  
  { 0,0,0 }
};


int Bin2Int(const char* s)
{
  int val=0;
  while (*s)
    {
      val<<=1;
      if (*s=='1') val++;
      s++;
    }

  return val;
}


class CoeffTabInitializer
{
public:
  CoeffTabInitializer()
  {
    for (int i=0;coeff[i].nbits;i++)
      {
	uint32 bits  = coeff[i].bits;
	int    nbits = coeff[i].nbits;

	bits <<= (17-nbits);

	Insert(coefftab_first, bits,nbits,     coeff[i].run,coeff[i].level);
	Insert(coefftab_nonfirst, bits,nbits,  coeff[i].run,coeff[i].level);

	bits |= 1<<(17-nbits);

	Insert(coefftab_first, bits,nbits,     coeff[i].run,-coeff[i].level);
	Insert(coefftab_nonfirst, bits,nbits,  coeff[i].run,-coeff[i].level);
      }

    Insert(coefftab_first,    0x2<<15,2, 0, 1);
    Insert(coefftab_first,    0x3<<15,2, 0,-1);

    Insert(coefftab_nonfirst, 0x2<<15,2, 0, 0);
    Insert(coefftab_nonfirst, 0x6<<14,3, 0, 1);
    Insert(coefftab_nonfirst, 0x7<<14,3, 0,-1);

    Insert(coefftab_first,    0x1<<11,6, 1,0);
    Insert(coefftab_nonfirst, 0x1<<11,6, 1,0);


    // B15


    for (int i=0;coeff_B15[i].bits;i++)
      {
	uint32 bits  = Bin2Int(coeff_B15[i].bits);
	int    nbits = strlen(coeff_B15[i].bits);

	bits <<= (17-nbits);
	Insert(coefftab_B15, bits,nbits,  coeff_B15[i].run, coeff_B15[i].level);
	bits |= 1<<(17-nbits);
	Insert(coefftab_B15, bits,nbits,  coeff_B15[i].run,-coeff_B15[i].level);
      }

    Insert(coefftab_B15, 0x6<<13,4, 0, 0);
    Insert(coefftab_B15, 0x1<<11,6, 1, 0);
  }

  void Insert(CoeffTab* coeff,uint32 bits,int nbits,uint8 run,int16 level)
  {
    for (int i=0 ; i < (1<<(17-nbits)) ; i++)
      {
	coeff[bits | i].run   = run;
	coeff[bits | i].level = level;
	coeff[bits | i].nbits = nbits;
      }
  }
} dummy2348756;

bool GetRunLen_old(FastBitBuf& bs,int& run,int& value ,bool B15,bool first,bool MPEG1);

/*maybeinline*/ bool GetRunLen(FastBitBuf& bs,int& run,int& value ,bool B15,bool first,bool MPEG1)
{
#if 0
  return GetRunLen_old(bs,run,value ,B15,first,MPEG1);
#endif

  uint32 code = bs.PeekBits(17);

  CoeffTab* entry;

  if (B15)
    entry = &coefftab_B15[code];
  else
    {
      if (first) entry = &coefftab_first[code];
      else       entry = &coefftab_nonfirst[code];
    }

  bs.SkipBits(entry->nbits);

  if (entry->level)
    {
      value = entry->level;
      run   = entry->run;

      return false;
    }

  if (entry->run == 0)
    {
      return true;
    }

  run = bs.GetBits(6);

  if (MPEG1)
    {
      value = bs.GetBits(8);
      if (value==0)
	value = bs.GetBits(8);
      else if (value==128)
	value = bs.GetBits(8) - 256;
      else if (value>128)
	value -= 256;
    }
  else
    {
      value = bs.GetBits(12);
      if (value==0)
	throw Excpt_Huffman(ErrSev_Warning,
			    "Invalid MPEG stream, not allowed MPEG-2 escape seq value 0 read.");

      if (value >= 0x800)
	value = -(2048-(value-0x800));
    }

  return false;
}

maybeinline bool GetRunLen_old(FastBitBuf& bs,int& run,int& value ,bool B15,bool first,bool MPEG1)
{
  uint16 code = bs.PeekBits(16);

  DCTtab* tab;

  if (!B15 && first && code&0x8000)
    {
      if (code&0x4000) value=-1;
      else             value= 1;
      run=0;

      bs.SkipBitsFast(2);
      return false;
    }

  if (code>=16384 && !B15)
    tab = &DCTtabnext[(code>>12)-4];
  else if (code>=1024)
    {
      if (B15)
        tab = &DCTtab0a[(code>>8)-4];
      else
        tab = &DCTtab0[(code>>8)-4];
    }
  else if (code>=512)
    {
      if (B15)
        tab = &DCTtab1a[(code>>6)-8];
      else
        tab = &DCTtab1[(code>>6)-8];
    }
  else if (code>=256)
    tab = &DCTtab2[(code>>4)-16];
  else if (code>=128)
    tab = &DCTtab3[(code>>3)-16];
  else if (code>=64)
    tab = &DCTtab4[(code>>2)-16];
  else if (code>=32)
    tab = &DCTtab5[(code>>1)-16];
  else if (code>=16)
    tab = &DCTtab6[code-16];
  else
    {
      MessageDisplay::Show(ErrSev_Warning,"invalid Huffman code for DCT run/level-pair");
      return false;
    }


  bs.SkipBitsFast(tab->len);

  if (tab->run==64)  // EOB
    {
      return true;
    }

  if (tab->run==65) /* escape */
    {
      if (MPEG1)
        {
          run = bs.GetBits(6);
      
          value = bs.GetBits(8);
          if (value==0)
            value = bs.GetBits(8);
          else if (value==128)
            value = bs.GetBits(8) - 256;
          else if (value>128)
            value -= 256;
        }
      else
        {
          run = bs.GetBits(6);
          value = bs.GetBits(12);
          if (value==0)
            throw Excpt_Huffman(ErrSev_Warning,
				"Invalid MPEG stream, not allowed MPEG-2 escape seq value 0 read.");

          if (value >= 0x800)
            value = -(2048-(value-0x800));
        }
    }
  else
    {
      run   = tab->run;
      value = tab->level;
      if (bs.GetBits(1))
        value = -value;
    }

  return false;
}


maybeinline bool GetRunLen_ENVELOPE(FastBitBuf& bs,int& run,int& value ,bool B15,bool first,bool MPEG1)
{
  uint32 nextbits = bs.PeekBits(20);

  for (int i=0;i<20;i++)
    {
      if (nextbits & ((1<<19)>>i)) cout << "1"; else cout << "0";
    }

  cout << " ";

  bool ret = GetRunLen_old(bs,run,value,B15,first,MPEG1);
  //bool ret = GetRunLen_new(bs,run,value,B15,first,MPEG1);
  
  cout << run << " " << value << " " << (ret ? "T" : "F") << endl;
  return ret;
}






//static void get_mpeg1_intra_block (picture_t * picture, slice_t * slice,
//                                   int16_t * dest)

#if 0
maybeinline bool GetRunLen_old(FastBitBuf& bs,int& run,int& value ,bool B15,bool first,bool MPEG1)
{
    int i;
    int j;
    int val;
    DCTtab * tab;
    uint32_t bit_buf;
    int bits;

    i = 0;

    bs.MakeLocalCopy(bit_buf,bits);
    //bit_buf = bitstream_buf;
    //bits = bitstream_bits;

    NEEDBITS (bit_buf, bits);

    while (1) {
        if (bit_buf >= 0x28000000) {

            tab = DCT_B14AC_5 - 5 + UBITS (bit_buf, 5);

            i += tab->run;
            if (i >= 64)
                break;  // end of block

        normal_code:
            j = scan[i];
            bit_buf <<= tab->len;
            bits += tab->len + 1;
            val = tab->level;
            val = (val ^ SBITS (bit_buf, 1)) - SBITS (bit_buf, 1);

            value = val;

            bit_buf <<= 1;
            NEEDBITS (bit_buf, bits);

            continue;

        } else if (bit_buf >= 0x04000000) {

            tab = DCT_B14_8 - 4 + UBITS (bit_buf, 8);

            i += tab->run;
            if (i < 64)
                goto normal_code;

            // escape code

            i += UBITS (bit_buf << 6, 6) - 64;
            if (i >= 64)
                break;  // illegal, but check needed to avoid buffer overflow

            j = scan[i];

            DUMPBITS (bit_buf, bits, 12);
            NEEDBITS (bit_buf, bits);
            val = SBITS (bit_buf, 8);
            if (! (val & 0x7f)) {
                DUMPBITS (bit_buf, bits, 8);
                val = UBITS (bit_buf, 8) + 2 * val;
            }
            val = (val * quantizer_scale * quant_matrix[j]) / 16;

            SATURATE (val);
            dest[j] = val;

            DUMPBITS (bit_buf, bits, 8);
            NEEDBITS (bit_buf, bits);

            continue;

        } else if (bit_buf >= 0x02000000) {
            tab = DCT_B14_10 - 8 + UBITS (bit_buf, 10);
            i += tab->run;
            if (i < 64)
                goto normal_code;
        } else if (bit_buf >= 0x00800000) {
            tab = DCT_13 - 16 + UBITS (bit_buf, 13);
            i += tab->run;
            if (i < 64)
                goto normal_code;
        } else if (bit_buf >= 0x00200000) {
            tab = DCT_15 - 16 + UBITS (bit_buf, 15);
            i += tab->run;
            if (i < 64)
                goto normal_code;
        } else {
            tab = DCT_16 + UBITS (bit_buf, 16);
            bit_buf <<= 16;
            bit_buf |= getword () << (bits + 16);
            i += tab->run;
            if (i < 64)
                goto normal_code;
        }
        break;  // illegal, but check needed to avoid buffer overflow
    }
    DUMPBITS (bit_buf, bits, 2);        // dump end of block code


    bs.RestoreFromLocal(bit_buf,bits);
    //bitstream_buf = bit_buf;
    //bitstream_bits = bits;
}
#endif


/************************************* initialization ********************************/

static void InitTAB(const DirectVLCTableEntry* src,DirectVLC* dest,int invalid_value)
{
  int maxbits=0;
  for (uint16 n=0; src[n].nBits>=0 ;n++)
    if (src[n].nBits>maxbits)
      maxbits=src[n].nBits;

  int size = (1<<maxbits);

  for (int i=0;i<size;i++)
    dest[i].value = invalid_value;

  for (uint16 n=0; src[n].nBits>=0 ;n++)
    if (src[n].nBits>0)
      {
        uint16 vlc_base = src[n].bits << (maxbits - src[n].nBits);
        
        for (int i=0;i<(1<<(maxbits - src[n].nBits));i++)
          {
            dest[vlc_base+i].value = n;
            dest[vlc_base+i].bits  = src[n].nBits;
          }
      }
}

static class DummyInit_23487635
{
public:
  DummyInit_23487635()
    {
      InitTAB(vlc_mbaddrinc_tab,vlc_mbaddr,-1);
      InitTAB(vlc_mbmode_P_tab,vlc_mbmode_P,0);
      InitTAB(vlc_mbmode_B_tab,vlc_mbmode_B,0);
      InitTAB(vlc_cbp_tab,vlc_cbp,-1);
      InitTAB(vlc_DClum_tab,vlc_DClum,-1);
      InitTAB(vlc_DCchrom_tab,vlc_DCchrom,-1);
      InitTAB(vlc_mcode_tab,vlc_mc,-1);
    }
} dummyinit_23845673;
