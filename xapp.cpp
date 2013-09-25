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

class TestWindow : public GXWindow {
public:
    TestWindow(int w, int h) : GXWindow(w, h) {
        this->setTitle("Hit a key to toggle opaque/translucent");
        fDoOpaque = true;
        fStartTime = GTime::GetMSec();
        fCounter = 0;
    }
    
protected:
    virtual void onDraw(GContext* ctx) {
        ctx->clear(GColor::Make(1, 1, 1, 1));
        
        const int w = this->width();
        const int h = this->height();
        for (int i = 0; i < 1000; ++i) {
            GColor c = rand_color(fRand);
            if (fDoOpaque) {
                c.fA = 1;
            }
            ctx->fillIRect(rand_rect(fRand, w, h), c);
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

