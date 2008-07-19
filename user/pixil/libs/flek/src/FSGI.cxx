#include <Flek/math.H>
#include <Flek/FFile.H>
#include <Flek/FImage.H>
#include <Flek/FSGI.H>
#include <stdio.h>
#include <string.h>

/*
 * Reads 8 bit run length encoded data of size width from input.  
 * The read data is stored in the array row.
 */
static int
get_rle8(FFile & input, int width, uchar * row)
{
    int i;
    uchar c;
    int run_count;		// RLE runs
    int length;			// Bytes read 

    length = 0;

    while (width > 0) {
	input.get_hi(c);
	if (input.bad()) {
	    // CET - libstdc++ is evil on Linux
	    // cerr << "ERROR" << endl;
	    fprintf(stderr, "ERROR\n");
	    return -1;
	}

	length++;

	run_count = c & 127;
	if (run_count == 0)
	    break;

	if (c & 128) {
	    input.read(row, run_count);
	    length += run_count;
	    width -= run_count;
	    row += run_count;
	} else {
	    input.get_hi(c);
	    length++;
	    for (i = 0; i < run_count; i++, row++, width--)
		*row = c;
	}
    }
    return (width > 0 ? -1 : length);
}

static int
put_rle8(FFile & output, int width, uchar * row)
{
    int length;			// Byte count of output line 
    int run_count;		// Number of repeated/non-repeated pixels
    int x;
    int i;
    uchar *start, repeat;

    for (x = width, length = 0; x > 0;) {
	start = row;
	row += 2;
	x -= 2;

	while (x > 0 && (row[-2] != row[-1] || row[-1] != row[0])) {
	    row++;
	    x--;
	}

	row -= 2;
	x += 2;

	run_count = row - start;
	while (run_count > 0) {
	    i = run_count > 126 ? 126 : run_count;
	    run_count -= i;

	    output.put_hi((uchar) (128 | i));
	    length++;

	    while (i > 0) {
		output.put_hi((uchar) * start);

		start++;
		length++;
		i--;
	    }
	}

	if (x <= 0)
	    break;

	start = row;
	repeat = row[0];

	row++;
	x--;

	while (x > 0 && *row == repeat) {
	    row++;
	    x--;
	}

	run_count = row - start;
	while (run_count > 0) {
	    i = run_count > 126 ? 126 : run_count;
	    run_count -= i;

	    output.put_hi((uchar) i);

	    output.put_hi((uchar) repeat);

	    length += 2;
	}
    }

    length++;

    output.put_hi((char) 0);
    if (output.bad())
	return -1;

    return length;
}

typedef struct
{
    int bpc;			// Bytes per channel
    int compression;		// Compression 
    ushort width,		// Width in pixels
      height,			// Height in pixels
      channels;			// Number of channels 
    ulong first_row,		// File offset for first row
      next_row,			// File offset for next row
    **table,			// Offset table for compression
    **length;			// Length table for compression 
    uchar *arle_row;		// Advanced RLE compression buffer
    long arle_offset,		// Advanced RLE buffer offset
      arle_length;		// Advanced RLE buffer length
}
sgiT;

