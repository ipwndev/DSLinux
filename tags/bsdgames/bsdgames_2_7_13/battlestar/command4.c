/*	$NetBSD: command4.c,v 1.2 2003/08/07 09:37:00 agc Exp $	*/

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char sccsid[] = "@(#)com4.c	8.2 (Berkeley) 4/28/95";
#else
__RCSID("$NetBSD: command4.c,v 1.2 2003/08/07 09:37:00 agc Exp $");
#endif
#endif				/* not lint */

#include "extern.h"

int
take(from)
	unsigned int from[];
{
	int     firstnumber, heavy, bulky, value;

	firstnumber = wordnumber;
	if (wordnumber < wordcount && wordvalue[wordnumber + 1] == OFF) {
		wordnumber++;
		wordvalue[wordnumber] = TAKEOFF;
		wordtype[wordnumber] = VERB;
		return (cypher());
	} else {
		wordnumber++;
		while (wordnumber <= wordcount && wordtype[wordnumber] == OBJECT) {
			value = wordvalue[wordnumber];
			printf("%s:\n", objsht[value]);
			heavy = (carrying + objwt[value]) <= WEIGHT;
			bulky = (encumber + objcumber[value]) <= CUMBER;
			if ((testbit(from, value) || wiz || tempwiz) && heavy && bulky && !testbit(inven, value)) {
				setbit(inven, value);
				carrying += objwt[value];
				encumber += objcumber[value];
				ourtime++;
				if (testbit(from, value))
					printf("Taken.\n");
				else
					printf("Zap! Taken from thin air.\n");
				clearbit(from, value);
				if (value == MEDALION)
					win--;
			} else if (testbit(inven, value))
				printf("You're already holding %s%s.\n",
				    A_OR_AN_OR_BLANK(value),
				    objsht[value]);
			else if (!testbit(from, value))
				printf("I don't see any %s around here.\n", objsht[value]);
			else if (!heavy)
				printf("The %s %stoo heavy.\n", objsht[value],
				    IS_OR_ARE(value));
			else
				printf("The %s %stoo cumbersome to hold.\n",
				    objsht[value], IS_OR_ARE(value));
			if (wordnumber < wordcount - 1 && wordvalue[++wordnumber] == AND)
				wordnumber++;
			else
				return (firstnumber);
		}
	}
	/* special cases with their own return()'s */

	if (wordnumber <= wordcount && wordtype[wordnumber] == NOUNS)
		switch (wordvalue[wordnumber]) {

		case SWORD:
			if (testbit(from, SWORD)) {
				wordtype[wordnumber--] = OBJECT;
				return (take(from));
			}
			if (testbit(from, TWO_HANDED)) {
				wordvalue[wordnumber] = TWO_HANDED;
				wordtype[wordnumber--] = OBJECT;
				return (take(from));
			}
			wordvalue[wordnumber] = BROAD;
			wordtype[wordnumber--] = OBJECT;
			return (take(from));

		case BODY:
			if (testbit(from, MAID)) {
				wordvalue[wordnumber] = MAID;
				wordtype[wordnumber--] = OBJECT;
				return (take(from));
			} else if (testbit(from, DEADWOOD)) {
				wordvalue[wordnumber] = DEADWOOD;
				wordtype[wordnumber--] = OBJECT;
				return (take(from));
			} else if (testbit(from, DEADNATIVE)) {
				wordvalue[wordnumber] = DEADNATIVE;
				wordtype[wordnumber--] = OBJECT;
				return (take(from));
			} else {
				if (testbit(from, DEADGOD)) {
					wordvalue[wordnumber] = DEADGOD;
					wordtype[wordnumber--] = OBJECT;
					return (take(from));
				} else {
					wordvalue[wordnumber] = DEADTIME;
					wordtype[wordnumber--] = OBJECT;
					return (take(from));
				}
			}
			break;

		case AMULET:
			if (testbit(location[position].objects, AMULET)) {
				puts("The amulet is warm to the touch, and its beauty catches your breath.");
				puts("A mist falls over your eyes, but then it is gone.  Sounds seem clearer");
				puts("and sharper but far away as if in a dream.  The sound of purling water");
				puts("reaches you from afar.  The mist falls again, and your heart leaps in horror.");
				puts("The gold freezes your hands and fathomless darkness engulfs your soul.");
			}
			wordtype[wordnumber--] = OBJECT;
			return (take(from));

		case MEDALION:
			if (testbit(location[position].objects, MEDALION)) {
				puts("The medallion is warm, and it rekindles your spirit with the warmth of life.");
				puts("Your amulet begins to glow as the medallion is brought near to it, and together\nthey radiate.");
			}
			wordtype[wordnumber--] = OBJECT;
			return (take(from));

		case TALISMAN:
			if (testbit(location[position].objects, TALISMAN)) {
				puts("The talisman is cold to the touch, and it sends a chill down your spine.");
			}
			wordtype[wordnumber--] = OBJECT;
			return (take(from));

		case NORMGOD:
			if (testbit(location[position].objects, BATHGOD) && (testbit(wear, AMULET) || testbit(inven, AMULET))) {
				puts("She offers a delicate hand, and you help her out of the sparkling springs.");
				puts("Water droplets like liquid silver bedew her golden skin, but when they part");
				puts("from her, they fall as teardrops.  She wraps a single cloth around her and");
				puts("ties it at the waist.  Around her neck hangs a golden amulet.");
				puts("She bids you to follow her, and walks away.");
				pleasure++;
				followgod = ourtime;
				clearbit(location[position].objects, BATHGOD);
			} else
				if (!testbit(location[position].objects, BATHGOD))
					puts("You're in no position to take her.");
				else
					puts("She moves away from you.");
			break;

		default:
			puts("It doesn't seem to work.");
		}
	else
		puts("You've got to be kidding.");
	return (firstnumber);
}

