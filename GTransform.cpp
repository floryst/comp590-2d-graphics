/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include <algorithm>
#include <cmath>
#include "GTransform.h"
#include "GRect.h"

void GTransform::cat(const GTransform& other) {
	// compute translations before scales
	transX = other.scaleX * transX + other.transX;
	transY = other.scaleY * transY + other.transY;
	scaleX = other.scaleX * scaleX;
	scaleY = other.scaleY * scaleY;
}

void GTransform::translate(float tx, float ty) {
	transX += tx;
	transY += ty;
}

void GTransform::pretranslate(float tx, float ty) {
	transX += tx * scaleX;
	transY += ty * scaleY;
}

void GTransform::scale(float sx, float sy) {
	float offsetX = transX;
	float offsetY = transY;
	// move to origin
	this->translate(-offsetX, -offsetY);
	scaleX *= sx;
	scaleY *= sy;
	// move back
	this->translate(offsetX, offsetY);
}

GPoint GTransform::map(float x, float y) {
	struct GPoint point;
	point.set(scaleX * x + transX, scaleY * y + transY);
	return point;
}

GRect GTransform::map(const GRect& rect) {
	GPoint pt1 = this->map(rect.fLeft, rect.fTop);
	GPoint pt2 = this->map(rect.fRight, rect.fBottom);

	GRect r = GRect::MakeLTRB(pt1.x(), pt1.y(), pt2.x(), pt2.y());
	// preserve top-left and bottom-right coords
	r.sort();
	return r;
}

GTransform GTransform::invert() {
	// compute adjoint * 1/det
	double det = scaleX * scaleY;
	if (fabs(det) < EPSILON)
		// user should not give us invertible matrix.
		return GTransform();

	// Computing inverse is equivalent to scaling by (1/sx,1/sy)
	// and translating by (-tx/sx,-ty/sy).
	GTransform t;
	t.scale(1.0f / scaleX, 1.0f / scaleY);
	t.translate(-1.0f * transX / scaleX, -1.0f * transY / scaleY);
	return t;
}
