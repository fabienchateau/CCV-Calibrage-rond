MAINTAINERCLEANFILES = Makefile.in
INCLUDES = -I../include -I../../include

if ENABLE_STATIC_CPI
noinst_LTLIBRARIES = libv4l.la
else
libcpi_LTLIBRARIES = libv4l.la
endif

libcpidir = $(libdir)/unicap$(pkg_version)/cpi


libv4l_la_LDFLAGS = 

libv4l_la_LIBADD = @PTHREAD_LIBS@ -L../../libunicap/.libs/ -lunicap

libv4l_la_SOURCES =  \
	queue.c \
	v4l.c v4l.h 
