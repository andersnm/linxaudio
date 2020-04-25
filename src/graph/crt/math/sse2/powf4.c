/* pow_ps implemented as exp(y*log(x)) using Julien Pommier's exp_ps() and
   log_ps().
*/

#include "math_sse2.h"

extern v4sf _vdecl_logf4(v4sf x);
extern v4sf _vdecl_expf4(v4sf x);

v4sf _vdecl_powf4(v4sf x, v4sf y) {
	x = _vdecl_logf4(x);
	x = _mm_mul_ps(y, x);
	return _vdecl_expf4(x);
}
