/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GContext_DEFINED
#define GContext_DEFINED

#include "GTypes.h"

class GBitmap;
class GColor;
class GPaint;
class GPoint;
class GRect;

class GContext {
public:
    GContext();
    virtual ~GContext();

    /**
     *  Push a copy of the CTM onto an internal stack. Subsequent changes to the
     *  CTM (e.g. scale, translate) are retained until the balancing call to
     *  restore() which pops the copy off the internal stack and copies it back
     *  into the CTM.
     */
    void save();
    void restore();
    int getSaveCount() const { return fSaveCount; }
    void restoreToCount(int count);

    /**
     *  Preconcat the CTM with the specified translation.
     */
    virtual void translate(float tx, float ty) = 0;
    
    /**
     *  Preconcat the CTM with the specified scale.
     */
    virtual void scale(float sx, float sy) = 0;

    /**
     *  Copy information about the context's backend into the provided
     *  bitmap. Ownership of the pixel memory is not affected by this call,
     *  though the returned pixel address will remain valid for the lifetime
     *  of the context.
     */
    virtual void getBitmap(GBitmap*) const = 0;

    /**
     *  Set the entire context's pixels to the specified color.
     */
    virtual void clear(const GColor&) = 0;

    /**
     *  Draw the specified rectangle with the specified paint, blending using
     *  SRC_OVER mode. If the rectangle is inverted (e.g. width or height < 0)
     *  or empty, then nothing is drawn.
     *
     *  The rectangle is transform by the CTM.
     */
    virtual void drawRect(const GRect&, const GPaint&) = 0;

    /**
     *  Draw the specified bitmap, positioning its top-left corner at (x, y).
     *  Apply the paint's alpha to the bitmap and then blend it using
     *  SRC_OVER.
     *  If alpha is outside of the unit interval [0...1] it will be pinned to
     *  the nearest legal value.
     *  Note that the RGB in the paint are ignored.
     *
     *  The bitmap's position and size are transformed by the CTM.
     */
    virtual void drawBitmap(const GBitmap&, float x, float y, const GPaint&) = 0;

    /**
     *  Fill the triangle with the specified paint, blending using SRC_OVER mode.
     */
    virtual void drawTriangle(const GPoint vertices[3], const GPaint&) = 0;

    /**
     *  Fill the convex polygon with the specified paint, blending using
     *  SRC_OVER mode. The base implementation calls drawTriangle repeatedly,
     *  but subclass may override this behavior.
     */
    virtual void drawConvexPolygon(const GPoint vertices[], int count,
                                   const GPaint&);

    /**
     *  Create a new context that will draw into the specified bitmap. The
     *  caller is responsible for managing the lifetime of the pixel memory.
     *  If the new context cannot be created, return NULL.
     */
    static GContext* Create(const GBitmap&);

    /**
     *  Create a new context is sized to match the requested dimensions. The
     *  context is responsible for managing the lifetime of the pixel memory.
     *  If the new context cannot be created, return NULL.
     */
    static GContext* Create(int width, int height);

protected:
    virtual void onSave() = 0;
    virtual void onRestore() = 0;

private:
    int fSaveCount;
};

/**
 *  Helper class to call restoreToCount() on a GContext
 */
class GAutoRestoreToCount {
public:
    GAutoRestoreToCount(GContext* ctx) : fCtx(ctx) {
        fSaveCount = ctx->getSaveCount();
    }

    ~GAutoRestoreToCount() {
        fCtx->restoreToCount(fSaveCount);
    }

private:
    GContext* fCtx;
    int       fSaveCount;
};


#endif
