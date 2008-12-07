#ifndef ds_bitmap_h
#define ds_bitmap_h

#include "types.h"

// struct data aligment = 2 bytes
// each variable has size >= 2 bytes
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(2)
#define PACK_STRUCT
#endif

typedef
struct bitmap_file_header
{
	Uint16	bf_type;	// File header, must be 'BM' == 0x424d
	Uint32	bf_size;	// file size in bytes
	Uint16	bf_reserved1;	// not used, must be 0
	Uint16	bf_reserved2;	// not used, must be 0
	Uint32	bf_data_offset;	// bitmap file data offset
} bitmap_file_header;

typedef
struct bitmap_info_header
{
	Uint32	bi_size;	// bi_size = sizeof(bitmap_info_header)
	Uint32	bi_width;	// image width
	Uint32	bi_height;	// image height
	Uint16	bi_planes;	// planes number; supported only 0
	Uint16	bi_bpp;		// bit per pixel
	Uint32	bi_compression;	// compression type; supported 0 (no compression)
	Uint32	bi_data_size;	// size of bitmat data segment in bytes
	Uint32	bi_pixels_per_meter_X;	// pixels per meter (used while printing on paper)
	Uint32	bi_pixels_per_meter_Y;
	Uint32	bi_colour_count;	// amount of elements of colours array
	Uint32	bi_colour_used;	// amount of colours used in bitmap
} bitmap_info_header;

#ifdef PACK_STRUCT
#pragma pack(1)
#endif

struct bitmap_color
{
	Uint8	bc_blue;	// color RGB definition
	Uint8	bc_green;
	Uint8	bc_red;
	Uint8	bc_reserved;	// not used; must be 0
};

#ifdef PACK_STRUCT
#pragma pack(pop)
#endif

// show bitmap using framebuffer
extern int bitmap_play(char * filename);

#endif
