/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GPointT_DEFINED
#define GPointT_DEFINED

#include "GTypes.h"

class GPoint {
public:
    float   fX;
    float   fY;
    
    float x() const { return fX; }
    float y() const { return fY; }
    
    void set(float x, float y) {
        fX = x;
        fY = y;
    }
};

#endif
