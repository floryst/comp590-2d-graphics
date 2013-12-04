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

	x0 = edge.topX;
	y0 = edge.topY;
	x1 = edge.botX;
	y1 = edge.botY;

	dx = x1 - x0;
	dy = y1 - y0;

	curX = 

	/*
	curY = floorf(y0+0.5f)+0.5f;
	curX = x0 + slope * (curY - y0);

	endY = floorf(y1-0.5f)+0.5f;
	endX = x1 + slope * (endY - y1);
	*/
}

bool GEdgeWalker::step() {
	if (fequals(curX, endX) || fequals(curY, endY))
		return false;
	
}
