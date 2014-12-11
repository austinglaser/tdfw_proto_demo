CXX = g++

CPPFLAGS = -Wextra -Wall -g

OPENCV_LIBS = opencv_core opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_calib3d \
	      opencv_objdetect opencv_contrib opencv_legacy boost_thread

INCDIR = /usr/include/opencv /usr/include/opencv2
LIBDIR = /usr/local/lib
LIBS = $(OPENCV_LIBS)

IINCDIR = $(patsubst %,-I%,$(INCDIR))
LLIBDIR= $(patsubst %,-L%,$(LIBDIR))
LLIBS = $(patsubst %,-l%,$(LIBS))

all: test

test: test.cpp
	$(CXX) $(CPPFLAGS) $(IINCDIR) $(LLIBDIR) $(LLIBS) $^ -o $@

.PHONY: clean

clean:
	rm -rf *.o test images
