/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GColor_DEFINED
#define GColor_DEFINED

#include "GTypes.h"

/**
 *  Holds a nonpremultiplied color, with each component
 *  normalized to [0...1].
 */
class GColor {
public:
    float   fA, fR, fG, fB;
    
    void set(float a, float r, float g, float b) {
        fA = a;
        fR = r;
        fG = g;
        fB = b;
    }

    static GColor Make(float a, float r, float g, float b) {
        GColor c = { a, r, g, b };
        return c;
    }
};

#endif
