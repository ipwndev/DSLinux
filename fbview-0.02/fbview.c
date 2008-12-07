#include <stdio.h>
#include <string.h>

#include "bitmap.h"			// bitmap codec
#include "ds_png.h"

#define PNG_SUPPORT

char extension[255];		// uppercased file extension; f.e. "BMP"
char filename[255];		// multimedia file name; f.e. "image.bmp"
int (*play_function)(char *);		// pointer to codec function

// functions decodes parameters
// set config variables, filename and extension
int decode_params(int argc, char ** argv)
{
	char * ext_start;
	char i;

	if (argc < 2) return 1;
	strcpy(filename, argv[1]);

	ext_start = strrchr(filename, '.');
	if (!ext_start) return 2;
	strcpy(extension, ext_start+1);

	for (i=0; extension[i]; i++)
		extension[i] = toupper(extension[i]);

	return 0;
}

// --------------------------------------------------------------

int main(int argc, char ** argv)
{
    printf ("fbview - GNU frame buffer image viewer.\n");
    printf ("Supportet file formats: BMP-24bit, PNG\n");
    printf ("written by galactic, 2008\n\n");

    if (decode_params(argc, argv)) {
	return 1;
    }

    if (strcmp(extension, "BMP") == 0) {
	printf ("File decoded as \"Bitmap Image File\".\n\n");
	play_function = bitmap_play;
    } else

#ifdef PNG_SUPPORT
    if (strcmp(extension, "PNG") == 0) {
	printf ("File decoded as \"PNG Image File\".\n\n");
	play_function = png_play;
    } else
#endif

	play_function = NULL;

    if (play_function) {
	return ((* play_function)(filename));
    } else {
	printf ("Unknown file extension \"%s\".\n", extension);
	return 3;
    }

    return 0;
}
