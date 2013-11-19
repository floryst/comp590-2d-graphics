/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GBlitter_DEFINED
#define GBlitter_DEFINED

#include "GBitmap.h"
#include "GPaint.h"
#include "GRect.h"
#include "GEdge.h"

class GBlitter {
public:
	static void blitConvexPolygon(const GBitmap& bitmap, const GEdge edges[], const GRect& clipBox, const GPaint& paint);

private:
	static void rowBlitOpaqueColor(GPixel* pixels, int count, GPixel pixel);
	static void rowBlitTransparentColor(GPixel* pixels, int count, GPixel pixel);
};

#endif
