#include "GSlide.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GPaint.h"
#include "GRect.h"
#include "GRandom.h"
#include "GTime.h"
#include "app_utils.h"

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
        
        fDa = 0;
        fScale = 1;
        fDScale = 0;
        fRad = 0;
        fDRad = 0;
    }
    virtual ~Shape() {}
    
    void setup(float dx, float dy) {
        fDx = dx; fDy = dy;
    }
    
    void bounce(int w, int h, GMSec now);
    
    const GPaint& getPaint() const { return fPaint; }
    
    void draw(GContext* ctx) {
        ctx->save();
        ctx->translate(fX, fY);
        ctx->scale(fScale * fFlipX, fScale * fFlipY);
        if (fDRad) {
            ctx->rotate(fRad);
        }
        this->onDraw(ctx);
        ctx->restore();
    }

    void toggleScale() {
        fScale = 1;
        fDScale = fDScale ? 0 : (gRand.nextF() * 2 - 1) * 0.5f;
    }
    
    void toggleRotate() {
        fRad = 0;
        fDRad = fDRad ? 0 : gRand.nextF() * 0.05f;
    }
    
    void toggleFade() {
        fA = 1;
        fDa = fDa ? 0 : gRand.nextF();
    }

    virtual const char* name() = 0;

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
        Shape* s = new RectShape(bm, x, y);
        s->toggleFade();
        return s;
    }

    virtual const char* name() { return "RectShape"; }

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
        Shape* s = new TriShape(bm, x, y);
        s->toggleScale();
        s->toggleRotate();
        return s;
    }
    
    virtual const char* name() { return "TriShape"; }

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
            float sv = sin(angle);
            float cv = cos(angle);
            fPts[i].set(scaleX * sv, scaleY * cv);
        }
        fPaint.setRGB(gRand.nextF(), gRand.nextF(), gRand.nextF());
    }
    
    virtual ~PolyShape() {
        delete[] fPts;
    }
    
    static Shape* Create(const GBitmap& bm, int x, int y) {
        Shape* s = new PolyShape(bm, x, y);
        s->toggleFade();
        return s;
    }

    virtual const char* name() { return "PolyShape"; }

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
        Shape* s = new BitmapShape(bm, x, y);
        s->toggleRotate();
        return s;
    }

    virtual const char* name() { return "BitmapShape"; }

protected:
    virtual void onDraw(GContext* ctx) {
        ctx->drawBitmap(fBM, -fBM.width() * 0.5f, -fBM.height() * 0.5f,
                        this->getPaint());
    }
    
private:
    GBitmap fBM;
};

///////////////////////////////////////////////////////////////////////////////

typedef Shape* (*ShapeFactory)(const GBitmap&, int, int);

class ShapeSlide : public GSlide {
    Shape** fShapes;
    int     fShapeCount;

    ShapeFactory    fFactory;
    int             fRepeatCount;

    const char* fName;

public:
    ShapeSlide(ShapeFactory fact, int repeatCount)
        : fFactory(fact)
        , fRepeatCount(repeatCount)
    {
        fShapes = NULL;
        fShapeCount = 0;
        fName = "";
    }

    virtual ~ShapeSlide() {
        for (int i = 0; i < fShapeCount; ++i) {
            delete fShapes[i];
        }
        delete[] fShapes;
    }

    virtual void initWithBitmaps(const GBitmap bitmaps[], int bitmapCount) {
        fShapeCount = bitmapCount * fRepeatCount;
        fShapes = new Shape*[fShapeCount];

        GRandom rand;
        float speed = 300;
        for (int i = 0; i < fShapeCount; ++i) {
            fShapes[i] = fFactory(bitmaps[i % bitmapCount], 320, 240);
            fShapes[i]->setup(rand.nextF() * speed,
                              rand.nextF() * speed);
        }
        
        fName = fShapes[0]->name();
    }

    static GSlide* Create(void* factory) {
        return new ShapeSlide((ShapeFactory)factory, 10);
    }

protected:
    virtual void onDraw(GContext* ctx) {
        GBitmap bm;
        ctx->getBitmap(&bm);

        ctx->save();
        GAutoRestoreToCount restore(ctx);

        GMSec now = GTime::GetMSec();
        for (int i = 0; i < fShapeCount; ++i) {
            fShapes[i]->bounce(bm.width(), bm.height(), now);
            fShapes[i]->draw(ctx);
        }
    }

    virtual bool onHandleKey(int ascii) {
        switch (ascii) {
            case 's':
            case 'S':
                for (int i = 0; i < fShapeCount; ++i) {
                    fShapes[i]->toggleScale();
                }
                return true;
            case 'f':
            case 'F':
                for (int i = 0; i < fShapeCount; ++i) {
                    fShapes[i]->toggleFade();
                }
                return true;
            case 'r':
            case 'R':
                for (int i = 0; i < fShapeCount; ++i) {
                    fShapes[i]->toggleRotate();
                }
                return true;
        }
        return this->GSlide::onHandleKey(ascii);
    }

    virtual const char* onName() {
        return fName;
    }
};

GSlide::Registrar rect_shape_reg(ShapeSlide::Create, (void*)RectShape::Create);
GSlide::Registrar tri_shape_reg(ShapeSlide::Create, (void*)TriShape::Create);
GSlide::Registrar poly_shape_reg(ShapeSlide::Create, (void*)PolyShape::Create);
GSlide::Registrar bitmap_shape_reg(ShapeSlide::Create, (void*)BitmapShape::Create);
