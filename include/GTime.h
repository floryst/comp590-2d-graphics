/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GTime_DEFINED
#define GTime_DEFINED

#include "GTypes.h"

typedef unsigned long GMSec;

class GTime {
public:
    static GMSec GetMSec();
};

#endif
