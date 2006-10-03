#ifndef _MP3_H_
#define _MP3_H_

class Mp3 {

    	unsigned char  *src;
	int	        pos;
	int		len;
public:
	Mp3(unsigned char *buffer, int len, long flags);

	void Decompress(short * dst, long n); // return number of good samples
};

#endif /* _MP3_H_ */
