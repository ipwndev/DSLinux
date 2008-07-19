/*
 * Taken mainly from xmp, (C) 1996-98 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Completely untested.. so fix it
 */

#ifdef HAVE_SYS_AUDIOIO_H
#include <sys/audioio.h>
#elif defined(HAVE_SYS_AUDIO_IO_H)
#include <sys/audio.io.h>
#elif defined(HAVE_SUN_AUDIOIO_H)
#include <sun/audioio.h>
#endif
#include <stropts.h>
#include <errno.h>
#include <alloca.h>


/* if you want to confirm proper device setup, uncomment the following line */
/* #define ESDBG_DRIVER */

static char *default_device = "/dev/audio";

static char *my_ports = NULL;

static void
free_the_crap(void)
{
    if(my_ports)
	free(my_ports);
}

#define ARCH_esd_audio_devices
const char *esd_audio_devices()
{
    /*
     * I don't know what's the use for this crappy format. Are we supposed
     * to return everything the OS can possibly support or just the
     * devices we have available? If the later, shouldn't we make a difference
     * between play and record devices?
     *
     * Let's assume we want to return available devices. In that case we
     * first have to make a difference between device (something living in
     * the file system, which can be opened in order to talk to the audio
     * driver, which essentialy correspond to audio card) and audio ports
     * (which are things like speaker, lineout etc.). It seems esd uses
     * "device" for our "ports". But how does it make a difference between
     * different audio cards? They don't have to support the same ports.
     *
     * Anyway, the original code was just returning the static string.
     * We'll try to return the available ports. But we need to calculate
     * the stupid string, which means we have to free it at some point.
     * This thing is a library and it can be dlclosed() at any time. Oh, joy.
     * 
     * We could free the string at esd_audio_close() time, but it doesn't
     * make much sense, because that's not library unload and the
     * calculated string won't be different when another esd_audio_devices()
     * call comes. So we'll use atexit() to register the clean-up function.
     * On Solaris 8 that frees the string at library unload time.
     * On earlier releases it doesn't. So you'll have a small memory
     * leak. If somebody explains me what's the use for this function, I
     * might even want to fix it.  -- dave@arsdigita.com
     */

    const char *device;
    char *ctl;
    int fd, err;
    struct audio_info auinfo;
    unsigned int avail_ports;

    if (my_ports)
	return my_ports + 2;

    /* Use the control device, because opening the normal device might block */
    if (!(device = getenv("AUDIODEV")))
    {
	device = default_device;
    }
    ctl = alloca(strlen(device) + 4);
    strcpy(ctl, device);
    strcat(ctl, "ctl");

    /* Try to open non-blocking. We don't want to block on this shit. */

    do {
	fd = open(ctl, O_WRONLY | O_NONBLOCK);
    } while(fd == -1 && errno == EINTR);
    if (fd == -1)
    {
	if(errno == EBUSY)
	    /* Just return some default crap. */
	    goto hell;
	/* Otherwise we don't have available audio device, so no ports. */
	return "";
    }

    /* Get the available ports */
    do {
	err = ioctl(fd, AUDIO_GETINFO, &auinfo);
    } while (err == -1 && errno == EINTR);
    close(fd);
    if (err == -1)
	goto hell;
    avail_ports = auinfo.record.avail_ports | auinfo.play.avail_ports;
    if (!(my_ports = malloc(100))) /* XXX hardcoded string size */
	goto hell;
    atexit(free_the_crap);

    /* Now build the string. Everybody's returning only output ports, so... */

    my_ports[0] = 0;
    if (avail_ports & AUDIO_SPEAKER)
	strcat(my_ports, ", speaker");
    if (avail_ports & AUDIO_HEADPHONE)
	strcat(my_ports, ", headphone");
    if (avail_ports & AUDIO_LINE_OUT)
	strcat(my_ports, ", lineout");
#ifdef AUDIO_SPDIF_OUT
    if (avail_ports & AUDIO_SPDIF_OUT)
	strcat(my_ports, ", spdif");
#endif
#ifdef AUDIO_AUX1_OUT
    if (avail_ports & AUDIO_AUX1_OUT)
	strcat(my_ports, ", aux1");
#endif
#ifdef AUDIO_AUX2_OUT
    if (avail_ports & AUDIO_AUX2_OUT)
	strcat(my_ports, ", aux2");
#endif
    return my_ports + 2;

hell:
    return "speaker, lineout, headphone";
}


