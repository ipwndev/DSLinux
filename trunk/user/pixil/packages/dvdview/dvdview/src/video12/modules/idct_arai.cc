
#include "video12/modules/idct_arai.hh"

#include <math.h>
#include <assert.h>
#include <iostream.h>
#include <iomanip.h>

static Pixel* clip2_0_255;
static Pixel  clip2_0_255_data[1024];

static class InitCLIP_DCT
{
public:
  InitCLIP_DCT()
    {
      clip2_0_255 = &clip2_0_255_data[512];

      for (int i=-512;i<512;i++)
        {
          int val=i;
          if (val<0) val=0;
          if (val>255) val=255;
          clip2_0_255[i]=val;
        }
    }
} dummyclass_weiour;


/* private data */
static short iclip[1024]; /* clipping table */
static short *iclp;

// static int c1,c2;

const int FDCT_M1Scale      = 30;
const int FDCT_PostM1_VK    = 12;
const int FDCT_PostM1_NK    = 16;
const int FDCT_PostM1_Shift = 14;

const int FDCT_M2Scale      = 30;
const int FDCT_PostM2_VK    = 16;
const int FDCT_PostM2_NK    = 15;
const int FDCT_PostM2_Shift = 31;

const int FDCT_M3Scale      = 30;
const int FDCT_PostM3_VK    = 17;
const int FDCT_PostM3_NK    = 15;
const int FDCT_PostM3_Shift = 30;



const int IDCT_M3Scale      = 33;  // DOCS !
const int IDCT_PostM3_VK    = 10;
const int IDCT_PostM3_NK    = 20;
const int IDCT_PostM3_Shift = 13;

const int IDCT_M2Scale      = 28;
const int IDCT_PostM2_VK    = 12;
const int IDCT_PostM2_NK    = 16;
const int IDCT_PostM2_Shift = 32;

const int IDCT_M1Scale      = 28;
const int IDCT_PostM1_VK    = 16;
const int IDCT_PostM1_NK    = 14;
const int IDCT_PostM1_Shift = 30;


typedef int FDCTInt;
typedef int IDCTInt;



#define SC(x,u,v) { D(x); ibuf[u][v] = ((x) >> (FDCTScale-FDCT_STAGE1_NK)); }

#define D(x,s) { cout << setbase(16) << setfill('0') << setw(16) << (x) << setbase(10) << " "; \
  ibuf[c1][c2]=x; c2++; if (c2==8) { c1++; c2=0; } cout << setfill(' '); }

//#define DESCALE(x,f)   ((x<0) ? ((x - ((1LL<<(f-1)))  ) >> (f)) : ((x + ((1LL<<(f-1)))  ) >> (f)))
#define DESCALE(x,f)    ((x + ((1LL<<(f)) -1 )  ) >> (f))

//#define DESCALE(x,f)      (x >> (f))
//#define DESCALE(x,f)   ((x<0) ? ((x - (1LL<<(f))/2  ) >> (f)) : ((x + (1LL<<(f))/2  ) >> (f)))
//#define DESCALE(x,f)    ((x + ((1LL<<(f))/2 )  ) >> (f))

#define IDESCALE(x,f)    ((x + ((1LL<<(f-1)) )  ) >> (f))
#define IDESCALENeg(x,f) ((x - ((1LL<<(f-1)) )  ) >> (f))

/* Eine 32x32 -> 64bit Multiplikation durchfuehren */
#define MUL32(a,b) (((long long)(a))*((long)(b)))

const long long FDCT_M1ScaleFact = (1LL<<FDCT_M1Scale);
const long long FDCT_M2ScaleFact = (1LL<<FDCT_M2Scale);
const long long FDCT_M3ScaleFact = (1LL<<FDCT_M3Scale);

const long long IDCT_M3ScaleFact = (1LL<<IDCT_M3Scale);
const long long IDCT_M2ScaleFact = (1LL<<IDCT_M2Scale);
const long long IDCT_M1ScaleFact = (1LL<<IDCT_M1Scale);

#ifndef PI
#define PI 3.1415926535
#endif

typedef long long DCTInt2;

