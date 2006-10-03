

typedef struct {
  uint8 run;
  uint8 level;
  uint8 len;
} DCTtabb;



static DCTtabb DCT_16 [] = {
    {129, 0, 0}, {129, 0, 0}, {129, 0, 0}, {129, 0, 0},
    {129, 0, 0}, {129, 0, 0}, {129, 0, 0}, {129, 0, 0},
    {129, 0, 0}, {129, 0, 0}, {129, 0, 0}, {129, 0, 0},
    {129, 0, 0}, {129, 0, 0}, {129, 0, 0}, {129, 0, 0},
    {  2,18, 0}, {  2,17, 0}, {  2,16, 0}, {  2,15, 0},
    {  7, 3, 0}, { 17, 2, 0}, { 16, 2, 0}, { 15, 2, 0},
    { 14, 2, 0}, { 13, 2, 0}, { 12, 2, 0}, { 32, 1, 0},
    { 31, 1, 0}, { 30, 1, 0}, { 29, 1, 0}, { 28, 1, 0}
};

static DCTtabb DCT_15 [] = {
    {  1,40,15}, {  1,39,15}, {  1,38,15}, {  1,37,15},
    {  1,36,15}, {  1,35,15}, {  1,34,15}, {  1,33,15},
    {  1,32,15}, {  2,14,15}, {  2,13,15}, {  2,12,15},
    {  2,11,15}, {  2,10,15}, {  2, 9,15}, {  2, 8,15},
    {  1,31,14}, {  1,31,14}, {  1,30,14}, {  1,30,14},
    {  1,29,14}, {  1,29,14}, {  1,28,14}, {  1,28,14},
    {  1,27,14}, {  1,27,14}, {  1,26,14}, {  1,26,14},
    {  1,25,14}, {  1,25,14}, {  1,24,14}, {  1,24,14},
    {  1,23,14}, {  1,23,14}, {  1,22,14}, {  1,22,14},
    {  1,21,14}, {  1,21,14}, {  1,20,14}, {  1,20,14},
    {  1,19,14}, {  1,19,14}, {  1,18,14}, {  1,18,14},
    {  1,17,14}, {  1,17,14}, {  1,16,14}, {  1,16,14}
};

static DCTtabb DCT_13 [] = {
    { 11, 2,13}, { 10, 2,13}, {  6, 3,13}, {  4, 4,13},
    {  3, 5,13}, {  2, 7,13}, {  2, 6,13}, {  1,15,13},
    {  1,14,13}, {  1,13,13}, {  1,12,13}, { 27, 1,13},
    { 26, 1,13}, { 25, 1,13}, { 24, 1,13}, { 23, 1,13},
    {  1,11,12}, {  1,11,12}, {  9, 2,12}, {  9, 2,12},
    {  5, 3,12}, {  5, 3,12}, {  1,10,12}, {  1,10,12},
    {  3, 4,12}, {  3, 4,12}, {  8, 2,12}, {  8, 2,12},
    { 22, 1,12}, { 22, 1,12}, { 21, 1,12}, { 21, 1,12},
    {  1, 9,12}, {  1, 9,12}, { 20, 1,12}, { 20, 1,12},
    { 19, 1,12}, { 19, 1,12}, {  2, 5,12}, {  2, 5,12},
    {  4, 3,12}, {  4, 3,12}, {  1, 8,12}, {  1, 8,12},
    {  7, 2,12}, {  7, 2,12}, { 18, 1,12}, { 18, 1,12}
};

static DCTtabb DCT_B14_10 [] = {
    { 17, 1,10}, {  6, 2,10}, {  1, 7,10}, {  3, 3,10},
    {  2, 4,10}, { 16, 1,10}, { 15, 1,10}, {  5, 2,10}
};

static DCTtabb DCT_B14_8 [] = {
    { 65, 0, 6}, { 65, 0, 6}, { 65, 0, 6}, { 65, 0, 6},
    {  3, 2, 7}, {  3, 2, 7}, { 10, 1, 7}, { 10, 1, 7},
    {  1, 4, 7}, {  1, 4, 7}, {  9, 1, 7}, {  9, 1, 7},
    {  8, 1, 6}, {  8, 1, 6}, {  8, 1, 6}, {  8, 1, 6},
    {  7, 1, 6}, {  7, 1, 6}, {  7, 1, 6}, {  7, 1, 6},
    {  2, 2, 6}, {  2, 2, 6}, {  2, 2, 6}, {  2, 2, 6},
    {  6, 1, 6}, {  6, 1, 6}, {  6, 1, 6}, {  6, 1, 6},
    { 14, 1, 8}, {  1, 6, 8}, { 13, 1, 8}, { 12, 1, 8},
    {  4, 2, 8}, {  2, 3, 8}, {  1, 5, 8}, { 11, 1, 8}
};

