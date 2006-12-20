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

#include "fft.h"

//#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#ifndef PI
#ifdef M_PI
#define PI M_PI
#else
#define PI            3.14159265358979323846	/* pi */
#endif
#endif

GR_EVENT ev;
static int flag = -1;

/* ########### */
/* # Structs # */
/* ########### */

struct _struct_fft_state
{
    /* Temporary data stores to perform FFT in. */
    short int real[FFT_BUFFER_SIZE];
    short int imag[FFT_BUFFER_SIZE];
};

/* ############################# */
/* # Local function prototypes # */
/* ############################# */

static void fft_prepare(const sound_sample * input, short int *re,
			short int *im);
static void fft_calculate(short int *re, short int *im);
static void fft_output(const short int *re, const short int *im, int *output);
static int reverseBits(unsigned int initial);

/* #################### */
/* # Global variables # */
/* #################### */

/* Table to speed up bit reverse copy */
static unsigned int bitReverse[FFT_BUFFER_SIZE];

/* The next two tables could be made to use less space in memory, since they
 * overlap hugely, but hey. */
static short int sintable[FFT_BUFFER_SIZE / 2];
static short int costable[FFT_BUFFER_SIZE / 2];

/* ############################## */
/* # Externally called routines # */
/* ############################## */

/* --------- */
/* FFT stuff */
/* --------- */

/*
 * Initialisation routine - sets up tables and space to work in.
 * Returns a pointer to internal state, to be used when performing calls.
 * On error, returns NULL.
 * The pointer should be freed when it is finished with, by fft_close().
 */
fft_state *
fft_init(void)
{
    fft_state *state;
    unsigned int i;

    state = (fft_state *) malloc(sizeof(fft_state));
    if (!state)
	return NULL;

    for (i = 0; i < FFT_BUFFER_SIZE; i++) {
	bitReverse[i] = reverseBits(i);
    }
    for (i = 0; i < FFT_BUFFER_SIZE / 2; i++) {
	float j = 2 * PI * i / FFT_BUFFER_SIZE;
	float test;
	costable[i] = (short int) (cos(j) * (1 << 15));
	test = cos(j);
	sintable[i] = (short int) (sin(j) * (1 << 15));
    }

    return state;
}

/*
 * Do all the steps of the FFT, taking as input sound data (as described in
 * sound.h) and returning the intensities of each frequency as floats in the
 * range 0 to ((FFT_BUFFER_SIZE / 2) * 32768) ^ 2
 *
 * FIXME - the above range assumes no frequencies present have an amplitude
 * larger than that of the sample variation.  But this is false: we could have
 * a wave such that its maximums are always between samples, and it's just
 * inside the representable range at the places samples get taken.
 * Question: what _is_ the maximum value possible.  Twice that value?  Root
 * two times that value?  Hmmm.  Think it depends on the frequency, too.
 *
 * The input array is assumed to have FFT_BUFFER_SIZE elements,
 * and the output array is assumed to have (FFT_BUFFER_SIZE / 2 + 1) elements.
 * state is a (non-NULL) pointer returned by fft_init.
 */
void
fft_perform(const sound_sample * input, int *output, fft_state * state,
	    int dataFrameSkip)
{

    // initalize flag
    if (flag == -1)
	flag = dataFrameSkip;


    if (flag == dataFrameSkip) {

	flag = 0;

	/* Convert data from sound format to be ready for FFT */
	fft_prepare(input, state->real, state->imag);

	/* Do the actual FFT */
	fft_calculate(state->real, state->imag);

	/* Convert the FFT output into intensities */
	fft_output(state->real, state->imag, output);

    } else {

	flag++;

	return;

    }
}

/*
 * Free the state.
 */
void
fft_close(fft_state * state)
{
    if (state)
	free(state);
}

/* ########################### */
/* # Locally called routines # */
/* ########################### */

/*
 * Prepare data to perform an FFT on
 */
static void
fft_prepare(const sound_sample * input, short int *re, short int *im)
{
    unsigned int i;
    short int *realptr = re;
    short int *imagptr = im;

    /* Get input, in reverse bit order */
    for (i = 0; i < FFT_BUFFER_SIZE; i++) {
	*realptr++ = input[bitReverse[i]];
	*imagptr++ = 0;
    }
}

/*
 * Take result of an FFT and calculate the intensities of each frequency
 * Note: only produces half as many data points as the input had.
 * This is roughly a consequence of the Nyquist sampling theorm thingy.
 * (FIXME - make this comment better, and helpful.)
 * 
 * The two divisions by 4 are also a consequence of this: the contributions
 * returned for each frequency are split into two parts, one at i in the
 * table, and the other at FFT_BUFFER_SIZE - i, except for i = 0 and
 * FFT_BUFFER_SIZE which would otherwise get float (and then 4* when squared)
 * the contributions.
 */
