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

all: cvXw cvFb

%.o: %.cc
	gcc $(CFLAGS) -c $<

cvXw: MainXW.cc Window.cc Window.h timer.h $(CORE)
	gcc $(CFLAGS) -o $@ MainXW.cc Window.cc $(CORE) -lX11 $(LIBS)

cvFb: MainFB.cc timer.h $(CORE)
	gcc $(CFLAGS) -o $@ MainFB.cc $(CORE) $(LIBS)

clean:
	rm -f *.o

distclean: clean
	rm -f cvXw cvFb

