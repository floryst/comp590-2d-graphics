/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GSlide.h"
#include "GColor.h"

class BlueSlide : public GSlide {
protected:
    virtual void onDraw(GContext* ctx) {
        ctx->clear(GColor::Make(1, 0, 0, 1));
    }

    virtual const char* onName() {
        return "blue";
    }

public:
    static GSlide* Create(void*) {
        return new BlueSlide;
    }
};

GSlide::Registrar BlueSlide_reg(BlueSlide::Create);
