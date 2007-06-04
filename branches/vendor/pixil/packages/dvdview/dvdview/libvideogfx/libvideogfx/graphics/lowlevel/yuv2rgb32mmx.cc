/*
 *  yuv2rgb32mmx.cc
 */

#include <iostream.h>
#include <iomanip.h>

#include "yuv2rgb32mmx.hh"


static uint64 UVoffset   = 0x0080008000800080LL;     //   0  4x  128   // UV offs
static uint64 Yoffset    = 0x1010101010101010LL;     //   8  8x   16   // Y offs
static uint64 Cb2Rfact   = 0x0066006600660066LL;     //  16  4x  102 =  409/4         // Cb  ->R
static uint64 CbCr2Gfact = 0x0034001900340019LL;     //  24  2x (52 25) = 208/4 100/4 // CbCr->G
static uint64 Cb2Bfact   = 0x0081008100810081LL;     //  32  4x  129 =  516/4         // Cb  ->B
static uint64 Yfact      = 0x004A004A004A004ALL;     //  40  4x   74 =  298/4         // Y mul
static uint64 shift6bit  = 0x0000000000000006LL;     //  40  4x   74 =  298/4         // Y mul

static uint64 tmp_cr, tmp_rimpact, tmp_rimpact2, tmp_gimpact, tmp_bimpact, tmp_bimpact2;


bool i2r_32bit_BGR_mmx::s_CanConvert(const Image_YUV<Pixel>& img,const RawImageSpec_RGB& spec)
{
  if (spec.resize_to_fixed || spec.resize_with_factor) return false;
  if (spec.bits_per_pixel != 32) return false;
  if (spec.r_bits != 8 || spec.g_bits != 8 || spec.b_bits != 8) return false;
  if (!spec.little_endian) return false;
  if (spec.r_shift!=16 || spec.g_shift!= 8 || spec.b_shift!= 0) return false;

  ImageParam_YUV param;
  img.GetParam(param);

  if (param.nocolor==true) return false;
  if (param.chroma !=Chroma420) return false;

  int w = (param.width+7) & ~7;
  if (spec.bytes_per_line < 4*w) return false;

  return true;
}

