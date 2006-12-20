/*********************************************************************
  libvideogfx/x11/imgwin.hh

  purpose:
    X11-wrapper classes that simplify window creation and
    displaying true color images in them.

  notes:

  to do:
    - There seems to be a problem with MultiWindowRefresh. Some
      parts of the image sometimes don't get updated. At the moment
      everything seems to work, though.

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
   31/Jul/2000 - Dirk Farin - new function: MultiWindowRefresh
   03/Aug/1999 - Dirk Farin - new class: ImageWindow_Autorefresh_X11
   29/Jul/1999 - Dirk Farin - first implementation
 *********************************************************************/

#ifndef LIBVIDEOGFX_X11_IMGWIN_HH
#define LIBVIDEOGFX_X11_IMGWIN_HH

#include "dispimg.hh"

#include "libvideogfx/graphics/lowlevel/img2raw.hh"

/* Wrapper class for creating one X11 window with the highest color depth possible.
 */
class ImageWindow_X11
{
public:
  ImageWindow_X11();
  ~ImageWindow_X11(); // Window will be closed in destructor.

  void SetPosition(int x,int y) { d_xpos=x; d_ypos=y; }

  void Create(int w,int h,const char* title,const X11Server* server=NULL,GR_WINDOW_ID parent=0);
  void Close();

  GR_WINDOW_ID   AskWindow();
  int AskDisplay();

private:
  bool        d_initialized;

  struct NXSpecificData* d_nxdata; // This hides Nano-X datatypes from global namespace.

  int d_xpos,d_ypos;
};


/* Enhanced ImageWindow_X11-class that can accept an image and that watches
   X11 events to automatically redraw itself.
   To get fully automatic redrawing you have to create a new thread that
   calls RedrawForever() which will never return.
*/
class ImageWindow_Autorefresh_X11 : public ImageWindow_X11,     // the window itself
				    private DisplayImage_X11,   // the image to be displayed
				    private Image2Raw           // the transformation for image representation convertion
{
public:
   ImageWindow_Autorefresh_X11();
  ~ImageWindow_Autorefresh_X11();

  void Create(int w,int h,const char* title,const X11Server* server=NULL,GR_WINDOW_ID parent=0);
  void Close();

  void Display_const(const Image_YUV<Pixel>&);
  void Display_const(const Image_RGB<Pixel>&);
  void Display      (      Image_YUV<Pixel>&);  // Input image contents may be destroyed
  void Display      (      Image_RGB<Pixel>& i) { Display_const(i); }


  // --- user interaction ---

  char CheckForKeypress();
  char WaitForKeypress();    // Image will be refreshed while waiting for the keypress.

  void CheckForRedraw();
  void RedrawForever();

private:
  bool d_lastimg_was_RGB;
  bool d_lastimg_was_YUV;

  void Redraw(GR_EVENT_EXPOSURE& ev);

  friend int MultiWindowRefresh(ImageWindow_Autorefresh_X11*const*,int nWindows);
};

/* MultiWindowRefresh also checks for keypresses. The window index of the window,
   in which the keypress has occured, is returned. Otherwise -1 is returned.
 */
int MultiWindowRefresh(ImageWindow_Autorefresh_X11*const*,int nWindows);

#endif
