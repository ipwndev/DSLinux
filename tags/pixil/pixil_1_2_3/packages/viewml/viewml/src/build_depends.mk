include ../makeinclude

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

check-depends: 
	if [ ! -s .build_depends ]; then $(MAKE) -f build_depends.mk force-depends; fi

force-depends:
	@ echo "Generating dependancies for `pwd`..."
	@ echo "# Dependancy file for `pwd`" > .build_depends
	@ echo "# Automatically generated.  Do not edit." >> .build_depends
	$(CXX) -M $(INCLUDES) $(CXXFLAGS) $(CPPFILES) $(OTHERCPP) >> .build_depends

# End of file
