
/*
 * for CoreAudio
 *
 * If the playback/recording sound device is not stereo and its sample
 * size is not float (32 bit), this will not work.
 *
 * Please let me know if there is such a case.
 *
 * Shawn Hsiao <phsiao@mac.com>
 * Some enhancement by Masanori Sekino <msek@users.sourceforge.net>
 */

#include <CoreAudio/CoreAudio.h>
#include <limits.h>
#include <pthread.h>

#define BUF_SIZE ESD_BUF_SIZE

static AudioDeviceID gOutputDeviceID, gInputDeviceID;

static float OutputDataBuf[BUF_SIZE];
static float InputDataBuf[BUF_SIZE];
static int OutputWroteSamples = 0;
static int InputReadSamples = 0;

static pthread_mutex_t mutexOutput, mutexInput;
static pthread_cond_t condOutput, condInput;

static int audioPlaybackStarted = 0, audioRecordStarted = 0;
static int coreaudio_has_output_device = 0;
static int coreaudio_has_input_device = 0;
static int num_ca_input_channel = 0;
static int coreaudio_init = 0;

OSStatus PlaybackIOProc(AudioDeviceID inDevice,
			const AudioTimeStamp *inNow,
			const AudioBufferList *inInputData,
			const AudioTimeStamp *inInputTime,
			AudioBufferList *outOutputData,
			const AudioTimeStamp *inOutputTime,
			void *inClientData)
{
  float *bufPtr = outOutputData->mBuffers[0].mData;
  int i;

  pthread_mutex_lock(&mutexOutput);

  for (i = 0; i < OutputWroteSamples; i++)
    bufPtr[i] = OutputDataBuf[i];
  for ( ; i < BUF_SIZE; i++)
    bufPtr[i] = 0;
  OutputWroteSamples = 0;

  pthread_mutex_unlock(&mutexOutput);
  pthread_cond_signal(&condOutput);

  return (kAudioHardwareNoError);
}

OSStatus RecordIOProc(AudioDeviceID inDevice,
		      const AudioTimeStamp *inNow,
		      const AudioBufferList *inInputData,
		      const AudioTimeStamp *inInputTime,
		      AudioBufferList *outOutputData,
		      const AudioTimeStamp *inOutputTime,
		      void *inClientData)
{
  float *bufPtr = inInputData->mBuffers[0].mData;
  int i;

  pthread_mutex_lock(&mutexInput);

  if (num_ca_input_channel == 2) {
    for (i = 0; i < BUF_SIZE; i++)
      InputDataBuf[i] = bufPtr[i];
  }
  else {
    for (i = 0; i < BUF_SIZE/2; i++)
      InputDataBuf[2*i] = bufPtr[i];
  }
  InputReadSamples = 0;

  pthread_mutex_unlock(&mutexInput);
  pthread_cond_signal(&condInput);

  return (kAudioHardwareNoError);
}

#define ARCH_esd_audio_devices
const char *esd_audio_devices()
{
    return "coreaudio API only";
}

/*
 * This is called to reset the device status.
 * Returns -2 to indicate the device failed to initialize;
 * returns  -1 means any of rate/size/{mono,stereo} mismatched.
 */
