#pragma once

#include <math.h>

#if defined(__cplusplus)
extern "C" {
#endif

// These are not provided by MSVC math.h

#if defined(_MSC_VER) && _MSC_VER < 1200

float fminf(float a, float b);
float fmaxf(float a, float b);

#endif

#if defined(__cplusplus)
}
#endif
