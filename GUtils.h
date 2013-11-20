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
 * Epsilon value of 10^-6
 */
#define EPSILON	0.000001

/**
 * Checks to see if two floats are within some tolerance EPSILON.
 */
#define fequals(x,y) (fabs((x)-(y)) < EPSILON)

/**
 * Rounds a float to nearest integer.
 */
static inline int Round(float num) {
	return (int)floorf(num + 0.5f);
}

/**
 * Clamps rectangle to integer boundaries.
 */
static inline GIRect ClampRect(const GRect& rect) {
	return GIRect::MakeLTRB(
		Round(rect.fLeft),
		Round(rect.fTop),
		Round(rect.fRight),
		Round(rect.fBottom));
}

/**
 * Returns a GPixel version of a GColor instance.
 */
static inline GPixel ColorToPixel(const GColor& color) {
	float alpha = GPinToUnitFloat(color.fA);
	float red = GPinToUnitFloat(color.fR);
	float green = GPinToUnitFloat(color.fG);
	float blue = GPinToUnitFloat(color.fB);

	return GPixel_PackARGB(
		Round(alpha * 255.0f),
		Round(red * alpha * 255.0f),
		Round(green * alpha * 255.0f),
		Round(blue * alpha * 255.0f));
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
		Round(src_a * alpha + transparency * GPixel_GetA(*dst)),
		Round(GPixel_GetR(src) * alpha + transparency * GPixel_GetR(*dst)),
		Round(GPixel_GetG(src) * alpha + transparency * GPixel_GetG(*dst)),
		Round(GPixel_GetB(src) * alpha + transparency * GPixel_GetB(*dst)));
}

#endif
