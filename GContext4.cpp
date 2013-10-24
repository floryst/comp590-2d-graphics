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

static inline GIRect pin_rect(const GRect& frect) {
	int 
		left = (int)frect.fLeft,
		right = (int)frect.fRight,
		bottom = (int)frect.fBottom,
		top = (int)frect.fTop;
	if (frect.fLeft - left > 0.5f)
		left++;
	if (frect.fTop - top > 0.5f)
		top++;
	if (frect.fRight - right > 0.5f)
		right++;
	if (frect.fBottom - bottom > 0.5f)
		bottom++;
	
	return GIRect::MakeLTRB(left, top, right, bottom);
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

		GRect trect = this->ctm.transform(rect);

		GBitmap subBitmap;
		if (!this->bitmap.extractSubset(trect.round(), &subBitmap))
			return;

		float alpha = pin_channel(color.fA);
		// opaque color
		if (1 == alpha) {
			clear_bitmap(subBitmap, color);
		}
		// alpha blending with src_over
		else {
			// clamping
			float red, green, blue;
			red = pin_channel(color.fR);
			green = pin_channel(color.fG);
			blue = pin_channel(color.fB);

			GPixel pixel = GPixel_PackARGB(
				SCALE_255(alpha),
				SCALE_255(red * alpha),
				SCALE_255(green * alpha),
				SCALE_255(blue * alpha));

			int x, y;
			int bmHeight = subBitmap.fHeight;
			int bmWidth = subBitmap.fWidth;
			int bmRowBytes = subBitmap.fRowBytes;

			// Optimize for well aligned (to sizeof(GPixel)) images
			if (bmRowBytes % sizeof(GPixel) == 0) {
				GPixel* bmPixels = subBitmap.fPixels;
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
				char* bmPixels = reinterpret_cast<char*>(subBitmap.fPixels);
				for (; bmHeight > 0; bmHeight--) {
					GPixel* bmoffset = reinterpret_cast<GPixel*>(bmPixels);
					for (x = 0; x < bmWidth; x++) {
						apply_src_over(bmoffset + x, pixel);
					}
					bmPixels += bmRowBytes;
				}
			}
		}
	}

	void drawBitmap(const GBitmap& srcBitmap, float x, float y, const GPaint& paint) {
		float alpha = paint.getAlpha();
		if (alpha <= 0)
			return;
		alpha = pin_channel(alpha);

		GRect srcBitmapRect = GRect::MakeLTRB(x, y, x+srcBitmap.fWidth, y+srcBitmap.fHeight);
		GRect trect = this->ctm.transform(srcBitmapRect);

		GRect intrect;
		if (!intrect.setIntersection(GRect::MakeWH(this->bitmap.fWidth, this->bitmap.fHeight), trect))
			return;
		GIRect irect = intrect.round();

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
			// Optimize by inlining transformation code
			float ty = invT.e*(iy+0.5f) + invT.f;
			// pull out the tx-x and put it here.
			float tx = invT.a*(dstx+0.5f)+invT.c - x;
			GPixel* src = reinterpret_cast<GPixel*>(srcBmPixels + static_cast<int>(ty-y)*srcBmRowBytes);
			for (ix = dstx; ix < maxx; ix++) {
				apply_src_over(dst + ix, *(src + static_cast<int>(tx)), alpha);
				tx += invT.a;
			}
		}
	}

	void translate(float tx, float ty) {
		GTransform translate(1, 0, tx, 0, 1, ty);
		this->ctm.preconcat(translate);
	}

	void scale(float sx, float sy) {
		// move to origin
		float offsetx = this->ctm.c;
		float offsety = this->ctm.f;
		this->ctm.preconcat(GTransform::Create(1, 0, -offsetx, 0, 1, -offsety));
		// scale
		this->ctm.preconcat(GTransform::Create(sx, 0, 0, 0, sy, 0));
		// move back
		this->ctm.preconcat(GTransform::Create(1, 0, offsetx, 0, 1, offsety));
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
