/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GXWindow.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GContext.h"
#include "GIRect.h"
#include "GRandom.h"
#include "GTime.h"

static GIRect rand_rect(GRandom& rand, int w, int h) {
    int cx = rand.nextRange(0, w);
    int cy = rand.nextRange(0, h);
    int cw = rand.nextRange(1, w/4);
    int ch = rand.nextRange(1, h/4);
    return GIRect::MakeXYWH(cx - cw/2, cy - ch/2, cw, ch);
}

static GColor rand_color(GRandom& rand) {
    return GColor::Make(rand.nextF(), rand.nextF(), rand.nextF(), rand.nextF());
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

static void alloc_bm(GBitmap* bm, int w, int h) {
    bm->fWidth = w;
    bm->fHeight = h;
    bm->fRowBytes = w * sizeof(GPixel);
    bm->fPixels = (GPixel*)malloc(h * bm->fRowBytes);
}

static void make_bm(const GBitmap& bm, float finalAlpha) {
    const int W = bm.fWidth;
    const int H = bm.fHeight;
    const GColor c0 = { 1, 1, 0, 0 };
    const GColor c1 = { finalAlpha, 0, 1, 1 };
    
    GContext* ctx = GContext::Create(bm);
    ctx->clear(GColor::Make(0, 0, 0, 0));
    
    GIRect r = GIRect::MakeWH(1, H);
    for (int x = 0; x < W; ++x) {
        GColor color;
        lerp(c0, c1, x * 1.0f / W, &color);
        ctx->fillIRect(r, color);
        r.offset(1, 0);
    }
    delete ctx;
}

class TestWindow : public GXWindow {
    GBitmap fBitmap;

public:
    TestWindow(int w, int h) : GXWindow(w, h) {
        this->setTitle("Hit a key to toggle opaque/translucent");
        fDoOpaque = true;
        fStartTime = GTime::GetMSec();
        fCounter = 0;

        alloc_bm(&fBitmap, 200, 200);
        make_bm(fBitmap, 0.5f);
    }

    virtual ~TestWindow() {
        free(fBitmap.fPixels);
    }
    
protected:
    virtual void onDraw(GContext* ctx) {
        ctx->clear(GColor::Make(1, 1, 1, 1));
        
        const int w = this->width();
        const int h = this->height();
#if 0
        for (int i = 0; i < 1000; ++i) {
            GColor c = rand_color(fRand);
            if (fDoOpaque) {
                c.fA = 1;
            }
            ctx->fillIRect(rand_rect(fRand, w, h), c);
        }
#else
        for (int i = 0; i < 100; ++i) {
            ctx->drawBitmap(fBitmap, 20, 20, fDoOpaque ? 1 : 0.5f);
        }
#endif

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
    
    virtual bool onKeyPress(const XEvent& evt) {
        fDoOpaque = !fDoOpaque;
        return this->INHERITED::onKeyPress(evt);
    }
    
private:
    GRandom fRand;
    GMSec   fStartTime;
    int     fCounter;
    bool    fDoOpaque;

    typedef GXWindow INHERITED;
};

int main(int argc, char* argv[]) {    
    return TestWindow(640, 480).run();
}

