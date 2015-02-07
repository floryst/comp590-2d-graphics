


#include "GContext.h"
#include "GSlide.h"
#include "GPaint.h"
#include "GRect.h"
#include "GBitmap.h"
#include "GRandom.h"
#include "GTime.h"
#include "app_utils.h"

static float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

static GPoint lerp(const GPoint& a, const GPoint& b, float t) {
    GPoint result = { lerp(a.fX, b.fX, t), lerp(a.fY, b.fY, t) };
    return result;
}

static GColor lerp(const GColor& a, const GColor& b, float t) {
    GColor result;
    result.set(lerp(a.fA, b.fA, t), lerp(a.fR, b.fR, t),
               lerp(a.fG, b.fG, t), lerp(a.fB, b.fB, t));
    return result;
}

static const char gWalls[] =
    "1111111111111111111111111"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000011111111110000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1000000000000000000000001"
    "1111111111111111111111111";

static GRandom gRand;

struct IPoint {
    int fX, fY;
    
    void set(int x, int y) {
        fX = x;
        fY = y;
    }

    bool operator==(const IPoint& other) const {
        return other.fX == fX && other.fY == fY;
    }

    IPoint operator+(const IPoint& pt) const {
        IPoint sum = { fX + pt.fX, fY + pt.fY };
        return sum;
    }

    operator GPoint() {
        GPoint p = { (float)fX, (float)fY };
        return p;
    }
};

static void scale(GPoint pts[], int count, float scale) {
    for (int i = 0; i < count; ++i) {
        pts[i].fX *= scale;
        pts[i].fY *= scale;
    }
}

class Maze {
public:
    enum Room {
        kEmpty_Room,
        kWall_Room
    };
    
private:
    GColor  fWallColor, fFloorColor;
    Room*   fRooms;
    int     fWidth, fHeight;

public:
    Maze(const char walls[], int width, int height) {
        fWidth = width;
        fHeight = height;
        fRooms = new Room[width * height];
        
        fFloorColor.set(1, .9, .9, .9);
        fWallColor.set(1, 0, 0, 0);
    
        for (int i = 0; i < width*height; ++i) {
            fRooms[i] = walls[i]=='1' ? kWall_Room : kEmpty_Room;
        }
    }
    
    ~Maze() {
        delete[] fRooms;
    }

    Room room(int x, int y) const {
        GASSERT((unsigned)x < (unsigned)fWidth);
        GASSERT((unsigned)y < (unsigned)fHeight);
        return fRooms[y * fWidth + x];
    }

    bool hitTest(int x, int y) const {
        return kEmpty_Room != this->room(x, y);
    }

    void draw(GContext* ctx) {
        GRect r = GRect::MakeWH(fWidth, fHeight);
        r.inset(0.1, 0.1);
        GPaint paint;
        paint.setColor(fFloorColor);
        ctx->drawRect(r, paint);

        paint.setColor(fWallColor);

        for (int y = 0; y < fHeight; ++y) {
            for (int x = 0; x < fWidth; ++x) {
                if (kWall_Room == this->room(x, y)) {
                    r.setXYWH(x, y, 1, 1);
                    r.inset(0.1, 0.1);
                    ctx->drawRect(r, paint);
                }
            }
        }
    }
};

class Seg {
public:
    GColor  fColor;

    Seg() {
        fColor.set(1, gRand.nextF(), gRand.nextF(), gRand.nextF());
    }
};

class Snake {
    enum {
        kMaxSegs = 100,

        kMaxPos = kMaxSegs + 1,
        X0      = 12,
        Y0      = 20,

        MSEC_DELTA  = 150,
        SEG_GROW    = 5,
    };
    Seg     fSeg[kMaxSegs];
    int     fSegCount;

    IPoint  fPos[kMaxPos];
    int     fHeadIndex;

    IPoint  fMove;
    GMSec   fNow;

    bool    fCrash;

