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
%                 Methods to Acquire / Destroy Quantum Pixels                 %
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
  Define declarations.
*/
#define QuantumSignature  0xab

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e Q u a n t u m I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireQuantumInfo() allocates the QuantumInfo structure.
%
%  The format of the AcquireQuantumInfo method is:
%
%      QuantumInfo *AcquireQuantumInfo(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o image: the image.
%
*/

static inline unsigned long MagickMax(const unsigned long x,
  const unsigned long y)
{
  if (x > y)
    return(x);
  return(y);
}

MagickExport QuantumInfo *AcquireQuantumInfo(const ImageInfo *image_info,
  Image *image)
{
  QuantumInfo
    *quantum_info;

  quantum_info=(QuantumInfo *) AcquireMagickMemory(sizeof(*quantum_info));
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  quantum_info->signature=MagickSignature;
  GetQuantumInfo(image_info,quantum_info);
  if (image != (const Image *) NULL)
    SetQuantumDepth(image,image->depth,quantum_info);
  return(quantum_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y Q u a n t u m I n f o                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyQuantumInfo() deallocates memory associated with the QuantumInfo
%  structure.
%
%  The format of the DestroyQuantumInfo method is:
%
%      QuantumInfo *DestroyQuantumInfo(QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o quantum_info: the quantum_info info.
%
*/
MagickExport QuantumInfo *DestroyQuantumInfo(QuantumInfo *quantum_info)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  if (quantum_info->pixels != (unsigned char *) NULL)
    {
      assert(quantum_info->pixels[quantum_info->extent] == QuantumSignature);
      quantum_info->pixels=(unsigned char *) RelinquishMagickMemory(
        quantum_info->pixels);
    }
  if (quantum_info->semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&quantum_info->semaphore);
  quantum_info->signature=(~MagickSignature);
  quantum_info=(QuantumInfo *) RelinquishMagickMemory(quantum_info);
  return(quantum_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Q u a n t u m E x t e n t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetQuantumExtent() returns the quantum pixel buffer extent.
%
%  The format of the GetQuantumExtent method is:
%
%      size_t GetQuantumExtent(Image *image,const QuantumInfo *quantum_info,
%        const QuantumType quantum_type)
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
*/
MagickExport size_t GetQuantumExtent(const Image *image,
  const QuantumInfo *quantum_info,const QuantumType quantum_type)
{
  MagickSizeType
    number_pixels;

  size_t
    packet_size;

  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  packet_size=1;
  switch (quantum_type)
  {
    case GrayAlphaQuantum: packet_size=2; break;
    case IndexAlphaQuantum: packet_size=2; break;
    case RGBQuantum: packet_size=3; break;
    case RGBAQuantum: packet_size=4; break;
    case RGBOQuantum: packet_size=4; break;
    case CMYKQuantum: packet_size=4; break;
    case CMYKAQuantum: packet_size=5; break;
    default: break;
  }
  number_pixels=GetPixelCacheExtent(image);
  if (quantum_info->pack == MagickFalse)
    return((size_t) (packet_size*number_pixels*((image->depth+7)/8)));
  return((size_t) ((packet_size*number_pixels*image->depth+7)/8));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Q u a n t u m I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetQuantumInfo() initializes the QuantumInfo structure to default values.
%
%  The format of the GetQuantumInfo method is:
%
%      GetQuantumInfo(const ImageInfo *image_info,QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o quantum_info: the quantum info.
%
*/
MagickExport void GetQuantumInfo(const ImageInfo *image_info,
  QuantumInfo *quantum_info)
{
  const char
    *option;

  assert(quantum_info != (QuantumInfo *) NULL);
  (void) ResetMagickMemory(quantum_info,0,sizeof(*quantum_info));
  quantum_info->quantum=8;
  quantum_info->maximum=1.0;
  quantum_info->scale=QuantumRange;
  quantum_info->pack=MagickTrue;
  quantum_info->signature=MagickSignature;
  if (image_info == (const ImageInfo *) NULL)
    return;
  option=GetImageOption(image_info,"quantum:format");
  if (option != (char *) NULL)
    quantum_info->format=(QuantumFormatType) ParseMagickOption(
      MagickQuantumFormatOptions,MagickFalse,option);
  option=GetImageOption(image_info,"quantum:minimum");
  if (option != (char *) NULL)
    quantum_info->minimum=atof(option);
  option=GetImageOption(image_info,"quantum:maximum");
  if (option != (char *) NULL)
    quantum_info->maximum=atof(option);
  if ((quantum_info->minimum == 0.0) && (quantum_info->maximum == 0.0))
    quantum_info->scale=0.0;
  else
    if (quantum_info->minimum == quantum_info->maximum)
      {
        quantum_info->scale=(MagickRealType) QuantumRange/quantum_info->minimum;
        quantum_info->minimum=0.0;
      }
    else
      quantum_info->scale=(MagickRealType) QuantumRange/(quantum_info->maximum-
        quantum_info->minimum);
  if ((quantum_info->format == FloatingPointQuantumFormat) &&
      (quantum_info->scale != 0.0))
    quantum_info->scale=1.0/quantum_info->scale;
  option=GetImageOption(image_info,"quantum:scale");
  if (option != (char *) NULL)
    quantum_info->scale=atof(option);
  option=GetImageOption(image_info,"quantum:polarity");
  if (option != (char *) NULL)
    quantum_info->min_is_white=LocaleCompare(option,"min-is-white") == 0 ?
      MagickTrue : MagickFalse;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Q u a n t u m P i x e l s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetQuantumPixels() returns the quantum pixels.
%
%  The format of the GetQuantumPixels method is:
%
%      unsigned char *QuantumPixels GetQuantumPixels(
%        const QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport unsigned char *GetQuantumPixels(const QuantumInfo *quantum_info)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  return(quantum_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t Q u a n t u m T y p e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetQuantumType() returns the quantum type of the image.
%
%  The format of the GetQuantumType method is:
%
%      QuantumType GetQuantumType(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport QuantumType GetQuantumType(const Image *image,
  ExceptionInfo *exception)
{
  QuantumType
    quantum_type;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  quantum_type=RGBQuantum;
  if (image->matte != MagickFalse)
    quantum_type=RGBAQuantum;
  if (image->colorspace == CMYKColorspace)
    {
      quantum_type=CMYKQuantum;
      if (image->matte != MagickFalse)
        quantum_type=CMYKAQuantum;
    }
  if (IsGrayImage(image,exception) != MagickFalse)
    {
      quantum_type=GrayQuantum;
      if (image->matte != MagickFalse)
        quantum_type=GrayAlphaQuantum;
    }
  else
    if (image->storage_class == PseudoClass)
      {
        quantum_type=IndexQuantum;
        if (image->matte != MagickFalse)
          quantum_type=IndexAlphaQuantum;
      }
  return(quantum_type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m F o r m a t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumAlphaType() sets the quantum format.
%
%  The format of the SetQuantumAlphaType method is:
%
%      void SetQuantumAlphaType(const QuantumAlphaType type,
%        QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o type: the alpha type (e.g. associate).
%
%    o quantum_info: the quantum_info info.
%
*/
MagickExport void SetQuantumAlphaType(const QuantumAlphaType type,
  QuantumInfo *quantum_info)
{
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->alpha_type=type;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m D e p t h                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumDepth() sets the quantum depth.
%
%  The format of the SetQuantumDepth method is:
%
%      unsigned char *SetQuantumDepth(const Image *image,
%        const unsigned long depth,QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o depth: the quantum depth.
%
%    o quantum_info: the quantum_info info.
%
*/
MagickExport unsigned char *SetQuantumDepth(const Image *image,
  const unsigned long depth,QuantumInfo *quantum_info)
{
  /*
    Allocate the quantum pixel buffer.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->depth=depth;
  if (quantum_info->format == FloatingPointQuantumFormat)
    {
      if (quantum_info->depth > 32)
        quantum_info->depth=64;
      else
        quantum_info->depth=32;
    }
  if (quantum_info->pixels != (unsigned char *) NULL)
    {
      assert(quantum_info->pixels[quantum_info->extent] == QuantumSignature);
      quantum_info->pixels=(unsigned char *) RelinquishMagickMemory(
        quantum_info->pixels);
    }
  quantum_info->pixels=(unsigned char *) AcquireQuantumMemory(
    (quantum_info->pad+5)*image->columns+1,((quantum_info->depth+7)/8)*
    sizeof(*quantum_info->pixels));
  if (quantum_info->pixels == (unsigned char *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  quantum_info->extent=(quantum_info->pad+5)*image->columns*
    ((quantum_info->depth+7)/8);
  (void) ResetMagickMemory(quantum_info->pixels,0,quantum_info->extent);
  quantum_info->pixels[quantum_info->extent]=QuantumSignature;
  return(quantum_info->pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m F o r m a t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumFormat() sets the quantum format.
%
%  The format of the SetQuantumFormat method is:
%
%      unsigned char *SetQuantumFormat(const Image *image,
%        const QuantumFormatType format,QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o format: the quantum format.
%
%    o quantum_info: the quantum_info info.
%
*/
MagickExport unsigned char *SetQuantumFormat(const Image *image,
  const QuantumFormatType format,QuantumInfo *quantum_info)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->format=format;
  return(SetQuantumDepth(image,quantum_info->depth,quantum_info));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m I m a g e T y p e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumImageType() sets the image type based on the quantum type.
%
%  The format of the SetQuantumImageType method is:
%
%      void ImageType SetQuantumImageType(Image *image,
%        const QuantumType quantum_type)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o quantum_type: Declare which pixel components to transfer (red, green,
%      blue, opacity, RGB, or RGBA).
%
*/
MagickExport void SetQuantumImageType(Image *image,
  const QuantumType quantum_type)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  switch (quantum_type)
  {
    case IndexQuantum:
    case IndexAlphaQuantum:
    {
      image->type=PaletteType;
      break;
    }
    case GrayQuantum:
    case GrayAlphaQuantum:
    {
      image->type=GrayscaleType;
      if (image->depth == 1)
        image->type=BilevelType;
      break;
    }
    case CyanQuantum:
    case MagentaQuantum:
    case YellowQuantum:
    case BlackQuantum:
    case CMYKQuantum:
    case CMYKAQuantum:
    {
      image->type=ColorSeparationType;
      break;
    }
    default:
    {
      image->type=TrueColorType;
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t Q u a n t u m P a d                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetQuantumPad() sets the quantum pad.
%
%  The format of the SetQuantumPad method is:
%
%      unsigned char *SetQuantumPad(const Image *image,const unsigned long pad,
%        QuantumInfo *quantum_info)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pad: the quantum pad.
%
%    o quantum_info: the quantum_info info.
%
*/
MagickExport unsigned char *SetQuantumPad(const Image *image,
  const unsigned long pad,QuantumInfo *quantum_info)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(quantum_info != (QuantumInfo *) NULL);
  assert(quantum_info->signature == MagickSignature);
  quantum_info->pad=pad;
  return(SetQuantumDepth(image,quantum_info->depth,quantum_info));
}
