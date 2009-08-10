/*
 *  (C) Copyright 2008-2009 Kamil Kopec <kamil_kopec@poczta.onet.pl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License Version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>

#include "ds_bmp.h"

/*
 * it would be much easier when alingment would be set to 2 bytes,
 * but arm processors doesn't support this. (#pragma pack())
 */

/**
 * bitmap header part 1
 */
typedef
struct bitmap_file_header
{
		Uint16	bf_type;	/* File header, must be 'BM' == 0x424d */
		Uint32	bf_size;	/* file size in bytes */
		Uint16	bf_reserved1;	/* not used, must be 0 */
		Uint16	bf_reserved2;	/* not used, must be 0 */
		Uint32	bf_data_offset;	/* bitmap file data offset */
} bitmap_file_header;

/**
 * bitmap header part 2
 */
typedef
struct bitmap_info_header
{
		Uint32	bi_size;	/* bi_size = sizeof(bitmap_info_header) */
		Uint32	bi_width;	/* image width */
		Uint32	bi_height;	/* image height */
		Uint16	bi_planes;	/* planes number; supported only 0 */
		Uint16	bi_bpp;		/* bit per pixel */
		Uint32	bi_compression;	/* compression type; supported 0 (no compression) */
		Uint32	bi_data_size;	/* size of bitmat data segment in bytes */
		Uint32	bi_pixels_per_meter_X;	/* pixels per meter (used while printing on paper) */
		Uint32	bi_pixels_per_meter_Y;
		Uint32	bi_colour_count;	/* amount of elements of colours array */
		Uint32	bi_colour_used;	/* amount of colours used in bitmap */
} bitmap_info_header;
	
/**
 * bitmap pallete structure. In bmp-24 bit it doesn't exists, so
 * it is not used in thes program.
 */
struct bitmap_color
{
		Uint8	bc_blue;	// color RGB definition
		Uint8	bc_green;
		Uint8	bc_red;
		Uint8	bc_reserved;	// not used; must be 0
};
	
/* ------------------------------------------------------------------- */

/**
 * load bmp file to fb_pixmap
 * @param filename name of bmp file on the disk to load
 * @param fb_pixmap pointer to the memory for pixmap
 * @param w width of loaded image
 * @param h height of loaded image
 * @return 0 if success, 1 if failure.
 */
int bmp2fb_pixmap(char * filename, fb_pixel ** fb_pixmap, Uint16 * w, Uint16 * h)
{
	/* bitmap headers */
	bitmap_file_header bmp_header;
	bitmap_info_header bmp_info;
	
	FILE * fp;
	Sint16 i, j;
	Uint8 R,G,B,A;
	
	/* open the file */
	if ((fp = fopen(filename, "rb")) == NULL)
		return 1;
	
	/* read header */
	if (!fread(&(bmp_header.bf_type), sizeof(bmp_header.bf_type), 1, fp)
		|| !fread(&(bmp_header.bf_size), sizeof(bmp_header.bf_size), 1, fp)
		|| !fread(&(bmp_header.bf_reserved1), sizeof(bmp_header.bf_reserved1), 1, fp)
		|| !fread(&(bmp_header.bf_reserved2), sizeof(bmp_header.bf_reserved2), 1, fp)
		|| !fread(&(bmp_header.bf_data_offset), sizeof(bmp_header.bf_data_offset), 1, fp))
	{
		fclose(fp);
		return 1;
	}
	
	/* check bitmap signature */
	if (bmp_header.bf_type != 0x4d42)
	{
		fclose(fp);
		return 1;
	}
	
	/* read fb_img info */
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
		return 1;
	}
	
	/* compression not supportet */
	if (bmp_info.bi_compression)
	{
		fclose(fp);
		return 1;
	}
	
	/* bitmap size > max_mem MB ? */
	if (bmp_info.bi_width * bmp_info.bi_height * FB_BPP / 8 > MEMSIZE)
	{
		fclose(fp);
		return 1;
	}
	
	if (bmp_info.bi_bpp != 24 && bmp_info.bi_bpp != 32)
	{
		fclose(fp);
		return 1;
	}
	
	fseek(fp, bmp_header.bf_data_offset, SEEK_SET);
	
	*w = bmp_info.bi_width;
	*h = bmp_info.bi_height;
	
	*fb_pixmap = (fb_pixel*)malloc(bmp_info.bi_width * bmp_info.bi_height * pixelsize);
	if ((*fb_pixmap)==NULL)
	{
		/* error while allocating memory */
		fclose(fp);
		return 1;
	}
	
	/* reading, each pixel of each line */
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
		fb_set_pixel((*fb_pixmap)+((i+j*bmp_info.bi_width)*pixelsize), R, G, B);
		}
		
		for (A = 0; A < bmp_info.bi_width % 4; A++) fgetc(fp);
	}
	
	/* everyting is ok, so return 0 */
	return 0;
}
	
