/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GTransform_DEFINED
#define GTransform_DEFINED

#include "GRect.h"

class GTransform {
public:
	float scaleX, scaleY, transX, transY;

	/**
	 * Create the identity matrix.
	 */
	GTransform() {
		scaleX = scaleY = 1;
		transX = transY = 0;
	}

	/**
	 * Applies a translation operation.
	 */
	void translate(float tx, float ty);

	/**
	 * Prepends a translation operation to current transform.
	 */
	void pretranslate(float tx, float ty);

	/**
	 * Scales a matrix.
	 */
	void scale(float sx, float sy);

	/**
	 * Perform a linear transformation on the point (x,y).
	 */
	GPoint map(float x, float y);

	/**
	 * Perform a linear transformation on the given point.
	 */
	GPoint map(const GPoint&);

	/**
	 * Performs a linear transformation on a rectangle.
	 */
	GRect map(const GRect&);

	/**
	 * Inverts the transform matrix.
	 */
	GTransform invert();

	/**
	 * Clones the current transform.
	 */
	GTransform clone() {
		GTransform t;
		t.scaleX = scaleX;
		t.scaleY = scaleY;
		t.transX = transX;
		t.transY = transY;
		return t;
	}

	/**
	 * Check to see if transform doesn't change scale.
	 */
	bool isOriginalScale() {
		return scaleX == 1 && scaleY == 1;
	}
};

#endif
