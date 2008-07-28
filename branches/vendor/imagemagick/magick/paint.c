/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                      PPPP    AAA   IIIII  N   N  TTTTT                      %
%                      P   P  A   A    I    NN  N    T                        %
%                      PPPP   AAAAA    I    N N N    T                        %
%                      P      A   A    I    N  NN    T                        %
%                      P      A   A  IIIII  N   N    T                        %
%                                                                             %
%                                                                             %
%                        Methods to Paint on an Image                         %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1998                                   %
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
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/draw.h"
#include "magick/draw-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/string_.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F l o o d f i l l P a i n t I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  FloodfillPaintImage() changes the color value of any pixel that matches
%  target and is an immediate neighbor.  If the method FillToBorderMethod is
%  specified, the color value is changed for any neighbor pixel that does not
%  match the bordercolor member of image.
%
%  By default target must match a particular pixel color exactly.
%  However, in many cases two colors may differ by a small amount.  The
%  fuzz member of image defines how much tolerance is acceptable to
%  consider two colors as the same.  For example, set fuzz to 10 and the
%  color red at intensities of 100 and 102 respectively are now
%  interpreted as the same color for the purposes of the floodfill.
%
%  The format of the FloodfillPaintImage method is:
%
%      MagickBooleanType FloodfillPaintImage(Image *image,
%        const ChannelType channel,const DrawInfo *draw_info,
%        const MagickPixelPacket target,const long x_offset,const long y_offset,
%        const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel(s).
%
%    o draw_info: the draw info.
%
%    o target: the RGB value of the target color.
%
%    o x_offset,y_offset: the starting location of the operation.
%
%    o invert: paint any pixel that does not match the target color.
%
*/
MagickExport MagickBooleanType FloodfillPaintImage(Image *image,
  const ChannelType channel,const DrawInfo *draw_info,
  const MagickPixelPacket *target,const long x_offset,const long y_offset,
  const MagickBooleanType invert)
{
#define MaxStacksize  (1UL << 15)
#define PushSegmentStack(up,left,right,delta) \
{ \
  if (s >= (segment_stack+MaxStacksize)) \
    ThrowBinaryException(DrawError,"SegmentStackOverflow",image->filename) \
  else \
    { \
      if ((((up)+(delta)) >= 0) && (((up)+(delta)) < (long) image->rows)) \
        { \
          s->x1=(double) (left); \
          s->y1=(double) (up); \
          s->x2=(double) (right); \
          s->y2=(double) (delta); \
          s++; \
        } \
    } \
}

  Image
    *floodplane_image;

  long
    offset,
    start,
    x1,
    x2,
    y;

  MagickBooleanType
    skip;

  MagickPixelPacket
    fill,
    pixel;

  PixelPacket
    fill_color;

  register const PixelPacket
    *p;

  register IndexPacket
    *indexes;

  register long
    x;

  register PixelPacket
    *q;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(draw_info != (DrawInfo *) NULL);
  assert(draw_info->signature == MagickSignature);
  if ((x_offset < 0) || (x_offset >= (long) image->columns))
    return(MagickFalse);
  if ((y_offset < 0) || (y_offset >= (long) image->rows))
    return(MagickFalse);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,ResetAlphaChannel);
  /*
    Set floodfill state.
  */
  floodplane_image=CloneImage(image,image->columns,image->rows,MagickTrue,
    &image->exception);
  if (floodplane_image == (Image *) NULL)
    return(MagickFalse);
  (void) SetImageAlphaChannel(floodplane_image,ResetAlphaChannel);
  segment_stack=(SegmentInfo *) AcquireQuantumMemory(MaxStacksize,
    sizeof(*segment_stack));
  if (segment_stack == (SegmentInfo *) NULL)
    {
      floodplane_image=DestroyImage(floodplane_image);
      ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
        image->filename);
    }
  /*
    Push initial segment on stack.
  */
  x=x_offset;
  y=y_offset;
  start=0;
  s=segment_stack;
  PushSegmentStack(y,x,x,1);
  PushSegmentStack(y+1,x,x,-1);
  GetMagickPixelPacket(image,&fill);
  GetMagickPixelPacket(image,&pixel);
  while (s > segment_stack)
  {
    /*
      Pop segment off stack.
    */
    s--;
    x1=(long) s->x1;
    x2=(long) s->x2;
    offset=(long) s->y2;
    y=(long) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    p=AcquireImagePixels(image,0,y,(unsigned long) (x1+1),1,&image->exception);
    q=GetImagePixels(floodplane_image,0,y,(unsigned long) (x1+1),1);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    indexes=GetIndexes(image);
    p+=x1;
    q+=x1;
    for (x=x1; x >= 0; x--)
    {
      if (q->opacity == (Quantum) TransparentOpacity)
        break;
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if (IsMagickColorSimilar(&pixel,target) == invert)
        break;
      q->opacity=(Quantum) TransparentOpacity;
      p--;
      q--;
    }
    if (SyncImagePixels(floodplane_image) == MagickFalse)
      break;
    skip=x >= x1 ? MagickTrue : MagickFalse;
    if (skip == MagickFalse)
      {
        start=x+1;
        if (start < x1)
          PushSegmentStack(y,start,x1-1,-offset);
        x=x1+1;
      }
    do
    {
      if (skip == MagickFalse)
        {
          if (x < (long) image->columns)
            {
              p=AcquireImagePixels(image,x,y,image->columns-x,1,
                &image->exception);
              q=GetImagePixels(floodplane_image,x,y,image->columns-x,1);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              indexes=GetIndexes(image);
              for ( ; x < (long) image->columns; x++)
              {
                if (q->opacity == (Quantum) TransparentOpacity)
                  break;
                SetMagickPixelPacket(image,p,indexes+x,&pixel);
                if (IsMagickColorSimilar(&pixel,target) == invert)
                  break;
                q->opacity=(Quantum) TransparentOpacity;
                p++;
                q++;
              }
              if (SyncImagePixels(floodplane_image) == MagickFalse)
                break;
            }
          PushSegmentStack(y,start,x-1,offset);
          if (x > (x2+1))
            PushSegmentStack(y,x2+1,x-1,-offset);
        }
      skip=MagickFalse;
      x++;
      if (x <= x2)
        {
          p=AcquireImagePixels(image,x,y,(unsigned long) (x2-x+1),1,
            &image->exception);
          q=GetImagePixels(floodplane_image,x,y,(unsigned long) (x2-x+1),1);
          if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
            break;
          indexes=GetIndexes(image);
          for ( ; x <= x2; x++)
          {
            if (q->opacity == (Quantum) TransparentOpacity)
              break;
            SetMagickPixelPacket(image,p,indexes+x,&pixel);
            if (IsMagickColorSimilar(&pixel,target) != invert)
              break;
            p++;
            q++;
          }
        }
      start=x;
    } while (x <= x2);
  }
  for (y=0; y < (long) image->rows; y++)
  {
    /*
      Tile fill color onto floodplane.
    */
    p=AcquireImagePixels(floodplane_image,0,y,image->columns,1,
      &image->exception);
    q=GetImagePixels(image,0,y,image->columns,1);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    indexes=GetIndexes(image);
    for (x=0; x < (long) image->columns; x++)
    {
      if (p->opacity != OpaqueOpacity)
        {
          fill_color=GetFillColor(draw_info,x,y);
          SetMagickPixelPacket(image,&fill_color,(IndexPacket *) NULL,&fill);
          if (image->colorspace == CMYKColorspace)
            ConvertRGBToCMYK(&fill);
          if ((channel & RedChannel) != 0)
            q->red=RoundToQuantum(fill.red);
          if ((channel & GreenChannel) != 0)
            q->green=RoundToQuantum(fill.green);
          if ((channel & BlueChannel) != 0)
            q->blue=RoundToQuantum(fill.blue);
          if ((channel & OpacityChannel) != 0)
            q->opacity=RoundToQuantum(fill.opacity);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            indexes[x]=RoundToQuantum(fill.index);
        }
      p++;
      q++;
    }
    if (SyncImagePixels(image) == MagickFalse)
      break;
  }
  segment_stack=(SegmentInfo *) RelinquishMagickMemory(segment_stack);
  floodplane_image=DestroyImage(floodplane_image);
  return(y == (long) image->rows ? MagickTrue : MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O i l P a i n t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OilPaintImage() applies a special effect filter that simulates an oil
%  painting.  Each pixel is replaced by the most frequent color occurring
%  in a circular region defined by radius.
%
%  The format of the OilPaintImage method is:
%
%      Image *OilPaintImage(const Image *image,const double radius,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o radius: the radius of the circular neighborhood.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static unsigned long **DestroyHistogramThreadSet(unsigned long **histogram)
{
  register long
    i;

  assert(histogram != (unsigned long **) NULL);
  for (i=0; i < (long) GetCacheViewMaximumThreads(); i++)
    if (histogram[i] != (unsigned long *) NULL)
      histogram[i]=(unsigned long *) RelinquishMagickMemory(histogram[i]);
  return((unsigned long **) RelinquishMagickMemory(histogram));
}

static unsigned long **AcquireHistogramThreadSet(const size_t count)
{
  register long
    i;

  unsigned long
    **histogram;

  histogram=(unsigned long **) AcquireQuantumMemory(
    GetCacheViewMaximumThreads(),sizeof(*histogram));
  if (histogram == (unsigned long **) NULL)
    return((unsigned long **) NULL);
  (void) ResetMagickMemory(histogram,0,GetCacheViewMaximumThreads()*
    sizeof(*histogram));
  for (i=0; i < (long) GetCacheViewMaximumThreads(); i++)
  {
    histogram[i]=AcquireQuantumMemory(count,sizeof(**histogram));
    if (histogram[i] == (unsigned long *) NULL)
      return(DestroyHistogramThreadSet(histogram));
  }
  return(histogram);
}

MagickExport Image *OilPaintImage(const Image *image,const double radius,
  ExceptionInfo *exception)
{
#define NumberPaintBins  256
#define OilPaintImageTag  "OilPaint/Image"

  Image
    *paint_image;

  long
    y;

  MagickBooleanType
    status;

  unsigned long
    **histogram,
    width;

  ViewInfo
    **image_view,
    **paint_view;

  /*
    Initialize painted image attributes.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  width=GetOptimalKernelWidth2D(radius,0.5);
  if ((image->columns < width) || (image->rows < width))
    ThrowImageException(OptionError,"ImageSmallerThanRadius");
  paint_image=CloneImage(image,0,0,MagickTrue,exception);
  if (paint_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(paint_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&paint_image->exception);
      paint_image=DestroyImage(paint_image);
      return((Image *) NULL);
    }
  histogram=AcquireHistogramThreadSet(NumberPaintBins);
  if (histogram == (unsigned long **) NULL)
    {
      paint_image=DestroyImage(paint_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Paint each row of the image.
  */
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  paint_view=AcquireCacheViewThreadSet(paint_image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    const IndexPacket
      *indexes;

    IndexPacket
      *paint_indexes;

    register const PixelPacket
      *p;

    register long
      id,
      x;

    register PixelPacket
      *q;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    p=AcquireCacheViewPixels(image_view[id],-((long) width/2L),y-(long)
      (width/2L),image->columns+width,width,exception);
    q=GetCacheViewPixels(paint_view[id],0,y,paint_image->columns,1);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    indexes=AcquireCacheViewIndexes(image_view[id]);
    paint_indexes=GetCacheViewIndexes(paint_view[id]);
    for (x=0; x < (long) image->columns; x++)
    {
      long
        k,
        v;

      register long
        i,
        u;

      unsigned long
        count;

      /*
        Assign most frequent color.
      */
      i=0;
      count=0;
      (void) ResetMagickMemory(histogram[id],0,NumberPaintBins*
        sizeof(**histogram));
      for (v=0; v < (long) width; v++)
      {
        for (u=0; u < (long) width; u++)
        {
          k=(long) ScaleQuantumToChar(PixelIntensityToQuantum(p+u+i));
          histogram[id][k]++;
          if (histogram[id][k] > count)
            {
              *q=(*(p+u+i));
              if ((indexes != (const IndexPacket *) NULL) &&
                  (paint_indexes != (const IndexPacket *) NULL))
                paint_indexes[x]=indexes[x+u+i];
              count=histogram[id][k];
            }
        }
        i+=image->columns+width;
      }
      p++;
      q++;
    }
    if (SyncCacheView(paint_view[id]) == MagickFalse)
      status=MagickFalse;
    if (SetImageProgress(image,OilPaintImageTag,y,image->rows) == MagickFalse)
      status=MagickFalse;
  }
  paint_view=DestroyCacheViewThreadSet(paint_view);
  image_view=DestroyCacheViewThreadSet(image_view);
  histogram=DestroyHistogramThreadSet(histogram);
  if (status == MagickFalse)
    paint_image=DestroyImage(paint_image);
  return(paint_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     O p a q u e P a i n t I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  OpaquePaintImage() changes any pixel that matches color with the color
%  defined by fill.
%
%  By default color must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  Fuzz defines
%  how much tolerance is acceptable to consider two colors as the same.
%  For example, set fuzz to 10 and the color red at intensities of 100 and
%  102 respectively are now interpreted as the same color.
%
%  The format of the OpaquePaintImage method is:
%
%      MagickBooleanType OpaquePaintImage(Image *image,
%        const PixelPacket *target,const PixelPacket *fill,
%        const MagickBooleanType invert)
%      MagickBooleanType OpaquePaintImageChannel(Image *image,
%        const ChannelType channel,const PixelPacket *target,
%        const PixelPacket *fill,const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: the channel(s).
%
%    o target: the RGB value of the target color.
%
%    o fill: the replacement color.
%
%    o invert: paint any pixel that does not match the target color.
%
*/

MagickExport MagickBooleanType OpaquePaintImage(Image *image,
  const MagickPixelPacket *target,const MagickPixelPacket *fill,
  const MagickBooleanType invert)
{
  return(OpaquePaintImageChannel(image,AllChannels,target,fill,invert));
}

MagickExport MagickBooleanType OpaquePaintImageChannel(Image *image,
  const ChannelType channel,const MagickPixelPacket *target,
  const MagickPixelPacket *fill,const MagickBooleanType invert)
{
#define OpaquePaintImageTag  "Opaque/Image"

  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **image_view;

  /*
    Make image color opaque.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(target != (MagickPixelPacket *) NULL);
  assert(fill != (MagickPixelPacket *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      progress;

    MagickPixelPacket
      pixel;

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
    GetMagickPixelPacket(image,&pixel);
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,q,indexes+x,&pixel);
      if (IsMagickColorSimilar(&pixel,target) != invert)
        {
          if ((channel & RedChannel) != 0)
            q->red=RoundToQuantum(fill->red);
          if ((channel & GreenChannel) != 0)
            q->green=RoundToQuantum(fill->green);
          if ((channel & BlueChannel) != 0)
            q->blue=RoundToQuantum(fill->blue);
          if ((channel & OpacityChannel) != 0)
            q->opacity=RoundToQuantum(fill->opacity);
          if (((channel & IndexChannel) != 0) &&
              (image->colorspace == CMYKColorspace))
            indexes[x]=RoundToQuantum(fill->index);
        }
      q++;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      status=MagickFalse;
    progress=SetImageProgress(image,OpaquePaintImageTag,y,image->rows);
    if (progress == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheViewThreadSet(image_view);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T r a n s p a r e n t P a i n t I m a g e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TransparentPaintImage() changes the opacity value associated with any pixel
%  that matches color to the value defined by opacity.
%
%  By default color must match a particular pixel color exactly.  However,
%  in many cases two colors may differ by a small amount.  Fuzz defines
%  how much tolerance is acceptable to consider two colors as the same.
%  For example, set fuzz to 10 and the color red at intensities of 100 and
%  102 respectively are now interpreted as the same color.
%
%  The format of the TransparentPaintImage method is:
%
%      MagickBooleanType TransparentPaintImage(Image *image,
%        const MagickPixelPacket *target,const Quantum opacity,
%        const MagickBooleanType invert)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o target: the target color.
%
%    o opacity: the replacement opacity value.
%
%    o invert: paint any pixel that does not match the target color.
%
*/
MagickExport MagickBooleanType TransparentPaintImage(Image *image,
  const MagickPixelPacket *target,const Quantum opacity,
  const MagickBooleanType invert)
{
#define TransparentPaintImageTag  "Transparent/Image"

  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **image_view;

  /*
    Make image color transparent.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  assert(target != (MagickPixelPacket *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->matte == MagickFalse)
    (void) SetImageAlphaChannel(image,ResetAlphaChannel);
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    MagickBooleanType
      progress;

    MagickPixelPacket
      pixel;

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
    GetMagickPixelPacket(image,&pixel);
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,q,indexes+x,&pixel);
      if (IsMagickColorSimilar(&pixel,target) != invert)
        q->opacity=opacity;
      q++;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      status=MagickFalse;
    progress=SetImageProgress(image,TransparentPaintImageTag,y,image->rows);
    if (progress == MagickFalse)
      status=MagickFalse;
  }
  image_view=AcquireCacheViewThreadSet(image);
  return(status);
}
