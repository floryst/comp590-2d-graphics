/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef app_utils_DEFINED
#define app_utils_DEFINED

#include "GBitmap.h"
#include "GColor.h"

class GContext;
class GPaint;

class AutoBitmap : public GBitmap {
public:
    AutoBitmap(int width, int height, int slop = 13) {
        fWidth = width;
        fHeight = height;
        fRowBytes = (width + slop) * sizeof(GPixel);
        fPixels = (GPixel*)malloc(fHeight * fRowBytes);
    }
    ~AutoBitmap() {
        free(fPixels);
    }
};

///////////////////////////////////////////////////////////////////////////////

static GPixel* next_row(const GBitmap& bm, GPixel* row) {
    return (GPixel*)((char*)row + bm.fRowBytes);
}

static void assert_unit_float(float x) {
    GASSERT(x >= 0 && x <= 1);
}

static int unit_float_to_byte(float x) {
    GASSERT(x >= 0 && x <= 1);
    return (int)(x * 255 + 0.5f);
}

/*
 *  Pins each float value to be [0...1]
 *  Then scales them to bytes, and packs them into a GPixel
 */
static GPixel color_to_pixel(const GColor& c) {
    assert_unit_float(c.fA);
    assert_unit_float(c.fR);
    assert_unit_float(c.fG);
    assert_unit_float(c.fB);
    int a = unit_float_to_byte(c.fA);
    int r = unit_float_to_byte(c.fR * c.fA);
    int g = unit_float_to_byte(c.fG * c.fA);
    int b = unit_float_to_byte(c.fB * c.fA);
    
    return GPixel_PackARGB(a, r, g, b);
}

static float color_dot(const float c[], float s0, float s1, float s2, float s3) {
    float res = c[0] * s0 + c[4] * s1 + c[8] * s2 + c[12] * s3;
    GASSERT(res >= 0);
    // our bilerp can have a tiny amount of error, resulting in a dot-prod
    // of slightly greater than 1, so we have to pin here.
    if (res > 1) {
        res = 1;
    }
    return res;
}

static GColor lerp4colors(const GColor corners[], float dx, float dy) {
    float LT = (1 - dx) * (1 - dy);
    float RT = dx * (1 - dy);
    float RB = dx * dy;
    float LB = (1 - dx) * dy;
    
    return GColor::Make(color_dot(&corners[0].fA, LT, RT, RB, LB),
                        color_dot(&corners[0].fR, LT, RT, RB, LB),
                        color_dot(&corners[0].fG, LT, RT, RB, LB),
                        color_dot(&corners[0].fB, LT, RT, RB, LB));
}

/**
 *  fill a bitmap with a single color
 */
static void app_fill_color(const GBitmap& bm, const GColor& color) {
    const GPixel pixel = color_to_pixel(color);
    for (int y = 0; y < bm.height(); ++y) {
        GPixel* row = bm.getAddr(0, y);
        for (int x = 0; x < bm.width(); ++x) {
            row[x] = pixel;
        }
        row = next_row(bm, row);
    }
}

/**
 *  colors[] are for each corner's starting color [LT, RT, RB, LB]
 */
static void app_fill_ramp(const GBitmap& bm, const GColor colors[4]) {
    const float xscale = 1.0f / (bm.width() - 1);
    const float yscale = 1.0f / (bm.height() - 1);
    
    GPixel* row = bm.fPixels;
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            GColor c = lerp4colors(colors, x * xscale, y * yscale);
            row[x] = color_to_pixel(c);
        }
        row = next_row(bm, row);
    }
}

void app_draw_convexpoly(GContext*, const GPoint[], int count, const GPaint&);
void app_draw_convexpoly(GContext*, const GPoint[], int count, const GPaint[]);


#endif
