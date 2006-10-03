/*********************************************************************
  libvideogfx/x11/dispimg.hh

  purpose:
   Wrapper class around X11 XImage. Whenever possible the MIT-SHM
   extension is used for storing the images.

  notes:
   - Create() is not multi-threading save as X11-errors are caught

  to do:
   - Check that different endian schemes on client and server are working.
   - Try to redefine interface so that hardward YUV2RGB convertion is possible.
   - Support for other display depths.

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   21/Jan/2000 - Dirk Farin - X11 specific data is now hidden to circumvent
                              global namespace pollution by definitions in
                              X11-header files.
   20/Oct/1999 - Dirk Farin - Bugfix. mayUseMITSHM was uninitialized
   29/Jul/1999 - Dirk Farin - Integration into ULib, changes in user interface.
   07/Jul/1999 - Dirk Farin - robust handling of MIT-SHM failures
   25/Jan/1999 - Dirk Farin - support 8bit displays
   14/Jan/1999 - Dirk Farin - Bugfix: 24 and 32bit display code now working
   07/Jan/1999 - Dirk Farin - support for 24 and 32bit deep displays
   03/Jan/1999 - Dirk Farin - support for X11 shared memory extension
   30/Dec/1998 - Dirk Farin - first implementation (16 bit deep displays only, 5:6:5 format)
 *********************************************************************/

#ifndef LIBVIDEOGFX_X11_DISPIMG_HH
#define LIBVIDEOGFX_X11_DISPIMG_HH

#include "libvideogfx/nanox/server.hh"

typedef struct {
	int width;
	int height;
	int bytes_per_line;
	int bits_per_pixel;
	int byte_order;		// LSBFirst
	int red_mask;
	int green_mask;
	int blue_mask;
	char *data;
} XImage;

#define LSBFirst 0

class DisplayImage_X11
{
public:
  DisplayImage_X11();
  ~DisplayImage_X11();

  void UseMITSHM(bool flag=true);

  void Create(int w,int h,GR_WINDOW_ID win,const X11Server* server=NULL);

  XImage& AskXImage();

  void PutImage(int srcx0=0,int srcy0=0,int w=0,int h=0, int dstx0=0,int dsty0=0);

private:
  struct DisplayImage_Data* d_data;
};

#endif
