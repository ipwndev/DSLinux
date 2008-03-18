/* OS/2 DART support for EsounD
   17-12-99: Andrew Zabolotny <bit@eltech.ru>
*/

#define INCL_DOS
#define INCL_OS2MM
#include <os2.h>
/* Prevent a warning: PPFN redefined */
#define PPFN _PPFN
#include <os2me.h>
#undef PPFN

/* Define the macro below to grab audio device into exclusive use */
#undef DART_EXCLUSIVE
/* Define the macro below to rise our priority to timecritical */
#undef DART_TIMECRITICAL

/* Allocate 4 audio buffers for smooth playback (should be power of two) */
#define BUFFER_COUNT		8
#define BUFFER_COUNT_MASK	(BUFFER_COUNT - 1)

static MCI_MIX_BUFFER MixBuffers[BUFFER_COUNT];
static MCI_MIXSETUP_PARMS MixSetupParms;
static MCI_BUFFER_PARMS BufferParms;
static ULONG DeviceIndex = 0;		/* use default waveaudio device	*/
static ULONG DeviceHandle = 0;
static int BufferSize;			/* Size	of one audio buffer */
static volatile int MixBufferIndex;	/* Next	free buffer index */
static volatile int MixBufferCount;	/* Number of buffers to	play */
static HMTX MixBufferSem;		/* Mix buffer semaphore	*/

/*******************************************************************/
/* display available devices */
#define ARCH_esd_audio_devices
const char * esd_audio_devices()
{
    return "(card index: 0:default, 1, 2, ...)";
}

static int __audio_start ()
{
#ifdef DART_EXCLUSIVE
    MCI_GENERIC_PARMS GenericParms;
    /* grab exclusive rights to device instance (not entire device) */
    GenericParms.hwndCallback = 0;  /* Not needed, so set to 0 */
    if (mciSendCommand(DeviceHandle, MCI_ACQUIREDEVICE, MCI_EXCLUSIVE_INSTANCE,
                       (PVOID) &GenericParms, 0) != MCIERR_SUCCESS) {
        fprintf (stderr, "Cannot get exclusive access to the audio device!\n");
        return -1;
    }
#endif

    return 0;
}

static int __audio_stop ()
{
    MCI_GENERIC_PARMS GenericParms;
    GenericParms.hwndCallback = 0;
    mciSendCommand (DeviceHandle, MCI_STOP, MCI_WAIT, (PVOID) &GenericParms, 0);

#ifdef DART_EXCLUSIVE
    GenericParms.hwndCallback = 0;  /* Not needed, so set to 0 */
    if (mciSendCommand(DeviceHandle, MCI_RELEASEDEVICE, MCI_RETURN_RESOURCE,
                       (PVOID) &GenericParms, 0) != MCIERR_SUCCESS) {
        fprintf (stderr, "Cannot release exclusive access to the audio device!\n");
        return -1;
    }
#endif
    MixBufferCount = 0;
}

/* Buffer update thread (created and called by DART)
   This is a high-priority thread used to compute and update the audio stream,
   automatically created by the DART subsystem. We compute the next audio
   buffer and feed it to the waveaudio device. */
static LONG APIENTRY Dart_UpdateBuffers(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG ulFlags)
{
    int flags = (ulFlags & ~MIX_STREAM_ERROR);

    /* sanity check */
    if (!pBuffer)
        return TRUE;

    /* if we have finished a buffer, we're ready to play a new one */
    if ((flags == MIX_WRITE_COMPLETE)
     || (flags == MIX_READ_COMPLETE)) {
        if (MixBufferCount) {
            DosRequestMutexSem (MixBufferSem, SEM_INDEFINITE_WAIT);
            MixBufferCount--;
            DosReleaseMutexSem (MixBufferSem);

            if (!MixBufferCount)
                /* If there are no buffers left, stop playing/recording */
                __audio_stop ();
        }
    }
    return TRUE;
}

