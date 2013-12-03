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

	dx = abs(x1-x0);
	dy = -abs(y1-y0);
	sx = x0 < x1 ? 1 : -1;
	error = dx - 1;

	curX = x0;
	curY = y0;
}

bool GEdgeWalker::step() {
	while (true) {
		if (curX == x1 && curY == y1)
			return false;
		int e2 = 2*error;
		if (e2 >= dy) {
			error += dy;
			curX += sx;
		}
		if (e2 <= dx) {
			error += dx;
			++curY;
			return true;
		}
	}
}
