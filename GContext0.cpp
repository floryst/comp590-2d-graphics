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

class GBitmap;
class GColor;

// Clamps a given float channel to rgb value
float rgb_clamp(float channel) {
	if (channel > 1.0f)
		return 1.0f;
	if (channel < 0.0f)
		return 0.0f;
	return channel;
}

// Converts float channel to rgb value
int inline rgb_float2int(float channel) {
	return static_cast<int>(channel * 255.0f + 0.5f);
}

class GContext0 : public GContext {
public:

	GContext0(const GBitmap& bitmap, GPixel* pix) {
		this->pixref = pix;
		this->gbitmap = bitmap;
	}

	~GContext0() {
		if (this->pixref)
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
		GPixel cpixel = 
			(rgb_float2int(alpha) << GPIXEL_SHIFT_A) |
			(rgb_float2int(red * alpha) << GPIXEL_SHIFT_R) |
			(rgb_float2int(green * alpha) << GPIXEL_SHIFT_G) |
			(rgb_float2int(blue * alpha) << GPIXEL_SHIFT_B);

		int x, y;
		char* charPixels = reinterpret_cast<char*>(this->gbitmap.fPixels);
		for (y = 0; y < this->gbitmap.fHeight; y++) {
			int byteOffset = y * this->gbitmap.fRowBytes;
			char* charPixelsOffset = charPixels + byteOffset;
			for (x = 0; x < this->gbitmap.fWidth; x++) {
				*reinterpret_cast<GPixel*>(charPixelsOffset + x * sizeof(GPixel)) = cpixel;
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
	if (bitmap.fRowBytes < bitmap.fWidth * sizeof(GPixel) ||
		bitmap.fPixels == NULL)
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