static bool IsInitialized = false;

static DCTInt2  A1;
static DCTInt2 MA2;
static DCTInt2  A3;
static DCTInt2  A4;
static DCTInt2  A5;

static DCTInt2  A1b;
static DCTInt2 MA2b;
static DCTInt2  A3b;
static DCTInt2  A4b;
static DCTInt2  A5b;

static IDCTInt C2;
static IDCTInt C4;
static IDCTInt C6;
static IDCTInt Q;
static IDCTInt R;

static IDCTInt C2b;
static IDCTInt C4b;
static IDCTInt C6b;
static IDCTInt Qb;
static IDCTInt Rb;

static float OutputScaleFactor[8];  // fuer FDCT
static float InputScaleFactor[8];   // fuer IDCT

static long long d_fact[8][8];

static void Init()
{
  if (IsInitialized)
    return;

  /* Beachte: Jeder Wert wird exakt einmal zugewiesen und nicht veraendert, dadurch
     ist der Code Multithread-sicher. */

  // FDCT

  A1 = (DCTInt2)(sqrt(0.5)*FDCT_M1ScaleFact+0.5);
  MA2 = (DCTInt2)((-(cos(PI/8.0) - cos(3.0*PI/8.0)))*FDCT_M1ScaleFact-0.5);
  A3 = (DCTInt2)(sqrt(0.5)*FDCT_M1ScaleFact+0.5);
  A4 = (DCTInt2)((cos(PI/8.0) + cos(3.0*PI/8.0))*FDCT_M1ScaleFact+0.5);
  A5 = (DCTInt2)(cos(3.0*PI/8.0)*FDCT_M1ScaleFact+0.5);

  A1b = (DCTInt2)(sqrt(0.5)*FDCT_M2ScaleFact+0.5);
  MA2b = (DCTInt2)((-(cos(PI/8.0) - cos(3.0*PI/8.0)))*FDCT_M2ScaleFact-0.5);
  A3b = (DCTInt2)(sqrt(0.5)*FDCT_M2ScaleFact+0.5);
  A4b = (DCTInt2)((cos(PI/8.0) + cos(3.0*PI/8.0))*FDCT_M2ScaleFact+0.5);
  A5b = (DCTInt2)(cos(3.0*PI/8.0)*FDCT_M2ScaleFact+0.5);

  OutputScaleFactor[0] = sqrt(0.5)/2.0;
  {for (int i=1;i<8;i++) OutputScaleFactor[i] = 0.25/cos(PI*(float)i/16.0);}


  // IDCT

  C2  = (DCTInt2)(2.0*cos(PI/8.0)*IDCT_M2ScaleFact+0.5);
  C4  = (DCTInt2)(sqrt(2.0)*IDCT_M2ScaleFact+0.5);
  C6  = (DCTInt2)(2.0*cos(3.0*PI/8.0)*IDCT_M2ScaleFact+0.5);
  Q   = (DCTInt2)((-2.0*cos(PI/8.0) + 2.0*cos(3.0*PI/8.0))*IDCT_M2ScaleFact-0.5);
  R   = (DCTInt2)(( 2.0*cos(PI/8.0) + 2.0*cos(3.0*PI/8.0))*IDCT_M2ScaleFact+0.5);
  /*
    cout << "C2 " << 2.0*cos(PI/8.0) << endl;
    cout << "C4 " << sqrt(2.0) << endl;
    cout << "C6 " << 2.0*cos(3.0*PI/8.0) << endl;
    cout << "Q  " << (-2.0*cos(PI/8.0) + 2.0*cos(3.0*PI/8.0)) << endl;
    cout << "R  " << ( 2.0*cos(PI/8.0) + 2.0*cos(3.0*PI/8.0)) << endl;
  */
  C2b = (DCTInt2)(2.0*cos(PI/8.0)*IDCT_M1ScaleFact+0.5);
  C4b = (DCTInt2)(sqrt(2.0)*IDCT_M1ScaleFact+0.5);
  C6b = (DCTInt2)(2.0*cos(3.0*PI/8.0)*IDCT_M1ScaleFact+0.5);
  Qb  = (DCTInt2)((-2.0*cos(PI/8.0) + 2.0*cos(3.0*PI/8.0))*IDCT_M1ScaleFact-0.5);
  Rb  = (DCTInt2)(( 2.0*cos(PI/8.0) + 2.0*cos(3.0*PI/8.0))*IDCT_M1ScaleFact+0.5);
  
  InputScaleFactor[0] = 0.5*0.5*sqrt(2);
  {for (int i=1;i<8;i++) InputScaleFactor[i] = 0.5*cos(PI*((float)i)/16.0);}

  IsInitialized=true;


#if 1
  for (int u=0;u<8;u++)
    for (int v=0;v<8;v++)
      {
	d_fact[v][u] = (DCTInt2)(InputScaleFactor[u]*InputScaleFactor[v]*IDCT_M3ScaleFact+0.5);
      }
#else
  for (int u=0;u<8;u++)
    for (int v=0;v<8;v++)
      {
	d_fact[v][u] = (DCTInt2)(InputScaleFactor[u]*InputScaleFactor[v] * fact[v][u] *
                                 IDCT_M3ScaleFact+0.5);
      }
#endif





  {
    int i;
    
    iclp = iclip+512;
    for (i= -512; i<512; i++)
      iclp[i] = (i<-256) ? -256 : ((i>255) ? 255 : i);
  }
}


