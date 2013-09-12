/**
 *  Copyright 2013 Mike Reed
 *
 *  COMP 590 -- Fall 2013
 */

#ifndef GTypes_DEFINED
#define GTypes_DEFINED

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 *  This header should be (directly or indirectly) included by all other
 *  headers and cpp files for COMP 590.
 */

/**
 *  Call g_crash() when you want to halt execution
 */
static inline void g_crash() {
    fprintf(stderr, "g_crash called\n");
    *(int*)0x50FF8001 = 12345;
}

#ifdef NDEBUG
    #define GASSERT(pred)
#else
    #define GASSERT(pred)   do { if (!(pred)) g_crash(); } while (0)
#endif

/**
 *  Given an array (not a pointer), this macro will return the number of
 *  elements declared in that array.
 */
#define GARRAY_COUNT(array) (int)(sizeof(array) / sizeof(array[0]))

#endif
