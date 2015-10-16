CFLAGS=-march=native -mtune=native -m64 -O3 -s -fomit-frame-pointer -ffast-math -funroll-loops
LIBS=-lstdc++ -lv4l2 -lm
ifneq ("$(USE_OMP)","")
CFLAGS+=-fopenmp
LIBS+=-lpthread
endif
ifneq ("$(USE_CPU)","")
CFLAGS+=-DUSE_CPU
endif
CORE=camera.o kalman.o sse_utils.o

all: cvXw cvFb cvBs cvBc

%.o: %.cc
	gcc $(CFLAGS) -c $<

cvXw: MainXW.cc Window.cc Window.h timer.h $(CORE)
	gcc $(CFLAGS) -o $@ MainXW.cc Window.cc $(CORE) -lX11 $(LIBS)

cvFb: MainFB.cc timer.h $(CORE)
	gcc $(CFLAGS) -o $@ MainFB.cc $(CORE) $(LIBS)

cvBs: MainBS.cc timer.h $(CORE)
	gcc -std=c++11 $(CFLAGS) -o $@ MainBS.cc $(CORE) $(LIBS) -lzmq -llz4

cvBc: MainBC.cc Window.cc Window.h timer.h $(CORE)
	gcc -std=c++11 $(CFLAGS) -o $@ MainBC.cc Window.cc $(CORE) $(LIBS) -lX11 -lzmq

clean:
	rm -f *.o

distclean: clean
	rm -f cvXw cvFb cvBs cvBc

