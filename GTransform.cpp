/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include <algorithm>
#include "GTransform.h"
#include "GRect.h"

void GTransform::cat(const GTransform& other) {
	float _a = a, _b = b, _c = c, _d = d, _e = e, _f = f;
	a = other.a * _a + other.b * _d;
	b = other.a * _b + other.b * _e;
	c = other.a * _c + other.b * _f + other.c;
	d = other.d * _a + other.e * _d;
	e = other.d * _b + other.e * _e;
	f = other.d * _c + other.e * _f + other.f;
}

void GTransform::translate(float tx, float ty) {
	c += tx;
	f += ty;
}

void GTransform::scale(float sx, float sy) {
	float offsetx = c;
	float offsety = f;
	// move to origin
	this->translate(-offsetx, -offsety);
	this->cat(GTransform::Create(sx, 0, 0, 0, sy, 0));
	// move back
	this->translate(offsetx, offsety);
}

GPoint<float> GTransform::transform(float x, float y) {
	struct GPoint<float> point;
	point.x = a*x + b*y + c;
	point.y = d*x + e*y + f;
	return point;
}

GRect GTransform::transform(const GRect& rect) {
	GPoint<float> pt1 = this->transform(rect.fLeft, rect.fTop);
	GPoint<float> pt2 = this->transform(rect.fRight, rect.fBottom);
	// preserve top-left and bottom-right coords
	GRect r = GRect::MakeLTRB(
		std::min(pt1.x, pt2.x),
		std::min(pt1.y, pt2.y),
		std::max(pt1.x, pt2.x),
		std::max(pt1.y, pt2.y));
	return r;
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

GTransform GTransform::Create(float a, float b, float c, float d, float e, float f) {
	GTransform t(a, b, c, d, e, f);
	return t;
}
