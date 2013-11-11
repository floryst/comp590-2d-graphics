/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GRectT_DEFINED
#define GRectT_DEFINED

#include "GPoint.h"

template <typename T> class GRectT {
public:
    T fLeft;
    T fTop;
    T fRight;
    T fBottom;

    T x() const { return fLeft; }
    T y() const { return fTop; }
    T width() const { return fRight - fLeft; }
    T height() const { return fBottom - fTop; }

    bool isEmpty() const {
        return fLeft >= fRight || fTop >= fBottom;
    }
    
    void setEmpty() {
        fLeft = fTop = fRight = fBottom = 0;
    }

    void setLTRB(T l, T t, T r, T b) {
        fLeft = l;
        fTop = t;
        fRight = r;
        fBottom = b;
    }
    
    void setXYWH(T x, T y, T w, T h) {
        fLeft = x;
        fTop = y;
        fRight = x + w;
        fBottom = y + h;
    }

    void offset(T dx, T dy) {
        fLeft += dx;
        fTop += dy;
        fRight += dx;
        fBottom += dy;
    }
    
    void inset(T dx, T dy) {
        fLeft += dx;
        fTop += dy;
        fRight -= dx;
        fBottom -= dy;
    }
    
    bool contains(const GRectT<T>& r) const {
        return fLeft <= r.fLeft && fTop <= r.fTop &&
               r.fRight <= fRight && r.fBottom <= fBottom;
    }

    void sort() {
        if (fLeft > fRight) {
            GSwap(fLeft, fRight);
        }
        if (fTop > fBottom) {
            GSwap(fTop, fBottom);
        }
    }
    
    bool intersects(const GRectT& other) const {
        return fLeft < other.fRight && other.fLeft < fRight &&
               fTop < other.fBottom && other.fTop < fBottom;
    }

    bool setIntersection(const GRectT& r0, const GRectT& r1) {
        T l = GMax(r0.fLeft, r1.fLeft);
        T r = GMin(r0.fRight, r1.fRight);
        if (l >= r) {
            return false;
        }
        T t = GMax(r0.fTop, r1.fTop);
        T b = GMin(r0.fBottom, r1.fBottom);
        if (t >= b) {
            return false;
        }
        this->setLTRB(l, t, r, b);
        return true;
    }
};

///////////////////////////////////////////////////////////////////////////////

class GIRect : public GRectT<int32_t> {
public:
    static GIRect MakeEmpty() {
        GIRect r;
        r.setEmpty();
        return r;
    }
    
    static GIRect MakeWH(int32_t w, int32_t h) {
        GIRect r;
        r.setLTRB(0, 0, w, h);
        return r;
    }
    
    static GIRect MakeLTRB(int32_t l, int32_t t, int32_t r, int32_t b) {
        GIRect rect;
        rect.setLTRB(l, t, r, b);
        return rect;
    }
    
    static GIRect MakeXYWH(int32_t x, int32_t y, int32_t w, int32_t h) {
        GIRect r;
        r.setXYWH(x, y, w, h);
        return r;
    }
};

static inline int GRoundToInt(float x) {
    return (int)floorf(x + 0.5f);
}

class GRect : public GRectT<float> {
public:
    GRect() {}
    GRect(const GIRect& src) {
        *this = src;
    }

    GRect& operator=(const GIRect& src) {
        this->setLTRB((float)src.fLeft, (float)src.fTop,
                      (float)src.fRight, (float)src.fBottom);
        return *this;
    }
    
    float centerX() const { return (fLeft + fRight) * 0.5f; }
    float centerY() const { return (fTop + fBottom) * 0.5f; }

    GRect& setBounds(const GPoint pts[], int count) {
        if (count <= 0) {
            this->setEmpty();
        } else {
            float minx = pts[0].x();
            float maxx = minx;
            float miny = pts[0].y();
            float maxy = miny;
            for (int i = 1; i < count; ++i) {
                float x = pts[i].x();
                float y = pts[i].y();
                minx = GMin(minx, x);
                maxx = GMax(maxx, x);
                miny = GMin(miny, y);
                maxy = GMax(maxy, y);
            }
            this->setLTRB(minx, miny, maxx, maxy);
        }
        return *this;
    }
    
    GIRect round() const {
        return GIRect::MakeLTRB(GRoundToInt(fLeft), GRoundToInt(fTop),
                                GRoundToInt(fRight), GRoundToInt(fBottom));
    }

    GIRect roundOut() const {
        return GIRect::MakeLTRB((int)floorf(fLeft), (int)floorf(fTop),
                                (int)ceilf(fRight), (int)ceilf(fBottom));
    }

    ///////////////////////////////////////////////////////////////////////////

    static GRect MakeEmpty() {
        GRect r;
        r.setEmpty();
        return r;
    }
    
    static GRect MakeWH(float w, float h) {
        GRect r;
        r.setLTRB(0, 0, w, h);
        return r;
    }
    
    static GRect MakeLTRB(float l, float t, float r, float b) {
        GRect rect;
        rect.setLTRB(l, t, r, b);
        return rect;
    }
    
    static GRect MakeXYWH(float x, float y, float w, float h) {
        GRect r;
        r.setXYWH(x, y, w, h);
        return r;
    }
};

#endif

