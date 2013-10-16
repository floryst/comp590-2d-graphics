/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GContext.h"
#include "GBitmap.h"
#include "GPaint.h"
#include "GRect.h"
#include "GRandom.h"
#include "GTime.h"

#include <string.h>
#include <stdlib.h>

static bool gVerbose;
static int gRepeatCount = 1;

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

static GPixel* next_row(const GBitmap& bm, GPixel* row) {
    return (GPixel*)((char*)row + bm.fRowBytes);
}

static double time_erase(GContext* ctx, const GColor& color) {
    GBitmap bm;
    ctx->getBitmap(&bm);

    int loop = 2 * 1000 * gRepeatCount;
    
    GMSec before = GTime::GetMSec();
    
    for (int i = 0; i < loop; ++i) {
        ctx->clear(color);
    }
    
    GMSec dur = GTime::GetMSec() - before;
    
    return dur * 1000.0 / (bm.fWidth * bm.fHeight) / gRepeatCount;
}

static void clear_bench() {
    const int DIM = 1 << 8;
    static const struct {
        int fWidth;
        int fHeight;
    } gSizes[] = {
        { DIM * DIM, 1 },
        { 1, DIM * DIM },
        { DIM, DIM },
    };

    const GColor color = { 0.5, 1, 0.5, 0 };
    double total = 0;
    
    for (int i = 0; i < GARRAY_COUNT(gSizes); ++i) {
        const int w = gSizes[i].fWidth;
        const int h = gSizes[i].fHeight;
        
        GContext* ctx = GContext::Create(w, h);
        if (!ctx) {
            fprintf(stderr, "GContext::Create failed [%d %d]\n", w, h);
            exit(-1);
        }
        
        double dur = time_erase(ctx, color);
        if (gVerbose) {
            printf("[%5d, %5d] %8.4f per-pixel\n", w, h, dur);
        }
        delete ctx;
        
        total += dur;
    }
    printf("Clear time %8.4f per-pixel\n", total / GARRAY_COUNT(gSizes));
}

///////////////////////////////////////////////////////////////////////////////

static GRect rand_rect_255(GRandom& rand) {
    float x = rand.nextF() * 15;
    float y = rand.nextF() * 15;
    float w = rand.nextF() * 255;
    float h = rand.nextF() * 255;
    return GRect::MakeXYWH(x, y, w, h);
}

static double time_rect(GContext* ctx, const GRect& rect, float alpha,
                        GRect (*proc)(GRandom&)) {
    int loop = 20 * 1000 * gRepeatCount;
    
    GMSec before = GTime::GetMSec();
    GColor color = { alpha, 0, 0, 0 };
    GRandom rand;
    GPaint paint;

    double area = 0;
    if (proc) {
        for (int i = 0; i < loop; ++i) {
            GRect r = proc(rand);
            color.fR = rand.nextF();
            paint.setColor(color);
            ctx->drawRect(r, paint);
            // this is not really accurage for 'area', since we should really
            // measure the intersected-area of r w/ the context's bitmap
            area += r.width() * r.height();
        }
    } else {
        for (int i = 0; i < loop; ++i) {
            color.fR = rand.nextF();
            paint.setColor(color);
            ctx->drawRect(rect, paint);
            area += rect.width() * rect.height();
        }
    }
    GMSec dur = GTime::GetMSec() - before;
    

    return dur * 1000 * 1000.0 / area;
}

static void rect_bench() {
    const int W = 256;
    const int H = 256;
    static const struct {
        float fWidth;
        float fHeight;
        float fAlpha;
        const char* fDesc;
        GRect (*fProc)(GRandom&);
    } gRec[] = {
        { 2, H,    1.0f,   "opaque narrow", NULL },
        { W, 2,    1.0f,   "opaque   wide", NULL },

        { 2, H,    0.5f,   " blend narrow", NULL },
        { W, 2,    0.5f,   " blend   wide", NULL },
        { W, H,    0.5f,   " blend random", rand_rect_255 },

        { W, H,    0.0f,   "  zero   full", NULL },
    };

    GContext* ctx = GContext::Create(W, H);
    ctx->clear(GColor::Make(1, 1, 1, 1));

    double total = 0;
    for (int i = 0; i < GARRAY_COUNT(gRec); ++i) {
        GRect r = GRect::MakeWH(gRec[i].fWidth, gRec[i].fHeight);
        double dur = time_rect(ctx, r, gRec[i].fAlpha, gRec[i].fProc);
        if (gVerbose) {
            printf("Rect %s %8.4f per-pixel\n", gRec[i].fDesc, dur);
        }
        total += dur;
    }
    printf("Rect  time %8.4f per-pixel\n", total / GARRAY_COUNT(gRec));
    delete ctx;
}

