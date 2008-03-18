/*   aRts sound server support for EsounD
 *   22.05.2002 Igor Mokrushin (igor@avtomir.ru):
 *
 *   Prepared according to advices and suggestions by Vlad Harchev <hvv@hippo.ru>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <dlfcn.h>

enum arts_parameter_t_enum { ARTS_P_BUFFER_TIME = 2 };

typedef void *arts_stream_t;
typedef enum arts_parameter_t_enum arts_parameter_t;

static void *handle;
				    
static int (*arts_init)(void);
static void (*arts_free)(void);
static int (*arts_read)(arts_stream_t sst, void *bbuffer, int ccount);
static int (*arts_write)(arts_stream_t sst, const void *bbuffer, int ccount);
static void (*arts_close_stream)(arts_stream_t sst);
static int (*arts_stream_set)(arts_stream_t sst, arts_parameter_t pparam, int vvalue);

static arts_stream_t (*arts_play_stream)(int rrate, int bbits, int cch, const char *nn);
static arts_stream_t (*arts_record_stream)(int rrate, int bbits, int cch, const char *nn);

static arts_stream_t stream;
static int err_arts = -1;
static int arts_exit = 1;

static int arts()
{

    handle = dlopen ("libartsc.so", RTLD_LAZY);

    if (!handle) {
	arts_exit = 0;
	err_arts = -1;
	return err_arts;
    }
    
    arts_init = dlsym(handle, "arts_init");
    arts_free = dlsym(handle, "arts_free");
    arts_read = dlsym(handle, "arts_read");
    arts_write =  dlsym(handle, "arts_write");
    arts_play_stream = dlsym(handle, "arts_play_stream");
    arts_record_stream = dlsym(handle, "arts_record_stream");
    arts_close_stream = dlsym(handle, "arts_close_stream");
    arts_stream_set = dlsym(handle, "arts_stream_set");

    err_arts = arts_init();

    if (err_arts < 0) {
	arts_exit = 0;
        dlclose(handle);
        return err_arts;
    }
    
    return err_arts;
}

const char *arts_esd_audio_devices()
{
    return "sound server aRts";
}

int arts_esd_audio_open()
{
    int channels;
    int bits;

    channels = ( ( ( esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO )
        	? /* stereo */ 2 : /* mono */ 1 );
    bits = ( (esd_audio_format & ESD_MASK_BITS) == ESD_BITS16 )
        ? /* 16 bit */ 16 : /* 8 bit */ 8;
    
    
    if ((esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD) {
	stream = arts_record_stream(esd_audio_rate, bits, channels, "esd");
    } else {
	stream = arts_play_stream(esd_audio_rate, bits, channels, "esd");
    }
    /* fix latency */
    arts_stream_set(stream, ARTS_P_BUFFER_TIME, 0);

    return 0;
}

void arts_esd_audio_close()
{
    arts_close_stream(stream);
    arts_exit = 0;
    arts_free();
    dlclose(handle);
    return;
}
	
int arts_esd_audio_write( void *buffer, int buf_size )
{
    return arts_write(stream, buffer, buf_size);
}
    
int arts_esd_audio_read( void *buffer, int buf_size )
{
    return arts_read(stream, buffer, buf_size);
}

void arts_esd_audio_pause()
{
    return;
}

void arts_esd_audio_flush()
{
    return;
}

static void impl_exit()
{
    if ((arts_exit > 0) && (handle)) {
	if (err_arts >= 0) 
	    arts_free();
	dlclose(handle);
    }
}

void artschk() 
{
    if(arts() >= 0) {
	impl_esd_audio_devices = arts_esd_audio_devices;
	impl_esd_audio_open = arts_esd_audio_open;
	impl_esd_audio_close = arts_esd_audio_close;
	impl_esd_audio_pause = arts_esd_audio_pause;
	impl_esd_audio_flush = arts_esd_audio_flush;
	impl_esd_audio_write = arts_esd_audio_write;
	impl_esd_audio_read = arts_esd_audio_read;
    }
    atexit(impl_exit);
}
