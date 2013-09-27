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

// Tests the intersection of two rectangles
static int intersect(const GIRect& rect1, const GIRect rect2, GIRect& intersect) {
	
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
	float res_a = src_a + transparency * GPixel_GetA(*dst);
	*dst = GPixel_PackARGB(
		ROUND(res_a),
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
		if (rect.width() < 0 ||
			rect.height() < 0 ||
			color.fA <= 0)
			return;

		// TODO intersect
		// clamping
		int rTop = rect.fTop < 0 ? 0 : rect.fTop;
		int rBottom = rect.fBottom > this->gbitmap.fHeight ?
			this->gbitmap.fHeight : rect.fBottom;
		int rLeft = rect.fLeft < 0 ? 0 : rect.fLeft;
		int rRight = rect.fRight > this->gbitmap.fWidth ?
			this->gbitmap.fWidth : rect.fRight;

		int rWidth = rect.width() * sizeof(GPixel);
		int bmRowBytes = this->gbitmap.fRowBytes;
		char* bmPixels = reinterpret_cast<char*>(this->gbitmap.fPixels);

		float alpha = pin_channel(color.fA);
		PremultColor pcolor = {
			alpha,
			pin_channel(color.fR) * alpha,
			pin_channel(color.fG) * alpha,
			pin_channel(color.fB) * alpha};

		int x, y;
		if (alpha == 1) {
			GPixel pixel = GPixel_PackARGB(
				FLOAT2INT(pcolor.A),
				FLOAT2INT(pcolor.R),
				FLOAT2INT(pcolor.G),
				FLOAT2INT(pcolor.B));

			for (y = rTop; y < rBottom; y++) {
				char* yoffset = bmPixels + y * bmRowBytes;
				for (x = rLeft; x < rRight; x++) {
					*reinterpret_cast<GPixel*>(yoffset + x * sizeof(GPixel)) = pixel;
				}
			}
		}
		else {
			for (y = rTop; y < rBottom; y++) {
				char* yoffset = bmPixels + y * bmRowBytes;
				for (x = rLeft; x < rRight; x++) {
					GPixel* dst = reinterpret_cast<GPixel*>(yoffset + x * sizeof(GPixel));
					apply_src_over(dst, pcolor);
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
		bitmap.fPixels == NULL ||
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
