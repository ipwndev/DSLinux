/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     IIIII  M   M   AAA    GGGG  EEEEE                       %
%                       I    MM MM  A   A  G      E                           %
%                       I    M M M  AAAAA  G  GG  EEE                         %
%                       I    M   M  A   A  G   G  E                           %
%                     IIIII  M   M  A   A   GGGG  EEEEE                       %
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
#include "magick/animate.h"
#include "magick/artifact.h"
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
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/paint.h"
#include "magick/pixel-private.h"
#include "magick/profile.h"
#include "magick/property.h"
#include "magick/quantize.h"
#include "magick/random_.h"
#include "magick/segment.h"
#include "magick/semaphore.h"
#include "magick/signature-private.h"
#include "magick/string_.h"
#include "magick/threshold.h"
#include "magick/timer.h"
#include "magick/utility.h"
#include "magick/version.h"
#include "magick/xwindow-private.h"

/*
  Constant declaration.
*/
const char
  *BackgroundColor = "#ffffff",  /* white */
  *BorderColor = "#dfdfdf",  /* gray */
  *DefaultTileFrame = "15x15+3+3",
  *DefaultTileGeometry = "120x120+4+3>",
  *DefaultTileLabel = "%f\n%wx%h\n%b",
  *ForegroundColor = "#000",  /* black */
  *LoadImageTag = "Load/Image",
  *LoadImagesTag = "Load/Images",
  *MatteColor = "#bdbdbd",  /* gray */
  *PSDensityGeometry = "72.0x72.0",
  *PSPageGeometry = "612x792",
  *SaveImageTag = "Save/Image",
  *SaveImagesTag = "Save/Images",
  *TransparentColor = "#00000000";  /* transparent black */

const double
  DefaultResolution = 72.0;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireImage() returns a pointer to an image structure initialized to
