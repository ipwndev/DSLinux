/*
 *  server.cc
 */

#include "config.h"

#include "server.hh"


class X11Server_Default : public X11Server
{
public:
  X11Server_Default()
    {
      display = XOpenDisplay(NULL);
    }
  
  ~X11Server_Default()
    {
      /* NOTE: eigentlich sollte der server schon wieder zugemacht werden,
	 aber wenn globale Windows erstellt werden kann es passieren, dass
	 der Server vor den Windows zugemacht wird, was dann schiefgeht. */
      // XCloseDisplay(display);
    }
    
  Display* AskDisplay() const { return display; }

private:
  Display* display;
};

static X11Server_Default default_x11server_OBJECT;
const X11Server& default_x11server = default_x11server_OBJECT;



