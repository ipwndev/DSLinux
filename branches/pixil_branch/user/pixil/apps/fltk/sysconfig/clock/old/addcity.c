/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */




/* Feature test switches */


/* System header files */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Local header files */
#include "hashdb_api.h"


/* Typedef, macros, enum/struct/unions */
typedef struct
{
    char iso_id[2 + 1],		/* ISO 3166 country id */
      long_reg[48];		/* Name */
}
iso_name_t;

/* Global scope variables */
unsigned short g_Debug = 0;


/* Local scope variables */
static char *dbname = "./cityzones.db", *txtfile = "./citylist.out";
static iso_name_t iso_head[240];

static int iso_cnt = 0;		/* Number of elements of iso_head in use */

/* Function prototypes */
void build_iso_list(void);
int iso_find(char *key);
int iso_ins(iso_name_t * isorec);
void get_records(hashdb_t * dbd, char *txtname);


/* main */
int
main(int argc, char **argv)
{
    int gopt,			/* getopt() value */
      initflg = 1,		/* Initialize the database */
      rc;			/* Result code */
    hashdb_t *hdbd;		/* Hashed db descriptor */

    /* Parse the commandline arguments */
    while ((gopt = getopt(argc, argv, "cf:t:v")) != -1)
	switch (gopt) {
	case 'c':
	    initflg = 1;
	    break;
	case 'f':
	    if (optarg && !access((char *) optarg, R_OK | W_OK))
		dbname = optarg;
	    break;
	case 't':
	    if (optarg && !(access((char *) optarg, R_OK)))
		txtfile = optarg;
	    break;
	case 'v':
	    g_Debug = 1;
	    break;
	}			/* end of while-switch */

    /* Initialize the database (if desired) */
    if (initflg && h_init_new_db(dbname, HASH_BLK_SIZE, (90 + 90 + 1)) != 0) {
	printf("Error in h_init_new_db() for \"%s\", error=%s\n",
	       dbname, strerror(errno));
	exit(EXIT_FAILURE);
    }

    /* end of if */
    /* Open the database */
    if ((hdbd = h_opendb(dbname, O_RDWR)) == NULL) {
	printf("Error in h_opendb(\"%s\"), error=%s.\n", dbname,
	       strerror(errno));
	exit(EXIT_FAILURE);
    }
    /* end of if */
    build_iso_list();
    get_records(hdbd, txtfile);

    h_closedb(hdbd);

    exit(EXIT_SUCCESS);
}				/* end of main() */

/*******************************************************************************\
**
**	Function:	void build_iso_list()
**	Desc:		Builds a sorted list of iso 3166 records (sorted by Country name,
**				not 2 char id)
**	Accepts:	Nothing (void)
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
build_iso_list(void)
{
    char *cp,			/* character pointer */
      isobuf[255];		/* Iso buffer */
    FILE *fiso;			/* File stream */
    iso_name_t tmpiso;		/* Temp iso */

    if ((fiso = fopen("/usr/share/zoneinfo/iso3166.tab", "r")) == NULL)
	return;

    while (fgets(isobuf, sizeof(isobuf), fiso) != NULL) {
	int len = strlen(isobuf);

	if (isobuf[0] == '#')
	    continue;
	if (isobuf[len - 1] == '\n')
	    isobuf[--len] = '\0';

	if ((cp = strchr(isobuf, 0x09)) == NULL)
	    continue;

	*cp = '\0';
	memset(&tmpiso, 0, sizeof(tmpiso));
	memcpy(tmpiso.iso_id, isobuf, sizeof(tmpiso.iso_id) - 1);
	memcpy(tmpiso.long_reg, cp + 1, sizeof(tmpiso.long_reg) - 1);

	iso_ins(&tmpiso);
    }				/* end of while */

    fclose(fiso);

}				/* end of build_iso_list() */

/*******************************************************************************\
**
**	Function:	int iso_find()
**	Desc:		Finds the given record in the array and returns its index, if
**				the record isn't found, it will return the index where it needs
**				to be inserted based on the following algorithm:
**					w = (w + 1) * -1
**	Accepts:	char *key = Key to look for
**	Returns:	int; >= 0 on successful find, < 0 on unable to find
**	
\*******************************************************************************/
int
iso_find(char *key)
{
    int cmprc = 0,		/* RC of the cmp */
      found = 0,		/* Found flag */
      hi,			/* High index in bsearch */
      lo,			/* Lo index in bsearch */
      mid = 0;			/* Middle idx in bsearch */

    hi = iso_cnt - 1;
    lo = 0;
    while (lo <= hi) {
	mid = (hi + lo) / 2;

	if ((cmprc = strcmp(key, iso_head[mid].long_reg)) == 0) {
	    found = mid;
	    break;
	} /* end of if */
	else if (cmprc < 0)
	    hi = mid - 1;
	else
	    lo = mid + 1;
    }				/* end of while */

    if (!found) {
	if (cmprc > 0)
	    mid++;
	found = (mid + 1) * -1;
    }
    /* end of if */
    return (found);
}				/* end of iso_find() */

