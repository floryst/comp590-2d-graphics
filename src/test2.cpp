#include <string.h>

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"

struct Size {
    int fW, fH;
};

template <typename T> class AutoDelete {
public:
    AutoDelete(T* obj) : fObj(obj) {}
    ~AutoDelete() { delete fObj; }

private:
    T* fObj;
};

static int check_pixels(const GBitmap& bm, GPixel expected) {
    const GPixel* row = bm.fPixels;
    for (int y = 0; y < bm.fHeight; ++y) {
        for (int x = 0; x < bm.fWidth; ++x) {
            GPixel pixel = row[x];
            if (pixel != expected) {
                fprintf(stderr, "at (%d, %d) expected %x but got %x\n",
                        x, y, expected, pixel);
                return -1;
            }
        }
        // skip to the next row
        row = (const GPixel*)((const char*)row + bm.fRowBytes);
    }
    return 0;
}

static int test_context(GContext* ctx, const Size& size) {
    if (!ctx) {
        fprintf(stderr, "GContext::Create failed\n");
        return -1;
    }

    AutoDelete<GContext> ad(ctx);


    GBitmap bitmap;
    // memset() not required, but makes it easier to detect errors in the
    // getBitmap implementation.
    memset(&bitmap, 0, sizeof(GBitmap));
    ctx->getBitmap(&bitmap);

    if (!bitmap.fPixels) {
        fprintf(stderr, "did not get valid fPixels from getBitmap\n");
        return -2;
    }
    if (bitmap.fRowBytes < bitmap.fWidth * 4) {
        fprintf(stderr, "fRowBytes too small from getBitmap [%zu]\n",
                bitmap.fRowBytes);
        return -3;
    }
    if (bitmap.fWidth != size.fW || bitmap.fHeight != size.fH) {
        fprintf(stderr,
                "mismatch on dimensions: expected [%d %d] but got [%d %d]",
                size.fW, size.fH, bitmap.fWidth, bitmap.fHeight);
        return -4;
    }

    // just test opaque red for now
    const GColor color = { 1, 1, 0, 0 };
    const GPixel pixel = (0xFF << GPIXEL_SHIFT_A) | (0xFF << GPIXEL_SHIFT_R);

    ctx->clear(color);
    return check_pixels(bitmap, pixel);
}

class BitmapAlloc {
public:
    BitmapAlloc(GBitmap* bitmap, int width, int height) {
        bitmap->fWidth = width;
        bitmap->fHeight = height;
        bitmap->fRowBytes = width * sizeof(GPixel)+1024;

        fPixels = malloc(bitmap->fHeight * bitmap->fRowBytes);
        bitmap->fPixels = (GPixel*)fPixels;
    }

    ~BitmapAlloc() {
        free(fPixels);
    }

private:
    void* fPixels;
};

int main(int argc, char** argv) {
    static const Size gSizes[] = {
        { 1, 1 },
        { 100, 1 },
        { 1, 100 },
        { 12345, 3 },
        { 3, 12345 },
        { 99, 99 },
    };

    for (int i = 0; i < GARRAY_COUNT(gSizes); ++i) {
        const int w = gSizes[i].fW;
        const int h = gSizes[i].fH;

        GBitmap bitmap;
        BitmapAlloc alloc(&bitmap, w, h);
        
        fprintf(stderr, "testing [%d %d]\n", w, h);

        int res;
        res = test_context(GContext::Create(bitmap), gSizes[i]);
        if (res) { return res; }
        res = test_context(GContext::Create(w, h), gSizes[i]);
        if (res) { return res; }
    }
    fprintf(stderr, "passed.\n");
    return 0;
}

