
#ifndef GAME_H
#define GAME_H

#include "objects.h"

class Game
{
    unsigned state;
    int efficiency;
  public:
    unsigned score, level, iteration;
    NXPicture logo;
    int grabbed;

    /* This is the actual size of the whole shooting match */

    static const unsigned short scrwidth = 240;
    static const unsigned short scrheight = 286;

    /* This is the size of the playground */
    static const unsigned short playwidth = 240;
    static const unsigned short playheight = 256;

    static const int PLAYING = 1;
    static const int BETWEEN = 2;
    static const int END = 3;
    static const int WAITING = 4;

#define DEFAULTC         0		/* cursors */
#define DOWNC           (-1)
#define BUCKETC         (-2)

#define ENDGAME 	200		/* dialog window IDs */
    static const int ENTERNAME = 201;
    static const int HIGHSCORE = 202;
    static const int SCORE = 203;

    static const int ENDLEVEL = -1;	/* Codes for updating score */
    static const int BILLPOINTS = 5;

    static const int EMPTY = -2;	/* Grabbed object */
    static const int BUCKET = -1;

    int RAND(int lb, int ub);
    int MAX(int x, int y);
    int MIN(int x, int y);
    int INTERSECT(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
		  int h2);

    void setup_level(unsigned int lev);
    void start(unsigned int lev);
    void quit();
    void update_info();
    void update_score(int action);
    void warp_to_level(unsigned int lev);
    void button_press(int x, int y);
    void button_release(int x, int y);
    void update();
    void main(int argc, char **argv);
};

#endif
