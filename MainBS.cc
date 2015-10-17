//----------------------------------------------------------------------------
// MainBS.cc - A simple camera broadcaster using V4L2 library and ZeroMQ.
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
#include <memory>
#include <string>
#include <zmq.hpp>
//----------------------------------------------------------------------------
#include "camera.h"
#include "kalman.h"
#include "buffer.h"
#include "timer.h"
#include "djpeg.h"
//----------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------
bool ready = true, kalman = true;
int width = 640, height = 480;
int psize = width * height, bsize = 3 * psize;
buffer_ptr<uchar> jpeg = CreateBuffer(bsize);
buffer_ptr<uchar> image;
//----------------------------------------------------------------------------
void Send(shared_ptr<zmq::socket_t> bcast, const void *frame)
{
	if (!image) {
		image = CreateBuffer(bsize);
		memcpy(image.get(), frame, bsize);
	} else {
		uchar *src = (uchar *)frame;
		uchar *dst = (uchar *)image.get();
		if (kalman) {
			KalmanFilter::Execute(src, dst);
		} else {
			memcpy(dst, src, bsize);
		}
	}
	size_t csize = DJpeg::Compress(image.get(), jpeg.get(), width, height);
	zmq::message_t msg(csize);
	memcpy(msg.data(), jpeg.get(), csize);
	bcast->send(msg);
}
//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	bool show_time = true;
	int cfd = Camera::Open("/dev/video0", width, height, 30);
	zmq::context_t context(1);
	shared_ptr<zmq::socket_t> bcast = make_shared<zmq::socket_t>(context, ZMQ_PUB);
	bcast->bind("tcp://*:1975");

	if (argc == 3 && string(argv[2]) == "-kf")
		kalman = false;

	zmq::pollitem_t items [] = {
		{ NULL,   0, ZMQ_POLLIN, 0 },
		{ NULL, cfd, ZMQ_POLLIN, 0 }
	};
	setvbuf(stdout, 0, _IONBF, 0);
	KalmanFilter::Init(bsize);
	Timer::Start();
	while (ready) {
		try {
		zmq::poll(&items[0], 2, 1);
		if (items[0].revents & ZMQ_POLLIN) {
			int k = getchar();
			if (k == 'q') break;
			if (k == 'k') kalman = !kalman;
			if (k == 't') show_time = !show_time;
		}
		if (items[1].revents & ZMQ_POLLIN) {
			void *frame = Camera::GetFrame();
			if (!frame) continue;

			Send(bcast, frame);
			if (show_time)
				Timer::Check();
		}
		} catch (zmq::error_t &e) {
		}
	}
	KalmanFilter::Release();
	Camera::Close();
	return 0;
}
//----------------------------------------------------------------------------
