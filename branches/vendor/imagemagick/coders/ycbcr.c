/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     Y   Y   YYYC  BBBB    YYYC  RRRR                        %
%                      Y Y   C      B   B  C      R   R                       %
%                       Y    C      BBBB   C      RRRR                        %
%                       Y    C      B   B  C      R  R                        %
%                       Y     YYYC  BBBB    YYYC  R   R                       %
%                                                                             %
%                                                                             %
%                     Read/Write RAW YCbCr Image Format.                      %
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
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/colorspace.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/quantum-private.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/statistic.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteYCBCRImage(const ImageInfo *,Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d Y C b C r I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadYCBCRImage() reads an image of raw Y, Cb, and Cr samples and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadYCBCRImage method is:
%
%      Image *ReadYCBCRImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
%
*/
static Image *ReadYCBCRImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *canvas_image,
    *image;

  long
    y;

  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register const PixelPacket
    *p;

  register long
    i,
    x;

  register PixelPacket
    *q;

  ssize_t
    count;

  size_t
    length;

  unsigned char
    *pixels;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  if (image_info->interlace != PartitionInterlace)
    {
      status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
      if (status == MagickFalse)
        {
          image=DestroyImageList(image);
          return((Image *) NULL);
        }
      for (i=0; i < image->offset; i++)
        if (ReadBlobByte(image) == EOF)
          {
            ThrowFileException(exception,CorruptImageError,
              "UnexpectedEndOfFile",image->filename);
            break;
          }
    }
  /*
    Create virtual canvas to support cropping (i.e. image.rgb[100x100+10+20]).
  */
  canvas_image=CloneImage(image,image->extract_info.width,1,MagickTrue,
    exception);
  (void) SetImageVirtualPixelMethod(canvas_image,BlackVirtualPixelMethod);
  quantum_info=AcquireQuantumInfo(image_info,canvas_image);
  pixels=GetQuantumPixels(quantum_info);
  quantum_type=RGBQuantum;
  if (LocaleCompare(image_info->magick,"YCbCrA") == 0)
    {
      quantum_type=RGBAQuantum;
      image->matte=MagickTrue;
    }
  if (image_info->number_scenes != 0)
    while (image->scene < image_info->scene)
    {
      /*
        Skip to next image.
      */
      image->scene++;
      length=ImportQuantumPixels(canvas_image,quantum_info,quantum_type,pixels);
      for (y=0; y < (long) image->rows; y++)
      {
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
      }
    }
  do
  {
    /*
      Read pixels to virtual canvas image then push to image.
    */
    if ((image_info->ping != MagickFalse) && (image_info->number_scenes != 0))
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    image->colorspace=YCbCrColorspace;
    if ((SetImageExtent(image,0,0) == MagickFalse) ||
        (SetImageExtent(canvas_image,0,0) == MagickFalse))
      break;
    switch (image_info->interlace)
    {
      case NoInterlace:
      default:
      {
        /*
          No interlacing:  YCbCrYCbCrYCbCrYCbCrYCbCrYCbCr...
        */
        length=ImportQuantumPixels(canvas_image,quantum_info,quantum_type,
          pixels);
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        for (y=0; y < (long) image->extract_info.height; y++)
        {
          q=GetImagePixels(canvas_image,0,0,canvas_image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,quantum_info,quantum_type,
            pixels);
          if (SyncImagePixels(canvas_image) == MagickFalse)
            break;
          count=ReadBlob(image,length,pixels);
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (long) image->rows))
            {
              p=AcquireImagePixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=SetImagePixels(image,0,y-image->extract_info.y,image->columns,
                1);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                q->red=p->red;
                q->green=p->green;
                q->blue=p->blue;
                if (image->matte != MagickFalse)
                  q->opacity=p->opacity;
                p++;
                q++;
              }
              if (SyncImagePixels(image) == MagickFalse)
                break;
            }
          if (image->previous == (Image *) NULL)
            if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
                (QuantumTick(y,image->rows) != MagickFalse))
              {
                status=image->progress_monitor(LoadImageTag,y,image->rows,
                  image->client_data);
                if (status == MagickFalse)
                  break;
              }
        }
        break;
      }
      case LineInterlace:
      {
        static QuantumType
          quantum_types[4] =
          {
            RedQuantum,
            GreenQuantum,
            BlueQuantum,
            OpacityQuantum
          };

        /*
          Line interlacing:  YYY...CbCbCb...CrCrCr...YYY...CbCbCb...CrCrCr...
        */
        length=ImportQuantumPixels(canvas_image,quantum_info,RedQuantum,pixels);
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        for (y=0; y < (long) image->extract_info.height; y++)
        {
          for (i=0; i < (image->matte != MagickFalse ? 4 : 3); i++)
          {
            quantum_type=quantum_types[i];
            q=GetImagePixels(canvas_image,0,0,canvas_image->columns,1);
            if (q == (PixelPacket *) NULL)
              break;
            length=ImportQuantumPixels(canvas_image,quantum_info,quantum_type,
              pixels);
            if (SyncImagePixels(canvas_image) == MagickFalse)
              break;
            count=ReadBlob(image,length,pixels);
            if (((y-image->extract_info.y) >= 0) && 
                ((y-image->extract_info.y) < (long) image->rows))
              {
                p=AcquireImagePixels(canvas_image,canvas_image->extract_info.x,
                  0,canvas_image->columns,1,exception);
                q=SetImagePixels(image,0,y-image->extract_info.y,image->columns,
                  1);
                if ((p == (const PixelPacket *) NULL) ||
                    (q == (PixelPacket *) NULL))
                  break;
                for (x=0; x < (long) image->columns; x++)
                {
                  switch (quantum_type)
                  {
                    case RedQuantum: q->red=p->red; break;
                    case GreenQuantum: q->green=p->green; break;
                    case BlueQuantum: q->blue=p->blue; break;
                    case OpacityQuantum: q->opacity=p->opacity; break;
                    default: break;
                  }
                  p++;
                  q++;
                }
                if (SyncImagePixels(image) == MagickFalse)
                  break;
              }
            }
          if (image->previous == (Image *) NULL)
            if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
                (QuantumTick(y,image->rows) != MagickFalse))
              {
                status=image->progress_monitor(LoadImageTag,y,image->rows,
                  image->client_data);
                if (status == MagickFalse)
                  break;
              }
        }
        break;
      }
      case PlaneInterlace:
      case PartitionInterlace:
      {
        /*
          Plane interlacing:  YYYYYY...CbCbCbCbCbCb...CrCrCrCrCrCr...
        */
        if (image_info->interlace == PartitionInterlace)
          {
            AppendImageFormat("Y",image->filename);
            status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
            if (status == MagickFalse)
              {
                canvas_image=DestroyImageList(canvas_image);
                image=DestroyImageList(image);
                return((Image *) NULL);
              }
             for (i=0; i < image->offset; i++)
               if (ReadBlobByte(image) == EOF)
                 {
                   ThrowFileException(exception,CorruptImageError,
                     "UnexpectedEndOfFile",image->filename);
                   break;
                 }
          }
        length=ImportQuantumPixels(canvas_image,quantum_info,RedQuantum,pixels);
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        for (y=0; y < (long) image->extract_info.height; y++)
        {
          q=GetImagePixels(canvas_image,0,0,canvas_image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,quantum_info,RedQuantum,
            pixels);
          if (SyncImagePixels(canvas_image) == MagickFalse)
            break;
          count=ReadBlob(image,length,pixels);
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (long) image->rows))
            {
              p=AcquireImagePixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=SetImagePixels(image,0,y-image->extract_info.y,image->columns,
                1);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                q->red=p->red;
                p++;
                q++;
              }
              if (SyncImagePixels(image) == MagickFalse)
                break;
            }
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            status=image->progress_monitor(LoadImageTag,100,400,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
        if (image_info->interlace == PartitionInterlace)
          {
            (void) CloseBlob(image);
            AppendImageFormat("Cb",image->filename);
            status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
            if (status == MagickFalse)
              {
                canvas_image=DestroyImageList(canvas_image);
                image=DestroyImageList(image);
                return((Image *) NULL);
              }
          }
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        for (y=0; y < (long) image->extract_info.height; y++)
        {
          q=GetImagePixels(canvas_image,0,0,canvas_image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,quantum_info,GreenQuantum,
            pixels);
          if (SyncImagePixels(canvas_image) == MagickFalse)
            break;
          count=ReadBlob(image,length,pixels);
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (long) image->rows))
            {
              p=AcquireImagePixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=SetImagePixels(image,0,y-image->extract_info.y,image->columns,
                1);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                q->green=p->green;
                p++;
                q++;
              }
              if (SyncImagePixels(image) == MagickFalse)
                break;
           }
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            status=image->progress_monitor(LoadImageTag,200,400,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
        if (image_info->interlace == PartitionInterlace)
          {
            (void) CloseBlob(image);
            AppendImageFormat("Cr",image->filename);
            status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
            if (status == MagickFalse)
              {
                canvas_image=DestroyImageList(canvas_image);
                image=DestroyImageList(image);
                return((Image *) NULL);
              }
          }
        count=ReadBlob(image,length,pixels);
        if (count != (ssize_t) length)
          break;
        for (y=0; y < (long) image->extract_info.height; y++)
        {
          q=GetImagePixels(canvas_image,0,0,canvas_image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          length=ImportQuantumPixels(canvas_image,quantum_info,BlueQuantum,
            pixels);
          if (SyncImagePixels(canvas_image) == MagickFalse)
            break;
          count=ReadBlob(image,length,pixels);
          if (((y-image->extract_info.y) >= 0) && 
              ((y-image->extract_info.y) < (long) image->rows))
            {
              p=AcquireImagePixels(canvas_image,canvas_image->extract_info.x,0,
                canvas_image->columns,1,exception);
              q=SetImagePixels(image,0,y-image->extract_info.y,image->columns,
                1);
              if ((p == (const PixelPacket *) NULL) ||
                  (q == (PixelPacket *) NULL))
                break;
              for (x=0; x < (long) image->columns; x++)
              {
                q->blue=p->blue;
                p++;
                q++;
              }
              if (SyncImagePixels(image) == MagickFalse)
                break;
            }
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            status=image->progress_monitor(LoadImageTag,300,400,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
        if (image->matte != MagickFalse)
          {
            if (image_info->interlace == PartitionInterlace)
              {
                (void) CloseBlob(image);
                AppendImageFormat("A",image->filename);
                status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
                if (status == MagickFalse)
                  {
                    canvas_image=DestroyImageList(canvas_image);
                    image=DestroyImageList(image);
                    return((Image *) NULL);
                  }
              }
            count=ReadBlob(image,length,pixels);
            if (count != (ssize_t) length)
              break;
            for (y=0; y < (long) image->extract_info.height; y++)
            {
              q=GetImagePixels(canvas_image,0,0,canvas_image->columns,1);
              if (q == (PixelPacket *) NULL)
                break;
              length=ImportQuantumPixels(canvas_image,quantum_info,AlphaQuantum,
                pixels);
              if (SyncImagePixels(canvas_image) == MagickFalse)
                break;
              count=ReadBlob(image,length,pixels);
              if (((y-image->extract_info.y) >= 0) && 
                  ((y-image->extract_info.y) < (long) image->rows))
                {
                  p=AcquireImagePixels(canvas_image,
                    canvas_image->extract_info.x,0,canvas_image->columns,1,
                    exception);
                  q=SetImagePixels(image,0,y-image->extract_info.y,
                    image->columns,1);
                  if ((p == (const PixelPacket *) NULL) ||
                      (q == (PixelPacket *) NULL))
                    break;
                  for (x=0; x < (long) image->columns; x++)
                  {
                    q->opacity=p->opacity;
                    p++;
                    q++;
                  }
                  if (SyncImagePixels(image) == MagickFalse)
                    break;
                }
            }
          }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            status=image->progress_monitor(LoadImageTag,400,400,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
        break;
      }
    }
    SetQuantumImageType(image,quantum_type);
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (count == (ssize_t) length)
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            status=image->progress_monitor(LoadImagesTag,TellBlob(image),
              GetBlobSize(image),image->client_data);
            if (status == MagickFalse)
              break;
          }
      }
  } while (count == (ssize_t) length);
  InheritException(exception,&image->exception);
  quantum_info=DestroyQuantumInfo(quantum_info);
  canvas_image=DestroyImage(canvas_image);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r Y C b C r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterYCBCRImage() adds attributes for the YCbCr image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterYCBCRImage method is:
