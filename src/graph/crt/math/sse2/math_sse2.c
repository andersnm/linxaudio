#include "math_sse2.h"

float log10f_sse2(float x) {
	v4sf xx = _mm_set_ps1(x);
	v4sf ret = log10_ps(xx);
	return ret.m128_f32[0];
}

float tanf_sse2(float x) {
	v4sf xx = _mm_set_ps1(x);
	v4sf ret = tan_ps(xx);
	return ret.m128_f32[0];
}

float cosf_sse2(float x) {
	v4sf xx = _mm_set_ps1(x);
	v4sf ret = _vdecl_cosf4(xx);
	return ret.m128_f32[0];
}

float sinf_sse2(float x) {
	v4sf xx = _mm_set_ps1(x);
	v4sf ret = _vdecl_sinf4(xx);
	return ret.m128_f32[0];
}

float powf_sse2(float x, float y) {
	v4sf xx = _mm_set_ps1(x);
	v4sf yy = _mm_set_ps1(y);
	v4sf ret = pow_ps(xx, yy);
	return ret.m128_f32[0];
}

#if !defined(_LIB)

#pragma function(log10f, tanf, sinf, cosf, powf, fmodf)

float log10f(float x) {
	return log10f_sse2(x);
}

float tanf(float x) {
	return tanf_sse2(x);
}

float cosf(float x) {
	return cosf_sse2(x);
}

float sinf(float x) {
	return sinf_sse2(x);
}

float powf(float x, float y) {
	return powf_sse2(x, y);
}

float fmodf(register const float n, register const float d) {
  register float q;
  if (d == 0.0)
    return 0;
  q = n/d;
  if (q < 0.0)
    q = (float)(unsigned long)(q - 1.0);
  else
    q = (float)(unsigned long)(q);
  return n - q*d; 
}

#endif