static DCTtabb DCT_B14AC_5 [] = {
		 {  1, 3, 5}, {  5, 1, 5}, {  4, 1, 5},
    {  1, 2, 4}, {  1, 2, 4}, {  3, 1, 4}, {  3, 1, 4},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {129, 0, 2}, {129, 0, 2}, {129, 0, 2}, {129, 0, 2},
    {129, 0, 2}, {129, 0, 2}, {129, 0, 2}, {129, 0, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}
};

static DCTtabb DCT_B14DC_5 [] = {
		 {  1, 3, 5}, {  5, 1, 5}, {  4, 1, 5},
    {  1, 2, 4}, {  1, 2, 4}, {  3, 1, 4}, {  3, 1, 4},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1},
    {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1},
    {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1},
    {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}, {  1, 1, 1}
};

static DCTtabb DCT_B15_10 [] = {
    {  6, 2, 9}, {  6, 2, 9}, { 15, 1, 9}, { 15, 1, 9},
    {  3, 4,10}, { 17, 1,10}, { 16, 1, 9}, { 16, 1, 9}
};

static DCTtabb DCT_B15_8 [] = {
    { 65, 0, 6}, { 65, 0, 6}, { 65, 0, 6}, { 65, 0, 6},
    {  8, 1, 7}, {  8, 1, 7}, {  9, 1, 7}, {  9, 1, 7},
    {  7, 1, 7}, {  7, 1, 7}, {  3, 2, 7}, {  3, 2, 7},
    {  1, 7, 6}, {  1, 7, 6}, {  1, 7, 6}, {  1, 7, 6},
    {  1, 6, 6}, {  1, 6, 6}, {  1, 6, 6}, {  1, 6, 6},
    {  5, 1, 6}, {  5, 1, 6}, {  5, 1, 6}, {  5, 1, 6},
    {  6, 1, 6}, {  6, 1, 6}, {  6, 1, 6}, {  6, 1, 6},
    {  2, 5, 8}, { 12, 1, 8}, {  1,11, 8}, {  1,10, 8},
    { 14, 1, 8}, { 13, 1, 8}, {  4, 2, 8}, {  2, 4, 8},
    {  3, 1, 5}, {  3, 1, 5}, {  3, 1, 5}, {  3, 1, 5},
    {  3, 1, 5}, {  3, 1, 5}, {  3, 1, 5}, {  3, 1, 5},
    {  2, 2, 5}, {  2, 2, 5}, {  2, 2, 5}, {  2, 2, 5},
    {  2, 2, 5}, {  2, 2, 5}, {  2, 2, 5}, {  2, 2, 5},
    {  4, 1, 5}, {  4, 1, 5}, {  4, 1, 5}, {  4, 1, 5},
    {  4, 1, 5}, {  4, 1, 5}, {  4, 1, 5}, {  4, 1, 5},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3}, {  2, 1, 3},
    {129, 0, 4}, {129, 0, 4}, {129, 0, 4}, {129, 0, 4},
    {129, 0, 4}, {129, 0, 4}, {129, 0, 4}, {129, 0, 4},
    {129, 0, 4}, {129, 0, 4}, {129, 0, 4}, {129, 0, 4},
    {129, 0, 4}, {129, 0, 4}, {129, 0, 4}, {129, 0, 4},
    {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4},
    {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4},
    {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4},
    {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4}, {  1, 3, 4},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2}, {  1, 1, 2},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3}, {  1, 2, 3},
    {  1, 4, 5}, {  1, 4, 5}, {  1, 4, 5}, {  1, 4, 5},
    {  1, 4, 5}, {  1, 4, 5}, {  1, 4, 5}, {  1, 4, 5},
    {  1, 5, 5}, {  1, 5, 5}, {  1, 5, 5}, {  1, 5, 5},
    {  1, 5, 5}, {  1, 5, 5}, {  1, 5, 5}, {  1, 5, 5},
    { 10, 1, 7}, { 10, 1, 7}, {  2, 3, 7}, {  2, 3, 7},
    { 11, 1, 7}, { 11, 1, 7}, {  1, 8, 7}, {  1, 8, 7},
    {  1, 9, 7}, {  1, 9, 7}, {  1,12, 8}, {  1,13, 8},
    {  3, 3, 8}, {  5, 2, 8}, {  1,14, 8}, {  1,15, 8}
};


inline int DequantizeIntra(int value,int qscale,int matrix)
{
#if MMX_DCT
  return value*qscale*matrix;
#else
  return value*qscale*matrix/16;
#endif
}

