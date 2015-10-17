CFLAGS=-std=c++11 -march=native -mtune=native -m64 -O3 -s -fomit-frame-pointer -ffast-math -funroll-loops
LIBS=-lstdc++ -lv4l2 -lm
ifneq ("$(USE_OMP)","")
CFLAGS+=-fopenmp
LIBS+=-lpthread
endif
ifneq ("$(USE_CPU)","")
CFLAGS+=-DUSE_CPU
endif
CORE=camera.o kalman.o sse_utils.o
DOBJ=djpeg.o
JPEG=$(DOBJ) -lturbojpeg

all: cvXw cvFb cvBs cvBc

%.o: %.cc
	$(CXX) $(CFLAGS) -c $<

cvXw: MainXW.cc Window.cc Window.h timer.h $(CORE)
	$(CXX) $(CFLAGS) -o $@ MainXW.cc Window.cc $(CORE) -lX11 $(LIBS)

cvFb: MainFB.cc timer.h $(CORE)
	$(CXX) $(CFLAGS) -o $@ MainFB.cc $(CORE) $(LIBS)

cvBs: MainBS.cc timer.h $(CORE) $(DOBJ)
	$(CXX) $(CFLAGS) -o $@ MainBS.cc $(CORE) $(JPEG) $(LIBS) -lzmq

cvBc: MainBC.cc Window.cc Window.h timer.h $(CORE) $(DOBJ)
	$(CXX) $(CFLAGS) -o $@ MainBC.cc Window.cc $(CORE) $(JPEG) $(LIBS) -lX11 -lzmq

clean:
	rm -f *.o

distclean: clean
	rm -f cvXw cvFb cvBs cvBc

