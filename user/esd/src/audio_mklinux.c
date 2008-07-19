#ifdef HAVE_MACHINE_SOUNDCARD_H
#  include <machine/soundcard.h>
#else
#  ifdef HAVE_SOUNDCARD_H
#    include <soundcard.h>
#  else
#    include <sys/soundcard.h>
#  endif
#endif

#define ARCH_esd_audio_open
int esd_audio_open()
{
    const char *device = "/dev/dsp";

    int afd = -1, value = 0, test = 0;
    int mode = O_WRONLY;

    /* if recording, set for full duplex mode */
    if ( (esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD )
        mode = O_RDWR;


    /* open the sound device */
    if ((afd = open(device, mode, 0)) == -1)
    {   /* Opening device failed */
        perror(device);
        return( -1 );
    }


    /* TODO: check that this is allowable */
    /* set for full duplex operation, if recording */
    if ( (esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD ) {
        ioctl( afd, SNDCTL_DSP_SETDUPLEX, 0 );
    }
#if 0
    /* set the sound driver fragment size and number */
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
#endif
    /* set the sound driver audio format for playback */
    value = test = ( (esd_audio_format & ESD_MASK_BITS) == ESD_BITS16 )
        ? /* 16 bit */ 16 : /* 8 bit */ 8;
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
    sleep(1); /* give the driver a chance to wake up, it's kinda finicky that way... */
    return afd;
}

#define ARCH_esd_audio_pause
void esd_audio_pause()
{
    /* per oss specs */
    ioctl( esd_audio_fd, SNDCTL_DSP_POST, 0 );
    return;
}

#define ARCH_esd_audio_write
int esd_audio_write( void *buffer, int buf_size ) {
  int stereo =  ( esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO;
  int bits = (esd_audio_format & ESD_MASK_BITS) == ESD_BITS16;
  int i;
 
 
  /* MkLinux uses unsigned bytes for mono 8-bit and signed bytes for stereo 8-bit */ 
  if (stereo && !bits)
    for (i = 0; i < buf_size; i++)
      ((char*)buffer)[i] = ((unsigned char*)buffer)[i] ^ 0x80;
 
  return write( esd_audio_fd, buffer, buf_size );
}