int
throw(name)
	const char   *name;
{
	unsigned int     n;
	int     deposit = 0;
	int     first, value;

	first = wordnumber;
	if (drop(name) != -1) {
		switch (wordvalue[wordnumber]) {

		case AHEAD:
			deposit = ahead;
			break;

		case BACK:
			deposit = back;
			break;

		case LEFT:
			deposit = left;
			break;

		case RIGHT:
			deposit = right;
			break;

		case UP:
			deposit = location[position].up * (location[position].access || position == FINAL);
			break;

		case DOWN:
			deposit = location[position].down;
			break;
		}
		wordnumber = first + 1;
		while (wordnumber <= wordcount) {
			value = wordvalue[wordnumber];
			if (deposit && testbit(location[position].objects, value)) {
				clearbit(location[position].objects, value);
				if (value != GRENADE)
					setbit(location[deposit].objects, value);
				else {
					puts("A thundering explosion nearby sends up a cloud of smoke and shrapnel.");
					for (n = 0; n < NUMOFWORDS; n++)
						location[deposit].objects[n] = 0;
					setbit(location[deposit].objects, CHAR);
				}
				if (value == ROPE && position == FINAL)
					location[position].access = 1;
				switch (deposit) {
				case 189:
				case 231:
					puts("The stone door is unhinged.");
					location[189].north = 231;
					location[231].south = 189;
					break;
				case 30:
					puts("The wooden door is blown open.");
					location[30].west = 25;
					break;
				case 31:
					puts("The door is not damaged.");
				}
			} else
				if (value == GRENADE && testbit(location[position].objects, value)) {
					puts("You are blown into shreds when your grenade explodes.");
					die();
				}
			if (wordnumber < wordcount - 1 && wordvalue[++wordnumber] == AND)
				wordnumber++;
			else
				return (first);
		}
		return (first);
	}
	return (first);
}

