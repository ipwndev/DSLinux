
#ifndef X11_UI_H
#define X11_UI_H

#include "Picture.h"
#include "MCursor.h"

class UI
{
    XtIntervalId timer;
    int playing;
    Picture icon;
    void get_coords(Position * x, Position * y);
    MCursor defaultcursor, downcursor;
    GC stdgc, whitegc;
  public:
      UI()
    {
	playing = 0;
	timer = (XtIntervalId) 0;
    }
    Display *display;
    XtAppContext app;
    Drawable window, rootwindow;
    Colormap colormap;
    int depth;
    XColor white, black;
    Pixmap offscreen;

    void restart_timer();
    void kill_timer();

    void pause_game();
    void resume_game();

    void initialize(int *argc, char **argv);
    void make_mainwin();
    void make_windows();
    void popup_dialog(int dialog);

    void set_cursor(int cursor);
    void load_cursors();
    void graph_init();
    void clear();
    void refresh();
    void draw(Picture picture, int x, int y);
    void draw_centered(Picture picture);
    void draw_line(int x1, int y1, int x2, int y2);
    void draw_str(char *str, int x, int y);

    void set_pausebutton(int action);
    void MainLoop();

    void update_scorebox(int level, int score);
    void update_hsbox(char *str);
};

#endif
