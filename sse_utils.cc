//----------------------------------------------------------------------------
// SSE implementation of some utility functions used in motion detection
//----------------------------------------------------------------------------
// Author: Lam H. Dao <daohailam(at)yahoo(dot)com>
//----------------------------------------------------------------------------
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//----------------------------------------------------------------------------
#include <emmintrin.h>
#ifdef __SSSE3__
#include <tmmintrin.h>
#endif
//----------------------------------------------------------------------------
#include <math.h>
//----------------------------------------------------------------------------
#define uchar	unsigned char
#define u64		unsigned long long
#define i64		long long
//----------------------------------------------------------------------------
// Calculate sum of absolute differences (SAD) of 2 images
//----------------------------------------------------------------------------
int CalcSAD(uchar *im0, uchar *im1, int size)
{
	__m128i s = _mm_setzero_si128();
	__m128i *a = (__m128i *)im0;
	__m128i *b = (__m128i *)im1;
	for (int i = 0; i < size; i+=16) {
		__m128i t = _mm_sad_epu8(*a, *b);
		s = _mm_add_epi32(s, t);
		a += 1;
		b += 1;
	}
	int *p = (int *)&s;
//	printf("SAD = %d\n", p[0]+p[2]);
	return p[0] + p[2];
}
//----------------------------------------------------------------------------
#ifndef __SSSE3__
inline
__m128i _mm_abs_epi16(__m128i n)
{
	__m128i mask = _mm_srai_epi16(n, 15);
	return _mm_xor_si128(_mm_add_epi16(mask, n), mask);
}
#endif
//----------------------------------------------------------------------------
#ifndef __x86_64__
#define _mm_cvtsi64_si128(v)	_mm_set_epi64x (0, v)
#endif
//----------------------------------------------------------------------------
inline
__m128i _mm_load_epu8(u64 *u)
{
	__m128i m = _mm_cvtsi64_si128(*u);
	return _mm_unpacklo_epi8(m, _mm_setzero_si128());
}
//----------------------------------------------------------------------------
// Calculate differences of 2 images, and also return root mean absolute error
// value of the diffences (sqrt(SAD/num_pixels))
//   imd = |im0 - im1|
//   ret = sqrt(sum(imd) / size)
//----------------------------------------------------------------------------
float ImageDiff(uchar *im0, uchar *im1, uchar *imd, int size, uchar eps)
{
	uchar *r = imd;
	uchar *b = &r[size];

	int f, t = 0;
	u64 *x = (u64 *)im0;
	u64 *y = (u64 *)im1;
	u64 *p = (u64 *)r;
	for (int n = 0; n < size; n += 8) {
		__m128i s, m, z;
		s = _mm_sub_epi16(_mm_load_epu8(y), _mm_load_epu8(x));
		m = _mm_cmplt_epi16(s, _mm_setzero_si128());
		m = _mm_packs_epi16(m, _mm_setzero_si128());
		f = _mm_movemask_epi8(m);
		s = _mm_packs_epi16(_mm_abs_epi16(s), _mm_setzero_si128());
		z = _mm_sad_epu8(s, _mm_setzero_si128());
		m = _mm_cmpgt_epi8(s, _mm_set1_epi8(eps)); // discard all values < eps
		s = _mm_and_si128(s, m);
		_mm_storel_epi64((__m128i *)p, s); *b++ = f;
		t += *((int *)&z);
		p += 1;
		x += 1;
		y += 1;
	}
	return sqrt((float)t / size);
}
//----------------------------------------------------------------------------
