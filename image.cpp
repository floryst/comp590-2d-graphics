/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include <string.h>
#include <string>

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GRandom.h"
#include "GIRect.h"

template <typename T> class GAutoDelete {
public:
    GAutoDelete(T* obj) : fObj(obj) {}
    ~GAutoDelete() { delete fObj; }

    T* get() const { return fObj; }
    operator T*() { return fObj; }
    T* operator->() { return fObj; }

    T* detach() {
        T* obj = fObj;
        fObj = NULL;
        return obj;
    }

private:
    T*  fObj;
};

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

static void translate(GIRect* r, int dx, int dy) {
    r->fLeft += dx;
    r->fTop += dy;
    r->fRight += dx;
    r->fBottom += dy;
}

static void make_rand_rect(GRandom& rand, GIRect* r, int w, int h) {
    int cx = rand.nextRange(0, w);
    int cy = rand.nextRange(0, h);
    int cw = rand.nextRange(1, w/4);
    int ch = rand.nextRange(1, h/4);
    r->setXYWH(cx - cw/2, cy - ch/2, cw, ch);
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

static GPixel* next_row(const GBitmap& bm, GPixel* row) {
    return (GPixel*)((char*)row + bm.fRowBytes);
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
    
    const GColor* colorPtr = colors;
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            ctx->fillIRect(GIRect::MakeXYWH(x * W, y * H, W, H), *colorPtr++);
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

    GIRect r = GIRect::MakeWH(1, H);
    for (int x = 0; x < W; ++x) {
        GColor color;
        lerp(c0, c1, x * 1.0f / W, &color);
        ctx->fillIRect(r, color);
        translate(&r, 1, 0);
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
    
    GRandom rand;
    for (int y = 0; y < H; y += N) {
        for (int x = 0; x < W; x += N) {
            GColor color;
            make_opaque_color(rand, &color);
            ctx->fillIRect(GIRect::MakeXYWH(x, y, N, N), color);
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
    
    GRandom rand;
    for (int i = 0; i < 400; ++i) {
        GColor color;
        make_translucent_color(rand, &color);
        color.fA /= 2;

        GIRect r;
        make_rand_rect(rand, &r, W, H);

        ctx->fillIRect(r, color);
    }
    *name = "blend";
    return ctx;
}

static void fill(GContext* ctx, int L, int T, int R, int B, const GColor& c) {
    if (R > L && B > T) {
        ctx->fillIRect(GIRect::MakeLTRB(L, T, R, B), c);
    }
}

static void frameIRect(GContext* ctx, const GIRect& r, int dx, int dy, const GColor& c) {
    GASSERT(dx >= 0);
    GASSERT(dy >= 0);

    GIRect inner = r;
    inner.fLeft += dx;
    inner.fRight -= dx;
    inner.fTop += dy;
    inner.fBottom -= dy;

    if (inner.width() <= 0 || inner.height() <= 0) {
        ctx->fillIRect(r, c);
    } else {
        fill(ctx, r.fLeft, r.fTop, r.fRight, inner.fTop, c);
        fill(ctx, r.fLeft, inner.fTop, inner.fLeft, inner.fBottom, c);
        fill(ctx, inner.fRight, inner.fTop, r.fRight, inner.fBottom, c);
        fill(ctx, r.fLeft, inner.fBottom, r.fRight, r.fBottom, c);
    }
}

static GContext* image_frame(const char** name) {
    const int W = 500;
    const int H = 500;
    
    GContext* ctx = GContext::Create(W, H);
    ctx->clear(gGColor_WHITE);
    
    GRandom rand;
    GIRect r;
    GColor c;
    for (int i = 0; i < 200; ++i) {
        make_rand_rect(rand, &r, W, H);
        make_translucent_color(rand, &c);
        c.fA = 0.80f;
        int h = rand.nextRange(0, 25);
        int w = rand.nextRange(0, 25);
        frameIRect(ctx, r, w, h, c);
    }
    *name = "frame";
    return ctx;
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

class AutoBitmap : public GBitmap {
public:
    AutoBitmap(int width, int height, int slop) {
        fWidth = width;
        fHeight = height;
        fRowBytes = (width + slop) * sizeof(GPixel);
        fPixels = (GPixel*)malloc(fHeight * fRowBytes);
    }
    ~AutoBitmap() {
        free(fPixels);
    }
};

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

static GContext* make_ramp(const GColor& clearColor, const GColor corners[4],
                           float globalAlpha) {
    const int W = 256;
    const int H = 256;
    
    GContext* ctx = GContext::Create(W, H);
    ctx->clear(clearColor);
    
    AutoBitmap bm(W, H, 17);
    fill_ramp(bm, corners);
    ctx->drawBitmap(bm, 0, 0, globalAlpha);
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
    image_bitmap_solid_alpha, image_bitmap_blend_alpha
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
        if (gVerbose) {
            printf("drawing... %s [%d %d]", name, drawnBM.width(), drawnBM.height());
        }

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
                if (gVerbose) {
                    printf(" ... match %d%%\n", (int)(s * 100));
                }
                score += s;
            } else {
                printf(" ... failed to read expected image at %s\n",
                        path.c_str());
            }
        } else {
            if (gVerbose) {
                printf("\n");
            }
        }
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

