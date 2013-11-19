/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GUtils_DEFINED
#define GUtils_DEFINED

#include "GRect.h"
#include "GPixel.h"
#include "GColor.h"

/**
 * Epsilon value of 10^-5
 */
#define EPSILON	0.00001

/**
 * Rounding of strictly non-negative floats.
 */
#define roundp(x)	static_cast<int>((x) + 0.5f)

/**
 * Clamps a value based on standard fill conventions.
 */
#define edgeClamp(x)	static_cast<int>(ceilf((x)-0.5f))
//#define edgeClamp(x)	static_cast<int>(floorf((x)+0.5f))

/**
 * Checks to see if two floats are within some tolerance EPSILON.
 */
#define fequals(x,y) (fabs((x)-(y)) < EPSILON)

/**
 * Clamps rectangle to integer boundaries.
 */
static inline GIRect clampRect(const GRect& rect) {
	return GIRect::MakeLTRB(
		edgeClamp(rect.fLeft),
		edgeClamp(rect.fTop),
		edgeClamp(rect.fRight),
		edgeClamp(rect.fBottom));
}

/**
 * Returns a GPixel version of a GColor instance.
 */
static inline GPixel colorToPixel(const GColor& color) {
	float alpha = GPinToUnitFloat(color.fA);
	float red = GPinToUnitFloat(color.fR);
	float green = GPinToUnitFloat(color.fG);
	float blue = GPinToUnitFloat(color.fB);

	return GPixel_PackARGB(
		roundp(alpha * 255.0f),
		roundp(red * alpha * 255.0f),
		roundp(green * alpha * 255.0f),
		roundp(blue * alpha * 255.0f));
}

/**
 * Applies SRC_OVER given dest and src pixels
 * Will write the composite pixel to *dst
 * src pixel will be in GPixel format.
 */
static inline void apply_src_over(GPixel* const dst, const GPixel src, float alpha = 1.0f) {
	float src_a = GPixel_GetA(src);
	float transparency = 1.0f - (src_a * alpha) / 255.0f;
	*dst = GPixel_PackARGB(
		roundp(src_a * alpha + transparency * GPixel_GetA(*dst)),
		roundp(GPixel_GetR(src) * alpha + transparency * GPixel_GetR(*dst)),
		roundp(GPixel_GetG(src) * alpha + transparency * GPixel_GetG(*dst)),
		roundp(GPixel_GetB(src) * alpha + transparency * GPixel_GetB(*dst)));
}

#endif
