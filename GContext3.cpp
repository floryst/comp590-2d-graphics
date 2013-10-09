/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GContext0_DEFINED
#define GContext0_DEFINED

#define ROUND(x) static_cast<int>((x) + 0.5f)
#define SCALE255(x) ROUND((x)*255.0f)

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GIRect.h"

class GBitmap;
class GColor;
class GIRect;

// Clamps a given channel to normalized boundaries
static float inline pin_channel(float channel) {
	if (channel > 1.0f)
		return 1.0f;
	if (channel < 0.0f)
		return 0.0f;
	return channel;
}

static void inline memset32(GPixel* start, GPixel color, int size) {
	GPixel* p;
	GPixel* end = p + size;
	while (p != end)
		*(p++) = color;
}

// Tests the intersection of two rectangles.
// One rectangle has the top-left corner centered at the origin.
static int intersect(const GIRect& rect, int width, int height, GIRect& rIntersect) {
	int rTop = rect.fTop < 0 ? 0 : rect.fTop;
	int rLeft = rect.fLeft < 0 ? 0 : rect.fLeft;
	int rBottom = rect.fBottom > height ? height : rect.fBottom;
	int rRight = rect.fRight > width ? width : rect.fRight;

	if (rRight - rLeft <= 0 ||
		rBottom - rTop <= 0)
		return 0;
	
	// Create new rect
	rIntersect = GIRect::MakeLTRB(rLeft, rTop, rRight, rBottom);
	return 1;

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
		float alpha, red, green, blue;

		// clamping
		alpha = pin_channel(color.fA);
		red = pin_channel(color.fR);
		green = pin_channel(color.fG);
		blue = pin_channel(color.fB);

		// GColor is not in premult space, make it premult
		GPixel cpixel = GPixel_PackARGB(
			SCALE255(alpha),
			SCALE255(red * alpha),
			SCALE255(green * alpha),
			SCALE255(blue * alpha));

		int x, y;
		int bmHeight = this->bitmap.fHeight;
		int bmWidth = this->bitmap.fWidth;
		int bmRowBytes = this->bitmap.fRowBytes;

		// Optimize for full-sized images
		if (bmWidth * 4 == bmRowBytes) {
			long bmArea = bmHeight * bmWidth;
			GPixel* bmPixels = this->bitmap.fPixels;
			GPixel* end = bmPixels + bmArea;
			while (bmPixels != end)
				*(bmPixels++) = cpixel;
		}
		// Optimize for well aligned (to sizeof(GPixel)) images
		else if (bmRowBytes % sizeof(GPixel) == 0) {
			GPixel* bmPixels = this->bitmap.fPixels;
			int bmRowGPixels = bmRowBytes / sizeof(GPixel);
			for (y = 0; y < bmHeight; y++) {
				GPixel* yoffset = bmPixels + y * bmRowGPixels;
				for (x = 0; x < bmWidth; x++) {
					*(yoffset + x) = cpixel;
				}
			}
		}
		// Worst case, image is not well aligned
		else {
			char* bmPixels = reinterpret_cast<char*>(this->bitmap.fPixels);
			for (y = 0; y < bmHeight; y++) {
				GPixel* yoffset = reinterpret_cast<GPixel*>(bmPixels + y * bmRowBytes);
				for (x = 0; x < bmWidth; x++) {
					*(yoffset + x) = cpixel;
				}
			}
		}
	}

	void fillIRect(const GIRect& rect, const GColor& color) {
		if (color.fA <= 0)
			return;

		GIRect irect;
		if (0 == intersect(rect, this->bitmap.fWidth, this->bitmap.fHeight, irect))
			return;

		int rTop = irect.fTop;
		int rBottom = irect.fBottom;
		int rLeft = irect.fLeft;
		int rRight = irect.fRight;
		int rWidth = irect.width() * sizeof(GPixel);

		int bmRowBytes = this->bitmap.fRowBytes;
		char* bmPixels = reinterpret_cast<char*>(this->bitmap.fPixels);

		float alpha = pin_channel(color.fA);
		GPixel pixel = GPixel_PackARGB(
			SCALE255(alpha),
			SCALE255(color.fR * alpha),
			SCALE255(color.fG * alpha),
			SCALE255(color.fB * alpha));

		int x, y;
		// opaque color doesn't need src_over math.
		if (1 == alpha) {
			for (y = rTop; y < rBottom; y++) {
				GPixel* yoffset = reinterpret_cast<GPixel*>(bmPixels + y * bmRowBytes);
				for (x = rLeft; x < rRight; x++) {
					*(yoffset + x) = pixel;
				}
			}
		}
		// alpha blending with src_over.
		else {
			for (y = rTop; y < rBottom; y++) {
				GPixel* yoffset = reinterpret_cast<GPixel*>(bmPixels + y * bmRowBytes);
				for (x = rLeft; x < rRight; x++) {
					apply_src_over(yoffset + x, pixel);
				}
			}
		}
	}

	void drawBitmap(const GBitmap& srcBitmap, int x, int y, float alpha = 1) {
		if (alpha <= 0)
			return;

		alpha = pin_channel(alpha);
		// make srcBitmap anchored to the origin
		GIRect bmRect = GIRect::MakeXYWH(-x, -y, this->bitmap.fWidth, this->bitmap.fHeight);
		GIRect dstRect;
		if (0 == intersect(bmRect, srcBitmap.fWidth, srcBitmap.fHeight, dstRect))
			return;

		int offsetx = dstRect.x();
		int offsety = dstRect.y();
		int dstWidth = dstRect.width();
		int dstHeight = dstRect.height();
		int dstBmRowBytes = this->bitmap.fRowBytes;
		char* dstBmPixels = reinterpret_cast<char*>(this->bitmap.fPixels) +
			(offsety + y) * dstBmRowBytes +
			(offsetx + x) * sizeof(GPixel);
		int srcBmRowBytes = srcBitmap.fRowBytes;
		char* srcBmPixels = reinterpret_cast<char*>(srcBitmap.fPixels) +
			offsety * srcBmRowBytes +
			offsetx * sizeof(GPixel);

		for (y = 0; y < dstHeight; y++) {
			GPixel* dstYoffset = reinterpret_cast<GPixel*>(dstBmPixels + y * dstBmRowBytes);
			GPixel* srcYoffset = reinterpret_cast<GPixel*>(srcBmPixels + y * srcBmRowBytes);
			for (x = 0; x < dstWidth; x++) {
				apply_src_over(dstYoffset + x, *(srcYoffset + x), alpha);
			}
		}
	}

private:
	// our bitmap
	GBitmap bitmap;

	// used as a reference to clear the bitmap pixels.
	GPixel* pixref;
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
