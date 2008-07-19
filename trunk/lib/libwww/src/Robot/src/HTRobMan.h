/*

  					Webbot - the W3C Mini Robot


!
  Webbot - the W3C Mini Robot
!
*/

/*
**	(c) COPRIGHT MIT 1995.
**	Please first read the full copyright statement in the file COPYRIGH.
*/

/*

This program illustrates how to travers links using the Anchor object
*/

#ifndef HTROBMAN_H
#define HTROBMAN_H

#include "WWWLib.h"			      /* Global Library Include file */
#include "WWWApp.h"				        /* Application stuff */
#include "WWWTrans.h"
#include "WWWInit.h"
#include "WWWSQL.h"

#include "HText.h"

#include "HTRobot.h"			     		 /* Implemented here */

#ifndef W3C_VERSION
#define W3C_VERSION 		"Unspecified"
#endif

#define APP_NAME		"W3CRobot"
#define APP_VERSION		W3C_VERSION
#define COMMAND_LINE		"http://www.w3.org/Robot/User/CommandLine"
#define ROBOTS_TXT              "/robots.txt"

#define DEFAULT_OUTPUT_FILE	"robot.out"
#define DEFAULT_RULE_FILE	"robot.conf"
#define DEFAULT_LOG_FILE       	"log-clf.txt"
#define DEFAULT_HIT_FILE       	"log-hit.txt"
#define DEFAULT_REL_FILE      	"log-rel.txt"
#define DEFAULT_LM_FILE       	"log-lastmodified.txt"
#define DEFAULT_TITLE_FILE     	"log-title.txt"
#define DEFAULT_REFERER_FILE   	"log-referer.txt"
#define DEFAULT_REJECT_FILE   	"log-reject.txt"
#define DEFAULT_NOTFOUND_FILE  	"log-notfound.txt"
#define DEFAULT_CONNEG_FILE  	"log-conneg.txt"
#define DEFAULT_NOALTTAG_FILE  	"log-alt.txt"
#define DEFAULT_FORMAT_FILE  	"log-format.txt"
#define DEFAULT_CHARSET_FILE  	"log-charset.txt"
#define DEFAULT_MEMLOG		"robot.mem"
#define DEFAULT_PREFIX		""
#define DEFAULT_IMG_PREFIX	""
#define DEFAULT_DEPTH		0
#define DEFAULT_DELAY		50			/* Write delay in ms */

#define DEFAULT_CACHE_SIZE	20			/* Default cache size */

#define DEFAULT_SQL_SERVER	"localhost"
#define DEFAULT_SQL_DB		"webbot"
#define DEFAULT_SQL_USER	"webbot"
#define DEFAULT_SQL_PW		""

#if 0
#define HT_MEMLOG		/* Is expensive in performance! */
#endif

#define MILLIES			1000
#define DEFAULT_TIMEOUT		20		          /* timeout in secs */

typedef enum _MRFlags {
    MR_IMG		= 0x1,
    MR_LINK		= 0x2,
    MR_PREEMPTIVE	= 0x4,
    MR_TIME		= 0x8,
    MR_SAVE	  	= 0x10,
    MR_QUIET	  	= 0x20,
    MR_REAL_QUIET  	= 0x40,
    MR_VALIDATE		= 0x80,
    MR_END_VALIDATE	= 0x100,
    MR_KEEP_META	= 0x200,
    MR_LOGGING		= 0x400,
    MR_DISTRIBUTIONS	= 0x800,
    MR_NOROBOTSTXT	= 0x1000,
    MR_NOMETATAGS	= 0x2000,
    MR_BFS      	= 0x4000,
    MR_REDIR            = 0x8000
} MRFlags;