///////////////////////////////////////////////////////////////////////////////

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
 *  colors[] are for each corner's starting color [LT, RT, RB, LB]
 */
static void fill_ramp(const GBitmap& bm, const GColor colors[4]) {
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

static void init(GBitmap* bm, int W, int H) {
    bm->fWidth = W;
    bm->fHeight = H;
    bm->fRowBytes = W * sizeof(GPixel);
    bm->fPixels = (GPixel*)malloc(bm->fRowBytes * bm->fHeight);
}

static double time_bitmap(GContext* ctx, const GBitmap& bm, float alpha) {
    int loop = 1000 * gRepeatCount;
    double area = bm.width() * bm.height();

    GPaint paint;
    paint.setAlpha(alpha);

    GMSec before = GTime::GetMSec();
    for (int i = 0; i < loop; ++i) {
        ctx->drawBitmap(bm, 0, 0, paint);
    }
    GMSec dur = GTime::GetMSec() - before;
    return dur * 500 * 1000.0 / (loop * area);
}

static void bitmap_bench_worker(bool doScale) {
    const int W = 256;
    const int H = 256;
    
    GColor corners[] = {
        GColor::Make(1, 1, 0, 0),   GColor::Make(1, 0, 1, 0),
        GColor::Make(1, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
    };
    
    const struct {
        const char* fDesc;
        const float fCornerAlpha;
        const float fGlobalAlpha;
    } gRec[] = {
        { "bitmap_solid_opaque",    1.0f,  1.0f },
        { "bitmap_blend_opaque",    0.5f,  1.0f },
        { "bitmap_solid_alpha ",     1.0f,  0.5f },
        { "bitmap_blend_alpha ",     0.5f,  0.5f },
    };
    
    GBitmap bitmaps[GARRAY_COUNT(gRec)];
    for (int i = 0; i < GARRAY_COUNT(gRec); ++i) {
        init(&bitmaps[i], W, H);
        corners[1].fA = corners[2].fA = gRec[i].fCornerAlpha;
        fill_ramp(bitmaps[i], corners);
    }
    
    GContext* ctx = GContext::Create(W, H);
    ctx->clear(GColor::Make(1, 1, 1, 1));
    
    const char* name = doScale ? "Bitmap_scale" : "Bitmap";
    
    if (doScale) {
        ctx->scale(1.1f, 1.1f);
    }

    double total = 0;
    for (int i = 0; i < GARRAY_COUNT(gRec); ++i) {
        double dur = time_bitmap(ctx, bitmaps[i], gRec[i].fGlobalAlpha);
        if (gVerbose) {
            printf("%s %s %8.4f per-pixel\n", name, gRec[i].fDesc, dur);
        }
        total += dur;
    }
    printf("%s time %7.4f per-pixel\n", name, total / GARRAY_COUNT(gRec));
    
    for (int i = 0; i < GARRAY_COUNT(bitmaps); ++i) {
        free(bitmaps[i].fPixels);
    }
    delete ctx;
}

static void bitmap_bench() {
    bitmap_bench_worker(false);
}

static void bitmap_scale_bench() {
    bitmap_bench_worker(true);
}

///////////////////////////////////////////////////////////////////////////////

typedef void (*BenchProc)();

static const BenchProc gBenches[] = {
    clear_bench,
    rect_bench,
    bitmap_bench,
    bitmap_scale_bench,
};

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help")) {
            printf("Time drawing commands on GContext.\n"
                   "--verbose (or -v) for verbose/detailed output.\n"
                   "--repeat N to run the internal loops N times to reduce noise.\n");
            return 0;
        }
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            gVerbose = true;
        } else if (!strcmp(argv[i], "--repeat")) {
            if (i == argc - 1) {
                fprintf(stderr, "need valid repeat_count # after --repeat\n");
                exit(-1);
            }
            int n = (int)atol(argv[i + 1]);
            if (n > 0) {
                gRepeatCount = n;
            } else {
                fprintf(stderr, "repeat value needs to be > 0\n");
                exit(-1);
            }
        }
    }

    for (int i = 0; i < GARRAY_COUNT(gBenches); ++i) {
        gBenches[i]();
    }
    return 0;
}

