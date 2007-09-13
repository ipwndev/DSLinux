#include <Flek/FPNM.H>

// CET - FIXME - this shouldn't use libstdc++ if it can be avoided- libstdc++
// causes compatibility problems under linux.  Maybe should use the regular
// stdio.h stuff?
#include <iostream>
#include <fstream>

using namespace std;

static int
skip_comments(ifstream & input)
{
    char c;
    // Skip space up to first comment.
    input.setf(ios::skipws);
    input >> c;
    input.unsetf(ios::skipws);
    if (c != '#') {
	input.putback(c);
	//input.unget ();
	input.setf(ios::skipws);
	return 0;
    }
    while (c == '#') {
	while (c != '\n') {
	    input >> c;
	    if (input.bad())
		return 1;
	}
	input >> c;
	if (input.bad())
	    return 1;
    }
    input.putback(c);
    //input.unget ();
    input.setf(ios::skipws);
    return 0;
}

bool
FPNM::valid(char *filename)
{
    ifstream input;
    int type;
    char c;

    input.open(filename);
    if (input.bad())
	return false;

    input.unsetf(ios::skipws);

    // Check the magic number
    input >> c;
    if (!((c == 'P') || (c == 'p')))
	return false;

    input >> c;
    type = c - 48;

    if ((type < 1) || (type > 7))
	return false;

    input.close();

    return true;
}

FImage *
FPNM::read(char *filename)
{
    ifstream input;
    int width, height;
    int type;
    int depth;
    char c;

    input.open(filename);
    if (input.bad())
	return 0;

    // Check the magic number
    input >> c;
    if (!((c == 'P') || (c == 'p')))
	return 0;

    // the file type
    input >> c;
    type = c - 48;

    // We support types 1-6 and nonstandard 7
    if ((type < 1) || (type > 7))
	return 0;

    if (skip_comments(input))
	return 0;

    switch (type) {
    case 2:
    case 3:
    case 5:
    case 6:
    case 7:
	input >> width;

	if (skip_comments(input))
	    return 0;

	input >> height;

	if (skip_comments(input))
	    return 0;

	input >> depth;

	break;
    case 1:
    case 4:
	input >> width;

	if (skip_comments(input))
	    return 0;

	input >> height;

	break;
    }

    input.unsetf(ios::skipws);

    // Single whitespace before input.
    input >> c;

    FImage *img = new FImage(width, height);
    FImage::iterator begin;
    FImage::iterator end;
    FImage::iterator i;
    uchar *pixel;
    int row;

    switch (type) {
    case 1:			// ASCII B&W
	for (row = height - 1; row >= 0; row--) {
	    begin = img->begin(row);
	    end = img->end(row);
	    int c;

	    for (i = begin; i != end; i++) {
		pixel = *i;
		input >> c;

		if (input.bad())
		    return img;	// return corrupt image.

		if (c)
		    pixel[0] = 255;
		else
		    pixel[0] = 0;
		pixel[1] = pixel[2] = pixel[0];
		pixel[3] = 255;
	    }
	}
	break;
    case 2:			// ASCII GRAY
	input.unsetf(ios::skipws);
	for (row = height - 1; row >= 0; row--) {
	    unsigned int g;
	    begin = img->begin(row);
	    end = img->end(row);

	    for (i = begin; i != end; i++) {
		pixel = *i;
		input >> g;
		pixel[0] = g;	// red
		pixel[1] = pixel[2] = pixel[0];	// blue & green
		pixel[3] = 255;	// alpha
		if (input.bad())
		    return img;	// return corrupt image.
	    }
	}
	break;
    case 3:			// ASCII RGB
	input.unsetf(ios::skipws);
	for (row = height - 1; row >= 0; row--) {
	    unsigned int r;
	    unsigned int g;
	    unsigned int b;
	    begin = img->begin(row);
	    end = img->end(row);

	    for (i = begin; i != end; i++) {
		pixel = *i;
		input >> r;
		pixel[0] = r;	// red
		input >> g;
		pixel[1] = g;	// green
		input >> b;
		pixel[2] = b;	// blue
		pixel[3] = 255;	// alpha
		if (input.bad())
		    return img;	// return corrupt image.
	    }
	}
	break;
    case 4:			// RAW BITPACKED B&W
	for (row = height - 1; row >= 0; row--) {
	    begin = img->begin(row);
	    end = img->end(row);
	    char c;
	    int cx = 0;

	    for (i = begin; i != end;) {
		input >> c;
		if (input.bad())
		    return img;	// return corrupt image.

		for (int j = 0; (j < 8) && (cx + j < width); j++) {
		    pixel = *i;
		    if (0x80 & (c << j))
			pixel[0] = pixel[1] = pixel[2] = 0;
		    else
			pixel[0] = pixel[1] = pixel[2] = 255;
		    pixel[3] = 255;
		    i++;
		}
		cx += 8;
	    }
	}
	break;
    case 5:			// RAW GRAY
	for (row = height - 1; row >= 0; row--) {
	    begin = img->begin(row);
	    end = img->end(row);

	    for (i = begin; i != end; i++) {
		pixel = *i;
		input >> pixel[0];	// red
		pixel[1] = pixel[2] = pixel[0];	// blue & green
		pixel[3] = 255;	// alpha
		if (input.bad())
		    return img;	// return corrupt image.
	    }
	}
	break;
    case 6:			// RAW RGB
	for (row = height - 1; row >= 0; row--) {
	    begin = img->begin(row);
	    end = img->end(row);

	    for (i = begin; i != end; i++) {
		pixel = *i;
		input >> pixel[0];	// red
		input >> pixel[1];	// green
		input >> pixel[2];	// blue
		pixel[3] = 255;	// alpha
		if (input.bad())
		    return img;	// return corrupt image.
	    }
	}
	break;
    case 7:			// RAW RGBA (nonstandard)
	for (row = height - 1; row >= 0; row--) {
	    begin = img->begin(row);
	    end = img->end(row);

	    for (i = begin; i != end; i++) {
		pixel = *i;
		input >> pixel[0];	// red
		input >> pixel[1];	// green
		input >> pixel[2];	// blue
		input >> pixel[3];	// alpha
		if (input.bad())
		    return img;	// return corrupt image.
	    }
	}
	break;
    }

    input.close();

    return img;
}

int
FPNM::write(char *filename, FImage * img)
{
    ofstream output;
    FImage::iterator begin;
    FImage::iterator end;
    FImage::iterator i;
    uchar *pixel;

    output.open(filename);
    if (output.bad())
	return 1;

    output << "P6" << endl;
    output << "# CREATOR: Flek's FPNM::write ()" << endl;
    output << img->width() << " " << img->height() << endl;
    output << "255 ";

    int height = img->height();

    for (int row = height - 1; row >= 0; row--) {
	begin = img->begin(row);
	end = img->end(row);

	for (i = begin; i != end; i++) {
	    pixel = *i;
	    output << pixel[0];	// red
	    output << pixel[1];	// green
	    output << pixel[2];	// blue
	}
    }
    output.close();
    return 0;
}
