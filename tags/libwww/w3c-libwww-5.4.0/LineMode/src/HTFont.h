/*

					The portable font concept



!The portable font concept!

*/

/*
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This is the Line mode browser version

*/

#ifndef HTFONT_H
#define HTFONT_H

typedef long int HTLMFont;	/* For now */
typedef long int HTFont;	/* For now as well */

#define HT_NON_BREAK_SPACE ((char)1)	/* For now */

#define HT_FONT		0
#define HT_CAPITALS	1
#define HT_BOLD		2
#define HT_UNDERLINE	4
#define HT_INVERSE	8
#define HT_DOUBLE	0x10

#define HT_BLACK	0
#define HT_WHITE	1

#endif /* HTFONT_H */

/*

End of HTFont

*/