int
drop(name)
	const char   *name;
{

	int     firstnumber, value;

	firstnumber = wordnumber;
	wordnumber++;
	while (wordnumber <= wordcount && (wordtype[wordnumber] == OBJECT || wordtype[wordnumber] == NOUNS)) {
		value = wordvalue[wordnumber];
		if (value == BODY) {	/* special case */
			wordtype[wordnumber] = OBJECT;
			if (testbit(inven, MAID) || testbit(location[position].objects, MAID))
				value = MAID;
			else if (testbit(inven, DEADWOOD) || testbit(location[position].objects, DEADWOOD))
				value = DEADWOOD;
			else if (testbit(inven, DEADGOD) || testbit(location[position].objects, DEADGOD))
				value = DEADGOD;
			else if (testbit(inven, DEADTIME) || testbit(location[position].objects, DEADTIME))
				value = DEADTIME;
			else if (testbit(inven, DEADNATIVE) || testbit(location[position].objects, DEADNATIVE))
				value = DEADNATIVE;
		}
		if (wordtype[wordnumber] == NOUNS && value == DOOR) {
			if (*name == 'K')
				puts("You hurt your foot.");
			else
				puts("You're not holding a door.");
		} else if (objsht[value] == NULL) {
			if (*name == 'K')
				puts("That's not for kicking!");
			else
				puts("You don't have that.");
		} else {
			printf("%s:\n", objsht[value]);
			if (testbit(inven, value)) {
				clearbit(inven, value);
				carrying -= objwt[value];
				encumber -= objcumber[value];
				if (value == BOMB) {
					puts("The bomb explodes.  A blinding white light and immense concussion obliterate us.");
					die();
				}
				if (value != AMULET && value != MEDALION && value != TALISMAN)
					setbit(location[position].objects, value);
				else
					tempwiz = 0;
				ourtime++;
				if (*name == 'K')
					puts("Drop kicked.");
				else
					printf("%s.\n", name);
			} else {
				if (*name != 'K') {
					printf("You aren't holding the %s.\n", objsht[value]);
					if (testbit(location[position].objects, value)) {
						if (*name == 'T')
							puts("Kicked instead.");
						else if (*name == 'G')
							puts("Given anyway.");
					}
				} else if (testbit(location[position].objects, value))
					puts("Kicked.");
				else if (testbit(wear, value))
					puts("Not while it's being worn.");
				else
					puts("Not found.");
			}
		}
		if (wordnumber < wordcount - 1 && wordvalue[++wordnumber] == AND)
			wordnumber++;
		else
			return (firstnumber);
	}
	puts("Do what?");
	return (-1);
}

int
takeoff()
{
	wordnumber = take(wear);
	return (drop("Dropped"));
}

int
puton()
{
	wordnumber = take(location[position].objects);
	return (wearit());
}

int
eat()
{
	int     firstnumber, value;

	firstnumber = wordnumber;
	wordnumber++;
	while (wordnumber <= wordcount) {
		value = wordvalue[wordnumber];
		if (wordtype[wordnumber] != OBJECT || objsht[value] == NULL)
			value = -2;
		switch (value) {

		case -2:
			puts("You can't eat that!");
			return (firstnumber);

		case -1:
			puts("Eat what?");
			return (firstnumber);

		default:
			printf("You can't eat %s%s!\n",
			    A_OR_AN_OR_BLANK(value), objsht[value]);
			return (firstnumber);

		case PAPAYAS:
		case PINEAPPLE:
		case KIWI:
		case COCONUTS:	/* eatable things */
		case MANGO:

			printf("%s:\n", objsht[value]);
			if (testbit(inven, value) &&
			    ourtime > ate - CYCLE &&
			    testbit(inven, KNIFE)) {
				clearbit(inven, value);
				carrying -= objwt[value];
				encumber -= objcumber[value];
				ate = max(ourtime, ate) + CYCLE / 3;
				snooze += CYCLE / 10;
				ourtime++;
				puts("Eaten.  You can explore a little longer now.");
			} else if (!testbit(inven, value))
				printf("You aren't holding the %s.\n", objsht[value]);
			else if (!testbit(inven, KNIFE))
				puts("You need a knife.");
			else
				puts("You're stuffed.");
			if (wordnumber < wordcount - 1 && wordvalue[++wordnumber] == AND)
				wordnumber++;
			else
				return (firstnumber);
		}		/* end switch */
	}			/* end while */
	return (firstnumber);
}
