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
static float channel_clamp(float channel) {
	if (channel > 1.0f)
		return 1.0f;
	if (channel < 0.0f)
		return 0.0f;
	return channel;
}

// Pack already premult argb into GPixel
static GPixel inline pack_argb(int a, int r, int g, int b) {
	return 
		(a << GPIXEL_SHIFT_A) |
		(r << GPIXEL_SHIFT_R) |
		(g << GPIXEL_SHIFT_G) |
		(b << GPIXEL_SHIFT_B);
}

// Unpacks a given pixel into separate channels
static void inline unpack_argb(GPixel pixel, int& a, int& r, int& g, int& b) {
	a = (pixel >> GPIXEL_SHIFT_A) & 0xff;
	r = (pixel >> GPIXEL_SHIFT_R) & 0xff;
	g = (pixel >> GPIXEL_SHIFT_G) & 0xff;
	b = (pixel >> GPIXEL_SHIFT_B) & 0xff;
}

// Applies SRC_OVER given dest and src pixels
// Will write the composite pixel to *dst
// src pixel will be in GColor format.
static inline GPixel apply_src_over(GPixel* const dst, const PremultColor& color) {
	int dst_a, dst_r, dst_g, dst_b;
	float src_a, src_r, src_g, src_b;

	unpack_argb(*dst, dst_a, dst_r, dst_g, dst_b);
	// make src premult
	src_r = color.R * 255.0f;
	src_g = color.G * 255.0f;
	src_b = color.B * 255.0f;

	float transparency = 1.0f - color.A;
	src_a = color.A * 255.0f;
	float res_a = src_a + transparency * dst_a;
	*dst = pack_argb(
		ROUND(res_a),
		ROUND(src_r + transparency * dst_r),
		ROUND(src_g + transparency * dst_g),
		ROUND(src_b + transparency * dst_b));
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
		alpha = channel_clamp(color.fA);
		red = channel_clamp(color.fR);
		green = channel_clamp(color.fG);
		blue = channel_clamp(color.fB);

		// GColor is not in premult space, make it premult
		GPixel cpixel = pack_argb(
			FLOAT2INT(alpha),
			FLOAT2INT(red * alpha),
			FLOAT2INT(green * alpha),
			FLOAT2INT(blue * alpha));

		int x, y;
		int bmHeight = this->gbitmap.fHeight;
		int bmWidth = this->gbitmap.fWidth;
		int bmRowBytes = this->gbitmap.fRowBytes;
		char* bmPixels = reinterpret_cast<char*>(this->gbitmap.fPixels);
		for (y = 0; y < bmHeight; y++) {
			char* yoffset = bmPixels + y * bmRowBytes;
			for (x = 0; x < bmWidth; x++) {
				// TODO does the compiler optimize sizeof to a constant?
				*reinterpret_cast<GPixel*>(yoffset + x * sizeof(GPixel)) = cpixel;
			}
		}
	}

	void fillIRect(const GIRect& rect, const GColor& color) {
		if (rect.width() < 0 || rect.height() < 0)
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

		float alpha = channel_clamp(color.fA);
		PremultColor pcolor = {
			alpha,
			channel_clamp(color.fR) * alpha,
			channel_clamp(color.fG) * alpha,
			channel_clamp(color.fB) * alpha};

		int x, y;
		if (pcolor.A == 0)
			return;
		else if (pcolor.A == 1) {

			GPixel pixel = pack_argb(
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
		(long)width * height >= 1024 * 1024 * 1024)
		return NULL;

	GBitmap bitmap;
	bitmap.fWidth = width;
	bitmap.fHeight = height;
	bitmap.fPixels = new GPixel[width * height];
	bitmap.fRowBytes = width * sizeof(GPixel);

	return new GContext0(bitmap, bitmap.fPixels);
}

#endif
