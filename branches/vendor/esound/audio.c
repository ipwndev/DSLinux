#include "esd.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>

/*******************************************************************/
/* globals */
esd_format_t esd_audio_format = ESD_BITS16 | ESD_STEREO;
int esd_audio_rate = ESD_DEFAULT_RATE;
char *esd_audio_device = NULL; /* aux device spec: /dev/dsp2, lineout, etc. */

/* the audio device, /dev/dsp, file descriptor */
static int esd_audio_fd = -1;

/*******************************************************************/
/* returns audio_fd for use by main prog - platform dependent */

/* ALSA before OSS as ALSA is OSS compatible */
#if defined(DRIVER_ALSA_09)
#  include "audio_alsa09.c"
#elif defined(DRIVER_ALSA) || defined(DRIVER_NEWALSA) 
#  include "audio_alsa.c"
#elif defined(DRIVER_OSS)
#  include "audio_oss.c"
#elif defined(DRIVER_AIX)
#  include "audio_aix.c"
#elif defined(DRIVER_IRIX)
#  include "audio_irix.c"
#elif defined(DRIVER_HPUX)
#  include "audio_hpux.c"
#elif defined(DRIVER_OSF)
#  include "audio_osf.c"
#elif defined(DRIVER_SOLARIS)
#  include "audio_solaris.c"
#elif defined(DRIVER_MKLINUX)
#  include "audio_mklinux.c"
#elif defined(DRIVER_DART)
#  include "audio_dart.c"
#elif defined(DRIVER_COREAUDIO)
#  include "audio_coreaudio.c"
#elif defined(DRIVER_ARTS)
#  include "audio_arts.c"
#else
#  include "audio_none.c"
#endif

/*******************************************************************/
/* display available devices */
#ifndef ARCH_esd_audio_devices
const char * esd_audio_devices()
{
    return "(default audio device)";
}
#endif

/*******************************************************************/
/* close the audio device */
#ifndef ARCH_esd_audio_close
void esd_audio_close()
{
    if ( esd_audio_fd != -1 ) 
       close( esd_audio_fd );
    esd_audio_fd = -1;
    return;
}
#endif

/*******************************************************************/
/* make the sound device quiet for a while */
#ifndef ARCH_esd_audio_pause
void esd_audio_pause()
{
    return;
}
#endif

#ifndef ARCH_esd_audio_write
/*******************************************************************/
/* dump a buffer to the sound device */
int esd_audio_write( void *buffer, int buf_size )
{
    return write( esd_audio_fd, buffer, buf_size );
}
#endif

#ifndef ARCH_esd_audio_read
/*******************************************************************/
/* read a chunk from the sound device */
int esd_audio_read( void *buffer, int buf_size )
{
    return read( esd_audio_fd, buffer, buf_size );
}
#endif

#ifndef ARCH_esd_audio_flush
/*******************************************************************/
/* flush the audio buffer */
void esd_audio_flush()
{
    fsync( esd_audio_fd );
    return;
}
#endif
