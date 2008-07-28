/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      CCCC   AAA    CCCC  H   H  EEEEE                       %
%                     C      A   A  C      H   H  E                           %
%                     C      AAAAA  C      HHHHH  EEE                         %
%                     C      A   A  C      H   H  E                           %
%                      CCCC  A   A   CCCC  H   H  EEEEE                       %
%                                                                             %
%                        V   V  IIIII  EEEEE  W   W                           %
%                        V   V    I    E      W   W                           %
%                        V   V    I    EEE    W W W                           %
%                         V V     I    E      WW WW                           %
%                          V    IIIII  EEEEE  W   W                           %
%                                                                             %
%                                                                             %
%                       ImageMagick Cache View Methods                        %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               February 2000                                 %
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
#include "magick/cache.h"
#include "magick/cache-private.h"
#include "magick/cache-view.h"
#include "magick/memory_.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/string_.h"

/*
  Typedef declarations.
*/
struct _ViewInfo
{
  unsigned long
    id;

  Image
    *image;

  VirtualPixelMethod
    virtual_pixel_method;

  MagickBooleanType
    debug;

  unsigned long
    signature;
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C a c h e V i e w                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCacheView() acquires a view into the pixel cache, using the
%  VirtualPixelMethod that is defined within the given image itself.
%
%  The format of the AcquireCacheView method is:
%
%      ViewInfo *AcquireCacheView(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport ViewInfo *AcquireCacheView(const Image *image)
{
  ViewInfo
    *cache_view;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_view=(ViewInfo *) AcquireMagickMemory(sizeof(*cache_view));
  if (cache_view == (ViewInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(cache_view,0,sizeof(*cache_view));
  cache_view->image=ReferenceImage((Image *) image);
  cache_view->id=GetNexus(cache_view->image->cache);
  cache_view->virtual_pixel_method=GetImageVirtualPixelMethod(image);
  cache_view->debug=IsEventLogging();
  cache_view->signature=MagickSignature;
  return(cache_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C a c h e V i e w I n d e x e s                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCacheViewIndexes() returns the indexes associated with the specified
%  view.
%
%  The format of the AcquireCacheViewIndexes method is:
%
%      const IndexPacket *AcquireCacheViewIndexes(const ViewInfo *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport const IndexPacket *AcquireCacheViewIndexes(
  const ViewInfo *cache_view)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(AcquireNexusIndexes(cache_view->image->cache,cache_view->id));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C a c h e V i e w P i x e l s                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCacheViewPixels() gets pixels from the in-memory or disk pixel cache
%  as defined by the geometry parameters.   A pointer to the pixels is returned
%  if the pixels are transferred, otherwise a NULL is returned.
%
%  The format of the AcquireCacheViewPixels method is:
%
%      const PixelPacket *AcquireCacheViewPixels(const ViewInfo *cache_view,
%        const long x,const long y,const unsigned long columns,
%        const unsigned long rows,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport const PixelPacket *AcquireCacheViewPixels(
  const ViewInfo *cache_view,const long x,const long y,
  const unsigned long columns,const unsigned long rows,ExceptionInfo *exception)
{
  const PixelPacket
    *pixels;

  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  pixels=AcquireCacheNexus(cache_view->image,cache_view->virtual_pixel_method,
    x,y,columns,rows,cache_view->id,exception);
  return(pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e C a c h e V i e w T h r e a d S e t                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCacheViewThreadSet() acquires a set of views, one for each possible
%  thread.
%
%  The format of the AcquireCacheViewThreadSet method is:
%
%      ViewInfo **AcquireCacheViewThreadSet(const Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
*/
MagickExport ViewInfo **AcquireCacheViewThreadSet(const Image *image)
{
  register long
    i;

  ViewInfo
    **cache_views;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  cache_views=(ViewInfo **) AcquireQuantumMemory(GetCacheViewMaximumThreads(),
    sizeof(**cache_views));
  if (cache_views == (ViewInfo **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  for (i=0; i < (long) GetCacheViewMaximumThreads(); i++)
    cache_views[i]=AcquireCacheView(image);
  return(cache_views);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e O n e C a c h e V i e w P i x e l                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCacheViewPixels() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.  If
%  you plan to modify the pixel, use GetOneCacheViewPixel() instead.
%
%  The format of the AcquireOneCacheViewPixel method is:
%
%      PixelPacket AcquireOneCacheViewPixel(const ViewInfo *cache_view,
%        const long x,const long y,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y:  These values define the offset of the pixel.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport PixelPacket AcquireOneCacheViewPixel(const ViewInfo *cache_view,
  const long x,const long y,ExceptionInfo *exception)
{
  const PixelPacket
    *pixels;

  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  pixels=AcquireCacheNexus(cache_view->image,cache_view->virtual_pixel_method,
    x,y,1,1,cache_view->id,exception);
  if (pixels == (const PixelPacket *) NULL)
    return(cache_view->image->background_color);
  return(*pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A c q u i r e O n e C a c h e V i e w V i r t u a l P i x e l             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AcquireCacheViewPixels() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.  If
%  you plan to modify the pixel, use GetOneCacheViewPixel() instead.
%
%  The format of the AcquireOneCacheViewPixel method is:
%
%      PixelPacket AcquireOneCacheViewVirtualPixel(const ViewInfo *cache_view,
%        const VirtualPixelMethod virtual_pixel_method,const long x,
%        const long y,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o virtual_pixel_method: the virtual pixel method.
%
%    o x,y:  These values define the offset of the pixel.
%
%    o exception: Return any errors or warnings in this structure.
%
*/
MagickExport PixelPacket AcquireOneCacheViewVirtualPixel(
  const ViewInfo *cache_view,const VirtualPixelMethod virtual_pixel_method,
  const long x,const long y,ExceptionInfo *exception)
{
  const PixelPacket
    *pixels;

  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  pixels=AcquireCacheNexus(cache_view->image,virtual_pixel_method,x,y,1,1,
    cache_view->id,exception);
  if (pixels == (const PixelPacket *) NULL)
    return(cache_view->image->background_color);
  return(*pixels);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o n e C a c h e V i e w                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CloneCacheView()  makes an exact copy of the specified cache view.
%
%  The format of the CloneCacheView method is:
%
%      ViewInfo *CloneCacheView(const ViewInfo *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport ViewInfo *CloneCacheView(const ViewInfo *cache_view)
{
  ViewInfo
    *clone_view;

  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  clone_view=(ViewInfo *) AcquireMagickMemory(sizeof(*clone_view));
  if (clone_view == (ViewInfo *) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(clone_view,0,sizeof(*clone_view));
  clone_view->image=ReferenceImage(cache_view->image);
  clone_view->id=GetNexus(clone_view->image->cache);
  clone_view->virtual_pixel_method=cache_view->virtual_pixel_method;
  clone_view->debug=cache_view->debug;
  clone_view->signature=MagickSignature;
  return(clone_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C a c h e V i e w                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCacheView() destroy the specified view returned by a previous call to
%  AcquireCacheView().
%
%  The format of the DestroyCacheView method is:
%
%      ViewInfo *DestroyCacheView(ViewInfo *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport ViewInfo *DestroyCacheView(ViewInfo *cache_view)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  if (cache_view->id != 0)
    DestroyCacheNexus(cache_view->image->cache,cache_view->id);
  cache_view->image=DestroyImage(cache_view->image);
  cache_view->signature=(~MagickSignature);
  cache_view=(ViewInfo *) RelinquishMagickMemory(cache_view);
  return(cache_view);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C a c h e V i e w T h r e a d S e t                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DestroyCacheViewThreadSet() destroys a set of cache views previously
%  acquired from the AcquireCacheViewThreadSet() method.
%
%  The format of the DestoryCacheViewSet method is:
%
%      ViewInfo **DestroyCacheViewThreadSet(ViewInfo **cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache views.
%
*/
MagickExport ViewInfo **DestroyCacheViewThreadSet(ViewInfo **cache_views)
{
  register long
    i;

  assert(cache_views != (ViewInfo **) NULL);
  for (i=0; i < (long) GetCacheViewMaximumThreads(); i++)
    cache_views[i]=DestroyCacheView(cache_views[i]);
  return((ViewInfo **) RelinquishMagickMemory(cache_views));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w C o l o r s p a c e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewColorspace() returns the image colorspace associated with the
%  specified view.
%
%  The format of the GetCacheViewColorspace method is:
%
%      ColorspaceType GetCacheViewColorspace(const ViewInfo *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport ColorspaceType GetCacheViewColorspace(const ViewInfo *cache_view)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(cache_view->image->colorspace);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w E x c e p t i o n                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewException() returns the image exception associated with the
%  specified view.
%
%  The format of the GetCacheViewException method is:
%
%      ExceptionInfo GetCacheViewException(const ViewInfo *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport ExceptionInfo *GetCacheViewException(const ViewInfo *cache_view)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(&cache_view->image->exception);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w I n d e x e s                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewIndexes() returns the indexes associated with the specified
%  view.
%
%  The format of the GetCacheViewIndexes method is:
%
%      IndexPacket *GetCacheViewIndexes(const ViewInfo *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport IndexPacket *GetCacheViewIndexes(const ViewInfo *cache_view)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(GetNexusIndexes(cache_view->image->cache,cache_view->id));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewPixels() gets pixels from the in-memory or disk pixel cache as
%  defined by the geometry parameters.   A pointer to the pixels is returned if
%  the pixels are transferred, otherwise a NULL is returned.
%
%  The format of the GetCacheViewPixels method is:
%
%      PixelPacket *GetCacheViewPixels(ViewInfo *cache_view,const long x,
%        const long y,const unsigned long columns,const unsigned long rows)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/
MagickExport PixelPacket *GetCacheViewPixels(ViewInfo *cache_view,const long x,
  const long y,const unsigned long columns,const unsigned long rows)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(GetCacheNexus(cache_view->image,x,y,columns,rows,cache_view->id));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e V i e w S t o r a g e C l a s s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetCacheViewStorageClass() returns the image storage class  associated with
%  the specified view.
%
%  The format of the GetCacheViewStorageClass method is:
%
%      ClassType GetCacheViewStorageClass(const ViewInfo *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport ClassType GetCacheViewStorageClass(const ViewInfo *cache_view)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(cache_view->image->storage_class);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t O n e C a c h e V i e w P i x e l                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  GetOneCacheViewPixel() returns a single pixel at the specified (x,y)
%  location.  The image background color is returned if an error occurs.
%
%  The format of the GetOneCacheViewPixel method is:
%
%      PixelPacket GetOneCacheViewPixel(const ViewInfo *cache_view,
%        const long x,const long y)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y:  These values define the offset of the pixel.
%
*/
MagickExport PixelPacket GetOneCacheViewPixel(const ViewInfo *cache_view,
  const long x,const long y)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(GetOnePixel(cache_view->image,x,y));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C a c h e V i e w P i x e l s                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCacheViewPixels() gets pixels from the in-memory or disk pixel cache as
%  defined by the geometry parameters.   A pointer to the pixels is returned
%  if the pixels are transferred, otherwise a NULL is returned.
%
%  The format of the SetCacheView method is:
%
%      PixelPacket *SetCacheViewPixels(ViewInfo *cache_view,const long x,
%        const long y,const unsigned long columns,const unsigned long rows)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o x,y,columns,rows:  These values define the perimeter of a region of
%      pixels.
%
*/

MagickExport PixelPacket *SetCacheView(ViewInfo *cache_view,const long x,
  const long y,const unsigned long columns,const unsigned long rows)
{
  return(SetCacheViewPixels(cache_view,x,y,columns,rows));
}

MagickExport PixelPacket *SetCacheViewPixels(ViewInfo *cache_view,const long x,
  const long y,const unsigned long columns,const unsigned long rows)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(SetCacheNexus(cache_view->image,x,y,columns,rows,cache_view->id));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C a c h e V i e w S t o r a g e C l a s s                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCacheViewStorageClass() sets the image storage class associated with
%  the specified view.
%
%  The format of the SetCacheViewStorageClass method is:
%
%      MagickBooleanType SetCacheViewStorageClass(ViewInfo *cache_view,
%        const ClassType storage_class)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o storage_class: the image storage class: PseudoClass or DirectClass.
%
*/
MagickExport MagickBooleanType SetCacheViewStorageClass(ViewInfo *cache_view,
  const ClassType storage_class)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(SetImageStorageClass(cache_view->image,storage_class));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C a c h e V i e w V i r t u a l P i x e l M e t h o d               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SetCacheViewVirtualPixelMethod() sets the virtual pixel method associated
%  with the specified cache view.
%
%  The format of the SetCacheViewVirtualPixelMethod method is:
%
%      MagickBooleanType SetCacheViewVirtualPixelMethod(ViewInfo *cache_view,
%        const VirtualPixelMethod virtual_pixel_method)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
%    o virtual_pixel_method: the virtual pixel method.
%
*/
MagickExport MagickBooleanType SetCacheViewVirtualPixelMethod(
  ViewInfo *cache_view,const VirtualPixelMethod virtual_pixel_method)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  cache_view->virtual_pixel_method=virtual_pixel_method;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S y n c C a c h e V i e w                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SyncCacheView() saves the cache_view pixels to the in-memory or disk cache.
%  The method returns MagickTrue if the pixel region is synced, otherwise
%  MagickFalse.
%
%  The format of the SyncCacheView method is:
%
%      MagickBooleanType SyncCacheView(ViewInfo *cache_view)
%
%  A description of each parameter follows:
%
%    o cache_view: the cache view.
%
*/
MagickExport MagickBooleanType SyncCacheView(ViewInfo *cache_view)
{
  assert(cache_view != (ViewInfo *) NULL);
  assert(cache_view->signature == MagickSignature);
  assert(cache_view->image != (Image *) NULL);
  if (cache_view->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      cache_view->image->filename);
  return(SyncCacheNexus(cache_view->image,cache_view->id));
}
