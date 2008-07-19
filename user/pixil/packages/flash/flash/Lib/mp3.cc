
#include "swf.h"
#include <unistd.h>
#include <fcntl.h>

#ifdef RCSID
static char *rcsid = "$Id$";
#endif


//
// MP3 tables
//

static int vertab[4]={2,3,1,0};
static int freqtab[4]={44100,48000,32000};
static int ratetab[2][3][16]=
{
  {
    {  0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,  0},
    {  0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,  0},
    {  0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,  0},
  },
  {
    {  0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,  0},
    {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,  0},
    {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,  0},
  },
};


//
// The Decompressor
//

// Constructor
Mp3::Mp3(unsigned char *buffer, int len, long flags)
{
    this->src = buffer;
    this->pos = 0;
    this->len = len;

    int fd = open("/tmp/data.mp3", O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (fd >= 0) {
	if (write(fd, buffer, len) != len)
	    perror("write mp3 data");
	close(fd);
    } else {
	perror("/tmp/data.mp3");
    }
}


void
Mp3::Decompress(short *dst, long n)
{
    int iFrameCount = 0;

    pos = 0;

    for (;;) {
	// Get the MP3 frame header
	U8  hdr[4];
	for (int i=0; i<4; i++)
	    hdr[i] = src[pos++];

	// Decode the MP3 frame header
	int ver     = vertab[((hdr[1] >> 3) & 3)];
	int layer   = 3 - ((hdr[1] >> 1) & 3);
	int pad     = (hdr[2] >>1 ) & 1;
	int stereo  = ((hdr[3] >> 6) & 3) != 3;
	int freq    = 0;
	int rate    = 0;

	if (hdr[0] != 0xFF || hdr[1] < 0xE0 || ver==3 || layer != 2) {
	    // bad MP3 header
	    printf("\t\tBAD MP3 FRAME HEADER\n");
	    break;
	} else {
	    freq = freqtab[(hdr[2] >>2 ) & 3] >> ver;
	    rate = ratetab[ver ? 1 : 0][layer][(hdr[2] >> 4) & 15] * 1000;

	    if (!freq || !rate) {
		// bad MP3 header
		printf("\t\tBAD MP3 FRAME HEADER\n");
		break;
	    }
	}

	// Get the size of a decoded MP3 frame
	int iDecodedFrameSize = (576 * (stereo + 1));
	if (!ver)
	    iDecodedFrameSize *= 2;

	// Get the size of this encoded MP3 frame
	int iEncodedFrameSize = ((ver ? 72 : 144) * rate) / freq + pad - 4;

	char* ppszMpegVer[4] = {"1","2","2.5","3?"};

	printf("Frame%d: MPEG%s Layer%d %dHz %s %dbps size:Encoded:%d Decoded:%d\n",
	       iFrameCount, ppszMpegVer[ver], layer+1,
	       freq, stereo ? "stereo" : "mono", rate,
	       iEncodedFrameSize, iDecodedFrameSize);

	// Decode the MP3 frame
	//DecodeMp3Frame(&m_fileBuf[m_filePos], iEncodedFrameSize, iDecodedFrameSize);

	// Move to the next frame
	iFrameCount++;
	if (pos + iEncodedFrameSize >= len)
	    break;
	pos += iEncodedFrameSize;
    }
}
