/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GXWindow.h"
#include "GBitmap.h"
#include <stdio.h>

GXWindow::GXWindow(int width, int height) {
    fDisplay = XOpenDisplay(NULL);
    if (!fDisplay) {
        fprintf(stderr, "can't open xdisplay\n");
        return;
    }

    fWidth = width;
    fHeight = height;
    fReadyToQuit = false;
    
    int screenNo = DefaultScreen(fDisplay);
    Window root = RootWindow(fDisplay, screenNo);
    fWindow = XCreateSimpleWindow(fDisplay, root, 0, 0, width, height, 1,
                                  BlackPixel(fDisplay, screenNo),
                                  WhitePixel(fDisplay, screenNo));
    
    XSelectInput(fDisplay, fWindow, StructureNotifyMask | ExposureMask | KeyPressMask);
    XMapWindow(fDisplay, fWindow);

    fGC = XCreateGC(fDisplay, fWindow, 0, NULL);
    fCtx = GContext::Create(width, height);
}

GXWindow::~GXWindow() {
    delete fCtx;

    if (fDisplay) {
        XFreeGC(fDisplay, fGC);
        XDestroyWindow(fDisplay, fWindow);
        XCloseDisplay(fDisplay);
    }
}

void GXWindow::setTitle(const char title[]) {
    XStoreName(fDisplay, fWindow, title);
}

void GXWindow::requestDraw() {
    if (!fNeedDraw) {
        fNeedDraw = true;
        
        XEvent evt;
        memset(&evt, 0, sizeof(evt));
        evt.type = Expose;
        evt.xexpose.display = fDisplay;
        XSendEvent(fDisplay, fWindow, false, ExposureMask, &evt);
    }
}

bool GXWindow::handleEvent(XEvent* evt) {
    switch (evt->type) {
        case ConfigureNotify: {
            const int w = evt->xconfigure.width;
            const int h = evt->xconfigure.height;
            if (w != fWidth || h != fHeight) {
                fWidth = w;
                fHeight = h;
                this->onResize(w, h);
                
                delete fCtx;
                fCtx = GContext::Create(w, h);
                // assume we will get called to redraw
            }
            return true;
        }
        case Expose:
            if (0 == evt->xexpose.count) {
                fNeedDraw = false;
                this->onDraw(fCtx);
                this->drawContextToWindow();
            }
            return true;
        case KeyPress: {
            char buffer[128];
            KeySym sym;
            memset(buffer, 0, sizeof(buffer));
            (void)XLookupString(&evt->xkey, buffer, sizeof(buffer), &sym, NULL);

            if (this->onKeyPress(*evt, sym)) {
                return true;
            }
            if (XK_Escape == sym) {
                this->setReadyToQuit();
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

void GXWindow::drawBitmap(const GBitmap& bm, int x, int y) {
    const int w = bm.width();
    const int h = bm.height();

    XImage image;
    memset(&image, 0, sizeof(image));
    
    image.width = w;
    image.height = h;
    image.format = ZPixmap;
    image.data = (char*)bm.pixels();
    image.byte_order = LSBFirst;
    image.bitmap_unit = 32;
    image.bitmap_bit_order = LSBFirst;
    image.bitmap_pad = 32;
    image.depth = 24;
    image.bytes_per_line = bm.rowBytes() - w * 4;
    image.bits_per_pixel = 32;
    
    if (XInitImage(&image)) {
        XPutImage(fDisplay, fWindow, fGC, &image, 0, 0, x, y, w, h);
    }
}

void GXWindow::drawContextToWindow() {
    GBitmap bitmap;
    fCtx->getBitmap(&bitmap);
    this->drawBitmap(bitmap, 0, 0);
}

int GXWindow::run() {
    if (!fDisplay) {
        return -1;
    }

    for (;;) {
        XEvent evt;
        XNextEvent(fDisplay, &evt);
        this->handleEvent(&evt);
        if (fReadyToQuit) {
            break;
        }
    }
    return 0;
}
