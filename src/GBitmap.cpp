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

    void* get() const { return fPtr; }

    void* detach() {
        void* ptr = fPtr;
        fPtr = NULL;
        return ptr;
    }

private:
    void* fPtr;
};

static void convertToPNG(const GPixel src[], int width, char dst[]) {
    for (int i = 0; i < width; i++) {
        GPixel c = *src++;
        int a = GPixel_GetA(c);
        int r = GPixel_GetR(c);
        int g = GPixel_GetG(c);
        int b = GPixel_GetB(c);
        
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

///////////////////////////////////////////////////////////////////////////////

class GAutoPNGReader {
public:
    GAutoPNGReader(png_structp png, png_infop info) {
        fPng = png;
        fInfo = info;
    }
    
    ~GAutoPNGReader() {
        png_read_end(fPng, fInfo);
    }

private:
    png_structp fPng;
    png_infop   fInfo;
};

static void swizzle_rgb_row(GPixel dst[], const uint8_t src[], int count) {
    for (int i = 0; i < count; ++i) {
        dst[i] = GPixel_PackARGB(0xFF, src[0], src[1], src[2]);
        src += 3;
    }
}

static int alpha_mul(unsigned a, unsigned c) {
    return (a * c + 127) / 255;
}

static void swizzle_rgba_row(GPixel dst[], const uint8_t src[], int count) {
    for (int i = 0; i < count; ++i) {
        unsigned a = src[3];
        dst[i] = GPixel_PackARGB(a,
                                 alpha_mul(a, src[0]),
                                 alpha_mul(a, src[1]),
                                 alpha_mul(a, src[2]));
        src += 4;
    }
}

typedef void (*swizzle_row_proc)(GPixel[], const uint8_t[], int);

#define SIGNATURE_BYTES 4

static bool always_false() {
    printf("error\n");
    return false;
}

bool GReadBitmapFromFile(const char path[], GBitmap* bitmap) {
    FILE* file = fopen(path, "rb");
    if (NULL == file) {
        return always_false();
    }
    GAutoFClose afc(file);

    uint8_t signature[SIGNATURE_BYTES];
    if (SIGNATURE_BYTES != fread(signature, 1, SIGNATURE_BYTES, file)) {
        return always_false();
    }
    if (png_sig_cmp(signature, 0, SIGNATURE_BYTES)) {
        return always_false();
    }
        
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                 NULL, NULL, NULL);
    if (NULL == png_ptr) {
        return always_false();
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (NULL == info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return always_false();
    }
    
    GAutoPNGReader reader(png_ptr, info_ptr);
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        return always_false();
    }
    
    png_init_io(png_ptr, file);
    png_set_sig_bytes(png_ptr, SIGNATURE_BYTES);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bitDepth, colorType;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bitDepth, &colorType,
                 NULL, NULL, NULL);

    if (8 != bitDepth) {
        return always_false();   // TODO: handle other formats
    }
    if (png_set_interlace_handling(png_ptr) > 1) {
        return always_false();   // TODO: support interleave
    }

    swizzle_row_proc row_proc = NULL;
    switch (colorType) {
        case PNG_COLOR_TYPE_RGB:
            row_proc = swizzle_rgb_row;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            row_proc = swizzle_rgba_row;
            break;
        default:
            return always_false();
    }

    png_read_update_info(png_ptr, info_ptr);

    GAutoFree rowStorage(malloc(png_get_rowbytes(png_ptr, info_ptr)));
    png_bytep srcRow = (png_bytep)rowStorage.get();
    if (!srcRow) {
        return always_false();
    }

    GAutoFree pixelStorage(malloc(height * width * 4));
    GPixel* dstRow = (GPixel*)pixelStorage.get();
    if (NULL == dstRow) {
        return always_false();
    }

    for (int y = 0; y < height; y++) {
        uint8_t* tmp = srcRow;
        png_read_rows(png_ptr, &tmp, NULL, 1);
        row_proc(dstRow, srcRow, width);
        dstRow += width;
    }

    bitmap->fWidth = width;
    bitmap->fHeight = height;
    bitmap->fRowBytes = width * 4;
    bitmap->fPixels = (GPixel*)pixelStorage.detach();
    return true;
}