/*******************************************************************************\
**
**	Function:	int iso_ins()
**	Desc:		Inserts the record into the list (sorted array)
**	Accepts:	iso_name_t *isorec = Key to look up
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
int
iso_ins(iso_name_t * isorec)
{
    int idx,			/* Index to insert into */
      rc = -1;			/* Return code, assume error */

//      memcpy(&iso_head[iso_cnt++], isorec, sizeof(iso_name_t));

    if ((idx = iso_find(isorec->long_reg)) < 0) {
	/* Only add those that aren't in the list */
	idx = (idx * -1) - 1;
	if (idx >= iso_cnt) {
	    memcpy(&iso_head[idx], isorec, sizeof(iso_name_t));
	} /* end of if */
	else {
	    memmove(&iso_head[idx + 1], &iso_head[idx],
		    (sizeof(iso_name_t) * (iso_cnt - idx)));
	    memcpy(&iso_head[idx], isorec, sizeof(iso_name_t));
	}			/* end of else */
	iso_cnt++;
	rc = 0;
    }
    /* end of if */
    return (rc);
}				/* end of iso_ins() */

/*******************************************************************************\
**
**	Function:	void get_records()
**	Desc:		Reads the data from the citylist.out (text file) and formats/encodes
**				them into a h_data_rec structure and adds it to the database
**	Accepts:	hashdb_t *dbd = Database descriptor
**				char *txtname = Path name of the textfile
**	Returns:	Nothing (void)
**
\*******************************************************************************/
void
get_records(hashdb_t * dbd, char *txtname)
{
    char *cp,			/* generic ptr */
     *txtflds[5],		/* Text fields */
      txtbuf[512];		/* Text file buffer */
    int cnt = 0, hashidx,	/* hash index */
      isoidx,			/* iso index */
      txtidx,			/* Index into txtflds */
      txtlen;			/* Length of line */
    FILE *ftxt;			/* text FILE handler */
    h_data_rec record;		/* Record */

    if ((ftxt = fopen(txtname, "r")) == NULL) {
	printf("Error opening textfile \"%s\", error=%s\n", txtname,
	       strerror(errno));
	return;
    }
    /* end of if */
    while (fgets(txtbuf, sizeof(txtbuf), ftxt) != NULL) {
	txtlen = strlen(txtbuf);
	if (txtbuf[txtlen - 1] == '\n')
	    txtbuf[--txtlen] = '\0';

	/* Parse the record */
	memset(&record, 0, HDB_DTAREC_SZ);
	/* txtflds[0] = city name, [1] = region [2] = lat [3] = lon [4] = timezone */
	for (cp = txtflds[0] = txtbuf, txtidx = 0; *cp; cp++) {
	    /* Once the 5th field has been found, stop! */
	    if (txtidx == 4)
		break;
	    if (*cp == 0x09) {
		*cp = '\0';
		txtflds[++txtidx] = (cp + 1);
	    }			/* end of if */
	}			/* end of for */

	/* Build the h_data_rec record */

	/* Make the ',' a NUL terminator in cityname (txtflds[0]) */
	for (cp = txtflds[0]; *cp; cp++) {
	    if (*cp == ',') {
		*cp = '\0';
		break;
	    }			/* end of if */
	}			/* end of for */
	memcpy(record.city_name, txtflds[0], sizeof(record.city_name) - 1);

	/* Get the correct ISO3166 2 char id */
	if (!memcmp(txtflds[1], "United States", 13)) {
	    cp = txtflds[0] + strlen(txtflds[0]) + 1;
	    memcpy(record.reg, cp, 2);
	} /* end of if */
	else {
	    if ((isoidx = iso_find(txtflds[1])) >= 0)
		memcpy(record.reg, iso_head[isoidx].iso_id, 2);
	    else
		memcpy(record.reg, txtflds[1], sizeof(record.reg) - 1);
	}			/* end of else */

	/* Set the latitude and longitude values */
	record.lat = (short) atoi(txtflds[2]);
	record.lon = (short) atoi(txtflds[3]);

	/* Time zone is stored as a 2 character directory + a 5 character file */
	if (txtflds[4][0] == '/')
	    memmove(&txtflds[4][0], &txtflds[4][1], strlen(txtflds[4]) - 1);
	if ((cp = strrchr(txtflds[4], '/')) == NULL) {
	    continue;
	}			/* end of if */
	sprintf(record.zinfo, "%2.2s%5.5s", txtflds[4], (cp + 1));

	/* Hash on the latitude */
	hashidx = record.lat / 60;
	hashidx += 90;

	if (h_addrec(dbd, hashidx, &record) == -1) {
	    printf("Error in h_addrec(%s,%s,%s), error=%s\n",
		   record.city_name, record.reg, record.zinfo,
		   strerror(errno));
	    continue;
	}			/* end of if */
	cnt++;
    }				/* end of while */

    fclose(ftxt);
    return;
}				/* end of get_records() */
