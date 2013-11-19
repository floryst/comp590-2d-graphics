/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#include "GUtils.h"
#include "GPoint.h"
#include "GEdge.h"

GEdge::GEdge(const GPoint& p1, const GPoint& p2) {
	// Sort top and bottom points.
	if (p1.y() > p2.y()) {
		topX = edgeClamp(p2.x());
		topY = edgeClamp(p2.y());
		botX = edgeClamp(p1.x());
		botY = edgeClamp(p1.y());
	}
	else {
		topX = edgeClamp(p1.x());
		topY = edgeClamp(p1.y());
		botX = edgeClamp(p2.x());
		botY = edgeClamp(p2.y());
	}

	isHorizontal = (topY == botY);
}

GEdgeWalker::GEdgeWalker(const GEdge& e, const GRect& cb) {
	edge = e;
	clipBox = cb;

	curX = edge.topX;
	curY = edge.topY;

	// assign step and delta values
	if (edge.topX < edge.botX) {
		dx = edge.botX - edge.topX;
		stepX = 1;
	}
	else {
		dx = edge.topX - edge.botX;
		stepX = -1;
	}
	dy = edge.botY - edge.topY;

	// the error value
	error = dx - dy;
}

bool GEdgeWalker::step() {
	if (curY == edge.botY)
		return false;
	float derror = error*2;
	while (derror >= dx) {
		error -= dy;
		curX += stepX;
		derror = error*2;
	}

	error += dx;
	curY++;
	/*
	currentXFloat += edge.slope;
	currentX = (int)floorf(currentXFloat+0.5f);
	*/
	return true;
}
