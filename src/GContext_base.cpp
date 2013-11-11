/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GContext.h"
#include "GPoint.h"

GContext::GContext() : fSaveCount(0) {}

GContext::~GContext() {}

void GContext::save() {
    fSaveCount += 1;
    this->onSave();
}

void GContext::restore() {
    if (fSaveCount > 0) {
        this->onRestore();
        fSaveCount -= 1;
    }
}

void GContext::restoreToCount(int count) {
    if (count < 0) {
        count = 0;
    }
    while (this->getSaveCount() > count) {
        this->restore();
    }
}

void GContext::drawConvexPolygon(const GPoint vertices[], int count,
                                 const GPaint& paint) {
    GPoint tri[3];
    tri[0] = vertices[0];
    tri[1] = vertices[1];
    for (int i = 2; i < count; ++i) {
        tri[2] = vertices[i];
        this->drawTriangle(tri, paint);
        tri[1] = vertices[i];
    }
}