static struct DummyInit
{
  DummyInit() { Init(); }
} dummyinit;



void IDCT_Int(const short* in[8], short* out[8])
{
  IDCTInt fin[8][8];
  IDCTInt tmp[8][8];

  IDCTInt a0,a1,a2,a3,a4,a5,a6,a7;
  IDCTInt       b2,   b4,b5,b6   ;
  IDCTInt n0,n1,n2,n3            ;
  IDCTInt          m3,m4,m5,m6,m7;
   DCTInt2 tmp1,tmp2,tmp3,tmp4;


  // Zeilentransformationen

  {for (int u=0;u<8;u++)
    {
      if (             !in[1][u] && !in[2][u] && !in[3][u] &&
          !in[4][u] && !in[5][u] && !in[6][u] && !in[7][u])
        {
          tmp[0][u] = tmp[1][u] = tmp[2][u] = tmp[3][u] = 
          tmp[4][u] = tmp[5][u] = tmp[6][u] = tmp[7][u] = in[0][u]<<13;
        }
      else
        {
          {for (int v=0;v<8;v++)
            fin[v][u] = IDESCALE(MUL32(d_fact[v][u] , in[v][u]) , IDCT_PostM3_Shift);}

          a0 = fin[0][u];
          a1 = fin[4][u];
          a2 = fin[2][u]-fin[6][u];
          a3 = fin[2][u]+fin[6][u];
          a4 = fin[5][u]-fin[3][u];
          tmp1 = fin[1][u]+fin[7][u];
          tmp2 = fin[3][u]+fin[5][u];
          a5 = tmp1 - tmp2;
          a6 = fin[1][u]-fin[7][u];
          a7 = tmp1 + tmp2;
          
          tmp4 = MUL32(C6,(a4+a6));
          b2 = IDESCALE(MUL32(C4 , a2)       , IDCT_PostM2_Shift);
          b4 = IDESCALE(MUL32(Q  , a4) -tmp4 , IDCT_PostM2_Shift);
          b5 = IDESCALE(MUL32(C4 , a5)       , IDCT_PostM2_Shift);
          b6 = IDESCALE(MUL32(R  , a6) -tmp4 , IDCT_PostM2_Shift);

          a0 >>= -(IDCT_M2Scale - IDCT_PostM2_Shift);
          a1 >>= -(IDCT_M2Scale - IDCT_PostM2_Shift);
          a3 >>= -(IDCT_M2Scale - IDCT_PostM2_Shift);
          a7 >>= -(IDCT_M2Scale - IDCT_PostM2_Shift);
          
          tmp3 = b6-a7;
          n0 = tmp3-b5;
          n1 = a0-a1;
          n2 = b2-a3;
          n3 = a0+a1;
          
          m3 = n1+n2;
          m4 = n3+a3;
          m5 = n1-n2;
          m6 = n3-a3;
          m7 = b4-n0;
          
          tmp[0][u] = m4+a7;
          tmp[1][u] = m3+tmp3;
          tmp[2][u] = m5-n0;
          tmp[3][u] = m6-m7;
          tmp[4][u] = m6+m7;
          tmp[5][u] = m5+n0;
          tmp[6][u] = m3-tmp3;
          tmp[7][u] = m4-a7;
        }
    }}


  // Spaltentransformationen

  {for (int v=0;v<8;v++)
    {
      a0 = tmp[v][0];
      a1 = tmp[v][4];
      a2 = tmp[v][2]-tmp[v][6];
      a3 = tmp[v][2]+tmp[v][6];
      a4 = tmp[v][5]-tmp[v][3];
      tmp1 = tmp[v][1]+tmp[v][7];
      tmp2 = tmp[v][3]+tmp[v][5];
      a5 = tmp1 - tmp2;
      a6 = tmp[v][1]-tmp[v][7];
      a7 = tmp1 + tmp2;

      tmp4 = MUL32(C6,(a4+a6));
      b2 = IDESCALE(MUL32(C4 , a2)       , IDCT_PostM1_Shift);
      b4 = IDESCALE(MUL32(Q  , a4) -tmp4 , IDCT_PostM1_Shift);
      b5 = IDESCALE(MUL32(C4 , a5)       , IDCT_PostM1_Shift);
      b6 = IDESCALE(MUL32(R  , a6) -tmp4 , IDCT_PostM1_Shift);

      a0 >>= -(IDCT_M1Scale - IDCT_PostM1_Shift);
      a1 >>= -(IDCT_M1Scale - IDCT_PostM1_Shift);
      a3 >>= -(IDCT_M1Scale - IDCT_PostM1_Shift);
      a7 >>= -(IDCT_M1Scale - IDCT_PostM1_Shift);

      tmp3 = b6-a7;
      n0 = tmp3-b5;
      n1 = a0-a1;
      n2 = b2-a3;
      n3 = a0+a1;

      m3 = n1+n2;
      m4 = n3+a3;
      m5 = n1-n2;
      m6 = n3-a3;
      m7 = b4-n0;

      out[v][0] = IDESCALE( (m4+a7)   , IDCT_PostM1_NK);
      out[v][1] = IDESCALE( (m3+tmp3) , IDCT_PostM1_NK);
      out[v][2] = IDESCALE( (m5-n0)   , IDCT_PostM1_NK);
      out[v][3] = IDESCALE( (m6-m7)   , IDCT_PostM1_NK);
      out[v][4] = IDESCALE( (m6+m7)   , IDCT_PostM1_NK);
      out[v][5] = IDESCALE( (m5+n0)   , IDCT_PostM1_NK);
      out[v][6] = IDESCALE( (m3-tmp3) , IDCT_PostM1_NK);
      out[v][7] = IDESCALE( (m4-a7)   , IDCT_PostM1_NK);
    }}
}





