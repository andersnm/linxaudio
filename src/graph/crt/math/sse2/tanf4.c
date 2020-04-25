/* This tancot_ps/tan_ps/cot_ps is loosely based on Julien Pommier's sin_ps() 
   and the corresponding algorithm of the cephes math library. Code by andyw.
*/
/* Copyright (C) 2007  Julien Pommier
   Copyright (C) 2015  andyw

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

_PS_CONST(0p0001, 1.0e-4);
_PS_CONST(minus_1, -1.0f);
_PS_CONST(cephes_tancot_1, 9.38540185543E-3);
_PS_CONST(cephes_tancot_2, 3.11992232697E-3);
_PS_CONST(cephes_tancot_3, 2.44301354525E-2);
_PS_CONST(cephes_tancot_4, 5.34112807005E-2);
_PS_CONST(cephes_tancot_5, 1.33387994085E-1);
_PS_CONST(cephes_tancot_6, 3.33331568548E-1);

// cotflg must be 0 or -1 (all bits set)
// adapted from sin_ps from above and tancotf in cephes. code by andyw
static v4sf tancot_ps(v4sf x, v4si cotflg) {
  v4sf xmm1, xmm2, xmm3, sign_bit, y;
  v4sf z, zz;
  v4si emm0, emm2;

  sign_bit = x;
  /* take the absolute value */
  x = _mm_and_ps(x, *(v4sf*)_ps_inv_sign_mask);
  /* extract the sign bit (upper one) */
  sign_bit = _mm_and_ps(sign_bit, *(v4sf*)_ps_sign_mask);
  
  /* scale by 4/Pi */
  y = _mm_mul_ps(x, *(v4sf*)_ps_cephes_FOPI);

  /* store the integer part of y in mm0 */
  emm2 = _mm_cvttps_epi32(y);
  /* j=(j+1) & (~1) (see the cephes sources) */
  emm2 = _mm_add_epi32(emm2, *(v4si*)_pi32_1);
  emm2 = _mm_and_si128(emm2, *(v4si*)_pi32_inv1);
  y = _mm_cvtepi32_ps(emm2);

  /* The magic pass: "Extended precision modular arithmetic" 
	 tancot: z = ((x - y * DP1) - y * DP2) - y * DP3; */
  xmm1 = *(v4sf*)_ps_minus_cephes_DP1;
  xmm2 = *(v4sf*)_ps_minus_cephes_DP2;
  xmm3 = *(v4sf*)_ps_minus_cephes_DP3;
  xmm1 = _mm_mul_ps(y, xmm1);
  xmm2 = _mm_mul_ps(y, xmm2);
  xmm3 = _mm_mul_ps(y, xmm3);
  z = _mm_add_ps(x, xmm1);
  z = _mm_add_ps(z, xmm2);
  z = _mm_add_ps(z, xmm3);

  zz = _mm_mul_ps(z, z);

  //if( x > 1.0e-4 ) {
	//y = ((((( 9.38540185543E-3 * zz + 3.11992232697E-3) * zz + 2.44301354525E-2) * zz + 5.34112807005E-2) * zz + 1.33387994085E-1) * zz + 3.33331568548E-1) * zz * z + z;

  y = _mm_mul_ps(*(v4sf*)_ps_cephes_tancot_1, zz);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_tancot_2);
  y = _mm_mul_ps(y, zz);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_tancot_3);
  y = _mm_mul_ps(y, zz);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_tancot_4);
  y = _mm_mul_ps(y, zz);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_tancot_5);
  y = _mm_mul_ps(y, zz);
  y = _mm_add_ps(y, *(v4sf*)_ps_cephes_tancot_6);

  y = _mm_mul_ps(y, zz);
  y = _mm_mul_ps(y, z);
  y = _mm_add_ps(y, z);

  // create results for 4 cases:

  //(j&2)&&cotflg => -y
  emm0 = _mm_and_si128(*(v4si*)_pi32_2, emm2); // emm2=j&2
  emm0 = _mm_andnot_si128(_mm_cmpeq_epi32(emm0, _mm_setzero_si128()), _mm_set_epi32(~0, ~0, ~0, ~0)); // expand (j&2) to all bits
  emm0 = _mm_and_si128(emm0, cotflg); // emm0 = emm2&cotflg;
  xmm2 = _mm_castsi128_ps(emm0); // xmm2 = mask for first case
  xmm3 = _mm_mul_ps(y, *(v4sf*)_ps_minus_1); // xmm3 = -y
  xmm1 = _mm_and_ps(xmm3, xmm2); // xmm1 has results for first case, can OR remainders

  //(j&2)&&!cotflg => -1.0/y
  emm0 = _mm_and_si128(*(v4si*)_pi32_2, emm2); // emm2=j&2
  emm0 = _mm_andnot_si128(_mm_cmpeq_epi32(emm0, _mm_setzero_si128()), _mm_set_epi32(~0, ~0, ~0, ~0)); // expand (j&2) to all bits
  emm0 = _mm_andnot_si128(cotflg, emm0); // emm0 = emm2&!cotflg;
  xmm2 = _mm_castsi128_ps(emm0); // xmm2 = mask for second case
  xmm3 = _mm_div_ps(*(v4sf*)_ps_minus_1, y); // xmm3 = -1/y
  xmm2 = _mm_and_ps(xmm3, xmm2); // xmm2 has results for second case, can OR into xmm1
  xmm1 = _mm_or_ps(xmm1, xmm2); // xmm1 has results for first and second cases

  //!(j&2)&&cotflg => 1.0/y
  emm0 = _mm_and_si128(*(v4si*)_pi32_2, emm2); // emm0=emm2&2
  emm0 = _mm_cmpeq_epi32(emm0, _mm_setzero_si128()); // expand !(j&2) to all bits
  emm0 = _mm_and_si128(cotflg, emm0); // emm0 = emm0&cotflg;
  xmm2 = _mm_castsi128_ps(emm0); // xmm2 = mask for third case
  xmm3 = _mm_div_ps(*(v4sf*)_ps_1, y); // xmm3 = 1/y
  xmm2 = _mm_and_ps(xmm3, xmm2); // xmm2 has results for third case, can OR into xmm1
  xmm1 = _mm_or_ps(xmm1, xmm2); // xmm1 has results for first, second and third cases

  //!(j&2)&&!cotflg => y
  emm0 = _mm_and_si128(*(v4si*)_pi32_2, emm2); // emm2=j&2
  emm0 = _mm_cmpeq_epi32(emm0, _mm_setzero_si128()); // expand !(j&2) to all bits

  emm0 = _mm_andnot_si128(cotflg, emm0); // emm0 = emm0&!cotflg;
  xmm2 = _mm_castsi128_ps(emm0); // xmm2 = mask for fourth case
  xmm2 = _mm_and_ps(y, xmm2); // xmm2 has results for third case, can OR into xmm1
  xmm1 = _mm_or_ps(xmm1, xmm2); // xmm1 has results for all cases

  //sign*y
  return _mm_xor_ps(xmm1, sign_bit);
}

v4sf _vdecl_tanf4(v4sf x)	{
	v4si cotflg = _mm_setzero_si128();
	return tancot_ps(x, cotflg);
}

v4sf _vdecl_cotf4(v4sf x)	{
	v4si cotflg = _mm_set_epi32(-1, -1, -1, -1);
	return tancot_ps(x, cotflg);
}
