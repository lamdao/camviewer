//--------------------------------------------------------------------------
// float4.h - Implementation of operations on 4-tuple of float using SSE
//--------------------------------------------------------------------------
// Author: Lam H. Dao <daohailam(at)yahoo(dot)com>
//--------------------------------------------------------------------------
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//--------------------------------------------------------------------------
#ifndef __SSE_FLOAT4_H
#define __SSE_FLOAT4_H
//----------------------------------------------------------------------------
#include <xmmintrin.h>
//----------------------------------------------------------------------------
#ifndef uchar
#define uchar unsigned char
#endif
//----------------------------------------------------------------------------
class float4
{
public:
	__m128 v;

	float4(__m128 o): v(o) {}
	float4(float f) {v=_mm_set1_ps(f);}
	float4(float *f): v(_mm_load_ps(f)) {}
	float4(uchar *f) {
		__m128i t = _mm_cvtsi32_si128(*(int*)f);
		t = _mm_unpacklo_epi8(t, _mm_setzero_si128());
		t = _mm_unpacklo_epi16(t, _mm_setzero_si128());
		v = _mm_cvtepi32_ps(t);
	}

	operator __m128 () const { return v; };
	operator int () const {
		__m128i t = _mm_cvtps_epi32(v);
		t = _mm_packs_epi32(t, _mm_setzero_si128());
		t = _mm_packus_epi16(t, _mm_setzero_si128());
		return ((int *)&t)[0];
	}

	friend float4 operator + (const float4 &m1, const float4 &m2) {
		return _mm_add_ps(m1, m2);
	}
	friend float4 operator - (const float4 &m1, const float4 &m2) {
		return _mm_sub_ps(m1, m2);
	}
	friend float4 operator * (const float4 &m1, const float4 &m2) {
		return _mm_mul_ps(m1, m2);
	}
	friend float4 operator / (const float4 &m1, const float4 &m2) {
		return _mm_div_ps(m1, m2);
	}

	void save(float *f) const {
		_mm_store_ps(f, v);
	}
	void save(uchar *u) const {
		__m128i t = _mm_cvtps_epi32(v);
		t = _mm_packs_epi32(t, _mm_setzero_si128());
		t = _mm_packus_epi16(t, _mm_setzero_si128());
		*((int *)u) = *((int *)&t);
	}
};
//----------------------------------------------------------------------------
#endif
