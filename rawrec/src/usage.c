/* Copyright (C) 2006  Britton Leo Kerin, see copyright. */

/* This function prints information about the syntax of the program
   options and arguments and thier correct usage. */

#include <stdio.h> 

#include "rawrec.h"

void usage( void )
{
printf(
"\nUsage: %s [option]... [file]\n\n"
, progname); 

printf(
"  -B SIZE,                       size of the main ring buffer\n"
);
printf(
"  --ring-buffer-size=SIZE\n\n"
);

printf(
"  -c CHANNELS,                   number of channels to record\n"
);
printf(
"  --channels=CHANNELS            ( 1 ==> mono, 2 ==> stereo )\n\n"
);

printf(
"  -d DEVICE,                     dsp device name (needed only if\n"
);
printf(
"  --audio-device=DEVICE          not /dev/dsp)\n\n"
);
                                    
printf(
"  -e SECS,                       silent seconds to record at end\n"
);
printf(
"  --end-record-time=SECS         (rawrec only)\n\n"
);

printf(
"  -E SMPS,                       silent samples to record at end\n"
); 
printf(
"  --end-record-samples=SAMPS     (rawrec only)\n\n"
);

printf(
"  -f FMT,                        sample format, ex. u8 (unsigned 8 bit),\n"
);
printf(
"  --sample-format=FMT            s16_le (signed 16 bit little endian)\n\n"
);

printf(
"  -g FRAG_SZ,                    kernel audio buffer fragment size (must\n"
);
printf(
"  --fragment-size=FRAG_SZ        be a power of two)\n\n"
);
                                   
printf(
"  -h,                            hold /dev/dsp (or argument to -d\n"
);
printf(
"  --hold-audio-device            even if dummy dsp is in use\n\n"
);

printf(
"  -j SECS,                       time to skip at start (rawplay\n"
);
printf(
"  --start-jump-time=SECS         only)\n\n"
);

printf(
"  -J SAMPS,                      samples to skip at start (rawplay\n"
);
printf(
"  --start-jump-samples=SAMPS     only)\n\n"
);

printf(
"  -p SECS,                       seconds to pause before beginning\n"
);
printf(
"  --start-pause-time=SECS        execution\n\n"
);

printf(
"  -P SAMPS,                      samples to pause before beginning\n"
);
printf(
"  --start_pause_samples=SAMPS    execution\n\n"
);

printf(
"  -r SECS,                       silent seconds to record at start\n"        
);
printf(
"  --start-record-time=SECS       (rawrec only)\n\n"
);

printf(
"  -R SAMPS,                      silent samples to record at start\n"
);
printf(
"  --start-record-samples=SAMPS   (rawrec only)\n\n"
);

printf( 
"  -s SPEED,                      sampling rate ( = 2 * maximum\n"
);
printf(
"  --sampling-rate=SPEED          representable frequency)\n\n"
);

printf(
"  -t SECS,                       time to record or play, excluding\n" 
);
printf(
"  --time-limit=SECS              silence or skipped sections\n\n"
);

printf(
"  -T SAMPS,                      samples to record or play, including\n" 
);
printf(
"  --sample-limit=SAMPS           silence or skipped sections\n\n"
);

printf(
"  -v,                            verbose operation\n"
);
printf( 
"  --verbose\n\n"
);

printf(
"  -z SECS,                       seconds to pause at end of execution\n"
);
printf(
"  --end-pause-time=SECS\n\n"
);

printf(
"  -Z SAMPS,                      samples to pause at end of execution\n"
);
printf(
"  --end-pause-samples=SAMPS\n\n"
);

printf(
"  -?,                            display this help message\n"
);
printf(
"  --help\n"
);

  return;
}
