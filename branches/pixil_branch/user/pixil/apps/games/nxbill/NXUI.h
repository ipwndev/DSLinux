/* Copyright 2003, Century Software */

#ifndef NX_UI_H
#define NX_UI_H

#include "NXPicture.h"

#ifndef PDA
#include "NXMCursor.h"
#endif

extern "C"
{
#include "nano-X.h"
#include "nxdraw.h"
}

class UI
{
  private:

    /* Widgets */
    GR_WINDOW_ID toplevel, menubar, field;
    GR_WINDOW_ID scorebox, endgamebox;

    /* Graphics contexts */
    GR_GC_ID stdgc, whitegc, menubargc;

    /* Font ID */
    GR_FONT_ID global_font;

    GR_TIMEOUT timeout;
    int playing;
    NXPicture icon;

#ifndef PDA
    NXMCursor defaultcursor, downcursor;
#endif

  public:
      UI();

    GR_WINDOW_ID offscreen;

    void restart_timer();
    void kill_timer();

    void pause_game();
    void resume_game();

    void initialize(int *argc, char **argv);
    void make_mainwin();
    void make_windows();
    void popup_dialog(int dialog);

#ifndef PDA
    void set_cursor(int cursor);
    void load_cursors();
#endif

    void graph_init();
    void clear();
    void refresh();
    void draw(NXPicture * picture, int x, int y);
    void draw_centered(NXPicture * picture);
    void draw_line(int x1, int y1, int x2, int y2);
    void draw_str(char *str, int x, int y);

    void set_pausebutton(int action);
    void MainLoop();

    void update_scorebox(int level, int score);
    void update_hsbox(char *str);

    void redraw_menubar();
    void menubar_buttonup(int x, int y);
    void close_program();

    int paused()
    {
	return (playing);
    }
};

#endif
