/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GBitmap_DEFINED
#define GBitmap_DEFINED

#include "GPixel.h"

class GBitmap {
public:
    int     fWidth;     // number of pixels in a row
    int     fHeight;    // number of rows of pixels
    GPixel* fPixels;    // address of first (top) row of pixels
    size_t  fRowBytes;  // number of bytes between rows of pixels
};

#endif

