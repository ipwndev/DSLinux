# -*- makefile -*-

AUTOMAKE_OPTIONS = foreign

KBD_UNSUBST_MAN = chvt.1 deallocvt.1 fgconsole.1 \
	kbd_mode.1 psfaddtable.1 \
	psfgettable.1 psfstriptable.1 setleds.1 \
	setmetamode.1 showkey.1 console.4 \
	console_ioctls.4 keymaps.5 \
	getkeycodes.8 setkeycodes.8 resizecons.8 \
	unicode_start.1 unicode_stop.1 \
	vt-is-UTF8.1 showcfont.1 charset.1 \
	openvt.1

KBD_SUBST_MAN = consolechars.8 dumpkeys.1 loadkeys.1

man_MANS = $(KBD_SUBST_MAN) $(KBD_UNSUBST_MAN)

$(KBD_SUBST_MAN) :
	rm -f $@
	sed s+'@'datadir@+$(datadir)+g $(srcdir)/$(@:=.in) > $@

EXTRA_DIST = $(KBD_UNSUBST_MAN) $(KBD_SUBST_MAN:=.in)

clean-local:
	rm -f $(KBD_SUBST_MAN)