%  default values.
%
%  The format of the AcquireImage method is:
%
%      Image *AcquireImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: Many of the image default values are set from this
%      structure.  For example, filename, compression, depth, background color,
%      and others.
%
*/
MagickExport Image *AcquireImage(const ImageInfo *image_info)
{
  Image
    *image;

  MagickStatusType
    flags;

  /*
    Allocate image structure.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  image=(Image *) AcquireMagickMemory(sizeof(*image));
  if (image == (Image *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(image,0,sizeof(*image));
  /*
    Initialize Image structure.
  */
  (void) CopyMagickString(image->magick,"MIFF",MaxTextExtent);
  image->storage_class=DirectClass;
  image->depth=MAGICKCORE_QUANTUM_DEPTH;
  image->colorspace=RGBColorspace;
  image->interlace=NoInterlace;
  image->ticks_per_second=UndefinedTicksPerSecond;
  image->compose=OverCompositeOp;
  image->blur=1.0;
  GetExceptionInfo(&image->exception);
  (void) QueryColorDatabase(BackgroundColor,&image->background_color,
    &image->exception);
  (void) QueryColorDatabase(BorderColor,&image->border_color,&image->exception);
  (void) QueryColorDatabase(MatteColor,&image->matte_color,&image->exception);
  (void) QueryColorDatabase(TransparentColor,&image->transparent_color,
    &image->exception);
  image->x_resolution=DefaultResolution;
  image->y_resolution=DefaultResolution;
  image->units=PixelsPerInchResolution;
  GetTimerInfo(&image->timer);
  (void) GetCacheInfo(&image->cache);
  image->blob=CloneBlobInfo((BlobInfo *) NULL);
  image->debug=IsEventLogging();
  image->reference_count=1;
  image->signature=MagickSignature;
  if (image_info == (ImageInfo *) NULL)
    return(image);
  /*
    Transfer image info.
  */
  SetBlobExempt(image,image_info->file != (FILE *) NULL ? MagickTrue :
    MagickFalse);
  (void) CopyMagickString(image->filename,image_info->filename,MaxTextExtent);
  (void) CopyMagickString(image->magick_filename,image_info->filename,
    MaxTextExtent);
  (void) CopyMagickString(image->magick,image_info->magick,MaxTextExtent);
  if (image_info->size != (char *) NULL)
    {
      (void) ParseAbsoluteGeometry(image_info->size,&image->extract_info);
      image->columns=image->extract_info.width;
      image->rows=image->extract_info.height;
      image->offset=image->extract_info.x;
      image->extract_info.x=0;
      image->extract_info.y=0;
    }
  if (image_info->extract != (char *) NULL)
    {
      RectangleInfo
        geometry;

      flags=ParseAbsoluteGeometry(image_info->extract,&geometry);
      if (((flags & XValue) != 0) || ((flags & YValue) != 0))
        {
          image->extract_info=geometry;
          Swap(image->columns,image->extract_info.width);
          Swap(image->rows,image->extract_info.height);
        }
    }
  image->compression=image_info->compression;
  image->quality=image_info->quality;
  image->endian=image_info->endian;
  image->interlace=image_info->interlace;
  image->units=image_info->units;
  if (image_info->density != (char *) NULL)
    {
      GeometryInfo
        geometry_info;

      flags=ParseGeometry(image_info->density,&geometry_info);
      image->x_resolution=geometry_info.rho;
      image->y_resolution=geometry_info.sigma;
      if ((flags & SigmaValue) == 0)
        image->y_resolution=image->x_resolution;
    }
  if (image_info->page != (char *) NULL)
    {
      char
        *geometry;

      image->page=image->extract_info;
      geometry=GetPageGeometry(image_info->page);
      (void) ParseAbsoluteGeometry(geometry,&image->page);
      geometry=DestroyString(geometry);
    }
  if (image_info->depth != 0)
    image->depth=image_info->depth;
  image->dither=image_info->dither;
  image->background_color=image_info->background_color;
  image->border_color=image_info->border_color;
  image->matte_color=image_info->matte_color;
  image->transparent_color=image_info->transparent_color;
  image->progress_monitor=image_info->progress_monitor;
  image->client_data=image_info->client_data;
  if (image_info->cache != (void *) NULL)
    CloneCacheMethods(image->cache,image_info->cache);
  (void) SetImageVirtualPixelMethod(image,image_info->virtual_pixel_method);
  (void) SyncImageOptions(image_info,image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e I m a g e C o l o r m a p                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireImageColormap() allocates an image colormap and initializes
%  it to a linear gray colorspace.  If the image already has a colormap,
%  it is replaced.  AcquireImageColormap() returns MagickTrue if successful,
%  otherwise MagickFalse if there is not enough memory.
%
%  The format of the AcquireImageColormap method is:
%
%      MagickBooleanType AcquireImageColormap(Image *image,
%        const unsigned long colors)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o colors: the number of colors in the image colormap.
%
*/

static inline unsigned long MagickMax(const unsigned long x,
  const unsigned long y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline unsigned long MagickMin(const unsigned long x,
  const unsigned long y)
{
  if (x < y)
    return(x);
  return(y);
}

MagickExport MagickBooleanType AcquireImageColormap(Image *image,
  const unsigned long colors)
{
  register long
    i;

  size_t
    length;

  /*
    Allocate image colormap.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  image->colors=MagickMin(colors,MaxColormapSize);
  length=(size_t) colors;
  if (image->colormap == (PixelPacket *) NULL)
    image->colormap=(PixelPacket *) AcquireQuantumMemory(length,
      sizeof(*image->colormap));
  else
    image->colormap=(PixelPacket *) ResizeQuantumMemory(image->colormap,length,
      sizeof(*image->colormap));
  if (image->colormap == (PixelPacket *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  #pragma omp parallel for
  for (i=0; i < (long) image->colors; i++)
  {
    unsigned long
      pixel;

    pixel=(unsigned long) (i*(QuantumRange/MagickMax(colors-1,1)));
    image->colormap[i].red=(Quantum) pixel;
    image->colormap[i].green=(Quantum) pixel;
    image->colormap[i].blue=(Quantum) pixel;
    image->colormap[i].opacity=OpaqueOpacity;
  }
  return(SetImageStorageClass(image,PseudoClass));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e I m a g e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireImageInfo() allocates the ImageInfo structure.
%
%  The format of the AcquireImageInfo method is:
%
%      ImageInfo *AcquireImageInfo(void)
%
*/
MagickExport ImageInfo *AcquireImageInfo(void)
{
  ImageInfo
    *image_info;

  image_info=(ImageInfo *) AcquireMagickMemory(sizeof(*image_info));
  if (image_info == (ImageInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  GetImageInfo(image_info);
  return(image_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e N e x t I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireNextImage() initializes the next image in a sequence to
%  default values.  The next member of image points to the newly allocated
%  image.  If there is a memory shortage, next is assigned NULL.
%
%  The format of the AcquireNextImage method is:
%
%      void AcquireNextImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o image_info: Many of the image default values are set from this
%      structure.  For example, filename, compression, depth, background color,
%      and others.
%
%    o image: the image.
%
*/
MagickExport void AcquireNextImage(const ImageInfo *image_info,Image *image)
{
  /*
    Allocate image structure.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  image->next=AcquireImage(image_info);
  if (GetNextImageInList(image) == (Image *) NULL)
    return;
  (void) CopyMagickString(GetNextImageInList(image)->filename,image->filename,
    MaxTextExtent);
  if (image_info != (ImageInfo *) NULL)
    (void) CopyMagickString(GetNextImageInList(image)->filename,
      image_info->filename,MaxTextExtent);
  DestroyBlob(GetNextImageInList(image));
  image->next->blob=ReferenceBlob(image->blob);
  image->next->endian=image->endian;
  image->next->scene=image->scene+1;
  image->next->previous=image;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A p p e n d I m a g e s                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AppendImages() takes all images from the current image pointer to the end
%  of the image list and and appends them to each other top-to-bottom if the
%  stack parameter is true, otherwise left-to-right.
%
%  The format of the AppendImage method is:
%
%      Image *AppendImages(const Image *image,const MagickBooleanType stack,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
%    o stack: A value other than 0 stacks the images top-to-bottom.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport Image *AppendImages(const Image *image,
  const MagickBooleanType stack,ExceptionInfo *exception)
{
#define AppendImageTag  "Append/Image"

  Image
    *append_image;

  long
    n,
    y;

  MagickBooleanType
    matte,
    status;

  register const Image
    *next;

  register long
    i;

  unsigned long
    height,
    number_images,
    width;

  ViewInfo
    **append_view,
    **image_view;

  /*
    Ensure the image have the same column width.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  matte=image->matte;
  number_images=1;
  width=image->columns;
  height=image->rows;
  next=GetNextImageInList(image);
  for ( ; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if (next->matte != MagickFalse)
      matte=MagickTrue;
    number_images++;
    if (stack != MagickFalse)
      {
        if (next->columns > width)
          width=next->columns;
        height+=next->rows;
        continue;
      }
    width+=next->columns;
    if (next->rows > height)
      height=next->rows;
  }
  /*
    Initialize append next attributes.
  */
  append_image=CloneImage(image,width,height,MagickTrue,exception);
  if (append_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(append_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&append_image->exception);
      append_image=DestroyImage(append_image);
      return((Image *) NULL);
    }
  append_image->matte=matte;
  (void) SetImageBackgroundColor(append_image);
  status=MagickTrue;
  i=0;
  append_view=AcquireCacheViewThreadSet(append_image);
  if (stack != MagickFalse)
    {
      /*
        Stack top-to-bottom.
      */
      for (n=0; n < (long) number_images; n++)
      {
        image_view=AcquireCacheViewThreadSet(image);
        #pragma omp parallel for
        for (y=0; y < (long) image->rows; y++)
        {
          const IndexPacket
            *indexes;

          IndexPacket
            *append_indexes;

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
          p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,
            exception);
          q=SetCacheViewPixels(append_view[id],0,i+y,append_image->columns,1);
          if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
            {
              status=MagickFalse;
              continue;
            }
          indexes=AcquireCacheViewIndexes(image_view[id]);
          append_indexes=GetCacheViewIndexes(append_view[id]);
          for (x=0; x < (long) image->columns; x++)
          {
            q->red=p->red;
            q->green=p->green;
            q->blue=p->blue;
            q->opacity=p->opacity;
            if (append_image->colorspace == CMYKColorspace)
              append_indexes[x]=indexes[x];
            p++;
            q++;
          }
          if (SyncCacheView(append_view[id]) == MagickFalse)
            continue;
        }
        image_view=DestroyCacheViewThreadSet(image_view);
        if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
            (QuantumTick(n,number_images) != MagickFalse))
          {
            status=image->progress_monitor(AppendImageTag,n,number_images,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
        i+=image->rows;
        image=GetNextImageInList(image);
      }
      append_view=DestroyCacheViewThreadSet(append_view);
      if (status == MagickFalse)
        append_image=DestroyImage(append_image);
      return(append_image);
    }
  /*
    Stack left-to-right.
  */
  for (n=0; n < (long) number_images; n++)
  {
    image_view=AcquireCacheViewThreadSet(image);
    #pragma omp parallel for
    for (y=0; y < (long) image->rows; y++)
    {
      const IndexPacket
        *indexes;

      IndexPacket
        *append_indexes;

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
      p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
      q=SetCacheViewPixels(append_view[id],i,y,image->columns,1);
      if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
        {
          status=MagickFalse;
          continue;
        }
      indexes=AcquireCacheViewIndexes(image_view[id]);
      append_indexes=GetCacheViewIndexes(append_view[id]);
      for (x=0; x < (long) image->columns; x++)
      {
        q->red=p->red;
        q->green=p->green;
        q->blue=p->blue;
        q->opacity=p->opacity;
        if (append_image->colorspace == CMYKColorspace)
          append_indexes[x]=indexes[x];
        p++;
        q++;
      }
      if (SyncCacheView(append_view[id]) == MagickFalse)
        continue;
    }
    image_view=DestroyCacheViewThreadSet(image_view);
    if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
        (QuantumTick(n,number_images) != MagickFalse))
      {
        status=image->progress_monitor(AppendImageTag,n,number_images,
          image->client_data);
        if (status == MagickFalse)
          break;
      }
    i+=image->columns;
    image=GetNextImageInList(image);
  }
  append_view=DestroyCacheViewThreadSet(append_view);
  if (status == MagickFalse)
    append_image=DestroyImage(append_image);
  return(append_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A v e r a g e I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AverageImages() takes a set of images and averages them together.  Each
%  image in the set must have the same width and height.  AverageImages()
%  returns a single image with each corresponding pixel component of each
%  image averaged.   On failure, a NULL image is returned and exception
%  describes the reason for the failure.
%
%  The format of the AverageImages method is:
%
%      Image *AverageImages(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image sequence.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

static MagickPixelPacket **DestroyPixelThreadSet(MagickPixelPacket **pixels)
{
  register long
    i;

  assert(pixels != (MagickPixelPacket **) NULL);
  for (i=0; i < (long) GetCacheViewMaximumThreads(); i++)
    if (pixels[i] != (MagickPixelPacket *) NULL)
      pixels[i]=(MagickPixelPacket *) RelinquishMagickMemory(pixels[i]);
  return((MagickPixelPacket **) RelinquishMagickMemory(pixels));
}

static MagickPixelPacket **AcquirePixelThreadSet(const Image *image)
{
  register long
    i,
    j;

  MagickPixelPacket
    **pixels;

  pixels=(MagickPixelPacket **) AcquireQuantumMemory(
    GetCacheViewMaximumThreads(),sizeof(*pixels));
  if (pixels == (MagickPixelPacket **) NULL)
    return((MagickPixelPacket **) NULL);
  (void) ResetMagickMemory(pixels,0,GetCacheViewMaximumThreads()*
    sizeof(*pixels));
  for (i=0; i < (long) GetCacheViewMaximumThreads(); i++)
  {
    pixels[i]=AcquireQuantumMemory(image->columns,sizeof(**pixels));
    if (pixels[i] == (MagickPixelPacket *) NULL)
      return(DestroyPixelThreadSet(pixels));
    for (j=0; j < (long) image->columns; j++)
      GetMagickPixelPacket(image,&pixels[i][j]);
  }
  return(pixels);
}

MagickExport Image *AverageImages(const Image *image,ExceptionInfo *exception)
{
#define AverageImageTag  "Average/Image"

  const Image
    *next;

  Image
    *average_image;

  long
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    **average_pixel;

  unsigned long
    number_images;

  ViewInfo
    **average_view;

  /*
    Ensure the image are the same size.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
    if ((next->columns != image->columns) || (next->rows != image->rows))
      ThrowImageException(OptionError,"ImageWidthsOrHeightsDiffer");
  /*
    Initialize average next attributes.
  */
  average_image=CloneImage(image,0,0,MagickTrue,exception);
  if (average_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(average_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&average_image->exception);
      average_image=DestroyImage(average_image);
      return((Image *) NULL);
    }
  average_pixel=AcquirePixelThreadSet(image);
  if (average_pixel == (MagickPixelPacket **) NULL)
    {
      average_image=DestroyImage(average_image);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  /*
    Average image pixels.
  */
  status=MagickTrue;
  number_images=GetImageListLength(image);
  average_view=AcquireCacheViewThreadSet(average_image);
  #pragma omp parallel for
  for (y=0; y < (long) average_image->rows; y++)
  {
    const Image
      *next;

    IndexPacket
      *average_indexes;

    MagickBooleanType
      progress;

    MagickPixelPacket
      pixel;

    register long
      i,
      id,
      x;

    register PixelPacket
      *q;

    ViewInfo
      *image_view;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    q=GetCacheViewPixels(average_view[id],0,y,average_image->columns,1);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    GetMagickPixelPacket(image,&pixel);
    average_indexes=GetCacheViewIndexes(average_view[id]);
    for (x=0; x < (long) average_image->columns; x++)
    {
      average_pixel[id][x].red=0.0;
      average_pixel[id][x].green=0.0;
      average_pixel[id][x].blue=0.0;
      average_pixel[id][x].opacity=0.0;
      if (average_image->colorspace == CMYKColorspace)
        average_pixel[id][x].index=0.0;
    }
    next=image;
    for (i=0; i < (long) number_images; i++)
    {
      IndexPacket
        *indexes;

      register const PixelPacket
        *p;

      register long
        x;

      image_view=AcquireCacheView(next);
      p=AcquireCacheViewPixels(image_view,0,y,next->columns,1,exception);
      if (p == (const PixelPacket *) NULL)
        break;
      indexes=GetCacheViewIndexes(image_view);
      for (x=0; x < (long) next->columns; x++)
      {
        SetMagickPixelPacket(next,p,indexes+x,&pixel);
        average_pixel[id][x].red+=QuantumScale*pixel.red;
        average_pixel[id][x].green+=QuantumScale*pixel.green;
        average_pixel[id][x].blue+=QuantumScale*pixel.blue;
        average_pixel[id][x].opacity+=QuantumScale*pixel.opacity;
        if (average_image->colorspace == CMYKColorspace)
          average_pixel[id][x].index+=QuantumScale*pixel.index;
        p++;
      }
      image_view=DestroyCacheView(image_view);
      next=GetNextImageInList(next);
    }
    for (x=0; x < (long) average_image->columns; x++)
    {
      average_pixel[id][x].red=(MagickRealType) QuantumRange*
        average_pixel[id][x].red/number_images;
      average_pixel[id][x].green=(MagickRealType) QuantumRange*
        average_pixel[id][x].green/number_images;
      average_pixel[id][x].blue=(MagickRealType) QuantumRange*
        average_pixel[id][x].blue/number_images;
      average_pixel[id][x].opacity=(MagickRealType) QuantumRange*
        average_pixel[id][x].opacity/number_images;
      if (average_image->colorspace == CMYKColorspace)
        average_pixel[id][x].index=(MagickRealType) QuantumRange*
          average_pixel[id][x].index/number_images;
      SetPixelPacket(average_image,&average_pixel[id][x],q,average_indexes+x);
      q++;
    }
    if (SyncCacheView(average_view[id]) == MagickFalse)
      status=MagickFalse;
    progress=SetImageProgress(image,AverageImageTag,y,average_image->rows);
    if (progress == MagickFalse)
      status=MagickFalse;
  }
  average_view=DestroyCacheViewThreadSet(average_view);
  average_pixel=DestroyPixelThreadSet(average_pixel);
  if (status == MagickFalse)
    average_image=DestroyImage(average_image);
  return(average_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C a t c h I m a g e E x c e p t i o n                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CatchImageException() returns if no exceptions are found in the image
%  sequence, otherwise it determines the most severe exception and reports
%  it as a warning or error depending on the severity.
%
%  The format of the CatchImageException method is:
%
%      ExceptionType CatchImageException(Image *image)
%
%  A description of each parameter follows:
%
%    o image: An image sequence.
%
*/
MagickExport ExceptionType CatchImageException(Image *image)
{
  ExceptionInfo
    *exception;

  ExceptionType
    severity;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  exception=AcquireExceptionInfo();
  GetImageException(image,exception);
  CatchException(exception);
  severity=exception->severity;
  exception=DestroyExceptionInfo(exception);
  return(severity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l i p I m a g e P a t h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ClipImagePath() sets the image clip mask based any clipping path information
%  if it exists.
%
%  The format of the ClipImagePath method is:
%
%      MagickBooleanType ClipImagePath(Image *image,const char *pathname,
%        const MagickBooleanType inside)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o pathname: name of clipping path resource. If name is preceded by #, use
%      clipping path numbered by name.
%
%    o inside: if non-zero, later operations take effect inside clipping path.
%      Otherwise later operations take effect outside clipping path.
%
*/

MagickExport MagickBooleanType ClipImage(Image *image)
{
  return(ClipImagePath(image,"#1",MagickTrue));
}

MagickExport MagickBooleanType ClipImagePath(Image *image,const char *pathname,
  const MagickBooleanType inside)
{
#define ClipImagePathTag  "ClipPath/Image"

  char
    *property;

  const char
    *value;

  Image
    *clip_mask;

  ImageInfo
    *image_info;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(pathname != NULL);
  property=AcquireString(pathname);
  (void) FormatMagickString(property,MaxTextExtent,"8BIM:1999,2998:%s",
    pathname);
  value=GetImageProperty(image,property);
  property=DestroyString(property);
  if (value == (const char *) NULL)
    {
      ThrowFileException(&image->exception,OptionError,"NoClipPathDefined",
        image->filename);
      return(MagickFalse);
    }
  image_info=AcquireImageInfo();
  clip_mask=BlobToImage(image_info,value,strlen(value),&image->exception);
  image_info=DestroyImageInfo(image_info);
  if (clip_mask == (Image *) NULL)
    return(MagickFalse);
  if (clip_mask->storage_class == PseudoClass)
    {
      (void) SyncImage(clip_mask);
      if (SetImageStorageClass(clip_mask,DirectClass) == MagickFalse)
        return(MagickFalse);
    }
  if (inside == MagickFalse)
    (void) NegateImage(clip_mask,MagickFalse);
  (void) FormatMagickString(clip_mask->magick_filename,MaxTextExtent,
    "8BIM:1999,2998:%s\nPS",pathname);
  (void) SetImageClipMask(image,clip_mask);
  clip_mask=DestroyImage(clip_mask);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImage() copies an image and returns the copy as a new image object.
%  If the specified columns and rows is 0, an exact copy of the image is
%  returned, otherwise the pixel data is undefined and must be initialized
%  with the SetImagePixels() and SyncImagePixels() methods.  On failure,
%  a NULL image is returned and exception describes the reason for the
%  failure.
%
%  The format of the CloneImage method is:
%
%      Image *CloneImage(const Image *image,const unsigned long columns,
%        const unsigned long rows,const MagickBooleanType orphan,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns: the number of columns in the cloned image.
%
%    o rows: the number of rows in the cloned image.
%
%    o orphan:  With a value other than 0, the cloned image is an orphan.  An
%      orphan is a stand-alone image that is not assocated with an image list.
%      In effect, the next and previous members of the cloned image is set to
%      NULL.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport Image *CloneImage(const Image *image,const unsigned long columns,
  const unsigned long rows,const MagickBooleanType orphan,
  ExceptionInfo *exception)
{
  Image
    *clone_image;

  MagickRealType
    scale;

  size_t
    length;

  /*
    Clone the image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  clone_image=(Image *) AcquireMagickMemory(sizeof(*clone_image));
  if (clone_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(clone_image,0,sizeof(*clone_image));
  clone_image->signature=MagickSignature;
  clone_image->storage_class=image->storage_class;
  clone_image->colorspace=image->colorspace;
  clone_image->matte=image->matte;
  clone_image->columns=image->columns;
  clone_image->rows=image->rows;
  clone_image->dither=image->dither;
  if (image->colormap != (PixelPacket *) NULL)
    {
      /*
        Allocate and copy the image colormap.
      */
      clone_image->colors=image->colors;
      length=(size_t) image->colors;
      clone_image->colormap=(PixelPacket *) AcquireQuantumMemory(length,
        sizeof(*clone_image->colormap));
      if (clone_image->colormap == (PixelPacket *) NULL)
        ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
      (void) CopyMagickMemory(clone_image->colormap,image->colormap,length*
        sizeof(*clone_image->colormap));
    }
  (void) CloneImageProfiles(clone_image,image);
  (void) CloneImageProperties(clone_image,image);
  (void) CloneImageArtifacts(clone_image,image);
  GetTimerInfo(&clone_image->timer);
  GetExceptionInfo(&clone_image->exception);
  InheritException(&clone_image->exception,&image->exception);
  if (image->ascii85 != (void *) NULL)
    Ascii85Initialize(clone_image);
  clone_image->magick_columns=image->magick_columns;
  clone_image->magick_rows=image->magick_rows;
  clone_image->type=image->type;
  (void) CopyMagickString(clone_image->magick_filename,image->magick_filename,
    MaxTextExtent);
  (void) CopyMagickString(clone_image->magick,image->magick,MaxTextExtent);
  (void) CopyMagickString(clone_image->filename,image->filename,MaxTextExtent);
  clone_image->progress_monitor=image->progress_monitor;
  clone_image->client_data=image->client_data;
  clone_image->reference_count=1;
  clone_image->previous=NewImageList();
  clone_image->list=NewImageList();
  clone_image->next=NewImageList();
  clone_image->clip_mask=NewImageList();
  clone_image->mask=NewImageList();
  clone_image->blob=ReferenceBlob(image->blob);
  clone_image->cache=ReferenceCache(image->cache);
  clone_image->debug=IsEventLogging();
  if (orphan == MagickFalse)
    {
      if (GetPreviousImageInList(image) != (Image *) NULL)
        clone_image->previous->next=clone_image;
      if (GetNextImageInList(image) != (Image *) NULL)
        clone_image->next->previous=clone_image;
    }
  if ((columns == 0) && (rows == 0))
    {
      if (image->montage != (char *) NULL)
        (void) CloneString(&clone_image->montage,image->montage);
      if (image->directory != (char *) NULL)
        (void) CloneString(&clone_image->directory,image->directory);
      if (image->clip_mask != (Image *) NULL)
        clone_image->clip_mask=CloneImage(image->clip_mask,0,0,MagickTrue,
          exception);
      if (image->mask != (Image *) NULL)
        clone_image->mask=CloneImage(image->mask,0,0,MagickTrue,exception);
      return(clone_image);
    }
  (void) SetImageExtent(clone_image,columns,rows);
  scale=(MagickRealType) clone_image->columns/(MagickRealType) image->columns;
  clone_image->page.width=(unsigned long) (scale*image->page.width+0.5);
  clone_image->page.x=(long) (scale*image->page.x+0.5);
  clone_image->tile_offset.x=(long) (scale*image->tile_offset.x+0.5);
  scale=(MagickRealType) clone_image->rows/(MagickRealType) image->rows;
  clone_image->page.height=(unsigned long) (scale*image->page.height+0.5);
  clone_image->page.y=(long) (image->page.y*scale+0.5);
  clone_image->tile_offset.y=(long) (scale*image->tile_offset.y+0.5);
  return(clone_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e I m a g e I n f o                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneImageInfo() makes a copy of the given image info structure.  If
%  NULL is specified, a new image info structure is created initialized to
%  default values.
%
%  The format of the CloneImageInfo method is:
%
%      ImageInfo *CloneImageInfo(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport ImageInfo *CloneImageInfo(const ImageInfo *image_info)
{
  ImageInfo
    *clone_info;

  clone_info=AcquireImageInfo();
  if (image_info == (ImageInfo *) NULL)
    return(clone_info);
  clone_info->compression=image_info->compression;
  clone_info->temporary=image_info->temporary;
  clone_info->adjoin=image_info->adjoin;
  clone_info->antialias=image_info->antialias;
  clone_info->scene=image_info->scene;
  clone_info->number_scenes=image_info->number_scenes;
  clone_info->depth=image_info->depth;
  if (image_info->size != (char *) NULL)
    (void) CloneString(&clone_info->size,image_info->size);
  if (image_info->extract != (char *) NULL)
    (void) CloneString(&clone_info->extract,image_info->extract);
  if (image_info->scenes != (char *) NULL)
    (void) CloneString(&clone_info->scenes,image_info->scenes);
  if (image_info->page != (char *) NULL)
    (void) CloneString(&clone_info->page,image_info->page);
  clone_info->interlace=image_info->interlace;
  clone_info->endian=image_info->endian;
  clone_info->units=image_info->units;
  clone_info->quality=image_info->quality;
  if (image_info->sampling_factor != (char *) NULL)
    (void) CloneString(&clone_info->sampling_factor,
      image_info->sampling_factor);
  if (image_info->server_name != (char *) NULL)
    (void) CloneString(&clone_info->server_name,image_info->server_name);
  if (image_info->font != (char *) NULL)
    (void) CloneString(&clone_info->font,image_info->font);
  if (image_info->texture != (char *) NULL)
    (void) CloneString(&clone_info->texture,image_info->texture);
  if (image_info->density != (char *) NULL)
    (void) CloneString(&clone_info->density,image_info->density);
  clone_info->pointsize=image_info->pointsize;
  clone_info->fuzz=image_info->fuzz;
  clone_info->pen=image_info->pen;
  clone_info->background_color=image_info->background_color;
  clone_info->border_color=image_info->border_color;
  clone_info->matte_color=image_info->matte_color;
  clone_info->transparent_color=image_info->transparent_color;
  clone_info->dither=image_info->dither;
  clone_info->monochrome=image_info->monochrome;
  clone_info->colors=image_info->colors;
  clone_info->colorspace=image_info->colorspace;
  clone_info->type=image_info->type;
  clone_info->orientation=image_info->orientation;
  clone_info->preview_type=image_info->preview_type;
  clone_info->group=image_info->group;
  clone_info->ping=image_info->ping;
  clone_info->verbose=image_info->verbose;
  if (image_info->view != (char *) NULL)
    (void) CloneString(&clone_info->view,image_info->view);
  if (image_info->authenticate != (char *) NULL)
    (void) CloneString(&clone_info->authenticate,image_info->authenticate);
  (void) CloneImageOptions(clone_info,image_info);
  clone_info->progress_monitor=image_info->progress_monitor;
  clone_info->client_data=image_info->client_data;
  clone_info->cache=image_info->cache;
  if (image_info->cache != (void *) NULL)
    clone_info->cache=ReferenceCache(image_info->cache);
  if (image_info->profile != (void *) NULL)
    clone_info->profile=(void *) CloneStringInfo((StringInfo *)
      image_info->profile);
  SetImageInfoFile(clone_info,image_info->file);
  SetImageInfoBlob(clone_info,image_info->blob,image_info->length);
  clone_info->stream=image_info->stream;
  clone_info->virtual_pixel_method=image_info->virtual_pixel_method;
  (void) CopyMagickString(clone_info->magick,image_info->magick,MaxTextExtent);
  (void) CopyMagickString(clone_info->unique,image_info->unique,MaxTextExtent);
  (void) CopyMagickString(clone_info->zero,image_info->zero,MaxTextExtent);
  (void) CopyMagickString(clone_info->filename,image_info->filename,
    MaxTextExtent);
  clone_info->subimage=image_info->scene;
  clone_info->subrange=image_info->number_scenes;
  clone_info->channel=image_info->channel;
  clone_info->debug=IsEventLogging();
  clone_info->signature=image_info->signature;
  return(clone_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C o m b i n e I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CombineImages() combines one or more images into a single image.  The
%  grayscale value of the pixels of each image in the sequence is assigned in
%  order to the specified channels of the combined image.   The typical
%  ordering would be image 1 => Red, 2 => Green, 3 => Blue, etc.
%
%  The format of the CombineImages method is:
%
%      Image *CombineImages(const Image *image,const ChannelType channel,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport Image *CombineImages(const Image *image,const ChannelType channel,
  ExceptionInfo *exception)
{
#define CombineImageTag  "Combine/Image"

  const Image
    *next;

  Image
    *combine_image;

  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **combine_view;

  /*
    Ensure the image are the same size.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if ((next->columns != image->columns) || (next->rows != image->rows))
      ThrowImageException(OptionError,"ImagesAreNotTheSameSize");
  }
  combine_image=CloneImage(image,0,0,MagickTrue,exception);
  if (combine_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(combine_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&combine_image->exception);
      combine_image=DestroyImage(combine_image);
      return((Image *) NULL);
    }
  if ((channel & OpacityChannel) != 0)
    combine_image->matte=MagickTrue;
  (void) SetImageBackgroundColor(combine_image);
  status=MagickTrue;
  combine_view=AcquireCacheViewThreadSet(combine_image);
  #pragma omp parallel for
  for (y=0; y < (long) combine_image->rows; y++)
  {
    const Image
      *next;

    MagickBooleanType
      progress;

    PixelPacket
      *pixels;

    register const PixelPacket
      *p;

    register long
      id,
      x;

    register PixelPacket
      *q;

    ViewInfo
      *image_view;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    pixels=GetCacheViewPixels(combine_view[id],0,y,combine_image->columns,1);
    if (pixels == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    next=image;
    if (((channel & RedChannel) != 0) && (next != (Image *) NULL))
      {
        image_view=AcquireCacheView(next);
        p=AcquireCacheViewPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        q=pixels;
        for (x=0; x < (long) combine_image->columns; x++)
        {
          q->red=PixelIntensityToQuantum(p);
          p++;
          q++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (((channel & GreenChannel) != 0) && (next != (Image *) NULL))
      {
        image_view=AcquireCacheView(next);
        p=AcquireCacheViewPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        q=pixels;
        for (x=0; x < (long) combine_image->columns; x++)
        {
          q->green=PixelIntensityToQuantum(p);
          p++;
          q++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (((channel & BlueChannel) != 0) && (next != (Image *) NULL))
      {
        image_view=AcquireCacheView(next);
        p=AcquireCacheViewPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        q=pixels;
        for (x=0; x < (long) combine_image->columns; x++)
        {
          q->blue=PixelIntensityToQuantum(p);
          p++;
          q++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (((channel & OpacityChannel) != 0) && (next != (Image *) NULL))
      {
        image_view=AcquireCacheView(next);
        p=AcquireCacheViewPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        q=pixels;
        for (x=0; x < (long) combine_image->columns; x++)
        {
          q->opacity=PixelIntensityToQuantum(p);
          p++;
          q++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (((channel & IndexChannel) != 0) &&
        (image->colorspace == CMYKColorspace) &&
        (next != (Image *) NULL))
      {
        IndexPacket
          *indexes;

        image_view=AcquireCacheView(next);
        p=AcquireCacheViewPixels(image_view,0,y,next->columns,1,exception);
        if (p == (const PixelPacket *) NULL)
          continue;
        indexes=GetIndexes(combine_image);
        for (x=0; x < (long) combine_image->columns; x++)
        {
          indexes[x]=PixelIntensityToQuantum(p);
          p++;
        }
        image_view=DestroyCacheView(image_view);
        next=GetNextImageInList(next);
      }
    if (SyncCacheView(combine_view[id]) == MagickFalse)
      status=MagickFalse;
    progress=SetImageProgress(image,CombineImageTag,y,combine_image->rows);
    if (progress == MagickFalse)
      status=MagickFalse;
  }
  combine_view=DestroyCacheViewThreadSet(combine_view);
  if (status == MagickFalse)
    combine_image=DestroyImage(combine_image);
  return(combine_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     C y c l e C o l o r m a p I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CycleColormap() displaces an image's colormap by a given number of
%  positions.  If you cycle the colormap a number of times you can produce
%  a psychodelic effect.
%
%  The format of the CycleColormapImage method is:
%
%      MagickBooleanType CycleColormapImage(Image *image,const long displace)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o displace:  displace the colormap this amount.
%
*/
MagickExport MagickBooleanType CycleColormapImage(Image *image,
  const long displace)
{
  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->storage_class == DirectClass)
    (void) SetImageType(image,PaletteType);
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    long
      index;

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
      index=(long) (indexes[x]+displace) % image->colors;
      if (index < 0)
        index+=image->colors;
      indexes[x]=(IndexPacket) index;
      q->red=image->colormap[index].red;
      q->green=image->colormap[index].green;
      q->blue=image->colormap[index].blue;
      q++;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
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
%   D e s t r o y I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImage() dereferences an image, deallocating memory associated with
%  the image if the reference count becomes zero.
%
%  The format of the DestroyImage method is:
%
%      Image *DestroyImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Image *DestroyImage(Image *image)
{
  MagickBooleanType
    destroy;

  /*
    Dereference image.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  destroy=MagickFalse;
  AcquireSemaphoreInfo(&image->semaphore);
  image->reference_count--;
  if (image->reference_count == 0)
    destroy=MagickTrue;
  RelinquishSemaphoreInfo(image->semaphore);
  if (destroy == MagickFalse)
    return((Image *) NULL);
  /*
    Destroy image.
  */
  DestroyImagePixels(image);
  if (image->clip_mask != (Image *) NULL)
    image->clip_mask=DestroyImage(image->clip_mask);
  if (image->mask != (Image *) NULL)
    image->mask=DestroyImage(image->mask);
  if (image->montage != (char *) NULL)
    image->montage=DestroyString(image->montage);
  if (image->directory != (char *) NULL)
    image->directory=DestroyString(image->directory);
  if (image->colormap != (PixelPacket *) NULL)
    image->colormap=(PixelPacket *) RelinquishMagickMemory(image->colormap);
  if (image->geometry != (char *) NULL)
    image->geometry=DestroyString(image->geometry);
#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)
  DestroyImageAttributes(image);
#endif
  DestroyImageProfiles(image);
  DestroyImageProperties(image);
  DestroyImageArtifacts(image);
  if (image->ascii85 != (Ascii85Info*) NULL)
    image->ascii85=(Ascii85Info *) RelinquishMagickMemory(image->ascii85);
  DestroyBlob(image);
  (void) DestroyExceptionInfo(&image->exception);
  if (image->semaphore != (SemaphoreInfo *) NULL)
    DestroySemaphoreInfo(&image->semaphore);
  image->signature=(~MagickSignature);
  image=(Image *) RelinquishMagickMemory(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y I m a g e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyImageInfo() deallocates memory associated with an ImageInfo
%  structure.
%
%  The format of the DestroyImageInfo method is:
%
%      ImageInfo *DestroyImageInfo(ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport ImageInfo *DestroyImageInfo(ImageInfo *image_info)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  if (image_info->size != (char *) NULL)
    image_info->size=DestroyString(image_info->size);
  if (image_info->extract != (char *) NULL)
    image_info->extract=DestroyString(image_info->extract);
  if (image_info->scenes != (char *) NULL)
    image_info->scenes=DestroyString(image_info->scenes);
  if (image_info->page != (char *) NULL)
    image_info->page=DestroyString(image_info->page);
  if (image_info->sampling_factor != (char *) NULL)
    image_info->sampling_factor=DestroyString(
      image_info->sampling_factor);
  if (image_info->server_name != (char *) NULL)
    image_info->server_name=DestroyString(
      image_info->server_name);
  if (image_info->font != (char *) NULL)
    image_info->font=DestroyString(image_info->font);
  if (image_info->texture != (char *) NULL)
    image_info->texture=DestroyString(image_info->texture);
  if (image_info->density != (char *) NULL)
    image_info->density=DestroyString(image_info->density);
  if (image_info->view != (char *) NULL)
    image_info->view=DestroyString(image_info->view);
  if (image_info->authenticate != (char *) NULL)
    image_info->authenticate=DestroyString(
      image_info->authenticate);
  DestroyImageOptions(image_info);
  if (image_info->cache != (void *) NULL)
    image_info->cache=DestroyCacheInfo(image_info->cache);
  if (image_info->profile != (StringInfo *) NULL)
    image_info->profile=(void *) DestroyStringInfo((StringInfo *)
      image_info->profile);
  image_info->signature=(~MagickSignature);
  image_info=(ImageInfo *) RelinquishMagickMemory(image_info);
  return(image_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D i s a s s o c i a t e I m a g e S t r e a m                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DisassociateImageStream() disassociates the image stream.
%
%  The format of the DisassociateImageStream method is:
%
%      MagickBooleanType DisassociateImageStream(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport void DisassociateImageStream(Image *image)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  (void) DetachBlob(image->blob);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e A l p h a C h a n n e l                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageAlphaChannel() returns MagickFalse if the image alpha channel is
%  not activated.  That is, the image is RGB rather than RGBA or CMYK rather
%  than CMYKA.
%
%  The format of the GetImageAlphaChannel method is:
%
%      MagickBooleanType GetImageAlphaChannel(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType GetImageAlphaChannel(const Image *image)
{
  assert(image != (const Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  return(image->matte);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e C l i p M a s k                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageClipMask() returns the clip path associated with the image.
%
%  The format of the GetImageClipMask method is:
%
%      Image *GetImageClipMask(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Image *GetImageClipMask(const Image *image,
  ExceptionInfo *exception)
{
  assert(image != (const Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->clip_mask == (Image *) NULL)
    return((Image *) NULL);
  return(CloneImage(image->clip_mask,0,0,MagickTrue,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e E x c e p t i o n                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageException() traverses an image sequence and returns any
%  error more severe than noted by the exception parameter.
%
%  The format of the GetImageException method is:
%
%      void GetImageException(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: Specifies a pointer to a list of one or more images.
%
%    o exception: return the highest severity exception.
%
*/
MagickExport void GetImageException(Image *image,ExceptionInfo *exception)
{
  register Image
    *next;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  for (next=image; next != (Image *) NULL; next=GetNextImageInList(next))
  {
    if (next->exception.severity == UndefinedException)
      continue;
    if (next->exception.severity > exception->severity)
      InheritException(exception,&next->exception);
    next->exception.severity=UndefinedException;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageInfo() initializes image_info to default values.
%
%  The format of the GetImageInfo method is:
%
%      void GetImageInfo(ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
*/
MagickExport void GetImageInfo(ImageInfo *image_info)
{
  ExceptionInfo
    *exception;

  /*
    File and image dimension members.
  */
  (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image_info != (ImageInfo *) NULL);
  (void) ResetMagickMemory(image_info,0,sizeof(*image_info));
  image_info->adjoin=MagickTrue;
  image_info->interlace=NoInterlace;
  image_info->channel=DefaultChannels;
  image_info->quality=UndefinedCompressionQuality;
  image_info->antialias=MagickTrue;
  image_info->dither=MagickTrue;
  exception=AcquireExceptionInfo();
  (void) QueryColorDatabase(BackgroundColor,&image_info->background_color,
    exception);
  (void) QueryColorDatabase(BorderColor,&image_info->border_color,exception);
  (void) QueryColorDatabase(MatteColor,&image_info->matte_color,exception);
  (void) QueryColorDatabase(TransparentColor,&image_info->transparent_color,
    exception);
  exception=DestroyExceptionInfo(exception);
  image_info->debug=IsEventLogging();
#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)
  if (GetMonitorHandler() != (MonitorHandler) NULL)
    image_info->progress_monitor=MagickMonitor;
#endif
  image_info->signature=MagickSignature;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e M a s k                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageMask() returns the mask associated with the image.
%
%  The format of the GetImageMask method is:
%
%      Image *GetImageMask(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Image *GetImageMask(const Image *image,ExceptionInfo *exception)
{
  assert(image != (const Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->mask == (Image *) NULL)
    return((Image *) NULL);
  return(CloneImage(image->mask,0,0,MagickTrue,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e T y p e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageType() returns the potential type of image:
%
%        Bilevel         Grayscale        GrayscaleMatte
%        Palette         PaletteMatte     TrueColor
%        TrueColorMatte  ColorSeparation  ColorSeparationMatte
%
%  To ensure the image type matches its potential, use SetImageType():
%
%    (void) SetImageType(image,GetImageType(image));
%
%  The format of the GetImageType method is:
%
%      ImageType GetImageType(const Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport ImageType GetImageType(const Image *image,ExceptionInfo *exception)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image->colorspace == CMYKColorspace)
    {
      if (image->matte == MagickFalse)
        return(ColorSeparationType);
      return(ColorSeparationMatteType);
    }
  if (IsMonochromeImage(image,exception) != MagickFalse)
    return(BilevelType);
  if (IsGrayImage(image,exception) != MagickFalse)
    {
      if (image->matte != MagickFalse)
        return(GrayscaleMatteType);
      return(GrayscaleType);
    }
  if (IsPaletteImage(image,exception) != MagickFalse)
    {
      if (image->matte != MagickFalse)
        return(PaletteMatteType);
      return(PaletteType);
    }
  if (image->matte != MagickFalse)
    return(TrueColorMatteType);
  return(TrueColorType);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t I m a g e V i r t u a l P i x e l M e t h o d                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetImageVirtualPixelMethod() gets the "virtual pixels" method for the
%  image.  A virtual pixel is any pixel access that is outside the boundaries
%  of the image cache.
%
%  The format of the GetImageVirtualPixelMethod() method is:
%
%      VirtualPixelMethod GetImageVirtualPixelMethod(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport VirtualPixelMethod GetImageVirtualPixelMethod(const Image *image)
{
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(GetCacheVirtualPixelMethod(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+     G r a d i e n t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GradientImage() applies a continuously smooth color transitions along a
%  vector from one color to another.
%
%  Note, the interface of this method will change in the future to support
%  more than one transistion.
%
%  The format of the GradientImage method is:
%
%      MagickBooleanType GradientImage(Image *image,
%        const PixelPacket *start_color,const PixelPacket *stop_color)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o start_color: the start color.
%
%    o stop_color: the stop color.
%
*/
MagickExport MagickBooleanType GradientImage(Image *image,
  const PixelPacket *start_color,const PixelPacket *stop_color)
{
  DrawInfo
    *draw_info;

  GradientInfo
    *gradient;

  MagickBooleanType
    status;

  register long
    i;

  /*
    Draw a linear gradient on the image.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(start_color != (const PixelPacket *) NULL);
  assert(stop_color != (const PixelPacket *) NULL);
  draw_info=AcquireDrawInfo();
  gradient=(&draw_info->gradient);
  gradient->bounding_box.width=image->columns;
  gradient->bounding_box.height=image->rows;
  gradient->gradient_vector.y2=(double) image->rows-1.0;
  gradient->spread=ReflectSpread;
  gradient->number_stops=2;
  gradient->stops=(StopInfo *) AcquireQuantumMemory(gradient->number_stops,
    sizeof(*gradient->stops));
  if (gradient->stops == (StopInfo *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  (void) ResetMagickMemory(gradient->stops,0,gradient->number_stops*
    sizeof(*gradient->stops));
  for (i=0; i < (long) gradient->number_stops; i++)
    GetMagickPixelPacket(image,&gradient->stops[i].color);
  SetMagickPixelPacket(image,start_color,(IndexPacket *) NULL,
    &gradient->stops[0].color);
  gradient->stops[0].offset=0.0;
  SetMagickPixelPacket(image,stop_color,(IndexPacket *) NULL,
    &gradient->stops[1].color);
  gradient->stops[1].offset=1.0;
  status=DrawGradientImage(image,draw_info);
  draw_info=DestroyDrawInfo(draw_info);
  if ((start_color->opacity == OpaqueOpacity) &&
      (stop_color->opacity == OpaqueOpacity))
    image->matte=MagickFalse;
  if ((IsGrayPixel(start_color) != MagickFalse) &&
      (IsGrayPixel(stop_color) != MagickFalse))
    image->type=GrayscaleType;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I n t e r p r e t I m a g e F i l e n a m e                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  InterpretImageFilename() interprets embedded characters in an image filename.
%  Only a single embedded sequence is replaced and the string length is
%  returned.
%
%  The format of the InterpretImageFilename method is:
%
%      long InterpretImageFilename(char *string,const size_t length,
%        const char *format,int value)
%
%  A description of each parameter follows.
%
%   o string:  InterpretImageFilename() returns the formatted string in this
%     character buffer.
%
%   o length: the maximum length of the string.
%
%   o  format:  A string describing the format to use to write the numeric
%      argument. Only the first numeric format identifier is replaced.
%
%   o  value:  Numeric value to substitute into format string.
%
*/
MagickExport long InterpretImageFilename(char *string,const size_t length,
  const char *format,int value)
{
  char
    *q;

  int
    c;

  register char
    *p;

  (void) CopyMagickString(string,format,length);
  for (p=strchr(format,'%'); p != (char *) NULL; p=strchr(p+1,'%'))
  {
    q=(char *) p+1;
    if (*q == '%')
      {
        p=q+1;
        continue;
      }
    if (*q == '0')
      (void) strtol(q,&q,10);
    if ((*q != 'd') && (*q != 'o') && (*q != 'x'))
      continue;
    q++;
    c=(*q);
    *q='\0';
    (void) FormatMagickString(string+(p-format),length-(p-format),p,value);
    *q=c;
    (void) ConcatenateMagickString(string,q,length);
    if (*(q-1) != '%')
      break;
    p++;
  }
  for (q=string; *q != '\0'; q++)
    if ((*q == '%') && (*(q+1) == '%'))
      (void) CopyMagickString(q,q+1,length-(q-string));
  return((long) strlen(string));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s H i g h D y n a m i c R a n g e I m a g e                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsHighDynamicRangeImage() returns MagickFalse if any pixel component is
%  non-integer or exceeds the bounds of the quantum depth (e.g. for Q16
%  0..65535.
%
%  The format of the IsHighDynamicRangeImage method is:
%
%      MagickBooleanType IsHighDynamicRangeImage(const Image *image,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType IsHighDynamicRangeImage(const Image *image,
  ExceptionInfo *exception)
{
#if !defined(MAGICKCORE_HDRI_SUPPORT)
  (void) image;
  (void) exception;
  return(MagickFalse);
#else
  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=MagickFalse;
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    MagickPixelPacket
      pixel;

    register const IndexPacket
      *indexes;

    register const PixelPacket
      *p;

    register long
      id,
      x;

    GetMagickPixelPacket(image,&pixel);
    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=AcquireCacheViewIndexes(image_view[id]);
    for (x=0; x < (long) image->columns; x++)
    {
      SetMagickPixelPacket(image,p,indexes+x,&pixel);
      if ((pixel.red < 0.0) || (pixel.red > QuantumRange) ||
          (pixel.red != (QuantumAny) pixel.red))
        break;
      if ((pixel.green < 0.0) || (pixel.green > QuantumRange) ||
          (pixel.green != (QuantumAny) pixel.green))
        break;
      if ((pixel.blue < 0.0) || (pixel.blue > QuantumRange) ||
          (pixel.blue != (QuantumAny) pixel.blue))
        break;
      if (pixel.matte != MagickFalse)
        {
          if ((pixel.opacity < 0.0) || (pixel.opacity > QuantumRange) ||
              (pixel.opacity != (QuantumAny) pixel.opacity))
            break;
        }
      if (pixel.colorspace == CMYKColorspace)
        {
          if ((pixel.index < 0.0) || (pixel.index > QuantumRange) ||
              (pixel.index != (QuantumAny) pixel.index))
            break;
        }
      p++;
    }
    if (x < (long) image->columns)
      {
        status=MagickFalse;
        status=MagickTrue;
        continue;
      }
  }
  image_view=DestroyCacheViewThreadSet(image_view);
  return(status);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s I m a g e O b j e c t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsImageObject() returns MagickTrue if the image sequence contains a valid
%  set of image objects.
%
%  The format of the IsImageObject method is:
%
%      MagickBooleanType IsImageObject(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsImageObject(const Image *image)
{
  register const Image
    *p;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
    if (p->signature != MagickSignature)
      return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     I s T a i n t I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsTaintImage() returns MagickTrue any pixel in the image has been altered
%  since it was first constituted.
%
%  The format of the IsTaintImage method is:
%
%      MagickBooleanType IsTaintImage(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType IsTaintImage(const Image *image)
{
  char
    magick[MaxTextExtent],
    filename[MaxTextExtent];

  register const Image
    *p;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  (void) CopyMagickString(magick,image->magick,MaxTextExtent);
  (void) CopyMagickString(filename,image->filename,MaxTextExtent);
  for (p=image; p != (Image *) NULL; p=GetNextImageInList(p))
  {
    if (p->taint != MagickFalse)
      return(MagickTrue);
    if (LocaleCompare(p->magick,magick) != 0)
      return(MagickTrue);
    if (LocaleCompare(p->filename,filename) != 0)
      return(MagickTrue);
  }
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M o d i f y I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ModifyImage() ensures that there is only a single reference to the image
%  to be modified, updating the provided image pointer to point to a clone of
%  the original image if necessary.
%
%  The format of the ModifyImage method is:
%
%      MagickBooleanType ModifyImage(Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType ModifyImage(Image **image,
  ExceptionInfo *exception)
{
  Image
    *clone_image;

  assert(image != (Image **) NULL);
  assert(*image != (Image *) NULL);
  assert((*image)->signature == MagickSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  if ((*image)->reference_count <= 1)
    return(MagickTrue);
  clone_image=CloneImage(*image,0,0,MagickTrue,exception);
  AcquireSemaphoreInfo(&(*image)->semaphore);
  (*image)->reference_count--;
  RelinquishSemaphoreInfo((*image)->semaphore);
  *image=clone_image;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   N e w M a g i c k I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NewMagickImage() creates a blank image canvas of the specified size and
%  background color.
%
%  The format of the NewMagickImage method is:
%
%      Image *NewMagickImage(const ImageInfo *image_info,
%        const unsigned long width,const unsigned long height,
%        const MagickPixelPacket *background)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o width: the image width.
%
%    o height: the image height.
%
%    o background: the image color.
%
*/
MagickExport Image *NewMagickImage(const ImageInfo *image_info,
  const unsigned long width,const unsigned long height,
  const MagickPixelPacket *background)
{
  Image
    *image;

  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **image_view;

  assert(image_info != (const ImageInfo *) NULL);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image_info->signature == MagickSignature);
  assert(background != (const MagickPixelPacket *) NULL);
  image=AcquireImage(image_info);
  image->columns=width;
  image->rows=height;
  image->colorspace=background->colorspace;
  image->matte=background->matte;
  image->fuzz=background->fuzz;
  image->depth=background->depth;
  image_view=AcquireCacheViewThreadSet(image);
  status=MagickTrue;
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
    q=SetCacheViewPixels(image_view[id],0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewIndexes(image_view[id]);
    for (x=0; x < (long) image->columns; x++)
    {
      SetPixelPacket(image,background,q,indexes+x);
      q++;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheViewThreadSet(image_view);
  if (status == MagickFalse)
    image=DestroyImage(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     P l a s m a I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  PlasmaImage() initializes an image with plasma fractal values.  The image
%  must be initialized with a base color and the random number generator
%  seeded before this method is called.
%
%  The format of the PlasmaImage method is:
%
%      MagickBooleanType PlasmaImage(Image *image,const SegmentInfo *segment,
%        unsigned long attenuate,unsigned long depth)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o segment:   Define the region to apply plasma fractals values.
%
%    o attenuate: Define the plasma attenuation factor.
%
%    o depth: Limit the plasma recursion depth.
%
*/

static inline Quantum PlasmaPixel(const MagickRealType pixel,
  const MagickRealType noise)
{
  return(RoundToQuantum(pixel+noise*GetPseudoRandomValue()-noise/2.0));
}

MagickExport MagickBooleanType PlasmaImage(Image *image,
  const SegmentInfo *segment,unsigned long attenuate,unsigned long depth)
{
  long
    x,
    x_mid,
    y,
    y_mid;

  MagickRealType
    plasma;

  PixelPacket
    u,
    v;

  register PixelPacket
    *q;

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(segment != (SegmentInfo *) NULL);
  if (((segment->x2-segment->x1) == 0.0) && ((segment->y2-segment->y1) == 0.0))
    return(MagickTrue);
  if (depth != 0)
    {
      SegmentInfo
        local_info;

      /*
        Divide the area into quadrants and recurse.
      */
      depth--;
      attenuate++;
      x_mid=(long) (segment->x1+segment->x2+0.5)/2;
      y_mid=(long) (segment->y1+segment->y2+0.5)/2;
      local_info=(*segment);
      local_info.x2=(double) x_mid;
      local_info.y2=(double) y_mid;
      (void) PlasmaImage(image,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.y1=(double) y_mid;
      local_info.x2=(double) x_mid;
      (void) PlasmaImage(image,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.x1=(double) x_mid;
      local_info.y2=(double) y_mid;
      (void) PlasmaImage(image,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.x1=(double) x_mid;
      local_info.y1=(double) y_mid;
      return(PlasmaImage(image,&local_info,attenuate,depth));
    }
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  x_mid=(long) (segment->x1+segment->x2+0.5)/2;
  y_mid=(long) (segment->y1+segment->y2+0.5)/2;
  if ((segment->x1 == (double) x_mid) && (segment->x2 == (double) x_mid) &&
      (segment->y1 == (double) y_mid) && (segment->y2 == (double) y_mid))
    return(MagickFalse);
  /*
    Average pixels and apply plasma.
  */
  plasma=(MagickRealType) QuantumRange/(2.0*attenuate);
  if ((segment->x1 != (double) x_mid) || (segment->x2 != (double) x_mid))
    {
      /*
        Left pixel.
      */
      x=(long) (segment->x1+0.5);
      u=GetOnePixel(image,x,(long) (segment->y1+0.5));
      v=GetOnePixel(image,x,(long) (segment->y2+0.5));
      q=SetImagePixels(image,x,y_mid,1,1);
      if (q == (PixelPacket *) NULL)
        return(MagickTrue);
      q->red=PlasmaPixel((MagickRealType) (u.red+v.red)/2.0,plasma);
      q->green=PlasmaPixel((MagickRealType) (u.green+v.green)/2.0,plasma);
      q->blue=PlasmaPixel((MagickRealType) (u.blue+v.blue)/2.0,plasma);
      (void) SyncImagePixels(image);
      if (segment->x1 != segment->x2)
        {
          /*
            Right pixel.
          */
          x=(long) (segment->x2+0.5);
          u=GetOnePixel(image,x,(long) (segment->y1+0.5));
          v=GetOnePixel(image,x,(long) (segment->y2+0.5));
          q=SetImagePixels(image,x,y_mid,1,1);
          if (q == (PixelPacket *) NULL)
            return(MagickTrue);
          q->red=PlasmaPixel((MagickRealType) (u.red+v.red)/2.0,plasma);
          q->green=PlasmaPixel((MagickRealType) (u.green+v.green)/2.0,plasma);
          q->blue=PlasmaPixel((MagickRealType) (u.blue+v.blue)/2.0,plasma);
          (void) SyncImagePixels(image);
        }
    }
  if ((segment->y1 != (double) y_mid) || (segment->y2 != (double) y_mid))
    {
      if ((segment->x1 != (double) x_mid) || (segment->y2 != (double) y_mid))
        {
          /*
            Bottom pixel.
          */
          y=(long) (segment->y2+0.5);
          u=GetOnePixel(image,(long) (segment->x1+0.5),y);
          v=GetOnePixel(image,(long) (segment->x2+0.5),y);
          q=SetImagePixels(image,x_mid,y,1,1);
          if (q == (PixelPacket *) NULL)
            return(MagickTrue);
          q->red=PlasmaPixel((MagickRealType) (u.red+v.red)/2.0,plasma);
          q->green=PlasmaPixel((MagickRealType) (u.green+v.green)/2.0,plasma);
          q->blue=PlasmaPixel((MagickRealType) (u.blue+v.blue)/2.0,plasma);
          (void) SyncImagePixels(image);
        }
      if (segment->y1 != segment->y2)
        {
          /*
            Top pixel.
          */
          y=(long) (segment->y1+0.5);
          u=GetOnePixel(image,(long) (segment->x1+0.5),y);
          v=GetOnePixel(image,(long) (segment->x2+0.5),y);
          q=SetImagePixels(image,x_mid,y,1,1);
          if (q == (PixelPacket *) NULL)
            return(MagickTrue);
          q->red=PlasmaPixel((MagickRealType) (u.red+v.red)/2.0,plasma);
          q->green=PlasmaPixel((MagickRealType) (u.green+v.green)/2.0,plasma);
          q->blue=PlasmaPixel((MagickRealType) (u.blue+v.blue)/2.0,plasma);
          (void) SyncImagePixels(image);
        }
    }
  if ((segment->x1 != segment->x2) || (segment->y1 != segment->y2))
    {
      /*
        Middle pixel.
      */
      x=(long) (segment->x1+0.5);
      y=(long) (segment->y1+0.5);
      u=GetOnePixel(image,x,y);
      x=(long) (segment->x2+0.5);
      y=(long) (segment->y2+0.5);
      v=GetOnePixel(image,x,y);
      q=SetImagePixels(image,x_mid,y_mid,1,1);
      if (q == (PixelPacket *) NULL)
        return(MagickTrue);
      q->red=PlasmaPixel((MagickRealType) (u.red+v.red)/2.0,plasma);
      q->green=PlasmaPixel((MagickRealType) (u.green+v.green)/2.0,plasma);
      q->blue=PlasmaPixel((MagickRealType) (u.blue+v.blue)/2.0,plasma);
      (void) SyncImagePixels(image);
    }
  if (((segment->x2-segment->x1) < 3.0) && ((segment->y2-segment->y1) < 3.0))
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e f e r e n c e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReferenceImage() increments the reference count associated with an image
%  returning a pointer to the image.
%
%  The format of the ReferenceImage method is:
%
%      Image *ReferenceImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport Image *ReferenceImage(Image *image)
{
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  AcquireSemaphoreInfo(&image->semaphore);
  image->reference_count++;
  RelinquishSemaphoreInfo(image->semaphore);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e s e t I m a g e P a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ResetImagePage() resets the image page canvas and position.
%
%  The format of the ResetImagePage method is:
%
%      MagickBooleanType ResetImagePage(Image *image,const char *page)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o page: the relative page specification.
%
*/
MagickExport MagickBooleanType ResetImagePage(Image *image,const char *page)
{
  MagickStatusType
    flags;

  RectangleInfo
    geometry;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  flags=ParseAbsoluteGeometry(page,&geometry);
  if ((flags & WidthValue) != 0)
    {
      if ((flags & HeightValue) == 0)
        geometry.height=geometry.width;
      image->page.width=geometry.width;
      image->page.height=geometry.height;
    }
  if ((flags & AspectValue) != 0)
    {
      if ((flags & XValue) != 0)
        image->page.x+=geometry.x;
      if ((flags & YValue) != 0)
        image->page.y+=geometry.y;
    }
  else
    {
      if ((flags & XValue) != 0)
        {
          image->page.x=geometry.x;
          if ((image->page.width == 0) && (geometry.x > 0))
            image->page.width=image->columns+geometry.x;
        }
      if ((flags & YValue) != 0)
        {
          image->page.y=geometry.y;
          if ((image->page.height == 0) && (geometry.y > 0))
            image->page.height=image->rows+geometry.y;
        }
    }
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e p a r a t e I m a g e C h a n n e l                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SeparateImageChannel() separates a channel from the image and returns it as
%  a grayscale image.  A channel is a particular color component of each pixel
%  in the image.
%
%  The format of the SeparateImageChannel method is:
%
%      MagickBooleanType SeparateImageChannel(Image *image,
%        const ChannelType channel)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: Identify which channel to extract: RedChannel, GreenChannel,
%      BlueChannel, OpacityChannel, CyanChannel, MagentaChannel,
%      YellowChannel, or BlackChannel.
%
*/
MagickExport MagickBooleanType SeparateImageChannel(Image *image,
  const ChannelType channel)
{
#define SeparateImageTag  "Separate/Image"

  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **image_view;

  /*
    Separate DirectClass packets.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  status=MagickTrue;
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
    switch (channel)
    {
      case RedChannel:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->green=q->red;
          q->blue=q->red;
          q++;
        }
        break;
      }
      case GreenChannel:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=q->green;
          q->blue=q->green;
          q++;
        }
        break;
      }
      case BlueChannel:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=q->blue;
          q->green=q->blue;
          q++;
        }
        break;
      }
      case OpacityChannel:
      {
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=q->opacity;
          q->green=q->opacity;
          q->blue=q->opacity;
          q++;
        }
        break;
      }
      case BlackChannel:
      {
        if ((image->storage_class != PseudoClass) &&
            (image->colorspace != CMYKColorspace))
          break;
        for (x=0; x < (long) image->columns; x++)
        {
          q->red=indexes[x];
          q->green=indexes[x];
          q->blue=indexes[x];
          q++;
        }
        break;
      }
      default:
        break;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      status=MagickFalse;
    if (SetImageProgress(image,SeparateImageTag,y,image->rows) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheViewThreadSet(image_view);
  image->colorspace=RGBColorspace;
  image->matte=MagickFalse;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e p a r a t e I m a g e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SeparateImages() returns a separate grayscale image for each channel
%  specified.
%
%  The format of the SeparateImages method is:
%
%      MagickBooleanType SeparateImages(const Image *image,
%        const ChannelType channel,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o channel: Identify which channels to extract: RedChannel, GreenChannel,
%      BlueChannel, OpacityChannel, CyanChannel, MagentaChannel,
%      YellowChannel, or BlackChannel.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport Image *SeparateImages(const Image *image,const ChannelType channel,
  ExceptionInfo *exception)
{
  Image
    *images,
    *separate_image;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  images=NewImageList();
  if ((channel & RedChannel) != 0)
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,RedChannel);
      AppendImageToList(&images,separate_image);
    }
  if ((channel & GreenChannel) != 0)
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,GreenChannel);
      AppendImageToList(&images,separate_image);
    }
  if ((channel & BlueChannel) != 0)
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,BlueChannel);
      AppendImageToList(&images,separate_image);
    }
  if ((channel & OpacityChannel) != 0)
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,OpacityChannel);
      AppendImageToList(&images,separate_image);
    }
  if (((channel & BlackChannel) != 0) && (image->colorspace == CMYKColorspace))
    {
      separate_image=CloneImage(image,0,0,MagickTrue,exception);
      (void) SeparateImageChannel(separate_image,BlackChannel);
      AppendImageToList(&images,separate_image);
    }
  return(images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e A l p h a C h a n n e l                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageAlphaChannel() activates, deactivates, resets, or sets the alpha
%  channel.
%
%  The format of the SetImageAlphaChannel method is:
%
%      MagickBooleanType SetImageAlphaChannel(Image *image,
%        const AlphaChannelType alpha_type)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o alpha_type:  The alpha channel type: ActivateAlphaChannel,
%      DeactivateAlphaChannel, ResetAlphaChannel, or SetAlphaChannel.
%
*/
MagickExport MagickBooleanType SetImageAlphaChannel(Image *image,
  const AlphaChannelType alpha_type)
{
  MagickBooleanType
    status;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  status=MagickFalse;
  switch (alpha_type)
  {
    case ActivateAlphaChannel:
    {
      image->matte=MagickTrue;
      break;
    }
    case DeactivateAlphaChannel:
    {
      image->matte=MagickFalse;
      break;
    }
    case ResetAlphaChannel:
    {
      if (image->matte == MagickFalse)
        {
          status=SetImageOpacity(image,OpaqueOpacity);
          image->matte=MagickTrue;
        }
      break;
    }
    case SetAlphaChannel:
    {
      status=CompositeImage(image,CopyOpacityCompositeOp,image,0,0);
      break;
    }
    case UndefinedAlphaChannel:
      break;
  }
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e B a c k g r o u n d C o l o r                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageBackgroundColor() initializes the image pixels to the image
%  background color.  The background color is defined by the background_color
%  member of the image structure.
%
%  The format of the SetImage method is:
%
%      MagickBooleanType SetImageBackgroundColor(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType SetImageBackgroundColor(Image *image)
{
  long
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    background;

  ViewInfo
    **image_view;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  if (image->background_color.opacity != OpaqueOpacity)
    image->matte=MagickTrue;
  GetMagickPixelPacket(image,&background);
  SetMagickPixelPacket(image,&image->background_color,(const IndexPacket *)
    NULL,&background);
  if (image->colorspace == CMYKColorspace)
    ConvertRGBToCMYK(&background);
  status=MagickTrue;
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
    q=SetCacheViewPixels(image_view[id],0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewIndexes(image_view[id]);
    for (x=0; x < (long) image->columns; x++)
    {
      SetPixelPacket(image,&background,q,indexes+x);
      q++;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
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
%   S e t I m a g e S t o r a g e C l a s s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageStorageClass() sets the image class: DirectClass for true color
%  images or PseudoClass for colormapped images.
%
%  The format of the SetImageStorageClass method is:
%
%      MagickBooleanType SetImageStorageClass(Image *image,
%        const ClassType storage_class)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o storage_class:  The image class.
%
*/
MagickExport MagickBooleanType SetImageStorageClass(Image *image,
  const ClassType storage_class)
{
  image->storage_class=storage_class;
  return(SyncImagePixelCache(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e C l i p M a s k                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageClipMask() associates a clip path with the image.  The clip path
%  must be the same dimensions as the image.  Set any pixel component of
%  the clip path to TransparentOpacity to prevent that corresponding image
%  pixel component from being updated when SyncImagePixels() is applied.
%
%  The format of the SetImageClipMask method is:
%
%      MagickBooleanType SetImageClipMask(Image *image,const Image *clip_mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o clip_mask: the image clip path.
%
*/
MagickExport MagickBooleanType SetImageClipMask(Image *image,
  const Image *clip_mask)
{
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (clip_mask != (const Image *) NULL)
    if ((clip_mask->columns != image->columns) ||
        (clip_mask->rows != image->rows))
      ThrowBinaryException(ImageError,"ImageSizeDiffers",image->filename);
  if (image->clip_mask != (Image *) NULL)
    image->clip_mask=DestroyImage(image->clip_mask);
  image->clip_mask=NewImageList();
  if (clip_mask == (Image *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  image->clip_mask=CloneImage(clip_mask,0,0,MagickTrue,&image->exception);
  if (image->clip_mask == (Image *) NULL)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e E x t e n t                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageExtent() sets the image size (i.e. columns & rows).
%
%  The format of the SetImageExtent method is:
%
%      MagickBooleanType SetImageExtent(Image *image,
%        const unsigned long columns,const unsigned long rows)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o columns:  The image width in pixels.
%
%    o rows:  The image height in pixels.
%
*/
MagickExport MagickBooleanType SetImageExtent(Image *image,
  const unsigned long columns,const unsigned long rows)
{
  if ((columns != 0) || (rows != 0))
    {
      image->columns=columns;
      image->rows=rows;
    }
  return(SyncImagePixelCache(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S e t I m a g e I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageInfo() initializes the `magick' field of the ImageInfo structure.
%  It is set to a type of image format based on the prefix or suffix of the
%  filename.  For example, `ps:image' returns PS indicating a Postscript image.
%  JPEG is returned for this filename: `image.jpg'.  The filename prefix has
%  precendence over the suffix.  Use an optional index enclosed in brackets
%  after a file name to specify a desired scene of a multi-resolution image
%  format like Photo CD (e.g. img0001.pcd[4]).  A True (non-zero) return value
%  indicates success.
%
%  The format of the SetImageInfo method is:
%
%      MagickBooleanType SetImageInfo(ImageInfo *image_info,
%        const MagickBooleanType rectify,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info..
%
%    o rectify: an unsigned value other than zero rectifies the attribute for
%      multi-frame support (user may want multi-frame but image format may not
%      support it).
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport MagickBooleanType SetImageInfo(ImageInfo *image_info,
  const MagickBooleanType rectify,ExceptionInfo *exception)
{
  char
    extension[MaxTextExtent],
    filename[MaxTextExtent],
    magic[MaxTextExtent],
    *q,
    subimage[MaxTextExtent];

  const MagicInfo
    *magic_info;

  const MagickInfo
    *magick_info;

  ExceptionInfo
    *sans_exception;

  Image
    *image;

  MagickBooleanType
    status;

  register const char
    *p;

  ssize_t
    count;

  unsigned char
    magick[2*MaxTextExtent];

  /*
    Look for 'image.format' in filename.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  *subimage='\0';
  GetPathComponent(image_info->filename,SubimagePath,subimage);
  if (*subimage != '\0')
    {
      /*
        Look for scene specification (e.g. img0001.pcd[4]).
      */
      if (IsSceneGeometry(subimage,MagickFalse) == MagickFalse)
        {
          if (IsGeometry(subimage) != MagickFalse)
            (void) CloneString(&image_info->extract,subimage);
        }
      else
        {
          unsigned long
            first,
            last;

          (void) CloneString(&image_info->scenes,subimage);
          image_info->scene=(unsigned long) atol(image_info->scenes);
          image_info->number_scenes=image_info->scene;
          p=image_info->scenes;
          for (q=(char *) image_info->scenes; *q != '\0'; p++)
          {
            while ((isspace((int) ((unsigned char) *p)) != 0) || (*p == ','))
              p++;
            first=(unsigned long) strtol(p,&q,10);
            last=first;
            while (isspace((int) ((unsigned char) *q)) != 0)
              q++;
            if (*q == '-')
              last=(unsigned long) strtol(q+1,&q,10);
            if (first > last)
              Swap(first,last);
            if (first < image_info->scene)
              image_info->scene=first;
            if (last > image_info->number_scenes)
              image_info->number_scenes=last;
            p=q;
          }
          image_info->number_scenes-=image_info->scene-1;
          image_info->subimage=image_info->scene;
          image_info->subrange=image_info->number_scenes;
        }
    }
  *extension='\0';
  GetPathComponent(image_info->filename,ExtensionPath,extension);
#if defined(MAGICKCORE_ZLIB_DELEGATE)
  if (*extension != '\0')
    if ((LocaleCompare(extension,"gz") == 0) ||
        (LocaleCompare(extension,"Z") == 0) ||
        (LocaleCompare(extension,"wmz") == 0))
      {
        char
          path[MaxTextExtent];

        (void) CopyMagickString(path,image_info->filename,MaxTextExtent);
        path[strlen(path)-strlen(extension)-1]='\0';
        GetPathComponent(path,ExtensionPath,extension);
      }
#endif
#if defined(MAGICKCORE_BZLIB_DELEGATE)
  if (*extension != '\0')
    if (LocaleCompare(extension,"bz2") == 0)
      {
        char
          path[MaxTextExtent];

        (void) CopyMagickString(path,image_info->filename,MaxTextExtent);
        path[strlen(path)-strlen(extension)-1]='\0';
        GetPathComponent(path,ExtensionPath,extension);
      }
#endif
  image_info->affirm=MagickFalse;
  sans_exception=AcquireExceptionInfo();
  if (*extension != '\0')
    {
      MagickFormatType
        format_type;

      register long
        i;

      static const char
        *format_type_formats[] =
        {
          "AUTOTRACE",
          "BROWSE",
          "CAPTION",
          "DCRAW",
          "EDIT",
          "FRACTAL",
          "LABEL",
          "LAUNCH",
          "MPEG:ENCODE",
          "PREVIEW",
          "PRINT",
          "PS:ALPHA",
          "PS:CMYK",
          "PS:COLOR",
          "PS:GRAY",
          "PS:MONO",
          "SCAN",
          "SHOW",
          "TMP",
          "WIN",
          (char *) NULL
        };

      /*
        User specified image format.
      */
      (void) CopyMagickString(magic,extension,MaxTextExtent);
      LocaleUpper(magic);
      /*
        Look for explicit image formats.
      */
      format_type=UndefinedFormatType;
      if ((LocaleNCompare(image_info->magick,"SGI",3) == 0) &&
          (LocaleCompare(magic,"RGB") == 0))
        format_type=ExplicitFormatType;
      i=0;
      while ((format_type != UndefinedFormatType) &&
             (format_type_formats[i] != (char *) NULL))
      {
        if ((*magic == *format_type_formats[i]) &&
            (LocaleCompare(magic,format_type_formats[i]) == 0))
          format_type=ExplicitFormatType;
        i++;
      }
      magick_info=GetMagickInfo(magic,sans_exception);
      if ((magick_info != (const MagickInfo *) NULL) &&
          (magick_info->format_type != UndefinedFormatType))
        format_type=format_type;
      if (format_type == UndefinedFormatType)
        (void) CopyMagickString(image_info->magick,magic,MaxTextExtent);
      else
        if (format_type == ExplicitFormatType)
          image_info->affirm=MagickTrue;
    }
  /*
    Look for explicit 'format:image' in filename.
  */
  *magic='\0';
  GetPathComponent(image_info->filename,MagickPath,magic);
  if (*magic == '\0')
    (void) CopyMagickString(magic,image_info->magick,MaxTextExtent);
  else
    {
      /*
        User specified image format.
      */
      LocaleUpper(magic);
      if (IsMagickConflict(magic) == MagickFalse)
        {
          (void) CopyMagickString(image_info->magick,magic,MaxTextExtent);
          if (LocaleCompare(magic,"TMP") != 0)
            image_info->affirm=MagickTrue;
          else
            image_info->temporary=MagickTrue;
        }
    }
  magick_info=GetMagickInfo(magic,sans_exception);
  sans_exception=DestroyExceptionInfo(sans_exception);
  if ((magick_info == (const MagickInfo *) NULL) ||
      (GetMagickEndianSupport(magick_info) == MagickFalse))
    image_info->endian=UndefinedEndian;
  GetPathComponent(image_info->filename,CanonicalPath,filename);
  (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
  if (rectify != MagickFalse)
    {
      /*
        Rectify multi-image file support.
      */
      (void) InterpretImageFilename(filename,MaxTextExtent,
        image_info->filename,(int) image_info->scene);
      if ((LocaleCompare(filename,image_info->filename) != 0) &&
          (strchr(filename,'%') == (char *) NULL))
        image_info->adjoin=MagickFalse;
      magick_info=GetMagickInfo(magic,exception);
      if (magick_info != (const MagickInfo *) NULL)
        if (GetMagickAdjoin(magick_info) == MagickFalse)
          image_info->adjoin=MagickFalse;
      return(MagickTrue);
    }
  if (image_info->affirm != MagickFalse)
    return(MagickTrue);
  /*
    Determine the image format from the first few bytes of the file.
  */
  image=AcquireImage(image_info);
  if (image == (Image *) NULL)
    return(MagickFalse);
  (void) CopyMagickString(image->filename,image_info->filename,MaxTextExtent);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImage(image);
      return(MagickFalse);
    }
  if ((IsBlobSeekable(image) == MagickFalse) ||
      (IsBlobExempt(image) != MagickFalse))
    {
      /*
        Copy standard input or pipe to temporary file.
      */
      *filename='\0';
      status=ImageToFile(image,filename,exception);
      (void) CloseBlob(image);
      if (status == MagickFalse)
        {
          image=DestroyImage(image);
          return(MagickFalse);
        }
      SetImageInfoFile(image_info,(FILE *) NULL);
      (void) CopyMagickString(image->filename,filename,MaxTextExtent);
      status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
      if (status == MagickFalse)
        {
          image=DestroyImage(image);
          return(MagickFalse);
        }
      (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
      image_info->temporary=MagickTrue;
    }
  (void) ResetMagickMemory(magick,0,sizeof(magick));
  count=ReadBlob(image,2*MaxTextExtent,magick);
  (void) CloseBlob(image);
  image=DestroyImage(image);
  /*
    Check magic.xml configuration file.
  */
  sans_exception=AcquireExceptionInfo();
  magic_info=GetMagicInfo(magick,(size_t) count,sans_exception);
  if ((magic_info != (const MagicInfo *) NULL) &&
      (GetMagicName(magic_info) != (char *) NULL))
    {
      (void) CopyMagickString(image_info->magick,GetMagicName(magic_info),
        MaxTextExtent);
      magick_info=GetMagickInfo(image_info->magick,sans_exception);
      if ((magick_info == (const MagickInfo *) NULL) ||
          (GetMagickEndianSupport(magick_info) == MagickFalse))
        image_info->endian=UndefinedEndian;
      sans_exception=DestroyExceptionInfo(sans_exception);
      return(MagickTrue);
    }
  p=GetImageMagick(magick,2*MaxTextExtent);
  if (p != (const char *) NULL)
    (void) CopyMagickString(image_info->magick,p,MaxTextExtent);
  magick_info=GetMagickInfo(image_info->magick,sans_exception);
  if ((magick_info == (const MagickInfo *) NULL) ||
      (GetMagickEndianSupport(magick_info) == MagickFalse))
    image_info->endian=UndefinedEndian;
  sans_exception=DestroyExceptionInfo(sans_exception);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e I n f o B l o b                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageInfoBlob() sets the image info blob member.
%
%  The format of the SetImageInfoBlob method is:
%
%      void SetImageInfoBlob(ImageInfo *image_info,const void *blob,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o blob: the blob.
%
%    o length: the blob length.
%
*/
MagickExport void SetImageInfoBlob(ImageInfo *image_info,const void *blob,
  const size_t length)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image_info->blob=(void *) blob;
  image_info->length=length;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e I n f o F i l e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageInfoFile() sets the image info file member.
%
%  The format of the SetImageInfoFile method is:
%
%      void SetImageInfoFile(ImageInfo *image_info,FILE *file)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o file: the file.
%
*/
MagickExport void SetImageInfoFile(ImageInfo *image_info,FILE *file)
{
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image_info->file=file;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e M a s k                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageMask() associates a mask with the image.  The mask must be the same
%  dimensions as the image.
%
%  The format of the SetImageMask method is:
%
%      MagickBooleanType SetImageMask(Image *image,const Image *mask)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o mask: the image mask.
%
*/
MagickExport MagickBooleanType SetImageMask(Image *image,
  const Image *mask)
{
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (mask != (const Image *) NULL)
    if ((mask->columns != image->columns) || (mask->rows != image->rows))
      ThrowBinaryException(ImageError,"ImageSizeDiffers",image->filename);
  if (image->mask != (Image *) NULL)
    image->mask=DestroyImage(image->mask);
  image->mask=NewImageList();
  if (mask == (Image *) NULL)
    return(MagickTrue);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  image->mask=CloneImage(mask,0,0,MagickTrue,&image->exception);
  if (image->mask == (Image *) NULL)
    return(MagickFalse);
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     S e t I m a g e O p a c i t y                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageOpacity() sets the opacity levels of the image.
%
%  The format of the SetImageOpacity method is:
%
%      MagickBooleanType SetImageOpacity(Image *image,const Quantum opacity)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o opacity: the level of transparency: 0 is fully opaque and QuantumRange is
%      fully transparent.
%
*/
MagickExport MagickBooleanType SetImageOpacity(Image *image,
  const Quantum opacity)
{
  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **image_view;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  image->matte=opacity != OpaqueOpacity ? MagickTrue : MagickFalse;
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
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
    for (x=0; x < (long) image->columns; x++)
    {
      q->opacity=opacity;
      q++;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
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
%   S e t I m a g e T y p e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageType() sets the type of image.  Choose from these types:
%
%        Bilevel        Grayscale       GrayscaleMatte
%        Palette        PaletteMatte    TrueColor
%        TrueColorMatte ColorSeparation ColorSeparationMatte
%        OptimizeType
%
%  The format of the SetImageType method is:
%
%      MagickBooleanType SetImageType(Image *image,const ImageType type)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o type: Image type.
%
*/
MagickExport MagickBooleanType SetImageType(Image *image,const ImageType type)
{
  ImageInfo
    *image_info;

  MagickBooleanType
    status;

  QuantizeInfo
    *quantize_info;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  status=MagickTrue;
  image_info=AcquireImageInfo();
  image_info->dither=image->dither;
  switch (type)
  {
    case BilevelType:
    {
      if (IsGrayImage(image,&image->exception) == MagickFalse)
        status=SetImageColorspace(image,GRAYColorspace);
      if (IsMonochromeImage(image,&image->exception) == MagickFalse)
        {
          quantize_info=AcquireQuantizeInfo(image_info);
          quantize_info->number_colors=2;
          quantize_info->colorspace=GRAYColorspace;
          status=QuantizeImage(quantize_info,image);
          quantize_info=DestroyQuantizeInfo(quantize_info);
        }
      image->matte=MagickFalse;
      break;
    }
    case GrayscaleType:
    {
      if (IsGrayImage(image,&image->exception) == MagickFalse)
        status=SetImageColorspace(image,GRAYColorspace);
      image->matte=MagickFalse;
      break;
    }
    case GrayscaleMatteType:
    {
      if (IsGrayImage(image,&image->exception) == MagickFalse)
        status=SetImageColorspace(image,GRAYColorspace);
      if (image->matte == MagickFalse)
        (void) SetImageAlphaChannel(image,ResetAlphaChannel);
      break;
    }
    case PaletteType:
    {
      if (image->colorspace != RGBColorspace)
        status=SetImageColorspace(image,RGBColorspace);
      if ((image->storage_class == DirectClass) || (image->colors > 256))
        {
          quantize_info=AcquireQuantizeInfo(image_info);
          quantize_info->number_colors=256;
          status=QuantizeImage(quantize_info,image);
          quantize_info=DestroyQuantizeInfo(quantize_info);
        }
      image->matte=MagickFalse;
      break;
    }
    case PaletteBilevelMatteType:
    {
      if (image->colorspace != RGBColorspace)
        status=SetImageColorspace(image,RGBColorspace);
      if (image->matte == MagickFalse)
        (void) SetImageAlphaChannel(image,ResetAlphaChannel);
      (void) BilevelImageChannel(image,AlphaChannel,(double) QuantumRange/2.0);
      quantize_info=AcquireQuantizeInfo(image_info);
      status=QuantizeImage(quantize_info,image);
      quantize_info=DestroyQuantizeInfo(quantize_info);
      break;
    }
    case PaletteMatteType:
    {
      if (image->colorspace != RGBColorspace)
        status=SetImageColorspace(image,RGBColorspace);
      if (image->matte == MagickFalse)
        (void) SetImageAlphaChannel(image,ResetAlphaChannel);
      quantize_info=AcquireQuantizeInfo(image_info);
      quantize_info->colorspace=TransparentColorspace;
      status=QuantizeImage(quantize_info,image);
      quantize_info=DestroyQuantizeInfo(quantize_info);
      break;
    }
    case TrueColorType:
    {
      if (image->colorspace != RGBColorspace)
        status=SetImageColorspace(image,RGBColorspace);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass);
      image->matte=MagickFalse;
      break;
    }
    case TrueColorMatteType:
    {
      if (image->colorspace != RGBColorspace)
        status=SetImageColorspace(image,RGBColorspace);
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass);
      if (image->matte == MagickFalse)
        (void) SetImageAlphaChannel(image,ResetAlphaChannel);
      break;
    }
    case ColorSeparationType:
    {
      if (image->colorspace != CMYKColorspace)
        {
          if (image->colorspace != RGBColorspace)
            status=SetImageColorspace(image,RGBColorspace);
          status=SetImageColorspace(image,CMYKColorspace);
        }
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass);
      image->matte=MagickFalse;
      break;
    }
    case ColorSeparationMatteType:
    {
      if (image->colorspace != CMYKColorspace)
        {
          if (image->colorspace != RGBColorspace)
            status=SetImageColorspace(image,RGBColorspace);
          status=SetImageColorspace(image,CMYKColorspace);
        }
      if (image->storage_class != DirectClass)
        status=SetImageStorageClass(image,DirectClass);
      if (image->matte == MagickFalse)
        (void) SetImageAlphaChannel(image,ResetAlphaChannel);
      break;
    }
    case OptimizeType:
    {
      status=SetImageType(image,GetImageType(image,&image->exception));
      break;
    }
    case UndefinedType:
      break;
  }
  image_info=DestroyImageInfo(image_info);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t I m a g e V i r t u a l P i x e l M e t h o d                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetImageVirtualPixelMethod() sets the "virtual pixels" method for the
%  image and returns the previous setting.  A virtual pixel is any pixel access
%  that is outside the boundaries of the image cache.
%
%  The format of the SetImageVirtualPixelMethod() method is:
%
%      VirtualPixelMethod SetImageVirtualPixelMethod(const Image *image,
%        const VirtualPixelMethod virtual_pixel_method)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o virtual_pixel_method: choose the type of virtual pixel.
%
*/
MagickExport VirtualPixelMethod SetImageVirtualPixelMethod(const Image *image,
  const VirtualPixelMethod virtual_pixel_method)
{
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  return(SetCacheVirtualPixelMethod(image,virtual_pixel_method));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S o r t C o l o r m a p B y I n t e n t s i t y                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SortColormapByIntensity() sorts the colormap of a PseudoClass image by
%  decreasing color intensity.
%
%  The format of the SortColormapByIntensity method is:
%
%      MagickBooleanType SortColormapByIntensity(Image *image)
%
%  A description of each parameter follows:
%
%    o image: A pointer to an Image structure.
%
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static int IntensityCompare(const void *x,const void *y)
{
  const PixelPacket
    *color_1,
    *color_2;

  int
    intensity;

  color_1=(const PixelPacket *) x;
  color_2=(const PixelPacket *) y;
  intensity=(int) PixelIntensityToQuantum(color_2)-
    (int) PixelIntensityToQuantum(color_1);
  return(intensity);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

MagickExport MagickBooleanType SortColormapByIntensity(Image *image)
{
  long
    y;

  MagickBooleanType
    status;

  register long
    i;

  unsigned short
    *pixels;

  ViewInfo
    **image_view;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->storage_class != PseudoClass)
    return(MagickTrue);
  /*
    Allocate memory for pixel indexes.
  */
  pixels=(unsigned short *) AcquireQuantumMemory((size_t) image->colors,
    sizeof(*pixels));
  if (pixels == (unsigned short *) NULL)
    ThrowBinaryException(ResourceLimitError,"MemoryAllocationFailed",
      image->filename);
  /*
    Assign index values to colormap entries.
  */
  #pragma omp parallel for
  for (i=0; i < (long) image->colors; i++)
    image->colormap[i].opacity=(IndexPacket) i;
  /*
    Sort image colormap by decreasing color popularity.
  */
  qsort((void *) image->colormap,(size_t) image->colors,
    sizeof(*image->colormap),IntensityCompare);
  /*
    Update image colormap indexes to sorted colormap order.
  */
  #pragma omp parallel for
  for (i=0; i < (long) image->colors; i++)
    pixels[(long) image->colormap[i].opacity]=(unsigned short) i;
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    IndexPacket
      index;

    register long
      id,
      x;

    register IndexPacket
      *indexes;

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
      index=(IndexPacket) pixels[(long) indexes[x]];
      indexes[x]=index;
      *q++=image->colormap[(long) index];
    }
  }
  image_view=DestroyCacheViewThreadSet(image_view);
  pixels=(unsigned short *) RelinquishMagickMemory(pixels);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r i p I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StripImage() strips an image of all profiles and comments.
%
%  The format of the StripImage method is:
%
%      MagickBooleanType StripImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport MagickBooleanType StripImage(Image *image)
{
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  DestroyImageProfiles(image);
  (void) DeleteImageProperty(image,"comment");
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   S y n c I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncImage() initializes the red, green, and blue intensities of each pixel
%  as defined by the colormap index.
%
%  The format of the SyncImage method is:
%
%      MagickBooleanType SyncImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
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

MagickExport MagickBooleanType SyncImage(Image *image)
{
  long
    y;

  MagickBooleanType
    range_exception,
    status;

  ViewInfo
    **image_view;

  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (image->storage_class == DirectClass)
    return(MagickFalse);
  range_exception=MagickFalse;
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    IndexPacket
      index;

    PixelPacket
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
    for (x=0; x < (long) image->columns; x++)
    {
      index=PushColormapIndex(image,(unsigned long) indexes[x],
        &range_exception);
      pixel=image->colormap[(long) index];
      q->red=pixel.red;
      q->green=pixel.green;
      q->blue=pixel.blue;
      q++;
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      status=MagickFalse;
  }
  image_view=DestroyCacheViewThreadSet(image_view);
  if (range_exception != MagickFalse)
    (void) ThrowMagickException(&image->exception,GetMagickModule(),
      CorruptImageError,"InvalidColormapIndex","`%s'",image->filename);
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     T e x t u r e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  TextureImage() repeatedly tiles the texture image across and down the image
%  canvas.
%
%  The format of the TextureImage method is:
%
%      MagickBooleanType TextureImage(Image *image,const Image *texture)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o texture: This image is the texture to layer on the background.
%
*/
MagickExport MagickBooleanType TextureImage(Image *image,const Image *texture)
{
#define TextureImageTag  "Texture/Image"
  long
    y;

  MagickBooleanType
    status;

  ViewInfo
    **image_view,
    **texture_view;

  /*
    Tile texture onto the image background.
  */
  assert(image != (Image *) NULL);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(image->signature == MagickSignature);
  if (texture == (const Image *) NULL)
    return(MagickFalse);
  if (SetImageStorageClass(image,DirectClass) == MagickFalse)
    return(MagickFalse);
  image_view=AcquireCacheViewThreadSet(image);
  texture_view=AcquireCacheViewThreadSet(texture);
  status=MagickTrue;
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    const PixelPacket
      *pixels;

    MagickPixelPacket
      composite,
      source;

    register long
      id,
      x;

    register const IndexPacket
      *texture_indexes;

    register const PixelPacket
      *p;

    register IndexPacket
      *indexes;

    register PixelPacket
      *q;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    (void) SetCacheViewVirtualPixelMethod(texture_view[id],
      TileVirtualPixelMethod);
    p=AcquireCacheViewPixels(texture_view[id],texture->tile_offset.x,(y+
      texture->tile_offset.y) % texture->rows,texture->columns,1,
      &image->exception);
    q=GetCacheViewPixels(image_view[id],0,y,image->columns,1);
    if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      {
        status=MagickFalse;
        continue;
      }
    texture_indexes=AcquireCacheViewIndexes(texture_view[id]);
    indexes=GetCacheViewIndexes(image_view[id]);
    GetMagickPixelPacket(image,&source);
    GetMagickPixelPacket(texture,&composite);
    pixels=p;
    for (x=0; x < (long) image->columns; x+=texture->columns)
    {
      register long
        z;

      unsigned long
        width;

      width=texture->columns;
      if ((unsigned long) (x+width) > image->columns)
        width=image->columns-x;
      p=pixels;
      for (z=0; z < (long) width; z++)
      {
        SetMagickPixelPacket(image,p,texture_indexes+x+z,&source);
        SetMagickPixelPacket(image,q,indexes+x+z,&composite);
        MagickPixelCompositeOver(&source,(texture->matte != MagickFalse ?
          source.opacity : OpaqueOpacity),&composite,(image->matte !=
          MagickFalse ? composite.opacity : OpaqueOpacity),&composite);
        SetPixelPacket(image,&composite,q,indexes+x+z);
        p++;
        q++;
      }
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      status=MagickFalse;
    if (SetImageProgress(image,TextureImageTag,y,image->rows) == MagickFalse)
      status=MagickFalse;
  }
  texture_view=DestroyCacheViewThreadSet(texture_view);
  image_view=DestroyCacheViewThreadSet(image_view);
  return(status);
}
