/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include <cstdlib>
#include <algorithm>
#include "GUtils.h"
#include "GPoint.h"
#include "GEdge.h"

GEdge::GEdge(const GPoint& p1, const GPoint& p2) {
	// Sort top and bottom points.
	if (p1.y() > p2.y()) {
		topX = Round(p2.x());
		topY = Round(p2.y());
		botX = Round(p1.x());
		botY = Round(p1.y());
	}
	else {
		topX = Round(p1.x());
		topY = Round(p1.y());
		botX = Round(p2.x());
		botY = Round(p2.y());
	}

	isHorizontal = (topY == botY);
}

GEdgeWalker::GEdgeWalker(const GEdge& e, const GRect& cb) {
	edge = e;
	clipBox = cb;

	x0 = edge.topX;
	y0 = edge.topY;
	x1 = edge.botX;
	y1 = edge.botY;

	steep = abs(y0-y1)>abs(x0-x1);
	if (steep) {
		std::swap(x0, y0);
		std::swap(x1, y1);
	}
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	dx = x1 - x0;
	dy = abs(y1-y0);
	error = dx / 2;
	y = y0;
	x = x0;
	if (y0 < y1)
		ystep = 1;
	else
		ystep = -1;
}

bool GEdgeWalker::step() {
	return true;
}
