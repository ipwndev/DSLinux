#include <stdio.h>
#include "qrect.h"

//#define QRECTDEBUG
//#define QRECTDEBUGPAUSE

/*
**
** Returns TRUE if the rectangle 'r' is inside this rectangle.
**
** If 'proper' is TRUE, this function returns TRUE only if 'r' is entirely
** inside (not on the edge).
**
*/
bool QRect::contains(const QRect & r, bool proper=false) const
{
  bool result;
  int b1,b2,l1,l2,r1,r2,t1,t2;

  t1 = m_nY;
  b1 = t1 + m_nHeight - 1;
  l1 = m_nX;
  r1 = l1 + m_nWidth - 1;

  t2 = r.m_nY;
  b2 = t2 + r.m_nHeight - 1;
  l2 = r.m_nX;
  r2 = l2 + r.m_nWidth - 1;

  if(proper)
  {
    t1++; b1--; l1++; r1--;
  }

  if((t2 >= t1) && (b2 <= b1) && (l2 >= l1) && (r2 <= r1))
    result = true;
  else
    result = false;

#ifdef QRECTDEBUG
  printf("QRect::contains:\n");
  printf("R1: t:%s b:%d l:%d r:%d\n",t1,b1,l1,r1);
  printf("R2: t:%s b:%d l:%d r:%d\n",t2,b2,l2,r2);
  if(result)
    printf("Returning true\n");
  else
    printf("Returning false\n");
#endif
#ifdef QRECTDEBUGPAUSE
  getchar();
#endif

  return(result);
}

/*
**
** Returns TRUE if the point 'p' is inside or on the edge of the rectangle.
**
** If 'proper' is TRUE, this function returns TRUE only if 'p' is inside
** (not on the edge).
**
*/
bool QRect::contains(const QPoint & p, bool proper=false) const
{
  bool result;
  int px,py,rb,rl,rr,rt;

  rt = m_nY;
  rb = rt + m_nHeight - 1;
  rl = m_nX;
  rr = rl + m_nWidth - 1;

  px = p.x();
  py = p.y();

  if(proper)
  {
    rt++; rb--; rl++; rr--;
  }
  
  if((py >= rt) && (py <= rb) && (px >= rl) && (py <= rr))
    result = true;
  else
    result = false;

#ifdef QRECTDEBUG
  printf("QRect::contains:\n");
  printf("R: t:%s b:%d l:%d r:%d\n",rt,rb,rl,rr);
  printf("P: x:%s y:%d\n",px,py);
  if(result)
    printf("Returning true\n");
  else
    printf("Returning false\n");
#endif
#ifdef QRECTDEBUGPAUSE
  getchar();
#endif

  return(result);
}

/*
**
** Returns TRUE if this rectangle intersects with 'r' (there is at least one
** pixel which is within both rectangles).
**
*/
bool QRect::intersects(const QRect & r) const
{
  bool result;
  int b1,b2,l1,l2,r1,r2,t1,t2;

  t1 = m_nY;
  b1 = t1 + m_nHeight - 1;
  l1 = m_nX;
  r1 = l1 + m_nWidth - 1;

  t2 = r.m_nY;
  b2 = t2 + r.m_nHeight - 1;
  l2 = r.m_nX;
  r2 = l2 + r.m_nWidth - 1;

  if((b2 < t1) || (t2 > b1) || (r2 < l1) || (l2 > r1))
    result = true;
  else
    result = false;

#ifdef QRECTDEBUG
  printf("QRect::intersects:\n");
  printf("R1: t:%s b:%d l:%d r:%d\n",t1,b1,l1,r1);
  printf("R2: t:%s b:%d l:%d r:%d\n",t2,b2,l2,r2);
  if(result)
    printf("Returning true\n");
  else
    printf("Returning false\n");
#endif
#ifdef QRECTDEBUGPAUSE
  getchar();
#endif
  
  return(result);
}

/*
**
** Returns the intersection of the two QRect's. If they don't overlap,
** a QRect at 0,0 with a width and height of 0 is returned.
**
*/
QRect QRect::intersect(const QRect & r) const
{
  QRect result;
  int b1,b2,b3,l1,l2,l3,r1,r2,r3,t1,t2,t3;

  if(intersects(r) == false)
  {
    result.m_nX = result.m_nY = 0;
    result.m_nWidth = result.m_nHeight = 0;
    return(result);
  }

  t1 = m_nY;
  b1 = t1 + m_nHeight - 1;
  l1 = m_nX;
  r1 = l1 + m_nWidth - 1;

  t2 = r.m_nY;
  b2 = t2 + r.m_nHeight - 1;
  l2 = r.m_nX;
  r2 = l2 + r.m_nWidth - 1;

  if(t1 > t2)
    t3 = t1;
  else
    t3 = t2;

  if(b1 < b2)
    b3 = b1;
  else
    b3 = b2;

  if(l1 > l2)
    l3 = l1;
  else
    l3 = l2;

  if(r1 < r2)
    r3 = r1;
  else
    r3 = r2;

  result.m_nX = l3;
  result.m_nY = t3;
  result.m_nWidth = (r3 - l3) + 1;
  result.m_nHeight = (b3 - t3) + 1;

#ifdef QRECTDEBUG
  printf("QRect::intersect:\n");
  printf("R1: t:%s b:%d l:%d r:%d\n",t1,b1,l1,r1);
  printf("R2: t:%s b:%d l:%d r:%d\n",t2,b2,l2,r2);
  printf("Result: t:%s b:%d l:%d r:%d\n",t3,b3,l3,r3);
#endif
#ifdef QRECTDEBUGPAUSE
  getchar();
#endif

  return(result);
}
