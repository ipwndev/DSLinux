/*
 *  image_tmpl.cc
 */

#include "image.hh"

template class Image<Pixel>;
template class Image_RGB<Pixel>;
template class Image_YUV<Pixel>;
template class Bitmap<Pixel>;

template class Bitmap<bool>;
template class Bitmap<float>;

#include "image.cc"
#include "bitmap.cc"
