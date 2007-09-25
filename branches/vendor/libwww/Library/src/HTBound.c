/*								     HTBound.c
**	MIME MULTIPART PARSER STREAM
**
**	(c) COPYRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
**	@(#) $Id: HTBound.c,v 2.14 1999/02/22 22:10:10 frystyk Exp $
**
**	This stream parses a MIME multipart stream and builds a set of new
**	streams via the stream stack each time we encounter a boundary start.
**	We get the boundary from the normal MIME parser via the Request object
**
** Authors
**	HF	Henrik Frystyk <frystyk@w3.org>
**
** History:
**	Nov 95	Written from scratch
**
*/

/* Library include files */
#include "wwwsys.h"
#include "WWWUtil.h"
#include "WWWCore.h"
#include "HTMerge.h"
#include "HTReqMan.h"
#include "HTBound.h"					 /* Implemented here */

#define PUTBLOCK(b, l)	(*me->target->isa->put_block)(me->target, b, l)
#define PUTDEBUG(b, l)	(*me->debug->isa->put_block)(me->debug, b, l)
#define FREE_TARGET	(*me->target->isa->_free)(me->target)

struct _HTStream {
    const HTStreamClass *	isa;
    HTStream *			target;
    HTStream *			orig_target;
    HTFormat			format;
    HTStream *			debug;		  /* For preamble and epilog */
    HTRequest *			request;
    BOOL			body;		  /* Body or preamble|epilog */
    HTEOLState			state;
    int				dash;			 /* Number of dashes */
    char *			boundary;
    char *			bpos;
};

/* ------------------------------------------------------------------------- */

PRIVATE int HTBoundary_put_block (HTStream * me, const char * b, int l)
{
    const char *start = b;
    const char *end = b;
    while (l-- > 0) {
	if (me->state == EOL_FCR) {
	    me->state = (*b == LF) ? EOL_FLF : EOL_BEGIN;
	} else if (me->state == EOL_FLF) {
	    if (me->dash == 2) {
		while (l>0 && *me->bpos && *me->bpos==*b) l--, me->bpos++, b++;
		if (!*me->bpos) {
		    HTTRACE(STREAM_TRACE, "Boundary.... `%s\' found\n" _ me->boundary);
		    me->bpos = me->boundary;
		    me->body = YES;
		    me->state = EOL_DOT;
		} else if (l>0) {
		    me->dash = 0;
		    me->bpos = me->boundary;
		    me->state = EOL_BEGIN;
		}
	    }
	    if (*b == '-') {
		me->dash++;
	    } else if (*b != CR && *b != LF) {
		me->dash = 0;
		me->state = EOL_BEGIN;
	    }
	} else if (me->state == EOL_SLF) {	    /* Look for closing '--' */
	    if (me->dash == 4) {
		if (end > start) {
		    int status = PUTBLOCK(start, end-start);
		    if (status != HT_OK) return status;
		}
		HTTRACE(STREAM_TRACE, "Boundary.... Ending\n");
		start = b;
		me->dash = 0;
		me->state = EOL_BEGIN;
	    }
	    if (*b == '-') {
		me->dash++;
	    } else if (*b != CR && *b != LF) {
		me->dash = 0;
		me->state = EOL_BEGIN;
	    }
	    me->body = NO;
	} else if (me->state == EOL_DOT) {
	    int status;
	    if (me->body) {
		if (me->target) FREE_TARGET;
		me->target = HTStreamStack(WWW_MIME,me->format,
					   HTMerge(me->orig_target, 2),
					   me->request, YES);
		if (end > start) {
		    if ((status = PUTBLOCK(start, end-start)) != HT_OK)
			return status;
		}
	    } else {
		if (me->debug)
		    if ((status = PUTDEBUG(start, end-start)) != HT_OK)
			return status;
	    }
	    start = b;
	    if (*b == '-') me->dash++;
	    me->state = EOL_SLF;
	} else if (*b == CR) {
	    me->state = EOL_FCR;
	    end = b;
	} else if (*b == LF) {
	    if (me->state != EOL_FCR) end = b;
	    me->state = EOL_FLF;
	}
	b++;
    }
    return (start<b && me->body) ? PUTBLOCK(start, b-start) : HT_OK;
}

PRIVATE int HTBoundary_put_string (HTStream * me, const char * s)
{
    return HTBoundary_put_block(me, s, (int) strlen(s));
}

PRIVATE int HTBoundary_put_character (HTStream * me, char c)
{
    return HTBoundary_put_block(me, &c, 1);
}

PRIVATE int HTBoundary_flush (HTStream * me)
{
    return (*me->target->isa->flush)(me->target);
}

PRIVATE int HTBoundary_free (HTStream * me)
{
    int status = HT_OK;
    if (me->target) {
	if ((status = (*me->target->isa->_free)(me->target)) == HT_WOULD_BLOCK)
	    return HT_WOULD_BLOCK;
    }
    HTTRACE(PROT_TRACE, "Boundary.... FREEING....\n");
    HT_FREE(me->boundary);
    HT_FREE(me);
    return status;
}

PRIVATE int HTBoundary_abort (HTStream * me, HTList * e)
{
    int status = HT_ERROR;
    if (me->target) status = (*me->target->isa->abort)(me->target, e);
    HTTRACE(PROT_TRACE, "Boundary.... ABORTING...\n");
    HT_FREE(me->boundary);
    HT_FREE(me);
    return status;
}

PRIVATE const HTStreamClass HTBoundaryClass =
{		
    "HTBoundary",
    HTBoundary_flush,
    HTBoundary_free,
    HTBoundary_abort,
    HTBoundary_put_character,
    HTBoundary_put_string,
    HTBoundary_put_block
};

PUBLIC HTStream * HTBoundary   (HTRequest *	request,
				void *		param,
				HTFormat	input_format,
				HTFormat	output_format,
				HTStream *	output_stream)
{
    HTResponse * response = HTRequest_response(request);
    HTParentAnchor * anchor = HTRequest_anchor(request);
    HTAssocList * type_param = response ?
	HTResponse_formatParam(response) :
	HTAnchor_formatParam(anchor);
    char * boundary = HTAssocList_findObject(type_param, "boundary");
    if (boundary) {
	HTStream * me;
	if ((me = (HTStream  *) HT_CALLOC(1, sizeof(HTStream))) == NULL)
	    HT_OUTOFMEM("HTBoundary");
	me->isa = &HTBoundaryClass;
	me->request = request;
	me->format = output_format;
	me->orig_target = output_stream;
	me->debug = HTRequest_debugStream(request);
	me->state = EOL_FLF;
	StrAllocCopy(me->boundary, boundary);		       /* Local copy */
	me->bpos = me->boundary;
	HTTRACE(STREAM_TRACE, "Boundary.... Stream created with boundary '%s\'\n" _ me->boundary);
	return me;
    } else {
	HTTRACE(STREAM_TRACE, "Boundary.... UNKNOWN boundary!\n");
	return HTErrorStream();
    }
}
