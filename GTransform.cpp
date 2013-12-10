/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include <cmath>
#include "GTransform.h"
#include "GRect.h"
#include "GUtils.h"

void GTransform::translate(float tx, float ty) {
	transX += tx;
	transY += ty;
}

void GTransform::pretranslate(float tx, float ty) {
	transX += tx * scaleX + ty * rotX;
	transY += tx * rotY + ty * scaleY;
}

void GTransform::scale(float sx, float sy) {
	scaleX *= sx;
	scaleY *= sy;
}

void GTransform::rotate(float radians) {
	float _scaleX = scaleX * cosf(radians) - rotY * sinf(radians);
	float _scaleY = rotX * sinf(radians) + scaleY * cosf(radians);
	float _rotX = rotX * cosf(radians) - scaleY * sinf(radians);
	float _rotY = scaleX * sinf(radians) + rotY * cosf(radians);

	scaleX = _scaleX;
	scaleY = _scaleY;
	rotX = _rotX;
	rotY = _rotY;
}

GPoint GTransform::map(float x, float y) {
	struct GPoint point;
	point.set(scaleX * x + rotX * y + transX, rotY * x + scaleY * y + transY);
	return point;
}

GPoint GTransform::map(const GPoint& point) {
	return this->map(point.x(), point.y());
}

GRect GTransform::map(const GRect& rect) {
	GPoint pt1 = this->map(rect.fLeft, rect.fTop);
	GPoint pt2 = this->map(rect.fRight, rect.fBottom);

	GRect r = GRect::MakeLTRB(pt1.x(), pt1.y(), pt2.x(), pt2.y());
	r.sort();
	return r;
}

GTransform GTransform::invert() {
	// compute adjoint * 1/det
	double det = scaleX * scaleY - rotX * rotY;
	if (fequals(det, 0.0f))
		// user should not give us invertible matrix.
		return GTransform();

	GTransform t;
	t.scaleX = scaleY / det;
	t.scaleY = scaleX / det;
	t.rotX = -rotX / det;
	t.rotY = -rotY / det;
	t.transX = (rotX * transY - transX * scaleY) / det;
	t.transY = (rotY * transX - transY * scaleX) / det;
	return t;
}
