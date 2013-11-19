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
	int topX, topY, botX, botY;
	bool isHorizontal;

	// When used as a type parameter, must have an empty constructor.
	GEdge() {}

	GEdge(const GPoint& p1, const GPoint& p2);
};

class GEdgeWalker {
public:
	int curX, curY;

	GEdgeWalker(const GEdge& e, const GRect& cb);

	bool step();

private:
	GEdge edge;
	GRect clipBox;
	int stepX;
	float dx, dy;
	float error;
};

#endif
