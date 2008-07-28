/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        SSSSS  TTTTT   AAA   TTTTT  IIIII  SSSSS  TTTTT  IIIII   CCCC        %
%        SS       T    A   A    T      I    SS       T      I    C            %
%         SSS     T    AAAAA    T      I     SSS     T      I    C            %
%           SS    T    A   A    T      I       SS    T      I    C            %
%        SSSSS    T    A   A    T    IIIII  SSSSS    T    IIIII   CCCC        %
%                                                                             %
%                                                                             %
%                          ImageMagick Image Methods                          %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
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
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/animate.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/client.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/compress.h"
#include "magick/constitute.h"
#include "magick/deprecate.h"
#include "magick/display.h"
#include "magick/draw.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/list.h"
#include "magick/image-private.h"
#include "magick/magic.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/module.h"
#include "magick/monitor.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/profile.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/segment.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/timer.h"
#include "magick/utility.h"
#include "magick/version.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e B o u n d i n g B o x                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageBoundingBox() returns the bounding box of an image canvas.
%
%  The format of the GetImageBoundingBox method is:
%
%      RectangleInfo GetImageBoundingBox(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o bounds: Method GetImageBoundingBox returns the bounding box of an
%      image canvas.
%
%    o image: the image.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport RectangleInfo GetImageBoundingBox(const Image *image,
  ExceptionInfo *exception)
{
  long
    y;

  MagickPixelPacket
    target[3],
    pixel;

  RectangleInfo
    bounds;

  register const PixelPacket
    *p;

  register IndexPacket
    *indexes;

  register long
    x;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  bounds.width=0;
  bounds.height=0;
  bounds.x=(long) image->columns;
  bounds.y=(long) image->rows;
  GetMagickPixelPacket(image,&target[0]);
  p=AcquireImagePixels(image,0,0,1,1,exception);
  if (p == (const PixelPacket *) NULL)
    return(bounds);
  SetMagickPixelPacket(image,p,GetIndexes(image),&target[0]);
  GetMagickPixelPacket(image,&target[1]);
  p=AcquireImagePixels(image,(long) image->columns-1,0,1,1,exception);
  SetMagickPixelPacket(image,p,GetIndexes(image),&target[1]);
  GetMagickPixelPacket(image,&target[2]);
  p=AcquireImagePixels(image,0,(long) image->rows-1,1,1,exception);
  SetMagickPixelPacket(image,p,GetIndexes(image),&target[2]);
  GetMagickPixelPacket(image,&pixel);
  for (y=0; y < (long) image->rows; y++)
  {
    p=AcquireImagePixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=GetIndexes(image);
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if ((x < bounds.x) &&
          (IsMagickColorSimilar(&pixel,&target[0]) == MagickFalse))
        bounds.x=x;
      if ((x > (long) bounds.width) &&
          (IsMagickColorSimilar(&pixel,&target[1]) == MagickFalse))
        bounds.width=(unsigned long) x;
      if ((y < bounds.y) &&
          (IsMagickColorSimilar(&pixel,&target[0]) == MagickFalse))
        bounds.y=y;
      if ((y > (long) bounds.height) &&
          (IsMagickColorSimilar(&pixel,&target[2]) == MagickFalse))
        bounds.height=(unsigned long) y;
      p++;
    }
  }
  if ((bounds.width == 0) || (bounds.height == 0))
    (void) ThrowMagickException(exception,GetMagickModule(),OptionWarning,
      "GeometryDoesNotContainImage","`%s'",image->filename);
  else
    {
      bounds.width-=(bounds.x-1);
      bounds.height-=(bounds.y-1);
    }
  return(bounds);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l D e p t h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelDepth() returns the depth of a particular image channel.
%
%  The format of the GetImageChannelDepth method is:
%
%      unsigned long GetImageDepth(const Image *image,ExceptionInfo *exception)
%      unsigned long GetImageChannelDepth(const Image *image,
%        const ChannelType channel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

MagickExport unsigned long GetImageDepth(const Image *image,
  ExceptionInfo *exception)
{
  return(GetImageChannelDepth(image,AllChannels,exception));
}

MagickExport unsigned long GetImageChannelDepth(const Image *image,
  const ChannelType channel,ExceptionInfo *exception)
{
  long
    y;

  MagickBooleanType
    status;

  register long
    id;

  unsigned long
    *current_depth,
    depth;
    
  ViewInfo
    **image_view;
  
  /*
    Compute image depth.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  current_depth=(unsigned long *) AcquireQuantumMemory(
    GetCacheViewMaximumThreads(),sizeof(*current_depth));
  if (current_depth == (unsigned long *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  status=MagickTrue;
  for (id=0; id < (long) GetCacheViewMaximumThreads(); id++)
    current_depth[id]=1;
  if ((image->storage_class == PseudoClass) && (image->matte == MagickFalse))
    {
      register const PixelPacket
        *p;

      register long
        i;

      p=image->colormap;
      #pragma omp parallel for
      for (i=0; i < (long) image->colors; i++)
      {  
        if (status == MagickFalse)
          continue;
        id=GetCacheViewThreadId();
        while (current_depth[id] < MAGICKCORE_QUANTUM_DEPTH)
        {
          MagickStatusType
            status;

          QuantumAny
            scale;

          status=0;
          scale=GetQuantumScale(current_depth[id]);
          if ((channel & RedChannel) != 0)
            status|=p->red != ScaleAnyToQuantum(ScaleQuantumToAny(p->red,
              current_depth[id],scale),current_depth[id],scale);
          if ((channel & GreenChannel) != 0)
            status|=p->green != ScaleAnyToQuantum(ScaleQuantumToAny(p->green,
              current_depth[id],scale),current_depth[id],scale);
          if ((channel & BlueChannel) != 0)
            status|=p->blue != ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,
              current_depth[id],scale),current_depth[id],scale);
          if (status == 0)
            break;
          current_depth[id]++;
        }
        p++;
      }
      depth=current_depth[0];
      for (id=1; id < (long) GetCacheViewMaximumThreads(); id++)
        if (depth < current_depth[id])
          depth=current_depth[id];
      current_depth=(unsigned long *) RelinquishMagickMemory(current_depth);
      return(depth);
    }
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    register const IndexPacket
      *indexes;

    register const PixelPacket
      *p;

    register long
      id,
      x;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      continue;
    indexes=AcquireCacheViewIndexes(image_view[id]);
    for (x=0; x < (long) image->columns; x++)
    {  
      while (current_depth[id] < MAGICKCORE_QUANTUM_DEPTH)
      {
        MagickStatusType
          status;

        QuantumAny
          scale;

        status=0;
        scale=GetQuantumScale(current_depth[id]);
        if ((channel & RedChannel) != 0)
          status|=p->red != ScaleAnyToQuantum(ScaleQuantumToAny(p->red,
            current_depth[id],scale),current_depth[id],scale);
        if ((channel & GreenChannel) != 0)
          status|=p->green != ScaleAnyToQuantum(ScaleQuantumToAny(p->green,
            current_depth[id],scale),current_depth[id],scale);
        if ((channel & BlueChannel) != 0)
          status|=p->blue != ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,
            current_depth[id],scale),current_depth[id],scale);
        if (((channel & OpacityChannel) != 0) && (image->matte != MagickFalse))
          status|=p->opacity != ScaleAnyToQuantum(ScaleQuantumToAny(p->opacity,
            current_depth[id],scale),current_depth[id],scale);
        if (((channel & IndexChannel) != 0) &&
            (image->colorspace == CMYKColorspace))
          status|=indexes[x] != ScaleAnyToQuantum(ScaleQuantumToAny(indexes[x],
            current_depth[id],scale),current_depth[id],scale);
        if (status == 0)
          break;
        current_depth[id]++;
      }
      p++;
    }
    if (current_depth[id] == MAGICKCORE_QUANTUM_DEPTH)
      status=MagickFalse;
  }
  image_view=DestroyCacheViewThreadSet(image_view);
  depth=current_depth[0];
  for (id=1; id < (long) GetCacheViewMaximumThreads(); id++)
    if (depth < current_depth[id])
      depth=current_depth[id];
  current_depth=(unsigned long *) RelinquishMagickMemory(current_depth);
  return(depth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   G e t I m a g e C h a n n e l E x t r e m a                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelExtrema() returns the extrema of one or more image channels.
%
%  The format of the GetImageChannelExtrema method is:
%
%      MagickBooleanType GetImageChannelExtrema(const Image *image,
%        const ChannelType channel,unsigned long *minima,unsigned long *maxima,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageExtrema(const Image *image,
  unsigned long *minima,unsigned long *maxima,ExceptionInfo *exception)
{
  return(GetImageChannelExtrema(image,AllChannels,minima,maxima,exception));
}

MagickExport MagickBooleanType GetImageChannelExtrema(const Image *image,
  const ChannelType channel,unsigned long *minima,unsigned long *maxima,
  ExceptionInfo *exception)
{
  double
    max,
    min;

  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=GetImageChannelRange(image,channel,&min,&max,exception);
  *minima=(unsigned long) (min+0.5);
  *maxima=(unsigned long) (max+0.5);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l M e a n                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelMean() returns the mean and standard deviation of one or more
%  image channels.
%
%  The format of the GetImageChannelMean method is:
%
%      MagickBooleanType GetImageChannelMean(const Image *image,
%        const ChannelType channel,double *mean,double *standard_deviation,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o mean: the average value in the channel.
%
%    o standard_deviation: the standard deviation of the channel.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageMean(const Image *image,double *mean,
  double *standard_deviation,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  status=GetImageChannelMean(image,AllChannels,mean,standard_deviation,
    exception);
  return(status);
}

MagickExport MagickBooleanType GetImageChannelMean(const Image *image,
  const ChannelType channel,double *mean,double *standard_deviation,
  ExceptionInfo *exception)
{
#define PixelSquared(x)  ((x)*(x))

  double
    area;

  long
    y;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    x;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *mean=0.0;
  *standard_deviation=0.0;
  area=0.0;
  for (y=0; y < (long) image->rows; y++)
  {
    p=AcquireImagePixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=AcquireIndexes(image);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        {
          *mean+=p->red;
          *standard_deviation+=(double) p->red*p->red;
          area++;
        }
      if ((channel & GreenChannel) != 0)
        {
          *mean+=p->green;
          *standard_deviation+=(double) p->green*p->green;
          area++;
        }
      if ((channel & BlueChannel) != 0)
        {
          *mean+=p->blue;
          *standard_deviation+=(double) p->blue*p->blue;
          area++;
        }
      if ((channel & OpacityChannel) != 0)
        {
          *mean+=p->opacity;
          *standard_deviation+=(double) p->opacity*p->opacity;
          area++;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          *mean+=indexes[x];
          *standard_deviation+=(double) indexes[x]*indexes[x];
          area++;
        }
      p++;
    }
  }
  if (y < (long) image->rows)
    return(MagickFalse);
  if (area != 0)
    {
      *mean/=area;
      *standard_deviation/=area;
    }
  *standard_deviation=sqrt(*standard_deviation-(*mean*(*mean)));
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l R a n g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelRange() returns the range of one or more image channels.
%
%  The format of the GetImageChannelRange method is:
%
%      MagickBooleanType GetImageChannelRange(const Image *image,
%        const ChannelType channel,double *minima,double *maxima,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o minima: the minimum value in the channel.
%
%    o maxima: the maximum value in the channel.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

MagickExport MagickBooleanType GetImageRange(const Image *image,
  double *minima,double *maxima,ExceptionInfo *exception)
{
  return(GetImageChannelRange(image,AllChannels,minima,maxima,exception));
}

MagickExport MagickBooleanType GetImageChannelRange(const Image *image,
  const ChannelType channel,double *minima,double *maxima,
  ExceptionInfo *exception)
{
  long
    y;

  MagickPixelPacket
    pixel;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    x;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  *maxima=(-1.0E-37);
  *minima=1.0E+37;
  GetMagickPixelPacket(image,&pixel);
  for (y=0; y < (long) image->rows; y++)
  {
    p=AcquireImagePixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=AcquireIndexes(image);
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if ((channel & RedChannel) != 0)
        {
          if (pixel.red < *minima)
            *minima=(double) pixel.red;
          if (pixel.red > *maxima)
            *maxima=(double) pixel.red;
        }
      if ((channel & GreenChannel) != 0)
        {
          if (pixel.green < *minima)
            *minima=(double) pixel.green;
          if (pixel.green > *maxima)
            *maxima=(double) pixel.green;
        }
      if ((channel & BlueChannel) != 0)
        {
          if (pixel.blue < *minima)
            *minima=(double) pixel.blue;
          if (pixel.blue > *maxima)
            *maxima=(double) pixel.blue;
        }
      if ((channel & OpacityChannel) != 0)
        {
          if (pixel.opacity < *minima)
            *minima=(double) pixel.opacity;
          if (pixel.opacity > *maxima)
            *maxima=(double) pixel.opacity;
        }
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        {
          if ((double) indexes[x] < *minima)
            *minima=(double) indexes[x];
          if ((double) indexes[x] > *maxima)
            *maxima=(double) indexes[x];
        }
      p++;
    }
  }
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C h a n n e l S t a t i s t i c s                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageChannelStatistics() returns statistics for each channel in the
%  image.  The statistics incude the channel depth, its minima and
%  maxima, the mean, and the standard deviation.  You can access the red
%  channel mean, for example, like this:
%
%      channel_statistics=GetImageChannelStatistics(image,excepton);
%      red_mean=channel_statistics[RedChannel].mean;
%
%  Use MagickRelinquishMemory() to free the statistics buffer.
%
%  The format of the GetImageChannelStatistics method is:
%
%      ChannelStatistics *GetImageChannelStatistics(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static inline double MagickMax(const double x,const double y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline double MagickMin(const double x,const double y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport ChannelStatistics *GetImageChannelStatistics(const Image *image,
  ExceptionInfo *exception)
{
  ChannelStatistics
    *channel_statistics;

  double
    area;

  long
    y;

  MagickStatusType
    status;

  QuantumAny
    scale;

  register const IndexPacket
    *indexes;

  register const PixelPacket
    *p;

  register long
    i,
    x;

  size_t
    length;

  unsigned long
    channels,
    depth;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  length=AllChannels+1UL;
  channel_statistics=(ChannelStatistics *) AcquireQuantumMemory(length,
    sizeof(*channel_statistics));
  if (channel_statistics == (ChannelStatistics *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(channel_statistics,0,length);
  for (i=0; i <= AllChannels; i++)
  {
    channel_statistics[i].depth=1;
    channel_statistics[i].maxima=(-1.0E-37);
    channel_statistics[i].minima=1.0E+37;
    channel_statistics[i].mean=0.0;
    channel_statistics[i].standard_deviation=0.0;
  }
  y=(long) image->rows;
  for (y=0; y < (long) image->rows; y++)
  {
    p=AcquireImagePixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    indexes=AcquireIndexes(image);
    for (x=0; x < (long) image->columns; )
    {
      if (channel_statistics[RedChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[RedChannel].depth;
          scale=GetQuantumScale(depth);
          status=p->red != ScaleAnyToQuantum(ScaleQuantumToAny(p->red,depth,
            scale),depth,scale) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[RedChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[GreenChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[GreenChannel].depth;
          scale=GetQuantumScale(depth);
          status=p->green != ScaleAnyToQuantum(ScaleQuantumToAny(p->green,
            depth,scale),depth,scale) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[GreenChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[BlueChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[BlueChannel].depth;
          scale=GetQuantumScale(depth);
          status=p->blue != ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,
            depth,scale),depth,scale) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[BlueChannel].depth++;
              continue;
            }
        }
      if (channel_statistics[OpacityChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
        {
          depth=channel_statistics[OpacityChannel].depth;
          scale=GetQuantumScale(depth);
          status=p->opacity != ScaleAnyToQuantum(ScaleQuantumToAny(p->opacity,
            depth,scale),depth,scale) ? MagickTrue : MagickFalse;
          if (status != MagickFalse)
            {
              channel_statistics[OpacityChannel].depth++;
              continue;
            }
        }
      if (image->colorspace == CMYKColorspace)
        {
          if (channel_statistics[BlackChannel].depth != MAGICKCORE_QUANTUM_DEPTH)
            {
              depth=channel_statistics[BlackChannel].depth;
              scale=GetQuantumScale(depth);
              status=indexes[x] != ScaleAnyToQuantum(ScaleQuantumToAny(
                indexes[x],depth,scale),depth,scale) ? MagickTrue : MagickFalse;
              if (status != MagickFalse)
                {
                  channel_statistics[BlackChannel].depth++;
                  continue;
                }
            }
        }
      if ((double) p->red < channel_statistics[RedChannel].minima)
        channel_statistics[RedChannel].minima=(double) p->red;
      if ((double) p->red > channel_statistics[RedChannel].maxima)
        channel_statistics[RedChannel].maxima=(double) p->red;
      channel_statistics[RedChannel].mean+=p->red;
      channel_statistics[RedChannel].standard_deviation+=(double)
        p->red*p->red;
      if ((double) p->green < channel_statistics[GreenChannel].minima)
        channel_statistics[GreenChannel].minima=(double) p->green;
      if ((double) p->green > channel_statistics[GreenChannel].maxima)
        channel_statistics[GreenChannel].maxima=(double) p->green;
      channel_statistics[GreenChannel].mean+=p->green;
      channel_statistics[GreenChannel].standard_deviation+=(double)
        p->green*p->green;
      if ((double) p->blue < channel_statistics[BlueChannel].minima)
        channel_statistics[BlueChannel].minima=(double) p->blue;
      if ((double) p->blue > channel_statistics[BlueChannel].maxima)
        channel_statistics[BlueChannel].maxima=(double) p->blue;
      channel_statistics[BlueChannel].mean+=p->blue;
      channel_statistics[BlueChannel].standard_deviation+=(double)
        p->blue*p->blue;
      if ((double) p->opacity < channel_statistics[OpacityChannel].minima)
        channel_statistics[OpacityChannel].minima=(double) p->opacity;
      if ((double) p->opacity > channel_statistics[OpacityChannel].maxima)
        channel_statistics[OpacityChannel].maxima=(double) p->opacity;
      channel_statistics[OpacityChannel].mean+=p->opacity;
      channel_statistics[OpacityChannel].standard_deviation+=(double)
        p->opacity*p->opacity;
      if (image->colorspace == CMYKColorspace)
        {
          if ((double) indexes[x] < channel_statistics[BlackChannel].minima)
            channel_statistics[BlackChannel].minima=(double) indexes[x];
          if ((double) indexes[x] > channel_statistics[BlackChannel].maxima)
            channel_statistics[BlackChannel].maxima=(double) indexes[x];
          channel_statistics[BlackChannel].mean+=indexes[x];
          channel_statistics[BlackChannel].standard_deviation+=(double)
            indexes[x]*indexes[x];
        }
      x++;
      p++;
    }
  }
  area=(double) image->columns*image->rows;
  for (i=0; i < AllChannels; i++)
  {
    channel_statistics[i].mean/=area;
    channel_statistics[i].standard_deviation/=area;
  }
  for (i=0; i < AllChannels; i++)
  {
    channel_statistics[AllChannels].depth=(unsigned long) MagickMax((double) 
      channel_statistics[AllChannels].depth,(double)
      channel_statistics[i].depth);
    channel_statistics[AllChannels].minima=MagickMin(
      channel_statistics[AllChannels].minima,channel_statistics[i].minima);
    channel_statistics[AllChannels].maxima=MagickMax(
      channel_statistics[AllChannels].maxima,channel_statistics[i].maxima);
    channel_statistics[AllChannels].mean+=channel_statistics[i].mean;
    channel_statistics[AllChannels].standard_deviation+=
      channel_statistics[i].standard_deviation;
  }
  channels=4;
  if (image->colorspace == CMYKColorspace)
    channels++;
  channel_statistics[AllChannels].mean/=channels;
  channel_statistics[AllChannels].standard_deviation/=channels;
  for (i=0; i <= AllChannels; i++)
    channel_statistics[i].standard_deviation=sqrt(
      channel_statistics[i].standard_deviation-
       (channel_statistics[i].mean*channel_statistics[i].mean));
  return(channel_statistics);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e Q u a n t u m D e p t h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageQuantumDepth() returns the depth of the image rounded to a legal
%  quantum depth: 8, 16, or 32.
%
%  The format of the GetImageQuantumDepth method is:
%
%      unsigned long GetImageQuantumDepth(const Image *image,
%        const MagickBooleanType constrain)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o constrain: A value other than MagickFalse, constrains the depth to
%      a maximum of MAGICKCORE_QUANTUM_DEPTH.
%
*/
MagickExport unsigned long GetImageQuantumDepth(const Image *image,
  const MagickBooleanType constrain)
{
  unsigned long
    depth;

  depth=image->depth;
  if (depth <= 8)
    depth=8;
  else
    if (depth <= 16)
      depth=16;
  if (constrain != MagickFalse)
    depth=(unsigned long) MagickMin((double) depth,(double)
      MAGICKCORE_QUANTUM_DEPTH);
  return(depth);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e C h a n n e l D e p t h                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageChannelDepth() sets the depth of the image.
%
%  The format of the SetImageChannelDepth method is:
%
%      MagickBooleanType SetImageDepth(Image *image,const unsigned long depth)
%      MagickBooleanType SetImageChannelDepth(Image *image,
%        const ChannelType channel,const unsigned long depth)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel.
%
%    o depth: the image depth.
%
*/

MagickExport MagickBooleanType SetImageDepth(Image *image,
  const unsigned long depth)
{
  return(SetImageChannelDepth(image,AllChannels,depth));
}

MagickExport MagickBooleanType SetImageChannelDepth(Image *image,
  const ChannelType channel,const unsigned long depth)
{
  long
    y;

  MagickBooleanType
    status;

  QuantumAny
    scale;

  ViewInfo
    **image_view;
  
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (GetImageDepth(image,&image->exception) <= (unsigned long)
      MagickMin((double) depth,(double) MAGICKCORE_QUANTUM_DEPTH))
    {
      image->depth=depth;
      return(MagickTrue);
    }
  /*
    Scale pixels to desired depth.
  */
  status=MagickTrue;
  scale=GetQuantumScale(depth);
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    register IndexPacket
      *indexes;

    register long
      id,
      x;

    register PixelPacket
      *q;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    q=GetCacheViewPixels(image_view[id],0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewIndexes(image_view[id]);
    for (x=0; x < (long) image->columns; x++)
    {
      if ((channel & RedChannel) != 0)
        q->red=ScaleAnyToQuantum(ScaleQuantumToAny(q->red,depth,scale),depth,
          scale);
      if ((channel & GreenChannel) != 0)
        q->green=ScaleAnyToQuantum(ScaleQuantumToAny(q->green,depth,scale),
          depth,scale);
      if ((channel & BlueChannel) != 0)
        q->blue=ScaleAnyToQuantum(ScaleQuantumToAny(q->blue,depth,scale),
          depth,scale);
      if (((channel & OpacityChannel) != 0) && (image->matte != MagickFalse))
        q->opacity=ScaleAnyToQuantum(ScaleQuantumToAny(q->opacity,depth,scale),
          depth,scale);
      if (((channel & IndexChannel) != 0) &&
          (image->colorspace == CMYKColorspace))
        indexes[x]=ScaleAnyToQuantum(ScaleQuantumToAny(indexes[x],depth,scale),
          depth,scale);
      q++;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      {
        status=MagickFalse;
        continue;
      }
  }
  image_view=DestroyCacheViewThreadSet(image_view);
  if (image->storage_class == PseudoClass)
    {
      QuantumAny
        scale;

      register long
        i;

      register PixelPacket
        *p;

      p=image->colormap;
      scale=GetQuantumScale(depth);
      #pragma omp parallel for
      for (i=0; i < (long) image->colors; i++)
      {
        if ((channel & RedChannel) != 0)
          p->red=ScaleAnyToQuantum(ScaleQuantumToAny(p->red,depth,
            scale),depth,scale);
        if ((channel & GreenChannel) != 0)
          p->green=ScaleAnyToQuantum(ScaleQuantumToAny(p->green,depth,
            scale),depth,scale);
        if ((channel & BlueChannel) != 0)
          p->blue=ScaleAnyToQuantum(ScaleQuantumToAny(p->blue,depth,
            scale),depth,scale);
        if ((channel & OpacityChannel) != 0)
          p->opacity=ScaleAnyToQuantum(ScaleQuantumToAny(p->opacity,depth,
            scale),depth,scale);
        p++;
      }
    }
  image->depth=depth;
  return(status);
}
