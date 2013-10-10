/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GPaint.h"

GPaint::GPaint() {
    fColor.set(1, 0, 0, 0);
//    fFilter = false;
}

GPaint::GPaint(const GPaint& src)
    : fColor(src.fColor)
//    , fFilter(src.fFilter)
{
}

GPaint::~GPaint() {
}

GPaint& GPaint::operator=(const GPaint& src) {
    fColor = src.fColor;
//    fFilter = src.fFilter;
    return *this;
}

void GPaint::setColor(const GColor& c) {
    fColor.fA = GPinToUnitFloat(c.fA);
    fColor.fR = GPinToUnitFloat(c.fR);
    fColor.fG = GPinToUnitFloat(c.fG);
    fColor.fB = GPinToUnitFloat(c.fB);
}

void GPaint::setAlpha(float alpha) {
    fColor.fA = GPinToUnitFloat(alpha);
}

