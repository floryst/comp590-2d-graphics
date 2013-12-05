/*
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GContext0_DEFINED
#define GContext0_DEFINED

#include <stack>
#include <algorithm>
#include "GContext.h"
#include "GBitmap.h"
#include "GPaint.h"
#include "GRect.h"
#include "GTransform.h"
#include "GUtils.h"
#include "GEdge.h"
#include "GBlitter.h"

#define clamp(val,lo,hi)	val < lo ? lo : val > hi ? hi : val

/**
 * Sorts edges based on y.
 */
int EdgeComparator(const void* edge1, const void* edge2) {
	const GEdge* e1 = reinterpret_cast<const GEdge*>(edge1);
	const GEdge* e2 = reinterpret_cast<const GEdge*>(edge2);
	if (e1->topY < e2->topY) return -1;
	return 1;
}

static void clear_bitmap(const GBitmap& bitmap, const GColor& color) {
	GPixel pixel = ColorToPixel(color);

	int x, y;
	int bmHeight = bitmap.fHeight;
	int bmWidth = bitmap.fWidth;
	int bmRowBytes = bitmap.fRowBytes;

	// Optimize for full-sized images
	if (bmWidth * 4 == bmRowBytes) {
		long bmArea = bmHeight * bmWidth;
		GPixel* bmPixels = bitmap.fPixels;
		GPixel* end = bmPixels + bmArea;
		while (bmPixels != end)
			*(bmPixels++) = pixel;
	}
	// Optimize for well aligned (to sizeof(GPixel)) images
	else if (bmRowBytes % sizeof(GPixel) == 0) {
		GPixel* bmPixels = bitmap.fPixels;
		int bmRowPixels = bmRowBytes / sizeof(GPixel);
		for (; bmHeight > 0; bmHeight--) {
			for (x = 0; x < bmWidth; x++) {
				*(bmPixels + x) = pixel;
			}
			bmPixels += bmRowPixels;
		}
	}
	// Worst case, image is not well aligned
	else {
		char* bmPixels = reinterpret_cast<char*>(bitmap.fPixels);
		for (; bmHeight > 0; bmHeight--) {
			GPixel* bmoffset = reinterpret_cast<GPixel*>(bmPixels);
			for (x = 0; x < bmWidth; x++) {
				*(bmoffset + x) = pixel;
			}
			bmPixels += bmRowBytes;
		}
	}
}

static void src_over_bitmap(const GBitmap& bitmap, const GColor& color) {
	GPixel pixel = ColorToPixel(color);
	
	int x, y;
	int bmHeight = bitmap.fHeight;
	int bmWidth = bitmap.fWidth;
	int bmRowBytes = bitmap.fRowBytes;

	// Optimize for well aligned (to sizeof(GPixel)) images
	if (bmRowBytes % sizeof(GPixel) == 0) {
		GPixel* bmPixels = bitmap.fPixels;
		int bmRowPixels = bmRowBytes / sizeof(GPixel);
		for (; bmHeight > 0; bmHeight--) {
			for (x = 0; x < bmWidth; x++) {
				apply_src_over(bmPixels + x, pixel);
			}
			bmPixels += bmRowPixels;
		}
	}
	// Worst case, image is not well aligned
	else {
		char* bmPixels = reinterpret_cast<char*>(bitmap.fPixels);
		for (; bmHeight > 0; bmHeight--) {
			GPixel* bmoffset = reinterpret_cast<GPixel*>(bmPixels);
			for (x = 0; x < bmWidth; x++) {
				apply_src_over(bmoffset + x, pixel);
			}
			bmPixels += bmRowBytes;
		}
	}
}

class GContext0 : public GContext {
public:

	GContext0(const GBitmap& bitmap, GPixel* pix=NULL) {
		this->pixref = pix;
		this->bitmap = bitmap;
	}

	~GContext0() {
		delete[] this->pixref;
	}

	void getBitmap(GBitmap* bitmap) const {
		*bitmap = this->bitmap;
	}

	void clear(const GColor& color) {
		clear_bitmap(this->bitmap, color);
	}

	void drawRect(const GRect& rect, const GPaint& paint) {
		if (paint.getAlpha() <= 0)
			return;

		GBitmap subBitmap;
		GRect tRect = this->ctm.map(rect);
		if (!this->bitmap.extractSubset(ClampRect(tRect), &subBitmap))
			return;

		float alpha = GPinToUnitFloat(paint.getAlpha());
		if (1 == alpha) {
			// opaque color
			clear_bitmap(subBitmap, paint.getColor());
		}
		else {
			// alpha blending with src_over
			src_over_bitmap(subBitmap, paint.getColor());
		}
	}