    static int next_index(int index) { return (index + 1) % kMaxPos; }
    static int prev_index(int index) { return (index + kMaxPos - 1) % kMaxPos; }
    static int comp_index(int index) { GASSERT(index >= 0); return index % kMaxPos; }

    bool validMove(int dx, int dy) const {
        return (fMove.fX != dx || fMove.fY != dy) &&
               (fMove.fX * dx + fMove.fY * dy >= 0);
    }

    GPoint  fSegPts[8];

    void drawSeg(GContext* ctx, const GColor& color, float x, float y) {
        GPaint paint;
        paint.setColor(color);

        ctx->save();
        ctx->translate(x + 0.5, y + 0.5);
        ctx->drawConvexPolygon(fSegPts, GARRAY_COUNT(fSegPts), paint);
        ctx->restore();
    }

public:
    Snake() {
        app_make_regular_poly(fSegPts, GARRAY_COUNT(fSegPts));
        scale(fSegPts, GARRAY_COUNT(fSegPts), 0.5f);

        this->reset();
    }

    void reset() {
        fSegCount = 10;
        for (int i = 0; i < fSegCount; ++i) {
            fPos[i].set(X0, Y0);
        }
        fHeadIndex = 0;
        fMove.set(0, 0);
        fNow = 0;

        fCrash = false;
    }

    void crash() { fCrash = true; }

    bool isMoving() const {
        return !fCrash && (fMove.fX || fMove.fY);
    }

    bool hitTest(int x, int y, bool skipHead) const {
        int index = fHeadIndex;
        int count = fSegCount;
        
        if (skipHead) {
            index = next_index(index);
            count -= 1;
        }
        for (int i = 0; i < count; ++i) {
            if (x == fPos[index].fX && y == fPos[index].fY) {
                return true;
            }
            index = next_index(index);
        }
        return false;
    }

    bool justStarting() const {
        // are we just beginning, in which case our head is on top of the rest
        // of our body, which would fail hitTest...
        int i0 = fHeadIndex;
        int i1 = next_index(i0);
        int i2 = next_index(i1);
        return (fPos[i0] == fPos[i1]) && (fPos[i1] == fPos[i2]);
    }

    bool selfIntersect() const {
        return !this->justStarting() &&
               this->hitTest(fPos[fHeadIndex].fX, fPos[fHeadIndex].fY, true);
    }

    void draw(GContext* ctx) {
        float t = 0;
        if (this->isMoving()) {
            GMSec now = GTime::GetMSec();
            GMSec delta = now - fNow;
            t = (float)delta / MSEC_DELTA;
        }

        // seed our future head
        fPos[prev_index(fHeadIndex)] = fPos[fHeadIndex] + fMove;

        int curr = comp_index(fHeadIndex + fSegCount - 1);
        GPoint pastPos;

        for (int i = fSegCount - 1; i >= 0; --i) {
            int prev = prev_index(curr);
            GPoint segPos = lerp(fPos[curr], fPos[prev], t);

            if (i < fSegCount - 1) {
                GPoint pos = lerp(segPos, pastPos, 0.5f);
                GColor col = lerp(fSeg[i].fColor, fSeg[i+1].fColor, 0.5f);
                this->drawSeg(ctx, col, pos.fX, pos.fY);
            }

            this->drawSeg(ctx, fSeg[i].fColor, segPos.fX, segPos.fY);
            curr = prev;
            pastPos = segPos;
        }
    }

    void move() {
        if (!this->isMoving()) {
            return;
        }

        GMSec now = GTime::GetMSec();
        GMSec delta = now - fNow;
        if (delta >= MSEC_DELTA) {
            GASSERT((unsigned)fHeadIndex < kMaxPos);
            IPoint next = fPos[fHeadIndex] + fMove;
            fHeadIndex = prev_index(fHeadIndex);
            GASSERT((unsigned)fHeadIndex < kMaxPos);
            fPos[fHeadIndex] = next;
            fNow = now;
        }
    }
    
