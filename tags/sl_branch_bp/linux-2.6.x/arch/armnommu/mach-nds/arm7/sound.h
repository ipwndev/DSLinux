
/* set the address of the buffer to play from */
extern void sound_set_address(u32 address);

/* set the size of the buffer to play from */
extern void sound_set_size(u32 size);

/* set the master volume */
extern void sound_set_master_volume(u8 vol);

/* set the number of channels */
extern void sound_set_channels(u8 channels);

/* set the sound format 0=s8, 1=s16, 2=ADPCM */
extern void sound_set_format(u8 format);

/* set the rate */
extern void sound_set_rate(u32 rate);

/* turn on/off sound 1 = on 0 = off */
extern void sound_set_power(u8 state);

/* start playing */
extern void sound_play(void);

/* stop playing */
extern void sound_stop(void);
