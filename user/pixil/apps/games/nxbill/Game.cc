 /*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "objects.h"

#define True 1
#define False 0

Horde bill;
Network net;
Library OS;
Bucket bucket;
Spark spark;
Scorelist scores;
Game game;
UI ui;

#ifndef XBILL_HOME
#define XBILL_HOME "/usr/local/share/xbill/images";
#endif

char xbill_path[256] = XBILL_HOME;

int
Game::RAND(int lb, int ub)
{
    return (rand() % (ub - lb + 1) + lb);
}

int
Game::MAX(int x, int y)
{
    return (x > y ? x : y);
}

int
Game::MIN(int x, int y)
{
    return (x < y ? x : y);
}

int
Game::INTERSECT(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
		int h2)
{
    return (((x2 - x1 <= w1 && x2 - x1 >= 0)
	     || (x1 - x2 <= w2 && x1 - x2 >= 0))
	    && ((y2 - y1 <= h1 && y2 - y1 >= 0)
		|| (y1 - y2 <= h2 && y1 - y2 >= 0)));
}

void
Game::setup_level(unsigned int lev)
{
    level = lev;
    bill.setup();
    grabbed = EMPTY;

#ifndef PDA
    ui.set_cursor(DEFAULTC);
#endif

    net.setup();
    iteration = efficiency = 0;
}

void
Game::start(unsigned int lev)
{
    state = PLAYING;
    score = 0;
    ui.restart_timer();
    ui.set_pausebutton(True);
    setup_level(lev);
}

void
Game::quit()
{
    scores.write();
    exit(0);
}

void
Game::update_info()
{
    static char str[80];
    sprintf(str, "Bill:%d/%d  System:%d/%d/%d  Level:%d  Score:%d",
	    bill.on_screen, bill.off_screen, net.base, net.off,
	    net.win, level, score);
    ui.draw_str(str, 5, playheight - 5);
    efficiency += ((100 * net.base - 10 * net.win) / net.units);
}

void
Game::update_score(int action)
{
    switch (action) {
    case ENDLEVEL:
	score += (level * efficiency / iteration);
	break;
    default:
	score += (action * action * BILLPOINTS);
    }
}

void
Game::warp_to_level(unsigned int lev)
{
    if (state == (int) PLAYING) {
	if (lev <= level)
	    return;
	setup_level(lev);
    } else {
	if (lev <= 0)
	    return;
	start(lev);
    }
}

void
Game::button_press(int x, int y)
{
    int i, counter = 0, flag = 0;
    if (state != (int) PLAYING)
	return;

    /* Ignore button presses if we are paused! */

    if (ui.paused())
	return;

#ifndef PDA
    ui.set_cursor(DOWNC);
#endif

    if (bucket.clicked(x, y)) {
#ifndef PDA
	ui.set_cursor(BUCKETC);
#endif
	grabbed = BUCKET;
    }
    for (i = 0; i < bill.MAX_BILLS && !flag; i++) {
	if (bill.list[i].state == bill.list[i].OFF
	    || bill.list[i].state == bill.list[i].DYING)
	    continue;
	if (bill.list[i].state == bill.list[i].STRAY &&
	    bill.list[i].clickedstray(x, y)) {
#ifndef PDA
	    ui.set_cursor(bill.list[i].cargo);
#endif
	    grabbed = i;
	    flag = 1;
	} else if (bill.list[i].state != bill.list[i].STRAY &&
		   bill.list[i].clicked(x, y)) {
	    if (bill.list[i].state == bill.list[i].AT)
		net.computers[bill.list[i].target_c].busy = 0;
	    bill.list[i].index = -1;
	    bill.list[i].cels = bill.dcels;
	    bill.list[i].x_offset = -2;
	    bill.list[i].y_offset = -15;
	    bill.list[i].state = bill.list[i].DYING;
	    counter++;
	}
    }
    if (counter)
	update_score(counter);
}

