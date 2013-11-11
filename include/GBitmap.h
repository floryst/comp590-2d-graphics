/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GBitmap_DEFINED
#define GBitmap_DEFINED

#include "GPixel.h"
#include "GRect.h"

class GBitmap {
public:
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    size_t rowBytes() const { return fRowBytes; }
    void* pixels() const { return fPixels; }

    GIRect asIRect() const {
        return GIRect::MakeWH(fWidth, fHeight);
    }

    GPixel* getAddr(int x, int y) const {
        GASSERT((unsigned)x < (unsigned)fWidth);
        GASSERT((unsigned)y < (unsigned)fHeight);
        return (GPixel*)((char*)fPixels + y * fRowBytes) + x;
    }

    /**
     *  Set dst bitmap to point to the subset of this bitmap, as specified by
     *  r. If the intersection of r with this bitmap is empty, return false
     *  and leave dst unchanged.
     *
     *  Note: this does not allocate new pixels for dst: it has dst point into
     *  the pixels referenced by this bitmap.
     */
    bool extractSubset(const GIRect& r, GBitmap* dst) const {
        GIRect subR;
        if (!subR.setIntersection(GIRect::MakeWH(fWidth, fHeight), r)) {
            return false;
        }
        dst->fWidth = subR.width();
        dst->fHeight = subR.height();
        dst->fRowBytes = fRowBytes;
        dst->fPixels = this->getAddr(subR.x(), subR.y());
        return true;
    }
    

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

