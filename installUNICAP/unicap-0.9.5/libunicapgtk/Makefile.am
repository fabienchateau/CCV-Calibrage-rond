MAINTAINERCLEANFILES = Makefile.in
INCLUDES = -I../include -I../libucil @GTK_PACKAGE_CFLAGS@ 
AM_CPPFLAGS = $(GTK_CPPFLAGS) -DSYSCONFDIR="\"$(sysconfdir)\"" -DBACKENDDIR="\"$(libdir)/unicap$(pkg_version)/backends\"" -DPKG_VERSION="\"@lt_major@\""

if BUILD_UNICAPGTK
UNICAPGTK_LIB=libunicapgtk.la
else
UNICAPGTK_LIB=
endif


lib_LTLIBRARIES = $(UNICAPGTK_LIB)

pkginclude_HEADERS = \
			unicapgtk_video_display.h \
			unicapgtk_device_property.h \
			unicapgtk_video_format_selection.h \
			unicapgtk_device_selection.h \
			unicapgtk_property_dialog.h \
			unicapgtk.h unicapgtk_version.h


libunicapgtk_la_LDFLAGS = -version-info @lt_major@:@lt_revision@:@lt_age@

libunicapgtk_la_SOURCES =  \
			  unicapgtk.c unicapgtk.h \
			  unicapgtk_video_display.c unicapgtk_video_display.h \
			  unicapgtk_device_property.c unicapgtk_device_property.h \
			  unicapgtk_video_format_selection.c unicapgtk_video_format_selection.h \
			  unicapgtk_device_selection.c unicapgtk_device_selection.h \
			  unicapgtk_property_dialog.c unicapgtk_property_dialog.h \
			  unicapgtk_priv.h \
			  backend_xv.c backend_xv.h \
			  backend_gtk.c backend_gtk.h backend.h 

libunicapgtk_la_LIBADD = \
			  @GTK_PACKAGE_LIBS@ \
			  $(X_XV_LIBS) \
			  $(X_LIBS) \
			  -L../libunicap/.libs \
			  -lunicap \
			  -L../libucil/.libs \
			  -lucil

libunicapgtk_la_CFLAGS = \
			 -DUNICAP_LOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libunicapgtk.pc