//// --------------- The following IDCT-code originates from libJPEG ! -----------------

/**********************************************************/
/* inverse two dimensional DCT, Chen-Wang algorithm       */
/* (cf. IEEE ASSP-32, pp. 803-816, Aug. 1984)             */
/* 32-bit integer arithmetic (8 bit coefficients)         */
/* 11 mults, 29 adds per DCT                              */
/*                                      sE, 18.8.91       */
/**********************************************************/
/* coefficients extended to 12 bit for IEEE1180-1990      */
/* compliance                           sE,  2.1.94       */
/**********************************************************/

/* this code assumes >> to be a two's-complement arithmetic */
/* right shift: (-2)>>1 == -1 , (-3)>>1 == -2               */

// #include "config.h"

#define W1 2841 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565  /* 2048*sqrt(2)*cos(7*pi/16) */

/* global declarations */
//void Initialize_Fast_IDCT _ANSI_ARGS_((void));
//void Fast_IDCT _ANSI_ARGS_((short *block));

/* private prototypes */
//static void idctrow _ANSI_ARGS_((short *blk));
//static void idctcol _ANSI_ARGS_((short *blk));

/* row (horizontal) IDCT
 *
 *           7                       pi         1
 * dst[k] = sum c[l] * src[l] * cos( -- * ( k + - ) * l )
 *          l=0                      8          2
 *
 * where: c[0]    = 128
 *        c[1..7] = 128*sqrt(2)
 */