/*******************************************************************/
/* open the audio device */
#define ARCH_esd_audio_open
int esd_audio_open()
{
    int bit, recording = ((esd_audio_format & ESD_MASK_FUNC) == ESD_RECORD);
    MCI_AMP_OPEN_PARMS AmpOpenParms;
    MCI_GENERIC_PARMS GenericParms;

    /* Recording does not work yet */
    if (recording)
        return -1;

    /* Create the mutex semaphore */
    DosCreateMutexSem (NULL, &MixBufferSem, 0, FALSE);

    DeviceIndex = esd_audio_device ? atoi (esd_audio_device) : 0;
    MixBufferIndex = 0;
    MixBufferCount = 0;

    MixBuffers[0].pBuffer = NULL;   /* marker */
    DeviceHandle = 0;
    memset(&GenericParms, 0, sizeof(MCI_GENERIC_PARMS));

    /* open AMP device */
    memset(&AmpOpenParms, 0, sizeof(MCI_AMP_OPEN_PARMS));
    AmpOpenParms.usDeviceID = 0;
    AmpOpenParms.pszDeviceType = (PSZ) MAKEULONG(MCI_DEVTYPE_AUDIO_AMPMIX,
        (USHORT) DeviceIndex);

    if (mciSendCommand(0, MCI_OPEN, MCI_WAIT | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE_ID,
            (PVOID) & AmpOpenParms, 0) != MCIERR_SUCCESS) {
        fprintf (stderr, "Error opening audio device %d\n", DeviceIndex);
        return -1;
    }

    DeviceHandle = AmpOpenParms.usDeviceID;

    /* setup playback parameters */
    memset(&MixSetupParms, 0, sizeof(MCI_MIXSETUP_PARMS));
    MixSetupParms.ulBitsPerSample = (esd_audio_format & ESD_BITS16) ? 16 : 8;
    MixSetupParms.ulFormatTag = MCI_WAVE_FORMAT_PCM;
    MixSetupParms.ulSamplesPerSec = esd_audio_rate;
    MixSetupParms.ulChannels = (esd_audio_format & ESD_STEREO) ? 2 : 1;
    MixSetupParms.ulFormatMode = recording ? MCI_RECORD : MCI_PLAY;
    MixSetupParms.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
    MixSetupParms.pmixEvent = Dart_UpdateBuffers;

    if (mciSendCommand(DeviceHandle, MCI_MIXSETUP,
            MCI_WAIT | MCI_MIXSETUP_INIT,
            (PVOID) &MixSetupParms, 0) != MCIERR_SUCCESS) {
        fprintf (stderr, "Audio device %d does not support playing %d bits %dKHz %s\n",
            DeviceIndex, MixSetupParms.ulBitsPerSample, esd_audio_rate / 1000,
            esd_audio_format & ESD_STEREO ? "stereo" : "mono");
        return -1;
    }

    /* We want audio buffers of a reasonable size, for about 1/16" */
    BufferSize = esd_audio_rate >> 4;
    if (esd_audio_format & ESD_STEREO)
        BufferSize <<= 1;
    if (esd_audio_format & ESD_BITS16)
        BufferSize <<= 1;
    for (bit = 15; bit >= 12; bit--)
        if (BufferSize & (1 << bit))
            break;
    BufferSize = (1 << bit);
    /* make sure buffer is not greater than 64 Kb, as DART can't handle this
       situation. */
    if (BufferSize > 65536)
        BufferSize = 65536;

    BufferParms.ulStructLength = sizeof(BufferParms);
    BufferParms.ulNumBuffers = BUFFER_COUNT;
    BufferParms.ulBufferSize = BufferSize;
    BufferParms.pBufList = MixBuffers;

    if (mciSendCommand(DeviceHandle, MCI_BUFFER,
            MCI_WAIT | MCI_ALLOCATE_MEMORY,
            (PVOID) &BufferParms, 0) != MCIERR_SUCCESS) {
        mciSendCommand(DeviceHandle, MCI_CLOSE, MCI_WAIT, (PVOID) &GenericParms, 0);
        fprintf (stderr, "Error while trying to allocate %d playback buffers of %dK\n",
            BufferParms.ulNumBuffers, BufferParms.ulBufferSize / 1024);
        return -1;
    }

    if (recording) {
        MCI_CONNECTOR_PARMS ConnectorParms;
        MCI_AMP_SET_PARMS AmpSetParms ;

        /* Set the connector to 'line in' */
        memset (&ConnectorParms, 0, sizeof (ConnectorParms));
        ConnectorParms.ulConnectorType = MCI_LINE_IN_CONNECTOR;
        mciSendCommand (DeviceHandle, MCI_CONNECTOR,
             MCI_WAIT | MCI_ENABLE_CONNECTOR | MCI_CONNECTOR_TYPE,
             &ConnectorParms, 0);

        /* Allow the user to hear what is being recorded
           by turning the monitor on */
        memset (&AmpSetParms, 0, sizeof (AmpSetParms));
        AmpSetParms.ulItem = MCI_AMP_SET_MONITOR;
        mciSendCommand (DeviceHandle, MCI_SET,
            MCI_WAIT | MCI_SET_ON | MCI_SET_ITEM,
            &AmpSetParms, 0);
    }

#ifdef DART_TIMECRITICAL
    /* Rise the priority of the program since we're using small buffers */
    DosSetPriority (PRTYS_THREAD, PRTYC_TIMECRITICAL, 0, 0);
#endif

    /* We don't really have a handle, so return a number >0 */
    return 1;
}