void dump_audio_info(audio_info_t *t, int play)
{
    if( play )
    {
	char *enc, aports[200], ports[200];

	switch( t->play.encoding )
	{
	case AUDIO_ENCODING_NONE:
	    enc = "AUDIO_ENCODING_NONE";
	    break;
	case AUDIO_ENCODING_ULAW:
	    enc = "AUDIO_ENCODING_ULAW";
	    break;
	case AUDIO_ENCODING_ALAW:
	    enc = "AUDIO_ENCODING_ALAW";
	    break;
	case AUDIO_ENCODING_LINEAR:
	    enc = "AUDIO_ENCODING_LINEAR";
	    break;
	}

	ports[0] = 0;
	if( t->play.port & AUDIO_SPEAKER )
	    strcat(ports, " & AUDIO_SPEAKER");
	if( t->play.port & AUDIO_HEADPHONE )
	    strcat(ports, " & AUDIO_HEADPHONE");
	if( t->play.port & AUDIO_LINE_OUT)
	    strcat(ports, " & AUDIO_LINE_OUT");
#ifdef AUDIO_SPDIF_OUT
	if ( t->play.port & AUDIO_SPDIF_OUT )
	    strcat(ports, " & AUDIO_SPDIF_OUT");
#endif
#ifdef AUDIO_AUX1_OUT
	if ( t->play.port & AUDIO_AUX1_OUT )
	    strcat(ports, " & AUDIO_AUX1_OUT");
#endif
#ifdef AUDIO_AUX2_OUT
	if ( t->play.port & AUDIO_AUX2_OUT )
	    strcat(ports, " & AUDIO_AUX2_OUT");
#endif

	aports[0] = 0;
	if( t->play.port & AUDIO_SPEAKER )
	    strcat(aports, " & AUDIO_SPEAKER");
	if( t->play.port & AUDIO_HEADPHONE )
	    strcat(aports, " & AUDIO_HEADPHONE");
	if( t->play.port & AUDIO_LINE_OUT)
	    strcat(aports, " & AUDIO_LINE_OUT");
#ifdef AUDIO_SPDIF_OUT
	if ( t->play.port & AUDIO_SPDIF_OUT )
	    strcat(aports, " & AUDIO_SPDIF_OUT");
#endif
#ifdef AUDIO_AUX1_OUT
	if ( t->play.port & AUDIO_AUX1_OUT )
	    strcat(aports, " & AUDIO_AUX1_OUT");
#endif
#ifdef AUDIO_AUX2_OUT
	if ( t->play.port & AUDIO_AUX2_OUT )
	    strcat(aports, " & AUDIO_AUX2_OUT");
#endif
               
	printf("Play Info:\n");
	printf(" Sample Rate: %d\n", t->play.sample_rate);
	printf(" Channels:    %d\n", t->play.channels);
	printf(" Bits/sample: %d\n", t->play.precision);
	printf(" Encoding:    %s\n", enc);
	printf(" Gain:        %d\n", t->play.gain);
	printf(" Port:        %s\n", ports + 3);
	printf(" availPorts:  %s\n", aports + 3);
	printf(" Mon Gain:    %d\n", t->monitor_gain);
	printf(" o/p Muted:   %d\n", t->output_muted);
	printf(" Balance:     %d\n", t->play.balance);
    }

    return;
}


