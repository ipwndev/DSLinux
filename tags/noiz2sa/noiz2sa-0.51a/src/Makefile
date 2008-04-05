# Noiz2sa makefile(MinGW 2.0.0)
# $Id: Makefile,v 1.4 2003/08/10 09:00:05 kenta Exp $

NAME   = noiz2sa
O      = o
RM     = rm -f
CC     = gcc
CXX    = g++

PROG        = $(NAME).exe

DEFAULT_CFLAGS = `sdl-config --cflags`
LDFLAGS        = `sdl-config --libs` -L. -lmingw32 -lmingwex -lSDL_mixer -lbulletml -mwindows -lstdc++

MORE_CFLAGS = -O3

CFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS)
CPPFLAGS  = $(DEFAULT_CFLAGS) $(MORE_CFLAGS) -I./bulletml/

OBJS =	$(NAME).$(O) ship.$(O) shot.$(O) frag.$(O) bonus.$(O) \
	foe.$(O) foecommand.$(O) barragemanager.$(O) attractmanager.$(O) \
	background.$(O) letterrender.$(O) \
	screen.$(O) clrtbl.$(O) vector.$(O) degutil.$(O) rand.$(O) soundmanager.$(O) \
	$(NAME)_res.$(O)

$(PROG): $(OBJS) 
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LDFLAGS)

$(NAME)_res.o: $(NAME).rc
	windres -i $(NAME).rc -o $(NAME)_res.o
clean:
	$(RM) $(PROG) *.$(O)