inline int DequantizeInter(int value,int qscale,int matrix)
{
  // Assert(value>0);
#if MMX_DCT
  return (2*value+1) * matrix * qscale / 2;
#else
  return (2*value+1) * matrix * qscale / 32;
#endif
}

inline void Saturate(int& value)
{
#if MMX_DCT
       if (value<-2048*16) value=-2048*16;
  else if (value> 2047*16) value= 2047*16;
#else
       if (value<-2048) value=-2048;
  else if (value> 2047) value= 2047;
#endif
}


static void get_mpeg1_intra_block(class FastBitBuf& bs,short* coeff,const int* scan,
				  int quantizer_scale,
				  const int* quant_matrix)
{
  int i;
  int j;
  int val;
  DCTtabb * tab;
  uint32 bit_buf;
  int bits;

  i = 0;
  bs.MakeLocalCopy(bit_buf,bits);

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
      val = DequantizeIntra(tab->level,quantizer_scale,quant_matrix[i]);

      // if (bitstream_get (1)) val = -val;
      val = (val ^ SBITS (bit_buf, 1)) - SBITS (bit_buf, 1);

      Saturate(val);
      coeff[j] = val;

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
      val = DequantizeIntra(val,quantizer_scale,quant_matrix[i]);

      Saturate(val);
      coeff[j] = val;

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
      bit_buf |= bs.GetNextWord() << (bits + 16);
      i += tab->run;
      if (i < 64)
	goto normal_code;
    }
    break;  // illegal, but check needed to avoid buffer overflow
  }
  DUMPBITS (bit_buf, bits, 2);        // dump end of block code

  bs.RestoreFromLocal(bit_buf,bits);
}




static void get_mpeg1_non_intra_block(class FastBitBuf& bs,short* coeff,const int* scan,
				      int quantizer_scale,
				      const int* quant_matrix)
{
  int i;
  int j;
  int val;
  DCTtabb * tab;
  uint32 bit_buf;
  int bits;

  i = -1;

  bs.MakeLocalCopy(bit_buf,bits);

  NEEDBITS (bit_buf, bits);
  if (bit_buf >= 0x28000000) {
    tab = DCT_B14DC_5 - 5 + UBITS (bit_buf, 5);
    goto entry_1;
  } else
    goto entry_2;

  while (1) {
    if (bit_buf >= 0x28000000) {

      tab = DCT_B14AC_5 - 5 + UBITS (bit_buf, 5);

    entry_1:
      i += tab->run;
      if (i >= 64)
	break;  // end of block

    normal_code:
      j = scan[i];
      bit_buf <<= tab->len;
      bits += tab->len + 1;
      val = DequantizeInter(tab->level,quantizer_scale,quant_matrix[i]);

      // if (bitstream_get (1)) val = -val;
      val = (val ^ SBITS (bit_buf, 1)) - SBITS (bit_buf, 1);

      Saturate (val);
      coeff[j] = val;

      bit_buf <<= 1;
      NEEDBITS (bit_buf, bits);

      continue;

    }

  entry_2:
    if (bit_buf >= 0x04000000) {

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
      val = DequantizeInter(val + SBITS (val, 1),quantizer_scale,quant_matrix[i]);

      Saturate (val);
      coeff[j] = val;

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
      bit_buf |= bs.GetNextWord() << (bits + 16);
      i += tab->run;
      if (i < 64)
	goto normal_code;
    }
    break;  // illegal, but check needed to avoid buffer overflow
  }
  DUMPBITS (bit_buf, bits, 2);        // dump end of block code
  bs.RestoreFromLocal(bit_buf,bits);
}


static void get_intra_block_B14(class FastBitBuf& bs,short* coeff,const int* scan,
				int quantizer_scale,
				const int* quant_matrix)
{
  int i;
  int j;
  int val;
  int mismatch;
  DCTtabb * tab;
  uint32 bit_buf;
  int bits;

  i = 0;
  mismatch = ~coeff[0];

  bs.MakeLocalCopy(bit_buf,bits);

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
      val = DequantizeIntra(tab->level , quantizer_scale , quant_matrix[i]);

      // if (bitstream_get (1)) val = -val;
      val = (val ^ SBITS (bit_buf, 1)) - SBITS (bit_buf, 1);

      Saturate (val);
      coeff[j] = val;
      mismatch ^= val;

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
      val = DequantizeIntra(SBITS (bit_buf, 12) , quantizer_scale , quant_matrix[i]);

      Saturate(val);
      coeff[j] = val;
      mismatch ^= val;

      DUMPBITS (bit_buf, bits, 12);
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
      bit_buf |= bs.GetNextWord () << (bits + 16);
      i += tab->run;
      if (i < 64)
	goto normal_code;
    }
    break;  // illegal, but check needed to avoid buffer overflow
  }
  coeff[63] ^= mismatch & 1;
  DUMPBITS (bit_buf, bits, 2);        // dump end of block code
  bs.RestoreFromLocal(bit_buf,bits);
}


