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

/**
 *  Compress 'bitmap' and write it to a new file specified by 'path'. If an
 *  error occurs, false is returned.
 */
bool GWriteBitmapToFile(const GBitmap& bitmap, const char path[]);

/**
 *  Decompress the image stored in 'path', and store the results in 'bitmap',
 *  allocating the memory for its pixels using malloc(). If the file cannot be
 *  decoded, 'bitmap' is ignored and false is returned.
 */
bool GReadBitmapFromFile(const char path[], GBitmap* bitmap);

#endif