/*******************************************************************/
/* close the audio device */
#define ARCH_esd_audio_close
void esd_audio_close()
{
    MCI_GENERIC_PARMS GenericParms;

    __audio_stop ();

    if (MixBuffers[0].pBuffer) {
        mciSendCommand(DeviceHandle, MCI_BUFFER, MCI_WAIT | MCI_DEALLOCATE_MEMORY,
                       &BufferParms, 0);
        MixBuffers[0].pBuffer = NULL;
    }
    if (DeviceHandle) {
        mciSendCommand(DeviceHandle, MCI_CLOSE, MCI_WAIT, (PVOID) &GenericParms, 0);
        DeviceHandle = 0;
    }
    DosCloseMutexSem (MixBufferSem);

#ifdef DART_TIMECRITICAL
    /* Drop the priority back */
    DosSetPriority (PRTYS_THREAD, PRTYC_REGULAR, 0, 0);
#endif
}

/*******************************************************************/
/* make the sound device quiet for a while */
#define ARCH_esd_audio_pause
void esd_audio_pause()
{
    /* Wait until all buffers are free */
    while (MixBufferCount)
        DosSleep (0);
    return;
}

#define ARCH_esd_audio_write
/*******************************************************************/
/* dump a buffer to the sound device */
int esd_audio_write( void *buffer, int buf_size )
{
    int i, bytes_played = 0;
    MCI_GENERIC_PARMS GenericParms;
    PMCI_MIX_BUFFER pbuf;

    while (buf_size) {
        /* Wait for buffers to free */
        while (MixBufferCount >= BUFFER_COUNT)
            DosSleep (0);

        /* fill next available buffer */
        pbuf = &MixBuffers[MixBufferIndex];
        MixBufferIndex = (MixBufferIndex + 1) & BUFFER_COUNT_MASK;

        i = buf_size;
        if (i > BufferSize)
            i = BufferSize;
        pbuf->ulBufferLength = i;
        memcpy (pbuf->pBuffer, buffer, i);
        buffer = ((char *)buffer) + i;
        bytes_played += i;
        buf_size -= i;

        /* Grab the mix buffers into exclusive use */
        if (DosRequestMutexSem (MixBufferSem, SEM_INDEFINITE_WAIT))
            break;
        MixBufferCount++;
        /* Release the semaphore */
        DosReleaseMutexSem (MixBufferSem);

        /* If the audio is stopped, grab it and start playing */
        if (MixBufferCount == 1)
            if (__audio_start () < 0)
                break;

        /* Output the buffer */
        MixSetupParms.pmixWrite(MixSetupParms.ulMixHandle, pbuf, 1);
    } /* endwhile */

    return bytes_played;
}

#define ARCH_esd_audio_read
/*******************************************************************/
/* read a chunk from the sound device */
int esd_audio_read( void *buffer, int buf_size )
{
    /* The following does not work yet - for unknown reason */
    int i, bytes_sampled = 0;
    MCI_GENERIC_PARMS GenericParms;
    PMCI_MIX_BUFFER pbuf;

    while (buf_size) {
        /* Wait for buffers to free */
        while (MixBufferCount >= BUFFER_COUNT)
            DosSleep (0);

        /* read into next available buffer */
        pbuf = &MixBuffers[MixBufferIndex];
        MixBufferIndex = (MixBufferIndex + 1) & BUFFER_COUNT_MASK;

        i = buf_size;
        if (i > BufferSize)
            i = BufferSize;
        pbuf->ulBufferLength = i;

        /* Grab the mix buffers into exclusive use */
        if (DosRequestMutexSem (MixBufferSem, SEM_INDEFINITE_WAIT))
            break;
        MixBufferCount++;
        /* Release the semaphore */
        DosReleaseMutexSem (MixBufferSem);

        /* If the audio is stopped, grab it and start playing */
        if (MixBufferCount == 1)
            if (__audio_start () < 0)
                break;

        /* Read into the buffer */
        MixSetupParms.pmixRead(MixSetupParms.ulMixHandle, pbuf, 1);

        buffer = ((char *)buffer) + i;
        bytes_sampled += i;
        buf_size -= i;
    } /* endwhile */

    while (MixBufferCount)
        DosSleep (0);

    return bytes_sampled;
}

#define ARCH_esd_audio_flush
/*******************************************************************/
/* flush the audio buffer */
void esd_audio_flush()
{
    /* Wait until all buffers are free */
    while (MixBufferCount)
        DosSleep (0);
}
