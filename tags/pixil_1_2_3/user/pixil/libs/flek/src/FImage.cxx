#include <Flek/math.H>
#include <Flek/FImage.H>
#include <string.h>
//#include <iostream.h>
//#include <fstream.h>

typedef unsigned long ulong;
typedef unsigned long *ulongPtr;
typedef unsigned short ushort;
typedef unsigned short *ushortPtr;
typedef unsigned char uchar;
typedef unsigned char *ucharPtr;

#define INT_MULT(a,b,t)  ((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8))
#define INT_BLEND(a,b,alpha,tmp)  (INT_MULT((a)-(b), alpha, tmp) + (b))

FImage::FImage()
{
    Data = 0;
    W = 0;
    H = 0;
    Channels = 4;
}

FImage::FImage(int w, int h, int channels)
{
    W = w;
    H = h;
    Channels = channels;
    Data = new uchar[W * H * Channels];
}

FImage::FImage(FImage * src)
{
    W = src->width();
    H = src->height();
    Channels = src->channels();
    Data = new uchar[W * H * Channels];
    memcpy((void *) Data, (void *) *(src->begin()), W * H * Channels);
}

FImage::~FImage()
{
    if (Data)
	delete[]Data;
}

void
FImage::channels(int dest_channels)
{
    uchar *src = Data;
    int src_channels = Channels;
    int k_max = min(src_channels, dest_channels);
    uchar *r = new uchar[W * H * dest_channels];
    uchar *dest = r;

    unsigned char *end = dest + dest_channels * W * H;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	for (int k = 0; k < k_max; k++)
	    dest[k] = src[k];
    }

    if ((Channels == 3) && (dest_channels == 4)) {
	dest = r;
	for (; dest < end; dest += dest_channels)
	    dest[3] = 255;
    }

    delete[]Data;
    Data = r;
    Channels = dest_channels;
}

void
FImage::flip_vertical()
{
    uchar row[Channels * W];
    for (int i = 0; i < H / 2; i++) {
	memcpy(row, &Data[Channels * W * i], Channels * W);
	memcpy(&Data[Channels * W * i], &Data[Channels * W * (H - i - 1)],
	       Channels * W);
	memcpy(&Data[Channels * W * (H - i - 1)], row, Channels * W);
    }
}

void
FImage::clear(uchar r, uchar g, uchar b, uchar a)
{
    int i;
    if (Channels >= 4)
	for (i = 0; i < Channels * W * H; i += Channels) {
	    Data[i] = r;
	    Data[i + 1] = g;
	    Data[i + 2] = b;
	    Data[i + 3] = a;
    } else if (Channels == 3) {
	for (i = 0; i < Channels * W * H; i += Channels) {
	    Data[i] = r;
	    Data[i + 1] = g;
	    Data[i + 2] = b;
	}
    } else if (Channels == 2) {
	for (i = 0; i < Channels * W * H; i += Channels) {
	    Data[i] = r;
	    Data[i + 1] = g;
	}
    } else if (Channels == 1) {
	for (i = 0; i < Channels * W * H; i += Channels)
	    Data[i] = r;
    }

}

FImage *
FImage::scale(int x, int y)
{
    FImage *r = new FImage(x, y, Channels);

    uchar *dest = *(r->begin());
    for (int i = 0; i < y; i++)
	for (int j = 0; j < x; j++)
	    for (int k = 0; k < Channels; k++, dest++) {
		float tx = ((float) j / (float) x);
		float ty = ((float) i / (float) y);
		int ix = (int) (tx * W);
		int iy = (int) (ty * H);
		dest[0] = Data[iy * Channels * W + ix * Channels + k];
	    }

    return r;
}

/*
 * All operations are done "in-place" on image A and do not return a 
 * new image for efficiency.  If you want a new image, copy image A
 * and then work on the copy.
 */

void
composite(unsigned char *dest,
	  const unsigned char *src,
	  int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    // b + (1-b)*a
	    if (dest_channels > 3)
		dest[3] = min(src[3] + INT_MULT(255 - src[3], dest[3], t1), 255);	//INT_MULT(dest[3], a, t1); //dest[3] = 255;
	} else
	    a = opacity;
	dest[0] = INT_BLEND(src[0], dest[0], a, t1);
	dest[1] = INT_BLEND(src[1], dest[1], a, t1);
	dest[2] = INT_BLEND(src[2], dest[2], a, t1);
    }
}

FImage *
composite(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	composite(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
		  (int) (value * 255), xf - xi, A->channels(), B->channels());
    return A;
}

void
add(unsigned char *dest,
    const unsigned char *src,
    int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = 255;
	} else
	    a = opacity;
	dest[0] =
	    INT_BLEND(clamp_upper(src[0] + dest[0], 255), dest[0], a, t1);
	dest[1] =
	    INT_BLEND(clamp_upper(src[1] + dest[1], 255), dest[1], a, t1);
	dest[2] =
	    INT_BLEND(clamp_upper(src[2] + dest[2], 255), dest[2], a, t1);
    }
}

FImage *
add(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	add(*(*A) (xi, row), *(*B) (xi - xo, row - yo), (int) (value * 255),
	    xf - xi, A->channels(), B->channels());
    return A;
}

void
subtract(unsigned char *dest,
	 const unsigned char *src,
	 int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = 255;
	} else
	    a = opacity;
	dest[0] = INT_BLEND(clamp_lower(dest[0] - src[0], 0), dest[0], a, t1);
	dest[1] = INT_BLEND(clamp_lower(dest[1] - src[1], 0), dest[1], a, t1);
	dest[2] = INT_BLEND(clamp_lower(dest[2] - src[2], 0), dest[2], a, t1);
    }
}

