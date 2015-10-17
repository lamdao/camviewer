#ifndef __DJPEG_H
#define __DJPEG_H
//--------------------------------------------------------------------------
#include <vector>
//--------------------------------------------------------------------------
#ifndef uchar
#define uchar unsigned char
#endif
//--------------------------------------------------------------------------
namespace DJpeg {
//--------------------------------------------------------------------------
// Decompress a Jpeg file-in-memory buffer (data) and return a RGB buffer
// Use this function when we know for sure the size of uncompressed image
// and pre-allocated it
//--------------------------------------------------------------------------
uchar *Decompress(void *jraw, size_t jsize, uchar *image, int &width, int &height);
//--------------------------------------------------------------------------
// Decompress a Jpeg file-in-memory buffer (data) and return a RGB buffer
//--------------------------------------------------------------------------
std::vector<uchar> Decompress(void *jraw, size_t jsize, int &width, int &height);
//--------------------------------------------------------------------------
// Decompress a Jpeg file-in-memory buffer (data) and return a RGB buffer
//--------------------------------------------------------------------------
std::vector<uchar> Decompress(std::vector<uchar> &data, int &width, int &height);
//--------------------------------------------------------------------------
// Load Jpeg file and decompress into a RGB buffer
//--------------------------------------------------------------------------
std::vector<uchar> Load(const char *filename, int &width, int &height);
//--------------------------------------------------------------------------
// Compress a raw RGB image buffer to Jpeg file-in-memory buffer
//--------------------------------------------------------------------------
size_t Compress(uchar *raw, uchar *image,
				int width, int height, int quality=90);
//--------------------------------------------------------------------------
// Compress and convert a raw RGB image buffer to Jpeg file-in-memory buffer
//--------------------------------------------------------------------------
size_t Compress(std::vector<uchar> &image,
				int width, int height, int quality=90);
//--------------------------------------------------------------------------
// Save a RGB buffer to a Jpeg file
//--------------------------------------------------------------------------
size_t Save(const char *filename, std::vector<uchar> &data,
			int width, int height, int quality=90);
//--------------------------------------------------------------------------
} // DJpeg
//--------------------------------------------------------------------------
#endif
