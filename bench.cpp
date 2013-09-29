/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GIRect.h"
#include "GRandom.h"
#include "GTime.h"

#include <string.h>
#include <stdlib.h>

static bool gVerbose;
static int gRepeatCount = 1;

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

static GIRect rand_rect_255(GRandom& rand) {
    int x = rand.nextU() & 15;
    int y = rand.nextU() & 15;
    int w = rand.nextU() & 255;
    int h = rand.nextU() & 255;
    return GIRect::MakeXYWH(x, y, w, h);
}

static double time_rect(GContext* ctx, const GIRect& rect, float alpha,
                        GIRect (*proc)(GRandom&)) {
    int loop = 20 * 1000 * gRepeatCount;
    
    GMSec before = GTime::GetMSec();
    GColor color = { alpha, 0, 0, 0 };
    GRandom rand;

    double area = 0;
    if (proc) {
        for (int i = 0; i < loop; ++i) {
            GIRect r = proc(rand);
            color.fR = rand.nextF();
            ctx->fillIRect(r, color);
            // this is not really accurage for 'area', since we should really
            // measure the intersected-area of r w/ the context's bitmap
            area += r.width() * r.height();
        }
    } else {
        for (int i = 0; i < loop; ++i) {
            color.fR = rand.nextF();
            ctx->fillIRect(rect, color);
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
        int fWidth;
        int fHeight;
        float fAlpha;
        const char* fDesc;
        GIRect (*fProc)(GRandom&);
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
        GIRect r = GIRect::MakeWH(gRec[i].fWidth, gRec[i].fHeight);
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

typedef void (*BenchProc)();

static const BenchProc gBenches[] = {
    clear_bench,
    rect_bench,
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

