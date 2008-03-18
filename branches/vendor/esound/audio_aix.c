/*
 * Completely, utterly untested.. most likely broke
 *   Code's by George L. Coulouris
 */

#include <sys/audio.h>

#define ARCH_esd_audio_open
int esd_audio_open()
{
   audio_init a_init;
   audio_change a_change;
   audio_control a_control;
   int afd;
   int mode = O_WRONLY;
   int flag;
   static const char device[] = "/dev/paud0/1";
   
   if ((esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD) {
       fprintf(stderr, "No idea how to record audio on solaris, FIXME\n");
       esd_audio_fd = -1;
       return -1;
   }

   if ((afd = open(device, mode)) == -1) {
       perror(device);
       esd_audio_fd = -1;
       return -1;
   }
   
   a_init.srate = esd_audio_rate;
   
   if ((esd_audio_format & ESD_MASK_CHAN) == ESD_STEREO)
       a_init.channels = 2;
   else
       a_init.channels = 1;

   if ((esd_audio_format & ESD_MASK_BITS) == ESD_BITS16)
       a_init.bits_per_sample = 16;
   else
       a_init.bits_per_sample = 8;
  
   a_init.mode = PCM;
   a_init.flags = FIXED | BIG_ENDIAN | TWOS_COMPLEMENT;
   a_init.operation = PLAY;
   
   a_change.balance = 0x3fff0000;
   a_change.balance_delay = 0;
   a_change.volume = 0x7fffffff;
   a_change.volume_delay = 0;
   a_change.monitor = AUDIO_IGNORE;
   a_change.input = AUDIO_IGNORE;
   a_change.output = OUTPUT_1;
   
   a_control.ioctl_request = AUDIO_CHANGE;
   a_control.position = 0;
   a_control.request_info = &a_change;
  
   if (ioctl(afd, AUDIO_INIT, &a_init) == -1) {
       perror("AUDIO_INIT");
       close(afd);
       esd_audio_fd = -1;
       return -1;
   }
  
   if (ioctl(afd, AUDIO_CONTROL, &a_control) == -1) {
       perror("AUDIO_CONTROL");
       close(afd);
       esd_audio_fd = -1;
       return -1;
   }
  
   a_control.ioctl_request = AUDIO_START;
  
   if (ioctl(afd, AUDIO_CONTROL, &a_control) == -1) {
       perror("AUDIO_CONTROL");
       close(afd);
       esd_audio_fd = -1;
       return -1;
   }
   
   esd_audio_fd = afd;
   return afd;
}
