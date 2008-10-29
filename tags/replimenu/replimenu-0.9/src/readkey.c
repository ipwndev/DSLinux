/*
 * readkey() - identifies pressed keys and keyboard escape sequences.
 * It has been tested against the following keyboard maps; uk, us, se-latin1, fi-latin1, hu, de,
 * slovene, fr.
 * 
 * essential part of replimenu (http://replimenu.sourceforge.net).
 */

#include <unistd.h>

/*****************************************************************************/

#define key_UP			0x10000000
#define key_DOWN		0x10000001
#define key_LEFT		0x10000002
#define key_RIGHT		0x10000003
#define key_PGUP		0x10000004
#define key_PGDOWN		0x10000005
#define key_HOME		0x10000006
#define key_END			0x10000007
#define key_INSERT		0x10000008
#define key_DELETE		0x10000009

#define key_SHIFTUP		0x1000000A
#define key_SHIFTDOWN		0x1000000B
#define key_SHIFTLEFT		0x1000000C
#define key_SHIFTRIGHT		0x1000000D
#define key_CTRLUP		0x1000000E
#define key_CTRLDOWN		0x1000000F
#define key_CTRLLEFT		0x10000010
#define key_CTRLRIGHT		0x10000011

#define key_F1			0x10000012
#define key_F2			0x10000013
#define key_F3			0x10000014
#define key_F4			0x10000015
#define key_F5			0x10000016
#define key_F6			0x10000017
#define key_F7			0x10000018
#define key_F8			0x10000019
#define key_F9			0x1000001A
#define key_F10			0x1000001B
#define key_F11			0x1000001C
#define key_F12			0x1000001D

#define key_DBLESC		0x1000001E
#define key_NUMPAD5		0x1000001F