int
sgi_put_row(FFile & output, sgiT & img, uchar * row, int y, int channel)
{
    int x;
    long offset;

    if (!row)
	return -1;

    switch (img.compression) {
    case FSGI::NONE:

	offset = 512 + (y + channel * img.height) * img.width * img.bpc;
	if (offset != output.tell())
	    output.seek(offset);

	if (img.bpc == 1)
	    output.write((char *) row, img.width);

	break;

    case FSGI::ARLE:

	// Check the last row written.
	if (img.arle_offset > 0) {
	    for (x = 0; x < img.width; x++)
		if (row[x] != img.arle_row[x])
		    break;

	    if (x == img.width) {
		img.table[channel][y] = img.arle_offset;
		img.length[channel][y] = img.arle_length;
		return (0);
	    }
	}
	// If that didn't match, search the previous rows.
	output.seek((long) img.first_row);

	if (img.bpc == 1) {
	    do {
		img.arle_offset = output.tell();
		if ((img.arle_length =
		     get_rle8(output, img.width, img.arle_row)) < 0) {
		    x = 0;
		    break;
		}

		for (x = 0; x < img.width; x++)
		    if (row[x] != img.arle_row[x])
			break;
	    }
	    while (x < img.width);
	} else {
	    // CET - libstdc++ is evil on Linux
	    //cerr << "SGI files larger than 1 byte per channel are not supported." << endl;
	    fprintf(stderr,
		    "SGI files larger than 1 byte per channel are not supported.\n");
	    return -1;
	}

	if (x == img.width) {
	    img.table[channel][y] = img.arle_offset;
	    img.length[channel][y] = img.arle_length;
	    return 0;
	} else
	    output.seek(0);

    case FSGI::RLE:

	img.table[channel][y] = img.next_row;
	offset = (long) img.next_row;

	if (offset != output.tell())
	    output.seek(offset);

	if (img.bpc == 1)
	    x = put_rle8(output, img.width, row);
	else {
	    // CET - libstdc++ is evil on Linux
	    //cerr << "SGI files larger than 1 byte per channel are not supported." << endl;
	    // CET - libstdc++ is evil on Linux
	    fprintf(stderr,
		    "SGI files larger than 1 byte per channel are not supported.\n");
	    return -1;
	}

	if (img.compression == FSGI::ARLE) {
	    img.arle_offset = offset;
	    img.arle_length = x;
	    memcpy(img.arle_row, row, img.width);
	}

	img.next_row = output.tell();
	img.length[channel][y] = x;

	return x;
    }

    return 0;
}

int
sgi_get_row(FFile & input, sgiT & img, uchar * row, int y, int channel)
{
    ulong offset;

    if ((!row) ||
	(y < 0) || (y >= img.height) ||
	(channel < 0) || (channel >= img.channels))
	return -1;

    switch (img.compression) {
    case FSGI::NONE:

	offset = 512 + (y + channel * img.height) * img.width * img.bpc;

	if (offset != (ulong) input.tell())
	    input.seek((long) offset);

	if (img.bpc == 1) {
	    input.read(row, img.width);
	    row += img.width;
	} else {
	    // CET - libstdc++ is evil on Linux
	    //cerr << "Not supported" << endl;
	    fprintf(stderr, "Not supported\n");
	    return -1;
	}
	break;

    case FSGI::RLE:
	offset = img.table[channel][y];
	if (offset != (ulong) input.tell())
	    input.seek((long) offset);
	if (input.bad()) {
	    // CET - libstdc++ is evil on Linux
	    //cerr << "ERROR: Bad seek" << endl;
	    fprintf(stderr, "ERROR: Bad seek\n");
	}
	if (input.bad()) {
	    // CET - libstdc++ is evil on Linux
	    //cerr << "ERROR: Bad seek (EOF)" << endl;
	    fprintf(stderr, "ERROR: Bad seek (EOF)\n");
	}
	if (img.bpc == 1)
	    return (get_rle8(input, img.width, row));

	// CET - libstdc++ is evil on Linux
	//cerr << "SGI files larger than 1 byte per channel are not supported." << endl;
	fprintf(stderr,
		"SGI files larger than 1 byte per channel are not supported.\n");
	return -1;
    }

    return 0;
}

#include <stdlib.h>

