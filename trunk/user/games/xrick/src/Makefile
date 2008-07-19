
EXEC = xrick
OBJS = unzip.o data.o scr_xrick.o scr_pause.o scr_imain.o scr_imap.o scr_gameover.o scr_getname.o dat_picsPC.o dat_picsST.o dat_screens.o dat_tilesPC.o dat_tilesST.o dat_maps.o dat_ents.o dat_spritesST.o dat_spritesPC.o ents.o e_bullet.o e_bomb.o e_rick.o e_sbonus.o e_them.o e_bonus.o e_box.o rects.o util.o game.o xrick.o draw.o maps.o sysvid.o syskbd.o control.o system.o scroller.o sysevt.o sysarg.o syssnd.o sysjoy.o dat_snd.o

CFLAGS += -I../include
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)  $(LDLIBS) $(LIBGCC) -lSDL -lpthread -lz $(LIBM)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -rf $(EXEC) *.elf *.gdb *.o
