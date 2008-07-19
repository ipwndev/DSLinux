#include "qpainter.h"
#include "qdrawutil.h"
#include "fl_draw.H"

void qDrawShadePanel( QPainter *p, int x, int y, int w, int h,
		      const QColorGroup & cg, bool sunken,
		      int lineWidth, const QBrush *fill ) 
{
  p->qDrawShadePanel(x,y,w,h,cg,sunken,lineWidth,fill);
}

void qDrawShadeLine( QPainter *p, int x1, int y1, int x2, int y2,
			    const QColorGroup &g, bool sunken,
			    int lineWidth, int midLineWidth )
{ 
  p->qDrawShadeLine(x1,y1,x2,y2,g,sunken,lineWidth, midLineWidth);
}

void qDrawShadeLine( QPainter *p, const QPoint &p1, const QPoint &p2,
		     const QColorGroup &g, bool sunken,
		     int lineWidth, int midLineWidth )
{ 
  qDrawShadeLine(p,p1.x(),p1.y(),p2.x(),p2.y(),g,
		 sunken,lineWidth,midLineWidth);
}
