/**
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

// Basic rounding of only positive integers
#define ROUND(x) static_cast<int>((x) + 0.5f)
#define SCALE_255(x) ROUND((x) * 255.0f)

// Clamps a given channel to normalized boundaries
static inline float pin_channel(float channel) {
	if (channel > 1.0f)
		return 1.0f;
	if (channel < 0.0f)
		return 0.0f;
	return channel;
}

// Applies SRC_OVER given dest and src pixels
// Will write the composite pixel to *dst
// src pixel will be in GPixel format.
static inline GPixel apply_src_over(GPixel* const dst, const GPixel src, float alpha = 1.0f) {
	float src_a = GPixel_GetA(src);
	float transparency = 1.0f - (src_a * alpha) / 255.0f;
	*dst = GPixel_PackARGB(
		ROUND(src_a * alpha + transparency * GPixel_GetA(*dst)),
		ROUND(GPixel_GetR(src) * alpha + transparency * GPixel_GetR(*dst)),
		ROUND(GPixel_GetG(src) * alpha + transparency * GPixel_GetG(*dst)),
		ROUND(GPixel_GetB(src) * alpha + transparency * GPixel_GetB(*dst)));
}

static void clear_bitmap(const GBitmap& bitmap, const GColor& color) {
	float alpha, red, green, blue;
	// clamping
	alpha = pin_channel(color.fA);
	red = pin_channel(color.fR);
	green = pin_channel(color.fG);
	blue = pin_channel(color.fB);

	// GColor is not in premult space, make it premult
	GPixel cpixel = GPixel_PackARGB(
		SCALE_255(alpha),
		SCALE_255(red * alpha),
		SCALE_255(green * alpha),
		SCALE_255(blue * alpha));

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
			*(bmPixels++) = cpixel;
	}
	// Optimize for well aligned (to sizeof(GPixel)) images
	else if (bmRowBytes % sizeof(GPixel) == 0) {
		GPixel* bmPixels = bitmap.fPixels;
		int bmRowPixels = bmRowBytes / sizeof(GPixel);
		for (; bmHeight > 0; bmHeight--) {
			for (x = 0; x < bmWidth; x++) {
				*(bmPixels + x) = cpixel;
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
				*(bmoffset + x) = cpixel;
			}
			bmPixels += bmRowBytes;
		}
	}
}

static void src_over_bitmap(const GBitmap& bitmap, const GColor& color) {
	// clamping
	float alpha, red, green, blue;
	alpha = pin_channel(color.fA);
	red = pin_channel(color.fR);
	green = pin_channel(color.fG);
	blue = pin_channel(color.fB);

	GPixel pixel = GPixel_PackARGB(
		SCALE_255(alpha),
		SCALE_255(red * alpha),
		SCALE_255(green * alpha),
		SCALE_255(blue * alpha));

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

	GContext0(const GBitmap& bitmap, GPixel* pix) {
		this->pixref = pix;
		this->bitmap = bitmap;
	}

	~GContext0() {
		// delete[] will auto-check for null
		delete[] this->pixref;
	}

	void getBitmap(GBitmap* bitmap) const {
		*bitmap = this->bitmap;
	}

	void clear(const GColor& color) {
		clear_bitmap(this->bitmap, color);
	}

	void drawRect(const GRect& rect, const GPaint& paint) {
		GColor color = paint.getColor();
		if (color.fA <= 0)
			return;

		GBitmap subBitmap;
		GRect trect = this->ctm.map(rect);
		if (!this->bitmap.extractSubset(trect.round(), &subBitmap))
			return;

		float alpha = pin_channel(color.fA);
		// opaque color
		if (1 == alpha) {
			clear_bitmap(subBitmap, color);
		}
		// alpha blending with src_over
		else {
			src_over_bitmap(subBitmap, color);
		}
	}

	void drawBitmap(const GBitmap& srcBitmap, float x, float y, const GPaint& paint) {
		float alpha = paint.getAlpha();
		if (alpha <= 0)
			return;
		alpha = pin_channel(alpha);

		GRect intersection;
		GRect srcRect = GRect::MakeWH(srcBitmap.fWidth, srcBitmap.fHeight);
		
		this->save();
		this->ctm.translate(x, y);
		GRect srcTRect = this->ctm.map(srcRect);
		
		if (!intersection.setIntersection(
				GRect::MakeWH(this->bitmap.fWidth, this->bitmap.fHeight), srcTRect)) {
			this->restore();
			return;
		}

		GIRect srcIRect = intersection.round();
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
			for (ix = dstX; ix < dstX + width; ix++) {
				GPoint pt = invT.map(ix + 0.5f, iy + 0.5f);
				int nx = (int)floorf(pt.x);
				int ny = (int)floorf(pt.y);
				GPixel* dst = 
						reinterpret_cast<GPixel*>(dstPixels + dstRowBytes * iy) + ix;
				GPixel* src =
						reinterpret_cast<GPixel*>(srcPixels + srcRowBytes * ny) + nx;
				apply_src_over(dst, *src, alpha);
			}
		}

		this->restore();

		/*
		float alpha = paint.getAlpha();
		if (alpha <= 0)
			return;
		alpha = pin_channel(alpha);

		GRect intersectRect;
		GRect srcRect = GRect::MakeWH(srcBitmap.fWidth, srcBitmap.fHeight);
		this->save();
		this->ctm.translate(x, y);
		GRect srcRectT = this->ctm.map(srcRect); 
		if (!intersectRect.setIntersection(
			GRect::MakeWH(this->bitmap.fWidth, this->bitmap.fHeight), srcRectT)) {
			this->restore();
			return;
		}
		GIRect irect = intersectRect.round();

		int rWidth = irect.width();
		int rHeight = irect.height();
		int dstx = irect.fLeft;
		int dsty = irect.fTop;

		char* srcBmPixels = reinterpret_cast<char*>(srcBitmap.fPixels);
		int srcBmRowBytes = srcBitmap.fRowBytes;
		char* dstBmPixels = reinterpret_cast<char*>(this->bitmap.fPixels);
		int dstBmRowBytes = this->bitmap.fRowBytes;

		GTransform invT = this->ctm.invert();

		int maxx = dstx + rWidth, maxy = dsty + rHeight;
		int ix, iy;
		for (iy = dsty; iy < maxy; iy++) {
			GPixel* dst = reinterpret_cast<GPixel*>(dstBmPixels + iy * dstBmRowBytes);
			for (ix = dstx; ix < maxx; ix++) {
				GPoint pt = invT.map(ix+0.5f, iy+0.5f);
				GPixel* src = reinterpret_cast<GPixel*>(srcBmPixels + GRoundToInt(pt.y)*srcBmRowBytes);
				apply_src_over(dst + ix, *(src + GRoundToInt(pt.x)), alpha);
			}
		}
		this->restore();

		*
		int maxx = dstx + rWidth, maxy = dsty + rHeight;
		int ix, iy;
		for (iy = dsty; iy < maxy; iy++) {
			GPixel* dst = reinterpret_cast<GPixel*>(dstBmPixels + iy * dstBmRowBytes);
			// Optimize by inlining transformation code
			float ty = invT.e*(iy+0.5f) + invT.f;
			// pull out the tx-x and put it here.
			float tx = invT.a*(dstx+0.5f)+invT.c - x;
			GPixel* src = reinterpret_cast<GPixel*>(srcBmPixels + static_cast<int>(ty-y)*srcBmRowBytes);
			for (ix = dstx; ix < maxx; ix++) {
				apply_src_over(dst + ix, *(src + static_cast<int>(tx)), alpha);
				tx += invT.a;
			}
		}*/
	}

	void drawTriangle(const GPoint vertices[3], const GPaint&) {
	}

	void drawConvexPolygon(const GPoint vertices[], int count, const GPaint&) {
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
	return new GContext0(bitmap, NULL);
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
