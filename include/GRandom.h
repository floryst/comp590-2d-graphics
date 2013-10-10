/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GRandom_DEFINED
#define GRandom_DEFINED

#include "GTypes.h"

class GRandom {
public:
    GRandom(uint32_t seed = 0) : fSeed(seed) {}

    uint32_t nextU() {
        fSeed = fSeed * 1664525 + 1013904223;   // numerical recipies
        return fSeed;
    }

    int32_t nextS() {
        return (int32_t)this->nextU();
    }
    
    float nextF() {
        return ((float)(this->nextU() & 0xFFFFFF)) / ((float)(1 << 24));
    }

    int nextRange(int min, int max) {
        GASSERT(max >= min);
        return min + this->nextU() % (max - min + 1);
    }

private:
    uint32_t fSeed;
};

#endif

