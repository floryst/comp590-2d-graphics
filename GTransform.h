/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GTransform_DEFINED
#define GTransform_DEFINED

class GTransform {
public:
	/*
	 * Represents a 3x3 transform matrix.
	 *  a b c
	 *  d e f
	 *  0 0 1
	 */
	float a, b, c, d, e, f;

	/**
	 * Create the identity matrix.
	 */
	GTransform() {
		a = e = 1;
		b = c = d = f = 0;
	}

	GTransform(float a, float b, float c, float d, float e, float f) :
		a(a), b(b), c(c), d(d), e(e), f(f) {}

	/**
	 * Preconcats the current matrix with given matrix.
	 *  (other) * (this)
	 */
	void preconcat(const GTransform& other);

	/**
	 * Perform a linear transformation on the point (x,y).
	 */
	void transform(float &x, float &y);

	/**
	 * Inverts the transform matrix.
	 */
	GTransform invert();

	/**
	 * Clones the current transform.
	 */
	GTransform clone() {
		GTransform t(a, b, c, d, e, f);
		return t;
	}
};

#endif
