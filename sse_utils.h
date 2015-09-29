#ifndef __SSE_WRAPPER_H
#define __SSE_WRAPPER_H
//----------------------------------------------------------------------------
int CalcSAD(unsigned char *im0, unsigned char *im1, int size);
float CalcDifferences(unsigned char *im0, unsigned char *im1,
			unsigned char *imd, int size, unsigned char eps);
//----------------------------------------------------------------------------
#endif