/* ------------------------------------------------------------------- */

/**
 * save pixmap to bmp file 
 * @param filename name of bmp file on the disk to save
 * @param fb_pixmap pixmap of image to save
 * @param w width
 * @param h height
 * @return 0 if success, 1 if failure
 */
int fb_pixmap2bmp(char * filename, fb_pixel * fb_pixmap, Uint16 w, Uint16 h)
{
	/* bitmap headers */
	bitmap_file_header bmp_header;
	bitmap_info_header bmp_info;
	
	FILE * fp;
	Sint16 i, j;
	Uint8 R,G,B,A;
	Uint32 bits_per_row;
	
	/* open the file */
	if ((fp = fopen(filename, "wb")) == NULL)
		return -1;
	
	/* size of each row of bmp must multiple of 4.
	 * this variable makes calculations easier
	 */
	bits_per_row = (w * 3) % 4 + w * 3;
	
	/* creating header */
	bmp_header.bf_type = 0x4d42; /* BMP signature */
	bmp_header.bf_size = bits_per_row * h + 0x36;
	bmp_header.bf_reserved1 = 0;
	bmp_header.bf_reserved2 = 0;
	bmp_header.bf_data_offset = 0x0036;/* offest to RGB pixmap */
	
	bmp_info.bi_size = 0x0028; /* size of header */
	bmp_info.bi_width = w;
	bmp_info.bi_height = h;
	bmp_info.bi_planes = 1;
	bmp_info.bi_bpp = 24;
	bmp_info.bi_compression = 0;
	bmp_info.bi_data_size = bits_per_row * h;
	bmp_info.bi_pixels_per_meter_X = 0x0b13;
	bmp_info.bi_pixels_per_meter_Y = 0x0b13;
	bmp_info.bi_colour_count = 0;
	bmp_info.bi_colour_used = 0;
	
	/* write header */
	if (!fwrite(&(bmp_header.bf_type), sizeof(bmp_header.bf_type), 1, fp)
		|| !fwrite(&(bmp_header.bf_size), sizeof(bmp_header.bf_size), 1, fp)
		|| !fwrite(&(bmp_header.bf_reserved1), sizeof(bmp_header.bf_reserved1), 1, fp)
		|| !fwrite(&(bmp_header.bf_reserved2), sizeof(bmp_header.bf_reserved2), 1, fp)
		|| !fwrite(&(bmp_header.bf_data_offset), sizeof(bmp_header.bf_data_offset), 1, fp))
	{
		fclose(fp);
		return 1;
	}
	
	/* write bmp info */
	if (!fwrite(&(bmp_info.bi_size), sizeof(bmp_info.bi_size), 1, fp)
		|| !fwrite(&(bmp_info.bi_width), sizeof(bmp_info.bi_width), 1, fp)
		|| !fwrite(&(bmp_info.bi_height), sizeof(bmp_info.bi_height), 1, fp)
		|| !fwrite(&(bmp_info.bi_planes), sizeof(bmp_info.bi_planes), 1, fp)
		|| !fwrite(&(bmp_info.bi_bpp), sizeof(bmp_info.bi_bpp), 1, fp)
		|| !fwrite(&(bmp_info.bi_compression), sizeof(bmp_info.bi_compression), 1, fp)
		|| !fwrite(&(bmp_info.bi_data_size), sizeof(bmp_info.bi_data_size), 1, fp)
		|| !fwrite(&(bmp_info.bi_pixels_per_meter_X), sizeof(bmp_info.bi_pixels_per_meter_X), 1, fp)
		|| !fwrite(&(bmp_info.bi_pixels_per_meter_Y), sizeof(bmp_info.bi_pixels_per_meter_Y), 1, fp)
		|| !fwrite(&(bmp_info.bi_colour_count), sizeof(bmp_info.bi_colour_count), 1, fp)
		|| !fwrite(&(bmp_info.bi_colour_used), sizeof(bmp_info.bi_colour_used), 1, fp))
	{
		fclose(fp);
		return 1;
	}
	
	/* writing data */
	for (j = bmp_info.bi_height-1; j >= 0; j--)
	{
		for (i = 0; i < bmp_info.bi_width; i++)
		{
			fb_get_pixel(fb_pixmap+((i+j*bmp_info.bi_width)*pixelsize), &R, &G, &B);
			fputc(B, fp); fputc(G, fp); fputc(R, fp);
		}
	
		for (A = 0; A < bmp_info.bi_width % 4; A++) fputc(0x00, fp);
	}
	
	/* everyting is ok, so return 0 */
	return 0;
}