%
%      unsigned long RegisterYCBCRImage(void)
%
*/
ModuleExport unsigned long RegisterYCBCRImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("YCbCr");
  entry->decoder=(DecodeImageHandler *) ReadYCBCRImage;
  entry->encoder=(EncodeImageHandler *) WriteYCBCRImage;
  entry->raw=MagickTrue;
  entry->description=ConstantString("Raw Y, Cb, and Cr samples");
  entry->module=ConstantString("YCbCr");
  (void) RegisterMagickInfo(entry);
  entry=SetMagickInfo("YCbCrA");
  entry->decoder=(DecodeImageHandler *) ReadYCBCRImage;
  entry->encoder=(EncodeImageHandler *) WriteYCBCRImage;
  entry->raw=MagickTrue;
  entry->description=ConstantString("Raw Y, Cb, Cr, and opacity samples");
  entry->module=ConstantString("YCbCr");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r Y C b C r I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterYCBCRImage() removes format registrations made by the
%  YCbCr module from the list of supported formats.
%
%  The format of the UnregisterYCBCRImage method is:
%
%      UnregisterYCBCRImage(void)
%
*/
ModuleExport void UnregisterYCBCRImage(void)
{
  (void) UnregisterMagickInfo("YCbCr");
  (void) UnregisterMagickInfo("YCbCrA");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e Y C b C r I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteYCBCRImage() writes an image to a file in the raw Y, Cb, and Cr
%  image format.
%
%  The format of the WriteYCBCRImage method is:
%
%      MagickBooleanType WriteYCBCRImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%
*/
static MagickBooleanType WriteYCBCRImage(const ImageInfo *image_info,
  Image *image)
{
  long
    y;

  MagickBooleanType
    status;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  QuantumType
    quantum_type;

  register const PixelPacket
    *p;

  ssize_t
    count;

  size_t
    length;

  unsigned char
    *pixels;

  /*
    Allocate memory for pixels.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  if (image_info->interlace != PartitionInterlace)
    {
      /*
        Open output image file.
      */
      status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
      if (status == MagickFalse)
        return(status);
    }
  quantum_type=RGBQuantum;
  if (LocaleCompare(image_info->magick,"YCbCrA") == 0)
    {
      quantum_type=RGBAQuantum;
      image->matte=MagickTrue;
    }
  scene=0;
  do
  {
    /*
      Convert MIFF to YCbCr raster pixels.
    */
    if (image->colorspace != RGBColorspace)
      (void) SetImageColorspace(image,YCbCrColorspace);
    if ((LocaleCompare(image_info->magick,"YCbCrA") == 0) &&
        (image->matte == MagickFalse))
      (void) SetImageAlphaChannel(image,ResetAlphaChannel);
    quantum_info=AcquireQuantumInfo(image_info,image);
    pixels=GetQuantumPixels(quantum_info);
    switch (image_info->interlace)
    {
      case NoInterlace:
      default:
      {
        /*
          No interlacing:  YCbCrYCbCrYCbCrYCbCrYCbCrYCbCr...
        */
        for (y=0; y < (long) image->rows; y++)
        {
          p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          length=ExportQuantumPixels(image,quantum_info,quantum_type,pixels);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          if (image->previous == (Image *) NULL)
            if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
                (QuantumTick(y,image->rows) != MagickFalse))
              {
                status=image->progress_monitor(SaveImageTag,y,image->rows,
                  image->client_data);
                if (status == MagickFalse)
                  break;
              }
        }
        break;
      }
      case LineInterlace:
      {
        /*
          Line interlacing:  YYY...CbCbCb...CrCrCr...YYY...CbCbCb...CrCrCr...
        */
        for (y=0; y < (long) image->rows; y++)
        {
          p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          length=ExportQuantumPixels(image,quantum_info,RedQuantum,pixels);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          length=ExportQuantumPixels(image,quantum_info,GreenQuantum,pixels);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          length=ExportQuantumPixels(image,quantum_info,BlueQuantum,pixels);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
          if (quantum_type == RGBAQuantum)
            {
              length=ExportQuantumPixels(image,quantum_info,AlphaQuantum,
                pixels);
              count=WriteBlob(image,length,pixels);
              if (count != (ssize_t) length)
                break;
            }
          if ((image->progress_monitor != (MagickProgressMonitor) NULL) &&
              (QuantumTick(y,image->rows) != MagickFalse))
            {
              status=image->progress_monitor(SaveImageTag,y,image->rows,
                image->client_data);
              if (status == MagickFalse)
                break;
            }
        }
        break;
      }
      case PlaneInterlace:
      case PartitionInterlace:
      {
        /*
          Plane interlacing:  YYYYYY...CbCbCbCbCbCb...CrCrCrCrCrCr...
        */
        if (image_info->interlace == PartitionInterlace)
          {
            AppendImageFormat("Y",image->filename);
            status=OpenBlob(image_info,image,WriteBinaryBlobMode,
              &image->exception);
            if (status == MagickFalse)
              return(status);
          }
        for (y=0; y < (long) image->rows; y++)
        {
          p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          length=ExportQuantumPixels(image,quantum_info,RedQuantum,pixels);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            status=image->progress_monitor(LoadImageTag,100,400,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
        if (image_info->interlace == PartitionInterlace)
          {
            (void) CloseBlob(image);
            AppendImageFormat("Cb",image->filename);
            status=OpenBlob(image_info,image,WriteBinaryBlobMode,
              &image->exception);
            if (status == MagickFalse)
              return(status);
          }
        for (y=0; y < (long) image->rows; y++)
        {
          p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          length=ExportQuantumPixels(image,quantum_info,GreenQuantum,pixels);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            status=image->progress_monitor(LoadImageTag,200,400,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
        if (image_info->interlace == PartitionInterlace)
          {
            (void) CloseBlob(image);
            AppendImageFormat("Cr",image->filename);
            status=OpenBlob(image_info,image,WriteBinaryBlobMode,
              &image->exception);
            if (status == MagickFalse)
              return(status);
          }
        for (y=0; y < (long) image->rows; y++)
        {
          p=AcquireImagePixels(image,0,y,image->columns,1,&image->exception);
          if (p == (const PixelPacket *) NULL)
            break;
          length=ExportQuantumPixels(image,quantum_info,BlueQuantum,pixels);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            break;
        }
        if (quantum_type == RGBAQuantum)
          {
            if (image->progress_monitor != (MagickProgressMonitor) NULL)
              {
                status=image->progress_monitor(LoadImageTag,300,400,
                  image->client_data);
                if (status == MagickFalse)
                  break;
              }
            if (image_info->interlace == PartitionInterlace)
              {
                (void) CloseBlob(image);
                AppendImageFormat("A",image->filename);
                status=OpenBlob(image_info,image,WriteBinaryBlobMode,
                  &image->exception);
                if (status == MagickFalse)
                  return(status);
              }
            for (y=0; y < (long) image->rows; y++)
            {
              p=AcquireImagePixels(image,0,y,image->columns,1,
                &image->exception);
              if (p == (const PixelPacket *) NULL)
                break;
              length=ExportQuantumPixels(image,quantum_info,AlphaQuantum,
                pixels);
              count=WriteBlob(image,length,pixels);
              if (count != (ssize_t) length)
              break;
            }
          }
        if (image_info->interlace == PartitionInterlace)
          (void) CopyMagickString(image->filename,image_info->filename,
            MaxTextExtent);
        if (image->progress_monitor != (MagickProgressMonitor) NULL)
          {
            status=image->progress_monitor(LoadImageTag,400,400,
              image->client_data);
            if (status == MagickFalse)
              break;
          }
        break;
      }
    }
    quantum_info=DestroyQuantumInfo(quantum_info);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    if (image->progress_monitor != (MagickProgressMonitor) NULL)
      {
        status=image->progress_monitor(SaveImagesTag,scene,
          GetImageListLength(image),image->client_data);
        if (status == MagickFalse)
          break;
      }
    scene++;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}
