/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GSlide.h"

GSlide::Rec* GSlide::gHead;

void GSlide::Register(Factory fact, void* refcon) {
    Rec* rec = new Rec;
    rec->fNext = GSlide::gHead;
    rec->fFact = fact;
    rec->fRefCon = refcon;
    gHead = rec;
}

GSlide::Pair* GSlide::CopyPairArray(int* count) {
    const Rec* rec = gHead;
    int counter = 0;
    while (rec) {
        counter += 1;
        rec = rec->fNext;
    }

    Pair* pair = new Pair[counter];
    rec = gHead;
    for (int i = 0; i < counter; ++i) {
        pair[i].fFact = rec->fFact;
        pair[i].fRefCon = rec->fRefCon;
        rec = rec->fNext;
    }

    *count = counter;
    return pair;
}

///////////////////////////////////////////////////////////////////////////////

GSlide::GSlide() {
    fSize.set(640, 480);
}

GSlide::~GSlide() {}

void GSlide::draw(GContext* ctx) {
    ctx->save();
    int sc = ctx->getSaveCount();
    this->onDraw(ctx);
    ctx->restoreToCount(sc);
}

bool GSlide::handleKey(int ascii) {
    return this->onHandleKey(ascii);
}

const char* GSlide::name() {
    return this->onName();
}

float GSlide::width() const { return fSize.fX; }
float GSlide::height() const { return fSize.fY; }
GRect GSlide::bounds() const { return GRect::MakeWH(fSize.fX, fSize.fY); }

