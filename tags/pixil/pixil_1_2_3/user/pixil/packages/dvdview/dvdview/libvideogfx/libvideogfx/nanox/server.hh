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

#include <nano-X.h>

class X11Server
{
public:
  virtual ~X11Server() { }

  virtual int AskDisplay() const = 0;
};

extern const X11Server& default_x11server;

#endif
