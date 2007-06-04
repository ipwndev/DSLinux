#include "qfont.h"



#ifdef NEVER // *** original method saved for posterity
void QFont::SelectFont() const
{
  if(weight() > Normal)
    fl_font(FL_COURIER+1,m_nPixelSize);
  else
    fl_font(FL_COURIER,m_nPixelSize);    
}
#endif

void QFont::SelectFont() const
{
  char fname[256];
  int ftype;

  // determine base font
  strcpy(fname,(char *)m_sFamily);
  if(fname[0] == 0x00)
    ftype = FL_HELVETICA;
  else if(stricmp(fname,"times") == 0)
    ftype = FL_TIMES;
  else if(stricmp(fname,"courier") == 0)
    ftype = FL_COURIER;
  else
    ftype = FL_HELVETICA;

  // determine font weight
  if(weight() == Bold)
    ftype += FL_BOLD;

  // determine font italic
  if(italic() == TRUE)
    ftype += FL_ITALIC;

  // set the font
  fl_font(ftype,m_nPixelSize);    
}

// CRH for qpushbutton
const int QFont::getFont() const
{
  char fname[256];
  int ftype;

  // determine base font
  strcpy(fname,(char *)m_sFamily);
  if(fname[0] == 0x00)
    ftype = FL_HELVETICA;
  else if(stricmp(fname,"times") == 0)
    ftype = FL_TIMES;
  else if(stricmp(fname,"courier") == 0)
    ftype = FL_COURIER;
  else
    ftype = FL_HELVETICA;

  // determine font weight
  if(weight() == Bold)
    ftype += FL_BOLD;

  // determine font italic
  if(italic() == TRUE)
    ftype += FL_ITALIC;

  // return the font
  return ftype;    
}


QFont::QFont() 
{ 
  m_nPixelSize=12;
}



#ifdef NEVER // *** original method saved for posterity
QFont::QFont ( const QString & family, int pointSize,
	       int weight, bool italic) 
{
  m_nPixelSize = pointSize;
}
#endif

QFont::QFont ( const QString & family, int pointSize,
	       int weight, bool italic) 
{
  m_sFamily = family;
  m_nPixelSize = pointSize;
  m_nWeight = weight;
  m_bItalic = italic;
  m_bUnderline = false;
  m_bStrikeOut = false;
}





















