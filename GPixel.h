/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GPixel_DEFINED
#define GPixel_DEFINED

#include "GTypes.h"

#define GPIXEL_SHIFT_A  24
#define GPIXEL_SHIFT_R  16
#define GPIXEL_SHIFT_G   8
#define GPIXEL_SHIFT_B   0

/**
 *  Defines our 32bit pixel to be just an int. It packes its components based
 *  on the defines GPIXEL_SHIFT_... for each component.
 */
typedef uint32_t GPixel;

#endif