unsigned int readkey(void) {

	#define got_escapesequence	1
	#define got_csi			2
	#define got_O			4
	#define got_TWO			8	/* '2' */
	#define got_FIVE		16	/* '5' */
	#define got_THREE		32	/* '3' */
	#define got_SIX			64	/* '6' */
	#define got_secondCSI		128	/* '[' */
	#define got_ONE			256	/* '1' */
	#define got_CSI_ONE_FIVE	512	/* [15 */
	#define got_CSI_ONE_SEVEN	1024	/* [17 */
	#define got_CSI_ONE_EIGHT	2048	/* [18 */
	#define got_CSI_ONE_NINE	4096	/* [19 */
	#define got_CSI_TWO_ZERO	8192	/* [20 */
	#define got_CSI_TWO_ONE		16384	/* [21 */
	#define got_CSI_TWO_THREE	32768	/* [23 */
	#define got_CSI_TWO_FOUR	65536	/* [24 */
	#define got_FOUR		131072	/* '4' */

	unsigned int whatwegot;
	char onechar[1];

	whatwegot = 0;

	while (1) {
		read(STDIN_FILENO, onechar, 1);

		if (whatwegot & got_escapesequence) {
			if (whatwegot & got_csi) {
				if (whatwegot & got_secondCSI) {
					if (onechar[0] == 'A')
						return key_F1;
					else if (onechar[0] == 'B')
						return key_F2;
					else if (onechar[0] == 'C')
						return key_F3;
					else if (onechar[0] == 'D')
						return key_F4;
					else if (onechar[0] == 'E')
						return key_F5;
					else
						return onechar[0];
				}
				else if (whatwegot & got_ONE) {		/* '1' */
					if (whatwegot & got_CSI_ONE_FIVE) {
						if (onechar[0] == '~')
							return key_F5;
						else
							return onechar[0];
					}
					else if (whatwegot & got_CSI_ONE_SEVEN) {
						if (onechar[0] == '~')
							return key_F6;
						else
							return onechar[0];
					}
					else if (whatwegot & got_CSI_ONE_EIGHT) {
						if (onechar[0] == '~')
							return key_F7;
						else
							return onechar[0];
					}
					else if (whatwegot & got_CSI_ONE_NINE) {
						if (onechar[0] == '~')
							return key_F8;
						else
							return onechar[0];
					}
					else {
						if (onechar[0] == '~')
							return key_HOME;
						else if (onechar[0] == '5')
							whatwegot |= got_CSI_ONE_FIVE;
						else if (onechar[0] == '7')
							whatwegot |= got_CSI_ONE_SEVEN;
						else if (onechar[0] == '8')
							whatwegot |= got_CSI_ONE_EIGHT;
						else if (onechar[0] == '9')
							whatwegot |= got_CSI_ONE_NINE;
						else
							return onechar[0];
					}
				}
				else if (whatwegot & got_TWO) {		/* '2' */
					if (whatwegot & got_CSI_TWO_ZERO) {
						if (onechar[0] == '~')
							return key_F9;
						else
							return onechar[0];
					}
					else if (whatwegot & got_CSI_TWO_ONE) {
						if (onechar[0] == '~')
							return key_F10;
						else
							return onechar[0];
					}
					else if (whatwegot & got_CSI_TWO_THREE) {
						if (onechar[0] == '~')
							return key_F11;
						else
							return onechar[0];
					}
					else if (whatwegot & got_CSI_TWO_FOUR) {
						if (onechar[0] == '~')
							return key_F12;
						else
							return onechar[0];
					}
					else {
						if (onechar[0] == '~')
							return key_INSERT;
						else if (onechar[0] == 'A')
							return key_SHIFTUP;
						else if (onechar[0] == 'B')
							return key_SHIFTDOWN;
						else if (onechar[0] == 'D')
							return key_SHIFTLEFT;
						else if (onechar[0] == 'C')
							return key_SHIFTRIGHT;
						else if (onechar[0] == '0')
							whatwegot |= got_CSI_TWO_ZERO;
						else if (onechar[0] == '1')
							whatwegot |= got_CSI_TWO_ONE;
						else if (onechar[0] == '3')
							whatwegot |= got_CSI_TWO_THREE;
						else if (onechar[0] == '4')
							whatwegot |= got_CSI_TWO_FOUR;
						else
							return onechar[0];
					}
				}
				else if (whatwegot & got_THREE) {	/* '3' */
					if (onechar[0] == '~')
						return key_DELETE;
					else
						return onechar[0];
				}
				else if (whatwegot & got_FOUR) {	/* '4' */
					if (onechar[0] == '~')
						return key_END;
					else
						return onechar[0];
				}
				else if (whatwegot & got_FIVE) {	/* '5' */
					if (onechar[0] == '~')
						return key_PGUP;
					else if (onechar[0] == 'A')
						return key_CTRLUP;
					else if (onechar[0] == 'B')
						return key_CTRLDOWN;
					else if (onechar[0] == 'D')
						return key_CTRLLEFT;
					else if (onechar[0] == 'C')
						return key_CTRLRIGHT;
					else
						return onechar[0];
				}
				else if (whatwegot & got_SIX) {		/* '6' */
					if (onechar[0] == '~')
						return key_PGDOWN;
					else
						return onechar[0];
				}
				else {
					if (onechar[0] == 'A')
						return key_UP;
					else if (onechar[0] == 'B')
						return key_DOWN;
					else if (onechar[0] == 'D')
						return key_LEFT;
					else if (onechar[0] == 'C')
						return key_RIGHT;
					else if (onechar[0] == 'H')
						return key_HOME;
					else if (onechar[0] == 'F')
						return key_END;
					else if (onechar[0] == 'E')
						return key_NUMPAD5;
					else if (onechar[0] == 'G')
						return key_NUMPAD5;
					else if (onechar[0] == '1')
						whatwegot |= got_ONE;
					else if (onechar[0] == '2')
						whatwegot |= got_TWO;
					else if (onechar[0] == '3')
						whatwegot |= got_THREE;
					else if (onechar[0] == '4')
						whatwegot |= got_FOUR;
					else if (onechar[0] == '5')
						whatwegot |= got_FIVE;
					else if (onechar[0] == '6')
						whatwegot |= got_SIX;
					else if (onechar[0] == '[')
						whatwegot |= got_secondCSI;
					else
						return onechar[0];
				}
			}
			else if (whatwegot & got_O) {
				if (onechar[0] == 'P')
					return key_F1;
				else if (onechar[0] == 'Q')
					return key_F2;
				else if (onechar[0] == 'R')
					return key_F3;
				else if (onechar[0] == 'S')
					return key_F4;
				else
					return onechar[0];
			}
			else {
				if (onechar[0] == '[')
					whatwegot |= got_csi;
				else if (onechar[0] == 'O')
					whatwegot |= got_O;
				else if (onechar[0] == 033)
					return key_DBLESC;
				else {
					/* else we drop the entire sequence */
					return onechar[0];
				}
			}
		}

		if (!(whatwegot & got_escapesequence)) {
			if (onechar[0] == 033)
				whatwegot |= got_escapesequence;
			else
				return onechar[0];
		}
	}
	/* we should never end-up here */
	return 0;
}

/*****************************************************************************/
