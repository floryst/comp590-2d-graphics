/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include <string.h>

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GPaint.h"
#include "GRect.h"
#include "GRandom.h"

#include "app_utils.h"

#define LOOP    100

static const GColor GColor_TRANSPARENT = { 0,  0, 0, 0 };
static const GColor GColor_WHITE = { 1, 1, 1, 1 };
static const GColor GColor_BLACK = { 1, 0, 0, 0 };

///////////////////////////////////////////////////////////////////////////////

static bool gVerbose;

struct Stats {
    Stats() : fTests(0), fTrials(0), fFailures(0), fScore(0) {}

    bool addTrial(bool success) {
        fTrials += 1;
        if (!success) {
            fFailures += 1;
        }
        return success;
    }

    double localPercent() const {
        return 100.0 * (fTrials - fFailures) / fTrials;
    }
    
    void nextTest() {
        ++fTests;
        fScore += this->localPercent();
    }

    int countTests() const { return fTests; }

    double totalPercent() const {
        return fScore / fTests;
    }

private:
    int fTests;
    int fTrials, fFailures;
    double fScore;
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

struct Size {
    int fW, fH;
};

static int min(int a, int b) { return a < b ? a : b; }
static int max(int a, int b) { return a > b ? a : b; }

static int pixel_max_diff(uint32_t p0, uint32_t p1) {
    int da = abs(GPixel_GetA(p0) - GPixel_GetA(p1));
    int dr = abs(GPixel_GetR(p0) - GPixel_GetR(p1));
    int dg = abs(GPixel_GetG(p0) - GPixel_GetG(p1));
    int db = abs(GPixel_GetB(p0) - GPixel_GetB(p1));
    
    return max(da, max(dr, max(dg, db)));
}

static const GPixel* next_row(const GBitmap& bm, const GPixel* row) {
    return (const GPixel*)((const char*)row + bm.fRowBytes);
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
                    fprintf(stderr, "at (%d, %d) expected %x but got %x\n",
                            x, y, expected, pixel);
                }
                return false;
            }
        }
        row = next_row(bm, row);
    }
    return true;
}

