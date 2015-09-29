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