#define ARCH_esd_audio_open
int esd_audio_open()
{
#define LEN_DEVICE_NAME 128
  OSStatus status;
  UInt32 propertySize, bufferByteCount;
  char deviceName[LEN_DEVICE_NAME];
  struct AudioStreamBasicDescription streamDesc;
  int rval;

  /*
   * We only need to do this once, the rest are taken cared by
   * disable/enable calback.
   */
  if (coreaudio_init) {
    return (0);
  }

  /********************** playback section ***************************/
  /* get default output device */
  propertySize = sizeof(gOutputDeviceID);
  status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
	                            &propertySize,
				    &gOutputDeviceID);
  if (status) {
    fprintf(stderr, "get default output device failed, status = %d\n",
	    (int)status);
    return (-2);
  }

  if (gOutputDeviceID != kAudioDeviceUnknown) {
    /* got default output device */
    coreaudio_has_output_device = 1;

    /* get output device name */
    propertySize = sizeof(char)*LEN_DEVICE_NAME;
    status = AudioDeviceGetProperty(gOutputDeviceID,
				    1,
				    0,
				    kAudioDevicePropertyDeviceName,
				    &propertySize,
				    deviceName);
    if (status) {
      fprintf(stderr, "get device name failed, status = %d\n",
	      (int)status);
      return (-2);
    }

    /* get output format */
    propertySize = sizeof(struct AudioStreamBasicDescription);
    status = AudioDeviceGetProperty(gOutputDeviceID,
				    1,
				    0,
				    kAudioDevicePropertyStreamFormat,
				    &propertySize,
				    &streamDesc);
    if (status) {
      fprintf(stderr, "get device property failed, status = %d\n",
	      (int)status);
      return (-2);
    }

    if ((streamDesc.mSampleRate != 44100.0) ||
        (streamDesc.mFormatID != kAudioFormatLinearPCM) ||
       !(streamDesc.mFormatFlags & kLinearPCMFormatFlagIsFloat) ||
        (streamDesc.mChannelsPerFrame != 2))
    {
      fprintf (stderr, "unsupported device.\n");
      return (-2);
    }

    /* set buffer size */
    bufferByteCount = BUF_SIZE * sizeof(float);
    propertySize = sizeof(bufferByteCount);
    status = AudioDeviceSetProperty(gOutputDeviceID,
                                    0,
                                    0,
                                    0,
                                    kAudioDevicePropertyBufferSize,
                                    propertySize,
                                    &bufferByteCount);
    if (status) {
      fprintf(stderr, "set device property failed, status = %d\n",
              (int)status);
    }

    fprintf(stderr, "using device %s for output:\n", deviceName);
    fprintf(stderr, "\twith sample rate %f, %ld channels and %ld-bit sample\n",
	    streamDesc.mSampleRate,
	    streamDesc.mChannelsPerFrame,
	    streamDesc.mBitsPerChannel);

    rval = pthread_mutex_init(&mutexOutput, NULL);
    if (rval) {
      fprintf(stderr, "mutex init failed\n");
      return (-1);
    }
    rval = pthread_cond_init(&condOutput, NULL);
    if (rval) {
      fprintf(stderr, "condition init failed\n");
      return (-1);
    }

    /* Registers PlaybackIOProc with the device without activating it. */
    status = AudioDeviceAddIOProc(gOutputDeviceID, PlaybackIOProc, (void *)1);
  }

  /********************** record section ***************************/
  /* get default input device */
  propertySize = sizeof(gInputDeviceID);
  status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
	                            &propertySize,
				    &gInputDeviceID);
  if (status) {
    fprintf(stderr, "get default input device failed, status = %d\n",
	    (int)status);
    return (-2);
  }

  if (gInputDeviceID != kAudioDeviceUnknown) {
    /* got default input device */
    coreaudio_has_input_device = 1;

    /* get input device name */
    propertySize = sizeof(char)*64;
    status = AudioDeviceGetProperty(gInputDeviceID,
				    1,
				    1,
				    kAudioDevicePropertyDeviceName,
				    &propertySize,
				    deviceName);
    if (status) {
      fprintf(stderr, "get device name failed, status = %d\n",
	      (int)status);
      return (-2);
    }

    /* get input format */
    propertySize = sizeof(struct AudioStreamBasicDescription);
    status = AudioDeviceGetProperty(gInputDeviceID,
				    1,
				    1,
				    kAudioDevicePropertyStreamFormat,
				    &propertySize,
				    &streamDesc);
    if (status) {
      fprintf(stderr, "get device property failed, status = %d\n",
	      (int)status);
      return (-2);
    }

    if ((streamDesc.mSampleRate != 44100.0) ||
        (streamDesc.mFormatID != kAudioFormatLinearPCM) ||
       !(streamDesc.mFormatFlags & kLinearPCMFormatFlagIsFloat) ||
        (streamDesc.mChannelsPerFrame != 2 &&
	 streamDesc.mChannelsPerFrame != 1))
    {
      fprintf (stderr, "unsupported device.\n");
      return (-2);
    }

    num_ca_input_channel = streamDesc.mChannelsPerFrame;

    /* set buffer size */
    bufferByteCount = BUF_SIZE / (2/num_ca_input_channel) * sizeof(float);
    propertySize = sizeof(bufferByteCount);
    status = AudioDeviceSetProperty(gInputDeviceID,
                                    0,
                                    0,
                                    1,
                                    kAudioDevicePropertyBufferSize,
                                    propertySize,
                                    &bufferByteCount);
    if (status) {
      fprintf(stderr, "set device property failed, status = %d\n",
              (int)status);
    }

    fprintf(stderr, "using device %s for input:\n", deviceName);
    fprintf(stderr, "\twith sample rate %f, %ld channels and %ld-bit sample\n",
	    streamDesc.mSampleRate,
	    streamDesc.mChannelsPerFrame,
	    streamDesc.mBitsPerChannel);

    rval = pthread_mutex_init(&mutexInput, NULL);
    if (rval) {
      fprintf(stderr, "mutex init failed\n");
      return (-1);
    }
    rval = pthread_cond_init(&condInput, NULL);
    if (rval) {
      fprintf(stderr, "condition init failed\n");
      return (-1);
    }

    /* Registers PlaybackIOProc with the device without activating it. */
    status = AudioDeviceAddIOProc(gInputDeviceID, RecordIOProc, (void *)1);
  }

  if (!coreaudio_has_output_device) {
    fprintf(stderr, "unknown output device.\n");
    return (-2);
  }
  /* Allow lack of recording device */
  /*
  if (!coreaudio_has_input_device) {
    fprintf(stderr, "unknown input device.\n");
    return (-2);
  }
  */

  /* Indicates the initialization is done */
  coreaudio_init = 1;

  esd_audio_fd = 0; /* this is meaningless anyway */

  return 0;
}

