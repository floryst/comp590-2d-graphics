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

class GContext {
public:
    GContext() {}
    virtual ~GContext() {}

    /**
     *  Return the information about the context's bitmap.
     */
    virtual void getBitmap(GBitmap*) const = 0;

    /**
     *  Set the entire context's pixels to the specified value.
     */
    virtual void clear(const GColor&) = 0;

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
