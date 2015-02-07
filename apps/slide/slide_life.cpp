/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GSlide.h"
#include "GBitmap.h"
#include "GColor.h"
#include "GPaint.h"
#include "GRandom.h"
#include "GTime.h"

static GRandom gRand;

class Board {
public:
    enum {
        N = 23
    };

    Board() {
        memset(fCell, 0, sizeof(fCell));
    }

    Board& operator=(const Board& src) {
        memcpy(fCell, src.fCell, sizeof(fCell));
        return *this;
    }

    void reseed(int32_t);
    void reseed() { this->reseed(gRand.nextU()); }
    void nextGeneration();

    bool get(int x, int y) const {
        GASSERT((unsigned)x < N);
        GASSERT((unsigned)y < N);
        return fCell[x][y];
    }

private:
    bool fCell[N][N];

    static int wrap(int index) {
        if (index < 0) {
            GASSERT(-1 == index);
            index = N - 1;
        } else if (index >= N) {
            GASSERT(N == index);
            index = 0;
        }
        return index;
    }

    bool check(int x, int y) const {
        return fCell[wrap(x)][wrap(y)];
    }

    int applyRule(int x, int y) const {
        int neighbors = this->check(x - 1, y - 1) +
                        this->check(x + 0, y - 1) +
                        this->check(x + 1, y - 1) +
                        this->check(x - 1, y + 0) +
                        this->check(x + 1, y + 0) +
                        this->check(x - 1, y + 1) +
                        this->check(x + 0, y + 1) +
                        this->check(x + 1, y + 1);
        if (fCell[x][y]) {
            return 2 == neighbors || 3 == neighbors;
        } else {
            return 3 == neighbors;
        }
    }
};

void Board::nextGeneration() {
    bool tmp[N][N];
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            tmp[x][y] = this->applyRule(x, y);
        }
    }
    memcpy(fCell, tmp, sizeof(tmp));
}

void Board::reseed(int32_t seed) {
    GRandom rand(seed);

    memset(fCell, 0, sizeof(fCell));
    for (int i = 0; i < N * N / 5; ++i) {
        int x = (rand.nextU() >> 17) % N;
        int y = (rand.nextU() >> 3) % N;
        fCell[x][y] = true;
    }
}

static void square_scale(GContext* ctx, float W, float H) {
    GBitmap bm;
    ctx->getBitmap(&bm);

    float sx = bm.width() / W;
    float sy = bm.height() / H;
    float dx = 0;
    float dy = 0;
    if (sy > sx) {
        sy = sx;
        dy = (bm.height() - sy * H) / 2;
    } else {
        sx = sy;
        dx = (bm.width() - sx * W) / 2;
    }
    
    ctx->translate(dx, dy);
    ctx->scale(sx, sy);
}

class LifeSlide : public GSlide {
    enum {
        MAX_GEN = 64
    };

    Board fBoard[3];
    GMSec fTime;
    int   fGeneration;

    void reseed() {
        fBoard[0].reseed();
        fBoard[1] = fBoard[2] = fBoard[0];
        fGeneration = 0;
    }

protected:
    virtual void onDraw(GContext* ctx) {
        GPaint paint;

        ctx->clear(GColor::Make(1, 1, 1, 1));
        
        square_scale(ctx, Board::N, Board::N);
        
        paint.setRGB(0.5, 0.5, 0.5);
        ctx->drawRect(GRect::MakeWH(Board::N, Board::N), paint);

        for (int y = 0; y < Board::N; ++y) {
            for (int x = 0; x < Board::N; ++x) {
                bool r = fBoard[0].get(x, y);
                bool g = fBoard[1].get(x, y);
                bool b = fBoard[2].get(x, y);
                if (r | g | b) {
                    paint.setRGB(r, g, b);
                    ctx->drawRect(GRect::MakeXYWH(x, y, 1, 1), paint);
                }
            }
        }

        GMSec now = GTime::GetMSec();
        if (now - fTime >= 500) {
            fTime = now;
            fBoard[2] = fBoard[1];
            fBoard[1] = fBoard[0];
            fBoard[0].nextGeneration();
            fGeneration += 1;
            if (fGeneration > MAX_GEN) {
                this->reseed();
            }
        }
    }

    virtual bool onHandleKey(int ascii) {
        if (' ' == ascii) {
            this->reseed();
            return true;
        }
        return this->GSlide::onHandleKey(ascii);
    }

    virtual const char* onName() {
        return "Life";
    }
    
public:
    LifeSlide() {
        fBoard[0].reseed();
        fBoard[1] = fBoard[2] = fBoard[0];
        fTime = 0;
    }

    static GSlide* Create(void*) {
        return new LifeSlide;
    }
};

GSlide::Registrar LifeSlide_reg(LifeSlide::Create);