inline void idctrow(short *blk)
{
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  /* shortcut */
  if (!((x1 = blk[4]<<11) | (x2 = blk[6]) | (x3 = blk[2]) |
        (x4 = blk[1]) | (x5 = blk[7]) | (x6 = blk[5]) | (x7 = blk[3])))
  {
    blk[0]=blk[1]=blk[2]=blk[3]=blk[4]=blk[5]=blk[6]=blk[7]=blk[0]<<3;
    return;
  }

  x0 = (blk[0]<<11) + 128; /* for proper rounding in the fourth stage */

  /* first stage */
  x8 = W7*(x4+x5);
  x4 = x8 + (W1-W7)*x4;
  x5 = x8 - (W1+W7)*x5;
  x8 = W3*(x6+x7);
  x6 = x8 - (W3-W5)*x6;
  x7 = x8 - (W3+W5)*x7;
  
  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6*(x3+x2);
  x2 = x1 - (W2+W6)*x2;
  x3 = x1 + (W2-W6)*x3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  
  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181*(x4+x5)+128)>>8;
  x4 = (181*(x4-x5)+128)>>8;
  
  /* fourth stage */
  blk[0] = (x7+x1)>>8;
  blk[1] = (x3+x2)>>8;
  blk[2] = (x0+x4)>>8;
  blk[3] = (x8+x6)>>8;
  blk[4] = (x8-x6)>>8;
  blk[5] = (x0-x4)>>8;
  blk[6] = (x3-x2)>>8;
  blk[7] = (x7-x1)>>8;
}

/* column (vertical) IDCT
 *
 *             7                         pi         1
 * dst[8*k] = sum c[l] * src[8*l] * cos( -- * ( k + - ) * l )
 *            l=0                        8          2
 *
 * where: c[0]    = 1/1024
 *        c[1..7] = (1/1024)*sqrt(2)
 */
inline void idctcol2(short **in,int i,short** out)
{
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  /* shortcut */
  if (!((x1 = (in[4][i]<<8)) | (x2 = in[6][i]) | (x3 = in[2][i]) |
        (x4 = in[1][i]) | (x5 = in[7][i]) | (x6 = in[5][i]) | (x7 = in[3][i])))
  {
      out[0][i] = out[1][i] = out[2][i] = out[3][i] =
      out[4][i] = out[5][i] = out[6][i] = out[7][i] = (in[0][i]+32)>>6;  //iclp
    return;
  }

  x0 = (in[0][i]<<8) + 8192;

  /* first stage */
  x8 = W7*(x4+x5) + 4;
  x4 = (x8+(W1-W7)*x4)>>3;
  x5 = (x8-(W1+W7)*x5)>>3;
  x8 = W3*(x6+x7) + 4;
  x6 = (x8-(W3-W5)*x6)>>3;
  x7 = (x8-(W3+W5)*x7)>>3;
  
  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6*(x3+x2) + 4;
  x2 = (x1-(W2+W6)*x2)>>3;
  x3 = (x1+(W2-W6)*x3)>>3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  
  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181*(x4+x5)+128)>>8;
  x4 = (181*(x4-x5)+128)>>8;
  
  /* fourth stage */
  out[0][i] = (x7+x1)>>14;
  out[1][i] = (x3+x2)>>14;
  out[2][i] = (x0+x4)>>14;
  out[3][i] = (x8+x6)>>14;
  out[4][i] = (x8-x6)>>14;
  out[5][i] = (x0-x4)>>14;
  out[6][i] = (x3-x2)>>14;
  out[7][i] = (x7-x1)>>14;
}