/*
 * This is called to reset the device status.
 * (before calls esd_audio_open again)
 */
#define ARCH_esd_audio_close
void esd_audio_close()
{
  OSStatus status;

  /* deactivate both of them */
  if (coreaudio_has_output_device && audioPlaybackStarted) {
    status = AudioDeviceStop(gOutputDeviceID, PlaybackIOProc);
    audioPlaybackStarted = 0;
  }

  if (coreaudio_has_input_device && audioRecordStarted) {
    status = AudioDeviceStop(gInputDeviceID, RecordIOProc);
    audioRecordStarted = 0;
  }

  return;
}


#define ARCH_esd_audio_pause
void esd_audio_pause()
{
  OSStatus status;

  if (coreaudio_has_output_device && audioPlaybackStarted) {
    status = AudioDeviceStop(gOutputDeviceID, PlaybackIOProc);
    audioPlaybackStarted = 0;
  }

  if (coreaudio_has_input_device && audioRecordStarted) {
    status = AudioDeviceStop(gInputDeviceID, RecordIOProc);
    audioRecordStarted = 0;
  }

  return;
}


#define ARCH_esd_audio_write
/*******************************************************************/
/* dump a buffer to the sound device */
int esd_audio_write( void *buffer, int buf_size )
{
  OSStatus status;
  float scale = 1.0 / SHRT_MAX;
  int remain_to_write = buf_size;

  if (!coreaudio_has_output_device)
    return -1;

  if (!audioPlaybackStarted) {
    status = AudioDeviceStart(gOutputDeviceID, PlaybackIOProc);
    audioPlaybackStarted = 1;
  }

  while (remain_to_write)
  {
    pthread_mutex_lock(&mutexOutput);

    while(OutputWroteSamples == BUF_SIZE)
      pthread_cond_wait(&condOutput, &mutexOutput);

    {
      short *src_data = (short *)buffer + (buf_size - remain_to_write) / sizeof(short);
      float *dst_data = OutputDataBuf + OutputWroteSamples;
      int src_samples = remain_to_write / sizeof(short);
      int dst_samples = BUF_SIZE - OutputWroteSamples;
      int n = (dst_samples < src_samples) ? dst_samples : src_samples;
      int i;

      for (i = 0; i < n; i++)
        dst_data[i] = scale * src_data[i];

      OutputWroteSamples += n;
      remain_to_write -= n * sizeof(short);
    }

    pthread_mutex_unlock(&mutexOutput);
  }

  return (buf_size);
}

#define ARCH_esd_audio_read
/*******************************************************************/
/* read a chunk from the sound device */
int esd_audio_read( void *buffer, int buf_size )
{
  OSStatus status;
  float scale = SHRT_MAX;
  int remain_to_read = buf_size;

  if (!coreaudio_has_input_device)
    return -1;

  if (!audioRecordStarted) {
    status = AudioDeviceStart(gInputDeviceID, RecordIOProc);
    audioRecordStarted = 1;
  }

  while (remain_to_read)
  {
    pthread_mutex_lock(&mutexInput);

    while(InputReadSamples == BUF_SIZE)
      pthread_cond_wait(&condInput, &mutexInput);

    {
      float *src_data = InputDataBuf + InputReadSamples;
      short *dst_data = (short *)buffer + (buf_size - remain_to_read) / sizeof(short);
      int src_samples = BUF_SIZE - InputReadSamples;
      int dst_samples = remain_to_read / sizeof(short);
      int n = (dst_samples < src_samples) ? dst_samples : src_samples;
      int i;

      for (i = 0; i < n; i++)
        dst_data[i] = (short)(scale * src_data[i]);

      InputReadSamples += n;
      remain_to_read -= n * sizeof(short);
    }

    pthread_mutex_unlock(&mutexInput);
  }

  return (buf_size);
}


#define ARCH_esd_audio_flush
/*******************************************************************/
/* flush the audio buffer */
void esd_audio_flush()
{
  /* nothing needed */
  return;
}