FImage *
FSGI::read(char *filename)
{
    FFile input;
    sgiT img;
    input.open(filename, FFileRead);
    uchar c;

    short magic;
    input.get_hi(magic);

    if (magic != MAGIC)
	return 0;

    input.get_hi(c);
    img.compression = c;
    input.get_hi(c);
    img.bpc = c;
    input.get_hi(img.width);	// Dimensions (ignore)
    input.get_hi(img.width);
    input.get_hi(img.height);
    input.get_hi(img.channels);
    unsigned long t;
    input.get_hi(t);		// Minimum pixel 
    input.get_hi(t);		// Maximum pixel 

    FImage *Nimg = new FImage(img.width, img.height, 4);

    if (img.compression) {
	int i, j;

	input.seek(512);

	if (input.bad()) {
	    // CET - libstdc++ is evil on Linux
	    //cerr << "ERROR @sgiReadFile." << endl;
	    fprintf(stderr, "ERROR @sgiReadFile.\n");
	}
	img.table = new ulongPtr[img.channels];
	img.table[0] = new ulong[img.height * img.channels];

	for (i = 1; i < img.channels; i++)
	    img.table[i] = img.table[0] + i * img.height;

	for (i = 0; i < img.channels; i++)
	    for (j = 0; j < img.height; j++) {
		ulong offset;
		input.get_hi(offset);
		img.table[i][j] = offset;
	    }
    }

    uchar *pixel = *(Nimg->begin());

    // Allocate enough memory for one line of the image.
    uchar **rows = new ucharPtr[img.channels];
    rows[0] = new uchar[img.width * img.channels];
    for (int z = 0; z < img.channels; z++)
	rows[z] = rows[0] + z * img.width;

    if (img.channels == 4) {
	for (int y = 0; y < img.height; y++) {
	    sgi_get_row(input, img, rows[0], y, 0);
	    sgi_get_row(input, img, rows[1], y, 1);
	    sgi_get_row(input, img, rows[2], y, 2);
	    sgi_get_row(input, img, rows[3], y, 3);

	    for (int x = 0; x < img.width; x++, pixel += 4) {
		pixel[0] = rows[0][x];
		pixel[1] = rows[1][x];
		pixel[2] = rows[2][x];
		pixel[3] = rows[3][x];
	    }
	}
    } else if (img.channels == 3) {
	for (int y = 0; y < img.height; y++) {
	    sgi_get_row(input, img, rows[0], y, 0);
	    sgi_get_row(input, img, rows[1], y, 1);
	    sgi_get_row(input, img, rows[2], y, 2);

	    for (int x = 0; x < img.width; x++, pixel += 4) {
		pixel[0] = rows[0][x];
		pixel[1] = rows[1][x];
		pixel[2] = rows[2][x];
		pixel[3] = 255;
	    }
	}
    } else if (img.channels == 2) {
	for (int y = 0; y < img.height; y++) {
	    sgi_get_row(input, img, rows[0], y, 0);
	    sgi_get_row(input, img, rows[1], y, 1);

	    for (int x = 0; x < img.width; x++, pixel += 4) {
		pixel[0] = rows[0][x];
		pixel[1] = rows[1][x];
		pixel[2] = 0;
		pixel[3] = 255;
	    }
	}
    } else if (img.channels == 1)	// Gray
    {
	for (int y = 0; y < img.height; y++) {
	    sgi_get_row(input, img, rows[0], y, 0);

	    for (int x = 0; x < img.width; x++, pixel += 4) {
		pixel[0] = pixel[1] = pixel[2] = rows[0][x];
		pixel[3] = 255;
	    }
	}
    }

    if (img.compression) {
	delete img.table[0];
	img.table[0] = 0;
	delete[]img.table;
    }
    delete[]rows[0];
    delete[]rows;

    return Nimg;
}

#include <string.h>

