/*

  					HTStyle: Style management for libwww


!
  Style Definition for Hypertext
!
*/

/*
**(c) COPYRIGHT MIT 1995.
**Please first read the full copyright statement in the file COPYRIGH.
*/

/*

Styles allow the translation between a logical property of a piece of text
and its physical representation.

A StyleSheet is a collection of styles, defining the translation necessary
to represent a document. It is a linked list of styles.
.
  Overriding this module
.

Why is the style structure declared in the HTStyle.h module, instead of having
the user browser define the structure, and the HTStyle routines just use
sizeof() for copying?

It's not obvious whether HTStyle.c should be common code. It's useful to
have common code for loading style sheets, especially if the movement toward
standard style sheets gets going.

If it IS common code, then both the hypertext object and HTStyle.c must know
the structure of a style, so HTStyle.h is a suitable place to put that. HTStyle.c
has to be compiled with a knowledge of the

It we take it out of the library, then of course HTStyle could be declared
as an undefined structure. The only references to it are in the
structure-flattening code HTML.c and HTPlain.c, which only use HTStypeNamed().

You can in any case override this function in your own code, which will prevent
the HTStyle from being loaded. You will be able to redefine your style structure
in this case without problems, as no other moule needs to know it.
*/

#ifndef GridStyle_H
#define GridStyle_H
#include "HText.h"
#include "HTFont.h"
#include "HTStyle.h"

typedef double HTCoord;

typedef int HTColor;		/* Sorry about the US spelling! */

typedef struct {
    short	kind;		/* only NX_LEFTTAB implemented*/
    HTCoord	position;	/* x coordinate for stop */
} HTTabStop;

#define STYLE_NAME_LENGTH	80	/* @@@@@@@@@@@ */

/*
.
  The Style Structure
.
*/

struct _HTStyle {

    /* Style management information */
    struct _HTStyle * next;                       /* Link for putting into stylesheet */
    char * name;                                                        /* Style name */
    char * SGMLTag;                                              /* Tag name to start */


    /*Character attributes(a la NXRun) */
    HTFont	font;                                                      /* Font id */
    HTCoord	fontSize;                        /* The size of font, not independent */
    HTColor	color;                                    /* text gray of current run */
    int	        superscript;                          /* superscript (-sub) in points */

    HTAnchor*anchor;/* Anchor id if any, else zero */

    /* Paragraph Attribtes(a la NXTextStyle) */
    HTCoord     indent1st;                            /* how far 1st line is indented */
    HTCoord     leftIndent;                        /* how far second line is indented */
    HTCoord     rightIndent;                            /* (Missing from NeXT version */
    short	alignment;                                      /* quad justification */
    HTCoord	lineHt;                                                /* line height */
    HTCoord	descentLine;                        /* descender bottom from baseline */
    HTTabStop	*tabs;                            /* array of tab stops, 0 terminated */

    BOOL        wordWrap;                         /* Yes means wrap at space not char */
    BOOL        freeFormat;                       /* Yes means \n is just white space */
    HTCoord     spaceBefore;                            /* Omissions from NXTextStyle */
    HTCoord     spaceAfter;
    int         paraFlags;                       /* Paragraph flags, bits as follows: */
};

#define PARA_KEEP          1               /* Do not break page within this paragraph */
#define	PARA_WITH_NEXT     2	            /* Do not break page after this paragraph */

#define HT_JUSTIFY         0 /* For alignment */
#define HT_LEFT            1
#define HT_RIGHT           2
#define HT_CENTER          3
  
#ifdef NOT_IN_GRIDSTYLE

/*
.
  Style Creation
.
(
  HtStyleModify
)

This routine is passed the style for a particular SGML nesting state, and
the element number of a new element whithin that state. The routine returns
a suitable style for text within the new element. It is passed a popinter
tothe nesting state so that it can link the style back to the nesting state
for later manipulation of the SGML nesting tree.
*/

extern HTStyle * HTStyleModify (HTStyle *   style,
                                HTNesting * nesting,
                                int         element_number);

/* Style functions: */
extern HTStyle * HTStyleNew (void);
extern HTStyle * HTStyleNewNamed (const char * name);
extern HTStyle * HTStyleFree (HTStyle * self);

#endif

/* Style Sheet */
struct _HTStyleSheet {
    char * name;
    HTStyle * styles;
};

#ifdef NOT_IN_GRIDSTYLE

/* Stylesheet functions: */
extern HTStyleSheet * HTStyleSheetNew (void);
extern HTStyleSheet * HTStyleSheetFree (HTStyleSheet * self);
extern HTStyle * HTStyleNamed (HTStyleSheet * self, const char * name);
extern HTStyle * HTStyleMatching (HTStyleSheet *self, HTStyle * style);

extern HTStyleSheet * HTStyleSheetAddStyle (HTStyleSheet * self, HTStyle * style);
extern HTStyleSheet * HTStyleSheetRemoveStyle (HTStyleSheet * self, HTStyle * style);

#define CLEAR_POINTER ((void *)-1)	/* Pointer value means "clear me" */

#endif /* NOT IN GRIDSTYLE */
#endif /* GRIDSTYLE_H */

/*

  

  @(#) $Id: GridStyle.html,v 1.10 1999/06/24 19:41:04 frystyk Exp $

*/
