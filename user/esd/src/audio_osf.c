/* Driver for Compaq Tru64 UNIX using MMS */

#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <mme/mme_api.h>
#include <errno.h>
#include "esd.h"
#include "config.h"

static HWAVEOUT hWaveOut = NULL;
static HWAVEIN hWaveIn = NULL;
static unsigned long nWaiting = 0;
static unsigned int sleep_delay = 50000;
static void *record_buf = NULL;
static unsigned long record_out = 0;

/*
 * Sometime we run out of memory because too much data has been accepted by
 * waveOutWrite but not yet played.  Most drivers don't let the data get this big
 * because they are limited by the normal I/O buffers (fopen, fsync, etc.).
 * This also has the affect of making esd_audio_flush() take a long time.
 * To avoid running out of memory unnecessarily, we impose a limit on how much
 * can be buffered at one time.
 */
#if !defined(ESD_OUT_BUFFER_PCT)
#define ESD_OUT_BUFFER_PCT 0.5
#endif
static unsigned long nWaitingMax = (unsigned long) -1L;
void
reduce_buffered_data()
{
    do {
	if (mmeCheckForCallbacks() == TRUE)
	    mmeProcessCallbacks();
	if (nWaiting > nWaitingMax)
	    usleep(sleep_delay);
    } while (nWaiting > nWaitingMax);
}

void CALLBACK waveInCallbackFunction(HWAVEIN hWaveIn, UINT uMsg,
				     DWORD dwInstance,
				     LPARAM lParam1, LPARAM lParam2);
void CALLBACK waveOutCallbackFunction(HWAVEOUT hWaveOut, UINT uMsg,
				      DWORD dwInstance,
				      LPARAM lParam1, LPARAM lParam2);

void
_mmeFreeMem(void *ptr, char *name)
{
    if (ptr == NULL)
	return;
    if (mmeFreeMem(ptr) != TRUE)
	fprintf(stderr, "esd: warning: unable to free %s\n", name);
}
#define mmeFreeMem(x) _mmeFreeMem(x,#x)

void
mmePrintError(int in, UINT uError)
{
    MMRESULT result;
    LPSTR lpText;
    UINT uSize = MAXERRORLENGTH;

    lpText = mmeAllocMem(uSize);
    if (lpText == NULL)
	result = ~MMSYSERR_NOERROR;
    else if (in)
	result = waveInGetErrorText(uError, lpText, uSize);
    else
	result = waveOutGetErrorText(uError, lpText, uSize);
    if (result != MMSYSERR_NOERROR) {
	fprintf(stderr, "esd: MME error #%u occurred\n", uError);
	return;
    }
    fprintf(stderr, "esd: %s\n", lpText);
    mmeFreeMem(lpText);
}
#define mmePrintInError(x)	mmePrintError(1,x)
#define mmePrintOutError(x)	mmePrintError(0,x)

void *
_mmeAllocMem(size_t size)
{
    void *ptr;

    if((ptr = mmeAllocMem(size)) == NULL)
	fprintf(stderr, "esd: not enough room to allocated %lu bytes!\n", size);
    return ptr;
}
#define mmeAllocMem(x)	_mmeAllocMem(x)

#define ARCH_esd_audio_devices
const char *esd_audio_devices()
{
    static char *str = NULL;
    UINT nDevs, i;
    LPWAVEOUTCAPS lpOutCaps = NULL;
    LPWAVEINCAPS lpInCaps = NULL;
    MMRESULT result;
    size_t str_size;
    int record = 0;
    static const char *unknown = "(unknown)";
    static const char *none = "(none)";

    if (str != NULL)
	free(str);
    if (esd_audio_format & ESD_MASK_MODE == ESD_STREAM &&
	esd_audio_format & ESD_MASK_FUNC == ESD_RECORD)
	record = 1;
    if (record)
	nDevs = waveInGetNumDevs();
    else
	nDevs = waveOutGetNumDevs();
    if (nDevs == 0)
	return none;
    if (record)
	lpInCaps = mmeAllocMem(sizeof(WAVEINCAPS));
    else
	lpOutCaps = mmeAllocMem(sizeof(WAVEOUTCAPS));
    if (lpInCaps == NULL && lpOutCaps == NULL)
	return unknown;
    str_size = nDevs * (MAXPNAMELEN + 15);
    if ((str = malloc (str_size)) == NULL) {
	mmeFreeMem(lpInCaps);
	mmeFreeMem(lpOutCaps);
	return unknown;
    }
    str[0] = '\0';
    for (i=0; i<nDevs; i++) {
	if (record)
	    result = waveInGetDevCaps(i, lpInCaps, sizeof(WAVEINCAPS));
	else
	    result = waveOutGetDevCaps(i, lpOutCaps, sizeof(WAVEOUTCAPS));
	if (result != MMSYSERR_NOERROR) {
    	    mmePrintError(record, result);
	    continue;
	}
	snprintf(str, str_size, "%s%s%d (%s)", str, str[0] ? ", " : "",
		 i, record ? lpInCaps->szPname : lpOutCaps->szPname);
    }
    mmeFreeMem(lpInCaps);
    mmeFreeMem(lpOutCaps);
    return str;
}

