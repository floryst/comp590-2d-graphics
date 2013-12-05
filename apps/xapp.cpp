/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GXWindow.h"
#include "GBitmap.h"
#include "GPaint.h"
#include "GContext.h"
#include "GRect.h"
#include "GRandom.h"
#include "GTime.h"
#include "app_utils.h"
#include <string>
#include <math.h>

static bool gAnimateScale;
static bool gAnimateRad;

static float cos_sin(float angle, float* sinvalue) {
    float cosvalue = cos(angle);
    float sv = sqrt(1 - cosvalue * cosvalue);
    if (angle > 3.14159265359) {
        sv = -sv;
    }
    *sinvalue = sv;
    return cosvalue;
}

static void fill_circle(const GBitmap& bm) {
    int w = bm.width();
    int h = bm.height();
    int r = (w < h ? w : h) >> 1;
    float cx = w/2;
    float cy = h/2;
    float rr = (float)r * r;

    GPixel* row = bm.fPixels;
    for (int y = 0; y < h; ++y) {
        float dy2 = (y - cy) * (y - cy);
        for (int x = 0; x < w; ++x) {
            float dx2 = (x - cx) * (x - cx);
            if (dx2 + dy2 > rr) {
                row[x] = 0;
            }
        }
        row = next_row(bm, row);
    }
}

static GRandom gRand;

class Shape {
public:
    Shape(float x, float y) : fX(x), fY(y), fA(1) {
        fPrevTime = GTime::GetMSec();

        if (gRand.nextF() > 0.5) {
            fFlipX = -1;
        } else {
            fFlipX = 1;
        }
        if (gRand.nextF() > 0.5) {
            fFlipY = -1;
        } else {
            fFlipY = 1;
        }
        
        fScale = 1;
        if (gAnimateScale) {
            fDScale = (gRand.nextF() * 2 - 1) * 0.5f;
        } else {
            fDScale = 0;
        }

        fRad = 0;
        fDRad = gAnimateRad ? gRand.nextF() * 0.05f : 0;
}
    virtual ~Shape() {}

    void setup(float dx, float dy, float da) {
        fDx = dx; fDy = dy; fDa = da;
    }
    
    void bounce(int w, int h, GMSec now);

    const GPaint& getPaint() const { return fPaint; }

    void draw(GContext* ctx) {
        ctx->save();
        ctx->translate(fX, fY);
        ctx->scale(fScale * fFlipX, fScale * fFlipY);
        if (gAnimateRad) {
            ctx->rotate(fRad);
        }
        this->onDraw(ctx);
        ctx->restore();
    }

protected:
    virtual void onDraw(GContext*) = 0;

private:
    GPaint fPaint;
    float fX, fY, fA;
    float fDx, fDy, fDa;
    float fFlipX, fFlipY;
    float fScale, fDScale;
    float fRad, fDRad;
    GMSec  fPrevTime;
};

static void bounce(float& x, float& dx, float scale, float max, float min = 0) {
    x += dx * scale;
    if (dx > 0 && x > max) {
        x = max;
        dx = -dx;
    } else if (dx < 0 && x < min) {
        x = min;
        dx = -dx;
    }
}

void Shape::bounce(int w, int h, GMSec now) {
    float dur = (now - fPrevTime) / 1000.0f;
    fPrevTime = now;

    ::bounce(fX, fDx, dur, w);
    ::bounce(fY, fDy, dur, h);
    ::bounce(fA, fDa, dur, 1);
    ::bounce(fScale, fDScale, dur, 2, 0.25f);
    fPaint.setAlpha(fA);

    fRad += fDRad;
}

///////////////////////////////////////////////////////////////////////////////

static void rand_pt(GPoint* p) {
    p->set((gRand.nextF() * 2 - 1) * 100,
           (gRand.nextF() * 2 - 1) * 100);
}

class RectShape : public Shape {
public:
    RectShape(const GBitmap& bm, int x, int y) : Shape(x, y) {
        fRect.setXYWH(x, y, bm.width(), bm.height());
        fRect.offset(-fRect.centerX(), -fRect.centerY());
        fPaint.setRGB(gRand.nextF(), gRand.nextF(), gRand.nextF());
    }
    
    static Shape* Create(const GBitmap& bm, int x, int y) {
        return new RectShape(bm, x, y);
    }
    
protected:
    virtual void onDraw(GContext* ctx) {
        fPaint.setAlpha(this->getPaint().getAlpha());
        ctx->drawRect(fRect, fPaint);
    }
    
private:
    GRect fRect;
    GPaint fPaint;
};

class TriShape : public Shape {
public:
    TriShape(const GBitmap& bm, int x, int y) : Shape(x, y) {
        for (int i = 0; i < 3; ++i) {
            rand_pt(&fPts[i]);
        }
        fPaint.setRGB(gRand.nextF(), gRand.nextF(), gRand.nextF());
    }

    static Shape* Create(const GBitmap& bm, int x, int y) {
        return new TriShape(bm, x, y);
    }
    
protected:
    virtual void onDraw(GContext* ctx) {
        fPaint.setAlpha(this->getPaint().getAlpha());
        ctx->drawTriangle(fPts, fPaint);
    }
    
private:
    GPoint fPts[3];
    GPaint fPaint;
};

class PolyShape : public Shape {
public:
    PolyShape(const GBitmap& bm, int x, int y) : Shape(x, y) {
        fCount = gRand.nextRange(3, 20);
        fPts = new GPoint[fCount];
        
        float scaleX = bm.width() * .5f;
        float scaleY = bm.height() * .5f;
        for (int i = 0; i < fCount; ++i) {
            float angle = i * 2 * 3.14159265359 / fCount;
            float sv, cv;
            cv = cos_sin(angle, &sv);
            fPts[i].set(scaleX * sv, scaleY * cv);
        }
        fPaint.setRGB(gRand.nextF(), gRand.nextF(), gRand.nextF());
    }
    
