/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include "GTransform.h"

void GTransform::preconcat(const GTransform& other) {
	float _a = a, _b = b, _c = c, _d = d, _e = e, _f = f;
	a = other.a * _a + other.b * _d;
	b = other.a * _b + other.b * _e;
	c = other.a * _c + other.b * _f + other.c;
	d = other.d * _a + other.e * _d;
	e = other.d * _b + other.e * _e;
	f = other.d * _c + other.e * _f + other.f;
}

void GTransform::transform(float &x, float &y) {
	float _x = x;
	float _y = y;
	x = a * _x + b * _y + c;
	y = d * _x + e * _y + f;

}

GTransform GTransform::invert() {
	// compute adjoint * 1/det
	float idet = a*e - b*d;
	idet = 1.0f / idet;

	GTransform t(
		idet * e,
		-idet * b,
		idet * (b*f - c*e),
		-idet * d,
		idet * a,
		-idet * (a*f - c*d));
	return t;
}
