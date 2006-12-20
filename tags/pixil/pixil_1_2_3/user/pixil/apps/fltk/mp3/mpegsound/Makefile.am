noinst_LIBRARIES	= libmpegsound.a

libmpegsound_a_SOURCES  = soundinputstream.cc fileinput.cc httpinput.cc \
			  soundplayer.cc rawplayer.cc rawtofile.cc \
			  mpegtable.cc filter.cc filter_2.cc \
			  mpegtoraw.cc mpeglayer1.cc mpeglayer2.cc \
			  mpeglayer3.cc bitwindow.cc huffmantable.cc \
			  wavetoraw.cc \
			  fileplayer.cc 

noinst_HEADERS		= mpegsound.h mpegsound_locals.h mpeglayer3tables.h fixed_point.h

INCLUDES                = @INCLUDES@ -I. -I../fltk
