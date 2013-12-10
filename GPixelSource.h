/**
 * Copyright 2013 Forrest Li
 *
 * COMP 590 -- Fall 2013
 */

#ifndef GPixelSource_DEFINED
#define GPixelSource_DEFINED




class GSource {
public:
	virtual GPixel nextPixel() = 0;
};

class GSourcePixel : public GSource {
public:

	GSourcePixel(GPixel source) : pixel(source) {}
	
	GPixel nextPixel() {
		return pixel;
	}

private:
	GPixel pixel;
}

class GSourceBitmap : public GSource {
public:

	GPixelBitmap(GPixel* src, GTransform ) : pixels(src) {}

	GPixel nextPixel() {
		return *(pixels++);
	}

private:
	GPixel* pixels;
}

#endif
