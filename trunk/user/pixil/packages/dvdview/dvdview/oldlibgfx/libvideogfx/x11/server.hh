/*********************************************************************
  libvideogfx/x11/server.hh

  purpose:

  notes:

  to do:

  author(s):
   - Dirk Farin, farin@ti.uni-mannheim.de

  modifications:
 *********************************************************************/

#ifndef LIBVIDEOGFX_X11_SERVER_HH
#define LIBVIDEOGFX_X11_SERVER_HH

#include <X11/Xlib.h>

class X11Server
{
public:
  virtual ~X11Server() { }

  virtual Display* AskDisplay() const = 0;
};

extern const X11Server& default_x11server;

#endif
