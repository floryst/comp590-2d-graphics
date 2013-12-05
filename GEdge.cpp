/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include <cstdlib>
#include <algorithm>
#include <cmath>
#include "GUtils.h"
#include "GPoint.h"
#include "GEdge.h"

GEdge::GEdge(const GPoint& p1, const GPoint& p2) {
	// Sort top and bottom points.
	if (p1.y() > p2.y()) {
		topX = p2.x();
		topY = p2.y();
		botX = p1.x();
		botY = p1.y();
	}
	else {
		topX = p1.x();
		topY = p1.y();
		botX = p2.x();
		botY = p2.y();
	}

	isHorizontal = (Round(topY) == Round(botY));
}

GEdgeWalker::GEdgeWalker(const GEdge& e, const GRect& cb) {
	edge = e;
	clipBox = cb;

	fx = edge.topX;
	fy = edge.topY;
	ex = edge.botX;
	ey = edge.botY;

	slope = (ex-fx) / (ey-fy);

	float dy = floorf(fy + 0.5f) + 0.5f - fy;
	fx += slope * dy;
}

bool GEdgeWalker::step(int curY) {
	fx += slope;
	if (slope > 0)
		return fx <= ex;
	if (curY + 1.5 >= ey)
		return false;
	return fx >= ex;

}
