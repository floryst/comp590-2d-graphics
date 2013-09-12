#include <string.h>

#include "GContext.h"
#include "GBitmap.h"
#include "GColor.h"

int main(int argc, char** argv) {
    const int W = 100;
    const int H = 100;
    GContext* ctx = GContext::Create(W, H);
    if (!ctx) {
        fprintf(stderr, "GContext::Create failed\n");
        return -1;
    }
    
    const GColor color = { 1, 1, 0, 0 };
    const GPixel pixel = 0xFFFF0000;
    
    ctx->clear(color);

    GBitmap bitmap;
    ctx->getBitmap(&bitmap);

    if (W != bitmap.fWidth || H != bitmap.fHeight) {
        fprintf(stderr, "unexpected width/height [%d %d]\n", bitmap.fWidth, bitmap.fHeight);
        return -1;
    }

    for (int y = 0; y < W; ++y) {
        const char* row = (const char*)bitmap.fPixels + bitmap.fRowBytes * y;
        for (int x = 0; x < H; ++x) {
            const GPixel value = ((const GPixel*)row)[x];
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