#define ARCH_esd_audio_open
int esd_audio_open()
{
    UINT uDeviceID;
    LPWAVEFORMATEX waveFormat;
    MMRESULT result;
    int record = 0;

    switch (esd_audio_format & ESD_MASK_MODE) {
	case ESD_STREAM:
	    switch (esd_audio_format & ESD_MASK_FUNC) {
		case ESD_MONITOR:
		case ESD_PLAY:
		    break;
		case ESD_RECORD:
		    record = 1;
		    break;
		default:
		    fprintf(stderr, "esd: desired functionality unknown\n");
		    return -1;
	    }
	    break;
	case ESD_SAMPLE:
	    switch (esd_audio_format & ESD_MASK_FUNC) {
		case ESD_LOOP:
		    break;
		case ESD_STOP:
		default:
		    fprintf(stderr, "esd: desired functionality unknown\n");
		    return -1;
	    }
	    break;
	case ESD_ADPCM:
	    fprintf(stderr, "esd: desired functionality unimplemented\n");
	    return -1;
	default:
	    fprintf(stderr, "esd: desired functionality unknown\n");
	    return -1;
    }

    if ((record && hWaveIn != NULL) || hWaveOut != NULL) {
	fprintf(stderr, "esd: already opened (multiples not allowed)\n");
	return -1;
    }
    if (esd_audio_device == NULL)
	uDeviceID = WAVE_MAPPER;
    else if(sscanf(esd_audio_device, "%u", &uDeviceID) != 1) {
	fprintf(stderr, "esd: invalid device ID specified\n");
	return -1;
    }

    if ((waveFormat = mmeAllocMem(sizeof(WAVEFORMATEX))) == NULL)
	return -1;
    waveFormat->wFormatTag = WAVE_FORMAT_PCM;
    switch (esd_audio_format & ESD_MASK_BITS) {
	case ESD_BITS8:
	    waveFormat->wBitsPerSample = 8;
	    break;
	case ESD_BITS16:
	    waveFormat->wBitsPerSample = 16;
	    break;
	default:
	    fprintf(stderr, "esd: unknown bits per sample\n");
	    return -1;
    }
    switch (esd_audio_format & ESD_MASK_CHAN) {
	case ESD_MONO:
	    waveFormat->nChannels = 1;
	    break;
	case ESD_STEREO:
	    waveFormat->nChannels = 2;
	    break;
	default:
	    fprintf(stderr, "esd: unknown channel setup\n");
	    return -1;
    }
    waveFormat->nSamplesPerSec = esd_audio_rate;
    nWaitingMax = esd_audio_rate * ESD_OUT_BUFFER_PCT;
    waveFormat->nAvgBytesPerSec = waveFormat->nSamplesPerSec *
	waveFormat->nChannels * waveFormat->wBitsPerSample/8;
    waveFormat->nBlockAlign = waveFormat->nChannels * waveFormat->wBitsPerSample/8;

    if (record)
	result = waveInOpen(&hWaveIn, uDeviceID, (void *)waveFormat,
			    waveInCallbackFunction, NULL, CALLBACK_FUNCTION);
    else
	result = waveOutOpen(&hWaveOut, uDeviceID, (void *)waveFormat,
			     waveOutCallbackFunction, NULL,
			     CALLBACK_FUNCTION | WAVE_OPEN_SHAREABLE);
    mmeFreeMem(waveFormat);
    if (result != MMSYSERR_NOERROR) {
	fprintf(stderr, "esd: unable to open audio device\n");
	mmePrintError(record, result);
	if (result == MMSYSERR_BADDEVICEID)
	    fprintf(stderr, "esd: valid devices: %s\n", esd_audio_devices());
	return -1;
    }
    if (record)
	result = waveInStart(hWaveIn);
    else
	result = waveOutReset(hWaveOut);
    if (result != MMSYSERR_NOERROR)
	mmePrintError(record, result);
    return 0;
}

#define ARCH_esd_audio_close
void esd_audio_close()
{
    MMRESULT result;

    if (hWaveIn != NULL) {
	result = waveInStop(hWaveIn);
	if (result != MMSYSERR_NOERROR)
	    result = waveInReset(hWaveIn);
	if (result != MMSYSERR_NOERROR)
	    result = waveInClose(hWaveIn);
	if (result != MMSYSERR_NOERROR)
	    hWaveIn = NULL;
	if (result != MMSYSERR_NOERROR)
	    mmePrintOutError(result);
    }
    if (hWaveOut != NULL) {
	esd_audio_flush();
	result = waveOutReset(hWaveOut);
	if (result != MMSYSERR_NOERROR)
	    mmePrintOutError(result);
	result = waveOutClose(hWaveOut);
	if (result != MMSYSERR_NOERROR)
	    hWaveOut = NULL;
	if (result != MMSYSERR_NOERROR)
	    mmePrintOutError(result);
    }
}

