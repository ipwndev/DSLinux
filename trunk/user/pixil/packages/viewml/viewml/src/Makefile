# uncomment the line below for Nano-X
_NANOX = 1

ifndef _NANOX
  # your version of FLTK (1.0.7, please!)
  FLTK = /home/davet/src/fltk/flmwin
else
  # your version of Nano-X FLTK (0.88, please!)
  FLTK = $(ROOTDIR)/user/pixil/libs/flnx
endif

# Where your Xlibs are (not needed for Nano-X/Mwin builds)
XLIBS = -L/usr/X11R6/lib -lX11

# Where Nano-X is (not needed if building for X only)
NANOX = $(ROOTDIR)/user/microwin

# Various support libraries
IMLIBS = `imlib-config --libs`
WWWLIBS = `$(ROOTDIR)/user/lib/libwww/libwww-config --libs`

# The everpresent CFLAGS
ifndef _NANOX
  CFLAGS = -g `/usr/bin/libwww-config --cflags`
else
  CFLAGS += "-Wno-deprecated  -g -DNANOX -D_NANOX `libwww-config --cflags`"
endif


NANOXLIBS = -L$(NANOX)/src/lib -lnano-X

################################################################

ifndef _NANOX
  LIBS = $(FLTK)/lib/libfltk.a -lfl $(XLIBS) -lpthread $(WWWLIBS) $(IMLIBS) 
else
  LIBS = $(FLTK)/lib/libfltk.a -lfl -lpthread $(NANOXLIBS) $(WWWLIBS)
endif

KDEINC = -I../kdecore -I../kimgio -I../kdeui -I../jscript -I../../../include/par

ifndef _NANOX
  INCLUDES = -I$(FLTK)/FL -I$(FLTK) -I./fltk -I. $(KDEINC)
else
  INCLUDES = -I$(FLTK)/FL -I$(FLTK) -I$(NANOX)/src/include -I./fltk -I. $(KDEINC) 
endif

CPPFILES = html.cpp htmlchain.cpp htmlclue.cpp htmldata.cpp htmlfont.cpp \
	htmliter.cpp htmltable.cpp htmltoken.cpp jscript.cpp htmlview.cpp \
	htmlframe.cpp htmlobj.cpp debug.cpp htmlform.cpp main.cpp http.cpp \
	http_.cpp

OTHERCPP = ../kdecore/kurl.cpp ../kdeui/kcursor.cpp fltk/qtimer.cpp \
	fltk/qobject.cpp fltk/qpainter.cpp fltk/qdrawutil.cpp \
	fltk/qfont.cpp fltk/qrect.cpp fltk/qregexp.cpp fltk/qstring.cpp \
	fltk/kcharsets.cpp fltk/qcolor.cpp fltk/qpixmap.cpp \
	fltk/qfontinfo.cpp fltk/qwidget.cpp fltk/history.cpp fltk/qscrollbar.cpp \
	fltk/nxslider.cpp fltk/nxscrollbar.cpp fltk/nxscroll.cpp

OBJS = html.o htmlchain.o htmlclue.o htmldata.o htmlfont.o htmliter.o \
	htmltable.o htmltoken.o jscript.o htmlview.o htmlframe.o htmlobj.o \
	debug.o htmlform.o main.o http.o http_.o

OTHEROBJS = ../kdecore/kurl.o ../jscript/builtin.o ../jscript/cpp2bison.o \
	../jscript/jsexec.o ../jscript/jstree.o ../jscript/t2.o \
	../jscript/t1.o ../kdeui/kcursor.o fltk/qtimer.o fltk/qobject.o \
	fltk/qpainter.o fltk/qdrawutil.o fltk/qfont.o fltk/qrect.o \
	fltk/qregexp.o fltk/qstring.o fltk/kcharsets.o fltk/qcolor.o \
	fltk/qpixmap.o fltk/qfontinfo.o fltk/qwidget.o fltk/history.o \
	fltk/qscrollbar.o fltk/nxslider.o fltk/nxscrollbar.o fltk/nxscroll.o

################################################################

OBJECTS = $(CPPFILES:.cxx=.o) $(CFILES:.c=.o)

all: jscript $(OBJS) $(OTHEROBJS) main

jscript:
	cd ../jscript && make

main: $(OBJS) $(OTHEROBJS)
	$(CXX) $(OBJS) $(OTHEROBJS) -o viewml $(LIBS)

depend:
	makedepend -I/usr/include/g++2 -I/usr/include/g++2/std \
	$(INCLUDES) $(CPPFILES) $(OTHERCPP)

.SUFFIXES:	.cpp .h .o

.cpp.o :
	$(CXX) -o $@ $(CRTBEGIN) $(CXXFLAGS) $(CFLAGS) $(INCLUDES) -I.. $(LDFLAGS) $(OBJS) $(CXXLIBS) $(LDLIBS) $(CRTEND)
.c.o :
	$(CC) $(CFLAGS) -I.. $(CFLAGS) -c -o $@ $<

