/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GSlide_DEFINED
#define GSlide_DEFINED

#include "GContext.h"
#include "GRect.h"

class GSlide {
public:
    typedef GSlide* (*Factory)(void*);

    GSlide();
    virtual ~GSlide();

    virtual void initWithBitmaps(const GBitmap[], int count) {}

    void draw(GContext*);
    bool handleKey(int ascii);
    const char* name();

    float width() const;
    float height() const;
    GRect bounds() const;

protected:
    // draw your content
    virtual void onDraw(GContext*) = 0;
    
    // ascii-keycode -- return true if you handled it
    virtual bool onHandleKey(int ascii) { return false; }

    // return the name for your slide
    virtual const char* onName() = 0;

private:
    GPoint fSize;

    struct Rec {
        Rec*    fNext;
        Factory fFact;
        void*   fRefCon;
    };
    static Rec* gHead;

public:
    static void Register(GSlide::Factory, void*);

    struct Pair {
        Factory fFact;
        void*   fRefCon;
    };
    
    // Array is allocated w/ new[]
    static Pair* CopyPairArray(int* count);

    class Registrar {
    public:
        Registrar(Factory fact, void* refcon = NULL) {
            GSlide::Register(fact, refcon);
        }
    };
};

#endif
