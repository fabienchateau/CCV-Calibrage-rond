MAINTAINERCLEANFILES = Makefile.in
INCLUDES = -I../include -I. -I../cpi/include
AM_CPPFLAGS=-DINSTALL_PREFIX="\"$(prefix)\"" -DPKGLIBDIR="\"$(pkglibdir)\"" $(GLIB_CPPFLAGS) -Wno-multichar
AM_CFLAGS = @GLIB_PACKAGE_CFLAGS@ @PANGO_PACKAGE_CFLAGS@ @AVCODEC_PACKAGE_CFLAGS@ @THEORA_PACKAGE_CFLAGS@ @ALSA_PACKAGE_CFLAGS@ -fno-strict-aliasing

if BUILD_UCIL
UCIL_LIB=libucil.la
else
UCIL_LIB=
endif

pkg_headers=

lib_LTLIBRARIES = $(UCIL_LIB)

libucil_la_LDFLAGS = -version-info @lt_major@:@lt_revision@:@lt_age@
libucil_la_LIBADD = @GLIB_PACKAGE_LIBS@ @PANGO_PACKAGE_LIBS@ @AVCODEC_PACKAGE_LIBS@ @THEORA_PACKAGE_LIBS@ @ALSA_PACKAGE_LIBS@ @PNG_PACKAGE_LIBS@ $(top_builddir)/libunicap/libunicap.la -lm

if HAVE_AVCODEC
libucil_avcavi_sources=ucil_avc_avi.c ucil_avc_avi.h
else
libucil_avcavi_sources=
endif

if HAVE_THEORA
libucil_theora_sources=ucil_theora.c ucil_theora.h
else
libucil_theora_sources=
endif

if HAVE_ALSA
libucil_alsa_sources=ucil_alsa.c ucil_alsa.h
pkg_headers+=ucil_alsa.h
else
libucil_alsa_sources=
endif

if HAVE_PNG
libucil_png_sources=ucil_png.c ucil_png.h
pkg_headers+=ucil_png.h
else
libucil_png_sources=
endif


libucil_la_SOURCES =   \
		ucil.h ucil.c \
		draw.c draw.h \
		colorspace.c colorspace.h \
		yuvops.c yuvops.h \
		rgbops.c rgbops.h \
		font.c  \
		video_file.c video_file.h \
		queue.c queue.h \
		ucil_rawavi.c ucil_rawavi.h avi.h\
		ucil_audio.c ucil_audio.h \
		$(libucil_avcodec_sources) \
		$(libucil_avcavi_sources) \
		$(libucil_theora_sources) \
		$(libucil_alsa_sources) \
		$(libucil_png_sources)


pkginclude_HEADERS = ucil.h ucil_version.h $(pkg_headers)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libucil.pc
