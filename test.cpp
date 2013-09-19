/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include <string.h>

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GIRect.h"
#include "GRandom.h"

template <typename T> class AutoDelete {
public:
    AutoDelete(T* obj) : fObj(obj) {}
    ~AutoDelete() { delete fObj; }

    T* operator->() { return fObj; }

private:
    T* fObj;
};

static const GColor GColor_TRANSPARENT = { 0,  0, 0, 0 };
static const GColor GColor_WHITE = { 1, 1, 1, 1 };
static const GColor GColor_BLACK = { 1, 0, 0, 0 };

///////////////////////////////////////////////////////////////////////////////

static bool gVerbose;

struct Stats {
    Stats() : fTests(0), fFailures(0) {}

    int fTests;
    int fFailures;
    
    double percent() const {
        return 100.0 * (fTests - fFailures) / fTests;
    }
};

typedef void (*ColorProc)(GRandom&, GColor*);

static void make_opaque_color(GRandom& rand, GColor* color) {
    color->fA = 1;
    color->fR = rand.nextF();
    color->fG = rand.nextF();
    color->fB = rand.nextF();
}

static void make_translucent_color(GRandom& rand, GColor* color) {
    color->fA = rand.nextF();
    color->fR = rand.nextF();
    color->fG = rand.nextF();
    color->fB = rand.nextF();
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

struct Size {
    int fW, fH;
};

static int max(int a, int b) { return a > b ? a : b; }

static int pixel_max_diff(uint32_t p0, uint32_t p1) {
    int da = abs(GPixel_GetA(p0) - GPixel_GetA(p1));
    int dr = abs(GPixel_GetR(p0) - GPixel_GetR(p1));
    int dg = abs(GPixel_GetG(p0) - GPixel_GetG(p1));
    int db = abs(GPixel_GetB(p0) - GPixel_GetB(p1));
    
    return max(da, max(dr, max(dg, db)));
}

static bool check_pixels(const GBitmap& bm, GPixel expected, int maxDiff) {
    GPixel pixel;

    const GPixel* row = bm.fPixels;
    for (int y = 0; y < bm.fHeight; ++y) {
        for (int x = 0; x < bm.fWidth; ++x) {
            pixel = row[x];
            if (pixel == expected) {
                continue;
            }

            // since pixels wre computed from floats, we may have a slop
            // for the expected value in each component.
            if (pixel_max_diff(pixel, expected) > maxDiff) {
                if (gVerbose) {
                    fprintf(stderr, "at (%d, %d) expected %x but got %x",
                            x, y, expected, pixel);
                }
                return false;
            }
        }
        // skip to the next row
        row = (const GPixel*)((const char*)row + bm.fRowBytes);
    }
    return true;
}

static void test_clear(Stats* stats, ColorProc colorProc, GContext* ctx,
                       const Size& size) {
    if (!ctx) {
        fprintf(stderr, "GContext::Create failed\n");
        exit(-1);
    }

    AutoDelete<GContext> ad(ctx);

    GBitmap bitmap;
    // memset() not required, but makes it easier to detect errors in the
    // getBitmap implementation.
    memset(&bitmap, 0, sizeof(GBitmap));
    ctx->getBitmap(&bitmap);

    if (!bitmap.fPixels) {
        fprintf(stderr, "did not get valid fPixels from getBitmap\n");
        exit(-1);
    }

    if (bitmap.fRowBytes < bitmap.fWidth * 4) {
        fprintf(stderr, "fRowBytes too small from getBitmap [%zu]\n",
                bitmap.fRowBytes);
        exit(-1);
    }

    if (bitmap.fWidth != size.fW || bitmap.fHeight != size.fH) {
        fprintf(stderr,
                "mismatch on dimensions: expected [%d %d] but got [%d %d]",
                size.fW, size.fH, bitmap.fWidth, bitmap.fHeight);
        exit(-1);
    }

    GRandom rand;
    for (int i = 0; i < 100; ++i) {
        GColor color;
        colorProc(rand, &color);
        const GPixel pixel = color_to_pixel(color);

        ctx->clear(color);
        if (!check_pixels(bitmap, pixel, 1)) {
            if (gVerbose) {
                fprintf(stderr, " for color(%g %g %g %g)\n",
                        color.fA, color.fR, color.fG, color.fB);
            }
            stats->fFailures += 1;
        }
        stats->fTests += 1;
    }
}

class BitmapAlloc {
public:
    BitmapAlloc(GBitmap* bitmap, int width, int height) {
        const size_t rb_slop = 17 * sizeof(GPixel);

        bitmap->fWidth = width;
        bitmap->fHeight = height;
        bitmap->fRowBytes = width * sizeof(GPixel) + rb_slop;

        fPixels = malloc(bitmap->fHeight * bitmap->fRowBytes);
        bitmap->fPixels = (GPixel*)fPixels;
    }

    ~BitmapAlloc() {
        free(fPixels);
    }

private:
    void* fPixels;
};

static void test_clear(Stats* stats, ColorProc colorProc) {
    static const int gDim[] = {
        1, 2, 3, 5, 10, 25, 200, 1001
    };
    
    for (int wi = 0; wi < GARRAY_COUNT(gDim); ++wi) {
        for (int hi = 0; hi < GARRAY_COUNT(gDim); ++hi) {
            const int w = gDim[wi];
            const int h = gDim[hi];
            Size size = { w, h };
            
            GBitmap bitmap;
            BitmapAlloc alloc(&bitmap, w, h);
            
            if (gVerbose) {
                fprintf(stderr, "testing [%d %d]\n", w, h);
            }
            
            test_clear(stats, colorProc, GContext::Create(bitmap), size);
            test_clear(stats, colorProc, GContext::Create(w, h), size);
        }
    }
}

static void test_clear_opaque(Stats* stats) {
    test_clear(stats, make_opaque_color);
}

static void test_clear_translucent(Stats* stats) {
    test_clear(stats, make_translucent_color);
}

static void test_simple_rect(Stats* stats) {
    AutoDelete<GContext> ctx(GContext::Create(100, 100));
    
    GBitmap bitmap;
    ctx->getBitmap(&bitmap);
    GIRect r = GIRect::MakeWH(bitmap.fWidth, bitmap.fHeight);
    
    const GColor colors[] = {
        { 1, 0, 0, 0 }, { 1, 0, 0, 1 }, { 1, 0, 1, 0 }, { 1, 0, 1, 1 },
        { 1, 1, 0, 0 }, { 1, 1, 0, 1 }, { 1, 1, 1, 0 }, { 1, 1, 1, 1 },
    };

    // test opaque
    for (int i = 0; i < GARRAY_COUNT(colors); ++i) {
        ctx->clear(GColor_TRANSPARENT);
        ctx->fillIRect(r, colors[i]);

        GPixel pixel = color_to_pixel(colors[i]);
        stats->fTests += 1;
        stats->fFailures += !check_pixels(bitmap, pixel, 0);
    }
}

static void test_rects(Stats* stats) {
    AutoDelete<GContext> ctx(GContext::Create(100, 100));
    
    GBitmap bitmap;
    ctx->getBitmap(&bitmap);
    GIRect r = GIRect::MakeWH(bitmap.fWidth, bitmap.fHeight);
    
    GPixel whitePixel = color_to_pixel(GColor_WHITE);
    GPixel blackPixel = color_to_pixel(GColor_BLACK);
    
    GRandom rand;
    // test transparent
    for (int i = 0; i < 100; ++i) {
        GColor c;
        make_translucent_color(rand, &c);
        c.fA = 0;   // force transparent
        
        ctx->clear(GColor_WHITE);
        ctx->fillIRect(r, c);
        check_pixels(bitmap, whitePixel, 0);
        
        ctx->clear(GColor_BLACK);
        ctx->fillIRect(r, c);
        stats->fFailures += !check_pixels(bitmap, blackPixel, 0);
        stats->fTests += 1;
    }
    
    // test blending
    for (int i = 0; i < 100; ++i) {
        GColor c;
        make_translucent_color(rand, &c);
        
        ctx->clear(GColor_TRANSPARENT);
        ctx->fillIRect(r, c);
        stats->fFailures += !check_pixels(bitmap, color_to_pixel(c), 1);
        stats->fTests += 1;
    }
}

static void test_bad_rects(Stats* stats) {
    AutoDelete<GContext> ctx(GContext::Create(10, 10));
    
    GBitmap bitmap;
    ctx->getBitmap(&bitmap);

    const GIRect rects[] = {
        { -20, -20, -10, -10 },
        { -20, -20,   5, -10 },
        { -20, -20,  20, -10 },
        
        { -20,   0, -10, 10 },
        {  20,   0,  30, 10 },

        { -20, 10, -10, 20 },
        { -20, 10,   5, 20 },
        { -20, 10,  20, 20 },
    };

    ctx->clear(GColor_WHITE);
    GPixel pixel = 0xFFFFFFFF;  // works for any pixel ordering

    for (int i = 0; i < GARRAY_COUNT(rects); ++i) {
        ctx->fillIRect(rects[i], GColor_BLACK);
        stats->fFailures += !check_pixels(bitmap, pixel, 0);
        stats->fTests += 1;
    }
}

///////////////////////////////////////////////////////////////////////////////

typedef void (*TestProc)(Stats*);

static const TestProc gTests[] = {
    test_clear_opaque, test_clear_translucent, test_simple_rect, test_rects,
    test_bad_rects,
};

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            gVerbose = true;
        }
    }

    Stats stats;
    for (int i = 0; i < GARRAY_COUNT(gTests); ++i) {
        gTests[i](&stats);
    }
    printf("Tests passed [%3d of %3d] %g%%\n",
           stats.fTests - stats.fFailures, stats.fTests, stats.percent());

    return 0;
}

