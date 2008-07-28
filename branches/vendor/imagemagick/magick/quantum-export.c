/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                QQQ   U   U   AAA   N   N  TTTTT  U   U  M   M               %
%               Q   Q  U   U  A   A  NN  N    T    U   U  MM MM               %
%               Q   Q  U   U  AAAAA  N N N    T    U   U  M M M               %
%               Q  QQ  U   U  A   A  N  NN    T    U   U  M   M               %
%                QQQQ   UUU   A   A  N   N    T     UUU   M   M               %
%                                                                             %
%                   EEEEE  X   X  PPPP    OOO   RRRR   TTTTT                  %
%                   E       X X   P   P  O   O  R   R    T                    %
%                   EEE      X    PPPP   O   O  RRRR     T                    %
%                   E       X X   P      O   O  R R      T                    %
%                   EEEEE  X   X  P       OOO   R  R     T                    %
%                                                                             %
%                      Methods to Export Quantum Pixels                       %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                               October 1998                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2008 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/color-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/cache.h"
#include "magick/constitute.h"
#include "magick/delegate.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/option.h"
#include "magick/pixel.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/semaphore.h"
#include "magick/statistic.h"
#include "magick/stream.h"
#include "magick/string_.h"
#include "magick/utility.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   E x p o r t Q u a n t u m P i x e l s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ExportQuantumPixels() transfers one or more pixel components from the image
%  pixel cache to a user supplied buffer.  The pixels are returned in network
%  byte order.  MagickTrue is returned if the pixels are successfully
%  transferred, otherwise MagickFalse.
%
%  The format of the ExportQuantumPixels method is:
%
%      size_t ExportQuantumPixels(Image *image,const QuantumInfo *quantum_info,
%        const QuantumType quantum_type,unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum_info: the quantum info.
%
%    o quantum_type: Declare which pixel components to transfer (RGB, RGBA,
%      etc).
%
%    o pixels:  The components are transferred to this buffer.
%
*/

static inline unsigned char *PopDoublePixel(const QuantumState *quantum_state,
  const double pixel,unsigned char *pixels)
{
  unsigned char
    quantum[8];

  *((double *) quantum)=(double) (pixel*quantum_state->inverse_scale+
    quantum_state->minimum); 
  if (quantum_state->endian != LSBEndian)
    {
      *pixels++=quantum[7];
      *pixels++=quantum[6];
      *pixels++=quantum[5];
      *pixels++=quantum[4];
      *pixels++=quantum[3];
      *pixels++=quantum[2];
      *pixels++=quantum[1];
      *pixels++=quantum[0];
      return(pixels);
    }
  *pixels++=quantum[0];
  *pixels++=quantum[1];
  *pixels++=quantum[2];
  *pixels++=quantum[3];
  *pixels++=quantum[4];
  *pixels++=quantum[5];
  *pixels++=quantum[6];
  *pixels++=quantum[7];
  return(pixels);
}

static inline unsigned char *PopFloatPixel(const QuantumState *quantum_state,
  const float pixel,unsigned char *pixels)
{
  unsigned char
    quantum[4];

  *((float *) quantum)=(float) ((double) pixel*quantum_state->inverse_scale+
    quantum_state->minimum); 
  if (quantum_state->endian != LSBEndian)
    {
      *pixels++=quantum[3];
      *pixels++=quantum[2];
      *pixels++=quantum[1];
      *pixels++=quantum[0];
      return(pixels);
    }
  *pixels++=quantum[0];
  *pixels++=quantum[1];
  *pixels++=quantum[2];
  *pixels++=quantum[3];
  return(pixels);
}

static inline unsigned char *PopQuantumPixel(QuantumState *quantum_state,
  const unsigned long depth,const QuantumAny pixel,unsigned char *pixels)
{
  register long
    i;

  register unsigned long
    quantum_bits;

  if (quantum_state->bits == 0UL)
    quantum_state->bits=8UL;
  for (i=(long) depth; i > 0L; )
  {
    quantum_bits=(unsigned long) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    i-=quantum_bits;
    if (quantum_state->bits == 8)
      *pixels='\0';
    quantum_state->bits-=quantum_bits;
    *pixels|=(((pixel >> i) &~ ((~0UL) << quantum_bits)) <<
      quantum_state->bits);
    if (quantum_state->bits == 0UL)
      {
        pixels++;
        quantum_state->bits=8UL;
      }
  }
  return(pixels);
}

static inline unsigned char *PopQuantumLongPixel(QuantumState *quantum_state,
  const unsigned long depth,const unsigned long pixel,unsigned char *pixels)
{
  register long
    i;

  unsigned long
    quantum_bits;

  if (quantum_state->bits == 0UL)
    quantum_state->bits=32UL;
  for (i=(long) depth; i > 0; )
  {
    quantum_bits=(unsigned long) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    quantum_state->pixel|=(((pixel >> (depth-i)) &
      quantum_state->mask[quantum_bits]) << (32UL-quantum_state->bits));
    i-=quantum_bits;
    quantum_state->bits-=quantum_bits;
    if (quantum_state->bits == 0U)
      {
        pixels=PopLongPixel(quantum_state->endian,quantum_state->pixel,pixels);
        quantum_state->pixel=0U;
        quantum_state->bits=32UL;
      }
  }
  return(pixels);
}

MagickExport size_t ExportQuantumPixels(Image *image,
  const QuantumInfo *quantum_info,const QuantumType quantum_type,
  unsigned char *pixels)
{
  EndianType
    endian;

  long
    bit;

  MagickRealType
    alpha;

  MagickSizeType
    number_pixels;

  QuantumAny
    scale;

  QuantumState
    quantum_state;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    x;

  register unsigned char
    *q;

  size_t
    extent;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  if (pixels == (unsigned char *) NULL)
    pixels=quantum_info->pixels;
  number_pixels=GetPixelCacheExtent(image);
  x=0;
  p=GetPixels(image);
  if (quantum_info->alpha_type == AssociatedQuantumAlpha)
    {
      register PixelPacket
        *q;

      /*
        Associate alpha.
      */
      q=GetPixels(image);
      for (x=0; x < (long) image->columns; x++)
      {
        alpha=QuantumScale*((double) QuantumRange-q->opacity);
        q->red=RoundToQuantum(alpha*q->red);
        q->green=RoundToQuantum(alpha*q->green);
        q->blue=RoundToQuantum(alpha*q->blue);
        q++;
      }
    }
  indexes=GetIndexes(image);
  q=pixels;
  InitializeQuantumState(quantum_info,image->endian,&quantum_state);
  extent=GetQuantumExtent(image,quantum_info,quantum_type);
  endian=quantum_state.endian;
  switch (quantum_type)
  {
    case IndexQuantum:
    {
      if (image->storage_class != PseudoClass)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ImageError,"ColormappedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=((long) number_pixels-7); x > 0; x-=8)
          {
            pixel=(unsigned char) *indexes++;
            *q=((pixel & 0x01) << 7);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 6);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 5);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 4);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 3);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 2);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 1);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 0);
            q++;
          }
          if ((number_pixels % 8) != 0)
            {
              *q='\0';
              for (bit=7; bit >= (long) (8-(number_pixels % 8)); bit--)
              {
                pixel=(unsigned char) *indexes++;
                *q|=((pixel & 0x01) << (unsigned char) bit);
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) (number_pixels-1) ; x+=2)
          {
            pixel=(unsigned char) *indexes++;
            *q=((pixel & 0xf) << 4);
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0xf) << 0);
            q++;
          }
          if ((number_pixels % 2) != 0)
            {
              pixel=(unsigned char) *indexes++;
              *q=((pixel & 0xf) << 4);
              q++;
            }
          break;
        }
        case 8:
        {
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopCharPixel((unsigned char) indexes[x],q);
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopShortPixel(endian,(unsigned short) indexes[x],q);
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopLongPixel(endian,(unsigned long) indexes[x],q);
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,indexes[x],q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case IndexAlphaQuantum:
    {
      if (image->storage_class != PseudoClass)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ImageError,"ColormappedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=((long) number_pixels-3); x > 0; x-=4)
          {
            pixel=(unsigned char) *indexes++;
            *q=((pixel & 0x01) << 7);
            pixel=(unsigned char) (p->opacity == (Quantum) TransparentOpacity ?
              1 : 0);
            *q|=((pixel & 0x01) << 6);
            p++;
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 5);
            pixel=(unsigned char) (p->opacity == (Quantum) TransparentOpacity ?
              1 : 0);
            *q|=((pixel & 0x01) << 4);
            p++;
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 3);
            pixel=(unsigned char) (p->opacity == (Quantum) TransparentOpacity ?
              1 : 0);
            *q|=((pixel & 0x01) << 2);
            p++;
            pixel=(unsigned char) *indexes++;
            *q|=((pixel & 0x01) << 1);
            pixel=(unsigned char) (p->opacity == (Quantum) TransparentOpacity ?
              1 : 0);
            *q|=((pixel & 0x01) << 0);
            p++;
            q++;
          }
          if ((number_pixels % 4) != 0)
            {
              *q='\0';
              for (bit=3; bit >= (long) (4-(number_pixels % 4)); bit-=2)
              {
                pixel=(unsigned char) *indexes++;
                *q|=((pixel & 0x01) << (unsigned char) bit);
                pixel=(unsigned char) (p->opacity == (Quantum)
                  TransparentOpacity ? 1 : 0);
                *q|=((pixel & 0x01) << (unsigned char) (bit-1));
                p++;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels ; x++)
          {
            pixel=(unsigned char) *indexes++;
            *q=((pixel & 0xf) << 4);
            pixel=(unsigned char) (16*QuantumScale*((Quantum) (QuantumRange-
              p->opacity))+0.5);
            *q|=((pixel & 0xf) << 0);
            p++;
            q++;
          }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopCharPixel((unsigned char) indexes[x],q);
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-p->opacity));
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopShortPixel(endian,(unsigned short) indexes[x],q);
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-p->opacity));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                pixel=(float)  (QuantumRange-p->opacity);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopLongPixel(endian,(unsigned long) indexes[x],q);
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-p->opacity));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                double
                  pixel;

                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                pixel=(double) (QuantumRange-p->opacity);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,indexes[x],q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) (QuantumRange-p->opacity),image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case GrayQuantum:
    {
      switch (quantum_info->depth)
      {
        case 1:
        {
          unsigned char
            black,
            white;
            
          black=0x00;
          white=0x01;
          if (quantum_info->min_is_white != MagickFalse)
            {
              black=0x01;
              white=0x00;
            }
          for (x=((long) number_pixels-7); x > 0; x-=8)
          {
            *q='\0';
            *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
              white) << 7;
            p++;
            *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
              white) << 6;
            p++;
            *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
              white) << 5;
            p++;
            *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
              white) << 4;
            p++;
            *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
              white) << 3;
            p++;
            *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
              white) << 2;
            p++;
            *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
              white) << 1;
            p++;
            *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
              white) << 0;
            p++;
            q++;
          }
          if ((number_pixels % 8) != 0)
            {
              *q='\0';
              for (bit=7; bit >= (long) (8-(number_pixels % 8)); bit--)
              {
                *q|=(PixelIntensityToQuantum(p) < (QuantumRange/2) ? black :
                  white) << bit;
                p++;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) (number_pixels-1) ; x+=2)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q=(((pixel >> 4) & 0xf) << 4);
            p++;
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q|=pixel >> 4;
            p++;
            q++;
          }
          if ((number_pixels % 2) != 0)
            {
              pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
              *q=(((pixel >> 4) & 0xf) << 4);
              p++;
              q++;
            }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          register unsigned short
            pixel;

          scale=GetQuantumScale(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=ScaleQuantumToShort(PixelIntensityToQuantum(p));
                q=PopShortPixel(endian,(unsigned short) ScaleQuantumToAny(
                  (Quantum) pixel,image->depth,scale),q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              PixelIntensityToQuantum(p),image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 12:
        {
          register unsigned short
            pixel;

          scale=GetQuantumScale(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=ScaleQuantumToShort(PixelIntensityToQuantum(p));
                q=PopShortPixel(endian,(unsigned short) (pixel >> 4),q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              PixelIntensityToQuantum(p),image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(PixelIntensityToQuantum(p));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) PixelIntensityToQuantum(p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(PixelIntensityToQuantum(p));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) PixelIntensityToQuantum(p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              PixelIntensityToQuantum(p),image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case GrayAlphaQuantum:
    {
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=((long) number_pixels-3); x > 0; x-=4)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q=(unsigned char) (((int) pixel != 0 ? 0x00 : 0x01) << 7);
            pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 6);
            p++;
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 5);
            pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 4);
            p++;
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 3);
            pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 2);
            p++;
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 1);
            pixel=(unsigned char) (p->opacity == OpaqueOpacity ? 0x00 : 0x01);
            *q|=(((int) pixel != 0 ? 0x00 : 0x01) << 0);
            p++;
            q++;
          }
          if ((number_pixels % 4) != 0)
            {
              *q='\0';
              for (bit=3; bit >= (long) (4-(number_pixels % 4)); bit-=2)
              {
                pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
                *q|=(((int) pixel != 0 ? 0x00 : 0x01) << (unsigned char) bit);
                pixel=(unsigned char) (p->opacity == OpaqueOpacity ?
                  0x00 : 0x01);
                *q|=(((int) pixel != 0 ? 0x00 : 0x01) <<
                  (unsigned char) (bit-1));
                p++;
              }
              q++;
            }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels ; x++)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            *q=(((pixel >> 4) & 0xf) << 4);
            pixel=(unsigned char) (16*QuantumScale*((Quantum) (QuantumRange-
              p->opacity))+0.5);
            *q|=pixel & 0xf;
            p++;
            q++;
          }
          break;
        }
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(PixelIntensityToQuantum(p));
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-p->opacity));
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(PixelIntensityToQuantum(p));
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-p->opacity));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) PixelIntensityToQuantum(p);
                q=PopFloatPixel(&quantum_state,pixel,q);
                pixel=(float) (QuantumRange-p->opacity);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(PixelIntensityToQuantum(p));
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-p->opacity));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) PixelIntensityToQuantum(p);
                q=PopDoublePixel(&quantum_state,pixel,q);
                pixel=(double) (QuantumRange-p->opacity);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              PixelIntensityToQuantum(p),image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) (QuantumRange-p->opacity),image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case RedQuantum:
    case CyanQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(p->red);
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(p->red);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(p->red);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case GreenQuantum:
    case MagentaQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(p->green);
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(p->green);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(p->green);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case BlueQuantum:
    case YellowQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(p->blue);
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(p->blue);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(p->blue);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case AlphaQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-p->opacity));
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-p->opacity));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                pixel=(float) (QuantumRange-p->opacity);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-p->opacity));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                double
                  pixel;

                pixel=(double) (QuantumRange-p->opacity);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) (QuantumRange-p->opacity),image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case BlackQuantum:
    {
      if (image->colorspace != CMYKColorspace)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ImageError,"ColorSeparatedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(indexes[x]);
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(indexes[x]);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(indexes[x]);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) indexes[x],image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case RGBQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopCharPixel(ScaleQuantumToChar(p->red),q);
            q=PopCharPixel(ScaleQuantumToChar(p->green),q);
            q=PopCharPixel(ScaleQuantumToChar(p->blue),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 10:
        {
          register unsigned long
            pixel;

          scale=GetQuantumScale(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=ScaleQuantumToAny(p->red,image->depth,scale) << 22 |
                  ScaleQuantumToAny(p->green,image->depth,scale) <<  12 |
                  ScaleQuantumToAny(p->blue,image->depth,scale) << 2;
                q=PopLongPixel(endian,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=ScaleQuantumToAny(p->red,image->depth,scale);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=ScaleQuantumToAny(p->green,image->depth,scale);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=ScaleQuantumToAny(p->blue,image->depth,scale);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToAny(p->red,image->depth,scale);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=ScaleQuantumToAny(p->green,image->depth,scale);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=ScaleQuantumToAny(p->blue,image->depth,scale);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 12:
        {
          register unsigned long
            pixel;

          scale=GetQuantumScale(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) (3*number_pixels-1); x+=2)
              {
                switch (x % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=ScaleQuantumToAny(p->red,image->depth,scale);
                    break;
                  }
                  case 1:
                  {
                    pixel=ScaleQuantumToAny(p->green,image->depth,scale);
                    break;
                  }
                  case 2:
                  {
                    pixel=ScaleQuantumToAny(p->blue,image->depth,scale);
                    p++;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                switch ((x+1) % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=ScaleQuantumToAny(p->red,image->depth,scale);
                    break;
                  }
                  case 1:
                  {
                    pixel=ScaleQuantumToAny(p->green,image->depth,scale);
                    break;
                  }
                  case 2:
                  {
                    pixel=ScaleQuantumToAny(p->blue,image->depth,scale);
                    p++;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                q+=quantum_info->pad;
              }
              for (bit=0; bit < (long) (3*number_pixels % 2); bit++)
              {
                switch ((x+bit) % 3)
                {
                  default:
                  case 0:
                  {
                    pixel=ScaleQuantumToAny(p->red,image->depth,scale);
                    break;
                  }
                  case 1:
                  {
                    pixel=ScaleQuantumToAny(p->green,image->depth,scale);
                    break;
                  }
                  case 2:
                  {
                    pixel=ScaleQuantumToAny(p->blue,image->depth,scale);
                    p++;
                    break;
                  }
                }
                q=PopShortPixel(endian,(unsigned short) (pixel << 4),q);
                q+=quantum_info->pad;
              }
              if (bit != 0)
                p++;
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                pixel=ScaleQuantumToAny(p->red,image->depth,scale);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=ScaleQuantumToAny(p->green,image->depth,scale);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                pixel=ScaleQuantumToAny(p->blue,image->depth,scale);
                q=PopQuantumLongPixel(&quantum_state,image->depth,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToAny(p->red,image->depth,scale);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=ScaleQuantumToAny(p->green,image->depth,scale);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            pixel=ScaleQuantumToAny(p->blue,image->depth,scale);
            q=PopQuantumPixel(&quantum_state,image->depth,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(p->red);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->green);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->blue);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(p->red);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->green);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->blue);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case RGBAQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(p->red);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->green);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->blue);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-p->opacity));
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(p->red);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->green);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->blue);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-p->opacity));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                pixel=(float) (QuantumRange-p->opacity);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(p->red);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->green);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->blue);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-p->opacity));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                pixel=(double) (QuantumRange-p->opacity);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              (Quantum) (QuantumRange-p->opacity),image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case RGBOQuantum:
    {
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(p->red);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->green);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->blue);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->opacity);
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(p->red);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->green);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->blue);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->opacity);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                q=PopFloatPixel(&quantum_state,(float) p->opacity,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(p->red);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->green);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->blue);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->opacity);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                q=PopDoublePixel(&quantum_state,(double) p->opacity,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->opacity,image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case CMYKQuantum:
    {
      if (image->colorspace != CMYKColorspace)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ImageError,"ColorSeparatedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(p->red);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->green);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->blue);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(indexes[x]);
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(p->red);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->green);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->blue);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(indexes[x]);
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(p->red);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->green);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->blue);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(indexes[x]);
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              indexes[x],image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    case CMYKAQuantum:
    {
      if (image->colorspace != CMYKColorspace)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ImageError,"ColorSeparatedImageRequired","`%s'",image->filename);
          return(extent);
        }
      switch (quantum_info->depth)
      {
        case 8:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToChar(p->red);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->green);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(p->blue);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar(indexes[x]);
            q=PopCharPixel(pixel,q);
            pixel=ScaleQuantumToChar((Quantum) (QuantumRange-p->opacity));
            q=PopCharPixel(pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          register unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToShort(p->red);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->green);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(p->blue);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort(indexes[x]);
            q=PopShortPixel(endian,pixel,q);
            pixel=ScaleQuantumToShort((Quantum) (QuantumRange-p->opacity));
            q=PopShortPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          register unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                float
                  pixel;

                q=PopFloatPixel(&quantum_state,(float) p->red,q);
                q=PopFloatPixel(&quantum_state,(float) p->green,q);
                q=PopFloatPixel(&quantum_state,(float) p->blue,q);
                q=PopFloatPixel(&quantum_state,(float) indexes[x],q);
                pixel=(float) (QuantumRange-p->opacity);
                q=PopFloatPixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=ScaleQuantumToLong(p->red);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->green);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(p->blue);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong(indexes[x]);
            q=PopLongPixel(endian,pixel,q);
            pixel=ScaleQuantumToLong((Quantum) (QuantumRange-p->opacity));
            q=PopLongPixel(endian,pixel,q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
        case 64:
        {
          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              double
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                q=PopDoublePixel(&quantum_state,(double) p->red,q);
                q=PopDoublePixel(&quantum_state,(double) p->green,q);
                q=PopDoublePixel(&quantum_state,(double) p->blue,q);
                q=PopDoublePixel(&quantum_state,(double) indexes[x],q);
                pixel=(double) (QuantumRange-p->opacity);
                q=PopDoublePixel(&quantum_state,pixel,q);
                p++;
                q+=quantum_info->pad;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->red,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->green,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->blue,image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              indexes[x],image->depth,scale),q);
            q=PopQuantumPixel(&quantum_state,image->depth,ScaleQuantumToAny(
              p->opacity,image->depth,scale),q);
            p++;
            q+=quantum_info->pad;
          }
          break;
        }
      }
      break;
    }
    default:
      break;
  }
  return(extent);
}
