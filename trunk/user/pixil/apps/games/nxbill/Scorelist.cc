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

extern char xbill_path[256];

FILE *
Scorelist::open_file(char *mode)
{
    char file[255];
    sprintf(file, "%sscores", xbill_path);
    return fopen(file, mode);
}

void
Scorelist::read()
{
    FILE *scorefile = open_file("r");
    int i;
    if (scorefile) {
	for (i = 0; i < 10; i++) {
	    fgets(name[i], 21, scorefile);
	    fscanf(scorefile, "%d%d\n", &(level[i]), &(score[i]));
	}
	fclose(scorefile);
    } else
	for (i = 0; i < 10; i++) {
	    strcpy(name[i], "me");
	    level[i] = score[i] = 0;
	}
}

void
Scorelist::write()
{
    int i, j;
    FILE *scorefile = open_file("w");
    if (!scorefile)
	return;
    for (i = 0; i < 10; i++) {
	fputs(name[i], scorefile);
	for (j = strlen(name[i]); j < 25; j++)
	    fputc(' ', scorefile);
	fprintf(scorefile, " %d %d\n", level[i], score[i]);
    }
    fclose(scorefile);
}

/*  Add new high score to list   */
void
Scorelist::recalc(char *str)
{
    int i;
    if (score[9] >= game.score)
	return;
    for (i = 9; i > 0; i--) {
	if (score[i - 1] < game.score) {
	    strcpy(name[i], name[i - 1]);
	    level[i] = level[i - 1];
	    score[i] = score[i - 1];
	} else
	    break;
    }
    strcpy(name[i], str);
    level[i] = game.level;
    score[i] = game.score;
}

void
Scorelist::update()
{
    char str[500], temp[40];
    int i, j;
    strcpy(str, "High Scores:\n\n");
    strcat(str, "Name                 Level    Score\n");
    for (i = 0; i < 10; i++) {
	strcat(str, name[i]);
	for (j = strlen(name[i]); j < 21; j++)
	    strcat(str, " ");
	sprintf(temp, "%5d  %7d\n", level[i], score[i]);
	strcat(str, temp);
    }
    ui.update_hsbox(str);
}
