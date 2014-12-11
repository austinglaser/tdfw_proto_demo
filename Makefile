CXX = g++

CPPFLAGS = -Wextra -Wall -g

OPENCV_LIBS = opencv_core opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_calib3d \
	      opencv_objdetect opencv_contrib opencv_legacy

INCDIR = /usr/include/opencv /usr/include/opencv2
LIBDIR = /usr/local/lib
LIBS = $(OPENCV_LIBS) boost_thread

IINCDIR = $(patsubst %,-I%,$(INCDIR))
LLIBDIR= $(patsubst %,-L%,$(LIBDIR))
LLIBS = $(patsubst %,-l%,$(LIBS))

all: demo

demo: demo.cpp
	$(CXX) $(CPPFLAGS) $(IINCDIR) $(LLIBDIR) $(LLIBS) $^ -o $@

.PHONY: clean

clean:
	rm -rf *.o demo images
