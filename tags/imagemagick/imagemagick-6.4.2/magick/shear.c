/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      SSSSS  H   H  EEEEE   AAA    RRRR                      %
%                      SS     H   H  E      A   A   R   R                     %
%                       SSS   HHHHH  EEE    AAAAA   RRRR                      %
%                         SS  H   H  E      A   A   R R                       %
%                      SSSSS  H   H  EEEEE  A   A   R  R                      %
%                                                                             %
%                                                                             %
%            Methods to Shear or Rotate an Image by an Arbitrary Angle        %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                  July 1992                                  %
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
%  The RotateImage, XShearImage, and YShearImage methods are based on the
%  paper "A Fast Algorithm for General Raster Rotatation" by Alan W. Paeth,
%  Graphics Interface '86 (Vancouver).  RotateImage is adapted from a similar
%  method based on the Paeth paper written by Michael Halle of the Spatial
%  Imaging Group, MIT Media Lab.
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/artifact.h"
#include "magick/blob-private.h"
#include "magick/cache-private.h"
#include "magick/color-private.h"
#include "magick/colorspace-private.h"
#include "magick/composite.h"
#include "magick/composite-private.h"
#include "magick/decorate.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/geometry.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/memory_.h"
#include "magick/list.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-private.h"
#include "magick/quantum.h"
#include "magick/resource_.h"
#include "magick/shear.h"
#include "magick/statistic.h"
#include "magick/threshold.h"
#include "magick/transform.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     A f f i n e T r a n s f o r m I m a g e                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AffineTransformImage() transforms an image as dictated by the affine matrix.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the AffineTransformImage method is:
%
%      Image *AffineTransformImage(const Image *image,
%        AffineMatrix *affine_matrix,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o affine_matrix: the affine matrix.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport Image *AffineTransformImage(const Image *image,
  const AffineMatrix *affine_matrix,ExceptionInfo *exception)
{
  AffineMatrix
    transform;

  Image
    *affine_image;

  PointInfo
    extent[4],
    min,
    max,
    point;

  register long
    i;

  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(affine_matrix != (AffineMatrix *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  /*
    Determine destination image bounding box.
  */
  extent[0].x=(double) image->page.x;
  extent[0].y=(double) image->page.y;
  extent[1].x=(double) (image->page.x+image->columns);
  extent[1].y=(double) image->page.y;
  extent[2].x=(double) (image->page.x+image->columns);
  extent[2].y=(double) (image->page.y+image->rows);
  extent[3].x=(double) image->page.x;
  extent[3].y=(double) (image->page.y+image->rows);
  for (i=0; i < 4; i++)
  {
    point=extent[i];
    extent[i].x=(double) (point.x*affine_matrix->sx+point.y*affine_matrix->ry+
      affine_matrix->tx);
    extent[i].y=(double) (point.x*affine_matrix->rx+point.y*affine_matrix->sy+
      affine_matrix->ty);
  }
  min=extent[0];
  max=extent[0];
  for (i=1; i < 4; i++)
  {
    if (min.x > extent[i].x)
      min.x=extent[i].x;
    if (min.y > extent[i].y)
      min.y=extent[i].y;
    if (max.x < extent[i].x)
      max.x=extent[i].x;
    if (max.y < extent[i].y)
      max.y=extent[i].y;
  }
  affine_image=CloneImage(image,(unsigned long) (max.x-min.x+0.5),
    (unsigned long) (max.y-min.y+0.5),MagickTrue,exception);
  if (affine_image == (Image *) NULL)
    return((Image *) NULL);
  affine_image->background_color.opacity=(Quantum) TransparentOpacity;
  (void) SetImageBackgroundColor(affine_image);
  /*
    Adjust transform translation for direct image to image transformation.
    That is, pixel 0,0 translates correctly to its location in destination.
  */
  transform=(*affine_matrix);
  transform.tx=extent[0].x-min.x;
  transform.ty=extent[0].y-min.y;
  /*
    Affine transform image.
  */
  (void) DrawAffineImage(affine_image,image,&transform);
  /*
    Add offset to resulting image.
  */
  affine_image->page.x=(long) floor(min.x+0.5);
  affine_image->page.y=(long) floor(min.y+0.5);
  /*
    Roughly calculate an appropriate virtual canvas (page) size.
  */
  affine_image->page.width=affine_image->columns;
  if (affine_image->page.x > 0)
    affine_image->page.width+=affine_image->page.x;
  affine_image->page.height=affine_image->rows;
  if (affine_image->page.y > 0)
    affine_image->page.height+=affine_image->page.y;
  return(affine_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C r o p T o F i t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CropToFitImage() crops the sheared image as determined by the bounding box
%  as defined by width and height and shearing angles.
%
%  The format of the CropToFitImage method is:
%
%      Image *CropToFitImage(Image **image,const MagickRealType x_shear,
%        const MagickRealType x_shear,const MagickRealType width,
%        const MagickRealType height,const MagickBooleanType rotate,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o x_shear, y_shear, width, height: Defines a region of the image to crop.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
static inline void CropToFitImage(Image **image,const MagickRealType x_shear,
  const MagickRealType y_shear,const MagickRealType width,
  const MagickRealType height,const MagickBooleanType rotate,
  ExceptionInfo *exception)
{
  Image
    *crop_image;

  PointInfo
    extent[4],
    min,
    max;

  RectangleInfo
    geometry,
    page;

  register long
    i;

  /*
    Calculate the rotated image size.
  */
  extent[0].x=(double) (-width/2.0);
  extent[0].y=(double) (-height/2.0);
  extent[1].x=(double) width/2.0;
  extent[1].y=(double) (-height/2.0);
  extent[2].x=(double) (-width/2.0);
  extent[2].y=(double) height/2.0;
  extent[3].x=(double) width/2.0;
  extent[3].y=(double) height/2.0;
  for (i=0; i < 4; i++)
  {
    extent[i].x+=x_shear*extent[i].y;
    extent[i].y+=y_shear*extent[i].x;
    if (rotate != MagickFalse)
      extent[i].x+=x_shear*extent[i].y;
    extent[i].x+=(double) (*image)->columns/2.0;
    extent[i].y+=(double) (*image)->rows/2.0;
  }
  min=extent[0];
  max=extent[0];
  for (i=1; i < 4; i++)
  {
    if (min.x > extent[i].x)
      min.x=extent[i].x;
    if (min.y > extent[i].y)
      min.y=extent[i].y;
    if (max.x < extent[i].x)
      max.x=extent[i].x;
    if (max.y < extent[i].y)
      max.y=extent[i].y;
  }
  geometry.x=(long) (min.x+0.5);
  geometry.y=(long) (min.y+0.5);
  geometry.width=(unsigned long) ((long) (max.x+0.5)-(long) (min.x+0.5));
  geometry.height=(unsigned long) ((long) (max.y+0.5)-(long) (min.y+0.5));
  page=(*image)->page;
  (void) ParseAbsoluteGeometry("0x0+0+0",&(*image)->page);
  crop_image=CropImage(*image,&geometry,exception);
  (*image)->page=page;
  if (crop_image != (Image *) NULL)
    {
      crop_image->page=page;
      *image=DestroyImage(*image);
      *image=crop_image;
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%     D e s k e w I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DeskewImage() utilizes a radon transform to straighten an image and returns
%  it.
%
%  The format of the DeskewImage method is:
%
%      Image *DeskewImage(const Image *image,const double threshold,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o threshold: deskewing threshold.
%
%    o exception: Return any errors or warnings in this structure.
%
*/

typedef struct _RadonInfo
{
  CacheType
    type;

  unsigned long
    width,
    height;

  MagickSizeType
    length;

  MagickBooleanType
    mapped;

  char
    path[MaxTextExtent];

  int
    file;

  unsigned short
    *cells;
} RadonInfo;

static void *DestroyRadonInfo(RadonInfo *radon_info)
{
  assert(radon_info != (RadonInfo *) NULL);
  switch (radon_info->type)
  {
    case MemoryCache:
    {
      if (radon_info->mapped == MagickFalse)
        radon_info->cells=(unsigned short *) RelinquishMagickMemory(
          radon_info->cells);
      else
        radon_info->cells=(unsigned short *) UnmapBlob(radon_info->cells,
          (size_t) radon_info->length);
      RelinquishMagickResource(MemoryResource,radon_info->length);
      break;
    }
    case MapCache:
    {
      radon_info->cells=(unsigned short *) UnmapBlob(radon_info->cells,(size_t)
        radon_info->length);
      RelinquishMagickResource(MapResource,radon_info->length);
    }
    case DiskCache:
    {
      if (radon_info->file != -1)
        (void) close(radon_info->file);
      (void) RelinquishUniqueFileResource(radon_info->path);
      RelinquishMagickResource(DiskResource,radon_info->length);
      break;
    }
    default:
      break;
  }
  return((RadonInfo *) RelinquishMagickMemory(radon_info));
}

static MagickBooleanType ResetRadonCells(RadonInfo *radon_info)
{
  long
    y;

  register long
    x;

  ssize_t
    count;

  unsigned short
    value;

  if (radon_info->type != DiskCache)
    {
      (void) ResetMagickMemory(radon_info->cells,0,(size_t) radon_info->length);
      return(MagickTrue);
    }
  value=0;
  (void) MagickSeek(radon_info->file,0,SEEK_SET);
  for (y=0; y < (long) radon_info->height; y++)
  {
    for (x=0; x < (long) radon_info->width; x++)
    {
      count=write(radon_info->file,&value,sizeof(*radon_info->cells));
      if (count != (ssize_t) sizeof(*radon_info->cells))
        break;
    }
    if (x < (long) radon_info->width)
      break;
  }
  return(y < (long) radon_info->height ? MagickFalse : MagickTrue);
}

static RadonInfo *AcquireRadonInfo(const Image *image,const unsigned long width,
  const unsigned long height,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  RadonInfo
    *radon_info;

  radon_info=(RadonInfo *) AcquireMagickMemory(sizeof(*radon_info));
  if (radon_info == (RadonInfo *) NULL)
    return((RadonInfo *) NULL);
  (void) ResetMagickMemory(radon_info,0,sizeof(*radon_info));
  radon_info->width=width;
  radon_info->height=height;
  radon_info->length=(MagickSizeType) width*height*sizeof(*radon_info->cells);
  radon_info->type=MemoryCache;
  status=AcquireMagickResource(AreaResource,radon_info->length);
  if ((status != MagickFalse) &&
      (radon_info->length == (MagickSizeType) ((size_t) radon_info->length)))
    {
      status=AcquireMagickResource(MemoryResource,radon_info->length);
      if (status != MagickFalse)
        {
          radon_info->cells=(unsigned short *) MapBlob(-1,IOMode,0,(size_t)
            radon_info->length);
          if (radon_info->cells != (unsigned short *) NULL)
            radon_info->mapped=MagickTrue;
          else
            radon_info->cells=(unsigned short *) AcquireMagickMemory((size_t)
              radon_info->length);
          if (radon_info->cells == (unsigned short *) NULL)
            RelinquishMagickResource(MemoryResource,radon_info->length);
        }
    }
  radon_info->file=(-1);
  if (radon_info->cells == (unsigned short *) NULL)
    {
      status=AcquireMagickResource(DiskResource,radon_info->length);
      if (status == MagickFalse)
        {
          (void) ThrowMagickException(exception,GetMagickModule(),CacheError,
            "CacheResourcesExhausted","`%s'",image->filename);
          return(DestroyRadonInfo(radon_info));
        }
      radon_info->type=DiskCache;
      (void) AcquireMagickResource(MemoryResource,radon_info->length);
      radon_info->file=AcquireUniqueFileResource(radon_info->path);
      if (radon_info->file == -1)
        return(DestroyRadonInfo(radon_info));
      status=AcquireMagickResource(MapResource,radon_info->length);
      if (status != MagickFalse)
        {
          status=ResetRadonCells(radon_info);
          if (status != MagickFalse)
            {
              radon_info->cells=(unsigned short *) MapBlob(radon_info->file,
                IOMode,0,(size_t) radon_info->length);
              if (radon_info->cells != (unsigned short *) NULL)
                radon_info->type=MapCache;
              else
                RelinquishMagickResource(MapResource,radon_info->length);
            }
        }
    }
  return(radon_info);
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static inline ssize_t ReadRadonCell(const RadonInfo *radon_info,
  const off_t offset,const size_t length,unsigned char *buffer)
{
  register ssize_t
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PPREAD)
  #pragma omp critical
  {
    i=(-1);
    if (MagickSeek(radon_info->file,offset,SEEK_SET) >= 0)
      {
#endif
        count=0;
        for (i=0; i < (ssize_t) length; i+=count)
        {
#if !defined(MAGICKCORE_HAVE_PPREAD)
          count=read(radon_info->file,buffer+i,MagickMin(length-i,(size_t)
            SSIZE_MAX));
#else
          count=pread(radon_info->file,buffer+i,MagickMin(length-i,(size_t)
            SSIZE_MAX),(off_t) (offset+i));
#endif
          if (count > 0)
            continue;
          count=0;
          if (errno != EINTR)
            {
              i=(-1);
              break;
            }
        }
#if !defined(MAGICKCORE_HAVE_PPREAD)
      }
  }
#endif
  return(i);
}

static inline ssize_t WriteRadonCell(const RadonInfo *radon_info,
  const off_t offset,const size_t length,const unsigned char *buffer)
{
  register ssize_t
    i;

  ssize_t
    count;

#if !defined(MAGICKCORE_HAVE_PPWRITE)
  #pragma omp critical
  {
    if (MagickSeek(radon_info->file,offset,SEEK_SET) >= 0)
      {
#endif
        count=0;
        for (i=0; i < (ssize_t) length; i+=count)
        {
#if !defined(MAGICKCORE_HAVE_PPWRITE)
          count=write(radon_info->file,buffer+i,MagickMin(length-i,(size_t)
            SSIZE_MAX));
#else
          count=pwrite(radon_info->file,buffer+i,MagickMin(length-i,(size_t)
            SSIZE_MAX),(off_t) (offset+i));
#endif
          if (count > 0)
            continue;
          count=0;
          if (errno != EINTR)
            {
              i=(-1);
              break;
            }
        }
#if !defined(MAGICKCORE_HAVE_PPWRITE)
      }
  }
#endif
  return(i);
}

static inline unsigned short GetRadonCell(const RadonInfo *radon_info,
  const long x,const long y)
{
  off_t
    i;

  unsigned short
    value;

  i=(off_t) radon_info->height*x+y;
  if ((i < 0) ||
      ((MagickSizeType) (i*sizeof(*radon_info->cells)) >= radon_info->length))
    return(0);
  if (radon_info->type != DiskCache)
    return(radon_info->cells[i]);
  value=0;
  (void) ReadRadonCell(radon_info,i*sizeof(*radon_info->cells),
    sizeof(*radon_info->cells),(unsigned char *) &value);
  return(value);
}

static inline MagickBooleanType SetRadonCell(const RadonInfo *radon_info,
  const long x,const long y,const unsigned short value)
{
  off_t
    i;

  ssize_t
    count;

  i=(off_t) radon_info->height*x+y;
  if ((i < 0) ||
      ((MagickSizeType) (i*sizeof(*radon_info->cells)) >= radon_info->length))
    return(0);
  if (radon_info->type != DiskCache)
    {
      radon_info->cells[i]=value;
      return(MagickTrue);
    }
  count=WriteRadonCell(radon_info,i*sizeof(*radon_info->cells),
    sizeof(*radon_info->cells),(unsigned char *) &value);
  if (count != (ssize_t) sizeof(*radon_info->cells))
    return(MagickFalse);
  return(MagickTrue);
}

static void RadonProjection(RadonInfo *source_cells,
  RadonInfo *destination_cells,const long sign,unsigned long *projection)
{
  RadonInfo
    *swap;

  register long
    x;

  register RadonInfo
    *p,
    *q;

  unsigned long
    step;

  p=source_cells;
  q=destination_cells;
  for (step=1; step < p->width; step*=2)
  {
    #pragma omp parallel for
    for (x=0; x < (long) p->width; x+=2*step)
    {
      long
        y;

      register long
        i;

      unsigned short
        cell;

      for (i=0; i < (long) step; i++)
      {
        for (y=0; y < (long) (p->height-i-1); y++)
        {
          cell=GetRadonCell(p,x+i,y);
          (void) SetRadonCell(q,x+2*i,y,cell+GetRadonCell(p,x+i+step,y+i));
          (void) SetRadonCell(q,x+2*i+1,y,cell+GetRadonCell(p,x+i+step,y+i+1));
        }
        for ( ; y < (long) (p->height-i); y++)
        {
          cell=GetRadonCell(p,x+i,y);
          (void) SetRadonCell(q,x+2*i,y,cell+GetRadonCell(p,x+i+step,y+i));
          (void) SetRadonCell(q,x+2*i+1,y,cell);
        }
        for ( ; y < (long) p->height; y++)
        {
          cell=GetRadonCell(p,x+i,y);
          (void) SetRadonCell(q,x+2*i,y,cell);
          (void) SetRadonCell(q,x+2*i+1,y,cell);
        }
      }
    }
    swap=p;
    p=q;
    q=swap;
  }
  #pragma omp parallel for
  for (x=0; x < (long) p->width; x++)
  {
    register long
      y;

    unsigned long
      sum;

    sum=0;
    for (y=0; y < (long) (p->height-1); y++)
    {
      long
        delta;

      delta=GetRadonCell(p,x,y)-(long) GetRadonCell(p,x,y+1);
      sum+=delta*delta;
    }
    projection[p->width+sign*x-1]=sum;
  }
}

static MagickBooleanType RadonTransform(const Image *image,
  const double threshold,unsigned long *projection,ExceptionInfo *exception)
{
  long
    y;

  MagickBooleanType
    status;

  RadonInfo
    *destination_cells,
    *source_cells;

  register long
    i;

  unsigned char
    byte;

  unsigned long
    count,
    width;

  unsigned short
    bits[256];

  ViewInfo
    **image_view;

  for (width=1; width < ((image->columns+7)/8); width<<=1) ;
  source_cells=AcquireRadonInfo(image,width,image->rows,exception);
  destination_cells=AcquireRadonInfo(image,width,image->rows,exception);
  if ((source_cells == (RadonInfo *) NULL) ||
      (destination_cells == (RadonInfo *) NULL))
    {
      if (destination_cells != (RadonInfo *) NULL)
        destination_cells=DestroyRadonInfo(destination_cells);
      if (source_cells != (RadonInfo *) NULL)
        source_cells=DestroyRadonInfo(source_cells);
      return(MagickFalse);
    }
  if (ResetRadonCells(source_cells) == MagickFalse)
    {
      destination_cells=DestroyRadonInfo(destination_cells);
      source_cells=DestroyRadonInfo(source_cells);
      return(MagickFalse);
    }
  for (i=0; i < 256; i++)
  {
    byte=(unsigned char) i;
    for (count=0; byte != 0; byte>>=1)
      count+=byte & 0x01;
    bits[i]=(unsigned short) count;
  }
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *p;

    register long
      i,
      id,
      x;

    unsigned long
      bit,
      byte;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    bit=0;
    byte=0;
    i=(long) (image->columns+7)/8;
    for (x=0; x < (long) image->columns; x++)
    {
      byte<<=1;
      if (((MagickRealType) p->red < threshold) ||
          ((MagickRealType) p->green < threshold) ||
          ((MagickRealType) p->blue < threshold))
        byte|=0x01;
      bit++;
      if (bit == 8)
        {
          (void) SetRadonCell(source_cells,--i,y,bits[byte]);
          bit=0;
          byte=0;
        }
      p++;
    }
    if (bit != 0)
      {
        byte<<=(8-bit);
        (void) SetRadonCell(source_cells,--i,y,bits[byte]);
      }
  }
  RadonProjection(source_cells,destination_cells,-1,projection);
  (void) ResetRadonCells(source_cells);
  #pragma omp parallel for
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *p;

    register long
      i,
      id,
      x;

    unsigned long
      bit,
      byte;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    bit=0;
    byte=0;
    i=0;
    for (x=0; x < (long) image->columns; x++)
    {
      byte<<=1;
      if (((MagickRealType) p->red < threshold) ||
          ((MagickRealType) p->green < threshold) ||
          ((MagickRealType) p->blue < threshold))
        byte|=0x01;
      bit++;
      if (bit == 8)
        {
          (void) SetRadonCell(source_cells,i++,y,bits[byte]);
          bit=0;
          byte=0;
        }
      p++;
    }
    if (bit != 0)
      {
        byte<<=(8-bit);
        (void) SetRadonCell(source_cells,i++,y,bits[byte]);
      }
  }
  RadonProjection(source_cells,destination_cells,1,projection);
  image_view=DestroyCacheViewThreadSet(image_view);
  destination_cells=DestroyRadonInfo(destination_cells);
  source_cells=DestroyRadonInfo(source_cells);
  return(MagickTrue);
}

static void GetImageBackgroundColor(Image *image,const long offset,
  ExceptionInfo *exception)
{
  long
    y;

  MagickPixelPacket
    background;

  MagickRealType
    count;

  ViewInfo
    *image_view;

  /*
    Compute average background color.
  */
  if (offset <= 0)
    return;
  GetMagickPixelPacket(image,&background);
  count=0.0;
  image_view=AcquireCacheView(image);
  for (y=0; y < (long) image->rows; y++)
  {
    register const PixelPacket
      *p;

    register long
      x;

    if ((y >= offset) && (y < ((long) image->rows-offset)))
      continue;
    p=AcquireCacheViewPixels(image_view,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      continue;
    for (x=0; x < (long) image->columns; x++)
    {
      if ((x >= offset) && (x < ((long) image->columns-offset)))
        continue;
      background.red+=QuantumScale*p->red;
      background.green+=QuantumScale*p->green;
      background.blue+=QuantumScale*p->blue;
      background.opacity+=QuantumScale*p->opacity;
      count++;
      p++;
    }
  }
  image_view=DestroyCacheView(image_view);
  image->background_color.red=RoundToQuantum((MagickRealType) QuantumRange*
    background.red/count);
  image->background_color.green=RoundToQuantum((MagickRealType) QuantumRange*
    background.green/count);
  image->background_color.blue=RoundToQuantum((MagickRealType) QuantumRange*
    background.blue/count);
  image->background_color.opacity=RoundToQuantum((MagickRealType) QuantumRange*
    background.opacity/count);
}

MagickExport Image *DeskewImage(const Image *image,const double threshold,
  ExceptionInfo *exception)
{
  const char
    *artifact;

  double
    degrees,
    sum;

  Image
    *clone_image,
    *median_image,
    *rotate_image;

  long
    skew;

  MagickBooleanType
    status;

  RectangleInfo
    geometry;

  register long
    i;

  unsigned long
    max_projection,
    *projection,
    width;

  /*
    Compute deskew angle.
  */
  for (width=1; width < ((image->columns+7)/8); width<<=1) ;
  projection=(unsigned long *) AcquireQuantumMemory((size_t) (2*width-1),
    sizeof(*projection));
  if (projection == (unsigned long *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  status=RadonTransform(image,threshold,projection,exception);
  if (status == MagickFalse)
    {
      projection=(unsigned long *) RelinquishMagickMemory(projection);
      ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
    }
  max_projection=0;
  sum=0.0;
  skew=0;
  for (i=0; i < (long) (2*width-1); i++)
  {
    if (projection[i] > max_projection)
      {
        skew=i-(long) width+1;
        max_projection=projection[i];
      }
    sum+=projection[i];
  }
  projection=(unsigned long *) RelinquishMagickMemory(projection);
  /*
    Deskew image.
  */
  degrees=RadiansToDegrees(-atan((double) skew/width/8));
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TransformEvent,GetMagickModule(),"  Deskew angle: %g",
      degrees);
  artifact=GetImageArtifact(image,"deskew:auto-crop");
  if (artifact == (const char *) NULL)
    return(RotateImage(image,degrees,exception));
  /*
    Auto-crop image.
  */
  clone_image=CloneImage(image,0,0,MagickTrue,exception);
  if (clone_image == (Image *) NULL)
    return((Image *) NULL);
  GetImageBackgroundColor(clone_image,atol(artifact),exception);
  rotate_image=RotateImage(clone_image,degrees,exception);
  clone_image=DestroyImage(clone_image);
  if (rotate_image == (Image *) NULL)
    return((Image *) NULL);
  median_image=MedianFilterImage(rotate_image,0.0,exception);
  rotate_image=DestroyImage(rotate_image);
  if (median_image == (Image *) NULL)
    return((Image *) NULL);
  geometry=GetImageBoundingBox(median_image,exception);
  median_image=DestroyImage(median_image);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TransformEvent,GetMagickModule(),"  Deskew geometry: "
      "%lux%lu%+ld%+ld",geometry.width,geometry.height,geometry.x,geometry.y);
  return(CropImage(image,&geometry,exception));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n t e g r a l R o t a t e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IntegralRotateImage()  rotates the image an integral of 90 degrees.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the rotated image.
%
%  The format of the IntegralRotateImage method is:
%
%      Image *IntegralRotateImage(const Image *image,unsigned long rotations,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o rotations: Specifies the number of 90 degree rotations.
%
*/
static Image *IntegralRotateImage(const Image *image,unsigned long rotations,
  ExceptionInfo *exception)
{
#define TileHeight  128
#define TileWidth  128
#define RotateImageTag  "Rotate/Image"

  Image
    *rotate_image;

  long
    y;

  MagickBooleanType
    status;

  RectangleInfo
    page;

  ViewInfo
    **image_view,
    **rotate_view;

  /*
    Initialize rotated image attributes.
  */
  assert(image != (Image *) NULL);
  page=image->page;
  rotations%=4;
  if ((rotations == 1) || (rotations == 3))
    rotate_image=CloneImage(image,image->rows,image->columns,MagickTrue,
      exception);
  else
    rotate_image=CloneImage(image,image->columns,image->rows,MagickTrue,
      exception);
  if (rotate_image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Integral rotate the image.
  */
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
  rotate_view=AcquireCacheViewThreadSet(rotate_image);
  switch (rotations)
  {
    case 0:
    {
      /*
        Rotate 0 degrees.
      */
      #pragma omp parallel for
      for (y=0; y < (long) image->rows; y++)
      {
        register const IndexPacket
          *indexes;

        register const PixelPacket
          *p;

        register IndexPacket
          *rotate_indexes;

        register long
          id;

        register PixelPacket
          *q;

        if (status == MagickFalse)
          continue;
        id=GetCacheViewThreadId();
        p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
        q=SetCacheViewPixels(rotate_view[id],0,y,rotate_image->columns,1);
        if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        indexes=AcquireCacheViewIndexes(image_view[id]);
        rotate_indexes=GetCacheViewIndexes(rotate_view[id]);
        (void) CopyMagickMemory(q,p,image->columns*sizeof(*p));
        if ((indexes != (IndexPacket *) NULL) &&
            (rotate_indexes != (IndexPacket *) NULL))
          (void) CopyMagickMemory(rotate_indexes,indexes,image->columns*
            sizeof(*indexes));
        if (SyncCacheView(rotate_view[id]) == MagickFalse)
          status=MagickFalse;
        if (SetImageProgress(image,RotateImageTag,y,image->rows) == MagickFalse)
          status=MagickFalse;
      }
      break;
    }
    case 1:
    {
      /*
        Rotate 90 degrees.
      */
      #pragma omp parallel for
      for (y=0; y < (long) image->rows; y++)
      {
        register const IndexPacket
          *indexes;

        register const PixelPacket
          *p;

        register IndexPacket
          *rotate_indexes;

        register long
          id;

        register PixelPacket
          *q;

        if (status == MagickFalse)
          continue;
        id=GetCacheViewThreadId();
        p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
        q=SetCacheViewPixels(rotate_view[id],(long) (image->rows-y-1),0,1,
          rotate_image->rows);
        if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        indexes=GetCacheViewIndexes(image_view[id]);
        rotate_indexes=GetCacheViewIndexes(rotate_view[id]);
        (void) CopyMagickMemory(q,p,(size_t) image->columns*sizeof(*q));
        if ((indexes != (IndexPacket *) NULL) &&
            (rotate_indexes != (IndexPacket *) NULL))
          (void) CopyMagickMemory(rotate_indexes,indexes,(size_t)
            image->columns*sizeof(*rotate_indexes));
        if (SyncCacheView(rotate_view[id]) == MagickFalse)
          status=MagickFalse;
        if (SetImageProgress(image,RotateImageTag,y,image->rows) == MagickFalse)
          status=MagickFalse;
      }
      Swap(page.width,page.height);
      Swap(page.x,page.y);
      if (page.width != 0)
        page.x=(long) (page.width-rotate_image->columns-page.x);
      break;
    }
    case 2:
    {
      /*
        Rotate 180 degrees.
      */
      #pragma omp parallel for
      for (y=0; y < (long) image->rows; y++)
      {
        register const IndexPacket
          *indexes;

        register const PixelPacket
          *p;

        register IndexPacket
          *rotate_indexes;

        register long
          id,
          x;

        register PixelPacket
          *q;

        if (status == MagickFalse)
          continue;
        id=GetCacheViewThreadId();
        p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
        q=SetCacheViewPixels(rotate_view[id],0,(long) (image->rows-y-1),
          image->columns,1);
        if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        indexes=AcquireCacheViewIndexes(image_view[id]);
        rotate_indexes=GetCacheViewIndexes(rotate_view[id]);
        q+=image->columns;
        for (x=0; x < (long) image->columns; x++)
          *--q=(*p++);
        if ((indexes != (IndexPacket *) NULL) &&
            (rotate_indexes != (IndexPacket *) NULL))
          for (x=0; x < (long) image->columns; x++)
            rotate_indexes[image->columns-x-1]=indexes[x];
        if (SyncCacheView(rotate_view[id]) == MagickFalse)
          status=MagickFalse;
        if (SetImageProgress(image,RotateImageTag,y,image->rows) == MagickFalse)
          status=MagickFalse;
      }
      if (page.width != 0)
        page.x=(long) (page.width-rotate_image->columns-page.x);
      if (page.height != 0)
        page.y=(long) (page.height-rotate_image->rows-page.y);
      break;
    }
    case 3:
    {
      /*
        Rotate 270 degrees.
      */
      #pragma omp parallel for
      for (y=0; y < (long) image->rows; y++)
      {
        register const IndexPacket
          *indexes;

        register const PixelPacket
          *p;

        register IndexPacket
          *rotate_indexes;

        register long
          id,
          x;

        register PixelPacket
          *q;

        if (status == MagickFalse)
          continue;
        id=GetCacheViewThreadId();
        p=AcquireCacheViewPixels(image_view[id],0,y,image->columns,1,exception);
        q=SetCacheViewPixels(rotate_view[id],y,0,1,rotate_image->rows);
        if ((p == (const PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          {
            status=MagickFalse;
            continue;
          }
        indexes=AcquireCacheViewIndexes(image_view[id]);
        rotate_indexes=GetCacheViewIndexes(rotate_view[id]);
        q+=image->columns;
        for (x=0; x < (long) image->columns; x++)
          *--q=(*p++);
        if ((indexes != (IndexPacket *) NULL) &&
            (rotate_indexes != (IndexPacket *) NULL))
          for (x=0; x < (long) image->columns; x++)
            rotate_indexes[image->columns-x-1]=indexes[x];
        if (SyncCacheView(rotate_view[id]) == MagickFalse)
          status=MagickFalse;
        if (SetImageProgress(image,RotateImageTag,y,image->rows) == MagickFalse)
          status=MagickFalse;
      }
      Swap(page.width,page.height);
      Swap(page.x,page.y);
      if (page.height != 0)
        page.y=(long) (page.height-rotate_image->rows-page.y);
      break;
    }
  }
  rotate_view=DestroyCacheViewThreadSet(rotate_view);
  image_view=DestroyCacheViewThreadSet(image_view);
  rotate_image->type=image->type;
  rotate_image->page=page;
  if (status == MagickFalse)
    rotate_image=DestroyImage(rotate_image);
  return(rotate_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X S h e a r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  XShearImage() shears the image in the X direction with a shear angle of
%  'degrees'.  Positive angles shear counter-clockwise (right-hand rule), and
%  negative angles shear clockwise.  Angles are measured relative to a vertical
%  Y-axis.  X shears will widen an image creating 'empty' triangles on the left
%  and right sides of the source image.
%
%  The format of the XShearImage method is:
%
%      MagickBooleanType XShearImage(Image *image,const MagickRealType degrees,
%        const unsigned long width,const unsigned long height,
%        const long x_offset,const long y_offset)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: A MagickRealType representing the shearing angle along the X
%      axis.
%
%    o width, height, x_offset, y_offset: Defines a region of the image
%      to shear.
%
*/

static inline MagickRealType Blend_(const MagickRealType p,
  const MagickRealType alpha,const MagickRealType q,const MagickRealType beta)
{
  return((1.0-QuantumScale*alpha)*p+(1.0-QuantumScale*beta)*q);
}

static inline void MagickCompositeBlend(const MagickPixelPacket *p,
  const MagickRealType alpha,const MagickPixelPacket *q,
  const MagickRealType beta,const MagickRealType area,
  MagickPixelPacket *composite)
{
  MagickRealType
    gamma;

  if ((alpha == TransparentOpacity) && (beta == TransparentOpacity))
    {
      *composite=(*p);
      return;
    }
  gamma=RoundToUnity((1.0-QuantumScale*(QuantumRange-(1.0-area)*
    (QuantumRange-alpha)))+(1.0-QuantumScale*(QuantumRange-area*
    (QuantumRange-beta))));
  composite->opacity=(MagickRealType) QuantumRange*(1.0-gamma);
  gamma=1.0/(fabs(gamma) <= MagickEpsilon ? 1.0 : gamma);
  composite->red=gamma*Blend_((MagickRealType) p->red,
    (MagickRealType) QuantumRange-(1.0-area)*(QuantumRange-alpha),
    (MagickRealType) q->red,(MagickRealType) (QuantumRange-area*(QuantumRange-
    beta)));
  composite->green=gamma*Blend_((MagickRealType) p->green,
    (MagickRealType) QuantumRange-(1.0-area)*(QuantumRange-alpha),
    (MagickRealType) q->green,(MagickRealType) (QuantumRange-area*(QuantumRange-
    beta)));
  composite->blue=gamma*Blend_((MagickRealType) p->blue,
    (MagickRealType) QuantumRange-(1.0-area)*(QuantumRange-alpha),
    (MagickRealType) q->blue,(MagickRealType) (QuantumRange-area*(QuantumRange-
    beta)));
  if (q->colorspace == CMYKColorspace)
    composite->index=gamma*Blend_((MagickRealType) p->index,
      (MagickRealType) QuantumRange-(1.0-area)*(QuantumRange-alpha),
      (MagickRealType) q->index,(MagickRealType) (QuantumRange-area*
      (QuantumRange-beta)));
}

static MagickBooleanType XShearImage(Image *image,const MagickRealType degrees,
  const unsigned long width,const unsigned long height,const long x_offset,
  const long y_offset)
{
#define XShearImageTag  "XShear/Image"

  long
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    background;

  ViewInfo
    **image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  GetMagickPixelPacket(image,&background);
  SetMagickPixelPacket(image,&image->background_color,(IndexPacket *) NULL,
    &background);
  if (image->colorspace == CMYKColorspace)
    ConvertRGBToCMYK(&background);
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
//  #pragma omp parallel for
  for (y=0; y < (long) height; y++)
  {
    enum {LEFT, RIGHT}
      direction;

    IndexPacket
      *indexes,
      *shear_indexes;

    long
      id,
      step;

    MagickPixelPacket
      pixel,
      source,
      destination;

    MagickRealType
      area,
      displacement;

    register long
      i;

    register PixelPacket
      *p,
      *q;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    p=GetCacheViewPixels(image_view[id],0,y_offset+y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewIndexes(image_view[id]);
    p+=x_offset;
    indexes+=x_offset;
    displacement=degrees*(MagickRealType) (y-height/2.0);
    if (displacement == 0.0)
      continue;
    if (displacement > 0.0)
      direction=RIGHT;
    else
      {
        displacement*=(-1.0);
        direction=LEFT;
      }
    step=(long) floor((double) displacement);
    area=(MagickRealType) (displacement-step);
    step++;
    pixel=background;
    GetMagickPixelPacket(image,&source);
    GetMagickPixelPacket(image,&destination);
    switch (direction)
    {
      case LEFT:
      {
        /*
          Transfer pixels left-to-right.
        */
        if (step > x_offset)
          break;
        q=p-step;
        shear_indexes=indexes-step;
        for (i=0; i < (long) width; i++)
        {
          if ((x_offset+i) < step)
            {
              SetMagickPixelPacket(image,++p,++indexes,&pixel);
              q++;
              shear_indexes++;
              continue;
            }
          SetMagickPixelPacket(image,p,indexes,&source);
          MagickCompositeBlend(&pixel,(MagickRealType) pixel.opacity,&source,
            (MagickRealType) p->opacity,area,&destination);
          SetPixelPacket(image,&destination,q++,shear_indexes++);
          SetMagickPixelPacket(image,p++,indexes++,&pixel);
        }
        MagickCompositeBlend(&pixel,(MagickRealType) pixel.opacity,&background,
          (MagickRealType) background.opacity,area,&destination);
        SetPixelPacket(image,&destination,q++,shear_indexes++);
        for (i=0; i < (step-1); i++)
          SetPixelPacket(image,&background,q++,shear_indexes++);
        break;
      }
      case RIGHT:
      {
        /*
          Transfer pixels right-to-left.
        */
        p+=width;
        indexes+=width;
        q=p+step;
        shear_indexes=indexes+step;
        for (i=0; i < (long) width; i++)
        {
          p--;
          indexes--;
          q--;
          shear_indexes--;
          if ((unsigned long) (x_offset+width+step-i) >= image->columns)
            continue;
          SetMagickPixelPacket(image,p,indexes,&source);
          MagickCompositeBlend(&pixel,(MagickRealType) pixel.opacity,&source,
            (MagickRealType) p->opacity,area,&destination);
          SetPixelPacket(image,&destination,q,shear_indexes);
          SetMagickPixelPacket(image,p,indexes,&pixel);
        }
        MagickCompositeBlend(&pixel,(MagickRealType) pixel.opacity,&background,
          (MagickRealType) background.opacity,area,&destination);
        SetPixelPacket(image,&destination,--q,--shear_indexes);
        for (i=0; i < (step-1); i++)
          SetPixelPacket(image,&background,--q,--shear_indexes);
        break;
      }
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      status=MagickFalse;
    if (SetImageProgress(image,XShearImageTag,y,height) == MagickFalse)
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
+   Y S h e a r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  YShearImage shears the image in the Y direction with a shear angle of
%  'degrees'.  Positive angles shear counter-clockwise (right-hand rule), and
%  negative angles shear clockwise.  Angles are measured relative to a
%  horizontal X-axis.  Y shears will increase the height of an image creating
%  'empty' triangles on the top and bottom of the source image.
%
%  The format of the YShearImage method is:
%
%      MagickBooleanType YShearImage(Image *image,const MagickRealType degrees,
%        const unsigned long width,const unsigned long height,
%        const long x_offset,const long y_offset)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: A MagickRealType representing the shearing angle along the Y
%      axis.
%
%    o width, height, x_offset, y_offset: Defines a region of the image
%      to shear.
%
*/
static MagickBooleanType YShearImage(Image *image,const MagickRealType degrees,
  const unsigned long width,const unsigned long height,const long x_offset,
  const long y_offset)
{
#define YShearImageTag  "YShear/Image"

  long
    x;

  MagickBooleanType
    status;

  MagickPixelPacket
    background;

  ViewInfo
    **image_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  GetMagickPixelPacket(image,&background);
  SetMagickPixelPacket(image,&image->background_color,(IndexPacket *) NULL,
    &background);
  if (image->colorspace == CMYKColorspace)
    ConvertRGBToCMYK(&background);
  status=MagickTrue;
  image_view=AcquireCacheViewThreadSet(image);
//  #pragma omp parallel for
  for (x=0; x < (long) width; x++)
  {
    enum {UP, DOWN}
      direction;

    IndexPacket
      *indexes,
      *shear_indexes;

    long
      id,
      step;

    MagickPixelPacket
      pixel,
      source,
      destination;

    MagickRealType
      area,
      displacement;

    register long
      i;

    register PixelPacket
      *p,
      *q;

    if (status == MagickFalse)
      continue;
    id=GetCacheViewThreadId();
    p=GetCacheViewPixels(image_view[id],x_offset+x,0,1,image->rows);
    if (p == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewIndexes(image_view[id]);
    p+=y_offset;
    indexes+=y_offset;
    displacement=degrees*(MagickRealType) (x-width/2.0);
    if (displacement == 0.0)
      continue;
    if (displacement > 0.0)
      direction=DOWN;
    else
      {
        displacement*=(-1.0);
        direction=UP;
      }
    step=(long) floor((double) displacement);
    area=(MagickRealType) (displacement-step);
    step++;
    pixel=background;
    GetMagickPixelPacket(image,&source);
    GetMagickPixelPacket(image,&destination);
    switch (direction)
    {
      case UP:
      {
        /*
          Transfer pixels top-to-bottom.
        */
        if (step > y_offset)
          break;
        q=p-step;
        shear_indexes=indexes-step;
        for (i=0; i < (long) height; i++)
        {
          if ((y_offset+i) < step)
            {
              SetMagickPixelPacket(image,++p,++indexes,&pixel);
              q++;
              shear_indexes++;
              continue;
            }
          SetMagickPixelPacket(image,p,indexes,&source);
          MagickCompositeBlend(&pixel,(MagickRealType) pixel.opacity,&source,
            (MagickRealType) p->opacity,area,&destination);
          SetPixelPacket(image,&destination,q++,shear_indexes++);
          SetMagickPixelPacket(image,p++,indexes++,&pixel);
        }
        MagickCompositeBlend(&pixel,(MagickRealType) pixel.opacity,&background,
          (MagickRealType) background.opacity,area,&destination);
        SetPixelPacket(image,&destination,q++,shear_indexes++);
        for (i=0; i < (step-1); i++)
          SetPixelPacket(image,&background,q++,shear_indexes++);
        break;
      }
      case DOWN:
      {
        /*
          Transfer pixels bottom-to-top.
        */
        p+=height;
        indexes+=height;
        q=p+step;
        shear_indexes=indexes+step;
        for (i=0; i < (long) height; i++)
        {
          p--;
          indexes--;
          q--;
          shear_indexes--;
          if ((unsigned long) (y_offset+height+step-i) >= image->rows)
            continue;
          SetMagickPixelPacket(image,p,indexes,&source);
          MagickCompositeBlend(&pixel,(MagickRealType) pixel.opacity,&source,
            (MagickRealType) p->opacity,area,&destination);
          SetPixelPacket(image,&destination,q,shear_indexes);
          SetMagickPixelPacket(image,p,indexes,&pixel);
        }
        MagickCompositeBlend(&pixel,(MagickRealType) pixel.opacity,&background,
          (MagickRealType) background.opacity,area,&destination);
        SetPixelPacket(image,&destination,--q,--shear_indexes);
        for (i=0; i < (step-1); i++)
          SetPixelPacket(image,&background,--q,--shear_indexes);
        break;
      }
    }
    if (SyncCacheView(image_view[id]) == MagickFalse)
      status=MagickFalse;
    if (SetImageProgress(image,YShearImageTag,x,width) == MagickFalse)
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
%   R o t a t e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RotateImage() creates a new image that is a rotated copy of an existing
%  one.  Positive angles rotate counter-clockwise (right-hand rule), while
%  negative angles rotate clockwise.  Rotated images are usually larger than
%  the originals and have 'empty' triangular corners.  X axis.  Empty
%  triangles left over from shearing the image are filled with the background
%  color defined by member 'background_color' of the image.  RotateImage
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  RotateImage() is based on the paper "A Fast Algorithm for General
%  Raster Rotatation" by Alan W. Paeth.  RotateImage is adapted from a similar
%  method based on the Paeth paper written by Michael Halle of the Spatial
%  Imaging Group, MIT Media Lab.
%
%  The format of the RotateImage method is:
%
%      Image *RotateImage(const Image *image,const double degrees,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o degrees: Specifies the number of degrees to rotate the image.
%
%    o exception: Return any errors or warnings in this structure.
%
%
*/
MagickExport Image *RotateImage(const Image *image,const double degrees,
  ExceptionInfo *exception)
{
  Image
    *integral_image,
    *rotate_image;

  long
    x_offset,
    y_offset;

  MagickRealType
    angle;

  PointInfo
    shear;

  RectangleInfo
    border_info;

  unsigned long
    height,
    rotations,
    width,
    y_width;

  /*
    Adjust rotation angle.
  */
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  angle=degrees;
  while (angle < -45.0)
    angle+=360.0;
  for (rotations=0; angle > 45.0; rotations++)
    angle-=90.0;
  rotations%=4;
  /*
    Calculate shear equations.
  */
  integral_image=IntegralRotateImage(image,rotations,exception);
  if (integral_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  shear.x=(-tan((double) DegreesToRadians(angle)/2.0));
  shear.y=sin((double) DegreesToRadians(angle));
  if ((shear.x == 0.0) && (shear.y == 0.0))
    return(integral_image);
  if (SetImageStorageClass(integral_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&integral_image->exception);
      integral_image=DestroyImage(integral_image);
      return(integral_image);
    }
  if (integral_image->matte == MagickFalse)
    (void) SetImageAlphaChannel(integral_image,ResetAlphaChannel);
  /*
    Compute image size.
  */
  width=image->columns;
  height=image->rows;
  if ((rotations == 1) || (rotations == 3))
    {
      width=image->rows;
      height=image->columns;
    }
  y_width=width+(long) (fabs(shear.x)*height+0.5);
  x_offset=(long) (width+((fabs(shear.y)*height+0.5)-width)/2.0+0.5);
  y_offset=(long) (height+((fabs(shear.y)*y_width+0.5)-height)/2.0+0.5);
  /*
    Surround image with a border.
  */
  integral_image->border_color=integral_image->background_color;
  integral_image->compose=CopyCompositeOp;
  border_info.width=(unsigned long) x_offset;
  border_info.height=(unsigned long) y_offset;
  rotate_image=BorderImage(integral_image,&border_info,exception);
  integral_image=DestroyImage(integral_image);
  if (rotate_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  /*
    Rotate the image.
  */
  (void) XShearImage(rotate_image,shear.x,width,height,x_offset,
    ((long) rotate_image->rows-height)/2);
  (void) YShearImage(rotate_image,shear.y,y_width,height,
    ((long) rotate_image->columns-y_width)/2,y_offset);
  (void) XShearImage(rotate_image,shear.x,y_width,rotate_image->rows,
    ((long) rotate_image->columns-y_width)/2,0);
  CropToFitImage(&rotate_image,shear.x,shear.y,(MagickRealType) width,
    (MagickRealType) height,MagickTrue,exception);
  rotate_image->compose=image->compose;
  rotate_image->page.width=0;
  rotate_image->page.height=0;
  return(rotate_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S h e a r I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ShearImage() creates a new image that is a shear_image copy of an existing
%  one.  Shearing slides one edge of an image along the X or Y axis, creating
%  a parallelogram.  An X direction shear slides an edge along the X axis,
%  while a Y direction shear slides an edge along the Y axis.  The amount of
%  the shear is controlled by a shear angle.  For X direction shears, x_shear
%  is measured relative to the Y axis, and similarly, for Y direction shears
%  y_shear is measured relative to the X axis.  Empty triangles left over from
%  shearing the image are filled with the background color defined by member
%  'background_color' of the image..  ShearImage() allocates the memory
%  necessary for the new Image structure and returns a pointer to the new image.
%
%  ShearImage() is based on the paper "A Fast Algorithm for General Raster
%  Rotatation" by Alan W. Paeth.
%
%  The format of the ShearImage method is:
%
%      Image *ShearImage(const Image *image,const double x_shear,
%        const double y_shear,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image: the image.
%
%    o x_shear, y_shear: Specifies the number of degrees to shear the image.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport Image *ShearImage(const Image *image,const double x_shear,
  const double y_shear,ExceptionInfo *exception)
{
  Image
    *integral_image,
    *shear_image;

  long
    x_offset,
    y_offset;

  PointInfo
    shear;

  RectangleInfo
    border_info;

  unsigned long
    y_width;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if ((x_shear != 0.0) && (fmod(x_shear,90.0) == 0.0))
    ThrowImageException(ImageError,"AngleIsDiscontinuous");
  if ((y_shear != 0.0) && (fmod(y_shear,90.0) == 0.0))
    ThrowImageException(ImageError,"AngleIsDiscontinuous");
  /*
    Initialize shear angle.
  */
  integral_image=CloneImage(image,0,0,MagickTrue,exception);
  if (integral_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  shear.x=(-tan(DegreesToRadians(x_shear)));
  shear.y=tan(DegreesToRadians(y_shear));
  if ((shear.x == 0.0) && (shear.y == 0.0))
    return(integral_image);
  if (SetImageStorageClass(integral_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&integral_image->exception);
      integral_image=DestroyImage(integral_image);
      return(integral_image);
    }
  if (integral_image->matte == MagickFalse)
    (void) SetImageAlphaChannel(integral_image,ResetAlphaChannel);
  /*
    Compute image size.
  */
  y_width=image->columns+(long) (fabs(shear.x)*image->rows+0.5);
  x_offset=(long) (image->columns+((fabs(shear.x)*image->rows)-
    image->columns)/2.0+0.5);
  y_offset=(long) (image->rows+((fabs(shear.y)*y_width+0.5)-image->rows)/2.0+
    0.5);
  /*
    Surround image with border.
  */
  integral_image->border_color=integral_image->background_color;
  integral_image->compose=CopyCompositeOp;
  border_info.width=(unsigned long) x_offset;
  border_info.height=(unsigned long) y_offset;
  shear_image=BorderImage(integral_image,&border_info,exception);
  if (shear_image == (Image *) NULL)
    ThrowImageException(ResourceLimitError,"MemoryAllocationFailed");
  integral_image=DestroyImage(integral_image);
  /*
    Shear the image.
  */
  if (shear_image->matte == MagickFalse)
    (void) SetImageAlphaChannel(shear_image,ResetAlphaChannel);
  (void) XShearImage(shear_image,shear.x,image->columns,image->rows,x_offset,
    ((long) shear_image->rows-image->rows)/2);
  (void) YShearImage(shear_image,shear.y,y_width,image->rows,
    ((long) shear_image->columns-y_width)/2,y_offset);
  CropToFitImage(&shear_image,shear.x,shear.y,(MagickRealType) image->columns,
    (MagickRealType) image->rows,MagickFalse,exception);
  shear_image->compose=image->compose;
  shear_image->page.width=0;
  shear_image->page.height=0;
  return(shear_image);
}
