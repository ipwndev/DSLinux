#
#	Makefile -- Build instructions for user games level apps
#

.EXPORT_ALL_VARIABLES:

dir_y =
dir_n =
dir_ = 

dir_$(CONFIG_USER_GAMES_ADVENT4)        += advent4
dir_$(CONFIG_USER_GAMES_BSDGAMES)	+= bsdgames
dir_$(CONFIG_USER_GAMES_CNIBBLES)	+= cnibbles
dir_$(CONFIG_USER_GAMES_DUNGEON)        += dungeon
dir_$(CONFIG_USER_GAMES_PACMAN)		+= pacman
dir_$(CONFIG_USER_GAMES_RUBIK)          += rubik
dir_$(CONFIG_USER_GAMES_SUDOKU)			+= sudoku
dir_$(CONFIG_USER_GAMES_XRICK)             += xrick
dir_$(CONFIG_USER_GAMES_NOIZ2SA)             += noiz2sa
#dir_$(CONFIG_USER_GAMES_XMAME)		    += xmame

all:
	+for i in $(dir_y) dummy ; do \
		[ $$i = dummy ] || $(MAKE) -C $$i || exit $? ; \
	done

romfs:
	+for i in $(dir_y) dummy ; do \
		[ $$i = dummy ] || $(MAKE) -C $$i romfs || exit $? ; \
	done

clean:
	-for i in $(dir_y) $(dir_n) $(dir_) ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i clean; \
	done