void i2r_32bit_BGR_mmx::Transform(const Image_YUV<Pixel>& img,uint8* mem,int firstline,int lastline)
{
  ImageParam_YUV param;
  img.GetParam(param);

  assert(param.chroma==Chroma420);
  assert((firstline%2) == 0);

  const Pixel*const* pix_y  = img.AskFrameY_const();
  const Pixel*const* pix_cb = img.AskFrameU_const();
  const Pixel*const* pix_cr = img.AskFrameV_const();

  const int h = param.height;
  const int w = param.width;

  for (int y=firstline;y<=lastline;y+=2)
    {
      const uint8*  yptr1 = (uint8*)pix_y[y];
      const uint8*  yptr2 = (uint8*)pix_y[y+1];
      const uint8*  cbptr = (uint8*)pix_cb[y/2];
      const uint8*  crptr = (uint8*)pix_cr[y/2];

      uint8* membuf_a = ((uint8*)(mem))+d_spec.bytes_per_line*(y-firstline);
      uint8* membuf_b = membuf_a+d_spec.bytes_per_line;

      for (int x=0;x<w;x+=8)
        {
          __asm__ __volatile__
            (
	     ".align 8 \n\t"
             "movd        (%1),%%mm1\n\t"   // 4 Cb-Werte nach mm1
             " pxor       %%mm0,%%mm0\n\t"  // mm0=0
             "movd        (%2),%%mm2\n\t"   // 4 Cr-Werte nach mm2
             " punpcklbw  %%mm0,%%mm1\n\t"  // Cb-Werte in mm1 auf 16bit Breite bringen
             "psubw       UVoffset,%%mm1\n\t"   // Offset 128 von Cb-Werten abziehen
             " punpcklbw  %%mm0,%%mm2\n\t"  // Cr-Werte in mm2 auf 16bit Breite bringen
             "psubw       UVoffset,%%mm2\n\t"   // Offset 128 von Cr-Werten abziehen

             " movq       %%mm1,%%mm3\n\t"  // Kopie von Cb-Werten nach mm3
             "movq        %%mm1,%%mm5\n\t"  // ... und nach mm5
             " punpcklwd  %%mm2,%%mm1\n\t"  // in mm1 ist jetzt: LoCr1 LoCb1 LoCr2 LoCb2
             "pmaddwd     CbCr2Gfact,%%mm1\n\t" // mm1 mit CbCr-MulAdd -> LoGimpact1 LoGimpact2
             " punpckhwd  %%mm2,%%mm3\n\t"  // in mm3 ist jetzt: HiCr1 HiCb1 HiCr2 HiCb2
             "pmaddwd     CbCr2Gfact,%%mm3\n\t" // mm3 mit CbCr-MulAdd -> HiGimpact1 HiGimpact2

             "movq        %%mm2,tmp_cr\n\t" // mm2 sichern (Cr)
             "movq        (%0),%%mm6\n\t"   // 8 Y-Pixel nach mm6
             "psubusb     Yoffset,%%mm6\n\t"  // Y -= 16
             " packssdw   %%mm3,%%mm1\n\t"  // mm1 enthaelt nun 4x G-Impact
             "movq        %%mm6,%%mm7\n\t"  // Y-Pixel nach mm7 kopieren
             " punpcklbw  %%mm0,%%mm6\n\t"  // 4 low Y-Pixel nach mm6
             "pmullw      Yfact,%%mm6\n\t" // ... diese mit Ymul multiplizieren
             " punpckhbw  %%mm0,%%mm7\n\t"  // 4 high Y-Pixel nach mm7
             "pmullw      Yfact,%%mm7\n\t" // ... diese mit Ymul multiplizieren
             " movq       %%mm1,%%mm4\n\t"  // G-Impact nach mm4
             "movq        %%mm1,tmp_gimpact\n\t" // G-Impact sichern
             " punpcklwd  %%mm1,%%mm1\n\t"  // beide low G-Impacts verdoppeln
             "movq        %%mm6,%%mm0\n\t"  // 4 low Y-Pixel nach mm0
             " punpckhwd  %%mm4,%%mm4\n\t"  // beide high G-Impacts verdoppeln
             "movq        %%mm7,%%mm3\n\t"  // 4 high Y-Pixel nach mm3
             " psubw      %%mm1,%%mm6\n\t"  // 4 low G in mm6 berechnen
               "psraw       shift6bit,%%mm6\n\t"// G-Werte in mm6 in richtige Position bringen
             " psubw      %%mm4,%%mm7\n\t"  // 4 high G in mm7 berechnen
             "movq        %%mm5,%%mm2\n\t"  // 4 Cr-Werte nach mm2
             " punpcklwd  %%mm5,%%mm5\n\t"  // beide low Cr-Impacts verdoppeln
             "pmullw      Cb2Bfact,%%mm5\n\t" // 4 low B-Impacts berechnen
             " punpckhwd  %%mm2,%%mm2\n\t"  // beide high Cr-Impacts verdoppeln
               "psraw       shift6bit,%%mm7\n\t"// G-Werte in mm7 in richtige Position bringen
             " pmullw     Cb2Bfact,%%mm2\n\t" // 4 high B-Impacts berechnen
             "packuswb    %%mm7,%%mm6\n\t"  // G-Werte in mm6 zusammenfassen

             "movq        %%mm5,tmp_bimpact\n\t" // 4 low B-Impacts sichern
             " paddw      %%mm0,%%mm5\n\t"  // 4 low B in mm5 berechnen
             "movq        %%mm2,tmp_bimpact2\n\t" // 4 high B-Impacts sichern
             " paddw      %%mm3,%%mm2\n\t"  // 4 high B in mm2 berechnen
               "psraw       shift6bit,%%mm5\n\t"// B-Werte in richtige Position bringen
               "psraw       shift6bit,%%mm2\n\t"// B-Werte in richtige Position bringen
             "packuswb    %%mm2,%%mm5\n\t"  // B-Werte in mm5 zusammenfassen

             "movq        tmp_cr,%%mm2\n\t" // 4 Cr-Werte nach mm2
             "movq        %%mm2,%%mm7\n\t"  // 4 Cr-Werte nach mm7 kopieren
             " punpcklwd  %%mm2,%%mm2\n\t"  // 2 low Cr Werte verdoppeln
             "pmullw      Cb2Rfact,%%mm2\n\t" // 2 low R-Impacts berechnen
             " punpckhwd  %%mm7,%%mm7\n\t"  // 2 high Cr Werte verdoppeln
             "pmullw      Cb2Rfact,%%mm7\n\t" // 4 high R-Impacts berechnen
             "movq        %%mm2,tmp_rimpact\n\t" // 4 low R-Impacts sichern
             "paddw       %%mm0,%%mm2\n\t"  // 4 low R berechnen
               "psraw      shift6bit,%%mm2\n\t" // 4 low R in richtige Position bringen
             " pxor       %%mm4,%%mm4\n\t"  // mm4=0
             "movq        %%mm7,tmp_rimpact2\n\t" // 4 high R-Impacts sichern
             " paddw      %%mm3,%%mm7\n\t"  // 4 high R berechnen
               "psraw      shift6bit,%%mm7\n\t" // 4 high R in richtige Position bringen
             " packuswb   %%mm7,%%mm2\n\t"  // R-Werte in mm2 zusammenfassen

	     //"movq        %%mm6,(%4)\n\t"


             // Nun noch in richtiges Display-Format umwandeln.

             : : "r" (yptr1), "r" (cbptr), "r" (crptr) 
             );


          __asm__ __volatile__
            (
	     ".align 8 \n\t"
	     "movq       %%mm2,%%mm7\n\t" // G
	     "movq       %%mm5,%%mm4\n\t" // B
	     "movq       %%mm6,%%mm3\n\t" // R

	     "pxor       %%mm0,%%mm0\n\t"
	     "punpcklbw  %%mm0,%%mm2\n\t"
	     "punpcklbw  %%mm6,%%mm5\n\t"
	     "movq       %%mm5,%%mm1\n\t"
	     "punpcklwd  %%mm2,%%mm5\n\t"
             "movq       %%mm5,  (%1)\n\t" // die ersten  2 RGB32 Pixel schreiben
	     "punpckhwd  %%mm2,%%mm1\n\t"
             "movq       %%mm1, 8(%1)\n\t" // die zweiten 2 RGB32 Pixel schreiben

	     "pxor       %%mm0,%%mm0\n\t"
	     "punpckhbw  %%mm0,%%mm7\n\t"
	     "punpckhbw  %%mm3,%%mm4\n\t"
	     "movq       %%mm4,%%mm2\n\t"
	     "punpcklwd  %%mm7,%%mm4\n\t"
             "movq       %%mm4,16(%1)\n\t" // die dritten 2 RGB32 Pixel schreiben
	     "punpckhwd  %%mm7,%%mm2\n\t"
             "movq       %%mm2,24(%1)\n\t" // die vierten 2 RGB32 Pixel schreiben


             // zweite der beiden Zeilen bearbeiten

             "movq       (%0),%%mm1\n\t"    // 8 Y-Pixel nach mm1
             " pxor      %%mm2,%%mm2\n\t"   // mm2=0
             "psubusb    Yoffset,%%mm1\n\t"   // Y-Offset subtrahieren
             "movq       %%mm1,%%mm5\n\t"   // 8 Y nach mm5
             " punpcklbw %%mm2,%%mm1\n\t"   // 4 low Y nach mm1
             "pmullw     Yfact,%%mm1\n\t"  // 4 low Y mit Ymul multiplizieren
             " punpckhbw %%mm2,%%mm5\n\t"   // 4 high Y nach mm5
             "pmullw     Yfact,%%mm5\n\t"  // 4 high Y mit Ymul multiplizieren
             " movq      %%mm1,%%mm0\n\t"   // 4 low Y nach mm0
             "paddw      tmp_rimpact,%%mm0\n\t"  // 4 low R-Impacts addieren -> 4 low R in mm0
             " movq      %%mm5,%%mm6\n\t"   // 4 high Y nach mm6
               "psraw      shift6bit,%%mm0\n\t" // 4 low R in richtige Position schieben
             "paddw      tmp_rimpact2,%%mm5\n\t"  // 4 high R-Impacts addieren -> 4 high R in mm5
             " movq      %%mm1,%%mm2\n\t"   // 4 low Y nach mm2
               "psraw      shift6bit,%%mm5\n\t" // 4 high R in richtige Position schieben
             "paddw      tmp_bimpact,%%mm2\n\t"  // 4 low B-Impacts addieren -> 4 low B in mm2
             " packuswb  %%mm5,%%mm0\n\t"   // 8 R Werte zusammenfassen nach mm0
               "psraw      shift6bit,%%mm2\n\t" // 4 low B in Position schieben
             " movq      %%mm6,%%mm5\n\t"   // 4 high Y nach mm5
             "paddw      tmp_bimpact2,%%mm6\n\t"  // 4 high B-Impacts addieren -> 4 high B in mm6
               "psraw      shift6bit,%%mm6\n\t" // 4 high B in richtige Position schieben
             "movq       tmp_gimpact,%%mm3\n\t"  // 4 low G-Impacts nach mm3
             "packuswb   %%mm6,%%mm2\n\t"   // 8 B Werte zusammenfassen nach mm2
             " movq      %%mm3,%%mm4\n\t"   // 4 low G-Impacts nach mm4 kopieren
             "punpcklwd  %%mm3,%%mm3\n\t"   // low 2 G-Impacts verdoppeln
             "punpckhwd  %%mm4,%%mm4\n\t"   // high 2 G-Impacts verdoppeln
             " psubw     %%mm3,%%mm1\n\t"   // 4 low G in mm1 berechnen
               "psraw      shift6bit,%%mm1\n\t" // 4 low G in richtige Position schieben
             " psubw     %%mm4,%%mm5\n\t"   // 4 high G in mm5 berechnen
               "psraw      shift6bit,%%mm5\n\t" // 4 high G in richtige Position schieben
             " packuswb  %%mm5,%%mm1\n\t"   // 8 G Werte in mm1 zusammenfassen

             // Nun noch in richtiges Display-Format umwandeln.

	     /*
	       6->1
	       2->0
	       5->2
	       3->3
	       4->4
	       7->7
	       0->6
	       1->5
	     */

	     "movq       %%mm0,%%mm7\n\t" // R
	     "movq       %%mm2,%%mm4\n\t" // B
	     "movq       %%mm1,%%mm3\n\t" // G

	     "pxor       %%mm6,%%mm6\n\t"
	     "punpcklbw  %%mm6,%%mm0\n\t"
	     "punpcklbw  %%mm1,%%mm2\n\t"
	     "movq       %%mm2,%%mm5\n\t"
	     "punpcklwd  %%mm0,%%mm2\n\t"
             "movq       %%mm2,  (%2)\n\t" // die ersten  2 RGB32 Pixel schreiben
	     "punpckhwd  %%mm0,%%mm5\n\t"
             "movq       %%mm5, 8(%2)\n\t" // die zweiten 2 RGB32 Pixel schreiben

	     "pxor       %%mm6,%%mm6\n\t"
	     "punpckhbw  %%mm6,%%mm7\n\t"
	     "punpckhbw  %%mm3,%%mm4\n\t"
	     "movq       %%mm4,%%mm0\n\t"
	     "punpcklwd  %%mm7,%%mm4\n\t"
             "movq       %%mm4,16(%2)\n\t" // die dritten 2 RGB32 Pixel schreiben
	     "punpckhwd  %%mm7,%%mm0\n\t"
             "movq       %%mm0,24(%2)\n\t" // die vierten 2 RGB32 Pixel schreiben

             : : "r" (yptr2), "r" (membuf_a), "r" (membuf_b) 
             );

          yptr1+=8;
          yptr2+=8;
          cbptr+=4;
          crptr+=4;
          membuf_a+=32;
          membuf_b+=32;
        }
    }

  __asm__
    (
     "emms\n\t"
     );
}
