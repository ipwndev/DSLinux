/* Turn the microphone on */
void mic_on(void);

/* Turn the microphone off */
void mic_off(void);

/* Set the mic power */
void mic_set_power(u8 power);

/* set the buffer for data to go. */
void mic_set_address(u32 buffer);

/* set the buffer size */
void mic_set_size(u32 size);

/* set the data format */
void mic_set_format(u8 format);

/* set the period size */
void mic_set_period_size(u32 size);

/* Set the sample rate */
void mic_set_rate(u32 rate);

/* Start Recording data */
void mic_start(void);

/* Stop Recording data */
int mic_stop(void);

/* Timer handler */
void mic_timer_handler(void);
