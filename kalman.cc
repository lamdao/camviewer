//--------------------------------------------------------------------------
// kalman.cc - Simple implementation of Kalman filter using SSE instructions
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
#include "float4.h"
#include <cstdio>
//--------------------------------------------------------------------------
#ifndef gain
#define gain	0.40f
#endif
//--------------------------------------------------------------------------
#ifndef nvar
#define nvar	0.05f
#endif
//--------------------------------------------------------------------------
namespace KalmanFilter {
//--------------------------------------------------------------------------
static int bsize;
static float *pv;
static float4 nv(nvar), one(1.0f), gv(gain), vg(1.0f - gain);
//--------------------------------------------------------------------------
void Init(int size)
{
	bsize = size;
	pv = (float *)valloc(size * sizeof(float));
	for (int n = 0; n < size; n++) pv[n] = nvar;
}
//--------------------------------------------------------------------------
static inline void SSE_Filter(uchar *src, uchar *dst, float *nsv)
{
	float4 d(dst), s(src), n(nsv);
	float4 k = n / (n + nv);
	(n * (one - k)).save(nsv);
	(gv * d + vg * s + k * (s - d)).save(dst);
}
//--------------------------------------------------------------------------
static inline void CPU_Filter(uchar *src, uchar *dst, float *nsv)
{
	float p = *src, n = *dst;
	float k = *nsv / (*nsv + nvar); *nsv *= (1.0f - k);
	*dst = (uchar)(gain * p + (1.0f - gain) * n + k * (n - p));
}
//--------------------------------------------------------------------------
void Execute(uchar *src, uchar *dst)
{
#ifndef USE_CPU
	#pragma omp parallel for
	for (int i = 0; i < bsize; i += 4) {
		SSE_Filter(&src[i], &dst[i], &pv[i]);
	}
#else
	#pragma omp parallel for
	for (int i = 0; i < bsize; i += 1) {
		CPU_Filter(&src[i], &dst[i], &pv[i]);
	}
#endif
}
//--------------------------------------------------------------------------
void Release()
{
	free(pv);
}
//--------------------------------------------------------------------------
}