static void get_intra_block_B15(class FastBitBuf& bs,short* coeff,const int* scan,
				int quantizer_scale,
				const int* quant_matrix)
{
  int i;
  int j;
  int val;
  int mismatch;
  DCTtabb * tab;
  uint32 bit_buf;
  int bits;

  i = 0;
  mismatch = ~coeff[0];

  bs.MakeLocalCopy(bit_buf,bits);

  NEEDBITS (bit_buf, bits);

  while (1) {
    if (bit_buf >= 0x04000000) {

      tab = DCT_B15_8 - 4 + UBITS (bit_buf, 8);

      i += tab->run;
      if (i < 64) {

      normal_code:
	j = scan[i];
	bit_buf <<= tab->len;
	bits += tab->len + 1;
	val = DequantizeIntra(tab->level , quantizer_scale , quant_matrix[i]);

	// if (bitstream_get (1)) val = -val;
	val = (val ^ SBITS (bit_buf, 1)) - SBITS (bit_buf, 1);

	Saturate(val);
	coeff[j] = val;
	mismatch ^= val;

	bit_buf <<= 1;
	NEEDBITS (bit_buf, bits);

	continue;

      } else {

	// end of block. I commented out this code because if we
	// dont exit here we will still exit at the later test :)

	//if (i >= 128) break;  // end of block

	// escape code

	i += UBITS (bit_buf << 6, 6) - 64;
	if (i >= 64)
	  break;      // illegal, but check against buffer overflow

	j = scan[i];

	DUMPBITS (bit_buf, bits, 12);
	NEEDBITS (bit_buf, bits);
	val = DequantizeIntra(SBITS (bit_buf, 12) , quantizer_scale , quant_matrix[i]);

	Saturate(val);
	coeff[j] = val;
	mismatch ^= val;

	DUMPBITS (bit_buf, bits, 12);
	NEEDBITS (bit_buf, bits);

	continue;

      }
    } else if (bit_buf >= 0x02000000) {
      tab = DCT_B15_10 - 8 + UBITS (bit_buf, 10);
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
      bit_buf |= bs.GetNextWord() << (bits + 16);
      i += tab->run;
      if (i < 64)
	goto normal_code;
    }
    break;  // illegal, but check needed to avoid buffer overflow
  }
  coeff[63] ^= mismatch & 1;
  DUMPBITS (bit_buf, bits, 4);        // dump end of block code
  bs.RestoreFromLocal(bit_buf,bits);
}


static void get_non_intra_block(class FastBitBuf& bs,short* coeff,const int* scan,
				int quantizer_scale,
				const int* quant_matrix)
{
  int i;
  int j;
  int val;
  int mismatch;
  DCTtabb * tab;
  uint32 bit_buf;
  int bits;

  i = -1;
  mismatch = 1;

  bs.MakeLocalCopy(bit_buf,bits);

  NEEDBITS (bit_buf, bits);
  if (bit_buf >= 0x28000000) {
    tab = DCT_B14DC_5 - 5 + UBITS (bit_buf, 5);
    goto entry_1;
  } else
    goto entry_2;

  while (1) {
    if (bit_buf >= 0x28000000) {

      tab = DCT_B14AC_5 - 5 + UBITS (bit_buf, 5);

    entry_1:
      i += tab->run;
      if (i >= 64)
	break;  // end of block

    normal_code:
      j = scan[i];
      bit_buf <<= tab->len;
      bits += tab->len + 1;
      val = DequantizeInter(tab->level , quantizer_scale , quant_matrix[i]);

      // if (bitstream_get (1)) val = -val;
      val = (val ^ SBITS (bit_buf, 1)) - SBITS (bit_buf, 1);

      Saturate(val);
      coeff[j] = val;
      mismatch ^= val;

      bit_buf <<= 1;
      NEEDBITS (bit_buf, bits);

      continue;

    }

  entry_2:
    if (bit_buf >= 0x04000000) {

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
      val = DequantizeInter(SBITS (bit_buf, 12) + SBITS (bit_buf, 1) , quantizer_scale, quant_matrix[i]);

      Saturate(val);
      coeff[j] = val;
      mismatch ^= val;

      DUMPBITS (bit_buf, bits, 12);
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
      bit_buf |= bs.GetNextWord () << (bits + 16);
      i += tab->run;
      if (i < 64)
	goto normal_code;
    }
    break;  // illegal, but check needed to avoid buffer overflow
  }
  coeff[63] ^= mismatch & 1;
  DUMPBITS (bit_buf, bits, 2);        // dump end of block code
  bs.RestoreFromLocal(bit_buf,bits);
}
