# process this file with automake to create a Makefile.in

if ENABLE_STATIC_CPI
#build cpi libraries first to allow static linkage to libunicap
SUBDIRS = po cpi libunicap libucil include tests libunicapgtk doc 
else
SUBDIRS = po libunicap libucil include tests libunicapgtk cpi doc 
endif

MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure config.h.in \
	stamp-h.in
DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

#`cd examples && find -name "*.c"` `cd examples && find -name "Makefile"`
EXTRA_DIST = config.rpath  \
intltool-extract.in \
intltool-merge.in 	\
intltool-update.in \
install-sh \
README.troubleshooting \
libunicap/libunicap.pc.in \
libunicapgtk/libunicapgtk.pc.in \
libucil/libucil.pc.in \
examples/Makefile \
examples/README \
examples/c/Makefile \
examples/c/device_info/Makefile \
examples/c/device_info/device_info.c \
examples/c/raw_image/Makefile \
examples/c/raw_image/raw_image.c \
examples/c/sdl_display/Makefile \
examples/c/sdl_display/sdl_display.c \
examples/c/sdl_display/sdl_rgb_display.c \
examples/c/sdl_display/sdl_display_multibuffer.c \
examples/c/sdl_image_saving/Makefile \
examples/c/sdl_image_saving/sdl_image_saving.c \
examples/c/sdl_image_saving/colorspace.c \
examples/c/sdl_image_saving/colorspace.h \
examples/c/xv_display/xv_display-0.0.7.tar.gz \
examples/c/device_property/Makefile \
examples/c/device_property/device_property.c \
examples/c/avcodec/Makefile \
examples/c/avcodec/recordvideo.c \
examples/c/gl/gl-video.c \
examples/c/gl/yuv.cg \
examples/c/gl/yuv.pso \
examples/c/gl/Makefile \
examples/c/OpenCV/highgui_face_detect/face_detect.c \
examples/c/OpenCV/highgui_face_detect/cam_display.c \
examples/c/OpenCV/highgui_face_detect/Makefile \
examples/c/OpenCV/haarcascade_frontalface_default.xml \
examples/unicapgtk/Makefile \
examples/unicapgtk/callback/Makefile \
examples/unicapgtk/callback/callback.c \
examples/unicapgtk/smallapp/Makefile \
examples/unicapgtk/smallapp/smallapp.c \
examples/unicapgtk/cropping/Makefile \
examples/unicapgtk/cropping/cropping.c \
examples/c++/Makefile \
examples/c++/unicap--/Makefile \
examples/c++/unicap--/UnicapDataBuffer.h \
examples/c++/unicap--/UnicapDevice.cpp \
examples/c++/unicap--/UnicapDevice.h \
examples/device_specific/Makefile \
examples/device_specific/dfg1394/Makefile \
examples/device_specific/dfg1394/sdl_display/Makefile \
examples/device_specific/dfg1394/sdl_display/sdl_display.c 

clean-local: 
	cd examples && make clean

examples/xv_display/Makefile:
	cd examples/xv_display && automake && autoconf && ./configure

examples/xv_display/xv_display-0.0.7.tar.gz: examples/xv_display/Makefile
	cd examples/xv_display && make dist


snapshot: dist
	mv $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-$(VERSION)-devel-`date +%Y%m%d`.tar.gz

ACLOCAL_AMFLAGS = -I m4
