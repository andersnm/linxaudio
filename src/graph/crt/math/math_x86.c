// Combine all .c source files to produce a single .obj file with the math
// library for an architecture. Workaround due to lack of .lib support in
// the linker.

#include "fltused.c"
#include "fminf.c"
#include "fmaxf.c"
#include "x87/msvcsupp.c"
