/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GTransform_DEFINED
#define GTransform_DEFINED

#include "GRect.h"

#define EPSILON	0.00000001

struct GPoint {
	float  x, y;
};

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
	 * Concatenates the current matrix with given matrix.
	 *  (other) * (this)
	 */
	void precat(const GTransform& other);

	/**
	 * Translates a matrix
	 */
	void translate(float tx, float ty);

	/**
	 * Scales a matrix.
	 */
	void scale(float sx, float sy);

	/**
	 * Perform a linear transformation on the point (x,y).
	 */
	GPoint map(float x, float y);

	/**
	 * Performs a linear transformation on a rectangle.
	 */
	GRect map(const GRect& rect);

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
