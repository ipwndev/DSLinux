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

////////////////////////////////////////////////////////////////////////////////
// MP3 Utility Methods
////////////////////////////////////////////////////////////////////////////////

#include "mp3utils.h"
#define HAVE_PTHREAD_H
#include "mpegsound.h"

void
Mp3Utils::strman(char *str, int max)
{
    int i;

    str[max] = 0;

    for (i = max - 1; i >= 0; i--)
	if (((unsigned char) str[i]) < 26 || str[i] == ' ')
	    str[i] = 0;
	else
	    break;
}

//--------------------------------------------------------------------------------

void
Mp3Utils::stripfilename(char *dtr, char *str, int max)
{
    char *ss;
    int p = 0, s = 0;

    for (; str[p]; p++)
	if (str[p] == '/') {
	    p++;
	    s = p;
	}

    ss = str + s;
    for (p = 0; p < max && ss[p]; p++)
	dtr[p] = ss[p];
    dtr[p] = 0;
}

//--------------------------------------------------------------------------------

// Parse out MP3 title, etc.
void
Mp3Utils::parseID3(char *path, char *title)
{

    Soundinputstream *fp;

    int err;

    fp = Soundinputstream::hopen(path, &err);

    int tryflag = 0;

    fp->setposition(fp->getsize() - 128);

    for (;;) {
	if (fp->getbytedirect() == 0x54) {
	    if (fp->getbytedirect() == 0x41) {
		if (fp->getbytedirect() == 0x47) {
		    fp->_readbuffer(title, 30);
		    strman(title, 30);
		    //fp->_readbuffer(data->artist  ,30);strman(data->artist,  30);
		    //fp->_readbuffer(data->album   ,30);strman(data->album,   30);
		    //fp->_readbuffer(data->year    , 7);strman(data->year,     7);
		    //fp->_readbuffer(data->comment ,30);strman(data->comment, 30);

		    break;
		}
	    }
	}

	tryflag++;
	if (tryflag == 1) {
	    fp->setposition(fp->getsize() - 125);	// for mpd 0.1, Sorry..
	} else {
	    stripfilename(title, path, 1024);

	    break;
	}
    }
    fp->setposition(0);

}
