#include <string.h>
#include <assert.h>

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"

int main(int argc, char** argv) {
    GPixel storage[100 * 100];
    GBitmap bitmap;
    bitmap.fWidth = 100;
    bitmap.fHeight = 100;
    bitmap.fPixels = storage;
    bitmap.fRowBytes = bitmap.fWidth * sizeof(GPixel);

    GContext* ctx = GContext::Create(bitmap);
    if (!ctx) {
        fprintf(stderr, "GContext::Create failed\n");
        return -1;
    }
    
    const GPixel pixel = 0xFFFF0000;
    const GColor color = { 1, 1, 0, 0 };
    
    ctx->clear(color);

    for (int y = 0; y < 100; ++y) {
        for (int x = 0; x < 100; ++x) {
            const GPixel value = storage[y * 100 + x];
            if (pixel != value) {
                fprintf(stderr, "at (%d, %d) expected %x but got %x\n",
                        x, y, pixel, value);
                return -1;
            }
        }
    }

    delete ctx;
    fprintf(stderr, "passed.\n");
    return 0;
}

