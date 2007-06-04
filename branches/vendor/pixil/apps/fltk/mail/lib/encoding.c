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


static char alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" "0123456789+/";


static int
cvt_ascii(unsigned char alpha)
{
    if ((alpha >= 'A') && (alpha <= 'Z'))
	return (int) (alpha - 'A');
    else if ((alpha >= 'a') && (alpha <= 'z'))
	return 26 + (int) (alpha - 'a');
    else if ((alpha >= '0') && (alpha <= '9'))
	return 52 + (int) (alpha - '0');
    else if (alpha == '+')
	return 62;
    else if (alpha == '/')
	return 63;
    else if (alpha == '=')
	return -2;
    else
	return -1;
}

int
decode_base64(unsigned char *in, unsigned char *out, int insize)
{
    int index;

    unsigned char outval;

    unsigned char *outpointer = out;

    int shift = 0;
    unsigned long accum = 0;
    unsigned long value = 0;

    for (index = 0; index < insize; index++) {
	value = cvt_ascii(in[index]);

	if (value < 64) {
	    accum <<= 6;
	    shift += 6;
	    accum |= value;

	    if (shift >= 8) {
		shift -= 8;
		value = accum >> shift;
		outval = (unsigned char) value & 0xFFl;
		*outpointer++ = outval;
	    }
	}

    }

    return ((int) (outpointer - out));
}

int
encode_base64(unsigned char *in, unsigned char *out, int insize)
{
    int index;
    unsigned char *outpointer = out;

    unsigned char outval;
    int shift = 0;
    unsigned long accum = 0;
    unsigned long value = 0;

    for (index = 0; index < insize; index++) {
	value = (unsigned long) in[index];

	accum <<= 8;
	shift += 8;
	accum |= value;

	while (shift >= 6) {
	    shift -= 6;
	    value = (accum >> shift) & 0x3Fl;
	    outval = alphabet[value];

	    *outpointer++ = outval;
	}
    }

    return ((int) (outpointer - out));
}
