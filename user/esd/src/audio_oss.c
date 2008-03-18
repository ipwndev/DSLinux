#include "config.h"

#ifdef HAVE_MACHINE_SOUNDCARD_H
#  include <machine/soundcard.h>
#else
#  ifdef HAVE_SOUNDCARD_H
#    include <soundcard.h>
#  else
#    include <sys/soundcard.h>
#  endif
#endif


/* FreeBSD uses a different identifier? what other BSDs? */
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#define SNDCTL_DSP_SETDUPLEX DSP_CAP_DUPLEX
#endif

#define ARCH_esd_audio_devices
const char *esd_audio_devices()
{
    return "/dev/dsp, /dev/dsp2, etc.";
}


#define ARCH_esd_audio_open
int esd_audio_open()
{
    const char *device;

    int afd = -1, value = 0, test = 0;
    int mode = O_WRONLY;

    /* if recording, set for full duplex mode */
    if ( (esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD )
        mode = O_RDWR;

    mode |= O_NONBLOCK;

    /* open the sound device */
    device = esd_audio_device ? esd_audio_device : "/dev/dsp";
    if ((afd = open(device, mode, 0)) == -1)
    {   /* Opening device failed */
        perror(device);
        return( -2 );
    }

    mode = fcntl(afd, F_GETFL);
    mode &= ~O_NONBLOCK;
    fcntl(afd, F_SETFL, mode);

    /* TODO: check that this is allowable */
    /* set for full duplex operation, if recording */
#if defined(SNDCTL_DSP_SETDUPLEX)
    if ( (esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD ) {
        ioctl( afd, SNDCTL_DSP_SETDUPLEX, 0 );
    }
#endif

    /* set the sound driver fragment size and number */
#if !defined(__powerpc__) /* does not exist on powerpc */
    /* fragment = max_buffers << 16 + log2(buffer_size), (256 16) */
    value = test = ( 0x0100 << 16 ) + 0x0008;
    if (ioctl(afd, SNDCTL_DSP_SETFRAGMENT, &test) == -1)
    {   /* Fatal error */
        perror( "SNDCTL_DSP_SETFRAGMENT" );
        close( afd );
        esd_audio_fd = -1;
        return( -1 );
    }
    if ( 0 /* value != test */ ) /* TODO: determine the real test */
    {   /* The device doesn't support the requested audio format. */
        fprintf( stderr, "unsupported fragment size: %d\n", value );
        close( afd );
        esd_audio_fd = -1;
        return( -1 );
    }
#endif /* #if !defined(__powerpc__) */

    /* set the sound driver audio format for playback */
    
    value = test = ( (esd_audio_format & ESD_MASK_BITS) == ESD_BITS16 )
        ? /* 16 bit */ AFMT_S16_NE : /* 8 bit */ AFMT_U8;

    if (ioctl(afd, SNDCTL_DSP_SETFMT, &test) == -1)
    {   /* Fatal error */
        perror("SNDCTL_DSP_SETFMT");
        close( afd );
        esd_audio_fd = -1;
        return( -1 );
    }

    ioctl(afd, SNDCTL_DSP_GETFMTS, &test);
    if ( !(value & test) ) /* TODO: should this be if ( value XOR test ) ??? */
    {   /* The device doesn't support the requested audio format. */
        fprintf( stderr, "unsupported sound format: %d\n", esd_audio_format );
        close( afd );
        esd_audio_fd = -1;
        return( -1 );
    }

    /* set the sound driver number of channels for playback */
    value = test = ( ( ( esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO )
        ? /* stereo */ 1 : /* mono */ 0 );
    if (ioctl(afd, SNDCTL_DSP_STEREO, &test) == -1)
    {   /* Fatal error */
        perror( "SNDCTL_DSP_STEREO" );
        close( afd );
        esd_audio_fd = -1;
        return( -1 );
    }

    /* set the sound driver number playback rate */
    test = esd_audio_rate;
    if ( ioctl(afd, SNDCTL_DSP_SPEED, &test) == -1)
    { /* Fatal error */
        perror("SNDCTL_DSP_SPEED");
        close( afd );
        esd_audio_fd = -1;
        return( -1 );
    }

    /* see if actual speed is within 5% of requested speed */
    if( fabs( test - esd_audio_rate ) > esd_audio_rate * 0.05 )
    {
        fprintf( stderr, "unsupported playback rate: %d\n", esd_audio_rate );
        close( afd );
        esd_audio_fd = -1;
        return( -1 );
    }

    /* value = test = buf_size; */
    esd_audio_fd = afd;
    return afd;
}

#define ARCH_esd_audio_pause
void esd_audio_pause()
{
    /* per oss specs */
    ioctl( esd_audio_fd, SNDCTL_DSP_POST, 0 );
    return;
}
