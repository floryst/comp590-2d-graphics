/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GContext0_DEFINED
#define GContext0_DEFINED

#define ROUND(x) static_cast<int>((x) + 0.5f)
#define FLOAT2INT(x) ROUND((x)*255.0f)

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GIRect.h"

class GBitmap;
class GColor;
class GIRect;

typedef struct {
	float A, R, G, B;
} PremultColor;

// Clamps a given channel to normalized boundaries
static float inline pin_channel(float channel) {
	if (channel > 1.0f)
		return 1.0f;
	if (channel < 0.0f)
		return 0.0f;
	return channel;
}

// Tests the intersection of two rectangles.
// One rectangle has the top-left corner centered at the origin.
static int intersect(const GIRect rect, int width, int height, GIRect& rIntersect) {
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
// src pixel will be in GColor format.
static inline GPixel apply_src_over(GPixel* const dst, const PremultColor& color) {
	float src_a, src_r, src_g, src_b;

	// make src premult
	src_a = color.A * 255.0f;
	src_r = color.R * 255.0f;
	src_g = color.G * 255.0f;
	src_b = color.B * 255.0f;

	float transparency = 1.0f - color.A;
	*dst = GPixel_PackARGB(
		ROUND(src_a + transparency * GPixel_GetA(*dst)),
		ROUND(src_r + transparency * GPixel_GetR(*dst)),
		ROUND(src_g + transparency * GPixel_GetG(*dst)),
		ROUND(src_b + transparency * GPixel_GetB(*dst)));
}

class GContext0 : public GContext {
public:

	GContext0(const GBitmap& bitmap, GPixel* pix) {
		this->pixref = pix;
		this->gbitmap = bitmap;
	}

	~GContext0() {
		// delete[] will auto-check for null
		delete[] this->pixref;
	}

	void getBitmap(GBitmap* bitmap) const {
		*bitmap = this->gbitmap;
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
			FLOAT2INT(alpha),
			FLOAT2INT(red * alpha),
			FLOAT2INT(green * alpha),
			FLOAT2INT(blue * alpha));

		int x, y;
		int bmHeight = this->gbitmap.fHeight;
		int bmWidth = this->gbitmap.fWidth;
		int bmRowBytes = this->gbitmap.fRowBytes;

		// Optimize for full-sized images
		if (bmWidth * 4 == bmRowBytes) {
			long bmArea = bmHeight * bmWidth;
			GPixel* bmPixels = this->gbitmap.fPixels;
			int i;
			for (i = 0; i < bmArea; i++) {
				*(bmPixels++) = cpixel;
			}
		}
		// Optimize for well aligned (to sizeof(GPixel)) images
		else if (bmRowBytes % sizeof(GPixel) == 0) {
			GPixel* bmPixels = this->gbitmap.fPixels;
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
			char* bmPixels = reinterpret_cast<char*>(this->gbitmap.fPixels);
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
		if (0 == intersect(rect, this->gbitmap.fWidth, this->gbitmap.fHeight, irect))
			return;

		int rTop = irect.fTop;
		int rBottom = irect.fBottom;
		int rLeft = irect.fLeft;
		int rRight = irect.fRight;
		int rWidth = irect.width() * sizeof(GPixel);

		int bmRowBytes = this->gbitmap.fRowBytes;
		char* bmPixels = reinterpret_cast<char*>(this->gbitmap.fPixels);

		float alpha = pin_channel(color.fA);
		PremultColor pcolor = {
			alpha,
			pin_channel(color.fR) * alpha,
			pin_channel(color.fG) * alpha,
			pin_channel(color.fB) * alpha};

		int x, y;
		// opaque color doesn't need src_over math.
		if (1 == alpha) {
			GPixel pixel = GPixel_PackARGB(
				FLOAT2INT(pcolor.A),
				FLOAT2INT(pcolor.R),
				FLOAT2INT(pcolor.G),
				FLOAT2INT(pcolor.B));

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
					apply_src_over(yoffset + x, pcolor);
				}
			}
		}
	}

private:
	// our bitmap
	GBitmap gbitmap;

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