inline void idctcol2b(short *in,int i,Pixel** out,bool add)
{
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  /* shortcut */
  if (!((x1 = (in[4*8]<<8)) | (x2 = in[6*8]) | (x3 = in[2*8]) |
        (x4 = in[1*8]) | (x5 = in[7*8]) | (x6 = in[5*8]) | (x7 = in[3*8])))
  {
    if (add)
      {
#if 0
        out[0][i] = clip2_0_255[ out[0][i] + ((in[0][i]+32)>>6)];
        out[1][i] = clip2_0_255[ out[1][i] + ((in[1][i]+32)>>6)];
        out[2][i] = clip2_0_255[ out[2][i] + ((in[2][i]+32)>>6)];
        out[3][i] = clip2_0_255[ out[3][i] + ((in[3][i]+32)>>6)];
        out[4][i] = clip2_0_255[ out[4][i] + ((in[4][i]+32)>>6)];
        out[5][i] = clip2_0_255[ out[5][i] + ((in[5][i]+32)>>6)];
        out[6][i] = clip2_0_255[ out[6][i] + ((in[6][i]+32)>>6)];
        out[7][i] = clip2_0_255[ out[7][i] + ((in[7][i]+32)>>6)];
        return;
#endif
      }
    else
      {
#if 1
        out[0][i] = out[1][i] = out[2][i] = out[3][i] =
        out[4][i] = out[5][i] = out[6][i] = out[7][i] = clip2_0_255[(in[0*8]+32)>>6];  //iclp
        return;
#endif
      }
  }

  x0 = (in[0*8]<<8) + 8192;

  /* first stage */
  x8 = W7*(x4+x5) + 4;
  x4 = (x8+(W1-W7)*x4)>>3;
  x5 = (x8-(W1+W7)*x5)>>3;
  x8 = W3*(x6+x7) + 4;
  x6 = (x8-(W3-W5)*x6)>>3;
  x7 = (x8-(W3+W5)*x7)>>3;
  
  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = W6*(x3+x2) + 4;
  x2 = (x1-(W2+W6)*x2)>>3;
  x3 = (x1+(W2-W6)*x3)>>3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;
  
  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181*(x4+x5)+128)>>8;
  x4 = (181*(x4-x5)+128)>>8;
  
  /* fourth stage */

  if (add)
    {
      out[0][i] = clip2_0_255[ out[0][i] + ((x7+x1)>>14) ];
      out[7][i] = clip2_0_255[ out[7][i] + ((x7-x1)>>14) ];
      out[1][i] = clip2_0_255[ out[1][i] + ((x3+x2)>>14) ];
      out[6][i] = clip2_0_255[ out[6][i] + ((x3-x2)>>14) ];
      out[2][i] = clip2_0_255[ out[2][i] + ((x0+x4)>>14) ];
      out[5][i] = clip2_0_255[ out[5][i] + ((x0-x4)>>14) ];
      out[3][i] = clip2_0_255[ out[3][i] + ((x8+x6)>>14) ];
      out[4][i] = clip2_0_255[ out[4][i] + ((x8-x6)>>14) ];
    }
  else
    {
      out[0][i] = clip2_0_255[ ((x7+x1)>>14) ];
      out[7][i] = clip2_0_255[ ((x7-x1)>>14) ];
      out[1][i] = clip2_0_255[ ((x3+x2)>>14) ];
      out[6][i] = clip2_0_255[ ((x3-x2)>>14) ];
      out[2][i] = clip2_0_255[ ((x0+x4)>>14) ];
      out[5][i] = clip2_0_255[ ((x0-x4)>>14) ];
      out[3][i] = clip2_0_255[ ((x8+x6)>>14) ];
      out[4][i] = clip2_0_255[ ((x8-x6)>>14) ];
    }
}

/* two dimensional inverse discrete cosine transform */
void IDCT_Int2(short* in[8], short* out[8])
{
  int i;

  for (i=0; i<8; i++)
    idctrow(in[i]);

  for (i=0; i<8; i++)
    idctcol2(in,i,out);
}


void IDCT_Int2b(short* in, Pixel* out[8],bool add)
{
  int i;

  for (i=0; i<8; i++)
    idctrow(in+8*i);

  for (i=0; i<8; i++)
    idctcol2b(in+i,i,out,add);
}
