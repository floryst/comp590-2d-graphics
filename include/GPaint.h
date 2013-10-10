/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GPaint_DEFINED
#define GPaint_DEFINED

#include "GColor.h"

class GPaint {
public:
    GPaint();
    GPaint(const GPaint&);
    ~GPaint();
    
    GPaint& operator=(const GPaint&);
    
//    bool isFilter() const { return fFilter; }
//    void setFilter(bool f) { fFilter = f; }
    
    float getAlpha() const { return fColor.fA; }
    void setAlpha(float a);
    
    const GColor& getColor() const { return fColor; }
    void setColor(const GColor& c);
    
private:
    GColor  fColor;
//    bool    fFilter;
};

#endif
