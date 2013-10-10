/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GXWindow_DEFINED
#define GXWindow_DEFINED

#define GContext    GContext_hidden

//#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#undef GContext

#include "GContext.h"

class GXWindow {
public:
    int run();

protected:
    GXWindow(int initial_width, int initial_height);
    virtual ~GXWindow();

    virtual void onDraw(GContext*) {}
    virtual void onResize(int w, int h) {}
    virtual bool onKeyPress(const XEvent&) { return false; }
    
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    
    void setTitle(const char title[]);
    void requestDraw();
    void setReadyToQuit() { fReadyToQuit = true; }
    
private:
    Display*    fDisplay;
    Window      fWindow;
    GC          fGC;
    
    GContext* fCtx;
    int fWidth;
    int fHeight;
    bool fReadyToQuit;
    bool fNeedDraw;

    bool handleEvent(XEvent*);
    void drawContextToWindow();
    void drawBitmap(const GBitmap&, int x, int y);
};

#endif
