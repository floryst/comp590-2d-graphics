/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include "GBlitter.h"
#include "GUtils.h"
#include "GPixel.h"
#include "GRect.h"

class GEdgeIterator {
public:
	GEdgeIterator(const GEdge& e, int sy, int ey) : 
			edge(e), startY(sy), endY(ey) {
		error = 0;
		dx = abs(e.topX - e.botX);
		dy = e.topY - e.botY;
		stepX = e.topX > e.botX ? -1 : 1;
		curX = e.topX;
		curY = e.topY;
	}
	
	bool step() {
		while (2 * (error - dy) <= dx) {
			error -= dy;
			curX += stepX;
		}
		error++;
		curY++;
	}
int getCurrentX() {
		return curX;
	}

private:
	GEdge edge;
	int startY, endY;
	int error;
	int dx, dy;
	int stepX;
	int curX, curY;
};

///////////////////////////////////////////////////////////////////////////////

void GColorBlitter::BlitHorizontal(int x, int y, int count, GPixel pixel) {
	GPixel* dst = this->bitmap.getAddr(x, y);
	while (count-- > 0)
		*dst++ = pixel;
}

void GColorBlitter::BlitRect(const GIRect& rect, GPixel pixel) {
	int x = rect.fLeft;
	int y = rect.fTop;

	// optimize for well-aligned bitmaps.
	if (bitmap.fRowBytes % sizeof(GPixel) == 0) {
		int rowPixels = bitmap.fRowBytes / sizeof(GPixel);
		GPixel* dst = bitmap.fPixels + rect.fTop * rowPixels;
		for (; y < rect.fBottom; y++) {
			for (; x < rect.fRight; x++)
				*(dst + x) = pixel;
			dst += rowPixels;
		}
	}
	// worst case with not well-aligned bitmap.
	else {
		char* dst = (char*)bitmap.fPixels + rect.fTop * bitmap.fRowBytes;
		for (; y < rect.fBottom; y++) {
			GPixel* dstOfPix = (GPixel*)dst;
			for (; x < rect.fRight; x++)
				*(dst + x) = pixel;
			dst += bitmap.fRowBytes;
		}

	}
}

void GColorBlitter::BlitEdgeArray(const GEdge* edges, const GIRect& clipBox) {
	
}

///////////////////////////////////////////////////////////////////////////////

/*
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
*/
