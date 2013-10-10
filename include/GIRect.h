/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GIRect_DEFINED
#define GIRect_DEFINED

#include "GTypes.h"

class GIRect {
public:
    int32_t fLeft;
    int32_t fTop;
    int32_t fRight;
    int32_t fBottom;

    int32_t x() const { return fLeft; }
    int32_t y() const { return fTop; }
    int32_t width() const { return fRight - fLeft; }
    int32_t height() const { return fBottom - fTop; }

    bool isEmpty() const {
        return fLeft >= fRight || fTop >= fBottom;
    }
    
    void setEmpty() {
        fLeft = fTop = fRight = fBottom = 0;
    }

    void setLTRB(int l, int t, int r, int b) {
        fLeft = l;
        fTop = t;
        fRight = r;
        fBottom = b;
    }
    
    void setXYWH(int x, int y, int w, int h) {
        fLeft = x;
        fTop = y;
        fRight = x + w;
        fBottom = y + h;
    }
    
    void offset(int dx, int dy) {
        fLeft += dx;
        fTop += dy;
        fRight += dx;
        fBottom += dy;
    }
    
    void inset(int dx, int dy) {
        fLeft += dx;
        fTop += dy;
        fRight -= dx;
        fBottom -= dy;
    }
    
    bool contains(int x, int y) const {
        return fLeft <= x && x < fRight && fTop <= y && y < fBottom;
    }

    ///////////////////////////////////////////////////////////////////////////

    static GIRect MakeEmpty() {
        GIRect r;
        r.setEmpty();
        return r;
    }

    static GIRect MakeWH(int w, int h) {
        GIRect r;
        r.setLTRB(0, 0, w, h);
        return r;
    }
    
    static GIRect MakeLTRB(int l, int t, int r, int b) {
        GIRect rect;
        rect.setLTRB(l, t, r, b);
        return rect;
    }
    
    static GIRect MakeXYWH(int x, int y, int w, int h) {
        GIRect r;
        r.setXYWH(x, y, w, h);
        return r;
    }
};

#endif

