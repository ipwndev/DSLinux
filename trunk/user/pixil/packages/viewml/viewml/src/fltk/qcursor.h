#ifndef __QCURSOR_H
#define __QCURSOR_H

#include "qpoint.h"
#include "qbitmap.h"

class QCursor
{
 public:
  QCursor() { }
  QCursor(const QBitmap & bitmap, const QBitmap & mask, int hotX=-1,
	    int hotY=-1) { }
  static QPoint pos() { return QPoint(0,0); }
};

extern const QCursor arrowCursor;      
extern const QCursor upArrowCursor;    
extern const QCursor crossCursor;      
extern const QCursor waitCursor;       
extern const QCursor ibeamCursor;      
extern const QCursor sizeVerCursor;    
extern const QCursor sizeHorCursor;    
extern const QCursor sizeBDiagCursor;  
extern const QCursor sizeFDiagCursor;  
extern const QCursor sizeAllCursor;    
extern const QCursor blankCursor;      

#endif
