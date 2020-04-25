C standard library routines for Windows. Enables linking object files from
Visual C++ without default libraries. Includes mathext.h for missing fminf,
fmaxf definitions. Otherwise assumes code to be compiled with default headers.

== CRT targets ==
	- x86 with x87
	- x86 with SSE2
	- x64 with SSE2

== Implemented functions ==
	
math.h:
	sinf, cosf, tanf, logf, log10f, sqrtf, powf, fminf, fmaxf
	sinf4, cosf4 etc for auto-vectorised code

string.h:
	memcpy, memset
