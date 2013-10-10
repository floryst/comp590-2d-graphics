/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#include "GTime.h"

#include <sys/time.h>

GMSec GTime::GetMSec() {
    struct timeval tv;

    if (gettimeofday(&tv, NULL)) {
        return 0;
    } else {
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
}

