/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include "GBlitter.h"
#include "GUtils.h"
#include "GPaint.h"
#include "GRect.h"

void GBlitter::blitConvexPolygon(const GBitmap& bitmap, const GEdge edges[], const GRect& clipBox, const GPaint& paint) {

	GPixel pixel = ColorToPixel(paint.getColor());

	int yTop = Round(clipBox.fTop);
	int yBottom = Round(clipBox.fBottom);
	int xLeft = Round(clipBox.fLeft);
	int xRight = Round(clipBox.fRight);

	GEdgeWalker walker1(*edges, clipBox);
	GEdgeWalker walker2(*(++edges), clipBox);

	int rowBytes = bitmap.fRowBytes;
	char* bytePixels = reinterpret_cast<char*>(bitmap.fPixels) + rowBytes * yTop;

	void (*rowBlitterFunc)(GPixel* pixels, int count, GPixel srcPixel) =
		paint.getAlpha() >= 1 ?
			GBlitter::rowBlitOpaqueColor : GBlitter::rowBlitTransparentColor;

	while (yTop < yBottom) {

		int left = walker1.curX;
		int right = walker2.curX;
		if (left > right)
			// GSwap vs std::swap?
			std::swap(left, right);

		left = std::max(left, xLeft);
		right = std::min(right, xRight);

		printf("{ line: %i; [%i -> %i) }\n", yTop, left, right);

		GPixel* pixels = reinterpret_cast<GPixel*>(bytePixels) + left;
		rowBlitterFunc(pixels, right-left, pixel);
	
		if (!walker1.step())
			walker1 = GEdgeWalker(*(++edges), clipBox);
		if (!walker2.step())
			walker2 = GEdgeWalker(*(++edges), clipBox);

		++yTop;
		bytePixels += rowBytes;
	}
}

void GBlitter::rowBlitOpaqueColor(GPixel* pixels, int count, GPixel pixel) {
	int x;
	for (x = 0; x < count; x++)
		*(pixels+x) = pixel;
}

void GBlitter::rowBlitTransparentColor(GPixel* pixels, int count, GPixel pixel) {
	int x;
	for (x = 0; x < count; x++)
		apply_src_over(pixels+x, pixel);
}
