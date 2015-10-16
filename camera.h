//--------------------------------------------------------------------------
// camera.h - Simple camera control interfaces using V4L2 library
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
#ifndef __V4L_CAMERA_H
#define __V4L_CAMERA_H
//--------------------------------------------------------------------------
#include <linux/videodev2.h>
#include <libv4l2.h>
//--------------------------------------------------------------------------
namespace Camera
{
int Open(const char *device, int width, int height, int fps);
void SetCaptureFormat(__u32 format);
void *GetFrame(void);
void Close(void);
}
//--------------------------------------------------------------------------
#endif
