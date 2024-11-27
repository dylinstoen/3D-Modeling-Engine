/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef UTIL_H
#define UTIL_H

#include <enum.h>

class Util {
public:
    static std::string EnumToString() {
        return "";
    }

    // Functions to encode and decode integers bitwise as floats
    //   This is used to read back integer ids from floating
    //   point buffers. Because of the IEEE754 spec, directly
    //   converting integers bitwise to floats usually results
    //   in denormalized floats on CPU, which aren't supported
    //   on GPU. To fix this we force a nonzero exponent.
    static const int FLOAT_SIG_BITS = 23;
    static const int FLOAT_EXP_MASK = 1 + (1 << FLOAT_SIG_BITS);

    union float_int_union {
        float f;
        int i;
    };

    static int ftoi(float f) {
        float_int_union fiu;
        fiu.f = f;
        return fiu.i > 0?fiu.i - FLOAT_EXP_MASK:0;
    }
    static float itof(int i) {
        float_int_union fiu;
        fiu.i = i + FLOAT_EXP_MASK;
        return fiu.f;
    }

};

#endif // UTIL_H
