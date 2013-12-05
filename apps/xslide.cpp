/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GSlide.h"
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

static const float gMaxScale = 32;
static const float gMinScale = 1 / gMaxScale;

class SlideWindow : public GXWindow {
    const GBitmap*  fBitmaps;
    int             fBitmapCount;

    float           fScale;

    GSlide*         fSlide;
    GSlide::Pair*   fSlideArray;
    int             fSlideCount;
    int             fSlideIndex;

    void updateTitle() {
        char buffer[100];
        sprintf(buffer, "%s : scale=%g", fSlide->name(), fScale);
        this->setTitle(buffer);
    }

    void initSlide() {
        delete fSlide;
        fSlide = fSlideArray[fSlideIndex].fFact(fSlideArray[fSlideIndex].fRefCon);
        fSlide->initWithBitmaps(fBitmaps, fBitmapCount);
        this->updateTitle();
    }

    void scaleAboutCenter(GContext* ctx) {
        GBitmap bm;
        ctx->getBitmap(&bm);
        
        float cx = bm.width() * 0.5f;
        float cy = bm.height() * 0.5f;
        
        ctx->translate(cx, cy);
        ctx->scale(fScale, fScale);
        ctx->translate(-cx, -cy);
    }
    
public:
    SlideWindow(int w, int h,
               const GBitmap bitmaps[], int bitmapCount) : GXWindow(w, h) {
        fScale = 1;

        fBitmaps = bitmaps;
        fBitmapCount = bitmapCount;

        fSlide = NULL;
        fSlideArray = GSlide::CopyPairArray(&fSlideCount);
        fSlideIndex = 0;
        this->initSlide();
    }

    virtual ~SlideWindow() {
        for (int i = 0; i < fBitmapCount; ++i) {
            free(fBitmaps[i].fPixels);
        }
        delete fSlide;
        delete[] fSlideArray;
    }
    
protected:
    virtual void onDraw(GContext* ctx) {
        ctx->clear(GColor::Make(1, 1, 1, 1));

        GAutoRestoreToCount artc(ctx);
        ctx->save();
        this->scaleAboutCenter(ctx);

        fSlide->draw(ctx);

        this->requestDraw();
    }
    
    virtual bool onKeyPress(const XEvent& evt, KeySym sym) {
//        printf("----- keypress %lu\n", sym);
        switch (sym) {
            case XK_Up:
                if (fScale < gMaxScale) {
                    fScale *= 2;
                    this->updateTitle();
                }
                return true;
            case XK_Down:
                if (fScale > gMinScale) {
                    fScale /= 2;
                    this->updateTitle();
                }
                return true;
            case XK_Left:
                if (--fSlideIndex < 0) {
                    fSlideIndex = fSlideCount - 1;
                }
                this->initSlide();
                return true;
            case XK_Right:
                if (++fSlideIndex >= fSlideCount) {
                    fSlideIndex = 0;
                }
                this->initSlide();
                return true;
        }
        if ((sym <= 0x7F) && fSlide->handleKey(sym)) {
            return true;
        }
        return this->INHERITED::onKeyPress(evt, sym);
    }
    
private:
    typedef GXWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    int fileCount = argc - 1;
    int bitmapCount = 0;
    GBitmap* bitmaps = new GBitmap[fileCount];
    for (int i = 0; i < fileCount; ++i) {
        if (GReadBitmapFromFile(argv[i + 1], &bitmaps[i])) {
            bitmapCount += 1;
        }
    }

    return SlideWindow(640, 480, bitmaps, bitmapCount).run();
}

