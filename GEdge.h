/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GEdge_DEFINED
#define GEdge_DEFINED

#include <algorithm>
#include "GUtils.h"
#include "GPoint.h"

class GEdge {
public:
	float topX, topY, botX, botY;
	bool isHorizontal;

	// When used as a type parameter, must have an empty constructor.
	GEdge() {}

	GEdge(const GPoint& p1, const GPoint& p2);
};

class GEdgeWalker {
public:
	float fx, fy;

	GEdgeWalker(const GEdge& e, const GRect& cb);

	bool step(int curY);

private:
	GEdge edge;
	GRect clipBox;

	float ex, ey;
	float slope;
};

#endif
