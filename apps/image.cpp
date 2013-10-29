/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include <string.h>
#include <string>

#include "GContext.h"
#include "GBitmap.h"
#include "GPaint.h"
#include "GRandom.h"
#include "GRect.h"

#include "app_utils.h"

static const GColor gGColor_TRANSPARENT_BLACK = { 0, 0, 0, 0 };
static const GColor gGColor_BLACK = { 1, 0, 0, 0 };
static const GColor gGColor_WHITE = { 1, 1, 1, 1 };

static void make_filename(std::string* str, const char path[], const char name[]) {
    str->append(path);
    if ('/' != (*str)[str->size() - 1]) {
        str->append("/");
    }
    str->append(name);
}

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

static void make_rand_rect(GRandom& rand, GRect* r, int w, int h) {
    int cx = rand.nextRange(0, w);
    int cy = rand.nextRange(0, h);
    int cw = rand.nextRange(1, w/4);
    int ch = rand.nextRange(1, h/4);
    r->setXYWH(cx - cw/2, cy - ch/2, cw, ch);
}

///////////////////////////////////////////////////////////////////////////////

typedef GContext* (*ImageProc)(const char**);

// Draw a grid of primary colors
static GContext* image_primaries(const char** name) {
    const int W = 64;
    const int H = 64;
    const GColor colors[] = {
        { 1, 1, 0, 0 }, { 1, 0, 1, 0 },          { 1, 0, 0, 1 },
        { 1, 1, 1, 0 }, { 1, 1, 0, 1 },          { 1, 0, 1, 1 },
        { 1, 0, 0, 0 }, { 1, 0.5f, 0.5f, 0.5f }, { 1, 1, 1, 1 },
    };
    
    GContext* ctx = GContext::Create(W*3, H*3);
    ctx->clear(gGColor_TRANSPARENT_BLACK);
    
    GPaint paint;
    const GColor* colorPtr = colors;
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            paint.setColor(*colorPtr++);
            ctx->drawRect(GRect::MakeXYWH(x * W, y * H, W, H), paint);
        }
    }
    
    *name = "primaries";
    return ctx;
}

static float lerp(float x0, float x1, float percent) {
    return x0 + (x1 - x0) * percent;
}

static void lerp(const GColor& c0, const GColor& c1, float percent, GColor* result) {
    result->fA = lerp(c0.fA, c1.fA, percent);
    result->fR = lerp(c0.fR, c1.fR, percent);
    result->fG = lerp(c0.fG, c1.fG, percent);
    result->fB = lerp(c0.fB, c1.fB, percent);
}

static GContext* image_ramp(const char** name) {
    const int W = 200;
    const int H = 100;
    const GColor c0 = { 1, 1, 0, 0 };
    const GColor c1 = { 1, 0, 1, 1 };

    GContext* ctx = GContext::Create(W, H);
    ctx->clear(gGColor_TRANSPARENT_BLACK);

    GPaint paint;
    GRect r = GRect::MakeWH(1, H);
    for (int x = 0; x < W; ++x) {
        GColor color;
        lerp(c0, c1, x * 1.0f / W, &color);
        paint.setColor(color);
        ctx->drawRect(r, paint);
        r.offset(1, 0);
    }
    *name = "ramp";
    return ctx;
}

static GContext* image_rand(const char** name) {
    const int N = 8;
    const int W = N * 40;
    const int H = N * 40;
    
    GContext* ctx = GContext::Create(W, H);
    ctx->clear(gGColor_TRANSPARENT_BLACK);
    
    GPaint paint;
    GRandom rand;
    for (int y = 0; y < H; y += N) {
        for (int x = 0; x < W; x += N) {
            GColor color;
            make_opaque_color(rand, &color);
            paint.setColor(color);
            ctx->drawRect(GRect::MakeXYWH(x, y, N, N), paint);
        }
    }
    *name = "rand";
    return ctx;
}

static GContext* image_blend(const char** name) {
    const int W = 300;
    const int H = 300;
    
    GContext* ctx = GContext::Create(W, H);
    ctx->clear(gGColor_BLACK);
    
    GPaint paint;
    GRandom rand;
    for (int i = 0; i < 400; ++i) {
        GColor color;
        make_translucent_color(rand, &color);
        color.fA /= 2;
        paint.setColor(color);

        GRect r;
        make_rand_rect(rand, &r, W, H);
        ctx->drawRect(r, paint);
    }
    *name = "blend";
    return ctx;
}

