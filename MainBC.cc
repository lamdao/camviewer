//----------------------------------------------------------------------------
// MainBC.cc - A simple camera client (receiver) using ZeroMQ.
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
#include <memory>
#include <zmq.hpp>
#include "timer.h"
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
template<typename T>
using buffer_ptr = std::unique_ptr<T,std::function<void(T*)>>;
//----------------------------------------------------------------------------
buffer_ptr<uchar> CreateBuffer(int bsize)
{
	return buffer_ptr<uchar>((uchar*)valloc(bsize), [](uchar *b) {free(b);});
}
//----------------------------------------------------------------------------
class MainWindow: public WinControl
{
	int size;
	int bsize;
	XImage *image;
	GC context;
	buffer_ptr<uchar> buffer;
	zmq::context_t ctx;
	zmq::socket_t socket;

	bool ready, kalman;
public:
	MainWindow(const char *ip): WinControl(ExposureMask|KeyPressMask),
		ready(false), kalman(true), buffer(nullptr), ctx(1), socket(ctx, ZMQ_SUB)
	{
		setvbuf(stdout, 0, _IONBF, 0);
		display = XOpenDisplay(NULL);
		if (!display) {
			printf("* Error: Cannot open display.\n");
			return;
		}
		screen = DefaultScreen(display);
		ScreenVisual = DefaultVisual(display, screen);
		ScreenColormap = DefaultColormap(display, screen);
		Root = DefaultRootWindow(display);
		CreateHandler(width = 640, height = 480);
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

		std::cout << "Connecting to " << ip << ":1975 server…" << std::endl;
		std::string server = std::string("tcp://")+ip+":1975";
		socket.connect(server.c_str());
		socket.setsockopt(ZMQ_SUBSCRIBE, "FRAME", 5);
		ready = true;
	}

	~MainWindow()
	{
		KalmanFilter::Release();

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

		int bfd, xfd = ConnectionNumber(display);
		size_t bfd_size = sizeof(bfd);
		socket.getsockopt(ZMQ_FD, &bfd, &bfd_size);
		zmq::pollitem_t items [] = {
			{ NULL, xfd, ZMQ_POLLIN, 0 },
			{ socket, 0, ZMQ_POLLIN, 0 }
		};

		Timer::Start();
		XSync(display, False);
		while (ready) {
			zmq::poll(&items[0], 2, 1);

			if ((items[0].revents & ZMQ_POLLIN) && XPending(display)) {
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
			if (items[1].revents & ZMQ_POLLIN) {
				Update();
				Timer::Check();
			}
		}
	}

	void Update()
	{
		zmq::message_t frame;
		socket.recv(&frame);

		uchar *src = (uchar *)frame.data(); src += 5;
		uchar *dst = buffer.get();
		if (!dst) {
			buffer = CreateBuffer(bsize);
			dst = buffer.get();
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
	MainWindow cv(argv[1]);
	cv.Process();
	return 0;
}
//----------------------------------------------------------------------------