/*
 * To anybody hacking at this:
 *
 *   return -1 means something failed, but the main library will try again
 *	       with the different options
 *   return -2 means big trouble and the main library won't try again
 *
 * -2 is the appropriate value if we detected audio device we don't support
 * or if we can't even open the device or something like that.
 *
 * -1 is intended for conditions when the device doesn't support audio
 * parameters. In that case the main library can try again, but it needs
 * to call us again to open the device with another set of audio
 * parameters.
 *
 * Don't just put "return -1" at random places. If everything fails (which
 * is -2 situation), the user will get 10 or more error messages.
 * That sucks.
 *
 * -- dave@arsdigita.com
 */

#define ARCH_esd_audio_open
int fill_play_info (audio_info_t *ainfo)
{
    /*
     * Volume, balance and output device should be controlled by
     * an external program - that way the user can set his preferences
     * for all players  -- dave@arsdigita.com
    */

    /* int gain = 64;       /* Range: 0 - 255 */
    int port;
    int bsize = 8180;
    if ( esd_audio_device == NULL )
	/* Don't change the output device unless specificaly requested */
	port = 0;
    else if ( !strcmp( esd_audio_device, "lineout" ) )
	port = AUDIO_LINE_OUT;
    else if ( !strcmp( esd_audio_device, "speaker" ) )
	port = AUDIO_SPEAKER;
    else if ( !strcmp( esd_audio_device, "headphone" ) )
	port = AUDIO_HEADPHONE;
	/*
	* The #ifdefs below are for the older OS releases. They won't have
	* these things defined, so the compilation would break without them.
	*/
#ifdef AUDIO_SPDIF_OUT
    else if ( !strcmp( esd_audio_device, "spdif" ) )
	port = AUDIO_SPDIF_OUT;
#endif
#ifdef AUDIO_AUX1_OUT
    else if ( !strcmp( esd_audio_device, "aux1" ) )
	port = AUDIO_AUX1_OUT;
#endif
#ifdef AUDIO_AUX2_OUT
    else if ( !strcmp( esd_audio_device, "aux2" ) )
	port = AUDIO_AUX2_OUT;
#endif
    else {
	return -1;
    }

    ainfo->play.sample_rate = esd_audio_rate;

    if ((esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO)
	ainfo->play.channels = 2;
    else
	ainfo->play.channels = 1;

    if ((esd_audio_format & ESD_MASK_BITS) == ESD_BITS16)
	ainfo->play.precision = 16;
    else
	ainfo->play.precision = 8;

    ainfo->play.encoding = AUDIO_ENCODING_LINEAR;
    /* ainfo->play.gain = gain; */
    if(port)
	ainfo->play.port = port;
    /* ainfo->play.balance = AUDIO_MID_BALANCE; */
    ainfo->play.buffer_size = bsize;
    ainfo->output_muted = 0;

#ifdef ESDBG_DRIVER
    dump_audio_info(ainfo,1);
#endif

   return 1;
}

int esd_audio_open()
{
    int afd = -1, cafd = -1;
    audio_device_t adev;
    const char *device;
    char *devicectl;
    int err;
    int ret;

    /*
     * Recording code needs access to the control device, but the playing
     * code currently doesn't. Control device is the name of audio device
     * with appended "ctl". So we'll just alloca() memory for that, because
     * it can't be very large and it will allow us to just return and not
     * care about freeing the memory.  -- dave@arsdigita.com
     */

    if ((device = getenv("AUDIODEV")) == NULL)
	device = default_device;
    devicectl = alloca(strlen(device) + 4);
    strcpy(devicectl, device);
    strcat(devicectl, "ctl");
    
    if ((esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD) {
	audio_info_t ainfo;
               
	AUDIO_INITINFO(&ainfo);
	do {
	    cafd = open(devicectl, O_RDWR);
	} while (cafd == -1 && errno == EINTR);
	if( cafd == -1 )
	{
	    fprintf(stderr,"esd: Could not open ctl device for recording\n");
	    esd_audio_fd = -1;
	    return -1;
	}
	do {
	    err = ioctl(cafd, AUDIO_GETDEV, &adev);
	} while (err == -1 && errno == EINTR);
	if (err == -1) {
	    fprintf(stderr, "esd: ioctl(\"%s\", AUDIO_GETDEV failed: %s\n",
		    devicectl, strerror(errno));
	    close(cafd);
	    esd_audio_fd = -1;
	    return -1;
	}

	/*
	 * Let's check the device name to see if we know how to handle
	 * it. The problem with this is that the new audio devices will
	 * become available over time and it will happen that this program
	 * can use it, but the device name won't be known to it. This just
	 * happened with the Blade machines which have SUNW,audiots.
	 * In attempt to prevent this from happening again, we'll try
	 * the following approach:
	 *  - First check if it's one we know we can't handle. If so,
	 *    print the error message and return.
	 *  - Then check if it's one we know about. If so, everything's
	 *    peachy keen.
	 *  - If it's a device we don't know about, assume that we can
	 *    use it and just print a warning.
	 *
	 *  -- dave@arsdigita.com
	 */

	if ( !(strcmp(adev.name, "SUNW,am79c30")))
	{
	     fprintf(stderr, "esd: Cannot handle device `%s'.\n",
		     adev.name);
	     esd_audio_fd = -1;
	     return -2;
	}
	if ( (strcmp(adev.name, "SUNW,CS4231") != 0) 
	     && (strcmp(adev.name, "SUNW,audiots") != 0)
	     && (strcmp(adev.name, "SUNW,sb16")  != 0)
	     && (strcmp(adev.name, "SUNW,sbpro") != 0)
	     && (strcmp(adev.name, "SUNW,dbri") != 0)  )
	 {
	    fprintf(stderr, "esd: Unknown device `%s', but will try anyway\n",
		    adev.name);
	 }
	/* SUNW,CS4231 and most others */
	{
	    int gain = 255; /* Range: 0 - 255 */
	    int port = AUDIO_MICROPHONE;
	    int bsize = 8180;
                       
	    ainfo.record.sample_rate = esd_audio_rate;
	    
	    if ((esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO)
		ainfo.record.channels = 2;
	    else
		ainfo.record.channels = 1;
                       
	    if ((esd_audio_format & ESD_MASK_BITS) == ESD_BITS16)
		ainfo.record.precision = 16;
	    else
		ainfo.record.precision = 8;
                       
	    ainfo.record.encoding = AUDIO_ENCODING_LINEAR;
	    ainfo.record.gain = gain;
	    ainfo.record.port = port;
	    ainfo.record.balance = AUDIO_MID_BALANCE;
	    ainfo.record.buffer_size = bsize;
	    ret = fill_play_info (&ainfo);
	    if (ret < 0) {
		fprintf(stderr, "esd: Unknown output device `%s'.\n",
			esd_audio_device);
		close(afd);
		esd_audio_fd = -1;
		return -1;
	    }

	    /* actually, it doesn't look like we need to set any
	       settings here-- they always seem to be the default, no
	       matter what else was spec.  
	    fprintf( stderr, "record set up: "
	             "rate=%d, channels=%d, precision=%d, gain=%d, port=%x\n",
		     ainfo.record.sample_rate, ainfo.record.channels, 
		     ainfo.record.precision, ainfo.record.gain, ainfo.record.port); 
	    */
	}
               
	do {
	    afd = open(device, O_RDWR);
	} while (afd == -1 && errno == EINTR);
	if (afd == -1) {
	    fprintf(stderr, "esd: Opening %s device failed: %s\n",
		    device, strerror(errno));
	    esd_audio_fd = -1;
	    return -1;
	}
 
	do {
	    err = ioctl(afd, AUDIO_SETINFO, &ainfo);
	} while (err == -1 && errno == EINTR);
	if (err == -1)
	{
	    fprintf(stderr, "esd: ioctl(\"%s\", AUDIO_SETINFO) failed: %s\n",
		    device, strerror(errno));
	    esd_audio_fd = -1;
	    return -1;
	}
	esd_audio_fd = afd;
	return afd;
    }
    /* implied else: if ( (esd_audio_format & ESD_MASK_FUNC) != ESD_RECORD ) */
    
    do {
	afd = open(device, O_WRONLY);
    } while (afd == -1 && errno == EINTR);
    if (afd == -1) {
       if(errno != EACCES && errno != ENOENT)
	   fprintf(stderr, "esd: Opening %s device failed: %s\n", device,
		   strerror(errno));
       /*
	* Don't spit errors in other cases; the user is likely to know
	* that he doesn't have audio device or that he doesn't have the
	* permission to use it (by default only the user logged at the
	* console can use audio device. All other users on the server can't.)
	* So every program which links to this library doesn't have to
	* tell him that.  -- dave@arsdigita.com
	*/
       esd_audio_fd = -1;
       return -2;
    }

    do {
	err = ioctl(afd, AUDIO_GETDEV, &adev);
    } while (err == -1 && errno == EINTR);
    if (err == -1) {
	fprintf(stderr, "esd: ioctl(\"%s\", AUDIO_GETDEV) failed: %s\n",
		device, strerror(errno));
        close(afd);
        esd_audio_fd = -1;

	/*
	 * Whatever this was, it's probably going to come again, so there's
	 * no point in trying to reopen the device with another setting.
	 * We didn't even come to that.  -- dave@arsdigita.com
	 */

        return -2;
    }

    /*
     * Device check as above: assume that unknown devices are new hardware
     * and that we can handle them just fine.  -- dave@arsdigita.com
     */
    if ( !strcmp(adev.name, "SUNW,am79c30"))
    {
	fprintf(stderr, "esd: Cannot handle device `%s'.\n", adev.name);
	close(afd);
	esd_audio_fd = -1;
	return -1;
    }
    if ( (strcmp(adev.name, "SUNW,CS4231") != 0)
	&& (strcmp(adev.name, "SUNW,audiots") != 0)
	&& (strcmp(adev.name, "SUNW,sb16")  != 0)
	&& (strcmp(adev.name, "SUNW,sbpro")  != 0)
	&& (strcmp(adev.name, "SUNW,dbri") != 0)  ) 
    {
 	fprintf(stderr, "esd: Unknown device `%s', but will try anyway.\n",
		adev.name);
    }

    {
	audio_info_t ainfo;
      
	AUDIO_INITINFO(&ainfo);
      
	ret = fill_play_info (&ainfo);
	if (ret < 0) {
		fprintf(stderr, "esd: Unknown output device `%s'.\n",
			esd_audio_device);
		close(afd);
		esd_audio_fd = -1;
		return -1;
	}

	do {
	    err = ioctl(afd, AUDIO_SETINFO, &ainfo);
	} while (err == -1 && errno == EINTR);
	if (err == -1)
	{
	    fprintf(stderr, "esd: ioctl(\"%s\", AUDIO_SETINFO) failed: %s\n",
		    device, strerror(errno));
	    close(afd);
	    esd_audio_fd = -1;
	    return -1;
	}
    }
    
    esd_audio_fd = afd;
    return afd;
}

#define ARCH_esd_audio_flush
void
esd_audio_flush()
{
    if (esd_audio_format & (ESD_PLAY | ESD_RECORD) == ESD_PLAY | ESD_RECORD)
	ioctl(esd_audio_fd, I_FLUSH, FLUSHRW);
    else
	if (esd_audio_format & ESD_PLAY == ESD_PLAY)
	    ioctl(esd_audio_fd, I_FLUSH, FLUSHW);
	else
	    if (esd_audio_format & ESD_RECORD == ESD_RECORD)
		ioctl(esd_audio_fd, I_FLUSH, FLUSHR);
}

#define ARCH_esd_audio_close
void
esd_audio_close()
{
    /* esd_audio_flush();  Should we flush here or not? */
    close(esd_audio_fd);
}
