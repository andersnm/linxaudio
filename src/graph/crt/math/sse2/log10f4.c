/* This log10_ps was adapted with minimal changes from Julien Pommier's log_ps()
   based on the corresponding algorithm of the cephes math library.
*/

/* Copyright (C) 2007  Julien Pommier

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  (this is the zlib license)
*/

#include "math_sse2.h"

_PS_CONST(cephes_log10_2a, 3.0078125E-1);
_PS_CONST(cephes_log10_2b, 2.48745663981195213739E-4);
_PS_CONST(cephes_log10_ea, 4.3359375E-1);
_PS_CONST(cephes_log10_eb, 7.00731903251827651129E-4);

v4sf log10_ps(v4sf x) {
  v4sf e;
  v4sf mask;
  v4sf tmp;
  v4sf y;
  v4sf z;
#ifdef USE_SSE2
  v4si emm0;
#else
  v2si mm0, mm1;
#endif
  v4sf one = *(v4sf*)_ps_1;

  v4sf invalid_mask = _mm_cmple_ps(x, _mm_setzero_ps());

  x = _mm_max_ps(x, *(v4sf*)_ps_min_norm_pos);  /* cut off denormalized stuff */

#ifndef USE_SSE2
  /* part 1: x = frexpf(x, &e); */
  COPY_XMM_TO_MM(x, mm0, mm1);
  mm0 = _mm_srli_pi32(mm0, 23);
  mm1 = _mm_srli_pi32(mm1, 23);
#else
  emm0 = _mm_srli_epi32(_mm_castps_si128(x), 23);
#endif
  /* keep only the fractional part */
  x = _mm_and_ps(x, *(v4sf*)_ps_inv_mant_mask);
  x = _mm_or_ps(x, *(v4sf*)_ps_0p5);

#ifndef USE_SSE2
  /* now e=mm0:mm1 contain the really base-2 exponent */
  mm0 = _mm_sub_pi32(mm0, *(v2si*)_pi32_0x7f);
  mm1 = _mm_sub_pi32(mm1, *(v2si*)_pi32_0x7f);
  e = _mm_cvtpi32x2_ps(mm0, mm1);
  _mm_empty(); /* bye bye mmx */
#else
  emm0 = _mm_sub_epi32(emm0, *(v4si*)_pi32_0x7f);
  e = _mm_cvtepi32_ps(emm0);
#endif

  e = _mm_add_ps(e, one);

  /* part2: 
     if( x < SQRTHF ) {
       e -= 1;
       x = x + x - 1.0;
     } else { x = x - 1.0; }
  */
  mask = _mm_cmplt_ps(x, *(v4sf*)_ps_cephes_SQRTHF);
  tmp = _mm_and_ps(x, mask);
  x = _mm_sub_ps(x, one);
  e = _mm_sub_ps(e, _mm_and_ps(one, mask));
  x = _mm_add_ps(x, tmp);


  z = _mm_mul_ps(x,x);

  y = *(v4sf*)_ps_cephes_log_p0;
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p1);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p2);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p3);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p4);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p5);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p6);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p7);
  y = _mm_mul_ps(y, x);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_log_p8);
  y = _mm_mul_ps(y, x);

  y = _mm_mul_ps(y, z);
  
  tmp = _mm_mul_ps(z, *(v4sf*)_ps_0p5);
  y = _mm_sub_ps(y, tmp);


/* multiply log of fraction by log10(e)
 * and base 2 exponent by log10(2)
  z = (x + y) * L10EB;  // accumulate terms in order of size
  z += y * L10EA; 
  z += x * L10EA; 
  
  z += e * L102B; 
  z += e * L102A; 
  */
  z = _mm_add_ps(x, y);
  z = _mm_mul_ps(z, *(v4sf*)_ps_cephes_log10_eb);
  tmp = _mm_mul_ps(y, *(v4sf*)_ps_cephes_log10_ea);
  z = _mm_add_ps(z, tmp);
  tmp = _mm_mul_ps(x, *(v4sf*)_ps_cephes_log10_ea);
  z = _mm_add_ps(z, tmp);

  tmp = _mm_mul_ps(e, *(v4sf*)_ps_cephes_log10_2b);
  z = _mm_add_ps(z, tmp);
  tmp = _mm_mul_ps(e, *(v4sf*)_ps_cephes_log10_2a);
  z = _mm_add_ps(z, tmp);

  z = _mm_or_ps(z, invalid_mask); // negative arg will be NAN
  return z;
}
