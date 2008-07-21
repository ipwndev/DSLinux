# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign
UNICODEDATA = @UNICODEDATA@

all: @UNICODELISTS@

unicodelists: utflist.long

utflist.long: ${UNICODEDATA} ${top_srcdir}/local-scripts/make-utflist
	${top_srcdir}/local-scripts/make-utflist < $< > $@

EXTRA_DIST = screenmaps/* spawn_*.c README.* sample.* utflist.* ??????