void
Game::button_release(int x, int y)
{
    int i;

#ifndef PDA
    ui.set_cursor(DEFAULTC);
#endif
    if (state != (int) PLAYING || grabbed == EMPTY)
	return;
    if (grabbed == BUCKET) {
	grabbed = EMPTY;
	for (i = 0; i < net.ncables; i++)
	    if (net.cables[i].onspark(x, y)) {
		net.cables[i].active = 0;
		net.cables[i].delay = spark.delay(level);
	    }
	return;
    }
    for (i = 0; i < net.units; i++)
	if (net.computers[i].oncomputer(x, y)
	    && net.computers[i].compatible(bill.list[grabbed].cargo)
	    &&
	    (net.computers[i].os == OS.WINGDOWS ||
	     net.computers[i].os == OS.OFF)) {
	    net.base++;
	    if (net.computers[i].os == OS.WINGDOWS)
		net.win--;
	    else
		net.off--;
	    net.computers[i].os = bill.list[grabbed].cargo;
	    bill.list[grabbed].state = bill.list[grabbed].OFF;
	    grabbed = EMPTY;
	    return;
	}
    grabbed = EMPTY;
}

void
Game::update()
{
    switch (state) {
    case PLAYING:
	ui.clear();
	bucket.draw();
	net.update();
	net.draw();
	bill.update();
	bill.draw();
	update_info();
	if (!(bill.on_screen + bill.off_screen)) {
	    update_score(ENDLEVEL);
	    state = BETWEEN;
	}
	if ((net.base + net.off) <= 1)
	    state = END;
	break;
    case END:
	ui.clear();
	net.toasters();
	net.draw();
	ui.refresh();
	ui.popup_dialog(ENDGAME);
	if (score > scores.score[9])
	    ui.popup_dialog(ENTERNAME);
	scores.update();
	ui.popup_dialog(HIGHSCORE);
	ui.clear();
	ui.draw_centered(&logo);
	ui.kill_timer();
	ui.set_pausebutton(False);
	state = WAITING;
	break;
    case BETWEEN:
	ui.update_scorebox(level, score);
	ui.popup_dialog(SCORE);
	state = PLAYING;
	setup_level(++level);
	break;
    }
    ui.refresh();
    iteration++;
}

void
Game::main(int argc, char **argv)
{
    int c;
    extern char *optarg;

    level = 0;
    //xbill_path[0] = 0;

    while (argv && argv[0] && (c = getopt(argc, argv, "I:l:L:")) != -1)
	switch (c) {
	case 'l':
	case 'L':
	    level = MAX(1, atoi(optarg));
	    break;
	case 'I':
	    if (strlen(optarg)) {
		strcpy(xbill_path, optarg);

		if (xbill_path[strlen(xbill_path) - 1] != '/')
		    strcat(xbill_path, "/");
	    }

	    break;
	}

    ui.initialize(&argc, argv);

    srand(time(NULL));
    ui.make_mainwin();
    ui.graph_init();
    ui.clear();
    logo.load("logo");
    ui.draw_centered(&logo);
    ui.refresh();
    ui.make_windows();
    scores.read();
    scores.update();

    bill.load_pix();
    OS.load_pix();
    net.load_pix();
    bucket.load_pix();
    spark.load_pix();

#ifndef PDA
    ui.load_cursors();
#endif
    state = WAITING;
    if (level)
	start(level);
    else
	ui.set_pausebutton(False);
    ui.MainLoop();
    exit(0);
}

int
main(int argc, char **argv)
{
    if (argc > 1 && !strcmp(argv[1], "-v")) {
	printf("XBill version 2.0\n\n");
	exit(0);
    }
    if (argc > 1 && !strcmp(argv[1], "-h")) {
	printf("microBill version 1.0\n");
	printf("Options:\n");
	printf("-l n, -L n\tStart at level n.\n");
	printf("-i [path]\tIndicate where the images live\n");
	printf("if not ./pixmaps\n");
	exit(0);
    }
    game.main(argc, argv);
}
