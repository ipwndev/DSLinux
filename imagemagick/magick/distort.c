/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               DDDD   IIIII  SSSSS  TTTTT   OOO   RRRR   TTTTT               %
%               D   D    I    SS       T    O   O  R   R    T                 %
%               D   D    I     SSS     T    O   O  RRRR     T                 %
%               D   D    I       SS    T    O   O  R R      T                 %
%               DDDD   IIIII  SSSSS    T     OOO   R  R     T                 %
%                                                                             %
%                                                                             %
%                     ImageMagick Image Distortion Methods.                   %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                              Anthony Thyssen                                %
%                                 June 2007                                   %
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
#include "magick/artifact.h"
#include "magick/cache-view.h"
#include "magick/colorspace-private.h"
#include "magick/composite-private.h"
#include "magick/distort.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/gem.h"
#include "magick/hashmap.h"
#include "magick/image.h"
#include "magick/list.h"
#include "magick/matrix.h"
#include "magick/memory_.h"
#include "magick/monitor-private.h"
#include "magick/pixel.h"
#include "magick/pixel-private.h"
#include "magick/resample.h"
#include "magick/registry.h"
#include "magick/semaphore.h"
#include "magick/splay-tree.h"
#include "magick/string_.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D i s t o r t I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DistortImage() distorts an image using various distortion methods, by
%  mapping color lookups of the source image to a new destination image
%  usally of the same size as the source image, unless 'bestfit' is set to
%  true.
%
%  If 'bestfit' is enabled, and distortion allows it, the destination image is
%  adjusted to ensure the whole source 'image' will just fit within the final
%  destination image, which will be sized and offset accordingly.  Also in
%  many cases the virtual offset of the source image will be taken into
%  account in the mapping.
%
%
%  The format of the DistortImage() method is:
%
%      Image *DistortImage(const Image *image,const DistortImageMethod method,
%        const unsigned long number_arguments,const double *arguments,
%        MagickBooleanType bestfit, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image to be distorted.
%
%    o method: the method of image distortion.
%
%    o number_arguments: the number of arguments given.
%
%    o arguments: the arguments for this distortion method.
%
%    o bestfit: Attempt to 'bestfit' the size of the resulting image.
%         This also forces the resulting image to be a 'layered' virtual
%         canvas image.  Can be overridden using 'distort:viewport' setting.
%
%    o exception: Return any errors or warnings in this structure
%
% Specific Distortion notes...
%
%  ArcDistortion will always ignore source image offset, and always 'bestfit'
%  the destination image with the top left corner offset relative to the polar
%  mapping center.
%
%  Bilinear has no simple inverse mapping so will not allow 'bestfit' style
%  of image distortion.
%
%  Affine, Perspective, and Bilinear, will do least squares fitting of the
%  distrotion when more than the minimum number of control point pairs are
%  provided.
%
%  Perspective, and Bilinear, will fall back to a Affine distortion when less
%  that 4 control point pairs are provided. While Affine distortions will let
%  you use any number of control point pairs, that is Zero pairs is a No-Op
%  (viewport only) distrotion, one pair is a translation and two pairs of
%  control points will do a scale-rotate-translate, without any shearing.
%
*/

static ResampleFilter **DestroyResampleFilterThreadSet(ResampleFilter **filter)
{
  register long
    i;

  assert(filter != (ResampleFilter **) NULL);
  for (i=0; i < (long) GetCacheViewMaximumThreads(); i++)
    if (filter[i] != (ResampleFilter *) NULL)
      filter[i]=DestroyResampleFilter(filter[i]);
  return((ResampleFilter **) RelinquishMagickMemory(filter));
}

static ResampleFilter **AcquireResampleFilterThreadSet(const Image *image,
  ExceptionInfo *exception)
{
  register long
    i;

  ResampleFilter
    **filter;

  filter=(ResampleFilter **) AcquireQuantumMemory(GetCacheViewMaximumThreads(),
    sizeof(*filter));
  if (filter == (ResampleFilter **) NULL)
    return((ResampleFilter **) NULL);
  (void) ResetMagickMemory(filter,0,GetCacheViewMaximumThreads()*
    sizeof(*filter));
  for (i=0; i < (long) GetCacheViewMaximumThreads(); i++)
  {
    filter[i]=AcquireResampleFilter(image,exception);
    if (filter[i] == (ResampleFilter *) NULL)
      return(DestroyResampleFilterThreadSet(filter));
  }
  return(filter);
}

static void InvertAffineCoefficients(const double *coefficients,double *inverse)
{
  double determinant;

  determinant=1.0/(coefficients[0]*coefficients[3]-coefficients[1]*coefficients[2]);
  inverse[0]=determinant*coefficients[3];
  inverse[1]=determinant*(-coefficients[1]);
  inverse[2]=determinant*(-coefficients[2]);
  inverse[3]=determinant*coefficients[0];
  inverse[4]=(-coefficients[4])*inverse[0]-coefficients[5]*inverse[2];
  inverse[5]=(-coefficients[4])*inverse[1]-coefficients[5]*inverse[3];
}


static void InvertPerspectiveCoefficients(const double *coefficients,
  double *inverse)
{
  double determinant;
  /* From "Digital Image Warping" by George Wolberg, page 53 */

  determinant=1.0/(coefficients[0]*coefficients[4]-coefficients[3]*coefficients[1]);
  inverse[0]=determinant*(coefficients[4]-coefficients[7]*coefficients[5]);
  inverse[1]=determinant*(coefficients[7]*coefficients[2]-coefficients[1]);
  inverse[2]=determinant*(coefficients[1]*coefficients[5]-coefficients[4]*coefficients[2]);
  inverse[3]=determinant*(coefficients[6]*coefficients[5]-coefficients[3]);
  inverse[4]=determinant*(coefficients[0]-coefficients[6]*coefficients[2]);
  inverse[5]=determinant*(coefficients[3]*coefficients[2]-coefficients[0]*coefficients[5]);
  inverse[6]=determinant*(coefficients[3]*coefficients[7]-coefficients[6]*coefficients[4]);
  inverse[7]=determinant*(coefficients[6]*coefficients[1]-coefficients[0]*coefficients[7]);
}

#if 0  
   For some reason compiler errors on % in this!
static inline double MagickFraction(double x)
{
  /* value mod 1 in a  -.5 to .5 range */
  return ((((x%1.0)+1.5)%1.0)-0.5);
}
#endif
static inline double MagickRound(double x)
{
  /* round the fraction to nearest integer */
  if (x >= 0.0)
    return((double) ((long) (x+0.5)));
  return((double) ((long) (x-0.5)));
#if 0
  return(x - MagickFraction(x));
#endif
}

static unsigned long poly_number_terms(double order)
{
  /* Return the number of terms for a 2d polynomial
     Order must either be an integer, or 1.5 to produce
     the 2 dimentional polyminal function...
        affine     1 (3)      u = c0 + c1*x + c2*y
        bilinear  1.5 (4)     u = '' + c3*x*y
        quadratic  2 (6)      u = '' + c4*x*x + c5*y*y
        cubic      3 (10)     u = '' + c6*x^3 + c7*x*x*y + c8*x*y*y + c9*y^3
        quartic    4 (15)     u = '' + c10*x^4 + ... + c14*y^4
        quintic    5 (21)     u = '' + c15*x^5 + ... + c20*y^5
     number in parenthesis is minimum number of points needed.
     Anything beyond quintic, has not been implemented until
     a more automated way of determined terms is found.
   */
  if ( order < 1 || order > 5 ||
       ( order != floor(order) && (order-1.5) > MagickEpsilon) )
    return 0; /* invalid polynomial order */
  return ( (long)floor((order+1)*(order+2)/2) );
}

static double poly_term(long n, double x, double y)
{
  /* return the result for this polynomial term */
  switch(n) {
    case  0:  return( 1.0 ); /* constant */
    case  1:  return(  x  );
    case  2:  return(  y  ); /* affine      order = 1   terms = 3 */
    case  3:  return( x*y ); /* bilinear    order = 1.5 terms = 4 */
    case  4:  return( x*x );
    case  5:  return( y*y ); /* quadratic   order = 2   terms = 6 */
    case  6:  return( x*x*x );
    case  7:  return( x*x*y );
    case  8:  return( x*y*y );
    case  9:  return( y*y*y ); /* cubic       order = 3   terms = 10 */
    case 10:  return( x*x*x*x );
    case 11:  return( x*x*x*y );
    case 12:  return( x*x*y*y );
    case 13:  return( x*y*y*y );
    case 14:  return( y*y*y*y ); /* quartic     order = 4   terms = 15 */
    case 15:  return( x*x*x*x*x );
    case 16:  return( x*x*x*x*y );
    case 17:  return( x*x*x*y*y );
    case 18:  return( x*x*y*y*y );
    case 19:  return( x*y*y*y*y );
    case 20:  return( y*y*y*y*y ); /* quintic     order = 5   terms = 21 */
  }
  return( 0 ); /* should never happen */
}
static const char *poly_term_str(long n)
{
  /* return the result for this polynomial term */
  switch(n) {
    case  0:  return(""); /* constant */
    case  1:  return("*i");
    case  2:  return("*j"); /* affine      order = 1   terms = 3 */
    case  3:  return("*i*j"); /* bilinear    order = 1.5 terms = 4 */
    case  4:  return("*i*i");
    case  5:  return("*j*j"); /* quadratic   order = 2   terms = 6 */
    case  6:  return("*i*i*i");
    case  7:  return("*i*i*j");
    case  8:  return("*i*j*j");
    case  9:  return("*j*j*j"); /* cubic       order = 3   terms = 10 */
    case 10:  return("*i*i*i*i");
    case 11:  return("*i*i*i*j");
    case 12:  return("*i*i*j*j");
    case 13:  return("*i*j*j*j");
    case 14:  return("*j*j*j*j"); /* quartic     order = 4   terms = 15 */
    case 15:  return("*i*i*i*i*i");
    case 16:  return("*i*i*i*i*j");
    case 17:  return("*i*i*i*j*j");
    case 18:  return("*i*i*j*j*j");
    case 19:  return("*i*j*j*j*j");
    case 20:  return("*j*j*j*j*j"); /* quintic     order = 5   terms = 21 */
  }
  return( "UNKNOWN" ); /* should never happen */
}
static double poly_term_dx(long n, double x, double y)
{
  /* polynomial term for x derivative */
  switch(n) {
    case  0:  return( 0.0 ); /* constant */
    case  1:  return( 1.0 );
    case  2:  return( 0.0 ); /* affine      order = 1   terms = 3 */
    case  3:  return(  y  ); /* bilinear    order = 1.5 terms = 4 */
    case  4:  return(  x  );
    case  5:  return( 0.0 ); /* quadratic   order = 2   terms = 6 */
    case  6:  return( x*x );
    case  7:  return( x*y );
    case  8:  return( y*y );
    case  9:  return( 0.0 ); /* cubic       order = 3   terms = 10 */
    case 10:  return( x*x*x );
    case 11:  return( x*x*y );
    case 12:  return( x*y*y );
    case 13:  return( y*y*y );
    case 14:  return( 0.0 ); /* quartic     order = 4   terms = 15 */
    case 15:  return( x*x*x*x );
    case 16:  return( x*x*x*y );
    case 17:  return( x*x*y*y );
    case 18:  return( x*y*y*y );
    case 19:  return( y*y*y*y );
    case 20:  return( 0.0 ); /* quintic     order = 5   terms = 21 */
  }
  return( 0.0 ); /* should never happen */
}
static double poly_term_dy(long n, double x, double y)
{
  /* polynomial term for y derivative */
  switch(n) {
    case  0:  return( 0.0 ); /* constant */
    case  1:  return( 0.0 );
    case  2:  return( 1.0 ); /* affine      order = 1   terms = 3 */
    case  3:  return(  x  ); /* bilinear    order = 1.5 terms = 4 */
    case  4:  return( 0.0 );
    case  5:  return(  y  ); /* quadratic   order = 2   terms = 6 */
    default:  return( poly_term_dx(n-1,x,y) ); /* weird but true */
  }
  /* NOTE: the only reason that last is not true for 'quadtratic'
     is due to the re-arrangement of terms to allow for 'bilinear'
  */
}

static double *DistortCoefficents(Image *image, DistortImageMethod *method,
     const unsigned long number_arguments, const double *arguments,
     MagickBooleanType bestfit, ExceptionInfo *exception)
{
  /* Determine correct coefficients for the given input arguments.
     The given distortion method may be modified, to use a simplier
     distortion method to better handle the requested arguments.

     The returned array of coefficients, will need to be freed by the caller.
     by calling    RelinquishMagickMemory()

     If NULL is returned an exception has occured.

     WARNING: this routine will change, to allow for generating coefficients
     for a FUTURE 2d gradient generation.
  */

  double
    *coefficients;

  register int
    i;

  unsigned long
    number_coefficients;   /* number of coefficients needed */

  /* If not enough control point pairs are found for specific distortions
    fall back to Affine distortion (allowing 0 to 3 point pairs)
  */
  if ( number_arguments < 16 &&
       (  *method == BilinearDistortion
       || *method == PerspectiveDistortion
       ) )
    *method = AffineDistortion;

  switch (*method)
  {
    case ScaleRotateTranslateDistortion:
    case AffineDistortion:
    case AffineProjectionDistortion:
      number_coefficients=6;
      break;
    case PerspectiveDistortion:
    case PerspectiveProjectionDistortion:
      number_coefficients=9;
      break;
    case BilinearDistortion:
      number_coefficients=8;
      break;
    case PolynomialDistortion:
      if ( number_arguments < 5 && (number_arguments-1)%4 != 0)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'","distort Polynomial",
                     "Invalid number of coord-pairs: order [sX,sY,dX,dY]*");
        return((double *) NULL);
      }
      i = poly_number_terms(arguments[0]);
      number_coefficients = i*2 + 2;
      if ( i == 0 )
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'","distort Polynomial",
                     "Invalid order, should be 1,1.5,2,3,4 or 5");
        return((double *) NULL);
      }
      if ( number_arguments < i*4+1)
      {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument", "%s : '%s %ld'","distort Polynomial",
                     "Not enough coord-pairs, minimum", number_coefficients);
        return((double *) NULL);
      }
      break;
    case ArcDistortion:
      number_coefficients=5;
      break;
    case UndefinedDistortion:
    default:
      number_coefficients=0; /* should never happen */
      break;
  }

  /* allocate the array of coefficients needed */
  coefficients = AcquireQuantumMemory(number_coefficients,
                        sizeof(*coefficients));
  if (coefficients == (double *) NULL) {
    (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
    return((double *) NULL);
  }
  /* zero out coeffiecents array */
  for (i=0; i < (long)number_coefficients; i++)
    coefficients[i] = 0.0;

  switch (*method)
  {
    case AffineDistortion:
    {
      /* Affine Distortion
           u =  c0*x + c2*y + c4
           v =  c1*x + c3*y + c5

         Input Arguments are any number of pairs of distorted coodinates
         Which will be used to generate coefficients for Distortion.
            u1,v1, x1,y1,  u2,v2, x2,y2,  u3,v3, x3,y3,  u4,v4, x4,y4 ...
      */
      if (number_arguments%4 != 0) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'","distort Map",
                     "Requires pairs of coords (sX,sY,dX,dY)");
        RelinquishMagickMemory(coefficients);
        return((double *) NULL);
      }
      /* handle special cases of not enough arguments */
      if ( number_arguments == 0 ) {
        /* null distortion -- use No-Op Affine Distortion
            coefficients: sx, rx, ry, sy, tx, ty
        */
        *method = AffineDistortion;
        coefficients[0] = 1.0;
        coefficients[3] = 1.0;
      }
      else if ( number_arguments == 4 ) {
        /* if only 1 pair of points - Affine Translation only
            coefficients: sx, rx, ry, sy, tx, ty
        */
        *method = AffineDistortion;
        coefficients[0] = 1.0;
        coefficients[3] = 1.0;
        coefficients[4] = arguments[0] - arguments[2];
        coefficients[5] = arguments[1] - arguments[3];
      }
      else {
        /* 2 or more points (usally 3) given.
           Solve a least squares simultanious equation for coefficients.
        */
        double
          **matrix,
          **vectors,
          terms[3];

        MagickBooleanType
          status;

        *method = AffineDistortion;
        matrix = AcquireMagickMatrix(3UL,3UL);
        vectors = AcquireMagickMatrix(2UL,3UL);
        if (matrix == (double **) NULL || vectors == (double **) NULL)
        {
          matrix = RelinquishMagickMatrix(matrix, 3UL);
          vectors = RelinquishMagickMatrix(vectors, 2UL);
          coefficients = RelinquishMagickMemory(coefficients);
          (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
          return((double *) NULL);
        }
        /* Add given control point pairs for least squares solving */
        for (i=0; i < (long) number_arguments; i+=4) {
          terms[0] = arguments[i+2];  /* x */
          terms[1] = arguments[i+3];  /* y */
          terms[2] = 1;               /* 1 */
          LeastSquaresAddTerms(matrix,vectors,terms,&(arguments[i]),3UL,2UL);
        }
        if ( number_arguments == 8 ) {
          /* Only two pairs were given, but we need 3 to solve the affine.
             Fake a pair by rotating p1 around p0 by 90 degrees.
             That is given    u0,v0 x0,y0   u1,v1 x1,y1
             Then   u2 = u0 - (v1-v0)   v2 = v0 + (u1-u0)
                    x2 = x0 - (y1-y0)   y2 = y0 + (x1-x0)
          */
          double
            uv2[2];

          uv2[0] = arguments[0] - arguments[5] + arguments[1];   /* u2 */
          uv2[1] = arguments[1] + arguments[4] - arguments[0];   /* v2 */
          terms[0] = arguments[2] - arguments[7] + arguments[3]; /* x2 */
          terms[1] = arguments[3] + arguments[6] - arguments[2]; /* y2 */
          terms[2] = 1;                                          /* 1 */
          LeastSquaresAddTerms(matrix,vectors,terms,uv2,3UL,2UL);
        }
      /* Solve for LeastSquares Coefficients */
        status=GaussJordanElimination(matrix,vectors,3UL,2UL);
        matrix = RelinquishMagickMatrix(matrix, 3UL);
        if ( status == MagickTrue ) {
          /* Affine is a bit weird in its ordering of coefficients.
            This ordering is for backward compatibility with the older
            affine functions of IM.

            coefficients order: sx, rx, ry, sy, tx, ty
            Where    u = sx*x + ry*y + tx
                     v = rx*x + sy*y + ty
          */
          coefficients[0] = vectors[0][0]; /* sx */
          coefficients[1] = vectors[1][0]; /* rx */
          coefficients[2] = vectors[0][1]; /* ry */
          coefficients[3] = vectors[1][1]; /* sy */
          coefficients[4] = vectors[0][2]; /* tx */
          coefficients[5] = vectors[1][2]; /* ty */
        }
        vectors = RelinquishMagickMatrix(vectors, 2UL);
        if ( status == MagickFalse ) {
          coefficients = RelinquishMagickMemory(coefficients);
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                      "InvalidArgument","%s : '%s'","distort Affine",
                      "Unsolvable Matrix");
          return((double *) NULL);
        }
      }
      if ( GetImageArtifact(image,"distort:verbose") != (const char *) NULL ) {
        double *inverse = AcquireQuantumMemory(8,sizeof(coefficients[i]));
        if (inverse == (double *) NULL) {
          coefficients = RelinquishMagickMemory(coefficients);
          (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
          return((double *) NULL);
        }
        InvertAffineCoefficients(coefficients, inverse);
        fprintf(stderr, "Affine Reverse Map\n");
        fprintf(stderr, "  -fx 'xx=%+lf*i %+lf*j %+lf;\n",
            coefficients[0], coefficients[2], coefficients[4]);
        fprintf(stderr, "       yy=%+lf*i %+lf*j %+lf; p{xx,yy}'\n",
            coefficients[1], coefficients[3], coefficients[5]);
        fprintf(stderr, "Affine Forward Map\n");
        fprintf(stderr, "  -distort AffineProjection \\\n      '");
        for (i=0; i<5; i++)
          fprintf(stderr, "%lg,", inverse[i]);
        fprintf(stderr, "%lg'\n", inverse[5]);
        inverse = RelinquishMagickMemory(inverse);
      }
      return(coefficients);
    }
    case AffineProjectionDistortion:
    {
      /*
        Arguments: Affine Matrix (forward mapping)
        Arguments  sx, rx, ry, sy, tx, ty
        Where      u = sx*x + ry*y + tx
                   v = rx*x + sy*y + ty
      */
      if (number_arguments != 6) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'","distort AffineProjection",
                     "Needs 6 numbers");
        return((double *) NULL);
      }
      InvertAffineCoefficients(arguments, coefficients);
      if ( GetImageArtifact(image,"distort:verbose") != (const char *) NULL ) {
        fprintf(stderr, "Affine Reverse Map\n");
        fprintf(stderr, "  -fx 'xx=%+lf*i %+lf*j %+lf;\n",
            coefficients[0], coefficients[2], coefficients[4]);
        fprintf(stderr, "       yy=%+lf*i %+lf*j %+lf; p{xx,yy}'\n",
            coefficients[1], coefficients[3], coefficients[5]);
        /* No AffineProjection output, as that is what was given! */
      }
      return(coefficients);
    }
    case PerspectiveDistortion:
    { /*
         Perspective Distortion (a ratio of affine distortions)

                p()     c0*x + c1*y + c2
            u = ----- = ------------------
                r()     c6*x + c7*y + 1

                q()     c3*x + c4*y + c5
            v = ----- = ------------------
                r()     c6*x + c7*y + 1

           c8 = Sign of 'r', or the denominator affine, for the actual image.
                This determines what part of the distorted image is 'ground'
                side of the horizon, the other part is 'sky' or invalid.
                Valid values are  +1.0  or  -1.0  only.

         WARNING: Perspective Equations are linked (not separatable) as c6
         and c7 are shared by both equations.  All 8 coefficients need to be
         determined simultaneously.
      */
      double
        **matrix,
        *vectors[1],
        terms[8];

      MagickBooleanType
        status;

      *method = PerspectiveDistortion;
      /* fake 1x8 vectors matrix directly from coefficients array */
      vectors[0] = &(coefficients[0]);
      /* 8x8 least-squares matrix (zeroed) */
      matrix = AcquireMagickMatrix(8UL,8UL);
      if (matrix == (double **) NULL) {
        (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
        return((double *) NULL);
      }
      /* Add control points for least squares solving */
      for (i=0; i < (long) number_arguments; i+=4) {
        terms[0] = arguments[i+2];        /*   c0*x   */
        terms[1] = arguments[i+3];        /*   c1*y   */
        terms[2]=1.0;                     /*   c2*1   */
        terms[3]=0.0;
        terms[4]=0.0;
        terms[5]=0.0;
        terms[6]=-terms[0]*arguments[i];  /* 1/(c6*x) */
        terms[7]=-terms[1]*arguments[i];  /* 1/(c7*y) */
        LeastSquaresAddTerms(matrix,vectors,terms,&(arguments[i]),
            8UL,1UL);

        terms[0]=0.0;
        terms[1]=0.0;
        terms[2]=0.0;
        terms[3] = arguments[i+2];          /*   c3*x   */
        terms[4] = arguments[i+3];          /*   c4*y   */
        terms[5]=1.0;                       /*   c5*1   */
        terms[6]=-terms[3]*arguments[i+1];  /* 1/(c6*x) */
        terms[7]=-terms[4]*arguments[i+1];  /* 1/(c7*y) */
        LeastSquaresAddTerms(matrix,vectors,terms,&(arguments[i+1]),
            8UL,1UL);
      }
      /* Solve for LeastSquares Coefficients */
      status=GaussJordanElimination(matrix,vectors,8UL,1UL);
      matrix = RelinquishMagickMatrix(matrix, 8UL);
      if ( status == MagickFalse ) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                    "InvalidArgument","%s : '%s'","distort Prespective",
                    "Unsolvable Matrix");
        return((double *) NULL);
      }
      /*
        Calculate 9'th coefficient! The ground-sky determination.
        What is sign of the 'ground' in r() denominator affine function?
        Just use any valid image coordinate in destination for determination.
        Picking destination coordinate from first coordinate pair.
      */
      coefficients[8] = coefficients[6]*arguments[2]
                          + coefficients[7]*arguments[3] + 1.0;
      coefficients[8] = (coefficients[8] < 0.0) ? -1.0 : +1.0;

      if ( GetImageArtifact(image,"distort:verbose") != (const char *) NULL ) {
        double *inverse = AcquireQuantumMemory(8,sizeof(coefficients[i]));
        if (inverse == (double *) NULL) {
          coefficients = RelinquishMagickMemory(coefficients);
          (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
          return((double *) NULL);
        }
        InvertPerspectiveCoefficients(coefficients, inverse);
        fprintf(stderr, "Perspective Reverse Map\n");
        fprintf(stderr, "  -fx 'xx=%+lf*i %+lf*j %+lf;\n",
            coefficients[0], coefficients[1], coefficients[2]);
        fprintf(stderr, "       yy=%+lf*i %+lf*j %+lf;\n",
            coefficients[3], coefficients[4], coefficients[5]);
        fprintf(stderr, "       rr=%+lf*i %+lf*j + 1; p{xx/rr,yy/rr}'\n",
            coefficients[6], coefficients[7]);
        fprintf(stderr, "Perspective Forward Map\n");
        fprintf(stderr, "  -distort PerspectiveProjection \\\n      '");
        for (i=0; i<4; i++)
          fprintf(stderr, "%lg,", inverse[i]);
        fprintf(stderr, "\n       ");
        for (; i<7; i++)
          fprintf(stderr, "%lg,", inverse[i]);
        fprintf(stderr, "%lg'\n", inverse[7]);
        inverse = RelinquishMagickMemory(inverse);
      }
      return(coefficients);
    }
    case PerspectiveProjectionDistortion:
    {
      /*
        Arguments: Perspective Coefficents (forward mapping)
      */
      if (number_arguments != 8) {
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'",
                     "distort PerspectiveProjection", "Needs 8 numbers");
        return((double *) NULL);
      }
      InvertPerspectiveCoefficients(arguments, coefficients);
      if ( GetImageArtifact(image,"distort:verbose") != (const char *) NULL ) {
        fprintf(stderr, "Perspective Reverse Map\n");
        fprintf(stderr, "  -fx 'xx=%+lf*i %+lf*j %+lf;\n",
            coefficients[0], coefficients[1], coefficients[2]);
        fprintf(stderr, "       yy=%+lf*i %+lf*j %+lf;\n",
            coefficients[3], coefficients[4], coefficients[5]);
        fprintf(stderr, "       rr=%+lf*i %+lf*j + 1; p{xx/rr,yy/rr}'\n",
            coefficients[6], coefficients[7]);
      }
      /*
        Calculate 9'th coefficient! The ground-sky determination.
        What is sign of the 'ground' in r() denominator affine function?
        Just use any valid image cocodinate in destination for determination.
        For a forward mapped perspective the images 0,0 coord will map to
        c2,c5 in the distorted image, so set the sign of denominator of that.
      */
      coefficients[8] = coefficients[6]*arguments[2]
                           + coefficients[7]*arguments[5] + 1.0;
      coefficients[8] = (coefficients[8] < 0.0) ? -1.0 : +1.0;
      return(coefficients);
    }
    case BilinearDistortion:
    {
      double
        **matrix,
        *vectors[2],
        terms[4];

      MagickBooleanType
        status;

      /* Bilinear Distortion (currently reversed -- this needs to be fixed)
            u = c0*x + c1*y + c2*x*y + c3;
            v = c4*x + c5*y + c6*x*y + c7;

         Input Arguments are pairs of distorted coodinates (minimum 4 pairs)
         Which will be used to generate the coefficients of the above.
            u1,v1, x1,y1,  u2,v2, x2,y2,  u3,v3, x3,y3,  u4,v4, x4,y4 ...
      */
      if (number_arguments%4 != 0) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'","distort Bilinear",
                     "Requires pairs of coords (sX,sY,dX,dY)");
        return((double *) NULL);
      }
      matrix = AcquireMagickMatrix(4UL,4UL);
      if (matrix == (double **) NULL) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
        return((double *) NULL);
      }
      /* fake a 2x3 vectors matrix from coefficients array */
      vectors[0] = &(coefficients[0]);
      vectors[1] = &(coefficients[4]);
      /* Add control points for least squares solving */
      for (i=0; i < (long) number_arguments; i+=4) {
        terms[0] = arguments[i+2];     /*  x  */
        terms[1] = arguments[i+3];     /*  y  */
        terms[2] = terms[0]*terms[1];  /* x*y */
        terms[3] = 1;                  /*  1  */
        LeastSquaresAddTerms(matrix,vectors,terms,&(arguments[i]),4UL,2UL);
      }
      /* Solve for LeastSquares Coefficients */
      status=GaussJordanElimination(matrix,vectors,4UL,2UL);
      matrix = RelinquishMagickMatrix(matrix, 4UL);
      if ( status == MagickFalse ) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'","distort Bilinear",
                     "Unsolvable Matrix");
        return((double *) NULL);
      }
      if ( GetImageArtifact(image,"distort:verbose") != (const char *) NULL ) {
        fprintf(stderr, "Bilinear Reverse Map\n");
        fprintf(stderr, "  -fx 'xx=%+lf*i %+lf*j %+lf*i*j %+lf;\n",
            coefficients[0], coefficients[1], coefficients[2], coefficients[3]);
        fprintf(stderr, "       yy=%+lf*i %+lf*j %+lf*i*j %+lf;\n",
            coefficients[4], coefficients[5], coefficients[6], coefficients[7]);
        fprintf(stderr, "       p{xx,yy}'\n");
      }
      return(coefficients);
    }
    case PolynomialDistortion:
    {
      /* Polynomial Distortion
           c0 = Order of the polynimial being created
           c1 = number of terms in one polynomial equation

            u = c2 + c3*x + c4*y + c5*x*y + c6*x^2 + c7*y^2 + c8*x^3 + ...
            v = ....

         Input Arguments are pairs of distorted coodinates
         Which will be used to generate the coefficients of the above.
            u1,v1, x1,y1,  u2,v2, x2,y2,  u3,v3, x3,y3,  u4,v4, x4,y4 ...
      */
      double
        **matrix,
        *vectors[2],
        *terms;

      long
        nterms;

      register long
        j;

      MagickBooleanType
        status;

      coefficients[0] = arguments[0];
      coefficients[1] = nterms = poly_number_terms(arguments[0]);

      terms = AcquireQuantumMemory(nterms, sizeof(*terms));
      matrix = AcquireMagickMatrix(nterms,nterms);
      if (terms == (double *) NULL && matrix == (double **) NULL) {
        terms = RelinquishMagickMemory(terms);
        matrix = RelinquishMagickMatrix(matrix, nterms);
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
        return((double *) NULL);
      }
      /* fake a 2xN vectors matrix from rest of coefficients array */
      vectors[0] = &(coefficients[2]);
      vectors[1] = &(coefficients[2+nterms]);
      /* Add control points for least squares solving */
      for (i=1; i < number_arguments; i+=4) {
        for (j=0; j < nterms; j++)
          terms[j] = poly_term(j,arguments[i+2],arguments[i+3]);
        LeastSquaresAddTerms(matrix,vectors,terms,&(arguments[i]),nterms,2UL);
      }
      terms = RelinquishMagickMemory(terms);
      /* Solve for LeastSquares Coefficients */
      status=GaussJordanElimination(matrix,vectors,nterms,2UL);
      matrix = RelinquishMagickMatrix(matrix, nterms);
      if ( status == MagickFalse ) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'","distort Polynomial",
                     "Unsolvable Matrix");
        return((double *) NULL);
      }
      if ( GetImageArtifact(image,"distort:verbose") != (const char *) NULL ) {
        fprintf(stderr, "Polynomial (order %lg, terms %lg) Reverse Map\n",
             coefficients[0], coefficients[1]);
        fprintf(stderr, "  -fx 'xx =");
        for (i=0; i<nterms; i++) {
          if ( i != 0 && i%4 == 0 ) fprintf(stderr, "\n         ");
          fprintf(stderr, " %+lf%s", coefficients[i+2], poly_term_str(i));
        }
        fprintf(stderr, ";\n       yy =");
        for (i=0; i<nterms; i++) {
          if ( i != 0 && i%4 == 0 ) fprintf(stderr, "\n         ");
          fprintf(stderr, " %+lf%s", coefficients[i+2+nterms], poly_term_str(i));
        }
        fprintf(stderr, ";\n       p{xx,yy}'\n");
      }
      return(coefficients);
    }
    case ScaleRotateTranslateDistortion:
    {
      double
        cosine, sine,
        x,y,sx,sy,a,nx,ny;

      /* Scale, Rotate and Translate Distortion
         Argument options, by number of arguments given:
           7: x,y, sx,sy, a, nx,ny
           6: x,y,   s,   a, nx,ny
           5: x,y, sx,sy, a
           4: x,y,   s,   a
           3: x,y,        a
           2:        s,   a
           1:             a
         Where actions are (in order of application)
            x,y     'center' of transforms     (default = image center)
            sx,sy   scale image by this amount (default = 1)
            a       angle of rotation          (argument required)
            nx,ny   move 'center' here         (default = no movement)
         And convert to affine mapping coefficients
      */
      x = nx = (double)image->columns/2.0;
      y = ny = (double)image->rows/2.0;
      if ( bestfit ) {
        x = nx += (double)image->page.x;
        y = ny += (double)image->page.y;
      }
      sx = sy = 1.0;
      switch ( number_arguments ) {
      case 0:
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'",
                     "distort ScaleTranslateRotate",
                     "Needs at least 1 argument");
        return((double *) NULL);
      case 1:
        a = arguments[0];
        break;
      case 2:
        sx = sy = arguments[0];
        a = arguments[1];
        break;
      default:
        x = nx = arguments[0];
        y = ny = arguments[1];
        switch ( number_arguments ) {
        case 3:
          a = arguments[2];
          break;
        case 4:
          sx = sy = arguments[2];
          a = arguments[3];
          break;
        case 5:
          sx = arguments[2];
          sy = arguments[3];
          a = arguments[4];
          break;
        case 6:
          sx = sy = arguments[2];
          a = arguments[3];
          nx = arguments[4];
          ny = arguments[5];
          break;
        case 7:
          sx = arguments[2];
          sy = arguments[3];
          a = arguments[4];
          nx = arguments[5];
          ny = arguments[6];
          break;
        default:
          coefficients = RelinquishMagickMemory(coefficients);
          (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'",
                     "distort ScaleTranslateRotate",
                     "Too Many Arguments (7 or less)");
          return((double *) NULL);
        }
        break;
      }
      /* FUTURE: trap if sx or sy == 0 -- image is scaled out of existance! */
      if ( fabs(sx) < MagickEpsilon || fabs(sy) < MagickEpsilon ) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'",
                     "distort ScaleTranslateRotate", "Zero Scale");
        return((double *) NULL);
      }
      /* Save the given arguments as an affine distortion */
      *method = AffineDistortion;
      a=DegreesToRadians(a);
      cosine=cos(a);
      sine=sin(a);
      coefficients[0]=cosine/sx;
      coefficients[1]=(-sine)/sy;
      coefficients[2]=sine/sx;
      coefficients[3]=cosine/sy;
      coefficients[4]=x-nx*coefficients[0]-ny*coefficients[2];
      coefficients[5]=y-nx*coefficients[1]-ny*coefficients[3];

      if ( GetImageArtifact(image,"distort:verbose") != (const char *) NULL ) {
        double *inverse = AcquireQuantumMemory(8,sizeof(coefficients[i]));
        if (inverse == (double *) NULL) {
          coefficients = RelinquishMagickMemory(coefficients);
          (void) ThrowMagickException(exception,GetMagickModule(),
                  ResourceLimitError,"MemoryAllocationFailed",
                  "%s", "DistortCoefficients");
          return((double *) NULL);
        }
        InvertAffineCoefficients(coefficients, inverse);
        fprintf(stderr, "Affine Reverse Map\n");
        fprintf(stderr, "  -fx 'xx=%+lf*i %+lf*j %+lf;\n",
            coefficients[0], coefficients[2], coefficients[4]);
        fprintf(stderr, "       yy=%+lf*i %+lf*j %+lf; p{xx,yy}'\n",
            coefficients[1], coefficients[3], coefficients[5]);
        fprintf(stderr, "Affine Forward Map\n");
        fprintf(stderr, "  -distort AffineProjection \\\n      '");
        for (i=0; i<5; i++)
          fprintf(stderr, "%lg,", inverse[i]);
        fprintf(stderr, "%lg'\n", inverse[5]);
        inverse = RelinquishMagickMemory(inverse);
      }
      return(coefficients);
    }
    case ArcDistortion:
    {
      /* Arc Distortion
         Args: arc_width  rotate  top_edge_radius  bottom_edge_radius
         All but first argument are optional
            arc_width      The angle over which to arc the image side-to-side
            rotate         Angle to rotate image from vertical center
            top_radius     Set top edge of source image at this radius
            bottom_radius  Set bootom edge to this radius (radial scaling)

         By default, if the radii arguments are nor provided the image radius
         is calculated so the horizontal center-line is fits the given arc
         without scaling.

         The output image size is ALWAYS adjusted to contain the whole image,
         and an offset is given to position image relative to the 0,0 point of
         the origin, allowing users to use relative positioning onto larger
         background (via -flatten).

         The arguments are converted to these coefficients
            c0: angle for center of source image
            c1: angle scale for mapping to source image
            c2: radius for top of source image
            c3: radius scale for mapping source image
            c4: centerline of arc within source image

         Note the coefficients use a center angle, so asymptotic join is
         furthest from both sides of the source image. This also means that
         for arc angles greater than 360 the sides of the image will be
         trimmed equally.
      */
      if ( number_arguments >= 1 && arguments[0] < MagickEpsilon ) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'", "distort Arc",
                     "Arc Angle Too Small");
        return((double *) NULL);
      }
      if ( number_arguments >= 3 && arguments[2] < MagickEpsilon ) {
        coefficients = RelinquishMagickMemory(coefficients);
        (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
                     "InvalidArgument","%s : '%s'", "distort Arc",
                     "Outer Radius Too Small");
        return((double *) NULL);
      }
      coefficients[0] = -MagickPI/2.0;
      if ( number_arguments >= 1 )
        coefficients[1] = DegreesToRadians(arguments[0]);
      else
        coefficients[1] = MagickPI/2.0;
      if ( number_arguments >= 2 )
        coefficients[0] += DegreesToRadians(arguments[1]);
      coefficients[0] -= MagickRound(coefficients[0]/(2.0*MagickPI))
                             *2.0*MagickPI;
      coefficients[3] = 1.0*image->rows-1;
      coefficients[2] = 1.0*image->columns/coefficients[1] + coefficients[3]/2.0;
      if ( number_arguments >= 3 ) {
        if ( number_arguments >= 4 )
          coefficients[3] = arguments[2] - arguments[3];
        else
          coefficients[3] *= arguments[2]/coefficients[2];
        coefficients[2] = arguments[2];
      }
      coefficients[4] = (1.0*image->columns-1.0)/2.0;
      if ( GetImageArtifact(image,"distort:verbose") != (const char *) NULL ) {
        fprintf(stderr, "Arc Reverse Map\n");
#if 0
  Not working yet
        fprintf(stderr, "c0=%-8lg  # angle for center line in source image\n",
             coefficients[0]);
        fprintf(stderr, "c1=%-8lg  # angle scale for mapping to source image\n",
             coefficients[1]);
        fprintf(stderr, "c2=%-8lg  # radius for top of source imagen\n",
             coefficients[2]);
        fprintf(stderr, "c3=%-8lg  # radius scale for mapping source image\n",
             coefficients[3]);
        fprintf(stderr, "c4=%-8lg  # centerline of arc within source image\n",
             coefficients[4]);

        coefficients[1] = 2.0*MagickPI*image->columns/coefficients[1];
        coefficients[3] = 1.0*image->rows/coefficients[3];
          point.x = (atan2((double)y,(double)x) - coefficients[0])/(2*MagickPI);
          point.x -= MagickRound(point.x);     /* angle */
          point.y = sqrt((double) x*x+y*y);    /* radius */
          point.x = point.x*coefficients[1] + coefficients[4];
          point.y = (coefficients[2] - point.y) * coefficients[3];


        fprintf(stderr, "  -size %ldx%ld -page -%ld-%ld xc: +swap \\\n",
            (long)coefficients[2]*2+2, (long)coefficients[2]*2+2,
            (long)coefficients[2]+1, (long)coefficients[2]+1);
        fprintf(stderr, "  -fx 'xx=(atan2(j-h/2,i-w/2) %+lg)*2*pi;\n",
            coefficients[0]);
        fprintf(stderr, "       xx=(xx%%1+1.5)%%1-0.5;\n");
        fprintf(stderr, "       yy=hypot(i-w/2,j-h/2);\n");
        fprintf(stderr, "       xx=xx*%lg + %lg;\n",
            2.0*MagickPI*image->columns/coefficients[1], coefficients[4]);
        fprintf(stderr, "       yy=(%lg - yy) * %lg;\n",
            coefficients[2], ((double)image->rows)/coefficients[3] );
        fprintf(stderr, "       v.p{xx,yy}'\n");
#endif
      }
      return(coefficients);
    }
    default:
      break;
  }
  /* you should never reach this point */
  assert(MagickTrue);
  coefficients = RelinquishMagickMemory(coefficients);
  return((double *) NULL);
}


