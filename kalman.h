//--------------------------------------------------------------------------
// kalman.h - Interfaces of Kalman filter using SSE instructions
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
#ifndef __KALMAN_FILTER_H
#define __KALMAN_FILTER_H
//--------------------------------------------------------------------------
#include "float4.h"
//--------------------------------------------------------------------------
namespace KalmanFilter
{
void Init(int size);
void Execute(uchar *src, uchar *dst);
void Release();
}
//--------------------------------------------------------------------------
#endif
