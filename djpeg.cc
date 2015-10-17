//----------------------------------------------------------------------------
// djpeg.cc - A simple wrapper implementation for turbojpeg library
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
#include <turbojpeg.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
//--------------------------------------------------------------------------
#include "djpeg.h"
//--------------------------------------------------------------------------
using namespace std;
//--------------------------------------------------------------------------
namespace DJpeg {
//--------------------------------------------------------------------------
// Decompress a Jpeg file-in-memory buffer (data) and return a RGB buffer
// Use this function when we know for sure the size of uncompressed image
// and pre-allocated it
//--------------------------------------------------------------------------
uchar *Decompress(void *jraw, size_t jsize, uchar *image, int &width, int &height)
{
	if (!image)
		return 0;

	tjhandle jpg = tjInitDecompress();
	if (jpg == nullptr) {
		cerr << "InitDecompress failed: " << tjGetErrorStr() << '\n';
		return 0;
	}

	if (tjDecompressHeader(jpg, (uchar *)jraw, jsize, &width, &height)) {
		cerr << "DecompressHeader failed: " << tjGetErrorStr() << '\n';
		return 0;
	}

	int bpp = tjPixelSize[TJPF_RGB];
	int pitch = bpp * width;

	if (tjDecompress(jpg, (uchar *)jraw, jsize,
				image, width, pitch, height, bpp, 0)) {
		cerr << "Decompress failed: " << tjGetErrorStr() << '\n';
		return 0;
	}
	tjDestroy(jpg);

	return image;
}
//--------------------------------------------------------------------------
// Decompress a Jpeg file-in-memory buffer (data) and return a RGB buffer
//--------------------------------------------------------------------------
vector<uchar> Decompress(void *jraw, size_t jsize, int &width, int &height)
{
	tjhandle jpg = tjInitDecompress();
	if (jpg == nullptr) {
		cerr << "InitDecompress failed: " << tjGetErrorStr() << '\n';
		return vector<uchar>(0);
	}

	if (tjDecompressHeader(jpg, (uchar *)jraw, jsize, &width, &height)) {
		cerr << "DecompressHeader failed: " << tjGetErrorStr() << '\n';
		return vector<uchar>(0);
	}

	int bpp = tjPixelSize[TJPF_RGB];
	int pitch = bpp * width;
	int size = pitch * height;
	vector<uchar> image(size);

	if (tjDecompress(jpg, (uchar *)jraw, jsize,
				&image[0], width, pitch, height, bpp, 0)) {
		cerr << "Decompress failed: " << tjGetErrorStr() << '\n';
		return vector<uchar>(0);
	}
	tjDestroy(jpg);

	return std::move(image);
}
//--------------------------------------------------------------------------
// Decompress a Jpeg file-in-memory buffer (data) and return a RGB buffer
//--------------------------------------------------------------------------
vector<uchar> Decompress(vector<uchar> &data, int &width, int &height)
{
	return Decompress(&data[0], data.size(), width, height);
}
//--------------------------------------------------------------------------
// Load Jpeg file and decompress into a RGB buffer
//--------------------------------------------------------------------------
vector<uchar> Load(const char *filename, int &width, int &height)
{
	ifstream ifp(filename, ios::binary|ios::ate);
	if (!ifp) {
		cerr << "Cannot open file " << filename << '\n';
		return vector<uchar>(0);
	}

	size_t size = ifp.tellg();
	vector<uchar> data(size);

	ifp.seekg(0);
	ifp.read((char *)&data[0], size);

	return Decompress(data, width, height);
}
//--------------------------------------------------------------------------
// Compress a raw RGB image buffer to Jpeg file-in-memory buffer
//--------------------------------------------------------------------------
size_t Compress(uchar *raw, uchar *image, int width, int height, int quality)
{
	tjhandle jpg = tjInitCompress();
	if (jpg == nullptr) {
		cerr << "InitCompress failed: " << tjGetErrorStr() << '\n';
		return 0;
	}

	size_t csize;
	int bpp = tjPixelSize[TJPF_RGB];
	int pitch = bpp * width;
	if (tjCompress(jpg, raw, width, pitch, height, bpp,
					image, &csize, TJ_444, quality, 0)) {
		cerr << "Compress failed: " << tjGetErrorStr() << '\n';
		return 0;
	}
	tjDestroy(jpg);
	return csize;
}
//--------------------------------------------------------------------------
// Compress and convert a raw RGB image buffer to Jpeg file-in-memory buffer
//--------------------------------------------------------------------------
size_t Compress(vector<uchar> &image, int width, int height, int quality)
{
	return Compress(image.data(), image.data(), width, height, quality);
}
//--------------------------------------------------------------------------
// Save a RGB buffer to a Jpeg file
//--------------------------------------------------------------------------
size_t Save(const char *filename, vector<uchar> &data,
								int width, int height, int quality)
{
	ofstream ofp(filename, ios::binary);
	size_t size = Compress(data, width, height, quality);
	ofp.write((char *)&data[0], size);
	return size;
}
//--------------------------------------------------------------------------
} // DJpeg