int
FSGI::write(char *filename, FImage * data, int compression, int channels)
{
    FFile output;
    output.open(filename, FFileWritePlus);

    sgiT img;
    uchar c;
    int i;

    img.bpc = 1;
    img.compression = compression;
    img.width = data->width();
    img.height = data->height();
    img.channels = channels;
    img.first_row = 0;
    img.next_row = 0;
    img.table = 0;
    img.length = 0;
    img.arle_row = 0;
    img.arle_offset = 0;
    img.arle_length = 0;

    output.put_hi((unsigned short) MAGIC);
    c = (img.compression != 0);
    output.put_hi(c);		// compression
    c = img.bpc;
    output.put_hi(c);		// bpc
    output.put_hi((unsigned short) 3);	// dimensions
    output.put_hi(img.width);
    output.put_hi(img.height);
    output.put_hi(img.channels);
    output.put_hi((long) 0);	// Minimum pixel 
    output.put_hi((long) 255);	// Maximum pixel 
    output.put_hi((long) 0);	// Reserved

    // This bit helps stop us from wasting processor time on blank space.
    int blankSize =
	max((unsigned short)
	    max((unsigned short) 488,
		(unsigned short) (img.width * img.channels)),
	    (unsigned short) (img.height * img.channels * 4));
    char *blank = new char[blankSize];
    memset(blank, 0, blankSize);

    output.write(blank, 488);	// 80+102*4 = 488

    switch (compression) {
    case FSGI::NONE:

	for (i = img.height; i > 0; i--)
	    output.write(blank, img.width * img.channels);

	break;

    case FSGI::ARLE:

	// Allocate an extra row for ARLE
	img.arle_row = new uchar[img.width];
	img.arle_offset = 0;

    case FSGI::RLE:

	// Write blank scanline tables for RLE and ARLE
	// Write 0s for img.height * img.channels * 2 * sizeof (long)
	output.write(blank, img.height * sizeof(long) * img.channels);
	output.write(blank, img.height * sizeof(long) * img.channels);

	img.first_row = output.tell();
	img.next_row = output.tell();

	img.table = new ulongPtr[img.channels];
	img.table[0] = new ulong[img.height * img.channels];

	img.length = new ulongPtr[img.channels];
	img.length[0] = new ulong[img.height * img.channels];

	for (i = 1; i < img.channels; i++) {
	    img.table[i] = img.table[0] + (i * img.height);
	    img.length[i] = img.length[0] + (i * img.height);
	}

	break;
    }

    uchar *pixel;

    uchar **rows = new ucharPtr[img.channels];
    rows[0] = new uchar[img.width * img.channels];

    for (int z = 1; z < img.channels; z++)
	rows[z] = rows[0] + z * img.width;

    pixel = *(data->begin());

    if (img.channels == 4) {
	for (int y = 0; y < img.height; y++) {
	    for (int x = 0; x < img.width; x++, pixel += 4) {
		rows[0][x] = pixel[0];
		rows[1][x] = pixel[1];
		rows[2][x] = pixel[2];
		rows[3][x] = pixel[3];
	    }

	    sgi_put_row(output, img, rows[0], y, 0);
	    sgi_put_row(output, img, rows[1], y, 1);
	    sgi_put_row(output, img, rows[2], y, 2);
	    sgi_put_row(output, img, rows[3], y, 3);
	}
    } else if (img.channels == 3) {
	for (int y = 0; y < img.height; y++) {
	    for (int x = 0; x < img.width; x++, pixel += 4) {
		rows[0][x] = pixel[0];
		rows[1][x] = pixel[1];
		rows[2][x] = pixel[2];
	    }

	    sgi_put_row(output, img, rows[0], y, 0);
	    sgi_put_row(output, img, rows[1], y, 1);
	    sgi_put_row(output, img, rows[2], y, 2);
	}
    } else if (img.channels == 2) {
	for (int y = 0; y < img.height; y++) {
	    for (int x = 0; x < img.width; x++, pixel += 4) {
		rows[0][x] = pixel[0];
		rows[1][x] = pixel[1];
	    }

	    sgi_put_row(output, img, rows[0], y, 0);
	    sgi_put_row(output, img, rows[1], y, 1);
	}
    } else if (img.channels == 1) {
	for (int y = 0; y < img.height; y++) {
	    for (int x = 0; x < img.width; x++, pixel += 4) {
		rows[0][x] = pixel[0];
	    }

	    sgi_put_row(output, img, rows[0], y, 0);
	}
    }

    if (img.compression != NONE) {
	output.seek(512);

	// Write the offset table.
	lo_to_hi(img.table[0], img.channels * img.height);
	output.write((char *) img.table[0], img.height * img.channels * 4);

	// Write the length table.
	lo_to_hi(img.length[0], img.channels * img.height);
	output.write((char *) img.length[0], img.height * img.channels * 4);

    }

    output.close();

    if (compression == FSGI::ARLE)
	delete[]img.arle_row;

    if (compression) {
	delete[]img.table[0];
	delete[]img.table;
	delete[]img.length[0];
	delete[]img.length;
    }

    delete[]rows[0];
    delete[]rows;
    delete[]blank;
    return 0;
}

bool
FSGI::valid(char *filename)
{
    FFile input;
    input.open(filename, FFileRead);

    short magic;
    input.get_hi(magic);

    input.close();

    if (magic != MAGIC)
	return false;

    return true;
}
