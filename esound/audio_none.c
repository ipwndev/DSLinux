#ifndef DRIVER_NONE

#error No known audio device.  Double check the autoconfigure output in 
#error config.log.  Theoretically, this should support the following
#error audio systems: 
#error      Linux (oss), aix, hpux, irix, and solaris
#error Currently, the following drivers are untested and probably need work:
#error      aix, hpux, irix, and solaris
#error Additional drivers appreciated. See README for info on how to help.

#else

#define ARCH_esd_audio_open
int esd_audio_open()
{
    printf("WARNING: sound not available");
    return -1;
}

#endif