#define ARCH_esd_audio_flush
void esd_audio_flush()
{
    if (hWaveOut == NULL)
	return;
    if (hWaveIn == NULL) ;
    do {
	if (mmeCheckForCallbacks() == TRUE)
	    mmeProcessCallbacks();
	if (nWaiting > 0)
	    usleep(sleep_delay);
    } while (nWaiting > 0);
    return;
}

#define ARCH_esd_audio_write
int esd_audio_write(void *buffer, int buf_size)
{
    MMRESULT result;
    LPWAVEHDR lpWaveOutHdr;
    void *b;

    if (hWaveOut == NULL) {
	fprintf(stderr, "esd: output device not open!\n");
	return -1;
    }

    /* Process pending data first */
    if (mmeCheckForCallbacks() == TRUE)
	mmeProcessCallbacks();
    if (nWaiting > nWaitingMax)
	reduce_buffered_data();

    /* Make a copy of the buffer */
    if ((b = mmeAllocBuffer((size_t)buf_size)) == NULL)
	return -1;
    bcopy(buffer, b, buf_size);
    
    if ((lpWaveOutHdr = mmeAllocMem(sizeof(WAVEHDR))) == NULL)
	return -1;
    lpWaveOutHdr->dwBufferLength = buf_size;
    lpWaveOutHdr->lpData = b;

    result = waveOutWrite(hWaveOut, lpWaveOutHdr, sizeof(WAVEHDR));
    mmeFreeMem(lpWaveOutHdr);
    if (result != MMSYSERR_NOERROR) {
	mmePrintOutError(result);
	return -1;
    }
    nWaiting += buf_size;

    /* Take the chance to process any pending data */
    if (mmeCheckForCallbacks() == TRUE)
	mmeProcessCallbacks();
    if (nWaiting > nWaitingMax)
	reduce_buffered_data();

    return 0;
}

#define ARCH_esd_audio_read
int esd_audio_read(void *buffer, int buf_size) {
    MMRESULT result;
    LPWAVEHDR lpWaveInHdr;

    if (hWaveIn == NULL) {
	fprintf(stderr, "esd: input device never opened!\n");
	return 0;
    }
    if ((lpWaveInHdr = mmeAllocMem(sizeof(WAVEHDR))) == NULL)
	return 0;
    lpWaveInHdr->dwBufferLength = (UINT) buf_size;
    if ((lpWaveInHdr->lpData = mmeAllocMem(lpWaveInHdr->dwBufferLength)) == NULL) {
	mmeFreeMem(lpWaveInHdr);
	return 0;
    }
    result = waveInAddBuffer(hWaveIn, lpWaveInHdr, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
	mmePrintInError(result);
	return 0;
    }
    mmeFreeMem(lpWaveInHdr);
    record_buf = buffer;
    record_out = 0;
    do {
	if (mmeCheckForCallbacks() == TRUE)
	    mmeProcessCallbacks();
	if (record_out == 0)
	    usleep(10000);	/* Use a fairly low latency */
    } while (record_out == 0);
    return record_out;
}

#define ARCH_esd_audio_pause
void esd_audio_pause()
{
#if 0
    /* Is this really right, or do they want a mute ? */
    /* How is the stream "unpaused" anyway? */
    MMRESULT result;
    //fprintf(stderr, "In esd_audio_pause()\n");
    if (hWaveOut == NULL)
	return;
    result = waveOutPause(hWaveOut);
    if (result != MMSYSERR_NOERROR) {
	mmePrintOutError(result);
	return;
    }
#endif
    return;
}

void CALLBACK waveOutCallbackFunction(HWAVEOUT hWaveOut, UINT uMsg,
				      DWORD dwInstance,
				      LPARAM lParam1, LPARAM lParam2)
{
    switch (uMsg) {
	case WOM_CLOSE:
	case WOM_OPEN:
	    break;
	case WOM_DONE:
	    nWaiting -= ((LPWAVEHDR)lParam1)->dwBufferLength;
	    mmeFreeBuffer(((LPWAVEHDR)lParam1)->lpData);
	    break;
	default:
	    fprintf(stderr, "esd: unknown message #%u given to callback function\n", uMsg);
	    break;
    }
}

void CALLBACK waveInCallbackFunction(HWAVEIN hWaveIn, UINT uMsg,
				     DWORD dwInstance,
				     LPARAM lParam1, LPARAM lParam2)
{
    switch (uMsg) {
	case WIM_CLOSE:
	    break;
	case WIM_OPEN:
	    break;
	case WIM_DATA:
	    bcopy(((LPWAVEHDR)lParam1)->lpData, record_buf,
		  ((LPWAVEHDR)lParam1)->dwBufferLength);
	    record_out = ((LPWAVEHDR)lParam1)->dwBufferLength;
	    mmeFreeBuffer(((LPWAVEHDR)lParam1)->lpData);
	    break;
	default:
	    fprintf(stderr, "esd: unknown message #%u given to callback function\n", uMsg);
	    break;
    }
}
