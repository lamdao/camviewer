#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
//--------------------------------------------------------------------------
namespace Camera {
//--------------------------------------------------------------------------
static struct {
	void * start;
	size_t length;
} buffer;
//--------------------------------------------------------------------------
static __u32 pixfmt = V4L2_PIX_FMT_BGR24;	// default capture format
static bool mm = true;						// use memory map
static int fd = -1;							// camera handler
//--------------------------------------------------------------------------
static inline void Exit(const char* msg)
{
	fprintf(stderr, "%s error %d, %s\n", msg, errno, strerror(errno));
	exit(EXIT_FAILURE);
}
//--------------------------------------------------------------------------
static inline bool SendRequest(int req, void *param)
{
	int r;
	do {
		r = v4l2_ioctl(fd, req, param);
	} while (-1 == r && EINTR == errno);
	return r != -1;
}
//--------------------------------------------------------------------------
#define return_on_error(msg)	\
	switch (errno) {			\
		case EAGAIN:			\
			return 0;			\
		case EIO:				\
		default:				\
			puts(msg);			\
			return 0;			\
	}
//--------------------------------------------------------------------------
void SetFormat(__u32 format)
{
	pixfmt = format;
}
//--------------------------------------------------------------------------
void *GetFrame(void)
{
	struct v4l2_buffer buf;
	if (mm) {
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (!SendRequest(VIDIOC_DQBUF, &buf)) {
			return_on_error("* Query buffer error.");
		}
		if (!SendRequest(VIDIOC_QBUF, &buf)) {
			return_on_error("* Query buffer error.");
		}
	} else {
		if (-1 == v4l2_read(fd, buffer.start, buffer.length)) {
			return_on_error("* Read error.");
		}
	}
	return buffer.start;
}
//--------------------------------------------------------------------------
static void CaptureStop(void)
{
	if (mm) {
		enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (!SendRequest(VIDIOC_STREAMOFF, &type))
			Exit("VIDIOC_STREAMOFF");
	}
}
//--------------------------------------------------------------------------
static void CaptureStart(void)
{
	if (mm) {
		enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		struct v4l2_buffer buf;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = 0;
		if (!SendRequest(VIDIOC_QBUF, &buf))
			Exit("VIDIOC_QBUF");
		if (!SendRequest(VIDIOC_STREAMON, &type))
			Exit("VIDIOC_STREAMON");
	}
}
//--------------------------------------------------------------------------
static void DeviceRelease()
{
	if (mm) {
		if (-1 == v4l2_munmap(buffer.start, buffer.length))
			Exit("munmap");
	} else {
		free(buffer.start);
	}
}
//--------------------------------------------------------------------------
static void InitReadBuffer(uint buffer_size)
{
	buffer.length = buffer_size;
	buffer.start = valloc(buffer_size);

	if (!buffer.start) {
		fprintf (stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}
//--------------------------------------------------------------------------
static void InitMemoryMap(const char *device)
{
	struct v4l2_requestbuffers req;
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (!SendRequest(VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support memory mapping\n", device);
			exit(EXIT_FAILURE);
		} else {
			Exit("VIDIOC_REQBUFS");
		}
	}

	struct v4l2_buffer buf;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;

	if (!SendRequest(VIDIOC_QUERYBUF, &buf))
		Exit("VIDIOC_QUERYBUF");

	buffer.length = buf.length;
	buffer.start = v4l2_mmap(NULL, buf.length,
						PROT_READ | PROT_WRITE,
						MAP_SHARED, fd, buf.m.offset);

	if (MAP_FAILED == buffer.start)
		Exit("v4l2 memory map");
}
//--------------------------------------------------------------------------
static void SetFrameRate(int fps)
{
	struct v4l2_streamparm framerate;
	// set the frame interval
	framerate.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	framerate.parm.capture.timeperframe.numerator = 1;
	framerate.parm.capture.timeperframe.denominator = fps;
	if (!SendRequest(VIDIOC_S_PARM, &framerate))
		fprintf(stderr, "Unable to set frame interval.\n");
}
//--------------------------------------------------------------------------
static void DeviceInit(const char *device, int width, int height, int fps)
{
	struct v4l2_capability cap;
	struct v4l2_format fmt;

	if (!SendRequest(VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is not a V4L2 device\n", device);
			exit(EXIT_FAILURE);
		} else {
			Exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is not a video capture device\n", device);
		exit(EXIT_FAILURE);
	}

	if (mm) {
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s does not support streaming I/O\n", device);
			exit(EXIT_FAILURE);
		}
	} else {
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf(stderr, "%s does not support read I/O\n", device);
			exit(EXIT_FAILURE);
		}
	}

	// v4l2_format
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	fmt.fmt.pix.pixelformat = pixfmt;

	if (!SendRequest(VIDIOC_S_FMT, &fmt))
		Exit("VIDIOC_S_FMT");

	if (fmt.fmt.pix.pixelformat != pixfmt) {
		fprintf(stderr, "Libv4l didn't accept format. Can't proceed.\n");
		exit(EXIT_FAILURE);
	}

	// VIDIOC_S_FMT may change width and height.
	if (width != fmt.fmt.pix.width) {
		width = fmt.fmt.pix.width;
		fprintf(stderr, "Image width set to %i by device %s.\n", width, device);
	}

	if (height != fmt.fmt.pix.height) {
		height = fmt.fmt.pix.height;
		fprintf(stderr, "Image height set to %i by device %s.\n", height, device);
	}
	
	SetFrameRate(fps);
	if (mm)
		InitMemoryMap(device);
	else
		InitReadBuffer(fmt.fmt.pix.sizeimage);

	CaptureStart();
}
//--------------------------------------------------------------------------
int Open(const char *device, int width, int height, int fps)
{
	struct stat st;
	// check if device exists
	if (-1 == stat(device, &st)) {
		fprintf(stderr, "Device '%s' not found: %d, %s\n", device, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	// check if it's char device
	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is not a video capture device\n", device);
		exit(EXIT_FAILURE);
	}
	// open device
	fd = v4l2_open(device, O_RDWR|O_NONBLOCK, 0);
	// check if opening was successfull
	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n", device, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	DeviceInit(device, width, height, fps);

	return fd;
}
//--------------------------------------------------------------------------
void Close(void)
{
	CaptureStop();
	DeviceRelease();
	if (-1 == v4l2_close(fd))
		Exit("close");
	fd = -1;
}
//--------------------------------------------------------------------------
} // namespace Camera
