/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GBlitter_DEFINED
#define GBlitter_DEFINED

#include "GBitmap.h"
#include "GPixel.h"
#include "GRect.h"
#include "GEdge.h"

class GBlitter {
public:
	//static void blitConvexPolygon(const GBitmap& bitmap, const GEdge edges[], const GRect& clipBox, const GPaint& paint);

	GBlitter();

	virtual void BlitHorizontal(int x, int y, int count) = 0;

	virtual void BlitRect(const GIRect& rect) = 0;

	/**
	 * Blits scanlines between edges of a convex polygon specified
	 * wih the edges array. Edges are assumed to be sorted with 
	 * top-most Y-values first.
	 */
	virtual void BlitEdgeArray(const GEdge* edges, const GIRect& clipBox) = 0;

/*
private:
	static void rowBlitOpaqueColor(GPixel* pixels, int count, GPixel pixel);
	static void rowBlitTransparentColor(GPixel* pixels, int count, GPixel pixel);
*/

};

class GColorBlitter : public GBlitter {
public:
	GColorBlitter(const GBitmap& bm) : bitmap(bm) {}
	
	void BlitHorizontal(int x, int y, int count, GPixel pixel);
	void BlitRect(const GIRect& rect, GPixel pixel);
	void BlitEdgeArray(const GEdge* edges, const GIRect& clipBox);

protected:
	GBitmap bitmap;
};

#endif
