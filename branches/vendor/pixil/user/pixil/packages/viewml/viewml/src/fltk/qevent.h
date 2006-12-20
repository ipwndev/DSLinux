#ifndef __QEVENT_H
#define __QEVENT_H

#include "qpoint.h"
#include "qrect.h"
#include "qsize.h"
#include "qkeycode.h"

#define Event_None		    0		// invalid event
#define Event_Timer		    1		// timer event
#define Event_MouseButtonPress	    2		// mouse button pressed
#define Event_MouseButtonRelease    3		// mouse button released
#define Event_MouseButtonDblClick   4		// mouse button double click
#define Event_MouseMove		    5		// mouse move
#define Event_KeyPress		    6		// key pressed
#define Event_KeyRelease	    7		// key released
#define Event_FocusIn		    8		// keyboard focus received
#define Event_FocusOut		    9		// keyboard focus lost
#define Event_Enter		   10		// mouse enters widget
#define Event_Leave		   11		// mouse leaves widget
#define Event_Paint		   12		// paint widget
#define Event_Move		   13		// move widget
#define Event_Resize		   14		// resize widget
#define Event_Create		   15		// after object creation
#define Event_Destroy		   16		// during object destruction
#define Event_Show		   17		// widget is shown
#define Event_Hide		   18		// widget is hidden
#define Event_Close		   19		// request to close widget
#define Event_Quit		   20		// request to quit application
#define Event_Accel		   30		// accelerator event
#define Event_Clipboard		   40		// internal clipboard event
#define Event_SockAct		   50		// socket activation
#define Event_DragEnter		   60		// drag moves into widget
#define Event_DragMove		   61		// drag moves in widget
#define Event_DragLeave		   62		// drag leaves or is cancelled
#define	Event_Drop		   63		// actual drop
#define	Event_DragResponse	   64		// drag accepted/rejected
#define Event_ChildInserted	   70		// new child widget
#define Event_ChildRemoved	   71		// deleted child widget
#define Event_LayoutHint	   72		// child min/max size changed
#define Event_ActivateControl	   80		// ActiveX activation
#define Event_DeactivateControl	   81		// ActiveX deactivation
#define Event_User		 1000		// first user event id

class QEvent
{
 public:
  QEvent() {} 
  QEvent(int type) { }

};

class QPaintEvent : public QEvent
{
 public:
  QPaintEvent( const QRect &paintRect )
    : QEvent(Event_Paint), r(paintRect) {}
  const QRect &rect() const	{ return r; }
 protected:
  QRect r;
};

class QResizeEvent : public QEvent
{
public:
    QResizeEvent( const QSize &size, const QSize &oldSize )
	: QEvent(Event_Resize), s(size), olds(oldSize) {}
    const QSize &size()	  const { return s; }
    const QSize &oldSize()const { return olds;}
protected:
    QSize s, olds;
};

enum ButtonState {				// mouse/keyboard state values
    NoButton	    = 0x00,
    LeftButton	    = 0x01,
    RightButton	    = 0x02,
    MidButton	    = 0x04,
    MouseButtonMask = 0x07,
    ShiftButton	    = 0x08,
    ControlButton   = 0x10,
    AltButton	    = 0x20,
    KeyButtonMask   = 0x38
};

class QMouseEvent : public QEvent
{
 protected:
  static QPoint g;
  QPoint m_Point;
  int	   b;
  unsigned short s;
 public:
  QMouseEvent() { s = 0; b = 0; }
  const QPoint &pos() const	{ return m_Point; }
  const QPoint &globalPos() const { return g; }
  int	   x()		const	{ return m_Point.x(); }
  int	   y()		const	{ return m_Point.y(); }
  int	   globalX()		const	{ return g.x(); }
  int	   globalY()		const	{ return g.y(); }
  int	   button()	const	{ return b; }
  int	   state()	const	{ return s; }
  
  void setButton(int _b) { b = _b; }
  void setPos(const QPoint & pos) { m_Point = pos; }
  void setGlobalPos(const QPoint & pos) { g = pos; }
};

class QCloseEvent : public QEvent
{
 public:
  void accept() { }
};

class QKeyEvent : public QEvent
{
public:
  QKeyEvent( int type, int key, int ascii, int state )
    : QEvent(type), k((unsigned short)key), 
    s((unsigned short)state), 
    a((unsigned char)ascii),
    accpt(true) {}
    int	   key()	const	{ return k; }
    int	   ascii()	const	{ return a; }
    int	   state()	const	{ return s; }
    bool   isAccepted() const	{ return accpt; }
    void   accept()		{ accpt = true; }
    void   ignore()		{ accpt = false; }
protected:
    unsigned short k, s;
    unsigned char  a;
    char   accpt;
};

class QTimerEvent : public QEvent
{

};

class QFocusEvent : public QEvent
{

};

#endif
