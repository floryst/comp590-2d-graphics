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
class GIRect;

class GContext {
public:
    GContext() {}
    virtual ~GContext() {}

    /**
     *  Copy information about the context's backend into the provided
     *  bitmap. Ownership of the pixel memory is not affected by this call,
     *  though the returned pixel address will remain valid for the lifetime
     *  of the context.
     */
    virtual void getBitmap(GBitmap*) const = 0;

    /**
     *  Set the entire context's pixels to the specified value.
     */
    virtual void clear(const GColor&) = 0;

    /**
     *  Fill the specified rectangle with the specified color, blending using
     *  SRC_OVER mode. If the rectangle is inverted (e.g. width or height < 0)
     *  or empty, then nothing is drawn.
     */
    virtual void fillIRect(const GIRect&, const GColor&) = 0;

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
};

#endif