clean :
	-@ rm -f *.o *.do $(DSONAME) $(LIBRARY) $(CLEAN) core *~ ../include/*~ makedepend cmap
	@touch makedepend
	rm fltk/*.o
	cd ../jscript && make clean

# DO NOT DELETE

html.o: ../kdecore/kurl.h ./fltk/qstring.h
html.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
html.o: /usr/include/errno.h /usr/include/features.h /usr/include/sys/cdefs.h
html.o: /usr/include/gnu/stubs.h /usr/include/bits/errno.h
html.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
html.o: /usr/include/stdio.h
html.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
html.o: /usr/include/bits/types.h /usr/include/libio.h
html.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
html.o: ./fltk/fltk-qdefs.h ./fltk/qstrlist.h ./fltk/qarray.h
html.o: ./fltk/qcollection.h ./fltk/kapp.h ./fltk/kcharsets.h ./fltk/qfont.h
html.o: ./fltk/qrect.h ./fltk/qpoint.h ./fltk/qsize.h ./fltk/fltk-qbase.h
html.o: ./fltk/qevent.h ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
html.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
html.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
html.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
html.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
html.o: /usr/include/time.h /usr/include/bits/siginfo.h
html.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
html.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
html.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h
html.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
html.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h /usr/include/sys/types.h
html.o: /usr/include/endian.h /usr/include/bits/endian.h
html.o: /usr/include/sys/select.h /usr/include/bits/select.h
html.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
html.o: /usr/include/X11/Xosdefs.h ./fltk/klocale.h ./fltk/qcursor.h
html.o: ./fltk/qbitmap.h ../kimgio/kimgio.h ../kdeui/kcursor.h
html.o: /usr/include/assert.h htmlchain.h htmltable.h ./fltk/qpainter.h
html.o: ./fltk/qpen.h ./fltk/qfontinfo.h ./fltk/qbrush.h ./fltk/qpixmap.h
html.o: ./fltk/qpaintdevice.h ./fltk/qwmatrix.h ./fltk/qfontmetrics.h
html.o: ./fltk/qstack.h ./fltk/qtimer.h htmlfont.h htmlobj.h htmlclue.h
html.o: htmliter.h html.h ./fltk/drag.h ./fltk/qwidget.h ./fltk/qpalette.h
html.o: htmldata.h htmlform.h jscript.h ../jscript/jsexec.h
html.o: ../jscript/jstree.h ../jscript/jserror.h htmltoken.h htmlframe.h
html.o: ./fltk/qframe.h htmlview.h ./fltk/qscrollbar.h /usr/include/stdlib.h
html.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/string.h
html.o: /usr/include/strings.h ./fltk/qregexp.h ./fltk/qprinter.h
html.o: ./fltk/qdrawutil.h html.moc
htmlclue.o: ../kdecore/kurl.h ./fltk/qstring.h
htmlclue.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmlclue.o: /usr/include/errno.h /usr/include/features.h
htmlclue.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmlclue.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
htmlclue.o: /usr/include/asm/errno.h /usr/include/stdio.h
htmlclue.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmlclue.o: /usr/include/bits/types.h /usr/include/libio.h
htmlclue.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmlclue.o: ./fltk/fltk-qdefs.h ./fltk/qstrlist.h ./fltk/qarray.h
htmlclue.o: ./fltk/qcollection.h htmlchain.h htmlobj.h ./fltk/qpainter.h
htmlclue.o: ./fltk/qpen.h ./fltk/qcolor.h ./fltk/qfont.h ./fltk/qrect.h
htmlclue.o: ./fltk/qpoint.h ./fltk/qsize.h ./fltk/fltk-qbase.h
htmlclue.o: ./fltk/qevent.h ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
htmlclue.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qobjectdefs.h
htmlclue.o: ./fltk/qobject.h ./fltk/qmetaobject.h ./fltk/qconnection.h
htmlclue.o: ./fltk/qdict.h /usr/include/signal.h /usr/include/bits/sigset.h
htmlclue.o: /usr/include/bits/signum.h /usr/include/time.h
htmlclue.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
htmlclue.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
htmlclue.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
htmlclue.o: /usr/include/sched.h /usr/include/bits/sched.h
htmlclue.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
htmlclue.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
htmlclue.o: /usr/include/sys/types.h /usr/include/endian.h
htmlclue.o: /usr/include/bits/endian.h /usr/include/sys/select.h
htmlclue.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
htmlclue.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
htmlclue.o: ./fltk/qfontinfo.h ./fltk/qbrush.h ./fltk/qpixmap.h
htmlclue.o: ./fltk/qpaintdevice.h ./fltk/qbitmap.h ./fltk/qwmatrix.h
htmlclue.o: ./fltk/qfontmetrics.h ./fltk/qstack.h ./fltk/qtimer.h htmlfont.h
htmlclue.o: ./fltk/kcharsets.h htmlclue.h htmliter.h html.h ./fltk/drag.h
htmlclue.o: ./fltk/qwidget.h ./fltk/qcursor.h ./fltk/qpalette.h htmldata.h
htmlclue.o: htmlform.h jscript.h ../jscript/jsexec.h ../jscript/jstree.h
htmlclue.o: ../jscript/jserror.h htmltoken.h htmlframe.h ./fltk/qframe.h
htmlclue.o: htmlview.h ./fltk/qscrollbar.h /usr/include/stdlib.h
htmlclue.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/string.h
htmlclue.o: ./fltk/qimage.h ./fltk/qdrawutil.h
htmldata.o: htmldata.h ./fltk/qcolor.h ./fltk/qstring.h
htmldata.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmldata.o: /usr/include/errno.h /usr/include/features.h
htmldata.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmldata.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
htmldata.o: /usr/include/asm/errno.h /usr/include/stdio.h
htmldata.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmldata.o: /usr/include/bits/types.h /usr/include/libio.h
htmldata.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmldata.o: ./fltk/fltk-qdefs.h ./fltk/qcursor.h ./fltk/qpoint.h
htmldata.o: ./fltk/qarray.h ./fltk/qbitmap.h ./fltk/kcharsets.h
htmldata.o: ./fltk/qfont.h ./fltk/qrect.h ./fltk/qsize.h ./fltk/fltk-qbase.h
htmldata.o: ./fltk/qstrlist.h ./fltk/qcollection.h ./fltk/qevent.h
htmldata.o: ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
htmldata.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qobjectdefs.h
htmldata.o: ./fltk/qobject.h ./fltk/qmetaobject.h ./fltk/qconnection.h
htmldata.o: ./fltk/qdict.h /usr/include/signal.h /usr/include/bits/sigset.h
htmldata.o: /usr/include/bits/signum.h /usr/include/time.h
htmldata.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
htmldata.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
htmldata.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
htmldata.o: /usr/include/sched.h /usr/include/bits/sched.h
htmldata.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
htmldata.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
htmldata.o: /usr/include/sys/types.h /usr/include/endian.h
htmldata.o: /usr/include/bits/endian.h /usr/include/sys/select.h
htmldata.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
htmldata.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
htmldata.o: ./fltk/kapp.h ./fltk/klocale.h
htmlfont.o: htmlfont.h ./fltk/qlist.h ./fltk/qcollection.h ./fltk/qcolor.h
htmlfont.o: ./fltk/qstring.h
htmlfont.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmlfont.o: /usr/include/errno.h /usr/include/features.h
htmlfont.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmlfont.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
htmlfont.o: /usr/include/asm/errno.h /usr/include/stdio.h
htmlfont.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmlfont.o: /usr/include/bits/types.h /usr/include/libio.h
htmlfont.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmlfont.o: ./fltk/fltk-qdefs.h ./fltk/qfont.h ./fltk/qrect.h ./fltk/qpoint.h
htmlfont.o: ./fltk/qarray.h ./fltk/qsize.h ./fltk/fltk-qbase.h
htmlfont.o: ./fltk/qstrlist.h ./fltk/qevent.h ./fltk/qkeycode.h
htmlfont.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
htmlfont.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
htmlfont.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
htmlfont.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
htmlfont.o: /usr/include/time.h /usr/include/bits/siginfo.h
htmlfont.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
htmlfont.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
htmlfont.o: /usr/include/pthread.h /usr/include/sched.h
htmlfont.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
htmlfont.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
htmlfont.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
htmlfont.o: /usr/include/endian.h /usr/include/bits/endian.h
htmlfont.o: /usr/include/sys/select.h /usr/include/bits/select.h
htmlfont.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
htmlfont.o: /usr/include/X11/Xosdefs.h ./fltk/kcharsets.h
htmlfont.o: /usr/include/string.h ./fltk/kapp.h ./fltk/klocale.h
htmlfont.o: ./fltk/qcursor.h ./fltk/qbitmap.h
htmliter.o: htmlobj.h /usr/include/stdio.h /usr/include/features.h
htmliter.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmliter.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmliter.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmliter.o: /usr/include/bits/types.h /usr/include/libio.h
htmliter.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmliter.o: ./fltk/qpainter.h ./fltk/qpen.h ./fltk/qcolor.h ./fltk/qstring.h
htmliter.o: /usr/include/errno.h /usr/include/bits/errno.h
htmliter.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
htmliter.o: ./fltk/fltk-qdefs.h ./fltk/qfont.h ./fltk/qrect.h ./fltk/qpoint.h
htmliter.o: ./fltk/qarray.h ./fltk/qsize.h ./fltk/fltk-qbase.h
htmliter.o: ./fltk/qstrlist.h ./fltk/qcollection.h ./fltk/qevent.h
htmliter.o: ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
htmliter.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qobjectdefs.h
htmliter.o: ./fltk/qobject.h ./fltk/qmetaobject.h ./fltk/qconnection.h
htmliter.o: ./fltk/qdict.h /usr/include/signal.h /usr/include/bits/sigset.h
htmliter.o: /usr/include/bits/signum.h /usr/include/time.h
htmliter.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
htmliter.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
htmliter.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
htmliter.o: /usr/include/sched.h /usr/include/bits/sched.h
htmliter.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
htmliter.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
htmliter.o: /usr/include/sys/types.h /usr/include/endian.h
htmliter.o: /usr/include/bits/endian.h /usr/include/sys/select.h
htmliter.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
htmliter.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
htmliter.o: ./fltk/qfontinfo.h ./fltk/qbrush.h ./fltk/qpixmap.h
htmliter.o: ./fltk/qpaintdevice.h ./fltk/qbitmap.h ./fltk/qwmatrix.h
htmliter.o: ./fltk/qfontmetrics.h ./fltk/qstack.h ./fltk/qtimer.h
htmliter.o: ../kdecore/kurl.h htmlfont.h ./fltk/kcharsets.h htmlclue.h
htmliter.o: htmliter.h htmltable.h
htmltable.o: ../kdecore/kurl.h ./fltk/qstring.h
htmltable.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmltable.o: /usr/include/errno.h /usr/include/features.h
htmltable.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmltable.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
htmltable.o: /usr/include/asm/errno.h /usr/include/stdio.h
htmltable.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmltable.o: /usr/include/bits/types.h /usr/include/libio.h
htmltable.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmltable.o: ./fltk/fltk-qdefs.h ./fltk/qstrlist.h ./fltk/qarray.h
htmltable.o: ./fltk/qcollection.h htmlchain.h htmlobj.h ./fltk/qpainter.h
htmltable.o: ./fltk/qpen.h ./fltk/qcolor.h ./fltk/qfont.h ./fltk/qrect.h
htmltable.o: ./fltk/qpoint.h ./fltk/qsize.h ./fltk/fltk-qbase.h
htmltable.o: ./fltk/qevent.h ./fltk/qkeycode.h ./fltk/qlist.h
htmltable.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
htmltable.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
htmltable.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
htmltable.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
htmltable.o: /usr/include/time.h /usr/include/bits/siginfo.h
htmltable.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
htmltable.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
htmltable.o: /usr/include/pthread.h /usr/include/sched.h
htmltable.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
htmltable.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
htmltable.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
htmltable.o: /usr/include/endian.h /usr/include/bits/endian.h
htmltable.o: /usr/include/sys/select.h /usr/include/bits/select.h
htmltable.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
htmltable.o: /usr/include/X11/Xosdefs.h ./fltk/qfontinfo.h ./fltk/qbrush.h
htmltable.o: ./fltk/qpixmap.h ./fltk/qpaintdevice.h ./fltk/qbitmap.h
htmltable.o: ./fltk/qwmatrix.h ./fltk/qfontmetrics.h ./fltk/qstack.h
htmltable.o: ./fltk/qtimer.h htmlfont.h ./fltk/kcharsets.h htmlclue.h
htmltable.o: htmliter.h htmltable.h html.h ./fltk/drag.h ./fltk/qwidget.h
htmltable.o: ./fltk/qcursor.h ./fltk/qpalette.h htmldata.h htmlform.h
htmltable.o: jscript.h ../jscript/jsexec.h ../jscript/jstree.h
htmltable.o: ../jscript/jserror.h htmltoken.h htmlframe.h ./fltk/qframe.h
htmltable.o: htmlview.h ./fltk/qscrollbar.h /usr/include/stdlib.h
htmltable.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/string.h
htmltable.o: ./fltk/qimage.h ./fltk/qdrawutil.h
htmltoken.o: htmltoken.h ./fltk/qlist.h ./fltk/qcollection.h
htmltoken.o: ./fltk/qstrlist.h ./fltk/qstring.h
htmltoken.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmltoken.o: /usr/include/errno.h /usr/include/features.h
htmltoken.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmltoken.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
htmltoken.o: /usr/include/asm/errno.h /usr/include/stdio.h
htmltoken.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmltoken.o: /usr/include/bits/types.h /usr/include/libio.h
htmltoken.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmltoken.o: ./fltk/fltk-qdefs.h ./fltk/qarray.h jscript.h
htmltoken.o: ../jscript/jsexec.h ./fltk/qstack.h ../jscript/jstree.h
htmltoken.o: ../jscript/jserror.h html.h ./fltk/qpainter.h ./fltk/qpen.h
htmltoken.o: ./fltk/qcolor.h ./fltk/qfont.h ./fltk/qrect.h ./fltk/qpoint.h
htmltoken.o: ./fltk/qsize.h ./fltk/fltk-qbase.h ./fltk/qevent.h
htmltoken.o: ./fltk/qkeycode.h ./fltk/qbuffer.h ./fltk/qiodevice.h
htmltoken.o: ./fltk/qregion.h ./fltk/qobjectdefs.h ./fltk/qobject.h
htmltoken.o: ./fltk/qmetaobject.h ./fltk/qconnection.h ./fltk/qdict.h
htmltoken.o: /usr/include/signal.h /usr/include/bits/sigset.h
htmltoken.o: /usr/include/bits/signum.h /usr/include/time.h
htmltoken.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
htmltoken.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
htmltoken.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
htmltoken.o: /usr/include/sched.h /usr/include/bits/sched.h
htmltoken.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
htmltoken.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
htmltoken.o: /usr/include/sys/types.h /usr/include/endian.h
htmltoken.o: /usr/include/bits/endian.h /usr/include/sys/select.h
htmltoken.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
htmltoken.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
htmltoken.o: ./fltk/qfontinfo.h ./fltk/qbrush.h ./fltk/qpixmap.h
htmltoken.o: ./fltk/qpaintdevice.h ./fltk/qbitmap.h ./fltk/qwmatrix.h
htmltoken.o: ./fltk/qfontmetrics.h ./fltk/qtimer.h ../kdecore/kurl.h
htmltoken.o: ./fltk/drag.h ./fltk/qwidget.h ./fltk/qcursor.h
htmltoken.o: ./fltk/qpalette.h htmldata.h ./fltk/kcharsets.h htmlobj.h
htmltoken.o: htmlfont.h htmlclue.h htmliter.h htmlform.h htmlframe.h
htmltoken.o: ./fltk/qframe.h htmlview.h ./fltk/qscrollbar.h
htmltoken.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/ctype.h
htmltoken.o: /usr/include/string.h /usr/include/strings.h ./fltk/kapp.h
htmltoken.o: ./fltk/klocale.h
jscript.o: /usr/include/stdlib.h /usr/include/features.h
jscript.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
jscript.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
jscript.o: /usr/include/sys/types.h /usr/include/bits/types.h
jscript.o: /usr/include/time.h /usr/include/endian.h
jscript.o: /usr/include/bits/endian.h /usr/include/sys/select.h
jscript.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
jscript.o: /usr/include/sys/sysmacros.h /usr/include/alloca.h
jscript.o: /usr/include/stdio.h
jscript.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
jscript.o: /usr/include/libio.h /usr/include/_G_config.h
jscript.o: /usr/include/bits/stdio_lim.h jscript.h ./fltk/qstring.h
jscript.o: /usr/include/errno.h /usr/include/bits/errno.h
jscript.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
jscript.o: ./fltk/fltk-qdefs.h ../jscript/jsexec.h ./fltk/qlist.h
jscript.o: ./fltk/qcollection.h ./fltk/qstack.h ../jscript/jstree.h
jscript.o: ../jscript/jserror.h html.h ./fltk/qpainter.h ./fltk/qpen.h
jscript.o: ./fltk/qcolor.h ./fltk/qfont.h ./fltk/qrect.h ./fltk/qpoint.h
jscript.o: ./fltk/qarray.h ./fltk/qsize.h ./fltk/fltk-qbase.h
jscript.o: ./fltk/qstrlist.h ./fltk/qevent.h ./fltk/qkeycode.h
jscript.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
jscript.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
jscript.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
jscript.o: /usr/include/bits/signum.h /usr/include/bits/siginfo.h
jscript.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
jscript.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
jscript.o: /usr/include/pthread.h /usr/include/sched.h
jscript.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
jscript.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
jscript.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
jscript.o: /usr/include/X11/Xosdefs.h ./fltk/qfontinfo.h ./fltk/qbrush.h
jscript.o: ./fltk/qpixmap.h ./fltk/qpaintdevice.h ./fltk/qbitmap.h
jscript.o: ./fltk/qwmatrix.h ./fltk/qfontmetrics.h ./fltk/qtimer.h
jscript.o: ../kdecore/kurl.h ./fltk/drag.h ./fltk/qwidget.h ./fltk/qcursor.h
jscript.o: ./fltk/qpalette.h htmldata.h ./fltk/kcharsets.h htmlobj.h
jscript.o: htmlfont.h htmlclue.h htmliter.h htmlform.h htmltoken.h
jscript.o: htmlframe.h ./fltk/qframe.h htmlview.h ./fltk/qscrollbar.h
htmlview.o: htmlview.h ./fltk/qscrollbar.h ./fltk/qwidget.h
htmlview.o: ./fltk/fltk-qbase.h /usr/include/stdio.h /usr/include/features.h
htmlview.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmlview.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmlview.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmlview.o: /usr/include/bits/types.h /usr/include/libio.h
htmlview.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmlview.o: ./fltk/qstring.h /usr/include/errno.h /usr/include/bits/errno.h
htmlview.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
htmlview.o: ./fltk/fltk-qdefs.h ./fltk/qstrlist.h ./fltk/qarray.h
htmlview.o: ./fltk/qcollection.h ./fltk/qevent.h ./fltk/qpoint.h
htmlview.o: ./fltk/qrect.h ./fltk/qsize.h ./fltk/qkeycode.h ./fltk/qlist.h
htmlview.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
htmlview.o: ./fltk/qcolor.h ./fltk/qobjectdefs.h ./fltk/qobject.h
htmlview.o: ./fltk/qmetaobject.h ./fltk/qconnection.h ./fltk/qdict.h
htmlview.o: /usr/include/signal.h /usr/include/bits/sigset.h
htmlview.o: /usr/include/bits/signum.h /usr/include/time.h
htmlview.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
htmlview.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
htmlview.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
htmlview.o: /usr/include/sched.h /usr/include/bits/sched.h
htmlview.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
htmlview.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
htmlview.o: /usr/include/sys/types.h /usr/include/endian.h
htmlview.o: /usr/include/bits/endian.h /usr/include/sys/select.h
htmlview.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
htmlview.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
htmlview.o: ./fltk/qpaintdevice.h ./fltk/qcursor.h ./fltk/qbitmap.h
htmlview.o: ./fltk/qpalette.h ./fltk/qfontmetrics.h ./fltk/qfont.h
htmlview.o: ./fltk/qfontinfo.h html.h ./fltk/qpainter.h ./fltk/qpen.h
htmlview.o: ./fltk/qbrush.h ./fltk/qpixmap.h ./fltk/qwmatrix.h
htmlview.o: ./fltk/qstack.h ./fltk/qtimer.h ../kdecore/kurl.h ./fltk/drag.h
htmlview.o: htmldata.h ./fltk/kcharsets.h htmlobj.h htmlfont.h htmlclue.h
htmlview.o: htmliter.h htmlform.h jscript.h ../jscript/jsexec.h
htmlview.o: ../jscript/jstree.h ../jscript/jserror.h htmltoken.h htmlframe.h
htmlview.o: ./fltk/qframe.h ../kdeui/kcursor.h htmlview.moc
htmlframe.o: htmlframe.h ./fltk/qwidget.h ./fltk/fltk-qbase.h
htmlframe.o: /usr/include/stdio.h /usr/include/features.h
htmlframe.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmlframe.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmlframe.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmlframe.o: /usr/include/bits/types.h /usr/include/libio.h
htmlframe.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmlframe.o: ./fltk/qstring.h /usr/include/errno.h /usr/include/bits/errno.h
htmlframe.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
htmlframe.o: ./fltk/fltk-qdefs.h ./fltk/qstrlist.h ./fltk/qarray.h
htmlframe.o: ./fltk/qcollection.h ./fltk/qevent.h ./fltk/qpoint.h
htmlframe.o: ./fltk/qrect.h ./fltk/qsize.h ./fltk/qkeycode.h ./fltk/qlist.h
htmlframe.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
htmlframe.o: ./fltk/qcolor.h ./fltk/qobjectdefs.h ./fltk/qobject.h
htmlframe.o: ./fltk/qmetaobject.h ./fltk/qconnection.h ./fltk/qdict.h
htmlframe.o: /usr/include/signal.h /usr/include/bits/sigset.h
htmlframe.o: /usr/include/bits/signum.h /usr/include/time.h
htmlframe.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
htmlframe.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
htmlframe.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
htmlframe.o: /usr/include/sched.h /usr/include/bits/sched.h
htmlframe.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
htmlframe.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
htmlframe.o: /usr/include/sys/types.h /usr/include/endian.h
htmlframe.o: /usr/include/bits/endian.h /usr/include/sys/select.h
htmlframe.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
htmlframe.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
htmlframe.o: ./fltk/qpaintdevice.h ./fltk/qcursor.h ./fltk/qbitmap.h
htmlframe.o: ./fltk/qpalette.h ./fltk/qfontmetrics.h ./fltk/qfont.h
htmlframe.o: ./fltk/qfontinfo.h ./fltk/qframe.h html.h ./fltk/qpainter.h
htmlframe.o: ./fltk/qpen.h ./fltk/qbrush.h ./fltk/qpixmap.h ./fltk/qwmatrix.h
htmlframe.o: ./fltk/qstack.h ./fltk/qtimer.h ../kdecore/kurl.h ./fltk/drag.h
htmlframe.o: htmldata.h ./fltk/kcharsets.h htmlobj.h htmlfont.h htmlclue.h
htmlframe.o: htmliter.h htmlform.h jscript.h ../jscript/jsexec.h
htmlframe.o: ../jscript/jstree.h ../jscript/jserror.h htmltoken.h htmlview.h
htmlframe.o: ./fltk/qscrollbar.h ../kdeui/kcursor.h /usr/include/strings.h
htmlframe.o: /usr/include/stdlib.h /usr/include/alloca.h htmlframe.moc
htmlobj.o: ../kdecore/kurl.h ./fltk/qstring.h
htmlobj.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmlobj.o: /usr/include/errno.h /usr/include/features.h
htmlobj.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmlobj.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
htmlobj.o: /usr/include/asm/errno.h /usr/include/stdio.h
htmlobj.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmlobj.o: /usr/include/bits/types.h /usr/include/libio.h
htmlobj.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmlobj.o: ./fltk/fltk-qdefs.h ./fltk/qstrlist.h ./fltk/qarray.h
htmlobj.o: ./fltk/qcollection.h ./fltk/kapp.h ./fltk/kcharsets.h
htmlobj.o: ./fltk/qfont.h ./fltk/qrect.h ./fltk/qpoint.h ./fltk/qsize.h
htmlobj.o: ./fltk/fltk-qbase.h ./fltk/qevent.h ./fltk/qkeycode.h
htmlobj.o: ./fltk/qlist.h ./fltk/qbuffer.h ./fltk/qiodevice.h
htmlobj.o: ./fltk/qregion.h ./fltk/qcolor.h ./fltk/qobjectdefs.h
htmlobj.o: ./fltk/qobject.h ./fltk/qmetaobject.h ./fltk/qconnection.h
htmlobj.o: ./fltk/qdict.h /usr/include/signal.h /usr/include/bits/sigset.h
htmlobj.o: /usr/include/bits/signum.h /usr/include/time.h
htmlobj.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
htmlobj.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
htmlobj.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
htmlobj.o: /usr/include/sched.h /usr/include/bits/sched.h
htmlobj.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
htmlobj.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
htmlobj.o: /usr/include/sys/types.h /usr/include/endian.h
htmlobj.o: /usr/include/bits/endian.h /usr/include/sys/select.h
htmlobj.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
htmlobj.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
htmlobj.o: ./fltk/klocale.h ./fltk/qcursor.h ./fltk/qbitmap.h htmlchain.h
htmlobj.o: htmlobj.h ./fltk/qpainter.h ./fltk/qpen.h ./fltk/qfontinfo.h
htmlobj.o: ./fltk/qbrush.h ./fltk/qpixmap.h ./fltk/qpaintdevice.h
htmlobj.o: ./fltk/qwmatrix.h ./fltk/qfontmetrics.h ./fltk/qstack.h
htmlobj.o: ./fltk/qtimer.h htmlfont.h html.h ./fltk/drag.h ./fltk/qwidget.h
htmlobj.o: ./fltk/qpalette.h htmldata.h htmlclue.h htmliter.h htmlform.h
htmlobj.o: jscript.h ../jscript/jsexec.h ../jscript/jstree.h
htmlobj.o: ../jscript/jserror.h htmltoken.h htmlframe.h ./fltk/qframe.h
htmlobj.o: htmlview.h ./fltk/qscrollbar.h /usr/include/stdlib.h
htmlobj.o: /usr/include/alloca.h /usr/include/ctype.h /usr/include/string.h
htmlobj.o: /usr/include/strings.h /usr/include/sys/stat.h
htmlobj.o: /usr/include/bits/stat.h ./fltk/qfile.h ./fltk/qimage.h
htmlobj.o: ./fltk/qdrawutil.h ./fltk/qmovie.h ./fltk/qregexp.h htmlobj.moc
debug.o: /usr/include/stdio.h /usr/include/features.h
debug.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
debug.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
debug.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
debug.o: /usr/include/bits/types.h /usr/include/libio.h
debug.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h html.h
debug.o: ./fltk/qpainter.h ./fltk/qpen.h ./fltk/qcolor.h ./fltk/qstring.h
debug.o: /usr/include/errno.h /usr/include/bits/errno.h
debug.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
debug.o: ./fltk/fltk-qdefs.h ./fltk/qfont.h ./fltk/qrect.h ./fltk/qpoint.h
debug.o: ./fltk/qarray.h ./fltk/qsize.h ./fltk/fltk-qbase.h ./fltk/qstrlist.h
debug.o: ./fltk/qcollection.h ./fltk/qevent.h ./fltk/qkeycode.h
debug.o: ./fltk/qlist.h ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
debug.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
debug.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
debug.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
debug.o: /usr/include/time.h /usr/include/bits/siginfo.h
debug.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
debug.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
debug.o: /usr/include/pthread.h /usr/include/sched.h
debug.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
debug.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
debug.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
debug.o: /usr/include/endian.h /usr/include/bits/endian.h
debug.o: /usr/include/sys/select.h /usr/include/bits/select.h
debug.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
debug.o: /usr/include/X11/Xosdefs.h ./fltk/qfontinfo.h ./fltk/qbrush.h
debug.o: ./fltk/qpixmap.h ./fltk/qpaintdevice.h ./fltk/qbitmap.h
debug.o: ./fltk/qwmatrix.h ./fltk/qfontmetrics.h ./fltk/qstack.h
debug.o: ./fltk/qtimer.h ../kdecore/kurl.h ./fltk/drag.h ./fltk/qwidget.h
debug.o: ./fltk/qcursor.h ./fltk/qpalette.h htmldata.h ./fltk/kcharsets.h
debug.o: htmlobj.h htmlfont.h htmlclue.h htmliter.h htmlform.h jscript.h
debug.o: ../jscript/jsexec.h ../jscript/jstree.h ../jscript/jserror.h
debug.o: htmltoken.h htmlframe.h ./fltk/qframe.h htmlview.h
debug.o: ./fltk/qscrollbar.h
htmlform.o: ./fltk/qobject.h ./fltk/qobjectdefs.h ./fltk/qmetaobject.h
htmlform.o: ./fltk/qstring.h
htmlform.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
htmlform.o: /usr/include/errno.h /usr/include/features.h
htmlform.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
htmlform.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
htmlform.o: /usr/include/asm/errno.h /usr/include/stdio.h
htmlform.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
htmlform.o: /usr/include/bits/types.h /usr/include/libio.h
htmlform.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
htmlform.o: ./fltk/fltk-qdefs.h ./fltk/qconnection.h ./fltk/qlist.h
htmlform.o: ./fltk/qcollection.h ./fltk/qdict.h ./fltk/fltk-qbase.h
htmlform.o: ./fltk/qstrlist.h ./fltk/qarray.h ./fltk/qevent.h ./fltk/qpoint.h
htmlform.o: ./fltk/qrect.h ./fltk/qsize.h ./fltk/qkeycode.h ./fltk/qbuffer.h
htmlform.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
htmlform.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
htmlform.o: /usr/include/sys/types.h /usr/include/time.h
htmlform.o: /usr/include/endian.h /usr/include/bits/endian.h
htmlform.o: /usr/include/sys/select.h /usr/include/bits/select.h
htmlform.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
htmlform.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
htmlform.o: /usr/include/signal.h /usr/include/bits/signum.h
htmlform.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
htmlform.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
htmlform.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
htmlform.o: /usr/include/sched.h /usr/include/bits/sched.h
htmlform.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
htmlform.o: ./fltk/qlistbox.h ./fltk/qwidget.h ./fltk/qpaintdevice.h
htmlform.o: ./fltk/qcursor.h ./fltk/qbitmap.h ./fltk/qpalette.h
htmlform.o: ./fltk/qfontmetrics.h ./fltk/qfont.h ./fltk/qfontinfo.h
htmlform.o: ./fltk/qcombobox.h ./fltk/qcheckbox.h ./fltk/qradiobutton.h
htmlform.o: ./fltk/qpushbutton.h ./fltk/qlineedit.h ./fltk/qmultilinedit.h
htmlform.o: ./fltk/qapp.h htmlform.h ./fltk/qtimer.h htmlobj.h
htmlform.o: ./fltk/qpainter.h ./fltk/qpen.h ./fltk/qbrush.h ./fltk/qpixmap.h
htmlform.o: ./fltk/qwmatrix.h ./fltk/qstack.h ../kdecore/kurl.h htmlfont.h
htmlform.o: ./fltk/kcharsets.h jscript.h ../jscript/jsexec.h
htmlform.o: ../jscript/jstree.h ../jscript/jserror.h html.h ./fltk/drag.h
htmlform.o: htmldata.h htmlclue.h htmliter.h htmltoken.h htmlframe.h
htmlform.o: ./fltk/qframe.h htmlview.h ./fltk/qscrollbar.h
htmlform.o: /usr/include/strings.h htmlform.moc
main.o: ./fltk/kapp.h ./fltk/kcharsets.h ./fltk/qfont.h ./fltk/qrect.h
main.o: ./fltk/qpoint.h ./fltk/qarray.h ./fltk/qsize.h ./fltk/fltk-qbase.h
main.o: /usr/include/stdio.h /usr/include/features.h /usr/include/sys/cdefs.h
main.o: /usr/include/gnu/stubs.h
main.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
main.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
main.o: /usr/include/bits/types.h /usr/include/libio.h
main.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
main.o: ./fltk/qstring.h /usr/include/errno.h /usr/include/bits/errno.h
main.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
main.o: ./fltk/fltk-qdefs.h ./fltk/qstrlist.h ./fltk/qcollection.h
main.o: ./fltk/qevent.h ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
main.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
main.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
main.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
main.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
main.o: /usr/include/time.h /usr/include/bits/siginfo.h
main.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
main.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
main.o: /usr/include/pthread.h /usr/include/sched.h /usr/include/bits/sched.h
main.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
main.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h /usr/include/sys/types.h
main.o: /usr/include/endian.h /usr/include/bits/endian.h
main.o: /usr/include/sys/select.h /usr/include/bits/select.h
main.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
main.o: /usr/include/X11/Xosdefs.h ./fltk/klocale.h ./fltk/qcursor.h
main.o: ./fltk/qbitmap.h html.h ./fltk/qpainter.h ./fltk/qpen.h
main.o: ./fltk/qfontinfo.h ./fltk/qbrush.h ./fltk/qpixmap.h
main.o: ./fltk/qpaintdevice.h ./fltk/qwmatrix.h ./fltk/qfontmetrics.h
main.o: ./fltk/qstack.h ./fltk/qtimer.h ../kdecore/kurl.h ./fltk/drag.h
main.o: ./fltk/qwidget.h ./fltk/qpalette.h htmldata.h htmlobj.h htmlfont.h
main.o: htmlclue.h htmliter.h htmlform.h jscript.h ../jscript/jsexec.h
main.o: ../jscript/jstree.h ../jscript/jserror.h htmltoken.h htmlframe.h
main.o: ./fltk/qframe.h htmlview.h ./fltk/qscrollbar.h http.h
main.o: ./fltk/history.h fltk/qlist.h
http.o: http.h ./fltk/qobject.h ./fltk/qobjectdefs.h ./fltk/qmetaobject.h
http.o: ./fltk/qstring.h
http.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
http.o: /usr/include/errno.h /usr/include/features.h /usr/include/sys/cdefs.h
http.o: /usr/include/gnu/stubs.h /usr/include/bits/errno.h
http.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
http.o: /usr/include/stdio.h
http.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
http.o: /usr/include/bits/types.h /usr/include/libio.h
http.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
http.o: ./fltk/fltk-qdefs.h ./fltk/qconnection.h ./fltk/qlist.h
http.o: ./fltk/qcollection.h ./fltk/qdict.h ./fltk/fltk-qbase.h
http.o: ./fltk/qstrlist.h ./fltk/qarray.h ./fltk/qevent.h ./fltk/qpoint.h
http.o: ./fltk/qrect.h ./fltk/qsize.h ./fltk/qkeycode.h ./fltk/qbuffer.h
http.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
http.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h /usr/include/sys/types.h
http.o: /usr/include/time.h /usr/include/endian.h /usr/include/bits/endian.h
http.o: /usr/include/sys/select.h /usr/include/bits/select.h
http.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
http.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
http.o: /usr/include/signal.h /usr/include/bits/signum.h
http.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
http.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
http.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
http.o: /usr/include/sched.h /usr/include/bits/sched.h
http.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
http.o: http.moc
http_.o: http.h ./fltk/qobject.h ./fltk/qobjectdefs.h ./fltk/qmetaobject.h
http_.o: ./fltk/qstring.h
http_.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
http_.o: /usr/include/errno.h /usr/include/features.h
http_.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
http_.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
http_.o: /usr/include/asm/errno.h /usr/include/stdio.h
http_.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
http_.o: /usr/include/bits/types.h /usr/include/libio.h
http_.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
http_.o: ./fltk/fltk-qdefs.h ./fltk/qconnection.h ./fltk/qlist.h
http_.o: ./fltk/qcollection.h ./fltk/qdict.h ./fltk/fltk-qbase.h
http_.o: ./fltk/qstrlist.h ./fltk/qarray.h ./fltk/qevent.h ./fltk/qpoint.h
http_.o: ./fltk/qrect.h ./fltk/qsize.h ./fltk/qkeycode.h ./fltk/qbuffer.h
http_.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
http_.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
http_.o: /usr/include/sys/types.h /usr/include/time.h /usr/include/endian.h
http_.o: /usr/include/bits/endian.h /usr/include/sys/select.h
http_.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
http_.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
http_.o: /usr/include/X11/Xosdefs.h /usr/include/signal.h
http_.o: /usr/include/bits/signum.h /usr/include/bits/siginfo.h
http_.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
http_.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
http_.o: /usr/include/pthread.h /usr/include/sched.h
http_.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
http_.o: /usr/include/bits/sigthread.h html.h ./fltk/qpainter.h ./fltk/qpen.h
http_.o: ./fltk/qfont.h ./fltk/qfontinfo.h ./fltk/qbrush.h ./fltk/qpixmap.h
http_.o: ./fltk/qpaintdevice.h ./fltk/qbitmap.h ./fltk/qwmatrix.h
http_.o: ./fltk/qfontmetrics.h ./fltk/qstack.h ./fltk/qtimer.h
http_.o: ../kdecore/kurl.h ./fltk/drag.h ./fltk/qwidget.h ./fltk/qcursor.h
http_.o: ./fltk/qpalette.h htmldata.h ./fltk/kcharsets.h htmlobj.h htmlfont.h
http_.o: htmlclue.h htmliter.h htmlform.h jscript.h ../jscript/jsexec.h
http_.o: ../jscript/jstree.h ../jscript/jserror.h htmltoken.h htmlframe.h
http_.o: ./fltk/qframe.h htmlview.h ./fltk/qscrollbar.h
../kdecore/kurl.o: ./fltk/qdir.h ./fltk/qstring.h
../kdecore/kurl.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
../kdecore/kurl.o: /usr/include/errno.h /usr/include/features.h
../kdecore/kurl.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
../kdecore/kurl.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
../kdecore/kurl.o: /usr/include/asm/errno.h /usr/include/stdio.h
../kdecore/kurl.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
../kdecore/kurl.o: /usr/include/bits/types.h /usr/include/libio.h
../kdecore/kurl.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
../kdecore/kurl.o: ./fltk/fltk-qdefs.h ../kdecore/kurl.h ./fltk/qstrlist.h
../kdecore/kurl.o: ./fltk/qarray.h ./fltk/qcollection.h ./fltk/qregexp.h
../kdecore/kurl.o: /usr/include/stdlib.h /usr/include/sys/types.h
../kdecore/kurl.o: /usr/include/time.h /usr/include/endian.h
../kdecore/kurl.o: /usr/include/bits/endian.h /usr/include/sys/select.h
../kdecore/kurl.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
../kdecore/kurl.o: /usr/include/sys/sysmacros.h /usr/include/alloca.h
../kdeui/kcursor.o: ../kdeui/kcursor.h ./fltk/kapp.h ./fltk/kcharsets.h
../kdeui/kcursor.o: ./fltk/qfont.h ./fltk/qrect.h ./fltk/qpoint.h
../kdeui/kcursor.o: ./fltk/qarray.h ./fltk/qsize.h ./fltk/fltk-qbase.h
../kdeui/kcursor.o: /usr/include/stdio.h /usr/include/features.h
../kdeui/kcursor.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
../kdeui/kcursor.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
../kdeui/kcursor.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
../kdeui/kcursor.o: /usr/include/bits/types.h /usr/include/libio.h
../kdeui/kcursor.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
../kdeui/kcursor.o: ./fltk/qstring.h /usr/include/errno.h
../kdeui/kcursor.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
../kdeui/kcursor.o: /usr/include/asm/errno.h ./fltk/fltk-qdefs.h
../kdeui/kcursor.o: ./fltk/qstrlist.h ./fltk/qcollection.h ./fltk/qevent.h
../kdeui/kcursor.o: ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
../kdeui/kcursor.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
../kdeui/kcursor.o: ./fltk/qobjectdefs.h ./fltk/qobject.h
../kdeui/kcursor.o: ./fltk/qmetaobject.h ./fltk/qconnection.h ./fltk/qdict.h
../kdeui/kcursor.o: /usr/include/signal.h /usr/include/bits/sigset.h
../kdeui/kcursor.o: /usr/include/bits/signum.h /usr/include/time.h
../kdeui/kcursor.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
../kdeui/kcursor.o: /usr/include/bits/sigcontext.h
../kdeui/kcursor.o: /usr/include/asm/sigcontext.h
../kdeui/kcursor.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
../kdeui/kcursor.o: /usr/include/sched.h /usr/include/bits/sched.h
../kdeui/kcursor.o: /usr/include/bits/pthreadtypes.h
../kdeui/kcursor.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
../kdeui/kcursor.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
../kdeui/kcursor.o: /usr/include/endian.h /usr/include/bits/endian.h
../kdeui/kcursor.o: /usr/include/sys/select.h /usr/include/bits/select.h
../kdeui/kcursor.o: /usr/include/sys/sysmacros.h
../kdeui/kcursor.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
../kdeui/kcursor.o: ./fltk/klocale.h ./fltk/qcursor.h ./fltk/qbitmap.h
fltk/qtimer.o: ./fltk/qtimer.h ./fltk/fltk-qbase.h /usr/include/stdio.h
fltk/qtimer.o: /usr/include/features.h /usr/include/sys/cdefs.h
fltk/qtimer.o: /usr/include/gnu/stubs.h
fltk/qtimer.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qtimer.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qtimer.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qtimer.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qtimer.o: ./fltk/qstring.h /usr/include/errno.h
fltk/qtimer.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qtimer.o: /usr/include/asm/errno.h ./fltk/fltk-qdefs.h ./fltk/qstrlist.h
fltk/qtimer.o: ./fltk/qarray.h ./fltk/qcollection.h ./fltk/qevent.h
fltk/qtimer.o: ./fltk/qpoint.h ./fltk/qrect.h ./fltk/qsize.h
fltk/qtimer.o: ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
fltk/qtimer.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
fltk/qtimer.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
fltk/qtimer.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
fltk/qtimer.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
fltk/qtimer.o: /usr/include/time.h /usr/include/bits/siginfo.h
fltk/qtimer.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
fltk/qtimer.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
fltk/qtimer.o: /usr/include/pthread.h /usr/include/sched.h
fltk/qtimer.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
fltk/qtimer.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
fltk/qtimer.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
fltk/qtimer.o: /usr/include/endian.h /usr/include/bits/endian.h
fltk/qtimer.o: /usr/include/sys/select.h /usr/include/bits/select.h
fltk/qtimer.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
fltk/qtimer.o: /usr/include/X11/Xosdefs.h /usr/include/unistd.h
fltk/qtimer.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
fltk/qtimer.o: /usr/include/getopt.h fltk/qtimer.moc
fltk/qobject.o: ./fltk/qobject.h ./fltk/qobjectdefs.h ./fltk/qmetaobject.h
fltk/qobject.o: ./fltk/qstring.h
fltk/qobject.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qobject.o: /usr/include/errno.h /usr/include/features.h
fltk/qobject.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qobject.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qobject.o: /usr/include/asm/errno.h /usr/include/stdio.h
fltk/qobject.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qobject.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qobject.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qobject.o: ./fltk/fltk-qdefs.h ./fltk/qconnection.h ./fltk/qlist.h
fltk/qobject.o: ./fltk/qcollection.h ./fltk/qdict.h ./fltk/fltk-qbase.h
fltk/qobject.o: ./fltk/qstrlist.h ./fltk/qarray.h ./fltk/qevent.h
fltk/qobject.o: ./fltk/qpoint.h ./fltk/qrect.h ./fltk/qsize.h
fltk/qobject.o: ./fltk/qkeycode.h ./fltk/qbuffer.h ./fltk/qiodevice.h
fltk/qobject.o: ./fltk/qregion.h ./fltk/qcolor.h /usr/include/X11/X.h
fltk/qobject.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
fltk/qobject.o: /usr/include/time.h /usr/include/endian.h
fltk/qobject.o: /usr/include/bits/endian.h /usr/include/sys/select.h
fltk/qobject.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
fltk/qobject.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
fltk/qobject.o: /usr/include/X11/Xosdefs.h /usr/include/signal.h
fltk/qobject.o: /usr/include/bits/signum.h /usr/include/bits/siginfo.h
fltk/qobject.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
fltk/qobject.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
fltk/qobject.o: /usr/include/pthread.h /usr/include/sched.h
fltk/qobject.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
fltk/qobject.o: /usr/include/bits/sigthread.h /usr/include/math.h
fltk/qobject.o: /usr/include/bits/huge_val.h /usr/include/bits/mathdef.h
fltk/qobject.o: /usr/include/bits/mathcalls.h
fltk/qobject.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/float.h
fltk/qobject.o: /usr/include/unistd.h /usr/include/bits/posix_opt.h
fltk/qobject.o: /usr/include/bits/confname.h /usr/include/getopt.h
fltk/qpainter.o: ./fltk/qpainter.h ./fltk/qpen.h ./fltk/qcolor.h
fltk/qpainter.o: ./fltk/qstring.h
fltk/qpainter.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qpainter.o: /usr/include/errno.h /usr/include/features.h
fltk/qpainter.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qpainter.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qpainter.o: /usr/include/asm/errno.h /usr/include/stdio.h
fltk/qpainter.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qpainter.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qpainter.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qpainter.o: ./fltk/fltk-qdefs.h ./fltk/qfont.h ./fltk/qrect.h
fltk/qpainter.o: ./fltk/qpoint.h ./fltk/qarray.h ./fltk/qsize.h
fltk/qpainter.o: ./fltk/fltk-qbase.h ./fltk/qstrlist.h ./fltk/qcollection.h
fltk/qpainter.o: ./fltk/qevent.h ./fltk/qkeycode.h ./fltk/qlist.h
fltk/qpainter.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
fltk/qpainter.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
fltk/qpainter.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
fltk/qpainter.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
fltk/qpainter.o: /usr/include/time.h /usr/include/bits/siginfo.h
fltk/qpainter.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
fltk/qpainter.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
fltk/qpainter.o: /usr/include/pthread.h /usr/include/sched.h
fltk/qpainter.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
fltk/qpainter.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
fltk/qpainter.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
fltk/qpainter.o: /usr/include/endian.h /usr/include/bits/endian.h
fltk/qpainter.o: /usr/include/sys/select.h /usr/include/bits/select.h
fltk/qpainter.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
fltk/qpainter.o: /usr/include/X11/Xosdefs.h ./fltk/qfontinfo.h
fltk/qpainter.o: ./fltk/qbrush.h ./fltk/qpixmap.h ./fltk/qpaintdevice.h
fltk/qpainter.o: ./fltk/qbitmap.h ./fltk/qwmatrix.h ./fltk/qfontmetrics.h
fltk/qpainter.o: ./fltk/qwidget.h ./fltk/qcursor.h ./fltk/qpalette.h
fltk/qdrawutil.o: ./fltk/qpainter.h ./fltk/qpen.h ./fltk/qcolor.h
fltk/qdrawutil.o: ./fltk/qstring.h
fltk/qdrawutil.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qdrawutil.o: /usr/include/errno.h /usr/include/features.h
fltk/qdrawutil.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qdrawutil.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qdrawutil.o: /usr/include/asm/errno.h /usr/include/stdio.h
fltk/qdrawutil.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qdrawutil.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qdrawutil.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qdrawutil.o: ./fltk/fltk-qdefs.h ./fltk/qfont.h ./fltk/qrect.h
fltk/qdrawutil.o: ./fltk/qpoint.h ./fltk/qarray.h ./fltk/qsize.h
fltk/qdrawutil.o: ./fltk/fltk-qbase.h ./fltk/qstrlist.h ./fltk/qcollection.h
fltk/qdrawutil.o: ./fltk/qevent.h ./fltk/qkeycode.h ./fltk/qlist.h
fltk/qdrawutil.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
fltk/qdrawutil.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
fltk/qdrawutil.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
fltk/qdrawutil.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
fltk/qdrawutil.o: /usr/include/time.h /usr/include/bits/siginfo.h
fltk/qdrawutil.o: /usr/include/bits/sigaction.h
fltk/qdrawutil.o: /usr/include/bits/sigcontext.h
fltk/qdrawutil.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
fltk/qdrawutil.o: /usr/include/pthread.h /usr/include/sched.h
fltk/qdrawutil.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
fltk/qdrawutil.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
fltk/qdrawutil.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
fltk/qdrawutil.o: /usr/include/endian.h /usr/include/bits/endian.h
fltk/qdrawutil.o: /usr/include/sys/select.h /usr/include/bits/select.h
fltk/qdrawutil.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
fltk/qdrawutil.o: /usr/include/X11/Xosdefs.h ./fltk/qfontinfo.h
fltk/qdrawutil.o: ./fltk/qbrush.h ./fltk/qpixmap.h ./fltk/qpaintdevice.h
fltk/qdrawutil.o: ./fltk/qbitmap.h ./fltk/qwmatrix.h ./fltk/qfontmetrics.h
fltk/qdrawutil.o: ./fltk/qdrawutil.h
fltk/qfont.o: ./fltk/qfont.h ./fltk/qrect.h ./fltk/qpoint.h ./fltk/qarray.h
fltk/qfont.o: ./fltk/qsize.h ./fltk/fltk-qbase.h /usr/include/stdio.h
fltk/qfont.o: /usr/include/features.h /usr/include/sys/cdefs.h
fltk/qfont.o: /usr/include/gnu/stubs.h
fltk/qfont.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qfont.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qfont.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qfont.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qfont.o: ./fltk/qstring.h /usr/include/errno.h /usr/include/bits/errno.h
fltk/qfont.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
fltk/qfont.o: ./fltk/fltk-qdefs.h ./fltk/qstrlist.h ./fltk/qcollection.h
fltk/qfont.o: ./fltk/qevent.h ./fltk/qkeycode.h ./fltk/qlist.h
fltk/qfont.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
fltk/qfont.o: ./fltk/qcolor.h ./fltk/qobjectdefs.h ./fltk/qobject.h
fltk/qfont.o: ./fltk/qmetaobject.h ./fltk/qconnection.h ./fltk/qdict.h
fltk/qfont.o: /usr/include/signal.h /usr/include/bits/sigset.h
fltk/qfont.o: /usr/include/bits/signum.h /usr/include/time.h
fltk/qfont.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
fltk/qfont.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
fltk/qfont.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
fltk/qfont.o: /usr/include/sched.h /usr/include/bits/sched.h
fltk/qfont.o: /usr/include/bits/pthreadtypes.h /usr/include/bits/sigthread.h
fltk/qfont.o: /usr/include/X11/X.h /usr/include/X11/Xlib.h
fltk/qfont.o: /usr/include/sys/types.h /usr/include/endian.h
fltk/qfont.o: /usr/include/bits/endian.h /usr/include/sys/select.h
fltk/qfont.o: /usr/include/bits/select.h /usr/include/sys/sysmacros.h
fltk/qfont.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
fltk/qrect.o: /usr/include/stdio.h /usr/include/features.h
fltk/qrect.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qrect.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qrect.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qrect.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qrect.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qrect.o: ./fltk/qrect.h ./fltk/qpoint.h ./fltk/qarray.h
fltk/qregexp.o: ./fltk/qregexp.h ./fltk/qstring.h
fltk/qregexp.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qregexp.o: /usr/include/errno.h /usr/include/features.h
fltk/qregexp.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qregexp.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qregexp.o: /usr/include/asm/errno.h /usr/include/stdio.h
fltk/qregexp.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qregexp.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qregexp.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qregexp.o: ./fltk/fltk-qdefs.h /usr/include/regex.h
fltk/qstring.o: /usr/include/assert.h /usr/include/features.h
fltk/qstring.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qstring.o: /usr/include/string.h
fltk/qstring.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qstring.o: ./fltk/qstring.h
fltk/qstring.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qstring.o: /usr/include/errno.h /usr/include/bits/errno.h
fltk/qstring.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
fltk/qstring.o: /usr/include/stdio.h /usr/include/bits/types.h
fltk/qstring.o: /usr/include/libio.h /usr/include/_G_config.h
fltk/qstring.o: /usr/include/bits/stdio_lim.h ./fltk/fltk-qdefs.h
fltk/qstring.o: ./fltk/qregexp.h
fltk/kcharsets.o: ./fltk/kcharsets.h ./fltk/qfont.h ./fltk/qrect.h
fltk/kcharsets.o: ./fltk/qpoint.h ./fltk/qarray.h ./fltk/qsize.h
fltk/kcharsets.o: ./fltk/fltk-qbase.h /usr/include/stdio.h
fltk/kcharsets.o: /usr/include/features.h /usr/include/sys/cdefs.h
fltk/kcharsets.o: /usr/include/gnu/stubs.h
fltk/kcharsets.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/kcharsets.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/kcharsets.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/kcharsets.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/kcharsets.o: ./fltk/qstring.h /usr/include/errno.h
fltk/kcharsets.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/kcharsets.o: /usr/include/asm/errno.h ./fltk/fltk-qdefs.h
fltk/kcharsets.o: ./fltk/qstrlist.h ./fltk/qcollection.h ./fltk/qevent.h
fltk/kcharsets.o: ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
fltk/kcharsets.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
fltk/kcharsets.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
fltk/kcharsets.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
fltk/kcharsets.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
fltk/kcharsets.o: /usr/include/time.h /usr/include/bits/siginfo.h
fltk/kcharsets.o: /usr/include/bits/sigaction.h
fltk/kcharsets.o: /usr/include/bits/sigcontext.h
fltk/kcharsets.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
fltk/kcharsets.o: /usr/include/pthread.h /usr/include/sched.h
fltk/kcharsets.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
fltk/kcharsets.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
fltk/kcharsets.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
fltk/kcharsets.o: /usr/include/endian.h /usr/include/bits/endian.h
fltk/kcharsets.o: /usr/include/sys/select.h /usr/include/bits/select.h
fltk/kcharsets.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
fltk/kcharsets.o: /usr/include/X11/Xosdefs.h
fltk/qcolor.o: ./fltk/qcolor.h ./fltk/qstring.h
fltk/qcolor.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qcolor.o: /usr/include/errno.h /usr/include/features.h
fltk/qcolor.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qcolor.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qcolor.o: /usr/include/asm/errno.h /usr/include/stdio.h
fltk/qcolor.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qcolor.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qcolor.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qcolor.o: ./fltk/fltk-qdefs.h /usr/include/string.h
fltk/qcolor.o: /usr/include/stdlib.h /usr/include/sys/types.h
fltk/qcolor.o: /usr/include/time.h /usr/include/endian.h
fltk/qcolor.o: /usr/include/bits/endian.h /usr/include/sys/select.h
fltk/qcolor.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
fltk/qcolor.o: /usr/include/sys/sysmacros.h /usr/include/alloca.h
fltk/qpixmap.o: ./fltk/qpixmap.h ./fltk/qstring.h
fltk/qpixmap.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qpixmap.o: /usr/include/errno.h /usr/include/features.h
fltk/qpixmap.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qpixmap.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qpixmap.o: /usr/include/asm/errno.h /usr/include/stdio.h
fltk/qpixmap.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qpixmap.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qpixmap.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qpixmap.o: ./fltk/fltk-qdefs.h ./fltk/qpaintdevice.h ./fltk/qobject.h
fltk/qpixmap.o: ./fltk/qobjectdefs.h ./fltk/qmetaobject.h
fltk/qpixmap.o: ./fltk/qconnection.h ./fltk/qlist.h ./fltk/qcollection.h
fltk/qpixmap.o: ./fltk/qdict.h ./fltk/fltk-qbase.h ./fltk/qstrlist.h
fltk/qpixmap.o: ./fltk/qarray.h ./fltk/qevent.h ./fltk/qpoint.h
fltk/qpixmap.o: ./fltk/qrect.h ./fltk/qsize.h ./fltk/qkeycode.h
fltk/qpixmap.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
fltk/qpixmap.o: ./fltk/qcolor.h /usr/include/X11/X.h /usr/include/X11/Xlib.h
fltk/qpixmap.o: /usr/include/sys/types.h /usr/include/time.h
fltk/qpixmap.o: /usr/include/endian.h /usr/include/bits/endian.h
fltk/qpixmap.o: /usr/include/sys/select.h /usr/include/bits/select.h
fltk/qpixmap.o: /usr/include/bits/sigset.h /usr/include/sys/sysmacros.h
fltk/qpixmap.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
fltk/qpixmap.o: /usr/include/signal.h /usr/include/bits/signum.h
fltk/qpixmap.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
fltk/qpixmap.o: /usr/include/bits/sigcontext.h /usr/include/asm/sigcontext.h
fltk/qpixmap.o: /usr/include/bits/sigstack.h /usr/include/pthread.h
fltk/qpixmap.o: /usr/include/sched.h /usr/include/bits/sched.h
fltk/qpixmap.o: /usr/include/bits/pthreadtypes.h
fltk/qpixmap.o: /usr/include/bits/sigthread.h ./fltk/qbitmap.h
fltk/qpixmap.o: ./fltk/qwmatrix.h /usr/include/Imlib.h
fltk/qpixmap.o: /usr/include/Imlib_types.h /usr/include/X11/Xutil.h
fltk/qpixmap.o: /usr/include/X11/Xatom.h /usr/include/X11/Xos.h
fltk/qpixmap.o: /usr/include/string.h /usr/include/fcntl.h
fltk/qpixmap.o: /usr/include/bits/fcntl.h /usr/include/unistd.h
fltk/qpixmap.o: /usr/include/bits/posix_opt.h /usr/include/bits/confname.h
fltk/qpixmap.o: /usr/include/getopt.h /usr/include/sys/time.h
fltk/qpixmap.o: /usr/include/bits/time.h /usr/include/X11/extensions/shape.h
fltk/qpixmap.o: /usr/include/stdlib.h /usr/include/alloca.h
fltk/qpixmap.o: /usr/include/math.h /usr/include/bits/huge_val.h
fltk/qpixmap.o: /usr/include/bits/mathdef.h /usr/include/bits/mathcalls.h
fltk/qpixmap.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/float.h
fltk/qpixmap.o: /usr/include/ctype.h /usr/include/netinet/in.h
fltk/qpixmap.o: /usr/include/limits.h /usr/include/bits/posix1_lim.h
fltk/qpixmap.o: /usr/include/bits/local_lim.h /usr/include/linux/limits.h
fltk/qpixmap.o: /usr/include/bits/posix2_lim.h /usr/include/stdint.h
fltk/qpixmap.o: /usr/include/bits/wordsize.h /usr/include/bits/socket.h
fltk/qpixmap.o: /usr/include/bits/sockaddr.h /usr/include/asm/socket.h
fltk/qpixmap.o: /usr/include/asm/sockios.h /usr/include/bits/in.h
fltk/qpixmap.o: /usr/include/bits/byteswap.h
fltk/qfontinfo.o: ./fltk/qfontinfo.h ./fltk/qfont.h ./fltk/qrect.h
fltk/qfontinfo.o: ./fltk/qpoint.h ./fltk/qarray.h ./fltk/qsize.h
fltk/qfontinfo.o: ./fltk/fltk-qbase.h /usr/include/stdio.h
fltk/qfontinfo.o: /usr/include/features.h /usr/include/sys/cdefs.h
fltk/qfontinfo.o: /usr/include/gnu/stubs.h
fltk/qfontinfo.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qfontinfo.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qfontinfo.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qfontinfo.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qfontinfo.o: ./fltk/qstring.h /usr/include/errno.h
fltk/qfontinfo.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qfontinfo.o: /usr/include/asm/errno.h ./fltk/fltk-qdefs.h
fltk/qfontinfo.o: ./fltk/qstrlist.h ./fltk/qcollection.h ./fltk/qevent.h
fltk/qfontinfo.o: ./fltk/qkeycode.h ./fltk/qlist.h ./fltk/qbuffer.h
fltk/qfontinfo.o: ./fltk/qiodevice.h ./fltk/qregion.h ./fltk/qcolor.h
fltk/qfontinfo.o: ./fltk/qobjectdefs.h ./fltk/qobject.h ./fltk/qmetaobject.h
fltk/qfontinfo.o: ./fltk/qconnection.h ./fltk/qdict.h /usr/include/signal.h
fltk/qfontinfo.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
fltk/qfontinfo.o: /usr/include/time.h /usr/include/bits/siginfo.h
fltk/qfontinfo.o: /usr/include/bits/sigaction.h
fltk/qfontinfo.o: /usr/include/bits/sigcontext.h
fltk/qfontinfo.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
fltk/qfontinfo.o: /usr/include/pthread.h /usr/include/sched.h
fltk/qfontinfo.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
fltk/qfontinfo.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
fltk/qfontinfo.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
fltk/qfontinfo.o: /usr/include/endian.h /usr/include/bits/endian.h
fltk/qfontinfo.o: /usr/include/sys/select.h /usr/include/bits/select.h
fltk/qfontinfo.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
fltk/qfontinfo.o: /usr/include/X11/Xosdefs.h
fltk/qwidget.o: ./fltk/qpoint.h ./fltk/qarray.h ./fltk/qwidget.h
fltk/qwidget.o: ./fltk/fltk-qbase.h /usr/include/stdio.h
fltk/qwidget.o: /usr/include/features.h /usr/include/sys/cdefs.h
fltk/qwidget.o: /usr/include/gnu/stubs.h
fltk/qwidget.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qwidget.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qwidget.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qwidget.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qwidget.o: ./fltk/qstring.h /usr/include/errno.h
fltk/qwidget.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qwidget.o: /usr/include/asm/errno.h ./fltk/fltk-qdefs.h
fltk/qwidget.o: ./fltk/qstrlist.h ./fltk/qcollection.h ./fltk/qevent.h
fltk/qwidget.o: ./fltk/qrect.h ./fltk/qsize.h ./fltk/qkeycode.h
fltk/qwidget.o: ./fltk/qlist.h ./fltk/qbuffer.h ./fltk/qiodevice.h
fltk/qwidget.o: ./fltk/qregion.h ./fltk/qcolor.h ./fltk/qobjectdefs.h
fltk/qwidget.o: ./fltk/qobject.h ./fltk/qmetaobject.h ./fltk/qconnection.h
fltk/qwidget.o: ./fltk/qdict.h /usr/include/signal.h
fltk/qwidget.o: /usr/include/bits/sigset.h /usr/include/bits/signum.h
fltk/qwidget.o: /usr/include/time.h /usr/include/bits/siginfo.h
fltk/qwidget.o: /usr/include/bits/sigaction.h /usr/include/bits/sigcontext.h
fltk/qwidget.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
fltk/qwidget.o: /usr/include/pthread.h /usr/include/sched.h
fltk/qwidget.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
fltk/qwidget.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
fltk/qwidget.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
fltk/qwidget.o: /usr/include/endian.h /usr/include/bits/endian.h
fltk/qwidget.o: /usr/include/sys/select.h /usr/include/bits/select.h
fltk/qwidget.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
fltk/qwidget.o: /usr/include/X11/Xosdefs.h ./fltk/qpaintdevice.h
fltk/qwidget.o: ./fltk/qcursor.h ./fltk/qbitmap.h ./fltk/qpalette.h
fltk/qwidget.o: ./fltk/qfontmetrics.h ./fltk/qfont.h ./fltk/qfontinfo.h
fltk/history.o: /usr/include/malloc.h
fltk/history.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/history.o: /usr/include/stdlib.h /usr/include/features.h
fltk/history.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/history.o: /usr/include/sys/types.h /usr/include/bits/types.h
fltk/history.o: /usr/include/time.h /usr/include/endian.h
fltk/history.o: /usr/include/bits/endian.h /usr/include/sys/select.h
fltk/history.o: /usr/include/bits/select.h /usr/include/bits/sigset.h
fltk/history.o: /usr/include/sys/sysmacros.h /usr/include/alloca.h
fltk/history.o: /usr/include/stdio.h
fltk/history.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/history.o: /usr/include/libio.h /usr/include/_G_config.h
fltk/history.o: /usr/include/bits/stdio_lim.h /usr/include/string.h
fltk/history.o: ./fltk/history.h
fltk/qscrollbar.o: ./fltk/qscrollbar.h ./fltk/qwidget.h ./fltk/fltk-qbase.h
fltk/qscrollbar.o: /usr/include/stdio.h /usr/include/features.h
fltk/qscrollbar.o: /usr/include/sys/cdefs.h /usr/include/gnu/stubs.h
fltk/qscrollbar.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stddef.h
fltk/qscrollbar.o: /usr/lib/gcc-lib/i386-redhat-linux/egcs-2.91.66/include/stdarg.h
fltk/qscrollbar.o: /usr/include/bits/types.h /usr/include/libio.h
fltk/qscrollbar.o: /usr/include/_G_config.h /usr/include/bits/stdio_lim.h
fltk/qscrollbar.o: ./fltk/qstring.h /usr/include/errno.h
fltk/qscrollbar.o: /usr/include/bits/errno.h /usr/include/linux/errno.h
fltk/qscrollbar.o: /usr/include/asm/errno.h ./fltk/fltk-qdefs.h
fltk/qscrollbar.o: ./fltk/qstrlist.h ./fltk/qarray.h ./fltk/qcollection.h
fltk/qscrollbar.o: ./fltk/qevent.h ./fltk/qpoint.h ./fltk/qrect.h
fltk/qscrollbar.o: ./fltk/qsize.h ./fltk/qkeycode.h ./fltk/qlist.h
fltk/qscrollbar.o: ./fltk/qbuffer.h ./fltk/qiodevice.h ./fltk/qregion.h
fltk/qscrollbar.o: ./fltk/qcolor.h ./fltk/qobjectdefs.h ./fltk/qobject.h
fltk/qscrollbar.o: ./fltk/qmetaobject.h ./fltk/qconnection.h ./fltk/qdict.h
fltk/qscrollbar.o: /usr/include/signal.h /usr/include/bits/sigset.h
fltk/qscrollbar.o: /usr/include/bits/signum.h /usr/include/time.h
fltk/qscrollbar.o: /usr/include/bits/siginfo.h /usr/include/bits/sigaction.h
fltk/qscrollbar.o: /usr/include/bits/sigcontext.h
fltk/qscrollbar.o: /usr/include/asm/sigcontext.h /usr/include/bits/sigstack.h
fltk/qscrollbar.o: /usr/include/pthread.h /usr/include/sched.h
fltk/qscrollbar.o: /usr/include/bits/sched.h /usr/include/bits/pthreadtypes.h
fltk/qscrollbar.o: /usr/include/bits/sigthread.h /usr/include/X11/X.h
fltk/qscrollbar.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
fltk/qscrollbar.o: /usr/include/endian.h /usr/include/bits/endian.h
fltk/qscrollbar.o: /usr/include/sys/select.h /usr/include/bits/select.h
fltk/qscrollbar.o: /usr/include/sys/sysmacros.h /usr/include/X11/Xfuncproto.h
fltk/qscrollbar.o: /usr/include/X11/Xosdefs.h ./fltk/qpaintdevice.h
fltk/qscrollbar.o: ./fltk/qcursor.h ./fltk/qbitmap.h ./fltk/qpalette.h
fltk/qscrollbar.o: ./fltk/qfontmetrics.h ./fltk/qfont.h ./fltk/qfontinfo.h