	void drawBitmap(const GBitmap& srcBitmap, float x, float y, const GPaint& paint) {
		float alpha = paint.getAlpha();
		if (alpha <= 0)
			return;
		alpha = GPinToUnitFloat(alpha);

		this->save();
		this->ctm.pretranslate(x, y);
		GRect srcRect = GRect::MakeWH(srcBitmap.fWidth, srcBitmap.fHeight);
		GRect srcTRect = this->ctm.map(srcRect);

		GRect intersection;
		if (!intersection.setIntersection(
				GRect::MakeWH(this->bitmap.fWidth, this->bitmap.fHeight), srcTRect)) {
			this->restore();
			return;
		}
		GIRect srcIRect = ClampRect(intersection);
		int width = srcIRect.width();
		int height = srcIRect.height();
		int dstX = srcIRect.fLeft;
		int dstY = srcIRect.fTop;

		char* srcPixels = reinterpret_cast<char*>(srcBitmap.fPixels);
		int srcRowBytes = srcBitmap.fRowBytes;
		char* dstPixels = reinterpret_cast<char*>(this->bitmap.fPixels);
		int dstRowBytes = this->bitmap.fRowBytes;
		GTransform invT = this->ctm.invert();

		int ix, iy;
		for (iy = dstY; iy < dstY + height; iy++) {
			GPixel* dst = reinterpret_cast<GPixel*>(dstPixels + dstRowBytes * iy);
			for (ix = dstX; ix < dstX + width; ix++) {
				GPoint pt = invT.map(ix + 0.5f, iy + 0.5f);
				int nx = (int)pt.x();
				int ny = (int)pt.y();
				// clamp to srcBitmap's width and height
				nx = nx < 0 ? 0 : (nx >= srcBitmap.fWidth ? srcBitmap.fWidth-1 : nx);
				ny = ny < 0 ? 0 : (ny >= srcBitmap.fHeight ? srcBitmap.fHeight-1 : ny);
				GPixel* src = reinterpret_cast<GPixel*>(srcPixels + srcRowBytes * ny) + nx;
				apply_src_over(dst+ix, *src, alpha);
			}
		}

		this->restore();
	}

	void drawConvexPolygon(const GPoint vertices[], int count, const GPaint& paint) {
		if (paint.getAlpha() <= 0)
			return;

		GRect polyRect;
		polyRect.setBounds(vertices, count);
		GRect bitmapRect = GRect::MakeWH(this->bitmap.fWidth, this->bitmap.fHeight);
		GRect polyTRect = this->ctm.map(polyRect);
		if (!polyTRect.intersects(bitmapRect))
			return;

		GRect clipBox;
		clipBox.setIntersection(polyTRect, bitmapRect);
		if (clipBox.isEmpty())
			return;
		
		GAutoArray<GEdge> edgeList(count);
		GEdge* edgeArray = edgeList.get();

		GPoint pt1 = this->ctm.map(vertices[0]);
		GPoint pt2;
		int i, edgeCount = 0;
		for (i = 0; i < count-1; i++) {
			pt2 = this->ctm.map(vertices[i+1]);
			GEdge edge(pt1, pt2);
			if (!edge.isHorizontal)
				edgeArray[edgeCount++] = edge;
			pt1 = pt2;
		}
		// Take care of first and last vertex connection.
		GEdge edge(pt1, this->ctm.map(vertices[0]));
		if (!edge.isHorizontal)
			edgeArray[edgeCount++] = edge;
		// optimize with custom sort for triangles.
		qsort(edgeArray, edgeCount, sizeof(GEdge), EdgeComparator);

		int yTop = Round(edgeArray[0].topY);
		int yBottom = Round(edgeArray[edgeCount-1].botY);
		int xLeft = clipBox.fLeft;
		int xRight = clipBox.fRight;

		GPixel pixel = ColorToPixel(paint.getColor());

		int rowBytes = this->bitmap.fRowBytes;
		char* bytePixels = reinterpret_cast<char*>(this->bitmap.fPixels);
		int low = 0;
		int high = this->bitmap.fHeight;

		GEdgeWalker walker1(*edgeArray, clipBox);
		GEdgeWalker walker2(*(++edgeArray), clipBox);
		int cnt = 2;
		while (yTop < yBottom && yTop < this->bitmap.fHeight) {
			if (yTop < 0) continue;

			int left = Round(walker1.fx);
			int right = Round(walker2.fx);
			if (left > right)
				std::swap(left, right);

			//printf("{ line: %i; %i -> %i }\n", yTop, left, right);

			GPixel* dst = reinterpret_cast<GPixel*>(bytePixels + yTop * rowBytes);
			while (left < right) {
				apply_src_over(dst+left, pixel);
				left++;
			}

			if (!walker1.step()) {
				walker1 = GEdgeWalker(*(++edgeArray), clipBox);
				cnt++;
			}
			if (!walker2.step()) {
				walker2 = GEdgeWalker(*(++edgeArray), clipBox);
				cnt++;
			}

			yTop++;
		}
	}
	
	void drawTriangle(const GPoint vertices[3], const GPaint& paint) {
		this->drawConvexPolygon(vertices, 3, paint);
	}

	void translate(float tx, float ty) {
		this->ctm.translate(tx, ty);
	}

	void scale(float sx, float sy) {
		this->ctm.scale(sx, sy);
	}

protected:
	void onSave() {
		GTransform saved = this->ctm.clone();
		this->tmStack.push(saved);
	}

	void onRestore() {
		this->ctm = this->tmStack.top();
		this->tmStack.pop();
	}

private:
	// our bitmap
	GBitmap bitmap;

	// used as a reference to clear the bitmap pixels.
	GPixel* pixref;

	// current transform matrix;
	GTransform ctm;

	// transform matrix stack
	std::stack<GTransform> tmStack;
};

GContext* GContext::Create(const GBitmap& bitmap) {
	// Some sanity checks. If people want to give a 
	// bad bitmap, then that's their problem.
	if (bitmap.fRowBytes < bitmap.fWidth * sizeof(GPixel) ||
		NULL == bitmap.fPixels ||
		bitmap.fHeight < 0)
		return NULL;
	return new GContext0(bitmap);
}

GContext* GContext::Create(int width, int height) {
	if (width <= 0 ||
		height <= 0 ||
		static_cast<long>(width) * height >= 1024 * 1024 * 1024)
		return NULL;

	GBitmap bitmap;
	bitmap.fWidth = width;
	bitmap.fHeight = height;
	bitmap.fPixels = new GPixel[width * height];
	bitmap.fRowBytes = width * sizeof(GPixel);

	return new GContext0(bitmap, bitmap.fPixels);
}

#endif