static void fill(GContext* ctx, float L, float T, float R, float B,
                 const GPaint& paint) {
    if (R > L && B > T) {
        ctx->drawRect(GRect::MakeLTRB(L, T, R, B), paint);
    }
}

static void frameRect(GContext* ctx, const GRect& r, float dx, float dy,
                      const GPaint& paint) {
    GASSERT(dx >= 0);
    GASSERT(dy >= 0);

    GRect inner = r;
    inner.inset(dx, dy);

    if (inner.width() <= 0 || inner.height() <= 0) {
        ctx->drawRect(r, paint);
    } else {
        fill(ctx, r.fLeft, r.fTop, r.fRight, inner.fTop, paint);
        fill(ctx, r.fLeft, inner.fTop, inner.fLeft, inner.fBottom, paint);
        fill(ctx, inner.fRight, inner.fTop, r.fRight, inner.fBottom, paint);
        fill(ctx, r.fLeft, inner.fBottom, r.fRight, r.fBottom, paint);
    }
}

static GContext* image_frame(const char** name) {
    const int W = 500;
    const int H = 500;
    
    GContext* ctx = GContext::Create(W, H);
    ctx->clear(gGColor_WHITE);
    
    GPaint paint;
    GRandom rand;
    GRect r;
    GColor c;
    for (int i = 0; i < 200; ++i) {
        make_rand_rect(rand, &r, W, H);
        make_translucent_color(rand, &c);
        c.fA = 0.80f;
        paint.setColor(c);
        int h = rand.nextRange(0, 25);
        int w = rand.nextRange(0, 25);
        frameRect(ctx, r, w, h, paint);
    }
    *name = "frame";
    return ctx;
}

///////////////////////////////////////////////////////////////////////////////

static GContext* make_ramp(const GColor& clearColor, const GColor corners[4],
                           float globalAlpha) {
    const int W = 256;
    const int H = 256;
    GPaint paint;
    paint.setAlpha(globalAlpha);
    
    GContext* ctx = GContext::Create(W, H);
    ctx->clear(clearColor);
    
    AutoBitmap bm(W, H, 17);
    app_fill_ramp(bm, corners);
    ctx->drawBitmap(bm, 0, 0, paint);
    return ctx;
}