static bool check_bitmaps(const GBitmap& a, const GBitmap& b, int maxDiff) {
    GASSERT(a.width() == b.width());
    GASSERT(a.height() == b.height());

    const GPixel* rowA = a.fPixels;
    const GPixel* rowB = b.fPixels;
    for (int y = 0; y < a.height(); ++y) {
        for (int x = 0; x < a.width(); ++x) {
            GPixel pixelA = rowA[x];
            GPixel pixelB = rowB[x];
            if (pixelA == pixelB) {
                continue;
            }
            
            if (pixel_max_diff(pixelA, pixelB) > maxDiff) {
                if (gVerbose) {
                    fprintf(stderr, "at (%d, %d) expected %x but got %x\n",
                            x, y, pixelA, pixelB);
                }
                return false;
            }
        }
        rowA = next_row(a, rowA);
        rowB = next_row(b, rowB);
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static GContext* create(const GBitmap& bm) {
    GContext* ctx = GContext::Create(bm);
    if (!ctx) {
        fprintf(stderr, "GContext::Create(w=%d h=%d rb=%zu px=%p) failed\n",
                bm.width(), bm.height(), bm.rowBytes(), bm.pixels());
        exit(-1);
    }
    return ctx;
}

static GContext* create(int w, int h) {
    GContext* ctx = GContext::Create(w, h);
    if (!ctx) {
        fprintf(stderr, "GContext::Create(w=%d h=%d) failed\n", w, h);
        exit(-1);
    }
    return ctx;
}

static void test_clear(Stats* stats, ColorProc colorProc, GContext* ctx,
                       const Size& size) {
    GAutoDelete<GContext> ad(ctx);

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
    for (int i = 0; i < 10; ++i) {
        GColor color;
        colorProc(rand, &color);
        const GPixel pixel = color_to_pixel(color);

        ctx->clear(color);
        if (!stats->addTrial(check_pixels(bitmap, pixel, 1))) {
            if (gVerbose) {
                fprintf(stderr, " for color(%g %g %g %g)\n",
                        color.fA, color.fR, color.fG, color.fB);
            }
        }
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
            
            test_clear(stats, colorProc, create(bitmap), size);
            test_clear(stats, colorProc, create(w, h), size);
        }
    }
}

static const char* test_clear_opaque(Stats* stats) {
    test_clear(stats, make_opaque_color);
    return "clear_opaque";
}

static const char* test_clear_translucent(Stats* stats) {
    test_clear(stats, make_translucent_color);
    return "clear_translucent";
}

static const char* test_simple_rect(Stats* stats) {
    GAutoDelete<GContext> ctx(GContext::Create(100, 100));
    
    GBitmap bitmap;
    ctx->getBitmap(&bitmap);
    GRect r = GRect::MakeWH(bitmap.fWidth, bitmap.fHeight);
    
    const GColor colors[] = {
        { 1, 0, 0, 0 }, { 1, 0, 0, 1 }, { 1, 0, 1, 0 }, { 1, 0, 1, 1 },
        { 1, 1, 0, 0 }, { 1, 1, 0, 1 }, { 1, 1, 1, 0 }, { 1, 1, 1, 1 },
    };

    GPaint paint;

    // test opaque
    for (int i = 0; i < GARRAY_COUNT(colors); ++i) {
        ctx->clear(GColor_TRANSPARENT);
        paint.setColor(colors[i]);

        ctx->drawRect(r, paint);

        GPixel pixel = color_to_pixel(colors[i]);
        stats->addTrial(check_pixels(bitmap, pixel, 0));
    }
    return "simple_rects";
}

static const char* test_rects(Stats* stats) {
    GAutoDelete<GContext> ctx(create(100, 100));
    
    GBitmap bitmap;
    ctx->getBitmap(&bitmap);
    GRect r = GRect::MakeWH(bitmap.fWidth, bitmap.fHeight);
    
    GPixel whitePixel = color_to_pixel(GColor_WHITE);
    GPixel blackPixel = color_to_pixel(GColor_BLACK);
    
    GRandom rand;
    GPaint paint;
    GColor color;

    // test transparent
    for (int i = 0; i < LOOP; ++i) {
        make_translucent_color(rand, &color);
        paint.setColor(color);
        paint.setAlpha(0);   // force transparent
        
        ctx->clear(GColor_WHITE);
        ctx->drawRect(r, paint);
        stats->addTrial(check_pixels(bitmap, whitePixel, 0));
        
        ctx->clear(GColor_BLACK);
        ctx->drawRect(r, paint);
        stats->addTrial(check_pixels(bitmap, blackPixel, 0));
    }
    
    // test blending
    for (int i = 0; i < LOOP; ++i) {
        make_translucent_color(rand, &color);
        paint.setColor(color);
        
        ctx->clear(GColor_TRANSPARENT);
        ctx->drawRect(r, paint);
        stats->addTrial(check_pixels(bitmap, color_to_pixel(paint.getColor()), 1));
    }
    return "draw_rects";
}

static const char* test_bad_rects(Stats* stats) {
    GAutoDelete<GContext> ctx(GContext::Create(10, 10));
    
    GBitmap bitmap;
    ctx->getBitmap(&bitmap);

    const GRect rects[] = {
        GRect::MakeLTRB( -20, -20, -10, -10 ),
        GRect::MakeLTRB( -20, -20,   5, -10 ),
        GRect::MakeLTRB( -20, -20,  20, -10 ),
        
        GRect::MakeLTRB( -20,   0, -10, 10 ),
        GRect::MakeLTRB(  20,   0,  30, 10 ),

        GRect::MakeLTRB( -20, 10, -10, 20 ),
        GRect::MakeLTRB( -20, 10,   5, 20 ),
        GRect::MakeLTRB( -20, 10,  20, 20 ),
    };

    ctx->clear(GColor_WHITE);
    GPixel pixel = 0xFFFFFFFF;  // works for any pixel ordering

    GPaint paint;
    for (int i = 0; i < GARRAY_COUNT(rects); ++i) {
        ctx->drawRect(rects[i], paint);
        
        stats->addTrial(check_pixels(bitmap, pixel, 0));
    }
    return "bad_rects";
}

///////////////////////////////////////////////////////////////////////////////

static bool intersect(const GIRect& a, const GIRect& b, GIRect* dst) {
    if (a.isEmpty() || b.isEmpty()) {
        return false;
    }
    if (a.fLeft >= b.fRight || b.fLeft >= a.fRight ||
        a.fTop >= b.fBottom || b.fTop >= a.fBottom) {
        return false;
    }
    dst->setLTRB(max(a.fLeft, b.fLeft), max(a.fTop, b.fTop),
                 min(a.fRight, b.fRight), min(a.fBottom, b.fBottom));
    return true;
}

static bool intersect2(const GIRect& a, const GIRect& b, GIRect* dst) {
    int32_t L = max(a.fLeft, b.fLeft);
    int32_t R = min(a.fRight, b.fRight);
    int32_t T = max(a.fTop, b.fTop);
    int32_t B = min(a.fBottom, b.fBottom);
    
    if (L < R && T < B) {
        dst->setLTRB(L, T, R, B);
        return true;
    }
    return false;
}

static bool extract_subset(const GBitmap& src, const GIRect& r, GBitmap* dst) {
    GIRect subR;
    if (!intersect(GIRect::MakeWH(src.width(), src.height()), r, &subR)) {
        return false;
    }
    dst->fWidth = subR.width();
    dst->fHeight = subR.height();
    dst->fRowBytes = src.rowBytes();
    dst->fPixels = src.getAddr(subR.x(), subR.y());
    return true;
}

static void rand_fill_opaque(const GBitmap& bm, GRandom rand) {
    for (int y = 0; y < bm.height(); ++y) {
        for (int x = 0; x < bm.width(); ++x) {
            *bm.getAddr(x, y) = rand.nextU() | (0xFF << GPIXEL_SHIFT_A);
        }
    }
}

class BTester {
public:
    virtual void fill(const GBitmap&, GRandom&) = 0;
    virtual bool check(const GBitmap&, const GBitmap&, const GColor& clear) = 0;

    void test(Stats* stats, const GColor& clearColor) {
        GRandom rand;
        GPaint paint;
        
        GAutoDelete<GContext> ctx(GContext::Create(102, 102));
        for (int i = 0; i < LOOP; ++i) {
            int width = rand.nextRange(1, 100);
            int height = rand.nextRange(1, 100);
            AutoBitmap bm(width, height, rand.nextRange(0, 100));
            this->fill(bm, rand);
            
            ctx->clear(clearColor);
            ctx->drawBitmap(bm, 1, 1, paint);
            
            GBitmap device, dev;
            ctx->getBitmap(&device);
            if (extract_subset(device,
                               GIRect::MakeXYWH(1, 1, bm.width(), bm.height()),
                               &dev)) {
                stats->addTrial(this->check(dev, bm, clearColor));
            }
        }
    }
};

class OpaqueBTester : public BTester {
public:
    virtual void fill(const GBitmap& bm, GRandom& rand) {
        rand_fill_opaque(bm, rand);
    }
    virtual bool check(const GBitmap& a, const GBitmap& b, const GColor&) {
        return check_bitmaps(a, b, 0);
    }
};

class TransparentBTester : public BTester {
public:
    virtual void fill(const GBitmap& bm, GRandom& rand) {
        memset(bm.fPixels, 0, bm.fHeight * bm.fRowBytes);
    }
    virtual bool check(const GBitmap& a, const GBitmap&, const GColor& clear) {
        return check_pixels(a, color_to_pixel(clear), 0);
    }
};

static const char* test_bitmap(Stats* stats) {
    OpaqueBTester opaqueBT;
    TransparentBTester transparentBT;

    BTester* const testers[] = {
        &opaqueBT, &transparentBT,
    };
    const GColor colors[] = {
        GColor::Make(0, 0, 0, 0),
        GColor::Make(1, 1, 1, 1),
        GColor::Make(0.5f, 1, 0.5, 0.13)
    };
    for (int i = 0; i < GARRAY_COUNT(colors); ++i) {
        for (int j = 0; j < GARRAY_COUNT(testers); ++j) {
            testers[j]->test(stats, colors[i]);
        }
    }
    return "draw_bitmap";
}

///////////////////////////////////////////////////////////////////////////////

static GPixel sample_mirror_x(const GBitmap& bm, int x, int y) {
    return *bm.getAddr(bm.width() - x - 1, y);
}

static GPixel sample_mirror_y(const GBitmap& bm, int x, int y) {
    return *bm.getAddr(x, bm.height() - y - 1);
}

typedef GPixel (*SampleProc)(const GBitmap&, int x, int y);

static bool cmp_sample(const GBitmap& a, const GBitmap& b, SampleProc proc) {
    GASSERT(a.width() == b.width());
    GASSERT(a.height() == b.height());

    for (int y = 0; y < a.height(); ++y) {
        for (int x = 0; x < a.width(); ++x) {
            GPixel orig = *a.getAddr(x, y);
            GPixel flip = proc(b, x, y);
            if (orig != flip) {
                return false;;
            }
        }
    }
    return true;
}

static const char* test_mirror_bitmap(Stats* stats) {
    const GColor corners[] = {
        // opaque
        GColor::Make(1, 1, 0, 0),   GColor::Make(1, 0, 1, 0),
        GColor::Make(1, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
        // per-pixel-alpha
        GColor::Make(0, 1, 0, 0),    GColor::Make(0.5f, 0, 1, 0),
        GColor::Make(0.5f, 0, 0, 1), GColor::Make(1, 0, 0, 0),
    };

    const GColor clearColors[] = {
        GColor::Make(1, 1, 1, 1),
        GColor::Make(1, 0, 0, 0),
        GColor::Make(0, 0, 0, 0),
    };
    
    const float alphaValues[] = { 0, 0.5f, 1 };

    AutoBitmap src(100, 100, 17);
    AutoBitmap dst0(100, 100, 13);
    AutoBitmap dst1(100, 100, 23);

    GAutoDelete<GContext> ctx0(GContext::Create(dst0));
    GAutoDelete<GContext> ctx1(GContext::Create(dst1));

    GPaint paint;
    
    for (int j = 0; j < GARRAY_COUNT(corners); j += 4) {
        app_fill_ramp(src, &corners[j]);
        for (int i = 0; i < GARRAY_COUNT(clearColors); ++i) {
            for (int k = 0; k < GARRAY_COUNT(alphaValues); ++k) {
                paint.setAlpha(alphaValues[k]);

                ctx0->clear(clearColors[i]);
                ctx0->drawBitmap(src, 0, 0, paint);

                ctx1->clear(clearColors[i]);
                ctx1->save();
                ctx1->translate(src.width(), 0);
                ctx1->scale(-1, 1);
                ctx1->drawBitmap(src, 0, 0, paint);
                ctx1->restore();
                stats->addTrial(cmp_sample(dst0, dst1, sample_mirror_x));
                
                ctx1->clear(clearColors[i]);
                ctx1->save();
                ctx1->translate(0, src.height());
                ctx1->scale(1, -1);
                ctx1->drawBitmap(src, 0, 0, paint);
                ctx1->restore();
                stats->addTrial(cmp_sample(dst0, dst1, sample_mirror_y));
            }
        }
    }
    return "mirror_bitmap";
}

static const char* test_bad_xform_bitmaps(Stats* stats) {
    const GColor corners[] = {
        // opaque
        GColor::Make(1, 1, 0, 0),   GColor::Make(1, 0, 1, 0),
        GColor::Make(1, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
        // per-pixel-alpha
        GColor::Make(0, 1, 0, 0),    GColor::Make(0.5f, 0, 1, 0),
        GColor::Make(0.5f, 0, 0, 1), GColor::Make(1, 0, 0, 0),
    };

    const struct {
        float   fX, fY;
    } gScales[] = {
        { 0, 1 }, { 1, 0 }, { 0.00001, 1 }, { 1, 0.00001 }
    };

    AutoBitmap src(100, 100, 3);
    AutoBitmap dst(100, 100, 11);

    GAutoDelete<GContext> ctx(GContext::Create(dst));
    ctx->clear(GColor::Make(0, 0, 0, 0));

    GPaint paint;

    for (int i = 0; i < GARRAY_COUNT(corners); i += 4) {
        app_fill_ramp(src, &corners[i]);
        for (int j = 0; j < GARRAY_COUNT(gScales); ++j) {
            ctx->save();
            ctx->scale(gScales[j].fX, gScales[j].fY);
            ctx->drawBitmap(src, 10, 10, paint);
            stats->addTrial(check_pixels(dst, 0, 0));
            ctx->restore();
        }
    }
    return "bad_xform_bitmap";
}

///////////////////////////////////////////////////////////////////////////////

typedef const char* (*TestProc)(Stats*);

static const TestProc gTests[] = {
    test_clear_opaque, test_clear_translucent,
    test_simple_rect, test_rects, test_bad_rects,
    test_bitmap,
    test_mirror_bitmap, test_bad_xform_bitmaps,
};

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
            gVerbose = true;
        }
    }

    Stats stats;
    for (int i = 0; i < GARRAY_COUNT(gTests); ++i) {
        const char* name = gTests[i](&stats);
        if (gVerbose) {
            printf("Test %20s %g%%\n", name, stats.localPercent());
        }
        stats.nextTest();
    }
    printf("Test [%d] %g%%\n", stats.countTests(), stats.totalPercent());

    return 0;
}

