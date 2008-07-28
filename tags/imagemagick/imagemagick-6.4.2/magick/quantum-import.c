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
%                   IIIII  M   M  PPPP    OOO   RRRR   TTTTT                  %
%                     I    MM MM  P   P  O   O  R   R    T                    %
%                     I    M M M  PPPP   O   O  RRRR     T                    %
%                     I    M   M  P      O   O  R R      T                    %
%                   IIIII  M   M  P       OOO   R  R     T                    %
%                                                                             %
%                       Methods to Import Quantum Pixels                      %
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
%   I m p o r t Q u a n t u m P i x e l s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImportQuantumPixels() transfers one or more pixel components from a user
%  supplied buffer into the image pixel cache of an image.  The pixels are
%  expected in network byte order.  It returns MagickTrue if the pixels are
%  successfully transferred, otherwise MagickFalse.
%
%  The format of the ImportQuantumPixels method is:
%
%      size_t ImportQuantumPixels(Image *image,const QuantumInfo *quantum_info,
%        const QuantumType quantum_type,const unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum_info: the quantum info.
%
%    o quantum_type: Declare which pixel components to transfer (red, green,
%      blue, opacity, RGB, or RGBA).
%
%    o pixels:  The pixel components are transferred from this buffer.
%
*/

static inline IndexPacket PushColormapIndex(Image *image,
  const unsigned long index,MagickBooleanType *range_exception)
{
  if (index < image->colors)
    return((IndexPacket) index);
  *range_exception=MagickTrue;
  return((IndexPacket) 0);
}

static inline const unsigned char *PushDoublePixel(
  const QuantumState *quantum_state,const unsigned char *pixels,double *pixel)
{
  unsigned char
    quantum[8];

  if (quantum_state->endian != LSBEndian)
    {
      quantum[7]=(*pixels++);
      quantum[6]=(*pixels++);
      quantum[5]=(*pixels++);
      quantum[5]=(*pixels++);
      quantum[3]=(*pixels++);
      quantum[2]=(*pixels++);
      quantum[1]=(*pixels++);
      quantum[0]=(*pixels++);
      *pixel=(*((double *) quantum));
      *pixel-=quantum_state->minimum;
      *pixel*=quantum_state->scale;
      return(pixels);
    }
  quantum[0]=(*pixels++);
  quantum[1]=(*pixels++);
  quantum[2]=(*pixels++);
  quantum[3]=(*pixels++);
  quantum[4]=(*pixels++);
  quantum[5]=(*pixels++);
  quantum[6]=(*pixels++);
  quantum[7]=(*pixels++);
  *pixel=(*((double *) quantum));
  *pixel-=quantum_state->minimum;
  *pixel*=quantum_state->scale;
  return(pixels);
}

static inline const unsigned char *PushFloatPixel(
  const QuantumState *quantum_state,const unsigned char *pixels,float *pixel)
{
  unsigned char
    quantum[4];

  if (quantum_state->endian != LSBEndian)
    {
      quantum[3]=(*pixels++);
      quantum[2]=(*pixels++);
      quantum[1]=(*pixels++);
      quantum[0]=(*pixels++);
      *pixel=(*((float *) quantum));
      *pixel-=quantum_state->minimum;
      *pixel*=quantum_state->scale;
      return(pixels);
    }
  quantum[0]=(*pixels++);
  quantum[1]=(*pixels++);
  quantum[2]=(*pixels++);
  quantum[3]=(*pixels++);
  *pixel=(*((float *) quantum));
  *pixel-=quantum_state->minimum;
  *pixel*=quantum_state->scale;
  return(pixels);
}

static inline const unsigned char *PushQuantumPixel(QuantumState *quantum_state,
  const unsigned long depth,const unsigned char *pixels,QuantumAny *quantum)
{
  register long
    i;

  register unsigned long
    quantum_bits;

  *quantum=(QuantumAny) 0;
  for (i=(long) depth; i > 0L; )
  {
    if (quantum_state->bits == 0UL)
      {
        quantum_state->pixel=(*pixels++);
        quantum_state->bits=8UL;
      }
    quantum_bits=(unsigned long) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    i-=quantum_bits;
    quantum_state->bits-=quantum_bits;
    *quantum=(*quantum << quantum_bits) | ((quantum_state->pixel >>
      quantum_state->bits) &~ ((~0UL) << quantum_bits));
  }
  return(pixels);
}

static inline const unsigned char *PushQuantumLongPixel(
  QuantumState *quantum_state,const unsigned long depth,
  const unsigned char *pixels,unsigned long *quantum)
{
  register long
    i;

  register unsigned long
    quantum_bits;

  *quantum=0UL;
  for (i=(long) depth; i > 0; )
  {
    if (quantum_state->bits == 0)
      {
        pixels=PushLongPixel(quantum_state->endian,pixels,
          &quantum_state->pixel);
        quantum_state->bits=32UL;
      }
    quantum_bits=(unsigned long) i;
    if (quantum_bits > quantum_state->bits)
      quantum_bits=quantum_state->bits;
    *quantum|=(((quantum_state->pixel >> (32UL-quantum_state->bits)) &
      quantum_state->mask[quantum_bits]) << (depth-i));
    i-=quantum_bits;
    quantum_state->bits-=quantum_bits;
  }
  return(pixels);
}

MagickExport size_t ImportQuantumPixels(Image *image,
  const QuantumInfo *quantum_info,const QuantumType quantum_type,
  const unsigned char *pixels)
{
  EndianType
    endian;

  long
    bit;

  MagickSizeType
    number_pixels;

  QuantumAny
    pixel,
    scale;

  QuantumState
    quantum_state;

  register const unsigned char
    *p;

  register IndexPacket
    *indexes;

  register long
    x;

  register PixelPacket
    *q;

  size_t
    extent;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  if (pixels == (const unsigned char *) NULL)
    pixels=quantum_info->pixels;
  number_pixels=GetPixelCacheExtent(image);
  x=0;
  p=pixels;
  q=GetPixels(image);
  indexes=GetIndexes(image);
  InitializeQuantumState(quantum_info,image->endian,&quantum_state);
  extent=GetQuantumExtent(image,quantum_info,quantum_type);
  endian=quantum_state.endian;
  switch (quantum_type)
  {
    case IndexQuantum:
    {
      MagickBooleanType
        range_exception;

      if (image->storage_class != PseudoClass)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ImageError,"ColormappedImageRequired","`%s'",image->filename);
          return(extent);
        }
      range_exception=MagickFalse;
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=0; x < ((long) number_pixels-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              if (quantum_info->min_is_white == MagickFalse)
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ?
                  0x00 : 0x01);
              else
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ?
                  0x00 : 0x01);
              indexes[x+bit]=PushColormapIndex(image,pixel,&range_exception);
              *q=image->colormap[(long) indexes[x+bit]];
              q++;
            }
            p++;
          }
          for (bit=0; bit < (long) (number_pixels % 8); bit++)
          {
            if (quantum_info->min_is_white == MagickFalse)
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ?
                0x00 : 0x01);
            else
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ?
                0x00 : 0x01);
            indexes[x+bit]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x+bit]];
            q++;
          }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < ((long) number_pixels-1); x+=2)
          {
            pixel=(unsigned char) ((*p >> 4) & 0xf);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            q++;
            pixel=(unsigned char) ((*p) & 0xf);
            indexes[x+1]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x+1]];
            p++;
            q++;
          }
          for (bit=0; bit < (long) (number_pixels % 2); bit++)
          {
            pixel=(unsigned char) ((*p++ >> 4) & 0xf);
            indexes[x+bit]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x+bit]];
            q++;
          }
          break;
        }
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                indexes[x]=PushColormapIndex(image,RoundToQuantum(pixel),
                  &range_exception);
                *q=image->colormap[(long) indexes[x]];
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                indexes[x]=PushColormapIndex(image,RoundToQuantum(pixel),
                  &range_exception);
                *q=image->colormap[(long) indexes[x]];
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      if (range_exception != MagickFalse)
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          CorruptImageError,"InvalidColormapIndex","`%s'",image->filename);
      break;
    }
    case IndexAlphaQuantum:
    {
      MagickBooleanType
        range_exception;

      if (image->storage_class != PseudoClass)
        {
          (void) ThrowMagickException(&image->exception,GetMagickModule(),
            ImageError,"ColormappedImageRequired","`%s'",image->filename);
          return(extent);
        }
      range_exception=MagickFalse;
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=0; x < ((long) number_pixels-3); x+=4)
          {
            for (bit=0; bit < 8; bit+=2)
            {
              if (quantum_info->min_is_white == MagickFalse)
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ?
                  0x00 : 0x01);
              else
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ?
                  0x00 : 0x01);
              indexes[x+bit/2]=(IndexPacket) (pixel == 0 ? 0 : 1);
              q->red=(Quantum) (pixel == 0 ? 0 : QuantumRange);
              q->green=q->red;
              q->blue=q->red;
              q->opacity=(Quantum) (((*p) & (1UL << (unsigned char) (6-bit)))
                == 0 ? TransparentOpacity : OpaqueOpacity);
              q++;
            }
          }
          for (bit=0; bit < (long) (number_pixels % 4); bit+=2)
          {
            if (quantum_info->min_is_white == MagickFalse)
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ?
                0x00 : 0x01);
            else
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ?
                0x00 : 0x01);
            indexes[x+bit/2]=(IndexPacket) (pixel == 0 ? 0 : 1);
            q->red=(Quantum) (pixel == 0 ? 0 : QuantumRange);
            q->green=q->red;
            q->blue=q->red;
            q->opacity=(Quantum) (((*p) & (1UL << (unsigned char) (6-bit))) ==
              0 ? TransparentOpacity : OpaqueOpacity);
            q++;
          }
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=(unsigned char) ((*p >> 4) & 0xf);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            q->opacity=(Quantum) (QuantumRange-(QuantumRange*((int) (*p) &
              0xf)/15));
            p++;
            q++;
          }
          break;
        }
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            p=PushCharPixel(p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            p=PushShortPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                indexes[x]=PushColormapIndex(image,RoundToQuantum(pixel),
                  &range_exception);
                *q=image->colormap[(long) indexes[x]];
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            p=PushLongPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                indexes[x]=PushColormapIndex(image,RoundToQuantum(pixel),
                  &range_exception);
                *q=image->colormap[(long) indexes[x]];
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            indexes[x]=PushColormapIndex(image,pixel,&range_exception);
            *q=image->colormap[(long) indexes[x]];
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleAnyToQuantum(pixel,
              image->depth,scale));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
      }
      if (range_exception != MagickFalse)
        (void) ThrowMagickException(&image->exception,GetMagickModule(),
          CorruptImageError,"InvalidColormapIndex","`%s'",image->filename);
      break;
    }
    case GrayQuantum:
    {
      switch (quantum_info->depth)
      {
        case 1:
        {
          register unsigned char
            pixel;

          for (x=0; x < ((long) number_pixels-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              if (quantum_info->min_is_white == MagickFalse)
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ? 0 : 255);
              else
                pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ? 0 : 255);
              q->red=(Quantum) (pixel == 0 ? 0 : QuantumRange);
              q->green=q->red;
              q->blue=q->red;
              q++;
            }
            p++;
          }
          for (bit=0; bit < (long) (number_pixels % 8); bit++)
          {
            if (quantum_info->min_is_white == MagickFalse)
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) == 0 ? 0 : 255);
            else
              pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ? 0 : 255);
            q->red=(Quantum) (pixel == 0 ? 0 : QuantumRange);
            q->green=q->red;
            q->blue=q->red;
            q++;
          }
          if (bit != 0)
            p++;
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          scale=GetQuantumScale(image->depth);
          for (x=0; x < ((long) number_pixels-1); x+=2)
          {
            pixel=(unsigned char) ((*p >> 4) & 0xf);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            q++;
            pixel=(unsigned char) ((*p) & 0xf);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            p++;
            q++;
          }
          for (bit=0; bit < (long) (number_pixels % 2); bit++)
          {
            pixel=(unsigned char) (*p++ >> 4);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            q++;
          }
          break;
        }
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->red=ScaleCharToQuantum(pixel);
            if (quantum_info->min_is_white != MagickFalse)
              q->red=(Quantum) (QuantumRange-q->red);
            q->green=q->red;
            q->blue=q->red;
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          scale=GetQuantumScale(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              if (image->endian == MSBEndian)
                {
                  for (x=0; x < (long) number_pixels/3; x++)
                  {
                    p=PushLongPixel(endian,p,&pixel);
                    q->red=ScaleAnyToQuantum((pixel >> 0) & 0x3ff,image->depth,
                      scale);
                    q->green=q->red;
                    q->blue=q->red;
                    q++;
                    q->red=ScaleAnyToQuantum((pixel >> 10) & 0x3ff,image->depth,
                      scale);
                    q->green=q->red;
                    q->blue=q->red;
                    q++;
                    q->red=ScaleAnyToQuantum((pixel >> 20) & 0x3ff,image->depth,
                      scale);
                    q->green=q->red;
                    q->blue=q->red;
                    p+=quantum_info->pad;
                    q++;
                  }
                  break;
                }
              for (x=0; x < (long) number_pixels/3; x++)
              {
                p=PushLongPixel(endian,p,&pixel);
                q->red=ScaleAnyToQuantum((pixel >> 22) & 0x3ff,image->depth,
                  scale);
                q->green=q->red;
                q->blue=q->red;
                q++;
                q->red=ScaleAnyToQuantum((pixel >> 12) & 0x3ff,image->depth,
                  scale);
                q->green=q->red;
                q->blue=q->red;
                q++;
                q->red=ScaleAnyToQuantum((pixel >> 2) & 0x3ff,image->depth,
                  scale);
                q->green=q->red;
                q->blue=q->red;
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 12:
        {
          scale=GetQuantumScale(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              unsigned short
                pixel;

              for (x=0; x < (long) (number_pixels-1); x+=2)
              {
                p=PushShortPixel(endian,p,&pixel);
                q->red=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),image->depth,
                  scale);
                q->green=q->red;
                q->blue=q->red;
                q++;
                p=PushShortPixel(endian,p,&pixel);
                q->red=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),image->depth,
                  scale);
                q->green=q->red;
                q->blue=q->red;
                p+=quantum_info->pad;
                q++;
              }
              for (bit=0; bit < (long) (number_pixels % 2); bit++)
              {
                p=PushShortPixel(endian,p,&pixel);
                q->red=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),image->depth,
                  scale);
                q->green=q->red;
                q->blue=q->red;
                p+=quantum_info->pad;
                q++;
              }
              if (bit != 0)
                p++;
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          if (quantum_info->min_is_white != MagickFalse)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushShortPixel(endian,p,&pixel);
                q->red=(Quantum) (QuantumRange-ScaleShortToQuantum(pixel));
                q->green=q->red;
                q->blue=q->red;
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->red=ScaleShortToQuantum(pixel);
            q->green=q->red;
            q->blue=q->red;
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                q->green=q->red;
                q->blue=q->red;
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->red=ScaleLongToQuantum(pixel);
            q->green=q->red;
            q->blue=q->red;
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                q->green=q->red;
                q->blue=q->red;
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            p+=quantum_info->pad;
            q++;
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

          for (x=0; x < ((long) number_pixels-3); x+=4)
          {
            for (bit=0; bit < 8; bit+=2)
            {
              pixel=(unsigned char)
                (((*p) & (1 << (7-bit))) != 0 ? 0x00 : 0x01);
              q->red=(Quantum) (pixel == 0 ? 0 : QuantumRange);
              q->green=q->red;
              q->blue=q->red;
              q->opacity=(Quantum) (((*p) & (1UL << (unsigned char) (6-bit)))
                == 0 ? TransparentOpacity : OpaqueOpacity);
              q++;
            }
            p++;
          }
          for (bit=0; bit < (long) (number_pixels % 4); bit+=2)
          {
            pixel=(unsigned char) (((*p) & (1 << (7-bit))) != 0 ? 0x00 : 0x01);
            q->red=(Quantum) (pixel == 0 ? 0 : QuantumRange);
            q->green=q->red;
            q->blue=q->red;
            q->opacity=(Quantum) (((*p) & (1UL << (unsigned char) (6-bit))) == 0
              ? TransparentOpacity : OpaqueOpacity);
            q++;
          }
          if (bit != 0)
            p++;
          break;
        }
        case 4:
        {
          register unsigned char
            pixel;

          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            pixel=(unsigned char) ((*p >> 4) & 0xf);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            q->opacity=(Quantum) (QuantumRange-(QuantumRange*((*p) & 0xf)/15));
            p++;
            q++;
          }
          break;
        }
        case 8:
        {
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->red=ScaleCharToQuantum(pixel);
            q->green=q->red;
            q->blue=q->red;
            p=PushCharPixel(p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=ScaleAnyToQuantum(pixel,image->depth,scale);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 12:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=ScaleAnyToQuantum(pixel,image->depth,scale);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->red=ScaleShortToQuantum(pixel);
            q->green=q->red;
            q->blue=q->red;
            p=PushShortPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                q->green=q->red;
                q->blue=q->red;
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->red=ScaleLongToQuantum(pixel);
            q->green=q->red;
            q->blue=q->red;
            p=PushLongPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                q->green=q->red;
                q->blue=q->red;
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            q->green=q->red;
            q->blue=q->red;
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleAnyToQuantum(pixel,
              image->depth,scale));
            p+=quantum_info->pad;
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->red=ScaleCharToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->red=ScaleShortToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->red=ScaleLongToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            p+=quantum_info->pad;
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->green=ScaleCharToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->green=ScaleShortToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->green=ScaleLongToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
            p+=quantum_info->pad;
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->blue=ScaleCharToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->blue=ScaleShortToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->blue=ScaleLongToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
            p+=quantum_info->pad;
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleAnyToQuantum(pixel,
              image->depth,scale));
            p+=quantum_info->pad;
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            indexes[x]=ScaleCharToQuantum(pixel);
            p+=quantum_info->pad;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            indexes[x]=ScaleShortToQuantum(pixel);
            p+=quantum_info->pad;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                indexes[x]=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            indexes[x]=ScaleLongToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                indexes[x]=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            indexes[x]=ScaleAnyToQuantum(pixel,image->depth,scale);
            p+=quantum_info->pad;
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->red=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->green=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->blue=ScaleCharToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          scale=GetQuantumScale(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushLongPixel(endian,p,&pixel);
                q->red=ScaleAnyToQuantum((pixel >> 22) & 0x3ff,image->depth,
                  scale);
                q->green=ScaleAnyToQuantum((pixel >> 12) & 0x3ff,image->depth,
                  scale);
                q->blue=ScaleAnyToQuantum((pixel >> 2) & 0x3ff,image->depth,
                  scale);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
            q++;
          }
          break;
        }
        case 12:
        {
          scale=GetQuantumScale(image->depth);
          if (quantum_info->pack == MagickFalse)
            {
              unsigned short
                pixel;

              for (x=0; x < (long) (3*number_pixels-1); x+=2)
              {
                p=PushShortPixel(endian,p,&pixel);
                switch (x % 3)
                {
                  default:
                  case 0:
                  {
                    q->red=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    break;
                  }
                  case 1:
                  {
                    q->green=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    break;
                  }
                  case 2:
                  {
                    q->blue=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    q++;
                    break;
                  }
                }
                p=PushShortPixel(endian,p,&pixel);
                switch ((x+1) % 3)
                {
                  default:
                  case 0:
                  {
                    q->red=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    break;
                  }
                  case 1:
                  {
                    q->green=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    break;
                  }
                  case 2:
                  {
                    q->blue=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    q++;
                    break;
                  }
                }
                p+=quantum_info->pad;
              }
              for (bit=0; bit < (long) (3*number_pixels % 2); bit++)
              {
                p=PushShortPixel(endian,p,&pixel);
                switch ((x+bit) % 3)
                {
                  default:
                  case 0:
                  {
                    q->red=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    break;
                  }
                  case 1:
                  {
                    q->green=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    break;
                  }
                  case 2:
                  {
                    q->blue=ScaleAnyToQuantum((QuantumAny) (pixel >> 4),
                      image->depth,scale);
                    q++;
                    break;
                  }
                }
                p+=quantum_info->pad;
              }
              if (bit != 0)
                p++;
              break;
            }
          if (quantum_info->quantum == 32UL)
            {
              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
                p=PushQuantumLongPixel(&quantum_state,image->depth,p,&pixel);
                q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->red=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->green=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->blue=ScaleShortToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->red=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->green=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->blue=ScaleLongToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->red=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->green=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->blue=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 10:
        {
          pixel=0;
          if (quantum_info->pack == MagickFalse)
            {
              long
                n;

              register long
                i;

              unsigned long
                quantum;

              n=0;
              quantum=0;
              for (x=0; x < (long) number_pixels; x++)
              {
                for (i=0; i < 4; i++)
                {
                  switch (n % 3)
                  {
                    case 0:
                    {
                      p=PushLongPixel(endian,p,&pixel);
                      quantum=(unsigned long) (ScaleShortToQuantum(
                        (unsigned short) (((pixel >> 22) & 0x3ff) << 6)));
                      break;
                    }
                    case 1:
                    {
                      quantum=(unsigned long) (ScaleShortToQuantum(
                        (unsigned short) (((pixel >> 12) & 0x3ff) << 6)));
                      break;
                    }
                    case 2:
                    {
                      quantum=(unsigned long) (ScaleShortToQuantum(
                        (unsigned short) (((pixel >> 2) & 0x3ff) << 6)));
                      break;
                    }
                  }
                  switch (i)
                  {
                    case 0: q->red=(Quantum) (quantum); break;
                    case 1: q->green=(Quantum) (quantum); break;
                    case 2: q->blue=(Quantum) (quantum); break;
                    case 3: q->opacity=(Quantum) (QuantumRange-quantum); break;
                  }
                  n++;
                }
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleShortToQuantum((unsigned short) (pixel << 6));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleShortToQuantum((unsigned short) (pixel << 6));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleShortToQuantum((unsigned short) (pixel << 6));
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleShortToQuantum(
              (unsigned short) (pixel << 6)));
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->red=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->green=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->blue=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->red=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->green=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->blue=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleAnyToQuantum(pixel,
              image->depth,scale));
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->red=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->green=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->blue=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->opacity=ScaleCharToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->red=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->green=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->blue=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->opacity=ScaleShortToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->opacity=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->red=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->green=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->blue=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->opacity=ScaleLongToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->opacity=RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=ScaleAnyToQuantum(pixel,image->depth,scale);
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->red=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->green=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->blue=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            indexes[x]=ScaleCharToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->red=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->green=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->blue=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            indexes[x]=ScaleShortToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                indexes[x]=(IndexPacket) RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->red=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->green=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->blue=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            indexes[x]=ScaleLongToQuantum(pixel);
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                indexes[x]=(IndexPacket) RoundToQuantum(pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            indexes[x]=ScaleAnyToQuantum(pixel,image->depth,scale);
            q++;
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
          unsigned char
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushCharPixel(p,&pixel);
            q->red=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->green=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->blue=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            indexes[x]=ScaleCharToQuantum(pixel);
            p=PushCharPixel(p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleCharToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 16:
        {
          unsigned short
            pixel;

          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushShortPixel(endian,p,&pixel);
            q->red=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->green=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->blue=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            indexes[x]=ScaleShortToQuantum(pixel);
            p=PushShortPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleShortToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
          }
          break;
        }
        case 32:
        {
          unsigned long
            pixel;

          if (quantum_info->format == FloatingPointQuantumFormat)
            {
              float
                pixel;

              for (x=0; x < (long) number_pixels; x++)
              {
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                indexes[x]=(IndexPacket) RoundToQuantum(pixel);
                p=PushFloatPixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushLongPixel(endian,p,&pixel);
            q->red=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->green=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->blue=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            indexes[x]=ScaleLongToQuantum(pixel);
            p=PushLongPixel(endian,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleLongToQuantum(pixel));
            p+=quantum_info->pad;
            q++;
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
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->red=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->green=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->blue=RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                indexes[x]=(IndexPacket) RoundToQuantum(pixel);
                p=PushDoublePixel(&quantum_state,p,&pixel);
                q->opacity=(Quantum) (QuantumRange-RoundToQuantum(pixel));
                p=PushDoublePixel(&quantum_state,p,&pixel);
                p+=quantum_info->pad;
                q++;
              }
              break;
            }
        }
        default:
        {
          scale=GetQuantumScale(image->depth);
          for (x=0; x < (long) number_pixels; x++)
          {
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->red=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->green=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->blue=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            indexes[x]=ScaleAnyToQuantum(pixel,image->depth,scale);
            p=PushQuantumPixel(&quantum_state,image->depth,p,&pixel);
            q->opacity=(Quantum) (QuantumRange-ScaleAnyToQuantum(pixel,
              image->depth,scale));
            q++;
          }
          break;
        }
      }
      break;
    }
    default:
      break;
  }
  if (quantum_info->alpha_type == DisassociatedQuantumAlpha)
    {
      MagickRealType
        alpha;

      /*
        Disassociate alpha.
      */
      q=GetPixels(image);
      for (x=0; x < (long) image->columns; x++)
      {
        alpha=QuantumScale*((MagickRealType) QuantumRange-q->opacity);
        alpha=1.0/(fabs(alpha) <= MagickEpsilon ? 1.0 : alpha);
        q->red=RoundToQuantum(alpha*q->red);
        q->green=RoundToQuantum(alpha*q->green);
        q->blue=RoundToQuantum(alpha*q->blue);
        q++;
      }
    }
  return(extent);
}