static GContext* image_bitmap_solid_opaque(const char** name) {
    const GColor corners[] = {
        GColor::Make(1, 1, 0, 0),   GColor::Make(1, 0, 1, 0),
        GColor::Make(1, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
    };
    
    *name = "bitmap_solid_opaque";
    return make_ramp(GColor::Make(1, 1, 1, 1), corners, 1);
}

static GContext* image_bitmap_blend_opaque(const char** name) {
    const GColor corners[] = {
        GColor::Make(1, 1, 0, 0),   GColor::Make(1, 0, 1, 0),
        GColor::Make(1, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
    };
    
    *name = "bitmap_blend_opaque";
    return make_ramp(GColor::Make(1, 1, 1, 1), corners, 0.5f);
}

static GContext* image_bitmap_solid_alpha(const char** name) {
    const GColor corners[] = {
        GColor::Make(0, 1, 0, 0),   GColor::Make(0.5f, 0, 1, 0),
        GColor::Make(0.5f, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
    };
    
    *name = "bitmap_solid_alpha";
    return make_ramp(GColor::Make(1, 1, 1, 1), corners, 1);
}

static GContext* image_bitmap_blend_alpha(const char** name) {
    const GColor corners[] = {
        GColor::Make(0, 1, 0, 0),   GColor::Make(0.5f, 0, 1, 0),
        GColor::Make(0.5f, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
    };
    
    *name = "bitmap_blend_alpha";
    return make_ramp(GColor::Make(1, 1, 1, 1), corners, 0.5f);
}

static GContext* image_rect_trans(const char** name) {
    *name = "rect_trans";
    
    GContext* ctx = GContext::Create(100, 100);
    ctx->clear(GColor::Make(0, 0, 0, 0));

    GPaint paint;
    paint.setARGB(0.1f, 0, 0, 0);

    GRect r = GRect::MakeWH(25, 100);
    for (int i = 0; i < 75; ++i) {
        ctx->drawRect(r, paint);
        ctx->translate(1, 0);
    }
    return ctx;
}

static GContext* image_rect_scale(const char** name) {
    *name = "rect_scale";
    
    GContext* ctx = GContext::Create(100, 100);
    ctx->clear(GColor::Make(0, 0, 0, 0));
    
    GPaint paint;
    paint.setARGB(0.1f, 0, 0, 0);
    
    GRect r = GRect::MakeWH(100, 100);
    for (int i = 0; i < 30; ++i) {
        ctx->scale(0.9f, 1);
        ctx->drawRect(r, paint);
    }
    return ctx;
}

static GContext* image_bitmap_scale_down(const char** name) {
    const GColor corners[] = {
        // opaque
        GColor::Make(1, 1, 0, 0),   GColor::Make(1, 0, 1, 0),
        GColor::Make(1, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
        // w/ alpha
        GColor::Make(0, 1, 0, 0),   GColor::Make(0.5f, 0, 1, 0),
        GColor::Make(0.5f, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
    };
    
    const float alphas[] = { 1, 0.5f };
    
    GContext* ctx = GContext::Create(256, 256);
    ctx->clear(GColor::Make(1, 1, 1, 1));
    
    ctx->scale(0.5f, 0.5f);
    
    GPaint paint;
    for (int i = 0; i < GARRAY_COUNT(corners); i += 4) {
        AutoBitmap bm(256, 256, 7);
        app_fill_ramp(bm, &corners[i]);
        for (int j = 0; j < GARRAY_COUNT(alphas); ++j) {
            paint.setAlpha(alphas[j]);
            ctx->drawBitmap(bm, (i / 4) * 256, j * 256, paint);
        }
    }
    *name = "bitmap_scale_down";
    return ctx;
}

static GContext* image_bitmap_mirror(const char** name) {
    const GColor corners[] = {
        // opaque
        GColor::Make(1, 1, 0, 0),   GColor::Make(1, 0, 1, 0),
        GColor::Make(1, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
    };

    const int W = 128;
    const int H = 128;

    GContext* ctx = GContext::Create(W*2, H*2);
    ctx->clear(GColor::Make(0, 0, 0, 0));

    AutoBitmap bm(W, H, 7);
    app_fill_ramp(bm, corners);

    const struct {
        float fSx, fSy, fTx, fTy;
    } gXforms[] = {
        { 1, 1, 0, 0 },  { -1, 1, 2*W, 0 },
        { 1, -1, 0, 2*H }, { -1, -1, 2*W, 2*H }
    };
    
    GPaint paint;
    for (int i = 0; i < GARRAY_COUNT(gXforms); ++i) {
        ctx->save();
        ctx->translate(gXforms[i].fTx, gXforms[i].fTy);
        ctx->scale(gXforms[i].fSx, gXforms[i].fSy);
        ctx->drawBitmap(bm, 0, 0, paint);
        ctx->restore();
    }
    *name = "bitmap_mirror";
    return ctx;
}

static GContext* image_bitmap_scale_up(const char** name) {
    const GColor corners[] = {
        // opaque
        GColor::Make(1, 1, 0, 0),   GColor::Make(1, 0, 1, 0),
        GColor::Make(1, 0, 0, 1),   GColor::Make(1, 0, 0, 0),
    };
    
    const int SCALE = 32;
    const int W = 8;
    const int H = 8;
    
    GContext* ctx = GContext::Create(W*SCALE, H*SCALE);
    ctx->clear(GColor::Make(0, 0, 0, 0));
    
    AutoBitmap bm(W, H, 7);
    app_fill_ramp(bm, corners);
    
    GPaint paint;
    ctx->scale(SCALE, SCALE);
    ctx->drawBitmap(bm, 0, 0, paint);

    *name = "bitmap_scale_up";
    return ctx;
}

///////////////////////////////////////////////////////////////////////////////

static int max(int a, int b) { return a > b ? a : b; }

static int pixel_max_diff(uint32_t p0, uint32_t p1) {
    int da = abs(GPixel_GetA(p0) - GPixel_GetA(p1));
    int dr = abs(GPixel_GetR(p0) - GPixel_GetR(p1));
    int dg = abs(GPixel_GetG(p0) - GPixel_GetG(p1));
    int db = abs(GPixel_GetB(p0) - GPixel_GetB(p1));
    
    return max(da, max(dr, max(dg, db)));
}

// return 0...1 amount that the images are the same. 1.0 means perfect equality.
static double compare_bitmaps(const GBitmap& a, const GBitmap& b, int maxDiff) {
    const GPixel* row_a = a.fPixels;
    const GPixel* row_b = b.fPixels;

    int diffCount = 0;

    for (int y = 0; y < a.height(); y++) {
        for (int x = 0; x < a.width(); ++x) {
            if (pixel_max_diff(row_a[x], row_b[x]) > maxDiff) {
                diffCount += 1;
            }
        }
    }
    
    double err = diffCount * 1.0 / (a.width() * a.height());
    return 1.0 - err;
}

///////////////////////////////////////////////////////////////////////////////

static const ImageProc gProcs[] = {
    image_primaries, image_ramp, image_rand, image_blend, image_frame,
    image_bitmap_solid_opaque, image_bitmap_blend_opaque,
    image_bitmap_solid_alpha, image_bitmap_blend_alpha,
    image_rect_trans, image_rect_scale,
    image_bitmap_scale_down, image_bitmap_mirror, image_bitmap_scale_up
};

static bool gVerbose;

int main(int argc, char** argv) {
    const char* writePath = NULL;
    const char* readPath = NULL;
    int tolerance = 1;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help")) {
            printf("generates a series of test images.\n"
                   "--write foo (or -w foo) writes the images as *.png files to foo directory\n");
            exit(0);
        } else if (!strcmp(argv[i], "-w") || !strcmp(argv[i], "--write")) {
            if (i == argc - 1) {
                fprintf(stderr, "need path following -w or --write\n");
                exit(-1);
            }
            writePath = argv[++i];
        } else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--read")) {
            if (i == argc - 1) {
                fprintf(stderr, "need path following -r or --read\n");
                exit(-1);
            }
            readPath = argv[++i];
        } else if (!strcmp(argv[i], "--tolerance")) {
            if (i == argc - 1) {
                fprintf(stderr, "need tolerance_value (0..255) to follow --tolerance\n");
                exit(-1);
            }
            int tol = (int)atol(argv[++i]);
            if (tol >= 0 || tol <= 255) {
                tolerance = tol;
            }
        } else if (!strcmp(argv[i], "--verbose") || !strcmp(argv[i], "-v")) {
            gVerbose = true;
        }
    }

    double score = 0;

    FILE* htmlFile = NULL;
    if (writePath) {
        std::string path;
        make_filename(&path, writePath, "index.html");
        remove(path.c_str());
        htmlFile = fopen(path.c_str(), "w");
        if (htmlFile) {
            fprintf(htmlFile, "<title>COMP590 PA2 Images</title>\n<body>\n");
        }
    }
    
    for (int i = 0; i < GARRAY_COUNT(gProcs); ++i) {
        const char* name = NULL;
        GAutoDelete<GContext> ctx(gProcs[i](&name));
        GBitmap drawnBM;
        ctx->getBitmap(&drawnBM);
        printf("drawing... %s [%d %d]", name, drawnBM.width(), drawnBM.height());

        if (writePath) {
            std::string path;
            make_filename(&path, writePath, name);
            path.append(".png");
            remove(path.c_str());

            if (!GWriteBitmapToFile(drawnBM, path.c_str())) {
                fprintf(stderr, "failed to write image to %s\n", path.c_str());
            } else if (htmlFile) {
                fprintf(htmlFile, "    <img src=\"%s.png\"> %s<p>\n", name, name);
            }
        }
        if (readPath) {
            std::string path;
            make_filename(&path, readPath, name);
            path.append(".png");
            
            GBitmap expectedBM;
            if (GReadBitmapFromFile(path.c_str(), &expectedBM)) {
                double s = compare_bitmaps(expectedBM, drawnBM, tolerance);
                printf(" ... match %d%%", (int)(s * 100));
                score += s;
            } else {
                printf(" ... failed to read expected image at %s",
                        path.c_str());
            }
        }
        printf("\n");
    }
    
    if (htmlFile) {
        fprintf(htmlFile, "</body>\n");
        fclose(htmlFile);
    }
    if (readPath) {
        int count = GARRAY_COUNT(gProcs);
        printf("Image score %d%% for %d images\n", (int)(score * 100 / count), count);
    }
    return 0;
}

