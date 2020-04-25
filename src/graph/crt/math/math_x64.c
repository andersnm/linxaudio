// Combine all .c source files to produce a single .obj file with the math
// library for an architecture. Workaround due to lack of .lib support in
// the linker.

#include "fltused.c"
#include "fminf.c"
#include "fmaxf.c"
#include "sse2/sinf4.c"
#include "sse2/cosf4.c"
#include "sse2/tanf4.c"
#include "sse2/logf4.c"
#include "sse2/log10f4.c"
#include "sse2/expf4.c"
#include "sse2/powf4.c"
