//----------------------------------------------------------------------------
// A simple framebuffer application for camera monitoring using V4L2 library.
// Images captured from camera is stablized/denoised by Kalman Filter.
//----------------------------------------------------------------------------
#if __GNUC__ > 4
#define _DEFAULT_SOURCE
#else
#define _BSD_SOURCE
#endif
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
//----------------------------------------------------------------------------
#include "camera.h"
#include "kalman.h"
#include "timer.h"
//----------------------------------------------------------------------------
bool ready = true, kalman = true;
int width = 640, height = 480;
//----------------------------------------------------------------------------
namespace FrameBuffer
{
int vfb = -1;
int size, bsize, bpp, vgap, vofs;
int fbsize;
//----------------------------------------------------------------------------
uchar *image = 0, *fbimage;
//----------------------------------------------------------------------------
void display_rgb15()
{
	int idx = vofs;
	uchar *src = (uchar *)image;
	for (int y = 0; y < height; y++, idx += vgap) {
		unsigned short *dst = (unsigned short *)&fbimage[idx];
		for (int x = 0; x < width; x++, src += 3) {
			*dst++ = ((src[0] >> 3) << 10) | ((src[1] >> 3) << 5) | (src[2] >> 3);
		}
	}
}
//----------------------------------------------------------------------------
void display_rgb16()
{
	int idx = vofs;
	uchar *src = (uchar *)image;
	for (int y = 0; y < height; y++, idx += vgap) {
		unsigned short *dst = (unsigned short *)&fbimage[idx];
		for (int x = 0; x < width; x++, src += 3) {
			*dst++ = ((src[0] >> 3) << 11) | ((src[1] >> 2) << 5) | (src[2] >> 3);
		}
	}
}
//----------------------------------------------------------------------------
void display_rgb24()
{
	int idx = vofs, wsize = 3 * width;
	uchar *src = (uchar *)image;
	for (int y = 0; y < height; y++, idx += vgap) {
		uchar *dst = &fbimage[idx];
		memcpy(dst, src, wsize);
		src += wsize;
	}
}
//----------------------------------------------------------------------------
void display_rgb32()
{
	int idx = vofs;
	uchar *src = (uchar *)image;
	for (int y = 0; y < height; y++, idx += vgap) {
		int *dst = (int *)&fbimage[idx];
		for (int x = 0; x < width; x++, src += 3) {
			*dst++ = (*(int *)src) & 0xFFFFFF;
		}
	}
}
//----------------------------------------------------------------------------
void (*Display)() = NULL;
//----------------------------------------------------------------------------
bool SetDisplayFx(int b)
{
	switch (b) {
		case 32:
			Display = display_rgb32;
			break;
		case 24:
			Display = display_rgb24;
			break;
		case 16:
			Display = display_rgb16;
			break;
		case 15:
			Display = display_rgb15;
			break;
		default:
			fprintf(stderr, "Unsupported display mode %d-bit\n", b);
			return false;
	}
	return true;
}
//----------------------------------------------------------------------------
int Init(const char *dev)
{
	fb_var_screeninfo scr;
	fb_fix_screeninfo fix;
	vfb = open(dev, O_RDWR);
	if (vfb == -1) {
		printf("Error: cannot open framebuffer device.\n");
		exit(-1);
	}

	if (ioctl(vfb, FBIOGET_FSCREENINFO, &fix)) {
		printf("Error reading fixed information.\n");
		close(vfb);
		exit(-2);
	}

	if (ioctl(vfb, FBIOGET_VSCREENINFO, &scr)) {
		printf("Error: cannot retrieve screen info.\n");
		close(vfb);
		exit(-3);
	}

	if (!SetDisplayFx(scr.bits_per_pixel)) {
		close(vfb);
		exit(-4);
	}
	printf("screen = %d x %d\n", scr.xres, scr.yres);

	bpp = scr.bits_per_pixel / 8;
	vgap = fix.line_length;
	vofs = (scr.xres - width) * bpp;
	fbsize = fix.smem_len;
	fbimage = (uchar *)mmap(0, fbsize, PROT_READ|PROT_WRITE, MAP_SHARED, vfb, 0);
	size = width * height;
	bsize = 3 * size;

	return vfb;
}
//----------------------------------------------------------------------------
void Update(const void *frame)
{
	if (!image) {
		image = (uchar *)valloc(bsize);
		memcpy(image, frame, bsize);
	} else {
		register uchar *src = (uchar *)frame;
		register uchar *dst = (uchar *)image;
		if (kalman) {
			KalmanFilter::Execute(src, dst);
		} else {
			memcpy(dst, src, bsize);
		}
	}
	Display();
}
//----------------------------------------------------------------------------
void Release()
{
	munmap(fbimage, fbsize);
	close(vfb);
	free(image);
}
//----------------------------------------------------------------------------
} // namespace FrameBuffer
//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	bool show_time = true;
	int cfd = Camera::Open("/dev/video0", width, height, 30);
	int vfb = FrameBuffer::Init("/dev/fb0");

	KalmanFilter::Init(FrameBuffer::bsize);
	Timer::Start();
	while (ready) {
		timeval tv = {0, 20000};
		fd_set rfds;

		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(cfd, &rfds);
		select(cfd+1, &rfds, 0, 0, &tv);
		if (FD_ISSET(cfd, &rfds)) {
			void *frame = Camera::GetFrame();
			if (!frame) continue;

			FrameBuffer::Update(frame);
			if (show_time)
				Timer::Check();
		}
		if (FD_ISSET(0, &rfds)) {
			int k = getchar();
			if ('q' == k) break;
			if ('k' == k) kalman = !kalman;
			if ('t' == k) show_time = !show_time;
		}
	}
	KalmanFilter::Release();
	FrameBuffer::Release();
	Camera::Close();
	return 0;
}
//----------------------------------------------------------------------------
