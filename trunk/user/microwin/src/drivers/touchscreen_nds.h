#ifndef TS_DEVICE
#define TS_DEVICE "Nintendo DS"

#define TS_DEVICE_FILE "/dev/touch"

struct ts_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	long value;
};

#endif

