/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GContext0_DEFINED
#define GContext0_DEFINED

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GIRect.h"

class GBitmap;
class GColor;
class GIRect;

// Clamps a given float channel to rgb value
float rgb_clamp(float channel) {
	if (channel > 1.0f)
		return 1.0f;
	if (channel < 0.0f)
		return 0.0f;
	return channel;
}

// Converts float channel to rgb value
unsigned inline rgb_float2int(float channel) {
	return static_cast<unsigned>(channel * 255.0f + 0.5f);
}

// Pack premult argb into GPixel
GPixel inline pack_argb(float a, float r, float g, float b) {
	return 
		(rgb_float2int(a) << GPIXEL_SHIFT_A) |
		(rgb_float2int(r * a) << GPIXEL_SHIFT_R) |
		(rgb_float2int(g * a) << GPIXEL_SHIFT_G) |
		(rgb_float2int(b * a) << GPIXEL_SHIFT_B);
}

// Unpacks a given pixel into separate channels
void inline unpack_argb(GPixel pixel, float& a, float& r, float& g, float& b) {
	// TODO how to optimize this?
	a = (pixel >> GPIXEL_SHIFT_A) / 255.0f;
	r = ((pixel >> GPIXEL_SHIFT_R) & 0xff) / 255.0f;
	g = ((pixel >> GPIXEL_SHIFT_G) & 0xff) / 255.0f;
	b = ((pixel >> GPIXEL_SHIFT_B) & 0xff) / 255.0f;
}

// Applies SRC_OVER given dest and src pixels
// Will write the composite pixel to *dst
// src pixel will be in GColor format.
GPixel apply_src_over(GPixel* dst, GPixel src) {
	float dst_a, dst_r, dst_g, dst_b;
	float src_a, src_r, src_g, src_b;

	unpack_argb(*dst, dst_a, dst_r, dst_g, dst_b);
	unpack_argb(src, src_a, src_r, src_g, src_b);
	
	float transparency = 1.0f - src_a;
	*dst = pack_argb(
		src_a + transparency * dst_a,
		src_r + transparency * dst_r,
		src_g + transparency * dst_g,
		src_b + transparency * dst_b);
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
		alpha = rgb_clamp(color.fA);
		red = rgb_clamp(color.fR);
		green = rgb_clamp(color.fG);
		blue = rgb_clamp(color.fB);

		// GColor is not in premult space, make it premult
		GPixel cpixel = pack_argb(alpha, red, green, blue);

		int x, y;
		int height = this->gbitmap.fHeight;
		int width = this->gbitmap.fWidth;
		int rowBytes = this->gbitmap.fRowBytes;
		int gPixelSize = sizeof(GPixel);
		char* charPixels = reinterpret_cast<char*>(this->gbitmap.fPixels);
		for (y = 0; y < height; y++) {
			int byteOffset = y * rowBytes;
			char* charPixelsOffset = charPixels + byteOffset;
			for (x = 0; x < width; x++) {
				*reinterpret_cast<GPixel*>(charPixelsOffset + x * gPixelSize) = cpixel;
			}
		}
	}

	void fillIRect(const GIRect& rect, const GColor& color) {

		if (rect.width() < 0 || rect.height() < 0)
			return;

		int rTop= rect.fTop < 0 ? 0 : rect.fTop;
		int rBottom = rect.fBottom > this->gbitmap.fHeight ?
			this->gbitmap.fHeight : rect.fBottom;
		int rLeft = rect.fLeft < 0 ? 0 : rect.fLeft;
		int rRight = rect.fRight > this->gbitmap.fWidth ?
			this->gbitmap.fWidth : rect.fRight;
		int rWidth = rect.width() * sizeof(GPixel);

		int bmRowBytes = this->gbitmap.fRowBytes;
		char* bmPixels = reinterpret_cast<char*>(this->gbitmap.fPixels);
		GPixel src = pack_argb(
			rgb_clamp(color.fA),
			rgb_clamp(color.fR),
			rgb_clamp(color.fG),
			rgb_clamp(color.fB));


		int x, y;
		for (y = rTop; y < rBottom; y++) {
			char* yoffset = bmPixels + y * bmRowBytes;
			for (x = rLeft; x < rRight; x++) {
				GPixel* dst = reinterpret_cast<GPixel*>(yoffset + x * sizeof(GPixel));
				apply_src_over(dst, src);
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
