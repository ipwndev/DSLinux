// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
//
// DESCRIPTION:
//	System interface, sound.
//
//-----------------------------------------------------------------------------

#ifndef __I_SOUND__
#define __I_SOUND__

#include "doomdef.h"

#include "doomstat.h"
#include "sounds.h"



// Init at program start...
void I_InitSound();

// ... shut down and relase at program termination.
void I_ShutdownSound(void);


//
//  SFX I/O
//

// Initialize channels?
void I_SetChannels();

// Get raw data lump index for sound descriptor.
int I_GetSfxLumpNum (sfxinfo_t* sfxinfo );


// Starts a sound in a particular sound channel.
int
I_StartSound
( int		id,
  int		vol,
  int		sep,
  int		pitch,
  int		priority );


// Stops a sound channel.
void I_StopSound(int handle);

// Called by S_*() functions
//  to see if a channel is still playing.
// Returns 0 if no longer playing, 1 if playing.
int I_SoundIsPlaying(int handle);

// Updates the volume, separation,
//  and pitch of a sound channel.
void
I_UpdateSoundParams
( int		handle,
  int		vol,
  int		sep,
  int		pitch );


//
//  MUSIC I/O
//
void I_InitMusic(void);
void I_ShutdownMusic(void);
// Volume.
void I_SetMusicVolume(int volume);
// PAUSE game handling.
void I_PauseSong(int handle);
void I_ResumeSong(int handle);
// Registers a song handle to song data.
int I_RegisterSong(void *data);
// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.
void
I_PlaySong
( int		handle,
  int		looping );
// Stops a song over 3 seconds.
void I_StopSong(int handle);
// See above (register), then think backwards
void I_UnRegisterSong(int handle);



#endif
//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2006/10/03 11:26:17  dslinux_amadeus
// adding pristine copy of pixil to HEAD so I can branch from it
//
// Revision 1.2  2003/09/08 22:34:28  jasonk
// Updated files because this fucker won't build for no fucking good reason.
//
// Revision 1.1.1.1  2003/09/04 21:08:12  jasonk
// Initial import
//
// Revision 1.1  2000/12/08 21:07:53  jeffw
// nxdoom initial entry -- No nxdoom/Makefile so it won't build automatically
//
//
//-----------------------------------------------------------------------------
