//----------------------------------------------------------------------------
// MainXW.cc - A simple X-window camera monitoring using V4L2 library.
// Images captured from camera is stablized/denoised by Kalman Filter.
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
#if __GNUC__ > 4
#define _DEFAULT_SOURCE
#else
#define _BSD_SOURCE
#endif
//----------------------------------------------------------------------------
#include <cstdio>
#include <cstring>
#include "timer.h"
#include "camera.h"
#include "kalman.h"
#include "Window.h"
//----------------------------------------------------------------------------
int screen = 0;
Display *display = NULL;
//----------------------------------------------------------------------------
Visual *ScreenVisual;
Colormap ScreenColormap;
//----------------------------------------------------------------------------
Window Root;
//----------------------------------------------------------------------------
class MainWindow: public WinControl
{
	int cfd;
	int size;
	int bsize;
	uchar *buffer;
	XImage *image;
	GC context;

	bool ready, kalman;
public:
	MainWindow(const char *dev): WinControl(ExposureMask|KeyPressMask),
		ready(false), kalman(false), buffer(0)
	{
		setvbuf(stdout, 0, _IONBF, 0);
		cfd = Camera::Open(dev, width = 640, height = 480, 30);
		display = XOpenDisplay(NULL);
		if (!display) {
			printf("* Error: Cannot open display.\n");
			return;
		}
		screen = DefaultScreen(display);
		ScreenVisual = DefaultVisual(display, screen);
		ScreenColormap = DefaultColormap(display, screen);
		Root = DefaultRootWindow(display);
		CreateHandler(width, height);
		context = XCreateGC(display, handler, 0, 0);
		size = width * height;
		bsize = 3 * size;
		image = XCreateImage(display, ScreenVisual, 24, ZPixmap, 0,
							(char *)valloc(4*size), width, height, 32, 0);
		if (!image) {
			printf("* Error: Cannot create image buffer\n");
			return;
		}
		KalmanFilter::Init(bsize);

		Show();
		ready = true;
	}

	~MainWindow()
	{
		KalmanFilter::Release();
		Camera::Close();

		free(buffer);
		if (image) {
			XDestroyImage(image);
		}
		XFreeGC(display, context);
		XDestroyWindow(display, handler);
		XCloseDisplay(display);
	}

	void Process()
	{
		if (!ready)
			return;

		int xfd = ConnectionNumber(display);

		Timer::Start();
		while (ready) {
			timeval tv = {0, 20000};
			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(xfd, &rfds);
			FD_SET(cfd, &rfds);
			if (select(xfd+1, &rfds, 0, 0, &tv) <= 0)
				continue;
			if (XPending(display)) {
				XEvent e;
				XNextEvent(display, &e);
				if (e.type == KeyPress) {
					int key = ((XKeyEvent *)&e)->keycode;
					if (key == 9) { // escape was pressed
						ready = false;
						break;
					} else if (key == 45) { // 'k' was pressed
						kalman = !kalman;
					}
				}
				else if (e.type == ClientMessage)
					break;
			}
			if (FD_ISSET(cfd, &rfds)) {
				Update(Camera::GetFrame());
				Timer::Check();
			}
		}
	}

	void Update(const void *frame)
	{
		if (!frame) return;
		register uchar *src = (uchar *)frame;
		register uchar *dst = buffer;
		if (!dst) {
			dst = buffer = (uchar *)valloc(bsize);
			memcpy(dst, src, bsize);
		} else if (kalman) {
			KalmanFilter::Execute(src, dst);
		} else {
			memcpy(dst, src, bsize);
		}
		register int *out = (int *)image->data;
		for (int i = 0; i < size; i++, dst += 3) {
			out[i] = *((int *)dst) & 0xFFFFFF;
		}
		XPutImage(display, handler, context, image, 0, 0, 0, 0, width, height);
	}
};
//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	MainWindow cv(argv[1] != NULL ? argv[1] : "/dev/video0");
	cv.Process();
	return 0;
}
//----------------------------------------------------------------------------
