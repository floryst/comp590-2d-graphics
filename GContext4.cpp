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

#define ROUND(x) static_cast<int>((x) + 0.5f)
#define SCALE255(x) ROUND((x)*255.0f)

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

	void drawRect(const GRect& rect, const GPaint& paint) {
		GColor color = paint.getColor();
		if (color.fA <= 0)
			return;

		// Compute the transformed rectangle.
		float x1 = rect.fLeft;
		float y1 = rect.fTop;
		float x2 = rect.fRight;
		float y2 = rect.fBottom;
		
		this->ctm.transform(x1, y1);
		this->ctm.transform(x2, y2);
		GRect scaledRect = GRect::MakeLTRB(x1, y1, x2, y2);

		// Get rectangle intersection
		GRect irect = GRect::MakeEmpty();
		GRect bmRect = GRect::MakeWH(this->bitmap.fWidth, this->bitmap.fHeight);
		if (!irect.setIntersection(scaledRect, bmRect))
			return;

		GIRect girect = pin_rect(irect);

		int rTop = girect.fTop;
		int rBottom = girect.fBottom;
		int rLeft = girect.fLeft;
		int rRight = girect.fRight;
		int rWidth = girect.width() * sizeof(GPixel);

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

	void drawBitmap(const GBitmap& srcBitmap, float x, float y, const GPaint& paint) {
		float alpha = paint.getAlpha();
		if (alpha <= 0)
			return;
		alpha = pin_channel(alpha);

		float x1 = x;
		float y1 = y;
		float x2 = x + srcBitmap.fWidth;
		float y2 = y + srcBitmap.fHeight;
		this->ctm.transform(x1, y1);
		this->ctm.transform(x2, y2);
		GRect scaledRect = GRect::MakeLTRB(x1, y1, x2, y2);

		GRect irect = GRect::MakeEmpty();
		GRect bmRect = GRect::MakeWH(this->bitmap.fWidth, this->bitmap.fHeight);
		if (!irect.setIntersection(scaledRect, bmRect))
			return;

		GIRect girect = pin_rect(irect);
		
		int rWidth = girect.width();
		int rHeight = girect.height();
		int dstx = girect.fLeft;
		int dsty = girect.fTop;

		char* srcBmPixels = reinterpret_cast<char*>(srcBitmap.fPixels);
		int srcBmRowBytes = srcBitmap.fRowBytes;
		char* dstBmPixels = reinterpret_cast<char*>(this->bitmap.fPixels);
		int dstBmRowBytes = this->bitmap.fRowBytes;

		GTransform invT = this->ctm.invert();

		int maxx = dstx + rWidth, maxy = dsty + rHeight;
		int ix, iy;
		for (iy = dsty; iy < maxy; iy++) {
			GPixel* dst = reinterpret_cast<GPixel*>(dstBmPixels + iy * dstBmRowBytes);
			int c, d;
			c = invT.c;
			d = invT.e*(iy+0.5f) + invT.f;
			GPixel* src = reinterpret_cast<GPixel*>(srcBmPixels + ((int)(d-y))*srcBmRowBytes);
			for (ix = dstx; ix < maxx; ix++) {
				c += invT.a;
				apply_src_over(dst + ix, *(src+(int)(c-x)), alpha);
			}
		}
	}

	void translate(float tx, float ty) {
		GTransform translate(1, 0, tx, 0, 1, ty);
		this->ctm.preconcat(translate);
	}

	void scale(float sx, float sy) {
		GTransform scale(sx, 0, 0, 0, sy, 0);
		this->ctm.preconcat(scale);
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
