/* Simple ESD handling for sounds and whatnot */

#include <fcntl.h>

#ifdef USE_ESOUND
#include <esd.h>
#endif

#ifdef USE_DSP
#include <linux/soundcard.h>
#endif


#include "z_zone.h"

#include "m_swap.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"

#include "doomdef.h"

#define SAMPLERATE 11025

static int SAMPLECOUNT=		2048;

#ifdef USE_ESOUND
int esd_sock;
int esd_stream;
#endif

#ifdef USE_DSP
int dsp_sock;
#endif

int sample_ids[NUMSFX];

// The actual lengths of all sound effects.
int 		lengths[NUMSFX];

#define CHANNEL_COUNT 8

struct
{
  int active;
  int position;
  int size;
  char *data;
  int age;
} channel[CHANNEL_COUNT];

static int open_dsp();
static int configure_dsp(int, int, int, int);
static void close_dsp(int);

void*
getsfx
( char*         sfxname,
  int*          len )
{
    unsigned char*      sfx;
    unsigned char*      paddedsfx;
    int                 i;
    int                 size;
    int                 paddedsize;
    char                name[20];
    int                 sfxlump;

    
    // Get the sound data from the WAD, allocate lump
    //  in zone memory.
    sprintf(name, "ds%s", sfxname);

    // Now, there is a severe problem with the
    //  sound handling, in it is not (yet/anymore)
    //  gamemode aware. That means, sounds from
    //  DOOM II will be requested even with DOOM
    //  shareware.
    // The sound list is wired into sounds.c,
    //  which sets the external variable.
    // I do not do runtime patches to that
    //  variable. Instead, we will use a
    //  default sound for replacement.
    if ( W_CheckNumForName(name) == -1 )
      sfxlump = W_GetNumForName("dspistol");
    else
      sfxlump = W_GetNumForName(name);
    
    size = W_LumpLength( sfxlump );

    // Debug.
    // fprintf( stderr, "." );
    //fprintf( stderr, " -loading  %s (lump %d, %d bytes)\n",
    //	     sfxname, sfxlump, size );
    //fflush( stderr );
    
    sfx = (unsigned char*)W_CacheLumpNum( sfxlump, PU_STATIC );

    // Pads the sound effect out to the mixing buffer size.
    // The original realloc would interfere with zone memory.
    paddedsize = ((size-8 + (SAMPLECOUNT-1)) / SAMPLECOUNT) * SAMPLECOUNT;

    // Allocate from zone memory.
    paddedsfx = (unsigned char *)Z_Malloc( paddedsize+8, PU_STATIC, 0 );
    // ddt: (unsigned char *) realloc(sfx, paddedsize+8);
    // This should interfere with zone memory handling,
    //  which does not kick in in the soundserver.

    // Now copy and pad.
    memcpy(  paddedsfx, sfx, size );

    for (i=size ; i<paddedsize+8 ; i++)
      paddedsfx[i] = 128;

    // Remove the cached lump.
    Z_Free( sfx );
    
    // Preserve padded length.
    *len = paddedsize;

    // Return allocated padded data.
    return (void *) (paddedsfx + 8);
}

int I_GetSfxLumpNum(sfxinfo_t* sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}

void I_SetChannels() {}

#ifdef NOTUSED
int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
  /* Ok, lets just start playing the sound */
  if (sample_ids[id])
    esd_sample_play(esd_sock, sample_ids[id]);

  return(sample_ids[id]);
}
#endif

int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{
  int i;

  int oldestval = -1;
  int old_index = -1;

  /* First ensure that this sound isn't already being played */
  for(i = 0; i < CHANNEL_COUNT;i++)
    {
      if (channel[i].active)
	if (channel[i].data = S_sfx[id].data)
	  {
	    channel[i].active = 0;
	    break;
	  }
    }

  for(i = 0; i < CHANNEL_COUNT; i++)
    {
      if (!channel[i].active)
	{
	  channel[i].position = 0;
	  channel[i].size = lengths[id];
	  channel[i].data = S_sfx[id].data;
	  channel[i].age = gametic;
	  channel[i].active = 1;
	  return;
	}

      if (old_index == -1)
	{
	  oldestval = channel[i].age;
	  old_index = i;
	}
      else
	{
	  if (channel[i].age < oldestval)
	    {
	      oldestval = channel[i].age;
	      old_index = i;
	    }
	}
    }

  if (old_index != -1)
    {
      channel[old_index].position = 0;
      channel[old_index].size = lengths[id];
      channel[old_index].data = S_sfx[id].data;
      channel[old_index].age = gametic;
      channel[old_index].active = 1;
    }
} 

