/*
 * $Id: soundmanager.c,v 1.4 2003/02/09 07:34:16 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * BGM/SE manager(using SDL_mixer).
 *
 * @version $Revision: 1.4 $
 */
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "SDL_mixer.h"
#include "soundmanager.h"

static int useAudio = 0;

#define MUSIC_NUM 7
#define SHARE_LOC "/usr/share/games/noiz2sa/"

static char *musicFileName[MUSIC_NUM] = {
  "stg0.ogg", "stg1.ogg", "stg2.ogg", "stg3.ogg", "stg4.ogg", "stg5.ogg", "stg00.ogg",
};
static Mix_Music *music[MUSIC_NUM];

#define CHUNK_NUM 7

static char *chunkFileName[CHUNK_NUM] = {
  "shot.wav", "hit.wav", "foedst.wav", "bossdst.wav", "shipdst.wav", "bonus.wav", "extend.wav",
};
static Mix_Chunk *chunk[CHUNK_NUM];
static int chunkFlag[CHUNK_NUM];

void closeSound() {
  int i;
  if ( !useAudio ) return;
  if ( Mix_PlayingMusic() ) {
    Mix_HaltMusic();
  }
  for ( i=0 ; i<MUSIC_NUM ; i++ ) {
    if ( music[i] ) {
      Mix_FreeMusic(music[i]);
    }
  }
  for ( i=0 ; i<CHUNK_NUM ; i++ ) {
    if ( chunk[i] ) {
      Mix_FreeChunk(chunk[i]);
    }
  }
  Mix_CloseAudio();
}


// Initialize the sound.

static void loadSounds() {
  int i;
  char name[56];

  for ( i=0 ; i<MUSIC_NUM ; i++ ) {
    strcpy(name, SHARE_LOC);
    strcat(name, "sounds/");
    strcat(name, musicFileName[i]);
    if ( NULL == (music[i] = Mix_LoadMUS(name)) ) {
      fprintf(stderr, "Couldn't load: %s\n", name);
      useAudio = 0;
      return;
    }
  }
  for ( i=0 ; i<CHUNK_NUM ; i++ ) {
    strcpy(name, SHARE_LOC);
    strcat(name, "sounds/");
    strcat(name, chunkFileName[i]);
    if ( NULL == (chunk[i] = Mix_LoadWAV(name)) ) {
      fprintf(stderr, "Couldn't load: %s\n", name);
      useAudio = 0;
      return;
    }
    chunkFlag[i] = 0;
  }
}

void initSound() {
  int audio_rate;
  Uint16 audio_format;
  int audio_channels;
  int audio_buffers;
  int interactive = 0;

  if ( SDL_InitSubSystem(SDL_INIT_AUDIO) < 0 ) {
    fprintf(stderr, "Unable to initialize SDL_AUDIO: %s\n", SDL_GetError());
    return;
  }

  audio_rate = 44100;
  audio_format = AUDIO_S16;
  audio_channels = 1;
  audio_buffers = 4096;

  if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return;
  } else {
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
  }

  useAudio = 1;
  loadSounds();
}

// Play/Stop the music/chunk.

void playMusic(int idx) {
  if ( !useAudio ) return;
  Mix_PlayMusic(music[idx], -1);
}

void fadeMusic() {
  if ( !useAudio ) return;
  Mix_FadeOutMusic(1280);
}

void stopMusic() {
  if ( !useAudio ) return;
  if ( Mix_PlayingMusic() ) {
    Mix_HaltMusic();
  }
}

void playChunk(int idx) {
  if ( !useAudio ) return;
  Mix_PlayChannel(idx, chunk[idx], 0);
}