FImage *
subtract(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	subtract(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
		 (int) (value * 255), xf - xi, A->channels(), B->channels());
    return A;
}

void
difference(unsigned char *dest,
	   const unsigned char *src,
	   int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = 255;
	} else
	    a = opacity;
	dest[0] =
	    INT_BLEND(max(dest[0], src[0]) - min(dest[0], src[0]), dest[0], a,
		      t1);
	dest[1] =
	    INT_BLEND(max(dest[1], src[1]) - min(dest[1], src[1]), dest[1], a,
		      t1);
	dest[2] =
	    INT_BLEND(max(dest[2], src[2]) - min(dest[2], src[2]), dest[2], a,
		      t1);
    }
}

FImage *
difference(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	difference(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
		   (int) (value * 255), xf - xi, A->channels(),
		   B->channels());
    return A;
}

void
lighten_only(unsigned char *dest,
	     const unsigned char *src,
	     int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = 255;
	} else
	    a = opacity;
	dest[0] = INT_BLEND(max(dest[0], src[0]), dest[0], a, t1);
	dest[1] = INT_BLEND(max(dest[1], src[1]), dest[1], a, t1);
	dest[2] = INT_BLEND(max(dest[2], src[2]), dest[2], a, t1);
    }
}

FImage *
lighten_only(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	lighten_only(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
		     (int) (value * 255), xf - xi, A->channels(),
		     B->channels());
    return A;
}

void
darken_only(unsigned char *dest,
	    const unsigned char *src,
	    int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = 255;
	} else
	    a = opacity;
	dest[0] = INT_BLEND(min(dest[0], src[0]), dest[0], a, t1);
	dest[1] = INT_BLEND(min(dest[1], src[1]), dest[1], a, t1);
	dest[2] = INT_BLEND(min(dest[2], src[2]), dest[2], a, t1);
    }
}

FImage *
darken_only(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	darken_only(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
		    (int) (value * 255), xf - xi, A->channels(),
		    B->channels());
    return A;
}

void
divide(unsigned char *dest,
       const unsigned char *src,
       int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = min(dest[3], src[3]);
	} else
	    a = opacity;
	dest[0] =
	    INT_BLEND(min((dest[0] * 256) / (1 + src[0]), 255), dest[0], a,
		      t1);
	dest[1] =
	    INT_BLEND(min((dest[1] * 256) / (1 + src[1]), 255), dest[1], a,
		      t1);
	dest[2] =
	    INT_BLEND(min((dest[2] * 256) / (1 + src[2]), 255), dest[2], a,
		      t1);
    }
}

FImage *
divide(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	divide(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
	       (int) (value * 255), xf - xi, A->channels(), B->channels());
    return A;
}

void
multiply(unsigned char *dest,
	 const unsigned char *src,
	 int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = min(dest[3], src[3]);
	} else
	    a = opacity;
	dest[0] = INT_BLEND(INT_MULT(src[0], dest[0], t1), dest[0], a, t1);
	dest[1] = INT_BLEND(INT_MULT(src[1], dest[1], t1), dest[1], a, t1);
	dest[2] = INT_BLEND(INT_MULT(src[2], dest[2], t1), dest[2], a, t1);
    }
}

FImage *
multiply(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	multiply(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
		 (int) (value * 255), xf - xi, A->channels(), B->channels());
    return A;
}

void
screen(unsigned char *dest,
       const unsigned char *src,
       int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = min(dest[3], src[3]);
	} else
	    a = opacity;
	dest[0] =
	    INT_BLEND(255 - INT_MULT(255 - src[0], 255 - dest[0], t1),
		      dest[0], a, t1);
	dest[1] =
	    INT_BLEND(255 - INT_MULT(255 - src[1], 255 - dest[1], t1),
		      dest[1], a, t1);
	dest[2] =
	    INT_BLEND(255 - INT_MULT(255 - src[2], 255 - dest[2], t1),
		      dest[2], a, t1);
    }
}

FImage *
screen(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	screen(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
	       (int) (value * 255), xf - xi, A->channels(), B->channels());
    return A;
}

void
overlay(unsigned char *dest,
	const unsigned char *src,
	int opacity, int length, int dest_channels, int src_channels)
{
    int a;
    register long t1;
    unsigned char *end = dest + dest_channels * length;
    for (; dest < end; dest += dest_channels, src += src_channels) {
	if (src_channels > 3) {
	    a = INT_MULT(src[3], opacity, t1);
	    if (dest_channels > 3)
		dest[3] = min(dest[3], src[3]);
	} else
	    a = opacity;
	dest[0] =
	    INT_BLEND(INT_MULT
		      (dest[0],
		       dest[0] + INT_MULT(2 * src[0], 255 - dest[0], t1), t1),
		      dest[0], a, t1);
	dest[1] =
	    INT_BLEND(INT_MULT
		      (dest[1],
		       dest[0] + INT_MULT(2 * src[1], 255 - dest[1], t1), t1),
		      dest[1], a, t1);
	dest[2] =
	    INT_BLEND(INT_MULT
		      (dest[2],
		       dest[0] + INT_MULT(2 * src[2], 255 - dest[2], t1), t1),
		      dest[2], a, t1);
    }
}

FImage *
overlay(FImage * A, FImage * B, int xo, int yo, float value)
{
    int xi = max(xo, 0);
    int yi = max(yo, 0);
    int xf = min(max(xo + B->width(), 0), A->width());
    int yf = min(max(yo + B->height(), 0), A->height());

    for (int row = yi; row < yf; row++)
	overlay(*(*A) (xi, row), *(*B) (xi - xo, row - yo),
		(int) (value * 255), xf - xi, A->channels(), B->channels());
    return A;
}
