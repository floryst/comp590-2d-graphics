/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GSlide.h"
#include "GColor.h"

class RedSlide : public GSlide {
protected:
    virtual void onDraw(GContext* ctx) {
        ctx->clear(GColor::Make(1, 1, 0, 0));
    }

    virtual const char* onName() {
        return "red";
    }

public:
    static GSlide* Create(void*) {
        return new RedSlide;
    }
};

GSlide::Registrar RedSlide_reg(RedSlide::Create);
