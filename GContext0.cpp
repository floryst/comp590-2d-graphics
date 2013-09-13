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

class GContext0 : public GContext {
public:

	/*
	 * pix can either be NULL or a pointer to a GPixel array
	 * (for easy delete).
	 * It's preferable to use the factory Create() method, so 
	 * please don't use this method directly.
	 */
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

	/*
	 * The given GColor must have components 0<=x<=1
	 */
	void clear(const GColor& color) {
		float alpha, red, green, blue;

		// sanity checks and clamping
		alpha = (color.fA > 1.0f) ? 1.0f : ((color.fA < 0.0f) ? 0.0f : color.fA);
		red = (color.fR > 1.0f) ? 1.0f : ((color.fR < 0.0f) ? 0.0f : color.fR);
		green = (color.fG > 1.0f) ? 1.0f : ((color.fG < 0.0f) ? 0.0f : color.fG);
		blue = (color.fB > 1.0f) ? 1.0f : ((color.fB < 0.0f) ? 0.0f : color.fB);

		// GColor is not in premult space, make it premult
		GPixel cpixel = 
			((int)(alpha*255.0f+0.5f) << GPIXEL_SHIFT_A) |
			((int)(alpha*red*255.0f+0.5f) << GPIXEL_SHIFT_R) |
			((int)(alpha*green*255.0f+0.5f) << GPIXEL_SHIFT_G) |
			((int)(alpha*blue*255.0f+0.5f) << GPIXEL_SHIFT_B);

		int x,y;
		for (y=0; y < this->gbitmap.fHeight; y++) {
			for (x=0; x < this->gbitmap.fWidth; x++) {
				int byteOffset = y*this->gbitmap.fRowBytes+x*sizeof(GPixel);
				*((GPixel*)(((char*)this->gbitmap.fPixels)+byteOffset)) = cpixel;
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
	// Check that fRowBytes is not less than fWidth
	if (bitmap.fRowBytes < bitmap.fWidth * sizeof(GPixel))
		return NULL;
	// catch for null fpixels
	if (bitmap.fPixels == NULL)
		return NULL;
	return new GContext0(bitmap, NULL);
}

GContext* GContext::Create(int width, int height) {
	if (width <= 0 || height <= 0)
		return NULL;
	GBitmap bitmap;

	bitmap.fWidth = width;
	bitmap.fHeight = height;
	bitmap.fPixels = new GPixel[width * height];
	bitmap.fRowBytes = width * sizeof(GPixel);

	return new GContext0(bitmap, bitmap.fPixels);
}

#endif
