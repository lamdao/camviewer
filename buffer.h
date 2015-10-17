#ifndef __CAMERA_BUFFER_H
#define __CAMERA_BUFFER_H
//----------------------------------------------------------------------------
#include <memory>
//----------------------------------------------------------------------------
#ifndef uchar
#define uchar unsigned char
#endif
//----------------------------------------------------------------------------
template<typename T>
using buffer_ptr = std::unique_ptr<T,std::function<void(T*)>>;
//----------------------------------------------------------------------------
static inline buffer_ptr<uchar> CreateBuffer(int bsize)
{
	return buffer_ptr<uchar>((uchar*)valloc(bsize), [](uchar *b) {free(b);});
}
//----------------------------------------------------------------------------
#endif
