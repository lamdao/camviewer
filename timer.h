#ifndef __TIMER_H
#define __TIMER_H
//----------------------------------------------------------------------------
#include <sys/time.h>
//----------------------------------------------------------------------------
namespace Timer
{
	static timeval t0, t1; 

	void Start()
	{   
		gettimeofday(&t0, 0); 
	}   

	void Stop()
	{   
		gettimeofday(&t1,0);
		int s = t1.tv_sec - t0.tv_sec;
		int u = t1.tv_usec - t0.tv_usec;
		if (u < 0) {
			u += 1000000;
			s -= 1;
		}   
		printf("%5.1f ms\r", 1E-3 * (1000000 * s + u));
	}   

	void Check()
	{
		Stop();
		Start();
	}
};
//----------------------------------------------------------------------------
#endif
