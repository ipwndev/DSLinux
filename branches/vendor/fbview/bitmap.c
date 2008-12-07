#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "framebuffer.h"
#include "keyboard.h"

// #define debug

bitmap_file_header bmp_header;
bitmap_info_header bmp_info;

// if img.res > fb.res then
// draw only a part of img
Uint16 position_x = 0;
Uint16 position_y = 0;

typedef fb_pixel bitmap_data;

// -------------------------------------------------------------------

void bitmap_dump_header(bitmap_file_header * bmp_header, bitmap_info_header * bmp_info)
{
    printf ("sizeof(Uint8): %d\n", sizeof(Uint8));
    printf ("sizeof(Uint16): %d\n", sizeof(Uint16));
    printf ("sizeof(Uint32): %d\n", sizeof(Uint32));
    printf ("sizeof(Uint64): %d\n", sizeof(Uint64));
    printf ("sizeof(bitmap_file_header): %d\n", sizeof(bitmap_file_header));
    printf ("sizeof(bitmap_info_header): %d\n\n", sizeof(bitmap_info_header));

    printf ("type: %d\n", bmp_header->bf_type);
    printf ("size: %d\n", bmp_header->bf_size);
    printf ("reserv1: %d\n", bmp_header->bf_reserved1);
    printf ("reserv2: %d\n", bmp_header->bf_reserved2);
    printf ("data offset: %d\n\n", bmp_header->bf_data_offset);

    printf ("size of header: %d\n", bmp_info->bi_size);
    printf ("width: %d\n", bmp_info->bi_width);
    printf ("height: %d\n", bmp_info->bi_height);
    printf ("planes: %d\n", bmp_info->bi_planes);
    printf ("bpp: %d\n", bmp_info->bi_bpp);
    printf ("compression: %d\n", bmp_info->bi_compression);
    printf ("size of data segment: %d\n", bmp_info->bi_data_size);
    printf ("px per metX: %d\n", bmp_info->bi_pixels_per_meter_X);
    printf ("px per metY: %d\n", bmp_info->bi_pixels_per_meter_Y);
    printf ("count colour: %d\n", bmp_info->bi_colour_count);
    printf ("colour used: %d\n\n", bmp_info->bi_colour_used);
}

// -------------------------------------------------------------------

fb_pixel * bitmap_load(char * filename)
{
    FILE * fp;
    fb_pixel * bmp_buffer;
    Sint16 i, j;
    Uint8 R,G,B,A;

    // open the file
    if ((fp = fopen(filename, "rb")) == NULL)
        return NULL;

    // read header
    if (!fread(&(bmp_header.bf_type), sizeof(bmp_header.bf_type), 1, fp)
	|| !fread(&(bmp_header.bf_size), sizeof(bmp_header.bf_size), 1, fp)
	|| !fread(&(bmp_header.bf_reserved1), sizeof(bmp_header.bf_reserved1), 1, fp)
	|| !fread(&(bmp_header.bf_reserved2), sizeof(bmp_header.bf_reserved2), 1, fp)
	|| !fread(&(bmp_header.bf_data_offset), sizeof(bmp_header.bf_data_offset), 1, fp))
    {
	fclose(fp);
	return NULL;
    }

    // check bitmap signature
    if (bmp_header.bf_type != 0x4d42)
    {
	fclose(fp);
	return NULL;
    }

    // read image info
    if (!fread(&(bmp_info.bi_size), sizeof(bmp_info.bi_size), 1, fp)
	|| !fread(&(bmp_info.bi_width), sizeof(bmp_info.bi_width), 1, fp)
	|| !fread(&(bmp_info.bi_height), sizeof(bmp_info.bi_height), 1, fp)
	|| !fread(&(bmp_info.bi_planes), sizeof(bmp_info.bi_planes), 1, fp)
	|| !fread(&(bmp_info.bi_bpp), sizeof(bmp_info.bi_bpp), 1, fp)
	|| !fread(&(bmp_info.bi_compression), sizeof(bmp_info.bi_compression), 1, fp)
	|| !fread(&(bmp_info.bi_data_size), sizeof(bmp_info.bi_data_size), 1, fp)
	|| !fread(&(bmp_info.bi_pixels_per_meter_X), sizeof(bmp_info.bi_pixels_per_meter_X), 1, fp)
	|| !fread(&(bmp_info.bi_pixels_per_meter_Y), sizeof(bmp_info.bi_pixels_per_meter_Y), 1, fp)
	|| !fread(&(bmp_info.bi_colour_count), sizeof(bmp_info.bi_colour_count), 1, fp)
	|| !fread(&(bmp_info.bi_colour_used), sizeof(bmp_info.bi_colour_used), 1, fp))
    {
        fclose(fp);
        return NULL;
    }

    #ifdef debug
	bitmap_dump_header(&bmp_header, &bmp_info);
    #endif

    // compression?
    if (bmp_info.bi_compression)
    {
	fclose(fp);
	return NULL;	// compression not supportet
    }

    // bitmap size > max_mem MB ?
    if (bmp_info.bi_width * bmp_info.bi_height * FB_BPP / 8 > MEMSIZE)
    {
	fclose(fp);
	return NULL;
    }

    if (bmp_info.bi_bpp != 24 && bmp_info.bi_bpp != 32)
    {
	fclose(fp);
	return NULL;
    }

    fseek(fp, bmp_header.bf_data_offset, SEEK_SET);

    bmp_buffer = malloc(bmp_info.bi_width * bmp_info.bi_height * pixelsize);
    if (bmp_buffer == NULL)
    {
	// error while allocating memory
	fclose(fp);
	return NULL;
    }

    for (j = bmp_info.bi_height-1; j >= 0; j--)
    {
	for (i = 0; i < bmp_info.bi_width; i++)
	{
	    switch (bmp_info.bi_bpp)
	    {
		case 24:
		B=fgetc(fp); G=fgetc(fp); R=fgetc(fp);
		break;
		case 32:
		B=fgetc(fp); G=fgetc(fp); R=fgetc(fp); A=fgetc(fp);
		break;
	    }
	    fb_setpixel(bmp_buffer+((i+j*bmp_info.bi_width)*pixelsize), /*0xff,0,0*/R, G, B);
	}
	
	for (A = 0; A < bmp_info.bi_width % 4; A++) fgetc(fp);
    }

    // everyting is ok,
    return bmp_buffer;
}

