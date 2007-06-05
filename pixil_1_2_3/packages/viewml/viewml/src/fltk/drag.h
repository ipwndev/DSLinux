#ifndef __DRAG_H
#define __DRAG_H

#include "qwidget.h"
#include "qpixmap.h"

#define Dnd_X_Precision 2
#define Dnd_Y_Precision 2
#define DndURL          128

#define kapp KApplication::getKApplication()

class KDNDIcon : public QWidget
{
 public:
  KDNDIcon(QPixmap & pixmap, int _x, int _y) { }

};

class KDNDWidget : public QWidget
{
  Q_OBJECT
    public:
  KDNDWidget( QWidget *_parent=0, const char *_name=0, WFlags f=0 ) :
    QWidget(_parent,_name,f) { }
// CRH     QWidget(_parent,_name,f) { cerr << "Created\n"; }
  virtual void startDrag( KDNDIcon *_icon, const char *_data, int _size,
			  int _type, int _dx, int _dy ) { }

  virtual Window findRootWindow( QPoint & p ) { return (Window)0; }
  virtual void dndMouseReleaseEvent( QMouseEvent * ) { }
  virtual void dndMouseMoveEvent( QMouseEvent *) { }
  virtual void mousePressEvent( QMouseEvent *) { }
  virtual void mouseMoveEvent( QMouseEvent * _me )
    { dndMouseMoveEvent(_me); }
  virtual void mouseReleaseEvent( QMouseEvent * _me ) 
    { dndMouseReleaseEvent(_me); }
  virtual void rootDropEvent( int _x, int _y ) { }
  virtual void rootDropEvent() { }
  virtual void resizeEvent( QResizeEvent *) { } 
  virtual void paintEvent(QPaintEvent * pe) { }
  bool drag;
};


#endif
