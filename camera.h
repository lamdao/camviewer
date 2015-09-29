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