static void
fft_output(const short int *re, const short int *im, int *output)
{
    int *outputptr = output;
    const short int *realptr = re;
    const short int *imagptr = im;
    //int *endptr    = output + FFT_BUFFER_SIZE / 2;
    int *endptr = output + 256;

#ifdef DEBUG
    unsigned int i, j;
#endif

    while (outputptr <= endptr) {
	*outputptr = (*realptr * *realptr) + (*imagptr * *imagptr);
	outputptr++;
	realptr++;
	imagptr++;
    }
    /* Do divisions to keep the constant and highest frequency terms in scale
     * with the other terms. */
    *output /= 4;
    *endptr /= 4;

#ifdef DEBUG
    printf("Recalculated input:\n");
    for (i = 0; i < FFT_BUFFER_SIZE; i++) {
	float val_real = 0;
	float val_imag = 0;
	for (j = 0; j < FFT_BUFFER_SIZE; j++) {
	    float fact_real = cos(-2 * j * i * PI / FFT_BUFFER_SIZE);
	    float fact_imag = sin(-2 * j * i * PI / FFT_BUFFER_SIZE);
	    val_real += fact_real * re[j] - fact_imag * im[j];
	    val_imag += fact_real * im[j] + fact_imag * re[j];
	}
	printf("%5d = %8f + i * %8f\n", i,
	       val_real / FFT_BUFFER_SIZE, val_imag / FFT_BUFFER_SIZE);
    }
    printf("\n");
#endif
}

/*
 * Actually perform the FFT
 */
static void
fft_calculate(short int *re, short int *im)
{
    unsigned int i, j, k;
    unsigned int exchanges;
    int fact_real, fact_imag;
    int tmp_real, tmp_imag;
    int tmp1, tmp2;
    unsigned int factfact;

    /* Set up some variables to reduce calculation in the loops */
    exchanges = 1;
    //factfact = FFT_BUFFER_SIZE / 2;
    factfact = 256;

    /* Loop through the divide and conquer steps */
    for (i = FFT_BUFFER_SIZE_LOG; i != 0; i--) {
	/* In this step, we have 2 ^ (i - 1) exchange groups, each with
	 * 2 ^ (FFT_BUFFER_SIZE_LOG - i) exchanges
	 */

	GrPeekEvent(&ev);
	if (ev.type == GR_EVENT_TYPE_BUTTON_DOWN ||
	    ev.type == GR_EVENT_TYPE_BUTTON_DOWN ||
	    ev.type == GR_EVENT_TYPE_UPDATE) {
	    fl_handle(ev);
	}

	/* Loop through the exchanges in a group */
	for (j = 0; j != exchanges; j++) {
	    /* Work out factor for this exchange
	     * factor ^ (exchanges) = -1
	     * So, real = cos(j * PI / exchanges),
	     *     imag = sin(j * PI / exchanges)
	     */
	    fact_real = costable[j * factfact];
	    fact_imag = sintable[j * factfact];

	    /* Loop through all the exchange groups */
	    for (k = j; k < FFT_BUFFER_SIZE; k += exchanges << 1) {
		int k1 = k + exchanges;


		/* newval[k]  := val[k] + factor * val[k1]
		 * newval[k1] := val[k] - factor * val[k1]
		 **/

		/* FIXME - potential scope for more optimization here? */
		tmp1 = fact_real;
		tmp1 *= re[k1];

		tmp2 = fact_imag;
		tmp2 *= im[k1];

		tmp_real = tmp1 - tmp2;

		tmp1 = fact_real;
		tmp1 *= im[k1];

		tmp2 = fact_imag;
		tmp2 *= re[k1];

		tmp_imag = tmp1 + tmp2;

		tmp_real >>= 15;
		tmp_imag >>= 15;

		re[k1] = re[k] - tmp_real;

		im[k1] = im[k] - tmp_imag;

		re[k] += tmp_real;

		im[k] += tmp_imag;
	    }
	}
	exchanges <<= 1;
	factfact >>= 1;
    }
}

static int
reverseBits(unsigned int initial)
{
    unsigned int reversed = 0, loop;
    for (loop = 0; loop < FFT_BUFFER_SIZE_LOG; loop++) {
	reversed <<= 1;
	reversed += (initial & 1);
	initial >>= 1;
    }
    return reversed;
}