void I_UpdateSound()
{
  int i;
  int size = SAMPLECOUNT; /* 16 bit */

#ifdef USE_ESOUND
  if (esd_stream <= 0)
    return;
#endif

#ifdef USE_DSP
  if (dsp_sock <= 0)
    return;
#endif

  /* Do a step, and mix in the current channels */
  for(i = 0; i < CHANNEL_COUNT; i++)
    {
      if(channel[i].active)
	{
	  int sizeleft = channel[i].size - channel[i].position;

	  if (sizeleft < (SAMPLECOUNT))
	    size = sizeleft;

#ifdef USE_ESOUND
	  if (write(esd_stream, channel[i].data + channel[i].position,
		    size) <= 0)
	    fprintf(stderr, "Errror writing sound!\n");
#endif

#ifdef USE_DSP
	  if (write(dsp_sock, channel[i].data + channel[i].position,
		    size) <= 0)
	    {
	      perror("dsp write");
	      fprintf(stderr, "Errror writing sound!\n");
	    }
#endif

	  channel[i].position += size;

	  if (channel[i].position >= channel[i].size)
	    {
	      channel[i].active = 0;
	    }
	}
    }
}

void I_StopSound(int handle)
{
}

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
  // unused
}

void I_ShutdownSound(void)
{
  int i;

#ifdef NOTUSED
  for(i = 0; i < NUMSFX; i++)
    if(sample_ids[i])
      esd_sample_free(esd_sock, sample_ids[i]);
#endif

#ifdef USE_ESOUND
  close(esd_stream);
  close(esd_sock);
#endif

#ifdef USE_DSP
  close_dsp(dsp_sock);
#endif

}

int I_SoundIsPlaying(int handle)
{
  return(1);
}

void I_SetMusicVolume(int volume)
{}

void I_InitSound()
{
#ifdef USE_ESOUND
  esd_format_t format = ESD_BITS8 | ESD_STEREO | ESD_STREAM | ESD_PLAY;
#endif

  int i, size = 0;
  int len = 0;
  int confirm_id = 0;

  fprintf(stderr, "I_InitSound starting: ");

#ifdef USE_ESOUND

  SAMPLECOUNT = SAMPLECOUNT * 1 * 1;

  /* Open up the ESD daemon */
  esd_sock = esd_open_sound(NULL);

  if (esd_sock == -1)
    {
      fprintf(stderr, "Unable to contact  the sound server\n");
      return;
    }

  esd_stream = esd_play_stream_fallback(format, SAMPLERATE, NULL, "doom");
  if (esd_stream <= 0)
    return;
#endif

#ifdef USE_DSP
  dsp_sock = open_dsp();
  
  if (dsp_sock == -1)
    {
      fprintf(stderr, "Unable to open /dev/dsp for writing\n");
      return;
    }

  if (configure_dsp(dsp_sock, 16, 0, SAMPLERATE) == -1)
    {
      fprintf(stderr, "Unable to configure /dev/dsp\n");
      dsp_sock = 0;
      return;
    }

#endif

  /* Now, load up all of the desired sounds */
  for(i = 1; i < NUMSFX; i++)
    {
      if (!S_sfx[i].link)
	{
	  // Load data from WAD file.
	  S_sfx[i].data = getsfx( S_sfx[i].name, &lengths[i] );
	}	
      else
	{
	  // Previously loaded already?
	  S_sfx[i].data = S_sfx[i].link->data;
	  lengths[i] = lengths[(S_sfx[i].link - S_sfx)/sizeof(sfxinfo_t)];
	}
    }

  fprintf(stderr, "Sound init completed\n");
}

void I_InitMusic(void)		{ }
void I_ShutdownMusic(void)	{ }

void I_PauseSong (int handle)
{
}

void I_ResumeSong (int handle)
{
}

int I_RegisterSong(void* data)
{
  return 1;
}

void I_PlaySong(int handle, int looping)
{}

void I_StopSong(int handle)
{}

void I_UnRegisterSong(int handle)
{
}

#ifdef USE_DSP
static int open_dsp()
{
  int fd = open("/dev/dsp", O_WRONLY, 0);

  if (fd == -1)
    {
      perror("open dsp");
      return(-1);
    }
}

static void close_dsp(int fd)
{
  close(fd);
}

static int configure_dsp(int fd, int size, int stereo, int speed)
{
  int bsize, lsize, lrate;

  /* Determine how big a block we can write at once */

  bsize = -1;

  if (ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &bsize) < 0)
    {
      perror("GETBLOCKSIZE");
      return(-1);
    }

  if (bsize < 4 || bsize > 65536)
    bsize = 1024;

  /* First sync it so we can change it */
  if (ioctl(fd, SNDCTL_DSP_SYNC, NULL) < 0)
    {
      perror("SYNC");
      return(-1);
    }

  lsize = AFMT_S16_LE;
  
  /* Set the sample size */
  if (ioctl(fd, SNDCTL_DSP_SETFMT, &lsize) == -1)
    {
      perror("SAMPLESIZE");
      return(-1);
    }

  fprintf(stderr, "Set %d as the size\n", lsize);

  /* set stereo */

  if (stereo)
    {
      if (ioctl(fd, SNDCTL_DSP_STEREO, &stereo) < 0)
	perror("STEREO");
    }

  /* set the speed */
  
  lrate = speed;

  if (ioctl(fd, SNDCTL_DSP_SPEED, &lrate) < 0)
    {
      perror("SPEED");
      return(-1);
    }

  /* Ok, now we're set.  We know how bit a block to */
  /* send, and we have set everything we need to set */
  /* go ahead and return */

  return(bsize);
}
  
#endif
