/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GBitmap.h"
#include <png.h>

class GAutoFClose {
public:
    GAutoFClose(FILE* fp) : fFP(fp) {}
    ~GAutoFClose() { ::fclose(fFP); }

private:
    FILE* fFP;
};

class GAutoFree {
public:
    GAutoFree(void* ptr) : fPtr(ptr) {}
    ~GAutoFree() { ::free(fPtr); }

private:
    void* fPtr;
};

static void convertToPNG(const GPixel src[], int width, char dst[]) {
    for (int i = 0; i < width; i++) {
        GPixel c = *src++;
        int a = (c >> GPIXEL_SHIFT_A) & 0xFF;
        int r = (c >> GPIXEL_SHIFT_R) & 0xFF;
        int g = (c >> GPIXEL_SHIFT_G) & 0xFF;
        int b = (c >> GPIXEL_SHIFT_B) & 0xFF;
        
        // PNG requires unpremultiplied, but GPixel is premultiplied
        if (0 != a && 255 != a) {
            r = r * 255 / a;
            g = g * 255 / a;
            b = b * 255 / a;
        }
        *dst++ = r;
        *dst++ = g;
        *dst++ = b;
        *dst++ = a;
    }
}

bool GWriteBitmapToFile(const GBitmap& bitmap, const char path[]) {
    FILE* f = ::fopen(path, "wb");
    if (!f) {
        return false;
    }

    GAutoFClose afc(f);

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                                  NULL, NULL, NULL);
    if (!png_ptr) {
        return false;
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr,  NULL);
        return false;
    }
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return false;
    }

    png_init_io(png_ptr, f);
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        return false;
    }
    
    const int bitDepth = 8;
    png_set_IHDR(png_ptr, info_ptr, bitmap.fWidth, bitmap.fHeight, bitDepth,
                 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    char* scanline = (char*)malloc(bitmap.fWidth * 4);
    GAutoFree gaf(scanline);

    const GPixel* srcRow = bitmap.fPixels;
    for (int y = 0; y < bitmap.fHeight; y++) {
        convertToPNG(srcRow, bitmap.fWidth, scanline);
        png_bytep row_ptr = (png_bytep)scanline;
        png_write_rows(png_ptr, &row_ptr, 1);
        srcRow = (const GPixel*)((const char*)srcRow + bitmap.fRowBytes);
    }

    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return true;
}