    void grow() {
        if (fSegCount + SEG_GROW >= kMaxSegs) {
            return;
        }
        int tail = comp_index(fHeadIndex + fSegCount - 1);
        int index = tail;
        for (int i = 0; i < SEG_GROW; ++i) {
            index = next_index(index);
            fPos[index] = fPos[tail];
        }
        fSegCount += SEG_GROW;
    }

    bool handleKey(int ascii) {
        if (fCrash) {
            return false;
        }
        int dx = 0;
        int dy = 0;
        switch (ascii) {
            case 'i': dy = -1; break;
            case 'j': dx = -1; break;
            case 'k': dy =  1; break;
            case 'l': dx =  1; break;
            case ' ': break;
            default: return false;
        }

        if (this->validMove(dx, dy)) {
            fMove.set(dx, dy);
        }
        return true;
    }

    IPoint head() const { return fPos[fHeadIndex]; }
};

class SnakeSlide : public GSlide {
    enum {
        W = 25,
        H = 25,
        
        APPLE_COUNT = 10,
    };

    Maze* fMaze;
    Snake* fSnake;

    int     fAppleCount;
    IPoint  fAppleLoc;
    GPoint  fApplePts[15];

    bool    fCrash;

    IPoint freeLoc() const {
        int x, y;
        for (;;) {
            x = gRand.nextU() % W;
            y = gRand.nextU() % H;

            if (!fSnake->hitTest(x, y, false) && !fMaze->hitTest(x, y)) {
                break;
            }
        }
        IPoint loc = { x, y };
        return loc;
    }

    void drawApple(GContext* ctx) {
        GPaint paint;
        paint.setRGB(1, 0, 0);
        
        ctx->save();
        ctx->translate(fAppleLoc.fX + 0.5, fAppleLoc.fY + 0.5);
        ctx->drawConvexPolygon(fApplePts, GARRAY_COUNT(fApplePts), paint);
        ctx->restore();
    }

    void updateState() {
        if (!fSnake->isMoving()) {
            return;
        }

        IPoint head = fSnake->head();

        if (head == fAppleLoc) {
            if (fAppleCount > 0) {
                --fAppleCount;
                fSnake->grow();
                fAppleLoc = this->freeLoc();
            }
        }

        fSnake->move();

        if (fSnake->selfIntersect() || fMaze->hitTest(head.fX, head.fY)) {
            fSnake->crash();
            fCrash = true;
            return;
        }
    }

    void reset() {
        // check number of lives
        fSnake->reset();

        fAppleCount = APPLE_COUNT;
        fAppleLoc = this->freeLoc();
        fCrash = false;
    }

public:
    SnakeSlide() {
        fMaze = new Maze(gWalls, W, H);
        fSnake = new Snake;
        
        app_make_regular_poly(fApplePts, GARRAY_COUNT(fApplePts));
        scale(fApplePts, GARRAY_COUNT(fApplePts), 0.45f);

        this->reset();
    }

    ~SnakeSlide() {
        delete fSnake;
        delete fMaze;
    }

protected:
    virtual void onDraw(GContext* ctx) {
        GBitmap bm;
        ctx->getBitmap(&bm);
        float sx = (float)bm.width() / W;
        float sy = (float)bm.height() / H;
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

        fMaze->draw(ctx);
        this->drawApple(ctx);
        fSnake->draw(ctx);

        this->updateState();
    }

    virtual bool onHandleKey(int ascii) {
        if (fCrash) {
            if (' ' == ascii) {
                this->reset();
                return true;
            }
        }
        if (fSnake->handleKey(ascii)) {
            return true;
        }
        return this->GSlide::onHandleKey(ascii);
    }

    virtual const char* onName() {
        return "move[ I J K L ] reset[ space ]";
    }

};

static GSlide* create_snakeslide(void*) {
    return new SnakeSlide;
}

GSlide::Registrar SnakeSlide_reg(create_snakeslide);