MagickExport Image *DistortImage(Image *image,DistortImageMethod method,
  const unsigned long number_arguments,const double *arguments,
  MagickBooleanType bestfit,ExceptionInfo *exception)
{
#define DistortImageTag  "Distort/Image"

  double
    *coefficients;

  Image
    *distort_image;

  RectangleInfo
    geometry;  /* geometry of the distorted space viewport */

  PointInfo
    point; /* best fit working */

  MagickBooleanType
    status;

  ResampleFilter
    **resample_filter;

  ViewInfo
    **distort_view;

  const char
    *artifact;

  long
    j;

  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);

  /*
    Convert input arguments into mapping coefficients for distortion
    Also replaces these distortions, into other forms
      ScaleRotateTranslateDistortion
      MapDistortion
    So these no longer need to be considered below.
  */
  coefficients = DistortCoefficents(image, &method, number_arguments,
       arguments, bestfit, exception);
  if ( coefficients == (double *) NULL )
    return((Image *) NULL);

  /*
    Determine the size and offset for a 'bestfit' destination.
    Usally the four corners of the source image is enough.
  */

  /* default output image bounds, when no 'bestfit' is requested */
  geometry.width=image->columns;
  geometry.height=image->rows;
  geometry.x=0;
  geometry.y=0;
  point.x=0.0;
  point.y=0.0;

  /* These distotions must be bestfit, as display is warped toomuch */
  if ( method == ArcDistortion )
    bestfit = MagickTrue;

  /* Work out the 'best fit', (required for ArcDistortion) */
  if ( bestfit ) {
    double
      min_x,max_x,min_y,max_y;

    register long
      x,y;

    min_x=max_x=min_y=max_y=0.0;   /* keep compiler happy */

/* defines to figure out the bounds of the distorted image */
#define InitalBounds(px,py) \
{ \
  min_x = max_x = (px); \
  min_y = max_y = (py); \
}
#define ExpandBounds(px,py) \
{ \
  if ( (px) < min_x )  min_x = (px); \
  if ( (px) > max_x )  max_x = (px); \
  if ( (py) < min_y )  min_y = (py); \
  if ( (py) > max_y )  max_y = (py); \
}

    switch (method)
    {
      case AffineDistortion:
      case AffineProjectionDistortion:
      { double inverse[6];
        InvertAffineCoefficients(coefficients, inverse);
        x = image->page.x;  y = image->page.y;
        point.x=inverse[0]*x+inverse[2]*y+inverse[4];
        point.y=inverse[1]*x+inverse[3]*y+inverse[5];
        InitalBounds( point.x, point.y );
        x = image->page.x+image->columns-1;  y = image->page.y;
        point.x=inverse[0]*x+inverse[2]*y+inverse[4];
        point.y=inverse[1]*x+inverse[3]*y+inverse[5];
        ExpandBounds( point.x, point.y );
        x = image->page.x;  y = image->page.y+image->rows-1;
        point.x=inverse[0]*x+inverse[2]*y+inverse[4];
        point.y=inverse[1]*x+inverse[3]*y+inverse[5];
        ExpandBounds( point.x, point.y );
        x = image->page.x+image->columns-1;
        y = image->page.y+image->rows-1;
        point.x=inverse[0]*x+inverse[2]*y+inverse[4];
        point.y=inverse[1]*x+inverse[3]*y+inverse[5];
        ExpandBounds( point.x, point.y );
        break;
      }
      case PerspectiveDistortion:
      case PerspectiveProjectionDistortion:
      { double inverse[8], scale;
        InvertPerspectiveCoefficients(coefficients, inverse);
        x = image->page.x;  y = image->page.y;
        scale=inverse[6]*x+inverse[7]*y+1.0;
        scale=1.0/(  (fabs(scale) <= MagickEpsilon) ? 1.0 : scale );
        point.x=scale*(inverse[0]*x+inverse[1]*y+inverse[2]);
        point.y=scale*(inverse[3]*x+inverse[4]*y+inverse[5]);
        InitalBounds( point.x, point.y );
        x = image->page.x+image->columns-1;  y = image->page.y;
        scale=inverse[6]*x+inverse[7]*y+1.0;
        scale=1.0/(  (fabs(scale) <= MagickEpsilon) ? 1.0 : scale );
        point.x=scale*(inverse[0]*x+inverse[1]*y+inverse[2]);
        point.y=scale*(inverse[3]*x+inverse[4]*y+inverse[5]);
        ExpandBounds( point.x, point.y );
        x = image->page.x;  y = image->page.y+image->rows-1;
        scale=inverse[6]*x+inverse[7]*y+1.0;
        scale=1.0/(  (fabs(scale) <= MagickEpsilon) ? 1.0 : scale );
        point.x=scale*(inverse[0]*x+inverse[1]*y+inverse[2]);
        point.y=scale*(inverse[3]*x+inverse[4]*y+inverse[5]);
        ExpandBounds( point.x, point.y );
        x = image->page.x+image->columns-1;
        y = image->page.y+image->rows-1;
        scale=inverse[6]*x+inverse[7]*y+1.0;
        scale=1.0/(  (fabs(scale) <= MagickEpsilon) ? 1.0 : scale );
        point.x=scale*(inverse[0]*x+inverse[1]*y+inverse[2]);
        point.y=scale*(inverse[3]*x+inverse[4]*y+inverse[5]);
        ExpandBounds( point.x, point.y );
        break;
      }
      case ArcDistortion:
      { double a, ca, sa;
        /* Forward Map Corners */
        a = coefficients[0]-coefficients[1]/2; ca = cos(a); sa = sin(a);
        point.x = coefficients[2]*ca;
        point.y = coefficients[2]*sa;
        InitalBounds( point.x, point.y );
        point.x = (coefficients[2]-coefficients[3])*ca;
        point.y = (coefficients[2]-coefficients[3])*sa;
        ExpandBounds( point.x, point.y );
        a = coefficients[0]+coefficients[1]/2; ca = cos(a); sa = sin(a);
        point.x = coefficients[2]*ca;
        point.y = coefficients[2]*sa;
        ExpandBounds( point.x, point.y );
        point.x = (coefficients[2]-coefficients[3])*ca;
        point.y = (coefficients[2]-coefficients[3])*sa;
        ExpandBounds( point.x, point.y );
        /* Orthogonal points along top of arc */
        for( a=ceil((coefficients[0]-coefficients[1]/2.0)*2.0/MagickPI)
                              *MagickPI/2.0;
               a<(coefficients[0]+coefficients[1]/2.0); a+=MagickPI/2.0 ) {
          ca = cos(a); sa = sin(a);
          point.x = coefficients[2]*ca;
          point.y = coefficients[2]*sa;
          ExpandBounds( point.x, point.y );
        }
        /*
          Convert the angle_to_width and radius_to_height
          to appropriate scaling factors, to allow faster processing
          in the mapping function.
        */
        coefficients[1] = 2.0*MagickPI*image->columns/coefficients[1];
        coefficients[3] = 1.0*image->rows/coefficients[3];
        break;
      }
      case BilinearDistortion:
      case PolynomialDistortion:
      default:
        /* no bestfit available for this distortion YET */
        bestfit = MagickFalse;
    }
    /* Set the output image geometry to calculated 'bestfit' */
    if ( bestfit ) {
      geometry.x=(long) floor(min_x-MagickEpsilon);
      geometry.y=(long) floor(min_y-MagickEpsilon);
      geometry.width=(unsigned long) ceil(max_x-geometry.x+1+MagickEpsilon);
      geometry.height=(unsigned long) ceil(max_y-geometry.y+1+MagickEpsilon);
    }
  }

  /* User provided a 'viewport' expert option which may
     overrides some parts of the current output image geometry.
     For ArcDistortion, this also overrides its default 'bestfit' setting.
  */
  artifact = GetImageArtifact(image,"distort:viewport");
  if (artifact != (const char *) NULL)
    (void) ParseAbsoluteGeometry(artifact,&geometry);

  /*
    Initialize the distort image attributes.
  */
  distort_image=CloneImage(image,geometry.width,geometry.height,MagickTrue,
    exception);
  if (distort_image == (Image *) NULL)
    return((Image *) NULL);
  if (SetImageStorageClass(distort_image,DirectClass) == MagickFalse)
    {
      InheritException(exception,&distort_image->exception);
      distort_image=DestroyImage(distort_image);
      return((Image *) NULL);
    }
  distort_image->page.x=geometry.x;
  distort_image->page.y=geometry.y;
  if (distort_image->background_color.opacity != OpaqueOpacity)
    distort_image->matte=MagickTrue;

  /* ----- MAIN LOOP -----
     Sample the source image to each pixel in the distort image.
  */
  status=MagickTrue;
  resample_filter=AcquireResampleFilterThreadSet(image,exception);
  distort_view=AcquireCacheViewThreadSet(distort_image);
  #pragma omp parallel for
  for (j=0; j < (long) distort_image->rows; j++)
  {
    long
      y;

    MagickPixelPacket
      pixel,    /* pixel to assign to distorted image */
      invalid;  /* the color to assign when distort result is invalid */

    PointInfo
      point;    /* point to sample (center of filtered resample of area) */

    register IndexPacket
      *indexes;

    register long
      i,
      id,
      x;

    register PixelPacket
      *q;

    double
      validity;

    id=GetCacheViewThreadId();
    q=SetCacheViewPixels(distort_view[id],0,j,distort_image->columns,1);
    if (q == (PixelPacket *) NULL)
      {
        status=MagickFalse;
        continue;
      }
    indexes=GetCacheViewIndexes(distort_view[id]);

    GetMagickPixelPacket(distort_image,&pixel);

    /* Define constant scaling vectors for Affine Distortions */
    switch (method)
    {
      case AffineDistortion:
      case AffineProjectionDistortion:
        ScaleResampleFilter( resample_filter[id],
          coefficients[0], coefficients[2],
          coefficients[1], coefficients[3] );
        break;
      default:
        break;
    }

    /* Initialize default pixel validity
     *    negative:         pixel is invalid  output 'matte_color'
     *    0.0 to 1.0:       antialiased, mix with resample output
     *    1.0 or greater:   use resampled output.
     */
    validity = 1.0;

    GetMagickPixelPacket(distort_image,&invalid);
    SetMagickPixelPacket(distort_image,&distort_image->matte_color,
      (IndexPacket *) NULL, &invalid);
    if (distort_image->colorspace == CMYKColorspace)
      ConvertRGBToCMYK(&invalid);   /* what about other color spaces? */
    point.x=0;
    point.y=0;
    y = j+geometry.y;   /* map y into virtual space */
    for (i=0; i < (long) distort_image->columns; i++)
    {
      x = i+geometry.x; /* map x into virtual space */
      switch (method)
      {
        case AffineDistortion:
        case AffineProjectionDistortion:
        {
          point.x=coefficients[0]*x+coefficients[2]*y+coefficients[4];
          point.y=coefficients[1]*x+coefficients[3]*y+coefficients[5];
          /* Affine partial derivitives are constant -- set above */
          break;
        }
        case PerspectiveDistortion:
        case PerspectiveProjectionDistortion:
        {
          double
            p,q,r,abs_r,abs_c6,abs_c7,scale;
          /* perspective is a ratio of affines */
          p=coefficients[0]*x+coefficients[1]*y+coefficients[2];
          q=coefficients[3]*x+coefficients[4]*y+coefficients[5];
          r=coefficients[6]*x+coefficients[7]*y+1.0;
          /* Pixel Validity -- is it a 'sky' or 'ground' pixel */
          validity = (r*coefficients[8] < 0.0) ? 0.0 : 1.0;
          /* Determine horizon anti-aliase blending */
          abs_r = fabs(r)*2;
          abs_c6 = fabs(coefficients[6]);
          abs_c7 = fabs(coefficients[7]);
          if ( abs_c6 > abs_c7 ) {
            if ( abs_r < abs_c6 )
              validity = 0.5 - coefficients[8]*r/coefficients[6];
          }
          else if ( abs_r < abs_c7 )
            validity = .5 - coefficients[8]*r/coefficients[7];
          /* Perspective Sampling Point (if valid) */
          if ( validity > 0.0 ) {
            scale = 1.0/r;
            point.x = p*scale;
            point.y = q*scale;
            /* Perspective Partial Derivatives or Scaling Vectors */
            scale *= scale;
            ScaleResampleFilter( resample_filter[id],
              (r*coefficients[0] - p*coefficients[6])*scale,
              (r*coefficients[1] - p*coefficients[7])*scale,
              (r*coefficients[3] - q*coefficients[6])*scale,
              (r*coefficients[4] - q*coefficients[7])*scale );
          }
          break;
        }
        case BilinearDistortion:
        {
          point.x=coefficients[0]*x+coefficients[1]*y+coefficients[2]*x*y+
            coefficients[3];
          point.y=coefficients[4]*x+coefficients[5]*y+coefficients[6]*x*y+
            coefficients[7];
          /* Bilinear partial derivitives of scaling vectors */
          ScaleResampleFilter( resample_filter[id],
              coefficients[0] + coefficients[2]*y,
              coefficients[1] + coefficients[2]*x,
              coefficients[4] + coefficients[6]*y,
              coefficients[5] + coefficients[6]*x );
          break;
        }
        case PolynomialDistortion:
        {
          register long
            k;
          long
            nterms=(long)coefficients[1];

          double
            dudx,dudy,dvdx,dvdy;

          point.x=point.y=dudx=dudy=dvdx=dvdy=0.0;
          for(k=0; k < nterms; k++) {
            point.x += poly_term(k,x,y)*coefficients[k+2];
            dudx += poly_term_dx(k,x,y)*coefficients[k+2];
            dudy += poly_term_dy(k,x,y)*coefficients[k+2];
            point.y += poly_term(k,x,y)*coefficients[k+2+nterms];
            dvdx += poly_term_dx(k,x,y)*coefficients[k+2+nterms];
            dvdy += poly_term_dy(k,x,y)*coefficients[k+2+nterms];
          }
          ScaleResampleFilter( resample_filter[id], dudx,dudy,dvdx,dvdy );
          break;
        }
        case ArcDistortion:
        {
          /* what is the angle and radius in the destination image */
          point.x = (atan2((double)y,(double)x) - coefficients[0])/(2*MagickPI);
          point.x -= MagickRound(point.x);     /* angle */
          point.y = sqrt((double) x*x+y*y);    /* radius */

          /* Polar Distortion Partial Derivatives or Scaling Vectors
             We give the deritives of  du/dr, dv/dr  and du/da, dv/da
             rather than provide the complex  du/dx,dv/dx and du/dy,dv/dy.
             The result will be the same, but it is simplier to calculate.
          */
          if ( point.y > MagickEpsilon )
            ScaleResampleFilter( resample_filter[id],
                coefficients[1]/(2*MagickPI)/point.y, 0, 0, coefficients[3] );
              //coefficients[1]*2*MagickPI/point.y, 0, 0, coefficients[3] );
          else
            ScaleResampleFilter( resample_filter[id],
                 MagickHuge, 0, 0, coefficients[3] );

          /* now scale the angle and radius for lookup image */
          point.x = point.x*coefficients[1] + coefficients[4];
          point.y = (coefficients[2] - point.y) * coefficients[3];
          break;
        }
        default:
        {
          /*
            Noop distortion (failsafe) should not happen!
          */
          point.x=(double) i;
          point.y=(double) j;
          break;
        }
      }
      /* map virtual canvas location back to real image coordinate */
      if ( bestfit && method != ArcDistortion ) {
        point.x -= image->page.x;
        point.y -= image->page.y;
      }

      if ( validity <= 0.0 ) {
        /* result of distortion is an invalid pixel - don't resample */
        SetPixelPacket(distort_image,&invalid,q,indexes);
      }
      else {
        /* resample the source image to find its correct color */
        pixel=ResamplePixelColor(resample_filter[id],point.x,point.y);
        /* if validity between 0.0 and 1.0 mix result with invalid pixel */
        if ( validity < 1.0 ) {
          /* Do a blend of sample color and invalid pixel */
          /* should this be a 'Blend', or an 'Over' compose */
          MagickPixelCompositeBlend(&pixel, validity,
               &invalid, (1.0-validity), &pixel);
        }
        SetPixelPacket(distort_image,&pixel,q,indexes);
      }
      q++;
      indexes++;
    }
    if (SyncCacheView(distort_view[id]) == MagickFalse)
      status=MagickFalse;
    if (SetImageProgress(image,DistortImageTag,y,image->rows) == MagickFalse)
      status=MagickFalse;
  }
  distort_view=DestroyCacheViewThreadSet(distort_view);
  resample_filter=DestroyResampleFilterThreadSet(resample_filter);

  /* Arc does not return an offset unless 'bestfit' is in effect
     And the user has not provided an overriding 'viewport'.
   */
  if ( method == ArcDistortion && !bestfit && artifact==(const char *)NULL ) {
    distort_image->page.x = 0;
    distort_image->page.y = 0;
  }
  if (status == MagickFalse)
    distort_image=DestroyImage(distort_image);
  return(distort_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S p a r s e C o l o r I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  SparseColorImage() takes a image and a set of coordinates of of known valid color
%  pixels within that image.  Then using various methods it it fills in all
%  the other unknown points for form a smooth gradient between those points.
%
%  No clone is made of the image, which is directly filled in.
%
%  The format of the SparseColorImage() method is:
%
%      Image *SparseColorImage(const Image *image,const SparseColorMethod
%        method, const unsigned long number_arguments,const double *arguments,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image to be filled in.
%
%    o method: the method to fill in the gradient between the points.
%
%    o number_arguments: the number of arguments given.
%
%    o arguments: the arguments for this distortion method.
%
%    o exception: Return any errors or warnings in this structure
%
*/

#if 0

   This is all under construction....

  Basically distortions and 2D color gradients are one and the same thing.
  Just that one distorts locations, the other distorts colors.


MagickExport Image *SparseColorImage(Image *image,const SparseColorMethod
  method, const unsigned long number_arguments,const double *arguments,
  ExceptionInfo *exception)
{
#define DistortImageTag  "Distort/Image"

#endif