    virtual ~PolyShape() {
        delete[] fPts;
    }

    static Shape* Create(const GBitmap& bm, int x, int y) {
        return new PolyShape(bm, x, y);
    }

protected:
    virtual void onDraw(GContext* ctx) {
        fPaint.setAlpha(this->getPaint().getAlpha());
        ctx->drawConvexPolygon(fPts, fCount, fPaint);
    }
    
private:
    GPoint* fPts;
    int     fCount;
    GPaint  fPaint;
};

class BitmapShape : public Shape {
public:
    BitmapShape(const GBitmap& bm, int x, int y) : Shape(x, y), fBM(bm) {}

    static Shape* Create(const GBitmap& bm, int x, int y) {
        return new BitmapShape(bm, x, y);
    }
    
protected:
    virtual void onDraw(GContext* ctx) {
        ctx->drawBitmap(fBM, -fBM.width() * 0.5f, -fBM.height() * 0.5f,
                        this->getPaint());
    }
    
private:
    GBitmap fBM;
};

typedef Shape* (*ShapeFactory)(const GBitmap&, int, int);

class TestWindow : public GXWindow {
    Shape** fShapes;
    int     fShapeCount;
    GBitmap* fBitmaps;
    int     fBitmapCount;
public:
    TestWindow(int w, int h,
               char const* const* files, int fileCount,
               bool doCircles, bool doFade, ShapeFactory fact,
               int repeat) : GXWindow(w, h) {
        fDoOpaque = true;
        fStartTime = GTime::GetMSec();
        fCounter = 0;

        GRandom rand;

        fBitmapCount = 0;
        fBitmaps = new GBitmap[fileCount];
        for (int i = 0; i < fileCount; ++i) {
            if (GReadBitmapFromFile(files[i], &fBitmaps[i])) {
                fBitmapCount += 1;
                
                if (doCircles) {
                    fill_circle(fBitmaps[i]);
                }
            } else {
                fprintf(stderr, "failed to decode %s\n", files[i]);
            }
        }

        fShapeCount = fBitmapCount * repeat;
        fShapes = new Shape*[fShapeCount];

        float speed = 300;
        for (int i = 0; i < fShapeCount; ++i) {
            fShapes[i] = fact(fBitmaps[i % fBitmapCount], w/2, h/2);
            fShapes[i]->setup(rand.nextF() * speed,
                              rand.nextF() * speed,
                              doFade ? rand.nextF() : 0);
        }
    }

    virtual ~TestWindow() {
        for (int i = 0; i < fShapeCount; ++i) {
            delete fShapes[i];
        }
        delete[] fShapes;

        for (int i = 0; i < fBitmapCount; ++i) {
            free(fBitmaps[i].fPixels);
        }
        delete[] fBitmaps;
    }
    
protected:
    virtual void onDraw(GContext* ctx) {
        ctx->clear(GColor::Make(1, 1, 1, 1));
        
        GMSec now = GTime::GetMSec();
        for (int i = 0; i < fShapeCount; ++i) {
            fShapes[i]->bounce(this->width(), this->height(), now);
            fShapes[i]->draw(ctx);
        }
        this->requestDraw();

        if (++fCounter > 100) {
            char buffer[100];
            int dur = GTime::GetMSec() - fStartTime;
            sprintf(buffer, "FPS %8.1f", fCounter * 1000.0 / dur);
            this->setTitle(buffer);

            fStartTime = GTime::GetMSec();
            fCounter = 0;
        }
    }
    
    virtual bool onKeyPress(const XEvent& evt, KeySym sym) {
        fDoOpaque = !fDoOpaque;
        return this->INHERITED::onKeyPress(evt, sym);
    }
    
private:
    GRandom fRand;
    GMSec   fStartTime;
    int     fCounter;
    bool    fDoOpaque;

    typedef GXWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    if (1 == argc) {
        fprintf(stderr, "usage: [--circles] [--fade] [--scale] [--repeat N] file1.png file2.png ...\n");
        return -1;
    }
    
    bool docircles = false;
    bool dofade = false;
    ShapeFactory fact = BitmapShape::Create;
    bool doRects = false;
    int repeat = 1;
    int firstFile = argc;

    for (int i = 1; i < argc; ++i) {
        if (strstr(argv[i], "--") == argv[i]) {
            if (!strcmp(argv[i], "--circles")) {
                docircles = true;
            } else if (!strcmp(argv[i], "--fade")) {
                dofade = true;
            } else if (!strcmp(argv[i], "--rotate")) {
                gAnimateRad = true;
            } else if (!strcmp(argv[i], "--scale")) {
                gAnimateScale = true;
            } else if (!strcmp(argv[i], "--repeat") && i < argc - 1) {
                repeat = atol(argv[++i]);
                if (repeat < 1) {
                    repeat = 1;
                }
            } else if (!strcmp(argv[i], "--rects")) {
                fact = RectShape::Create;
            } else if (!strcmp(argv[i], "--tris")) {
                fact = TriShape::Create;
            } else if (!strcmp(argv[i], "--polys")) {
                fact = PolyShape::Create;
            } else {
                fprintf(stderr, "unrecognized option %s\n", argv[i]);
                return -1;
            }
        } else {
            firstFile = i;
            break;
        }
    }
    int count = argc - firstFile;
    if (count <= 0) {
        fprintf(stderr, "need file1.png ... args\n");
        return -1;
    }

    return TestWindow(640, 480,
                      &argv[firstFile], count,
                      docircles, dofade, fact, repeat).run();
}