typedef struct _Robot {
    int			depth;			     /* How deep is our tree */
    int                 ndoc;
    int                *cdepth;                /* Number of nodes per level */
    int			cnt;				/* Count of requests */
    int                 cindex;         /* Number assigned to each document */

    HTList *		hyperdoc;	     /* List of our HyperDoc Objects */
    HTList *		htext;			/* List of our HText Objects */
    HTList *		fingers;

    HTList *            queue;                  /* Queue */
    int                 cq;

    int 		timer;
    int 		waits;

    char *		cwd;			/* Current dir URL */
    char *		rules;
    char *		prefix;
    char *		img_prefix;

    char *		logfile;		/* clf log */
    HTLog *             log;
    char *		reffile;		/* referer log */
    HTLog *             ref;
    char *		rejectfile;		/* unchecked links */
    HTLog *	        reject;
    char *		notfoundfile;		/* links that returned 404 */
    HTLog *	        notfound;
    char *		connegfile;		/* links that were conneg'ed */
    HTLog *	        conneg;
    char *		noalttagfile;		/* images without alt tags*/
    HTLog *	        noalttag;


    char *		hitfile;		/* links sorted after hit counts */
    char *		relfile;		/* link sorted after relationships */
    HTLinkType		relation;		/* Specific relation to look for */
    char *		titlefile;		/* links with titles */
    char *		mtfile;			/* media types encountered */
    char *		charsetfile;		/* charsets encountered */
    char *		lmfile;			/* sortef after last modified dates */

    char *		outputfile;		
    FILE *	        output;

    char *              furl;                              /* First url */

    MRFlags		flags;

    int                 redir_code;     /* 0 means all, otherwise 301, 302, 305... */ 

    long		get_bytes;	/* Total number of bytes processed using GET*/
    long                get_docs;     	/* Total number of documents using GET */

    long		head_bytes;	/* bytes processed bytes processed using HEAD */
    long                head_docs;   	/* Total number of documents using HEAD*/

    long		other_docs;

    ms_t		time;		/* Time of run */

#ifdef HT_POSIX_REGEX
    regex_t *		include;
    regex_t *		exclude;
    regex_t *		check;
    regex_t *		exc_robot;     /* Robots.txt exclusion */
#endif

#ifdef HT_MYSQL
    HTSQLLog *		sqllog;
    char *		sqlserver;
    char *		sqldb;
    char *		sqluser;
    char *		sqlpw;
    char *		sqlrelative;
    BOOL		sqlexternals;
    int			sqlflags;
#endif

} Robot;

typedef struct _Finger {
    Robot * robot;
    HTRequest * request;
    HTParentAnchor * dest;
} Finger;

/*
**  The HyperDoc object is bound to the anchor and contains information about
**  where we are in the search for recursive searches
*/

#define NO_CODE -1
#define REDIR_CODE -2

typedef struct _HyperDoc {
    HTParentAnchor * 	anchor;
    int			depth;
    int                 hits;
    int                 code;
    int                 index;
    char *              title;
    HTMethod            method;
} HyperDoc;

/*
** This is the HText object that is created every time we start parsing an 
** HTML object
*/
struct _HText {
    HTRequest *		request;
    BOOL		follow;
};

/*
**  A structure for calculating metadata distributions
*/
typedef struct _MetaDist {
    HTAtom *		name;
    int			hits;
} MetaDist;
 
#ifdef HT_POSIX_REGEX
PUBLIC regex_t * get_regtype (Robot * mr, const char * regex_str, int cflags);
#endif

PUBLIC int OutputData(const char  * fmt, ...);
PUBLIC HyperDoc * HyperDoc_new (Robot * mr,HTParentAnchor * anchor, int depth);
PUBLIC BOOL HyperDoc_delete (HyperDoc * hd);
PUBLIC Robot * Robot_new (void);
PUBLIC Finger * Finger_new (Robot * robot, HTParentAnchor * dest, HTMethod method);
PUBLIC BOOL Robot_registerHTMLParser (void);
PUBLIC void Cleanup (Robot * me, int status);
PUBLIC void VersionInfo (void);

PUBLIC int terminate_handler (HTRequest * request, HTResponse * response,
			       void * param, int status) ;

PUBLIC int bfs_terminate_handler (HTRequest * request, HTResponse * response,
			          void * param, int status) ;

PUBLIC int redirection_handler (HTRequest * request, HTResponse * response,
			        void * param, int status) ;

PUBLIC void Serving_queue(Robot *mr);

PUBLIC char *get_robots_txt(char *uri);

#endif

/*

  

  @(#) $Id: HTRobMan.html,v 1.9 1999/03/14 02:21:09 frystyk Exp $

*/
