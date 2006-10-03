#include <Flek/math.H>
#include <Flek/FFile.H>
#include <Flek/FImage.H>
#include <Flek/FJPEG.H>

extern "C"
{
#include <jpeglib.h>
}

/*
 * Code to read JPEG images.  Boiler plate stuff snarfed from the 
 * libjpeg examples.
 */

FImage *
FJPEG::read(char *filename)
{

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE *infile;
    int row_stride;

    if ((infile = fopen(filename, "rb")) == NULL) {
	fprintf(stderr, "can't open %s\n", filename);
	return 0;
    }
    // Step 1: allocate and initialize JPEG decompression object
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Step 2: specify data source (eg, a file)
    jpeg_stdio_src(&cinfo, infile);

    // Step 3: read file parameters with jpeg_read_header()
    (void) jpeg_read_header(&cinfo, TRUE);

    // Step 5: Start decompressor  
    (void) jpeg_start_decompress(&cinfo);

    FImage *rval =
	new FImage(cinfo.output_width, cinfo.output_height,
		   cinfo.output_components);
    row_stride = cinfo.output_width * cinfo.output_components;

    while (cinfo.output_scanline < cinfo.output_height) {
	unsigned char *i[1];
	i[0] =
	    *(rval->begin(cinfo.output_height - cinfo.output_scanline - 1));
	(void) jpeg_read_scanlines(&cinfo, i, 1);
    }

    // Step 7: Finish decompression 
    (void) jpeg_finish_decompress(&cinfo);

    // Step 8: Release JPEG decompression object and allocated memory
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);
    return rval;
}

int
FJPEG::write(char *filename, FImage * data, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    if (data->channels() == 3)
	cinfo.in_color_space = JCS_RGB;
    else if (data->channels() == 1)
	cinfo.in_color_space = JCS_GRAYSCALE;
    else
	return -1;

    FILE *outfile;
    int row_stride;
    unsigned char *row_pointer[1];
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if ((outfile = fopen(filename, "wb")) == NULL) {
	fprintf(stderr, "can't open %s\n", filename);
	return -1;
    }

    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = data->width();
    cinfo.image_height = data->height();
    if (data->channels() == 3)
	cinfo.in_color_space = JCS_RGB;
    else if (data->channels() == 1)
	cinfo.in_color_space = JCS_GRAYSCALE;
    else
	return -1;
    cinfo.input_components = data->channels();
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = data->width() * data->channels();

    while (cinfo.next_scanline < cinfo.image_height) {
	row_pointer[0] =
	    *(data->begin(cinfo.image_height - cinfo.next_scanline - 1));
	(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    return 0;
}
