/**
 *  Copyright 2013 Forrest Li
 *
 *  COMP 590 -- Fall 2013
 */

#include "GSlide.h"
#include "GColor.h"
#include "GPaint.h"
#include "app_utils.h"
#include "GTime.h"
#include "GRandom.h"

static GRandom gRand;

static void randColor(GRandom& rand, GColor* color) {
	color->fA = 1;
	color->fR = rand.nextF();
	color->fG = rand.nextF();
	color->fB = rand.nextF();
}

class Object {
private:
	GPoint pts[20];

	float x, y;

	int tX, tY;

	float size;

	GMSec fTime;

	GPaint paint;

public:

	Object(float s) {
		x = y = 0;
		size = s;
		fTime = 0;

		GColor color;
		randColor(gRand, &color);
		paint.setColor(color);

		app_make_regular_poly(pts, 20);
	}

	void draw(GContext* ctx) {
		GMSec now = GTime::GetMSec();
		if (now - fTime >= 5) {
			x += ((tX-x)/(size));
			y += ((tY-y)/(size));
			fTime = now;
		}

		ctx->save();
		ctx->translate(x, y);
		ctx->scale(size, size);
		ctx->drawConvexPolygon(pts, 20, paint);
		ctx->restore();
	}

	void setTargetXY(int x, int y) {
		tX = x;
		tY = y;
	}
};

class ForrestInertialSlide : public GSlide {
private:
	enum {
		kNumObj = 50
	};

	Object* objs[kNumObj];

	GMSec fTime;

public:
	
	ForrestInertialSlide() {
		int i;
		for (int i = 0; i < kNumObj; ++i)
			objs[kNumObj-i-1] = new Object(i+15);

		fTime = 0;
	}

	~ForrestInertialSlide() {
		int i;
		for (int i = 0; i < kNumObj; ++i)
			delete objs[i];
	}

protected:
    virtual void onDraw(GContext* ctx) {
		GPaint paint;
		GRect rect;
		GBitmap bitmap;

        ctx->clear(GColor::Make(1, 1, 1, 1));

		ctx->getBitmap(&bitmap);

		GMSec now = GTime::GetMSec();
		if (now - fTime >= 1400) {
			fTime = now;

			int x = gRand.nextRange(0, bitmap.width());
			int y = gRand.nextRange(0, bitmap.height());

			for (int i = 0; i < kNumObj; ++i) {
				objs[i]->setTargetXY(x, y);
			}
		}
		for (int i = 0; i < kNumObj; ++i)
			objs[i]->draw(ctx);

    }

    virtual const char* onName() {
        return "forrest_inertial";
    }

public:
    static GSlide* Create(void*) {
        return new ForrestInertialSlide;
    }
};

GSlide::Registrar ForrestInertialSlide_reg(ForrestInertialSlide::Create);