// -------------------------------------------------------------------

void bitmap_render(fb_pixel * bmp_buffer)
{
    Uint32 j;
    Uint32 copy_mem_size;
    Uint32 bmp_line_size;
    Uint32 fb_line_size;
    Uint16 max_j;

    fb_pixel * tmp_screen;
    fb_pixel * tmp_bmp;

    bmp_line_size = bmp_info.bi_width*pixelsize;
    fb_line_size = fb_xres*pixelsize;
    tmp_bmp = bmp_buffer + position_y*bmp_line_size + position_x*pixelsize - bmp_line_size;
    tmp_screen = screen - fb_line_size;
    copy_mem_size = (fb_xres > bmp_info.bi_width ? bmp_line_size : fb_line_size);
    max_j = (fb_yres > bmp_info.bi_height ? bmp_info.bi_height : fb_yres);

    for(j = 0; j <  max_j ;j++)
	memcpy(tmp_screen+=fb_line_size, tmp_bmp+=bmp_line_size, copy_mem_size);
}

// -------------------------------------------------------------------

void bitmap_free(fb_pixel * bmp_buffer)
{
    if (bmp_buffer) free(bmp_buffer);
}


// -------------------------------------------------------------------

int bitmap_play(char * filename)
{
    char ch;

    if (fb_init())
    {
	printf ("Unable to init framebuffer device\n");
	return 2;
    }

    fb_pixel * bmp_buffer;

    if ((bmp_buffer = bitmap_load(filename)) == NULL)
    {
	fb_uninit();
	printf ("Error while reading bitmap\n");
	return 1;
    }

    fb_clear_screen(screen);

    bitmap_render(bmp_buffer);

    init_keyboard();

    ch=0;
    while (1)
    {
	if (!kbhit()) 
	{
	    ch = readch();
	    if (ch == KEY_ESC) break;
	    if (ch == KEY_UP && position_y >= JUMP_SIZE) position_y-=JUMP_SIZE;
	    if (ch == KEY_DOWN && fb_yres <= (bmp_info.bi_height-position_y-JUMP_SIZE)) position_y+=JUMP_SIZE;
	    if (ch == KEY_LEFT && position_x >= JUMP_SIZE) position_x-=JUMP_SIZE;
	    if (ch == KEY_RIGHT && fb_xres <= (bmp_info.bi_width-position_x-JUMP_SIZE)) position_x+=JUMP_SIZE;
	    ch = 0;
	    bitmap_render(bmp_buffer);
	}
    }

    close_keyboard();

    fflush(stdin);

    fb_clear_screen(screen);

    bitmap_free(bmp_buffer);
    fb_uninit();
    return 0;
}
